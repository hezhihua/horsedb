#ifndef _RAFT_CLIENT_CONTEXT_IMP_
#define _RAFT_CLIENT_CONTEXT_IMP_
#include "server/mysql_handle.h"
#include "raft/LogEntryContext.h"
#include "logger/logger.h"

namespace horsedb {

struct ContextDetail :public ClientContextOption{

    SocketContext _socketContext;
    shared_ptr<TC_EpollServer::SendContext> _sc;
    TC_EpollServer::Handle *_handle;
    bool bSend;

};


struct ClientContextImp:public ClientContext
{
    ClientContextImp(ClientContextOption *contextOption):ClientContext(contextOption)
    {
        _cco=contextOption;
        init();
    }
    void init()
    {
        if (_cco!=nullptr)
        {
            _tContextDetail=dynamic_cast<ContextDetail*>(_cco);
        }
        
        
    }


    void send()
    {
        if (_CommandType==CM_Put)
        {
            TLOGINFO_RAFT( "sendOKFull,sid="<< _sessionid <<endl);
            SocketContext::sendOKFull(_tContextDetail->_socketContext,_tContextDetail->_sc,_tContextDetail->_handle,1,0,2,0);
        }
        else if (_CommandType==CM_Del)
        {
            SocketContext::sendOK(_tContextDetail->_socketContext,_tContextDetail->_sc,_tContextDetail->_handle);
        }

    }
    
    ContextDetail *_tContextDetail;
};

}



#endif