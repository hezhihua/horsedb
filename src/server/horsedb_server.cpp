

#include <vector>
#include <cassert>
#include <iostream>


#include "mysql_handle.h"
#include "horsedb_server.h"

#include "mysql/socket_context.h"
#include "client/ServantHandle.h"
#include "client/Servant.h"
#include "client/Communicator.h"
#include "logger/logger.h"
#include "cfg/config.h"

#include "raft/RaftDB.h"

#include "kv/dbimp.h"

using namespace std;
using namespace horsedb;
using namespace spdlog;

HorsedbServer g_app;




static bool onConnectEvent(const std::vector<char>& vBuffer)
{
	cout<<"onConnectEvent"<<endl;
	return true;
}


void HorsedbServer::initialize(const string &cfgPath)
{
	cout << "initialize ok" << endl;

	// _async_logger =  spdlog::rotating_logger_mt<spdlog::async_factory>("demosrv", "logs/demoserver.log", 1024 * 1024 * 5, 3);
	// auto daily_logger = spdlog::daily_logger_mt("daily_logger", "logs/demoserver.txt", 2, 30);
	// spdlog::set_level(spdlog::level::debug);
	// //async_logger->flush_on(spdlog::level::info);
	// daily_logger->flush_on(spdlog::level::debug);

	// spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][t:%t][%L]%v");
	// spdlog::flush_every(std::chrono::seconds(1));
	// _async_logger->info("This is a basic logger.");
	// daily_logger->info("This is a basic logger.");

	horsedb::Logger::getInstance()->init("horsedb", "logs/horsedb.log", "logs/horsedb.raft.log");


	_epollServer->setLocalLogger(horsedb::Logger::getInstance()->getLogger());
	_cfgPath=cfgPath;

	_epollServer->setMergeHandleNetThread(true);

	horsedb::Config::getInstance()->init(_cfgPath);//YAML::LoadFile(_cfgPath);// /root/svn/horsedb/config/server.cfg.yaml
	auto &_config=Config::getInstance()->getConfig();

	_bRaft=_config["server"]["raft_enable"].as<bool>();

	cout << "bind_mysql:" <<_config["server"]["bind_mysql"].as<string>()<< endl;
	cout << "rocksdb.path:" <<_config["rocksdb"]["path"].as<string>()<< endl;

	cout << "raft.init_nodes:" <<_config["raft"]["init_nodes"].as<string>()<< endl;
	cout << "raft.groupid:" <<_config["raft"]["groupid"].as<string>()<< endl;
	

	string rocksdbPath=_config["rocksdb"]["path"].as<string>();
	string dbNames=_config["rocksdb"]["dbname"].as<string>();
	vector<string> vdb=TC_Common::sepstr<string>(dbNames,",");
	auto it=std::find(vdb.begin(),vdb.end(),"sys");
	if (it==vdb.end())
	{
		vdb.push_back("sys");
	}

	_db= _bRaft? shared_ptr<DBBase>(DBBaseImp::getInstance()) : shared_ptr<DBBase>(DBBase::getInstance());

	_db->init(rocksdbPath,vdb);
	//_db= std::make_shared<DBBase>(rocksdbPath,vdb);
	_meta=std::make_shared<Meta>(_db);
	_table=std::make_shared<Table>(_meta,_db);

	
	_comm=new Communicator();
	

	static string raftObj = "horsedb.RaftDBServer.RaftDBObj@tcp -h 0.0.0.0 -p 8085";//"horsedb.RaftDBServer.RaftDBObj@tcp -h 139.186.68.18 -p 8085 -e 1";
	_comm->setProperty("sendqueuelimit", "1000000");
	_comm->setProperty("asyncqueuecap", "1000000");
	_comm->setProperty("netthread", TC_Common::tostr(1));

	//vector<string> vAddr=TC_Common::sepstr<string>(_config["raft"]["init_nodes"].as<string>(),":");
	//for (size_t i = 0; i < vAddr.size(); i++)
	{
		_pPrx= _comm->stringToProxy<RaftDBPrx>(raftObj);

		_pPrx->tars_connect_timeout(5000);
		_pPrx->tars_async_timeout(60*1000);

		//_mPrx[vAddr[i]]=_pPrx;
		
	}
		
	if (_bRaft)
	{
		_smImp.start();
	}

}



void HorsedbServer::bindMysql()
{
	auto &_config=Config::getInstance()->getConfig();
	TC_EpollServer::BindAdapterPtr lsPtr (new TC_EpollServer::BindAdapter(_epollServer));

	//设置adapter名称, 唯一
	lsPtr->setName("MysqlAdapter");
	//设置绑定端口
	lsPtr->setEndpoint(_config["server"]["bind_mysql"].as<string>());
	//设置最大连接数
	lsPtr->setMaxConns(1024);
	//设置启动线程数
	lsPtr->setHandle<MysqlHandle>(1);
	//设置协议解析器
	lsPtr->setProtocol(MysqlHandle::parseMysql<false>);
//		lsPtr->enableQueueMode();

	std::vector<char> vBuffer;
	AuthRequest authRequest;
	SocketContext socketContext;

	SocketContext::getShakeInitBuffer(authRequest,vBuffer);
	socketContext._packetIDIsReset=true;
	socketContext._AuthRequest=authRequest;


	lsPtr->setOnConnectEvent(onConnectEvent,vBuffer,socketContext);
	cout << "SocketAdapter::MysqlHandle ok" << endl;

	//绑定对象
	_epollServer->bind(lsPtr);
	


	cout << "SocketAdapter::bindMysql ok" << endl;
}

void HorsedbServer::bindRPC()
{
	auto &_config=Config::getInstance()->getConfig();
	TC_EpollServer::BindAdapterPtr lsPtr (new TC_EpollServer::BindAdapter(_epollServer));

	//设置adapter名称, 唯一
	lsPtr->setName("RpcAdapter");
	//设置绑定端口
	lsPtr->setEndpoint(_config["raft"]["local_node"].as<string>());
	//设置最大连接数
	lsPtr->setMaxConns(TC_EpollServer::BindAdapter::DEFAULT_MAX_CONN);

	lsPtr->setQueueCapacity(TC_EpollServer::BindAdapter::DEFAULT_QUEUE_CAP);

    lsPtr->setQueueTimeout(TC_EpollServer::BindAdapter::DEFAULT_QUEUE_TIMEOUT);

    lsPtr->setProtocolName("tars");

    lsPtr->setProtocol(AppProtocol::parse);

	//设置启动线程数
	lsPtr->setHandle<ServantHandle>(1);

	ServantManager::getInstance()->add<RaftDB>("RpcAdapter","horsedb.RaftDBServer.RaftDBObj");


	//绑定对象
	_epollServer->bind(lsPtr);



	cout << "SocketAdapter::bindRPC ok" << endl;
}

void HorsedbServer::waitForShutdown()
{
	_epollServer->waitForShutdown();
}



int main(int argc, char** argv)
{
	try
	{
		if (argc<2)
		{
			cerr<<"./horsedb-server ./server.cfg.yaml"<<endl;
			return -1;
		}
		

		TC_Common::ignorePipe();


		g_app.initialize(argv[1]);
		
		g_app.bindMysql();
		g_app.bindRPC();

		g_app.waitForShutdown();

	}
	catch (exception &ex)
	{
		cerr << "HorsedbServer::run ex:" << ex.what() << endl;
	}

	cout << "HorsedbServer::run HorsedbServer  thread exit." << endl;
}