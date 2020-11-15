#include "mysql/mysql_packet.h"


namespace horsedb{

    int MysqlPacket::appendIntLen(string  &desPacket, uint64_t src, size_t byteSize) 
    {
        for (size_t i = 0; i < byteSize; i++) 
        {
            desPacket.append(1, src & 0xff);
            src >>= 8;
        }

        return 0;
    }

    int MysqlPacket:: packetAuthReq(string  &desPacket, const AuthRequest &authRequest)
    {
        appendIntLen(desPacket, 0x0a,1);//协议版本号
        if (!authRequest._serverVersionName.empty()) 
        {
            desPacket.append(authRequest._serverVersionName);
        } 
        else if (authRequest._serverVersion > 30000 && authRequest._serverVersion < 100000) 
        {
            char ver[50]={0};
            sprintf(ver, "%d.%02d.%02d", authRequest._serverVersion / 10000,
                (authRequest._serverVersion % 10000) / 100, authRequest._serverVersion % 100);
            desPacket.append(string(ver));
        } 
        else 
        {
            desPacket.append("5.0.99");
        }

        appendIntLen(desPacket, 0x00,1);//todo
        appendIntLen(desPacket, authRequest._threadID,4);
        if (authRequest._authPluginData.size())
        {
            assert(authRequest._authPluginData.size()>=8);
            desPacket.append(authRequest._authPluginData.data(),8);
        } 
        else
        {
            desPacket.append("01234567");
        }

        appendIntLen(desPacket, 0x00,1); /* filler */
        appendIntLen(desPacket, authRequest._capabilities & 0xffff,2);
        appendIntLen(desPacket, authRequest._charset,1);
        appendIntLen(desPacket,  authRequest._serverStatus,2);
        appendIntLen(desPacket,  (authRequest._capabilities >> 16) & 0xffff,2);

        if (authRequest._capabilities & CLIENT_PLUGIN_AUTH) 
        {
            assert(authRequest._authPluginData.size()<255);
            cout<<"authRequest._authPluginData.size()="<<authRequest._authPluginData.size()<<endl;
            appendIntLen(desPacket, authRequest._authPluginData.size(),1);
        } 
        else 
        {
            appendIntLen(desPacket, 0x00,1); 
        }

        /* add the fillers */
        for (int i = 0; i < 10; i++) 
        {
            appendIntLen(desPacket, 0x00,1); 
        }

        if (authRequest._capabilities & CLIENT_PLUGIN_AUTH) 
        {
            assert(authRequest._authPluginData.size()>=8);
            desPacket.append(authRequest._authPluginData.data()+8,authRequest._authPluginData.size()-8);
            desPacket.append(authRequest._authPluginName.data(),authRequest._authPluginName.length());
            appendIntLen(desPacket, 0x00,1); 
        } 
        else
        {
            /* if we only have SECURE_CONNECTION it is 0-terminated */
            if (authRequest._authPluginData.size()) 
            {
                assert(authRequest._authPluginData.size()>=8);
                desPacket.append(authRequest._authPluginData.data()+8,authRequest._authPluginData.size()-8);
            }
            else 
            {
                desPacket.append("890123456789");
            }
             appendIntLen(desPacket, 0x00,1); 
        }

        return 0;
    }

    int MysqlPacket:: packetHeader(string  &header, uint8_t packetID,size_t bodylen)
    {
        appendIntLen(header, bodylen,3); 
        appendIntLen(header, packetID,1); 
        return 0;
    }

    int  MysqlPacket:: getHeaderPacketID(const string  &buff)
    {
        if (buff.size()<4)
        {
            return -1;
        }
        uint8_t packetID=buff[3] & 0xff;
        printf("packetID=%02x\n",packetID);
        return packetID;
    }

    void  MysqlPacket::skipHeader(string  &buff)
    {
        buff=buff.substr(MYSQL_HEADER_SIZE);
    }

    int   MysqlPacket::skipLen(string  &buff,size_t len)
    {
        if(len>buff.size()){return -1;}
        buff=buff.substr(len);
        return 0;
    }

    int MysqlPacket::unpacket2AuthRes( string  &srcPacket,  AuthResponse &authResponse)
    {
        int err = 0;
        uint16_t l_cap=0;
        /* extract the default db from it */

        /*
        * @\0\0\1
        *  \215\246\3\0 - client-flags
        *  \0\0\0\1     - max-packet-len
        *  \10          - charset-num
        *  \0\0\0\0
        *  \0\0\0\0
        *  \0\0\0\0
        *  \0\0\0\0
        *  \0\0\0\0
        *  \0\0\0       - fillers
        *  root\0       - username
        *  \24          - len of the scrambled buf
        *    ~    \272 \361 \346
        *    \211 \353 D    \351
        *    \24  \243 \223 \257
        *    \0   ^    \n   \254
        *    t    \347 \365 \244
        *  
        *  world\0
        */

        /* 4.0 uses 2 byte, 4.1+ uses 4 bytes, but the proto-flag is in the lower 2 bytes */
        err = err || peekInt16(srcPacket, &l_cap);
        if (err){return -1;}
        if (l_cap & CLIENT_PROTOCOL_41) 
        {
            err = err || getInt32(srcPacket, &authResponse._clientCapabilities);
            err = err || getInt32(srcPacket, &authResponse._maxPacketSize);
            err = err || getInt8(srcPacket, &authResponse._charset);

            err = err || skipLen(srcPacket, 23);
            if (err == 0 && (authResponse._clientCapabilities & CLIENT_SSL) && srcPacket.empty())
            {
                authResponse._sslRequest = true;
                return 0;
            }

            err = err || getStr(srcPacket,authResponse._userName);

            uint8_t len;
            /* new auth is 1-byte-len + data */
            err = err || getInt8(srcPacket, &len);
            getLenStr(srcPacket,len,authResponse._authPluginData);

            if ((authResponse._serverCapabilities & CLIENT_CONNECT_WITH_DB) &&
            (authResponse._clientCapabilities& CLIENT_CONNECT_WITH_DB)) 
            {
                err = err || getStr(srcPacket, authResponse._database);
            }

            if ((authResponse._serverCapabilities & CLIENT_PLUGIN_AUTH) && (authResponse._clientCapabilities & CLIENT_PLUGIN_AUTH)) 
            {
                err = err || getStr(srcPacket, authResponse._authPluginName);
            }


        }
        else
        {
            err = err || peekInt16(srcPacket, &l_cap);
            err = err || getInt32(srcPacket, &authResponse._maxPacketSize);
            err = err || getStr(srcPacket,authResponse._userName);
            if (srcPacket.size()) 
            {
                /* if there is more, it is the password without a terminating \0 */
                err = err || getStr(srcPacket,authResponse._userName);
            }

            if (!err) 
            {
                authResponse._clientCapabilities = l_cap;
            }
        }
        

        return err ? -1 : 0;

    }

    int   MysqlPacket::peekIntLen( string  &buff, uint64_t *value, size_t size)
    {
        size_t i=0;
        int shift;
        uint32_t r_l = 0, r_h = 0;

        const unsigned char *bytes=(const unsigned char *)buff.data();
        if (size>buff.size())
        {
            return -1;
        }

        for (i = 0, shift = 0; i < size && i < 4; i++, shift += 8, bytes++) 
        {
            r_l |= ((*bytes) << shift);
        }

        for (shift = 0; i < size; i++, shift += 8, bytes++) 
        {
            r_h |= ((*bytes) << shift);
        }

        *value = (((uint64_t)r_h << 32) | r_l);

        return 0;

    }

    int MysqlPacket:: peekInt16( string  &buff, uint16_t *value)
    {
        uint64_t v64;

        if (peekIntLen(buff, &v64, 2))
            return -1;

        if ((v64 & 0xffff) != v64)
        {
            return -1;
        }
        

        *value = v64 & 0xffff;

        return 0;

    }

    int   MysqlPacket::getIntLen( string  &buff, uint64_t *value, size_t size)
    {
        int err = 0;

        err = err || peekIntLen(buff, value, size);

        if (err) return -1;

        buff=buff.substr(size);

        return 0;
    }

    int   MysqlPacket::getInt8( string  &buff, uint8_t *value)
    {
        uint64_t v64;

        if (getIntLen(buff, &v64, 1)) return -1;

        if ((v64 & 0xff) != v64)
        {
        return -1;
        }
        

        *value = v64 & 0xff;

        return 0;
    }
    int   MysqlPacket::getInt16( string  &buff, uint16_t *value)
    {
        uint64_t v64;

        if (getIntLen(buff, &v64, 2)) return -1;

        if ((v64 & 0xffff)!= v64)
        {
            return -1;
        }
        

        *value = v64 & 0xffff;

        return 0;
    }

    int   MysqlPacket::getInt24( string  &buff, uint32_t *value)
    {
        uint64_t v64;

        if (getIntLen(buff, &v64, 3)) return -1;

        if ((v64 & 0x00ffffff) != v64)
        {
            return -1;
        }
        

        *value = v64 & 0x00ffffff;

        return 0;
    }
    int   MysqlPacket::getInt32( string  &buff, uint32_t *value)
    {
        uint64_t v64;

        if (getIntLen(buff, &v64, 4)) return -1;

        if ((v64 & 0xffffffff) != v64)
        {
            return -1;
        }
        

        *value = v64 & 0xffffffff;

        return 0;
    }

    int  MysqlPacket::getStr( string  &buff, string &des)
    {
        size_t i=0;
        for ( i = 0; i < buff.size(); i++)
        {
            if (buff[i]!='\0')
            {
                des.append(1,buff[i]);
            }
            else
            {
                break;
            }
            
        }

        if (i==buff.size())
        {
            des.clear();
            return -1;
        }
        
        return skipLen(buff,i+1);

    }

    int  MysqlPacket::getLenStr( string  &buff, uint8_t len, string &des)
    {
        size_t i=0;
        for ( i = 0; i < buff.size() && i <len; i++)
        {
            
            des.append(1,buff[i]);
            
        }
  
        return skipLen(buff,i);

    }

    int  MysqlPacket::appendAuthSwitch(string  &desPacket,const string & methodName, const string &salt)
    {
        appendIntLen(desPacket,0xfe,1);
        desPacket.append(methodName);
        desPacket.append(1,0x00);
        desPacket.append(salt.data(),salt.size());
        return 0;

    }

    string MysqlPacket::charsetGetName(int number)
    {
        static const char *charset[64] = {
            0, "big5", 0, 0, 0, 0, 0, 0, "latin1", 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, "gb2312", 0, 0, 0, "gbk", 0, 0, 0,
            0, "utf8", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "utf8mb4", 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, "binary"
        };

        if ((unsigned int )number >= (sizeof(charset) / sizeof(charset[0]))) 
        {
            return "";
        }

        return string(charset[number]);
    }

    int MysqlPacket::charsetGetNum(const string &charset)
    {
        static map<string ,int > mCharset={
            {"latin1", 0x08}, 
            {"big5", 0x01}, 
            {"gb2312", 0x18}, 
            {"gbk", 0x1c}, 
            {"utf8", 0x21}, 
            {"utf8mb4", 0x2d}, 
            {"binary", 0x3f}};

        if (mCharset.find(charset)!=mCharset.end())
        {
            return mCharset[charset];
        }
        else
        {
            return 0x21;
        }
    }

    int MysqlPacket::appendLenencInt(string &desPacket, uint64_t length)
    {
        if (length < 251) {
            desPacket.append(1,length);
        } else if (length < 65536) {
            desPacket.append(1,252);
            desPacket.append(1,(length >> 0) & 0xff);
            desPacket.append(1,(length >> 8) & 0xff);
        } else if (length < 16777216) {
            desPacket.append(1,253);
            desPacket.append(1,(length >> 0) & 0xff);
            desPacket.append(1,(length >> 8) & 0xff);
            desPacket.append(1,(length >> 16) & 0xff);

        } else {

            desPacket.append(1,254);
            desPacket.append(1,(length >> 0) & 0xff);
            desPacket.append(1,(length >> 8) & 0xff);
            desPacket.append(1,(length >> 16) & 0xff);
            desPacket.append(1,(length >> 24) & 0xff);
            desPacket.append(1,(length >> 32) & 0xff);
            desPacket.append(1,(length >> 40) & 0xff);
            desPacket.append(1,(length >> 48) & 0xff);
            desPacket.append(1,(length >> 56) & 0xff);//小端字节序,高位在高地址

        }

        return 0;
    }

    int MysqlPacket::appendLenencLenStr(string &desPacket,  const string &src)
    {
        if (src.length()==0)
        {
            desPacket.append(1,251);
        }else {
            appendLenencInt(desPacket,src.length());
            desPacket.append(src.data(),src.length());
        }

        return 0;

    }

    int MysqlPacket::appendOKPacket(string &desPacket, const OKPacket & oKPacket)
    {
        uint32_t capabilities = CLIENT_PROTOCOL_41;
        appendIntLen(desPacket,0x00,1);
        appendLenencInt(desPacket,oKPacket._affectedRows);
        appendLenencInt(desPacket,oKPacket._insertID);
        appendIntLen(desPacket,oKPacket._serverStatus,2);
        if (capabilities & CLIENT_PROTOCOL_41) 
        {
            appendIntLen(desPacket, oKPacket._warnings,2); /* no warnings */
        }
        return 0;

    }

    bool MysqlPacket::dataAppend( SocketContext &socketContext,string &sDest,const string& sBodydata)
    {
        string theSendData=sBodydata;
        do
        {
            size_t cur_packet_len = std::min(theSendData.size(), (size_t)PACKET_LEN_MAX);
            if (socketContext._packetIDIsReset) {
            socketContext._packetIDIsReset = false;
            /** the ++last_packet_id will make sure we send a 0 */
            socketContext._lastPacketID = 0xff;
            }

            string thisTimeHeader;
            MysqlPacket::appendIntLen(thisTimeHeader,cur_packet_len,3);
            MysqlPacket::appendIntLen(thisTimeHeader,++socketContext._lastPacketID,1);
            sDest.append(thisTimeHeader.data(),thisTimeHeader.size());
            sDest.append(theSendData.substr(0,cur_packet_len));

            if (cur_packet_len==PACKET_LEN_MAX)
            {
                string thisTimeHeader;
                MysqlPacket::appendIntLen(thisTimeHeader,0,3);
                MysqlPacket::appendIntLen(thisTimeHeader,++socketContext._lastPacketID,1);
                sDest.append(thisTimeHeader.data(),thisTimeHeader.size());

            }
            
            theSendData=theSendData.substr(cur_packet_len);

        } while (theSendData.size());

        return true;
        
    }
    bool MysqlPacket::appendResultSet( SocketContext &socketContext,string &sDest,const vector<st_mysql_field> &vField,const vector<string> &vRow)
    {
        string sSize;
        MysqlPacket::appendLenencInt(sSize,vField.size());
        dataAppend(socketContext,sDest,sSize);

        for (auto &field:vField)
        {
            string sFieldData;
            MysqlPacket::appendLenencLenStr(sFieldData,field.catalog.empty()?"def":field.catalog);
            MysqlPacket::appendLenencLenStr(sFieldData,field.db.empty()?"":field.db);
            MysqlPacket::appendLenencLenStr(sFieldData,field.table.empty()?"":field.table);
            MysqlPacket::appendLenencLenStr(sFieldData,field.org_table.empty()?"":field.org_table);
            MysqlPacket::appendLenencLenStr(sFieldData,field.name.empty()?"":field.name);
            MysqlPacket::appendLenencLenStr(sFieldData,field.org_name.empty()?"":field.org_name);
            sFieldData.append(1,'\x0c');
            sFieldData.append("\x08\x00",2);
            sFieldData.append(1,(field.length >> 0) & 0xff);
            sFieldData.append(1,(field.length >> 8) & 0xff);
            sFieldData.append(1,(field.length >> 16) & 0xff);
            sFieldData.append(1,(field.length >> 24) & 0xff);
            sFieldData.append(1,field.type);
            sFieldData.append(1,field.flags & 0xff);
            sFieldData.append(1,(field.flags >> 8) & 0xff);
            sFieldData.append(1,0x00);
            sFieldData.append("\x00\x00",2);
            dataAppend(socketContext,sDest,sFieldData);

        }

        /* EOF */
        string sEOF;
        sEOF.append(1, '\xfe');  /* EOF */
        sEOF.append( "\x00\x00", 2);  /* warning count */
        sEOF.append( "\x02\x00", 2);  /* flags */
        dataAppend(socketContext,sDest,sEOF);


        for (auto &row:vRow)
        {
            string sRow;
            MysqlPacket::appendLenencLenStr(sRow,row);
            dataAppend(socketContext,sDest,sRow);
        }
        /* EOF */
        dataAppend(socketContext,sDest,sEOF);
        
        return true;
    }

    bool MysqlPacket::appendRowResultSet( SocketContext &socketContext,string &sDest,const vector<st_mysql_field> &vField,vector<string> &vCloumnName, vector<map<string,string>> &vRowData)
    {
        string sSize;
        MysqlPacket::appendLenencInt(sSize,vField.size());
        dataAppend(socketContext,sDest,sSize);

        for (auto &field:vField)
        {
            string sFieldData;
            MysqlPacket::appendLenencLenStr(sFieldData,field.catalog.empty()?"def":field.catalog);
            MysqlPacket::appendLenencLenStr(sFieldData,field.db.empty()?"dfdb":field.db);
            MysqlPacket::appendLenencLenStr(sFieldData,field.table.empty()?"dftb":field.table);
            MysqlPacket::appendLenencLenStr(sFieldData,field.org_table.empty()?"dfot":field.org_table);
            MysqlPacket::appendLenencLenStr(sFieldData,field.name.empty()?"":field.name);
            MysqlPacket::appendLenencLenStr(sFieldData,field.org_name.empty()?"dfon":field.org_name);
            sFieldData.append(1,'\x0c');
            sFieldData.append("\x08\x00",2);
            sFieldData.append(1,(field.length >> 0) & 0xff);
            sFieldData.append(1,(field.length >> 8) & 0xff);
            sFieldData.append(1,(field.length >> 16) & 0xff);
            sFieldData.append(1,(field.length >> 24) & 0xff);
            sFieldData.append(1,field.type);
            sFieldData.append(1,field.flags & 0xff);
            sFieldData.append(1,(field.flags >> 8) & 0xff);
            sFieldData.append(1,0x00);
            sFieldData.append("\x00\x00",2);
            dataAppend(socketContext,sDest,sFieldData);

        }

        /* EOF */
        string sEOF;
        sEOF.append(1, '\xfe');  /* EOF */
        sEOF.append( "\x00\x00", 2);  /* warning count */
        sEOF.append( "\x02\x00", 2);  /* flags */
        dataAppend(socketContext,sDest,sEOF);


        for (size_t i = 0; i < vRowData.size(); i++)
        {
            auto &mRow=vRowData[i];
            string sRow;
            for (size_t i = 0; i < vCloumnName.size(); i++)
            {
                MysqlPacket::appendLenencLenStr(sRow,mRow[vCloumnName[i]]);

            }
            dataAppend(socketContext,sDest,sRow);

        }
        /* EOF */
        dataAppend(socketContext,sDest,sEOF);
        
        return true;
    }


}