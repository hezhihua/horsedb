#include "StateMachineImp.h"
#include "StateMachineImp.h"
#include "kv/DBBase.h"
#include "client/EndpointInfo.h"
#include  "logger/logger.h"
#include "kv/kvcmd.h"
#include "ClientContextImp.h"
#include "cfg/config.h"

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

        NodeOptions node_options;
        if (node_options.initial_conf.parse_from(sInitNodes) != 0) 
        {
            TLOGERROR_RAFT("Fail to parse configuration `" << sInitNodes << '\''<<endl) ;
            return -1;
        }
        node_options.election_timeout_ms = election_timeout_ms;
        node_options.fsm = this;
        node_options.node_owns_fsm = false;
        node_options.snapshot_interval_s = snapshot_interval_s;
        std::string prefix = "local://horse-data";
        node_options.log_uri = prefix + "/log";
        node_options.raft_meta_uri = prefix + "/raft_meta";
        node_options.snapshot_uri = prefix + "/snapshot";
        node_options.disable_cli = false;// "Don't allow raft_cli access this node"
        Node* node = new Node(sGid, PeerId(addr));
        if (node->init(node_options) != 0) 
        {
            TLOGERROR_RAFT( "Fail to init raft node"<<endl);
            delete node;
            return -1;
        }
        _node = node;
        return 0;

    }
    void StateMachineImp::on_apply(Iterator& iter)
    {
        auto dbptr=DBBase::getInstance();
        for (; iter.valid(); iter.next()) 
        {
            LogEntry tLogEntry= iter.entry();
            if (tLogEntry.cmdType == CM_Put)
            {               
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

             ClientContext* cc=(ClientContext* )iter.done();
            if (_node->is_leader())
            {
                cc->send();//响应客户端
            }

            if (cc && cc->_contextdt)
            {
                delete cc->_contextdt;
                delete cc;
            }
            

            

        }

        

    }

    //leader执行
    void StateMachineImp::add_cp_task( SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle)
    {
        vector<KVCmd> vCMDInfo;
        KVCmdContext::getInstance()->getCMDInfo(vCMDInfo);
        for (auto &item:vCMDInfo)
        {
            Task task;
            
            if (item._tType==KV_PUT)
            {
                LogEntry tLogEntry;
                LogEntryContext tLogEntryContext;
                tLogEntry.cmdType=CM_Put;
                tLogEntry.tPutReq.sKey=item._sKey;
                tLogEntry.tPutReq.sValue=item._sValue;

                tLogEntryContext._LogEntry=tLogEntry;
                
                ContextDetail *pContextDetail=new ContextDetail();
                pContextDetail->_handle=handle;
                pContextDetail->_sc=data;
                pContextDetail->_socketContext=socketContext;

                ClientContext *pClientContext=new ClientContextImp((void *)pContextDetail);
                pClientContext->_CommandType=CM_Put;

                tLogEntryContext._ClientContext=pClientContext;

                task._vLogEntry.push_back(tLogEntryContext);
                task._expected_term=_leader_term.load(std::memory_order_relaxed);
            }
            else if (false)
            {
                /* code */
            }

            _node->apply(task);
            
        }

        KVCmdContext::getInstance()->clear();


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