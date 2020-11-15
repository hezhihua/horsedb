#ifndef __TABLE__
#define __TABLE__
#include <string>
#include <vector>
#include <memory>
#include "SQLParser.h"
#include "util/sqlhelper.h"

#include "kv/meta.h"
#include "iguana/json.hpp"

using namespace std;
using namespace hsql;
 
#define  tablePrefix "t"
#define  recordPrefixSep  "_r"
#define  indexPrefixSep  "_i"


#define defaultPriveteKeyName "id"
#define defaultPriveteKeyLength 11

namespace horsedb{

      enum class DataType {
      UNKNOWN,
      INT,
      LONG,
      FLOAT,
      DOUBLE,
      CHAR,
      VARCHAR,
      TEXT,
      DATETIME
    };

    // Represents the type of a column, e.g., FLOAT or VARCHAR(10)
    struct ColumnType {
      ColumnType() = default;
      ColumnType(DataType data_type, int64_t length = 0);
      DataType data_type;
      int64_t length;  // Used for, e.g., VARCHAR(10)
    };
    //REFLECTION(ColumnType, data_type,length);


    struct Column
    {
      Column() = default;
      Column(string name,horsedb::ColumnType type,bool nullable):_name(name),_data_type(type.data_type),_length(type.length),_nullable(nullable){}
        string _name;
        //horsedb::ColumnType _type;
        DataType _data_type;
        int64_t _length;  // Used for, e.g., VARCHAR(10)

        bool _nullable;

    };
    REFLECTION(Column, _name,_data_type,_length,_nullable);

    struct Key
    {
      Key() = default;
      Key(string name,bool  isUniqueKey,bool  isPrivateKey):_name(name),_isUniqueKey(isUniqueKey),_isPrivateKey(isPrivateKey){}
        string _name;
        bool  _isUniqueKey;
        bool  _isPrivateKey;
        vector<string> _vColumnName;
    };
    REFLECTION(Key, _name,_isUniqueKey,_isPrivateKey,_vColumnName);

    struct Row
    {
      Row() = default;
      Row(const vector<string> &vData):_vData(vData){};
      vector<string> _vData;

    };
    REFLECTION(Row, _vData);

    class Table
    {
      public:


    // CREATE TABLE User {
    // 	ID int,
    // 	Name varchar(20),
    // 	Role varchar(20),
    // 	Age int,
    // 	PRIMARY KEY (ID),
    // 	Key idxAge (age) //目前只支持单列索引，不支持联合索引
    // };



      //每行数据按照如下规则进行编码成 Key-Value  
      //Key: tablePrefix{tableID}_recordPrefixSep{rowID}
      //Value: [col1, col2, col3]
    // t10_r1 --> ["HorseDB", "SQL Layer", 10]
    // t10_r2 --> ["HorseDB", "KV Engine", 20]
    // t10_r3 --> ["HorseDB", "Manager", 30]

      //Unique Index
      //Key: tablePrefix{tableID}_indexPrefixSep{indexID}_indexedColumnsValue
      //Value: rowID


      //非Unique Index
      //Key: tablePrefix{tableID}_indexPrefixSep{indexID}_indexedColumnsValue_rowID
      //Value: null
    // t10_i1_10_1 --> null
    // t10_i1_20_2 --> null
    // t10_i1_30_3 --> null
        Table(){};
        Table(std::shared_ptr<Meta> metaPtr,std::shared_ptr<DBBase> dbPtr,string dbname="");
        ~ Table(){};
        

        int initTable();
        int fillTable(const CreateStatement* createSt);
        int createTable(const CreateStatement* createSt,const string &dbname);
        int parseInsert(const InsertStatement* insertSt,vector<string> &vColumnName, vector<string> &vData, string &tbname);
        int insertTable(const InsertStatement* insertSt,const string &dbname);
        string insertTbKey(const string &tbname,string &sRowID);
        int selectTable(const SelectStatement* insertSt,vector<string> &vColumnName, vector<map<string,string>> &mData, const string &dbname);
        string getPreRowKeyByTb(const string &tbname);
        string IndexNotUniqueKey(const string &tbname,const string &columnName,const string &columnValue,const string &sRowID);
        string IndexUniqueKey(const string &tbname,const string &columnName,const string &columnValue);
        bool isIndexColumn(const string& sColumnName,Table &tb,bool &bUnique,bool &bPrivate);
        void  str2Row(const vector<string> &vStrRow,const Table &tTable,vector<map<string,string>> &vRowData,const vector<string> &vResultKey,
                      const map<string,string> &mCondition=map<string,string>());
        void indexRowKey2RowKey(const string &tbname,const vector<string> &vIndexRowKey,vector<string> &vRowKey);

        int createDatabase(const CreateStatement* createSt);
        int create(const CreateStatement* createSt,const string &dbname);
        int getDBTables(vector<string> &vTable,const string &dbname);
        bool getPrivateKeyName(const Table &tb,string& sPrivateColumnName);
        bool getMetaTable(const string &tbname,const string &dbname,Table &tTable);

        

        string _dbname;
        string _name;
        
        vector<Column> _vColumns;
        vector<Key> _vKey;

        std::shared_ptr<Meta> _metaPtr;
        std::shared_ptr<DBBase> _dbPtr;
    };
    REFLECTION(Table, _dbname,_name,_vColumns,_vKey);

}



#endif
