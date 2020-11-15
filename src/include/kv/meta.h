#ifndef __META__
#define __META__
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

#include "kv/DBBase.h"

using namespace std;

//存表的元信息
//Key: metaPrefix{dbname}_{tablename}
//Value:
#define metaPrefix  "m_" 


// //存数据库id
// //key :dbPrefix{dbID}
// //Value:null
// #define dbPrefix  "db_" 

// //存表id
// //key :tablePrefix{dbID}_{tableID}
// //Value:null
// #define tablePrefix  "table_" 


//存库和id映射关系
//key :db2idPrefix{dbname}
//Value:id
#define db2idPrefix  "db2id_" 

//存表和id映射关系
//key :table2idPrefix{dbname}_{tablename}
//Value:id
#define table2idPrefix  "tb2id_" 


//存表的索引和id映射关系 表内唯一
//key :index2idPrefix{dbname}_{tablename}_{columnname}
//Value:id
#define index2idPrefix  "index2id_" 

////////////////////////////////////////////////
//全局-库id ,存 目前 最大id
//key :dbSeq
//Value:id
#define dbSEQ  "seq_db" 

//全局-表id,存 目前 最大id
//key :tableSeqPrefix
//Value:id
#define tableSEQ  "seq_table" 

//表的indexid,表内唯一,存 目前 最大id
//key :indexSeqPrefix{dbname}_{tablename}
//Value:id
#define indexSeqPrefix  "seq_index_" 

//表的rowid,表内唯一,存 目前 最大id
//key :rowidPrefix{dbname}_{tablename}
//Value:id
#define rowidPrefix  "rowid_" 

namespace horsedb {
class  Meta
{
    private:
        /* data */
        std::shared_ptr<DBBase> _db;
        map<string,std::mutex> _mTBLock;
        map<string,std::mutex> _mDBLock;

        string _sysDBName;

    public:
        bool checkDB(string &value,const string &dbName);
        bool checkTable(const string &tableName,string &value,const string &dbName);
        bool checkIndex(const string &tableName,const string &columnName,string &value,const string &dbName);

        bool getDBID(string &dbID,const string &dbName);
        bool getTbID(const string &tableName,string &tbID,const string &dbName);
        bool getRowID(const string &tableName,string &rowID,const string &dbName);
        bool getIndexID(const string &tableName,const string &columnName,string &indexID,const string &dbName);

        string tableIDKey(const string &tableName,const string &dbName);
        string indexIDKey(const string &tableName,const string &columnName,const string &dbName);
        string dbIDKey(const string &dbName);

        string tableMetaKey(const string &tableName,const string &dbName);
        bool   getDBTables( vector<string> &vTable,const string &dbName);
        bool saveMeta(const string &tableName,string &value,const string &dbName);


        Meta(std::shared_ptr<DBBase> dbPtr);
        ~ Meta();
};


}




#endif