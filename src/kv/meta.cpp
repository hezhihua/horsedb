#include "kv/meta.h"
#include "kv/kvcmd.h"
#include "util/tc_common.h"

namespace horsedb{

     Meta:: Meta(std::shared_ptr<DBBase> dbPtr)
    {
        _db=dbPtr;

        _sysDBName="sys";

    }

    Meta::~ Meta()
    {

    }


    bool Meta::checkDB(string &value,const string &dbName)
    {
        if (dbName.empty())
        {
            return false;
        }
        
        string dbkey=dbIDKey(dbName);//db2idPrefix+dbName;
       return  _db->Get(dbkey,value,_sysDBName);

    }
    bool Meta::checkTable(const string &tableName,string &value,const string &dbName)
    {
        //string dbID;
        // if (!checkDB(dbName,dbID))
        // {
        //     return false;
        // }
        if (dbName.empty()|| tableName.empty())
        {
            return false;
        }

        string tbkey=tableIDKey(tableName,dbName);//table2idPrefix+dbName+"_"+tableName;
        return _db->Get(tbkey,value,_sysDBName);
        
    }
    bool Meta::checkIndex(const string &tableName,const string &columnName,string &value,const string &dbName)
    {
        string indexkey=indexIDKey(tableName,columnName,dbName);//index2idPrefix+dbName+"_"+tableName+"_"+columnName;
        return _db->Get(indexkey,value,_sysDBName);
        
    }


    bool Meta::getDBID(string &dbID,const string &dbName)
    {
        if (checkDB(dbID,dbName))
        {
            return true;
        }
        

        string value;
        string db2idkey=dbIDKey(dbName);//db2idPrefix+dbName;

        std::lock_guard<std::mutex> guard(_mDBLock[dbName]);

        if (!_db->Get(dbSEQ,value,_sysDBName))//第一次不存在
        {
            dbID="1";
            return _db->Put(dbSEQ,dbID,_sysDBName) && _db->Put(db2idkey,dbID,_sysDBName);
        }
        else
        {
            uint64_t dbOID=TC_Common::strto<uint64_t>(value);
            dbID=TC_Common::tostr(dbOID+1);
            return _db->Put(dbSEQ,dbID,_sysDBName) && _db->Put(db2idkey,dbID,_sysDBName);
        }

    }

    string Meta::tableIDKey(const string &tableName,const string &dbName)
    {
        return table2idPrefix+dbName+"_"+tableName;

    }

    string Meta::indexIDKey(const string &tableName,const string &columnName,const string &dbName)
    {
        return index2idPrefix+dbName+"_"+tableName+"_"+columnName;

    }

    string Meta::dbIDKey(const string &dbName)
    {
        return db2idPrefix+dbName;

    }

    string Meta::tableMetaKey(const string &tableName,const string &dbName)
    {
        return metaPrefix+dbName+"_"+tableName;

    }


    bool Meta::getTbID( const string &tableName,string &tbID,const string &dbName)
    {       

        if (checkTable(tableName,tbID,dbName))
        {
            return true;
        }
        
        string value;
        string tbseqkey=tableSEQ;
        string tb2idkey=tableIDKey(tableName,dbName);//table2idPrefix+dbName+"_"+tableName;

        
        std::lock_guard<std::mutex> guard(_mTBLock[dbName+tableName]);
        
        if (!_db->Get(tbseqkey,value,_sysDBName))//第一次不存在
        {
            tbID="1";
            return _db->Put(tbseqkey,tbID,_sysDBName) && _db->Put(tb2idkey,tbID,_sysDBName);
        }
        else
        {
            uint64_t tbOID=TC_Common::strto<uint64_t>(value);
            tbID=TC_Common::tostr(tbOID+1);
            return _db->Put(tbseqkey,tbID,_sysDBName) && _db->Put(tb2idkey,tbID,_sysDBName);
        }
        
    }

    bool Meta::getRowID( const string &tableName,string &rowID,const string &dbName)
    {
        string value;
        string rowidkey=rowidPrefix+dbName+"_"+tableName;

        
        std::lock_guard<std::mutex> guard(_mTBLock[dbName+tableName]);

        if (!_db->Get(rowidkey,value,_sysDBName))//第一次不存在
        {
            rowID="1";
            return _db->Put(rowidkey,rowID,_sysDBName) ;
        }
        else
        {
            uint64_t OID=TC_Common::strto<uint64_t>(value);
            rowID=TC_Common::tostr(OID+1);
            return _db->Put(rowidkey,rowID,_sysDBName) ;
        }

    }

    bool Meta::getIndexID( const string &tableName,const string &columnName,string &indexID,const string &dbName)
    {
        string value;
        string indexseqkey=indexSeqPrefix+dbName+"_"+tableName;
        string index2idkey=indexIDKey(tableName,columnName,dbName);//index2idPrefix+dbName+"_"+tableName+"_"+columnName;

        if (checkIndex(tableName,columnName,indexID,dbName))
        {
            return true;
        }

       
        std::lock_guard<std::mutex> guard(_mTBLock[dbName+tableName]);

        if (!_db->Get(indexseqkey,value,_sysDBName))//第一次不存在
        {
            indexID="1";
            return _db->Put(indexseqkey,indexID,_sysDBName) && _db->Put(index2idkey,indexID,_sysDBName);
        }
        else
        {
            uint64_t indexOID=TC_Common::strto<uint64_t>(value);
            indexID=TC_Common::tostr(indexOID+1);
            return _db->Put(indexseqkey,indexID,_sysDBName) && _db->Put(index2idkey,indexID,_sysDBName);
        }
    }

    bool Meta::saveMeta( const string &tableName,string &value,const string &dbName)
    {
        string metaKey=tableMetaKey(tableName,dbName);//metaPrefix+dbName+"_"+tableName;
        return _db->Put(metaKey,value,dbName);//直接覆盖

    }

    bool Meta::getDBTables( vector<string> &vTable,const string &dbName)
    {
        string  sPreKey="m_"+dbName;
        vector<string> vValue,vKeys;
        _db->PreKeyGet(sPreKey,vValue,dbName,vKeys);
        for (size_t i = 0; i < vKeys.size(); i++)
        {
            vector<string> vTem=TC_Common::sepstr<string>(vKeys[i],"_");
            if (vTem.size()==3)
            {
                vTable.push_back(vTem[2]);
            }
            
        }
        return true;


    }



}
