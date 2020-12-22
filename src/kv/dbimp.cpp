#include "kv/dbimp.h"
#include "kv/kvcmd.h"
#include <iostream>
#include "logger/logger.h"
#include  "util/tc_common.h"
#include  "server/horsedb_server.h"

using namespace std;

namespace horsedb{

    bool DBBaseImp::Create(const string&dbname,const map<string,string> &msession)
    {
        if (!DBExist(dbname))
        {
            ColumnFamilyHandle* cf;
            auto s = _db->CreateColumnFamily(ColumnFamilyOptions(), dbname, &cf);
            TLOGINFO_RAFT("CreateColumnFamily status:"<<s.ToString()<<endl);
            assert(s.ok());   

            _mhandles[dbname]= cf;
            if (s.ok() && g_app.isLeader())
            {
                
                KVCmdContext::getInstance()->pusb_back(KVCmd(KV_CREATE,"","",dbname,msession));
            }
        }
        
         
        return true;

    }
    bool DBBaseImp::Put(const string&key,const string &value,const string& db,const map<string,string> &msession)
    {
        if (!DBExist(db))
        {
            return false;
        }
        auto s = _db->Put(_WriteOptions, _mhandles[db], Slice(key), Slice(value));
        TLOGINFO_RAFT("Put status:"<<s.ToString()<<",key="<<key<<endl);
        if (s.ok()&& g_app.isLeader())
        {
            KVCmdContext::getInstance()->pusb_back(KVCmd(KV_PUT,key,value,db,msession));
        }
        

        return s.ok();

    }
    bool DBBaseImp::Delete(const string& key, const string& dbname,const map<string,string> &msession)
    {
        Status st = _db->Delete(_WriteOptions,_mhandles[dbname],key);
        if (st.ok()&& g_app.isLeader())
        {
            KVCmdContext::getInstance()->pusb_back(KVCmd(KV_DEL,key,"",dbname,msession));
        }
        return st.ok();

    }

}