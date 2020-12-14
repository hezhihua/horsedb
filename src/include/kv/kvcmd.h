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
        KVCmd(KVCMD_TYPE tType,string sKey,string sValue,string sDB):_tType(tType),_sKey(sKey),_sValue(sValue),_sDB(sDB){}
        KVCMD_TYPE _tType;
        string _sKey;
        string _sValue;
        string _sDB;

    };

    struct KVCmdContext:public TC_Singleton<KVCmdContext>
    { 
        void pusb_back(KVCmd &&tKVCmd) { std::unique_lock<std::mutex> lck(_mutex); _vCMDInfo.push_back(tKVCmd);}
        void getCMDInfo(vector<KVCmd>& vCMDInfo)  {std::unique_lock<std::mutex> lck(_mutex);vCMDInfo= _vCMDInfo;}
        void clear(){std::unique_lock<std::mutex> lck(_mutex);_vCMDInfo.clear();}

        vector<KVCmd> _vCMDInfo;
        std::mutex _mutex;

    };

}


#endif