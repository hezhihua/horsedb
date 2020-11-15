#include "mysql/command.h"
#include  "mysql/mysql_packet.h"



namespace horsedb{


    void Command::adminSelectVersionComment(SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle)
    {
        vector<st_mysql_field> vField;
        vector<string> vRow;
        st_mysql_field field;
        string sDest;
        field.name="@@version_comment";
        field.type = MYSQL_TYPE_VAR_STRING;
        vField.push_back(field);
        vRow.push_back("horseDB");

        MysqlPacket::appendResultSet(socketContext,sDest,vField,vRow);
        vector<char> sendBuff;
        sendBuff.assign(sDest.begin(), sDest.end());

        data->buffer()->setBuffer(sendBuff);
        handle->sendResponse(data);
        

    }
    void  Command::adminShowDatabases(SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,const vector<string> &vDatabase,TC_EpollServer::Handle *handle)
    {
        vector<st_mysql_field> vField;
        vector<string> vRow;
        st_mysql_field field;
        string sDest;
        field.name="Database";
        field.type = MYSQL_TYPE_VAR_STRING;
        vField.push_back(field);
        for (size_t i = 0; i < vDatabase.size(); i++)
        {
            vRow.push_back(vDatabase[i]);
        }

        MysqlPacket::appendResultSet(socketContext,sDest,vField,vRow);
        vector<char> sendBuff;
        sendBuff.assign(sDest.begin(), sDest.end());

        data->buffer()->setBuffer(sendBuff);
        handle->sendResponse(data);

    }
    void  Command::adminShowTables(SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,const vector<string> &vTable,TC_EpollServer::Handle *handle)
    {
        vector<st_mysql_field> vField;
        vector<string> vRow;
        st_mysql_field field;
        string sDest;
        field.name="Tables";//Tables_in_test
        field.type = MYSQL_TYPE_VAR_STRING;
        vField.push_back(field);
        for (size_t i = 0; i < vTable.size(); i++)
        {
            vRow.push_back(vTable[i]);
        }
        
        


        MysqlPacket::appendResultSet(socketContext,sDest,vField,vRow);
        vector<char> sendBuff;
        sendBuff.assign(sDest.begin(), sDest.end());

        data->buffer()->setBuffer(sendBuff);
        handle->sendResponse(data);

    }
    void  Command::adminShowFields(SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle)
    {
        vector<st_mysql_field> vField;
        
        st_mysql_field field;
        string sDest;
        field.name="Field";
        field.type = MYSQL_TYPE_VAR_STRING;
        vField.push_back(field);
        field.name="Type";
        field.type = MYSQL_TYPE_VAR_STRING;
        vField.push_back(field);
        field.name="Null";
        field.type = MYSQL_TYPE_VAR_STRING;
        vField.push_back(field);
        field.name="Key";
        field.type = MYSQL_TYPE_VAR_STRING;
        vField.push_back(field);
        field.name="Default";
        field.type = MYSQL_TYPE_VAR_STRING;
        vField.push_back(field);
        field.name="Extra";
        field.type = MYSQL_TYPE_VAR_STRING;
        vField.push_back(field);

        vector<string> vCloumnName={"Field","Type","Null","Key","Default","Extra"};


        map<string ,string> mRow;
        mRow["Field"]="id";
        mRow["Type"]="int(11)";
        mRow["Null"]="NO";
        mRow["Key"]="PRI";
        mRow["Default"]="NULL";
        mRow["Extra"]="auto_increment";

        vector<map<string,string>>  vRow={mRow};

        MysqlPacket::appendRowResultSet(socketContext,sDest,vField,vCloumnName,vRow);

        vector<char> sendBuff;
        sendBuff.assign(sDest.begin(), sDest.end());
        data->buffer()->setBuffer(sendBuff);
        handle->sendResponse(data);

    }

    void  Command::selectRowData(SocketContext &socketContext,const shared_ptr<TC_EpollServer::SendContext> &data,TC_EpollServer::Handle *handle,vector<string> &vCloumnName, vector<map<string,string>> &vRowData)
    {
        vector<st_mysql_field> vField;
        string sDest;

        for (size_t i = 0; i < vCloumnName.size(); i++)
        {
            st_mysql_field field;
            field.name=vCloumnName[i];
            field.type = MYSQL_TYPE_VAR_STRING;
            vField.push_back(field);
        }

        

        MysqlPacket::appendRowResultSet(socketContext,sDest,vField,vCloumnName,vRowData);

        vector<char> sendBuff;
        sendBuff.assign(sDest.begin(), sDest.end());
        data->buffer()->setBuffer(sendBuff);
        handle->sendResponse(data);

    }
}