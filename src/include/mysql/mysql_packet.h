#ifndef _MYSQL_PACKET_H
#define _MYSQL_PACKET_H

#include <string>
#include "mysql/socket_context.h"

using namespace std;


#define CLIENT_LONG_PASSWORD    1       /* new more secure passwords */
#define CLIENT_FOUND_ROWS       2       /* Found instead of affected rows */
#define CLIENT_LONG_FLAG        4       /* Get all column flags */
#define CLIENT_CONNECT_WITH_DB  8       /* One can specify db on connect */
#define CLIENT_NO_SCHEMA        16      /* Don't allow database.table.column */
#define CLIENT_COMPRESS         32      /* Can use compression protocol */
#define CLIENT_ODBC             64      /* Odbc client */
#define CLIENT_LOCAL_FILES      128     /* Can use LOAD DATA LOCAL */
#define CLIENT_IGNORE_SPACE     256     /* Ignore spaces before '(' */
#define CLIENT_PROTOCOL_41      512     /* New 4.1 protocol */
#define CLIENT_INTERACTIVE      1024    /* This is an interactive client */
#define CLIENT_SSL              2048    /* Switch to SSL after handshake */
#define CLIENT_IGNORE_SIGPIPE   4096    /* IGNORE sigpipes */
#define CLIENT_TRANSACTIONS     8192    /* Client knows about transactions */
#define CLIENT_RESERVED         16384   /* Old flag for 4.1 protocol  */
#define CLIENT_SECURE_CONNECTION 32768  /* New 4.1 authentication */
#define CLIENT_MULTI_STATEMENTS (1UL << 16) /* Enable/disable multi-stmt support */
#define CLIENT_MULTI_RESULTS    (1UL << 17) /* Enable/disable multi-results */
#define CLIENT_PS_MULTI_RESULTS (1UL << 18) /* Multi-results in PS-protocol */

#define CLIENT_PLUGIN_AUTH  (1UL << 19) /* Client supports plugin authentication */
#define CLIENT_CONNECT_ATTRS (1UL << 20) /* Client supports connection attributes */

/* Enable authentication response packet to be larger than 255 bytes. */
#define CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA (1UL << 21)

/* Don't close the connection for a connection with expired password. */
#define CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS (1UL << 22)

#define CLIENT_SSL_VERIFY_SERVER_CERT (1UL << 30)
#define CLIENT_REMEMBER_OPTIONS (1UL << 31)


#define CLIENT_DEPRECATE_EOF (1UL << 24)

#define CLIENT_ALL_FLAGS  (CLIENT_LONG_PASSWORD \
                           | CLIENT_FOUND_ROWS \
                           | CLIENT_LONG_FLAG \
                           | CLIENT_CONNECT_WITH_DB \
                           | CLIENT_NO_SCHEMA \
                           | CLIENT_COMPRESS \
                           | CLIENT_ODBC \
                           | CLIENT_LOCAL_FILES \
                           | CLIENT_IGNORE_SPACE \
                           | CLIENT_PROTOCOL_41 \
                           | CLIENT_INTERACTIVE \
                           | CLIENT_SSL \
                           | CLIENT_IGNORE_SIGPIPE \
                           | CLIENT_TRANSACTIONS \
                           | CLIENT_RESERVED \
                           | CLIENT_SECURE_CONNECTION \
                           | CLIENT_MULTI_STATEMENTS \
                           | CLIENT_MULTI_RESULTS \
                           | CLIENT_PS_MULTI_RESULTS \
                           | CLIENT_SSL_VERIFY_SERVER_CERT \
                           | CLIENT_REMEMBER_OPTIONS \
                           | CLIENT_PLUGIN_AUTH \
                           | CLIENT_CONNECT_ATTRS \
                           | CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA \
                           | CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS \
)

#define CLIENT_BASIC_FLAGS (((CLIENT_ALL_FLAGS & ~CLIENT_SSL) \
                                               & ~CLIENT_COMPRESS) \
                                               & ~CLIENT_SSL_VERIFY_SERVER_CERT)

#ifndef CLIENT_BASIC_FLAGS /* for mariadb version 10^ */
#define CLIENT_BASIC_FLAGS CLIENT_DEFAULT_FLAGS
#endif
#ifndef SERVER_MORE_RESULTS_EXISTS /* for mariadb version 10^ */
#define SERVER_MORE_RESULTS_EXISTS SERVER_MORE_RESULTS_EXIST
#endif
#ifndef CLIENT_PROGRESS /* mariadb progress reporting */
#define CLIENT_PROGRESS (1UL << 29)
#endif

#define MYSQL_VERSION_ID                50640

#if MYSQL_VERSION_ID < 50606
#define COMPATIBLE_BASIC_FLAGS (CLIENT_BASIC_FLAGS                      \
	|CLIENT_PLUGIN_AUTH_LENENC_CLIENT_DATA  \
	|CLIENT_CAN_HANDLE_EXPIRED_PASSWORDS    \
	|CLIENT_SESSION_TRACK                   \
	|CLIENT_PLUGIN_AUTH)
#else
#define COMPATIBLE_BASIC_FLAGS CLIENT_BASIC_FLAGS
#endif

#define HORSEDB_DEFAULT_FLAGS (COMPATIBLE_BASIC_FLAGS                     \
	& ~CLIENT_NO_SCHEMA /* permit database.table.column */ \
	& ~CLIENT_IGNORE_SPACE                     \
	& ~CLIENT_DEPRECATE_EOF                    \
	& ~CLIENT_LOCAL_FILES                      \
	& ~CLIENT_PROGRESS                         \
	& ~CLIENT_CONNECT_ATTRS)


#define MYSQL_HEADER_SIZE 4 
#define PACKET_LEN_MAX (0x00ffffff)

#define SERVER_STATUS_AUTOCOMMIT   2        /* Server in auto_commit mode */

#define  COM_QUIT  0x01  /*client close*/
#define  COM_INIT_DB  0x02  /*command:use db*/
#define  COM_QUERY  0x03  /*command:select,insert,update.....*/
#define  COM_FIELD_LIST	 0x04 /*获取数据表字段信息*/
#define  COM_CREATE_DB  0x05  /*创建数据库	*/
#define  COM_DROP_DB  0x06  /*删除数据库	*/


namespace horsedb{

    struct OKPacket{

        uint64_t _affectedRows;
        uint64_t _insertID;
        uint16_t _serverStatus;
        uint16_t _warnings;

        string _msg;
    };

    struct ErrorPacket{

        string          _errMsg;
        uint16_t        _errCode;
        string          _sqlState;
        MysqlVersion    _ver;
    };


    enum enum_field_types { MYSQL_TYPE_DECIMAL, MYSQL_TYPE_TINY,
                            MYSQL_TYPE_SHORT,  MYSQL_TYPE_LONG,
                            MYSQL_TYPE_FLOAT,  MYSQL_TYPE_DOUBLE,
                            MYSQL_TYPE_NULL,   MYSQL_TYPE_TIMESTAMP,
                            MYSQL_TYPE_LONGLONG,MYSQL_TYPE_INT24,
                            MYSQL_TYPE_DATE,   MYSQL_TYPE_TIME,
                            MYSQL_TYPE_DATETIME, MYSQL_TYPE_YEAR,
                            MYSQL_TYPE_NEWDATE, MYSQL_TYPE_VARCHAR,
                            MYSQL_TYPE_BIT,
                            MYSQL_TYPE_TIMESTAMP2,
                            MYSQL_TYPE_DATETIME2,
                            MYSQL_TYPE_TIME2,
                            MYSQL_TYPE_NEWDECIMAL=246,
                            MYSQL_TYPE_ENUM=247,
                            MYSQL_TYPE_SET=248,
                            MYSQL_TYPE_TINY_BLOB=249,
                            MYSQL_TYPE_MEDIUM_BLOB=250,
                            MYSQL_TYPE_LONG_BLOB=251,
                            MYSQL_TYPE_BLOB=252,
                            MYSQL_TYPE_VAR_STRING=253,
                            MYSQL_TYPE_STRING=254,
                            MYSQL_TYPE_GEOMETRY=255

    };

    struct st_mysql_field {
        string name;                 /* Name of column */
        string org_name;             /* Original column name, if an alias */
        string table;                /* Table of column if column was a field */
        string org_table;            /* Org table name, if table was an alias */
        string db;                   /* Database for table */
        string catalog;              /* Catalog for table */
        string def;                  /* Default value (set by mysql_list_fields) */
        uint64_t  length;       /* Width of column (create length) */
        uint64_t  max_length;   /* Max width for selected set */
        uint32_t  name_length;
        uint32_t  org_name_length;
        uint32_t  table_length;
        uint32_t  org_table_length;
        uint32_t  db_length;
        uint32_t  catalog_length;
        uint32_t  def_length;
        uint32_t  flags;         /* Div flags */
        uint32_t  decimals;      /* Number of decimals in field */
        uint32_t  charsetnr;     /* Character set */
        enum enum_field_types type; /* Type of field. See mysql_com.h for types */
        void *extension;
    };

    class MysqlPacket{

        public:

        static int appendIntLen(string  &desPacket, uint64_t src, size_t byteSize);

        static int packetAuthReq(string  &desPacket, const AuthRequest &authRequest);
        static int unpacket2AuthRes( string  &srcPacket,  AuthResponse &authResponse);
        static int  packetHeader(string  &header, uint8_t packetID,size_t bodylen);

        static int  getHeaderPacketID(const string  &buff);
        static void  skipHeader(string  &buff);
        static int   skipLen(string  &buff,size_t len);
        static int   peekIntLen( string  &buff, uint64_t *value, size_t size);
        static int   peekInt16( string  &buff, uint16_t *value);

        static int  getIntLen( string  &buff, uint64_t *value, size_t size);
        static int  getInt8( string  &buff, uint8_t *value);
        static int  getInt16( string  &buff, uint16_t *value);
        static int  getInt24( string  &buff, uint32_t *value);
        static int  getInt32( string  &buff, uint32_t *value);

        static int  getStr( string  &buff, string &des);
        static int  getLenStr( string  &buff, uint8_t len, string &des);
        static int  appendAuthSwitch(string  &desPacket,const string & methodName, const string &salt);
        static string charsetGetName(int number);
        static int charsetGetNum(const string &charset);
        static int appendLenencInt(string &desPacket, uint64_t length);
        static int appendLenencLenStr(string &desPacket, const string &src);

        static int appendOKPacket(string &desPacket, const OKPacket & oKPacket);

        static bool dataAppend( SocketContext &socketContext,string &sDest,const string& sBodydata);
        static bool appendResultSet( SocketContext &socketContext,string &sDest,const vector<st_mysql_field> &vFile,const vector<string> &vRow);
        static bool appendRowResultSet( SocketContext &socketContext,string &sDest,const vector<st_mysql_field> &vField,vector<string> &vCloumnName, vector<map<string,string>> &vRowData);
    };
}


#endif