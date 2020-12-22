#ifndef __HORSE_DB_IMP__
#define __HORSE_DB_IMP__
#include "kv/DBBase.h"
namespace horsedb{

    class DBBaseImp:public DBBase
    {
        bool Create(const string&dbname,const map<string,string> &msession=map<string,string>());
        bool Put(const string&key,const string &value,const string& dbname,const map<string,string> &msession=map<string,string>());
        bool Delete(const string& key, const string& dbname,const map<string,string> &msession=map<string,string>());

    };


}


#endif