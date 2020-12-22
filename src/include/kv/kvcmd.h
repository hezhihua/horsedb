#ifndef __KVCMD__
#define __KVCMD__
#include <string>
#include "util/tc_singleton.h"

using namespace std;


namespace horsedb{

    enum KVCMD_TYPE{
        KV_PUT,
        KV_DEL,
        KV_CREATE

    };

    struct KVCmd
    {
        KVCmd(KVCMD_TYPE tType,string sKey,string sValue,string sDB,const map<string,string> &msession):_tType(tType),_sKey(sKey),_sValue(sValue),_sDB(sDB)
        ,_mSession(msession){}
        KVCMD_TYPE _tType;
        string _sKey;
        string _sValue;
        string _sDB;
        map<string,string> _mSession;

    };

    struct KVCmdContext:public TC_Singleton<KVCmdContext>
    { 
        void pusb_back(KVCmd &&tKVCmd) 
        { 
            std::unique_lock<std::mutex> lck(_mutex); 
            string sessionid=tKVCmd._mSession["sid"]   ;       
            _mCMDInfo[sessionid].push_back(tKVCmd);
        }
        void getCMDInfo(const string& sessionid,vector<KVCmd>& vCMDInfo)  
        {
            std::unique_lock<std::mutex> lck(_mutex);
            vCMDInfo= _mCMDInfo[sessionid];
        }
        void clear(){std::unique_lock<std::mutex> lck(_mutex);_mCMDInfo.clear();}
        void clear(const string& sid)
        {
            std::unique_lock<std::mutex> lck(_mutex);
            auto it=_mCMDInfo.find(sid);
            if (it!=_mCMDInfo.end())
            {
                _mCMDInfo.erase(it);
            }
            
        }

        map<string,vector<KVCmd>>  _mCMDInfo;
        std::mutex _mutex;

    };

}


#endif