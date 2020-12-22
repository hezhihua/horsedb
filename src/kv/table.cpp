#include "kv/table.h"
#include "kv/kvcmd.h"
#include "util/tc_common.h"

#include<iostream>
#include<set>
#include<map>

using namespace std;

namespace horsedb {

    Table::Table(std::shared_ptr<Meta> metaPtr,std::shared_ptr<DBBase> dbPtr,string dbname)
    {
        _metaPtr=metaPtr;
        _dbPtr=dbPtr;
        _dbname=dbname;
    }


    int Table::initTable()
    {
        _dbname="";
        _name="";
        _vKey.clear();
        _vColumns.clear();
        return 0;
    }
    int Table::fillTable(const CreateStatement* createSt)
    {
        if (!createSt)
        {
            return -1;
        }

        initTable();
        set<string> setCloumn;

        _dbname=createSt->schema?createSt->schema:"";
        _name=createSt->tableName?createSt->tableName:"";
        if (createSt->columns)
        {
            bool hasPriveteKey=false;
            for (auto *column:*(createSt->columns))
            {
                if (column->name)
                {
                    cout<<"column="<<column->name<<endl;
                    horsedb::ColumnType tColumnType;
                    tColumnType.data_type=static_cast<horsedb::DataType> (column->type.data_type);
                    tColumnType.length=column->type.length;
                                   
                    if (column->iskey)
                    {
                        cout<<"key="<<column->name<<endl;
                        Key tKey(column->name,column->isuniquekey,column->isprivakey);
                        if (column->keyColumns)
                        {
                            for (auto *keycolunm:*(column->keyColumns))
                            {
                                cout<<"key's column="<<keycolunm<<endl;
                                tKey._vColumnName.push_back(keycolunm);
                            }
                        }
                        if (column->isprivakey)
                        {
                            hasPriveteKey=true;
                        }
                        
                        
                        _vKey.push_back(tKey);
                        
                    }
                    else
                    {
                        Column tColumn(column->name,tColumnType,column->nullable);
                        _vColumns.push_back(tColumn);
                        setCloumn.insert(column->name);

                    }
                }
                
                
            }

            if (!hasPriveteKey)
            {
                Key tKey("",false,true);
                tKey._vColumnName.push_back(defaultPriveteKeyName);

                _vKey.push_back(tKey);
                horsedb::ColumnType tColumnType;
                tColumnType.data_type=DataType ::INT;
                tColumnType.length=defaultPriveteKeyLength;
                Column tColumn(defaultPriveteKeyName,tColumnType,false);
                _vColumns.push_back(tColumn);
                setCloumn.insert(defaultPriveteKeyName);
            }
            

            for (size_t i = 0; i < _vKey.size(); i++)
            {
                for (size_t j = 0; j < _vKey[i]._vColumnName.size(); j++)
                {
                    if (setCloumn.find(_vKey[i]._vColumnName[j])==setCloumn.end())
                    {
                        cout<<"error,key="<<_vKey[i]._vColumnName[j]<<endl;
                        return -1;
                    }
                }
                                
            }
            
        }	
        return 0;			
    }


int Table::create(const CreateStatement* createSt,const string &dbname,const map<string,string> &mSession)
{
    int iRet=-1;
    do
    {
        if (dbname.empty())
        {
            cout<<"error,dbname empty"<<endl;
            break;
        }
        _dbname=dbname;
        
        if (createSt->type== hsql:: kCreateDatabase)
        {
            if (createSt->schema)
            {
                createDatabase(createSt,mSession);
            }
            
            
        }
        else if (createSt->type== hsql:: kCreateTable)
        {
            createTable(createSt,dbname,mSession);
        }

        iRet=0;
        
        
    } while (0);
    return iRet;
}
int Table::createDatabase(const CreateStatement* createSt,const map<string,string> &mSession)
{
    int iRet=-1;
    do
    {

        if (createSt->schema)
        {
            _dbname=createSt->schema;
            _dbPtr->Create(createSt->schema,mSession);
        }
        
        iRet=0;
        
        
    } while (0);

    return iRet;
    
    
}

int Table::getDBTables(vector<string> &vTable,const string &dbname)
{
    
    _metaPtr->getDBTables(vTable,dbname);

    return 0;

}

    int Table::createTable(const CreateStatement* createSt,const string &dbname,const map<string,string> &mSession)
    {
        int iRet=-1;
        do
        {
            iRet=fillTable(createSt);
            if (iRet<0)
            {
                cout<<"error,fillTable"<<endl;
                break;
            }
            
            
            _dbname=dbname;
            
            
            cout<<"_dbname="<<_dbname<<endl;
            string dbID,tbID,indexID;
            if (!_metaPtr->getDBID(dbID,dbname,mSession))
            {
                cout<<"error,getDBID"<<endl;
                break;
                
            }

            if (!_metaPtr->getTbID(_name,tbID,dbname,mSession))
            {
                cout<<"error,getTbID"<<endl;
                break;
            }
            
            for (size_t i = 0; i < _vKey.size(); i++)
            {
                if (_vKey[i]._vColumnName.size())
                {
                    if (!_metaPtr->getIndexID(_name,_vKey[i]._vColumnName[0],indexID,dbname,mSession))//暂时只能建立单字段索引 
                    {
                        cout<<"error,getIndexID"<<endl;
                        break;
                    }
                    
                }
                
            }

            iguana::string_stream ss;
            iguana::json::to_json(ss, *this);//*this
            auto json_str = ss.str();
            cout<<"json_str="<<json_str<<endl;
            _metaPtr->saveMeta(_name,json_str,dbname,mSession);
            iRet=0;
        } while (0);
        

        
        return iRet;
        	

    }

    int Table::parseInsert(const InsertStatement* insertSt,vector<string> &vCloumnName, vector<string> &vData, string &tbname)
    {
        
        if (insertSt==nullptr)
        {
            return -1;
        }
        if (insertSt->tableName==nullptr)
        {
            return -1;
        }

        tbname=insertSt->tableName;
        
        if (insertSt->columns)
        {
            for (char* col_name : *insertSt->columns) 
            {
                vCloumnName.push_back(col_name);

            }
        }
        if (insertSt->type==hsql::kInsertValues && insertSt->values)
        {
            for (Expr* expr : *insertSt->values) 
            {
                switch (expr->type)
                {
                case kExprLiteralFloat:
                    vData.push_back(TC_Common::tostr(expr->fval));
                    break;
                case kExprLiteralInt:
                    vData.push_back(TC_Common::tostr(expr->ival));
                    break;
                case kExprLiteralString:
                    vData.push_back(TC_Common::tostr(expr->name));
                    break;
                case kExprLiteralNull:
                    vData.push_back("");
                    break;

                
                default:
                    break;
                }

            }   
    
        }
        if (vCloumnName.size()!=vData.size())
        {
            cout<<"error,vCloumnName.size()!=vData.size()"<<endl;
            return -1;
        }
        

        return 0;

    }


    int Table::insertTable(const InsertStatement* insertSt,const string &dbname,const map<string,string> &mSession)
    {
        int iRet=-1;
        do
        {
            if (dbname.empty())
            {
                cout<<"error,dbname empty"<<endl;
                break;
            }

            _dbname=dbname;
            
                   
            vector<string> vCloumnName;
            vector<string> vData;
            string tbname;

            iRet=parseInsert(insertSt,vCloumnName,vData,tbname);
            if (iRet<0)
            {
                cout<<"error,parseInsert"<<endl;
                break;
            }

            map<string,string> mRow;
            
            
            if (dbname.empty())
            {
                cout<<"dbname empty"<<endl;
                break;
            }
            if (!_dbPtr->DBExist(dbname))
            {
                cout<<"dbname NOT Exist:"<<dbname<<endl;
                break;
            }          
            
            string tableMetaKey=_metaPtr->tableMetaKey(tbname,dbname);
            string sValue;
            _dbPtr->Get(tableMetaKey,sValue,dbname);
            if (sValue.empty())
            {
                cout<<"error,table meta info empty"<<endl;
                break;
            }
            Table tTable;
            iguana::json::from_json(tTable, sValue.data(),sValue.length());

            auto& vMetaColumns=tTable._vColumns;

            string sPrivateKey;
            getPrivateKeyName(tTable,sPrivateKey);
            if (sPrivateKey.empty())
            {
                cout<<"error,sPrivateKey empty"<<endl;
                break;
            }
            
            for (size_t i = 0,j=0; i < vCloumnName.size() && j<vData.size(); i++,j++)
            {
                
                mRow[vCloumnName[i]]=vData[j];

            }

            vector<string> vRow;
            for (size_t i = 0; i < vMetaColumns.size(); i++)
            {
                if (vMetaColumns[i]._name!=sPrivateKey)
                {
                    vRow.push_back(mRow[vMetaColumns[i]._name]);
                }
                
            }

            iguana::string_stream ss;
            iguana::json::to_json(ss, vRow);
            auto json_str = ss.str();
            cout<<"json_str="<<json_str<<endl;

            //insert row
            string sRowID;
            string sKey=insertTbKey(tbname,sRowID,mSession);
            if (sKey.empty())
            {
                cout<<"sKey empty"<<endl;
                break;
            }
            
            if (!_dbPtr->Put(sKey,json_str,dbname,mSession))
            {
               cout<<"error ,db Put ,key="<<sKey<<endl;
                break;
            }
            //insert index row
            for (size_t i = 0; i < tTable._vKey.size(); i++)
            {
                auto &tKey=tTable._vKey[i];
                if (tKey._isPrivateKey)
                {
                    /* do nothing */
                }
                else if (tKey._isUniqueKey)
                {
                    for (size_t j = 0; j < tKey._vColumnName.size(); j++)
                    {
                        string indexUKey=IndexUniqueKey(tbname,tKey._vColumnName[j],mRow[tKey._vColumnName[j]]);
                        if (indexUKey.empty())
                        {
                            cout<<"indexUKey empty"<<endl;
                            break;
                        }
                        if (!_dbPtr->Put(indexUKey,sRowID,dbname,mSession))
                        {
                            cout<<"error ,db Put key= "<<indexUKey<<endl;
                            break;
                        }

                    }
                    
                }
                else
                {
                    for (size_t j = 0; j < tKey._vColumnName.size(); j++)
                    {
                        string indexNUKey=IndexNotUniqueKey(tbname,tKey._vColumnName[j],mRow[tKey._vColumnName[j]],sRowID);
                        if (indexNUKey.empty())
                        {
                            cout<<"indexNUKey empty"<<endl;
                            break;
                        }
                        if (!_dbPtr->Put(indexNUKey,"",dbname,mSession))
                        {
                            cout<<"error ,db Put key= "<<sKey<<endl;
                            break;
                        }

                    }
                }
                
            }
            
            iRet=0;
            
        } while (0);

        return iRet;
        
    }

    //非Unique Index
    //Key: tablePrefix{tableID}indexPrefixSep{indexID}_indexedColumnsValue_rowID
    //Value: null
    // t10_i1_10_1 --> null
    // t10_i1_20_2 --> null
    // t10_i1_30_3 --> null
    string Table::IndexNotUniqueKey(const string &tbname,const string &columnName,const string &columnValue,const string &sRowID)
    {
        string sTbID,indexID,rowID;
        _metaPtr->getTbID(tbname,sTbID,_dbname);
        if (sTbID.empty())
        {
            return "";
        }
        _metaPtr->getIndexID(tbname,columnName,indexID,_dbname);
        if (indexID.empty())
        {
            return "";
        }


        return  tablePrefix+sTbID+indexPrefixSep+indexID+"_"+columnValue+"_"+sRowID ;

    }

        //Unique Index
      //Key: tablePrefix{tableID}indexPrefixSep{indexID}_indexedColumnsValue
      //Value: rowID
    string Table::IndexUniqueKey(const string &tbname,const string &columnName,const string &columnValue)
    {
        string sTbID,indexID,rowID;
        _metaPtr->getTbID(tbname,sTbID,_dbname);
        if (sTbID.empty())
        {
            return "";
        }
        _metaPtr->getIndexID(tbname,columnName,indexID,_dbname);
        if (indexID.empty())
        {
            return "";
        }

        return tablePrefix+sTbID+indexPrefixSep+indexID+"_"+columnValue;

    }





    //每行数据按照如下规则进行编码成 Key-Value  
    //Key: tablePrefix{tableID}_recordPrefixSep{rowID}
    //Value: [col1, col2, col3]
    // t10_r1 --> ["HorseDB", "SQL Layer", 10]
    // t10_r2 --> ["HorseDB", "KV Engine", 20]
    // t10_r3 --> ["HorseDB", "Manager", 30]
    string Table::insertTbKey(const string &tbname,string &sRowID,const map<string,string> &mSession)
    {

        string sTbID;
        _metaPtr->getTbID(tbname,sTbID,_dbname);
        if (sTbID.empty())
        {
            return "";
        }

        _metaPtr->getRowID(tbname,sRowID,_dbname,mSession);
        if (sRowID.empty())
        {
            return "";
        }

        //TC_Common::encodeID32(TC_Common::strto<uint32_t>(sRowID),sRowID);

        return tablePrefix+sTbID+recordPrefixSep+sRowID;    
    }

    //t10_r1
    string Table::getPreRowKeyByTb(const string &tbname)
    {
        string sTbID,sRowID;
        _metaPtr->getTbID(tbname,sTbID,_dbname);
        if (sTbID.empty())
        {
            return "";
        }

        return tablePrefix+sTbID+recordPrefixSep;
    }

    bool Table::isIndexColumn(const string& sColumnName,Table &tb,bool &bUnique,bool &bPrivate)
    {
        for (size_t i = 0; i < tb._vKey.size(); i++)
        {
            auto &tKey=tb._vKey[i];
            if (std::find(tKey._vColumnName.begin(),tKey._vColumnName.end(),sColumnName)!=tKey._vColumnName.end())
            {
                if ( tKey._isUniqueKey)
                {
                    bUnique=true;
                }else if (tKey._isPrivateKey)
                {
                    bPrivate=true;
                }
                
                
                return true;
            }
            
        }
        return false;     

    }

    bool Table::getPrivateKeyName(const Table &tb,string& sPrivateColumnName)
    {
        for (size_t i = 0; i < tb._vKey.size(); i++)
        {
            auto &tKey=tb._vKey[i];
            if (tKey._isPrivateKey && tKey._vColumnName.size())
            {
                sPrivateColumnName=tKey._vColumnName[0];
            }
            
            
        }
        return !sPrivateColumnName.empty();     

    }

    void Table::str2Row(const vector<string> &vStrRow,const Table &tTable,vector<map<string,string>> &vRowData,const vector<string> &vResultKey,const map<string,string> &mCondition)
    {
        auto& vMetaColumns=tTable._vColumns;
        string sPriveKeyColumnName;
        getPrivateKeyName(tTable,sPriveKeyColumnName);
        if (sPriveKeyColumnName.empty())
        {
            cout<<"error,sPriveKeyColumnName.empty"<<endl;
        }
        

        for (size_t i = 0,k=0; i < vStrRow.size() && k < vResultKey.size(); i++,k++)
        {
            vector<string> tRow;
            auto &sValue=vStrRow[i];
            iguana::json::from_json(tRow, sValue.data(),sValue.length());
            map<string,string> mRow;
            for (size_t i = 0,j=0; i < vMetaColumns.size() && j<tRow.size(); i++)
            {
                if (vMetaColumns[i]._name==sPriveKeyColumnName)
                {
                    /* code */
                }
                else
                {
                    mRow[vMetaColumns[i]._name]=tRow[j];
                    j++;
                }
  
            }

            auto vTemp=TC_Common::sepstr<string>(vResultKey[k],"_");
            if (vTemp.size())
            {
                string sRowID=vTemp[vTemp.size()-1];
                if (sRowID.length())
                {
                    sRowID=sRowID.substr(1);
                    mRow[sPriveKeyColumnName]=sRowID;
                    //cout<<"sRowID="<<sRowID<<endl;
                }

            }
            //过滤条件 
            bool isEq=true;
            for (auto it=mCondition.begin();it!=mCondition.end();it++)
            {
                if (mRow[it->first]!=it->second)
                {
                    isEq=false;
                }
                
            }

            if (isEq)
            {
                vRowData.push_back(mRow);
            }
        }
    }

    void Table::indexRowKey2RowKey(const string &tbname,const vector<string> &vIndexRowKey,vector<string> &vRowKey)
    {
        string sPreRowKey=getPreRowKeyByTb(tbname);
        for (size_t i = 0; i < vIndexRowKey.size(); i++)
        {
             auto vTemp=TC_Common::sepstr<string>(vIndexRowKey[i],"_");
             if (vTemp.size())
             {
                 string sRowID=vTemp[vTemp.size()-1];
                 cout<<" rowkey===="<<sPreRowKey+sRowID<<endl;
                 vRowKey.push_back(sPreRowKey+sRowID);
             }
             
        }
    }

    bool Table::getMetaTable(const string &tbname,const string &dbname,Table &tTable)
    {
        string tableMetaKey=_metaPtr->tableMetaKey(tbname,dbname);
        string sValue;
        _dbPtr->Get(tableMetaKey,sValue,dbname);
        if (sValue.empty())
        {
            cout<<"error,table meta info empty"<<endl;
            return false;
        }

        iguana::json::from_json(tTable, sValue.data(),sValue.length());
        return true;

    }


    int Table::selectTable(const SelectStatement* selectSt,vector<string> &vCloumnName, vector<map<string,string>> &vRowData, const string &dbname)
    {
        int iRet=-1;
        do
        {
            if (dbname.empty())
            {
                cout<<"error,dbname empty"<<endl;
                break;
            }
            _dbname=dbname;

            if (selectSt==nullptr||selectSt->selectList==nullptr)
            {
                break;
            }

            if (selectSt->fromTable == nullptr||selectSt->fromTable->name == nullptr)
            {
                break;
            }

            string tbname(selectSt->fromTable->name);

            
            Table tTable;
            if (!getMetaTable(tbname,dbname,tTable))
            {
                break;
            }
            
            
            auto& vMetaColumns=tTable._vColumns;

            for (Expr* expr : *selectSt->selectList)
            {
                switch (expr->type)
                {
                case kExprStar:
                    {
                        
                        for (size_t i = 0; i < vMetaColumns.size(); i++)
                        {
                            vCloumnName.push_back(vMetaColumns[i]._name);
                        }

                    }
                    break;
                case kExprColumnRef:
                {
                    vCloumnName.push_back(expr->name);

                }
                break;
                
                default:
                    break;
                }

            }

            //where
            map<string,string> mCondition;
            if (selectSt->whereClause)
            {
                if (selectSt->whereClause->type==kExprOperator )
                {
                    if (selectSt->whereClause->opType==hsql::kOpEquals)
                    {
                        string conditionName,conditionValue;
                        if (selectSt->whereClause->expr && selectSt->whereClause->expr->name)
                        {
                            conditionName=selectSt->whereClause->expr->name;
                        }
                        if (selectSt->whereClause->expr2 && selectSt->whereClause->expr2->name)
                        {
                            conditionValue=selectSt->whereClause->expr2->name;
                        }

                        if (conditionName.empty() || conditionValue.empty())
                        {
                            cout<<"error,condition empty"<<endl;
                            break;
                        }
                        mCondition[conditionName]=conditionValue;
                        bool bUnique=false, bPrivate=false;
                        if (isIndexColumn(conditionName,tTable,bUnique,bPrivate))
                        {
                            string sPreKey;
                            vector<string> vValueRow,vKey,vResultKey;
                            if (bPrivate)
                            {
                                string sRowData;
                                string sLastKey=getPreRowKeyByTb(tbname)+conditionValue;
                                _dbPtr->Get(sLastKey,sRowData,dbname);

                                vValueRow.push_back(sRowData);
                                vResultKey.push_back(sLastKey);
                                
                            }else if (bUnique)
                            {
                                string sRowData,sRowID;
                                string sKey= IndexUniqueKey(tbname,conditionName,conditionValue);
                                _dbPtr->Get(sKey,sRowID,dbname);
                                string sLastKey=getPreRowKeyByTb(tbname)+sRowID;
                                _dbPtr->Get(sLastKey,sRowData,dbname);

                                vValueRow.push_back(sRowData);
                                vResultKey.push_back(sLastKey);
                            }else                            
                            {
                                string sPreKey= IndexNotUniqueKey(tbname,conditionName,conditionValue,"");
                                _dbPtr->PreKeyGet(sPreKey,vValueRow,dbname,vKey);
                                vValueRow.clear();

                                indexRowKey2RowKey(tbname,vKey,vResultKey);
                                for (size_t i = 0; i < vResultKey.size(); i++)
                                {
                                    string sRowData;
                                    _dbPtr->Get(vResultKey[i],sRowData,dbname);
                                    vValueRow.push_back(sRowData);

                                }

                            }
                            
                            str2Row(vValueRow,tTable,vRowData,vResultKey);

                            iRet=0;
                            break;
                        }
                    }
                                      
                }

                
                 
            }
            
            //no where or where without index condition
            string preRowKey=getPreRowKeyByTb(tbname);
            if (preRowKey.empty())
            {
                break;
            }

            vector<string> vValueRow;
            vector<string> vKey;
            _dbPtr->PreKeyGet(preRowKey,vValueRow,dbname,vKey);
            str2Row(vValueRow,tTable,vRowData,vKey,mCondition);
            
            iRet=0;
    
        } while (0);

        return iRet;
        
        

    }
}