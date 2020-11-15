#ifndef _SOCKET_CONTEXT_
#define _SOCKET_CONTEXT_
#include<stdint.h>
#include<string>
#include<memory>
#include<vector>
#include<util/tc_epoll_server.h>
using namespace std;
using namespace horsedb;
namespace horsedb{

enum MysqlVersion { VERSION_PRE41, VERSION_41 };


struct AuthRequest{

uint8_t _protocolVersion;
string  _serverVersionName;
uint32_t _serverVersion;
uint32_t _threadID;
string  _authPluginData;
uint32_t _capabilities;
uint8_t _charset;
uint16_t _serverStatus;
string _authPluginName;
};

typedef std::shared_ptr<AuthRequest> AuthRequestPtr;

struct AuthResponse{

    uint32_t  _clientCapabilities;
	uint32_t _serverCapabilities;
	uint32_t _maxPacketSize;
	uint8_t _charset;
	string _userName;
	string _authPluginData;
	string _database;
	string _authPluginName;
	bool _sslRequest;
};
typedef std::shared_ptr<AuthResponse> AuthResponsePtr;





class SocketContext {

public:

 static void initAuthRequest(AuthRequest &authRequest);
 static void initAuthResponse( uint32_t  serverCapabilities,AuthResponse &authResponse);

 static bool getShakeInitBuffer(  AuthRequest &authRequest,std::vector<char>& vBuffer);

 static bool sendData( SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle,const string& sBodydata);


 static int  sendOKFull( SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle,uint64_t affectedRows,uint64_t insertID, uint64_t serverStatus, uint64_t warnings);
 static int  sendOK( SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle);

public:


    uint32_t _isMultiStmtSet:1;
	uint32_t _isClientCompressed:1;

	uint8_t _lastPacketID;
	bool   _packetIDIsReset;

    uint8_t _charsetCode;


    string _defaultDB;  

    string _username;
    string _group;

    string _charset;
    string _charsetClient;
    string _charsetConnection;
    string _charsetResults;
    string _sqlMode;

	//trasaction
	bool isStartTrasaction;
	bool isCommit;
	bool isRollback;
	string sXid;

    AuthRequest _AuthRequest;//server send
    AuthResponse _AuthResponse;//client send

	bool _bSkipTable=true;


};


}


#endif