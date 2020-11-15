#ifndef __MYSQL_HANDLE__
#define __MYSQL_HANDLE__

#include "util/tc_http.h"

#include "util/tc_network_buffer.h"
#include "util/tc_epoll_server.h"

#include "mysql/mysql_packet.h"
#include "mysql/socket_context.h"

/**
 * 处理类, 每个处理线程一个对象
 */

namespace horsedb{

    class MysqlHandle : public TC_EpollServer::Handle
    {

    public:
        virtual void initialize();
        virtual void handle(const shared_ptr<TC_EpollServer::RecvContext> &data);
        virtual void handleClose(const shared_ptr<TC_EpollServer::RecvContext> &data);
        virtual void heartbeat(){}
        

        static  PACKET_TYPE parseEcho(TC_NetWorkBuffer&in, vector<char> &out)
        {
            TC_EpollServer::Connection *c = (TC_EpollServer::Connection *)in.getConnection();
            cout << c->getIp() << endl;
            try
            {
                out = in.getBuffers();
                in.clearBuffers();
                return PACKET_FULL;
            }
            catch (exception &ex)
            {
                return PACKET_ERR;
            }

            return PACKET_LESS;             //表示收到的包不完全
        }

        template<typename T>
        static T net2host(T len)
        {
            switch(sizeof(T))
            {
                case sizeof(uint8_t): return len;
                case sizeof(uint16_t): return ntohs(len);
                case sizeof(uint32_t): return ntohl(len);
            }
            assert(true);
            return 0;
        }
        template< bool netorder>
        static PACKET_TYPE parseMysql(TC_NetWorkBuffer& in, vector<char>& out)
        {
            size_t len = MYSQL_HEADER_SIZE-1;//前面三个字节为长度

            if (in.getBufferLength() < len)
            {
                return PACKET_LESS;
            }

            string header;
            in.getHeader(len, header);

            assert(header.size() == len);

            int iBodyLen = 0;

            ::memcpy(&iBodyLen, header.c_str() , len);//网络字节序为大端,本机为小端字节序,mysql 协议为小端字节序

            if (netorder)
            {
                iBodyLen = net2host<int>(iBodyLen);
            }

            cout<<"iBodyLen="<<iBodyLen<<endl;
            //长度保护一下
            if (iBodyLen > 1024*1024*10-MYSQL_HEADER_SIZE)
            {
                return PACKET_ERR;
            }

            if (in.getBufferLength() < (uint32_t)(MYSQL_HEADER_SIZE+iBodyLen))
            {
                return PACKET_LESS;
            }

            in.getHeader(MYSQL_HEADER_SIZE+iBodyLen, out);

            assert(out.size() == static_cast<size_t>(MYSQL_HEADER_SIZE+iBodyLen));

            in.moveHeader(MYSQL_HEADER_SIZE+iBodyLen);

            return PACKET_FULL;
        }



    };

}






#endif