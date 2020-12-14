#include "kv/dbimp.h"
#include "kv/kvcmd.h"
#include <iostream>

using namespace std;

namespace horsedb{

    bool DBBaseImp::Create(const string&dbname)
    {
        if (!DBExist(dbname))
        {
            ColumnFamilyHandle* cf;
            auto s = _db->CreateColumnFamily(ColumnFamilyOptions(), dbname, &cf);
            cout<<"CreateColumnFamily status:"<<s.ToString()<<endl;
            assert(s.ok());   

            _mhandles[dbname]= cf;
            if (s.ok())
            {
                KVCmdContext::getInstance()->pusb_back(KVCmd(KV_CREATE,"","",dbname));
            }
        }
        
         
        return true;

    }
    bool DBBaseImp::Put(const string&key,const string &value,const string& db)
    {
        if (!DBExist(db))
        {
            return false;
        }
        auto s = _db->Put(_WriteOptions, _mhandles[db], Slice(key), Slice(value));
        cout<<"Put status:"<<s.ToString()<<endl;
        if (s.ok())
        {
            KVCmdContext::getInstance()->pusb_back(KVCmd(KV_PUT,key,value,db));
        }
        

        return s.ok();

    }
    bool DBBaseImp::Delete(const string& key, const string& dbname)
    {
        Status st = _db->Delete(_WriteOptions,_mhandles[dbname],key);
        if (st.ok())
        {
            KVCmdContext::getInstance()->pusb_back(KVCmd(KV_DEL,key,"",dbname));
        }
        return st.ok();

    }

}