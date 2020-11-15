#include "mysql/socket_context.h"
#include "mysql/mysql_packet.h"
#include<vector>
#include <algorithm> 

using namespace std;

namespace horsedb{

    void SocketContext::initAuthRequest(AuthRequest &authRequest)
    {

        for (int i = 0; i < 20; i++) 
        {
            /* 33 - 127 are printable characters */
            char c=(94.0 * (rand() / (RAND_MAX + 1.0))) + 33;
            authRequest._authPluginData.append(1,c) ;
        }

        authRequest._authPluginData.append(1,0x00) ;

        authRequest._authPluginName= "mysql_native_password";

        authRequest._serverStatus |= SERVER_STATUS_AUTOCOMMIT;
		authRequest._charset = 0xC0;

        authRequest._serverVersionName = "@*_*@ hosrseDB";//(@*_*@ hosrseDB)
		authRequest._threadID = 1;

        authRequest._capabilities = HORSEDB_DEFAULT_FLAGS;

    }

    void SocketContext::initAuthResponse( uint32_t  serverCapabilities,AuthResponse &authResponse)
    {

        authResponse._serverCapabilities=serverCapabilities;
        authResponse._clientCapabilities=HORSEDB_DEFAULT_FLAGS;

    }


    bool SocketContext::getShakeInitBuffer( AuthRequest &authRequest,std::vector<char>& vBuffer)
    {
        initAuthRequest(authRequest);
        string sBody,sHeader;
        MysqlPacket::packetAuthReq(sBody,authRequest);
        MysqlPacket::packetHeader(sHeader,0,sBody.size());
        cout<<"header len="<<sHeader.size()<<",body len="<<sBody.size()<<endl;

        string headbody=sHeader.append(sBody.data(),sBody.size());
        vBuffer.assign(headbody.begin(), headbody.end());

        return true;

    }

    bool SocketContext::sendData( SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &sc,TC_EpollServer::Handle *handle,const string& sBodydata)
    {
        string theSendData=sBodydata;
        do
        {
            size_t cur_packet_len = std::min(theSendData.size(), (size_t)PACKET_LEN_MAX);
            if (socketContext._packetIDIsReset) {
            socketContext._packetIDIsReset = false;
            /** the ++last_packet_id will make sure we send a 0 */
            socketContext._lastPacketID = 0xff;
            }

            string thisTimeSend;
            MysqlPacket::appendIntLen(thisTimeSend,cur_packet_len,3);
            MysqlPacket::appendIntLen(thisTimeSend,++socketContext._lastPacketID,1);
            thisTimeSend.append(theSendData.substr(0,cur_packet_len));

            vector<char> vThisTimeSend;
            vThisTimeSend.assign(thisTimeSend.begin(), thisTimeSend.end());

            sc->buffer()->setBuffer(vThisTimeSend);
            handle->sendResponse(sc);

            if (cur_packet_len==PACKET_LEN_MAX)
            {
                string thisTimeSendMax;
                MysqlPacket::appendIntLen(thisTimeSendMax,0,3);
                MysqlPacket::appendIntLen(thisTimeSendMax,++socketContext._lastPacketID,1);
                vector<char> vThisTimeSend;
                vThisTimeSend.assign(thisTimeSendMax.begin(), thisTimeSendMax.end());

                sc->buffer()->setBuffer(vThisTimeSend);
                handle->sendResponse(sc);

            }
            
            theSendData=theSendData.substr(cur_packet_len);

        } while (theSendData.size());

        return true;
        
    }



    int  SocketContext::sendOKFull( SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle,uint64_t affectedRows,uint64_t insertID, uint64_t serverStatus, uint64_t warnings)
    {
        OKPacket oKPacket;
        oKPacket._affectedRows=affectedRows;
        oKPacket._insertID=insertID;
        oKPacket._serverStatus=serverStatus;
        oKPacket._warnings=warnings;

        string sDes;
        MysqlPacket::appendOKPacket(sDes,oKPacket);
        sendData(socketContext,data,handle,sDes);

        return 0;

    }
    int  SocketContext::sendOK( SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle)
    {
        return sendOKFull(socketContext,data,handle,0,0,SERVER_STATUS_AUTOCOMMIT,0);
    }

}