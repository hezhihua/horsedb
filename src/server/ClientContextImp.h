#ifndef _RAFT_CLIENT_CONTEXT_IMP_
#define _RAFT_CLIENT_CONTEXT_IMP_
#include "server/mysql_handle.h"
#include "raft/LogEntryContext.h"

namespace horsedb {

struct ContextDetail{

    SocketContext _socketContext;
    shared_ptr<TC_EpollServer::SendContext> _sc;
    TC_EpollServer::Handle *_handle;
};
struct ClientContextImp:public ClientContext
{
    ClientContextImp(void *contextdt):ClientContext(contextdt)
    {
        init();
    }
    void init()
    {
        if (_contextdt!=nullptr)
        {
            _tContextDetail=*(ContextDetail *)_contextdt;
        }
        
        
    }


    void send()
    {
        if (_CommandType==CM_Put)
        {
            SocketContext::sendOKFull(_tContextDetail._socketContext,_tContextDetail._sc,_tContextDetail._handle,1,0,2,0);
        }
        else if (_CommandType==CM_Del)
        {
            SocketContext::sendOK(_tContextDetail._socketContext,_tContextDetail._sc,_tContextDetail._handle);
        }

    }
    
    ContextDetail _tContextDetail;
};

}



#endif