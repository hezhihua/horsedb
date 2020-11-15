#ifndef __ADMIN_COMMAND__
#define  __ADMIN_COMMAND__

#include<string>
#include "mysql/socket_context.h"

using namespace std;


namespace horsedb {

    class Command {
        public:
        static void  adminSelectVersionComment(SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle);
        static void  adminShowDatabases(SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,const vector<string> &vDatabase,TC_EpollServer::Handle *handle);
        static void  adminShowTables(SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,const vector<string> &vTable,TC_EpollServer::Handle *handle);
        static void  adminShowFields(SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle);

        static void  selectRowData(SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle,vector<string> &vCloumnName, vector<map<string,string>> &vRowData);
    };
}


#endif