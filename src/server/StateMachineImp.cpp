#include "StateMachineImp.h"
#include "StateMachineImp.h"
#include "kv/DBBase.h"
#include "client/EndpointInfo.h"
#include  "logger/logger.h"
#include "kv/kvcmd.h"

#include "cfg/config.h"
#include "horsedb_server.h"

namespace horsedb {
    int StateMachineImp::start()
    {
        string sLocalRpcAddr=Config::getInstance()->getConfig()["raft"]["local_node"].as<string>();
        string sInitNodes=Config::getInstance()->getConfig()["raft"]["init_nodes"].as<string>();
        string sGid=Config::getInstance()->getConfig()["raft"]["groupid"].as<string>();
         
        int election_timeout_ms =Config::getInstance()->getConfig()["raft"]["election_timeout_ms"].as<int>();
        int snapshot_interval_s =Config::getInstance()->getConfig()["raft"]["snapshot_interval_s"].as<int>();

        TC_Endpoint ep(sLocalRpcAddr);
        EndpointInfo addr(ep.getHost(), ep.getPort(), ep.getType(), ep.getGrid(), "", ep.getQos(), ep.getWeight(), ep.getWeightType(), ep.getAuthType());


        if (_node_options.initial_conf.parse_from(sInitNodes) != 0) 
        {
            TLOGERROR_RAFT("Fail to parse configuration `" << sInitNodes << '\''<<endl) ;
            return -1;
        }
        _node_options.election_timeout_ms = election_timeout_ms;
        _node_options.fsm = this;
        _node_options.node_owns_fsm = false;
        _node_options.snapshot_interval_s = snapshot_interval_s;
        std::string prefix = "local://horse-data";
        _node_options.log_uri = prefix + "/log";
        _node_options.raft_meta_uri = prefix + "/raft_meta";
        _node_options.snapshot_uri = prefix + "/snapshot";
        _node_options.disable_cli = false;// "Don't allow raft_cli access this node"
        _node_options.dbBase=g_app.getDB();
        Node* node = new Node(sGid, PeerId(addr));
        if (node->init(_node_options) != 0) 
        {
            TLOGERROR_RAFT( "Fail to init raft node"<<endl);
            delete node;
            return -1;
        }
        _node = node;
        return 0;

    }

    //应用到数据库
    void StateMachineImp::on_apply(Iterator& iter)
    {
        auto dbptr=_node_options.dbBase;
        for (; iter.valid(); iter.next()) 
        {
            LogEntry tLogEntry= iter.entry();
            TLOGINFO_RAFT( "apply to db,cmdtype="<<tLogEntry.cmdType <<endl);
            if (tLogEntry.cmdType == CM_Put)
            {      
                TLOGINFO_RAFT( "tPutReq.sKey="<<tLogEntry.tPutReq.sKey<<",tPutReq.sDB="<<tLogEntry.tPutReq.sDB<<endl);         
                dbptr->Put(tLogEntry.tPutReq.sKey,tLogEntry.tPutReq.sValue,tLogEntry.tPutReq.sDB);
                
            }
            else if (tLogEntry.cmdType == CM_Del)
            {
                dbptr->Delete(tLogEntry.tDelReq.sKey,tLogEntry.tDelReq.sDB);
            }
            else if (tLogEntry.cmdType == CM_Create)
            {
                dbptr->Create(tLogEntry.tPutReq.sDB);
            }

             ClientContext* cc=const_cast<ClientContext*>(iter.done());

             
            if (_node->is_leader() && cc!=nullptr)
            {
                auto it=_mSendContext.find(cc->_sessionid);
                if (it!=_mSendContext.end())
                {
                    
                    //一次请求/session可能有多次put,只响应一次
                    TLOGINFO_RAFT( "session "<< cc->_sessionid<<" had send" <<endl);
                    cc->send();
                    //响应客户端后直接删除，todo超时删除 
                    if (cc && cc->_cco)
                    {
                        delete cc->_cco;
                        delete cc;
                    }
                    _mSendContext.erase(it);

                }

            }

            
            

            

        }

        

    }

    string kvcmd2str(KVCMD_TYPE type)
    {
        if (type==KV_PUT)
        {
            return "KV_PUT";
        }else if (type==KV_DEL)
        {
            return "KV_DEL";
        }else
        {
            return "KV_CREATE";
        }
        
        
    }

    //leader执行分发任务
    void StateMachineImp::add_cp_task( SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle,const string &sessionid)
    {
        vector<KVCmd> vCMDInfo;
        KVCmdContext::getInstance()->getCMDInfo(sessionid,vCMDInfo);
        TLOGINFO_RAFT( "sessionid="<<sessionid <<",vCMDInfo.size()="<<vCMDInfo.size() <<endl);
        
        for (auto &item:vCMDInfo)
        {
            Task task;
            TLOGINFO_RAFT( "item._tType="<<kvcmd2str(item._tType) <<endl)

            if (item._tType==KV_PUT)
            {
                LogEntry tLogEntry;
                LogEntryContext tLogEntryContext;
                tLogEntry.cmdType=CM_Put;
                tLogEntry.tPutReq.sKey=item._sKey;
                tLogEntry.tPutReq.sValue=item._sValue;
                tLogEntry.tPutReq.sDB=item._sDB;
                tLogEntry.term=_node->get_term();

                tLogEntryContext._LogEntry=tLogEntry;


                auto it=_mSendContext.find(sessionid);
                if (it==_mSendContext.end())
                {
                    ContextDetail *pContextDetail=new ContextDetail();
                    pContextDetail->_handle=handle;
                    pContextDetail->_sc=data;
                    pContextDetail->_socketContext=socketContext;
                    

                    ClientContext *pClientContext=new ClientContextImp(pContextDetail);
                    pClientContext->_CommandType=CM_Put;
                    pClientContext->_sessionid=sessionid;
                    tLogEntryContext._ClientContext=pClientContext;

                    _mSendContext[sessionid]=pClientContext;
                    
                }
                else
                {
                   tLogEntryContext._ClientContext=it->second;
                }
                

                task._vLogEntry.push_back(tLogEntryContext);
                task._expected_term=_leader_term.load(std::memory_order_relaxed);
            }
            else if (item._tType==KV_DEL)
            {
                LogEntry tLogEntry;
                LogEntryContext tLogEntryContext;
                tLogEntry.cmdType=CM_Del;
                tLogEntry.tPutReq.sKey=item._sKey;
                tLogEntry.tPutReq.sValue=item._sValue;
                tLogEntry.tPutReq.sDB=item._sDB;

                tLogEntryContext._LogEntry=tLogEntry;


                auto it=_mSendContext.find(sessionid);
                if (it==_mSendContext.end())
                {
                    ContextDetail *pContextDetail=new ContextDetail();
                    pContextDetail->_handle=handle;
                    pContextDetail->_sc=data;
                    pContextDetail->_socketContext=socketContext;
                    

                    ClientContext *pClientContext=new ClientContextImp(pContextDetail);
                    pClientContext->_CommandType=CM_Del;
                    pClientContext->_sessionid=sessionid;
                    tLogEntryContext._ClientContext=pClientContext;

                    _mSendContext[sessionid]=pClientContext;
                    
                }
                else
                {
                   tLogEntryContext._ClientContext=it->second;
                }
                

                task._vLogEntry.push_back(tLogEntryContext);
                task._expected_term=_leader_term.load(std::memory_order_relaxed);
            }

            _node->apply(task);
            
        }

        KVCmdContext::getInstance()->clear(sessionid);


    }

    void StateMachineImp::on_leader_start(int64_t term)
    {
        _leader_term.store(term, std::memory_order_release);
        TLOGINFO_RAFT( "Node becomes leader"<<endl)
    }
     void StateMachineImp::on_leader_stop(){
         _leader_term.store(-1, std::memory_order_release);
        TLOGINFO_RAFT(  "Node stepped down : " << endl)
     }
}