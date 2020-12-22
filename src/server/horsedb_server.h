#ifndef  __HORSEDB_SERVER__
#define  __HORSEDB_SERVER__

#include "util/tc_epoll_server.h"
//#include "yaml-cpp/yaml.h"
//#include "spdlog/logger-inl.h"
#include "kv/DBBase.h"
#include "kv/meta.h"
#include "kv/table.h"
#include "raft/RaftDB.h"
#include "StateMachineImp.h"



using namespace horsedb;

class HorsedbServer
{
public:
	HorsedbServer()
	{
		_epollServer = new TC_EpollServer(3);
	};

	void initialize(const string &cfgPath);

    void bindMysql();
	void bindRPC();

    void waitForShutdown();

	std::shared_ptr<DBBase>& getDB(){return _db;}

	bool isLeader()
	{ 
		if (_smImp._node)
		{
			return _smImp._node->is_leader();
		}
		
		return false;
	}


    //std::shared_ptr<spdlog::logger>&  getlogger(){return _async_logger;}
	string _cfgPath;
    TC_EpollServer *_epollServer;
	//YAML::Node _config ;
	//std::shared_ptr<spdlog::logger> _async_logger;

	std::shared_ptr<DBBase>  _db;
	std::shared_ptr<horsedb::Meta> _meta;
	std::shared_ptr<horsedb::Table> _table;

	RaftDBPrx _pPrx ;
	map<string,RaftDBPrx> _mPrx;
	Communicator* _comm;

	StateMachineImp _smImp;
	bool _bRaft;

	std::atomic<int64_t> _sessionid;

};

extern HorsedbServer g_app;
#endif