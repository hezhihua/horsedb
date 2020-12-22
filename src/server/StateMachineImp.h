#ifndef _STATE_MACHINE_IMPL_
#define _STATE_MACHINE_IMPL_

#include "raft/Raft.h"
#include "mysql_handle.h"
#include "ClientContextImp.h"

namespace horsedb{

class StateMachineImp:public StateMachine
{
    public:
    virtual ~StateMachineImp(){}
    int start();
    void on_apply(Iterator& iter);
    void add_cp_task( SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle,const string &sessionid);

     void on_shutdown(){cout<<"on_shutdown"<<endl;}
    void on_snapshot_save(){cout<<"on_snapshot_save"<<endl;}
     void on_snapshot_save(const SnapshotMeta &meta){cout<<"on_snapshot_save"<<endl;}
     int on_snapshot_load(){cout<<"on_snapshot_load"<<endl;return 0;}
     void on_leader_start(int64_t term);
     void on_leader_stop();
     void on_error(){cout<<"on_error"<<endl;}
     void on_configuration_committed(const Configuration& conf){cout<<"on_configuration_committed"<<endl;}
     void on_configuration_committed(const Configuration& conf, int64_t index){cout<<"on_configuration_committed"<<endl;}
     void on_stop_following(const LeaderChangeContext& ctx){cout<<"on_stop_following"<<endl;}
     void on_start_following(const LeaderChangeContext& ctx){cout<<"on_start_following"<<endl;}





    Node* volatile _node;
    std::atomic<int64_t> _leader_term;
    NodeOptions _node_options;

    map<string,ClientContext *> _mSendContext;

};


}



#endif