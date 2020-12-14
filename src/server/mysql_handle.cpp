#include "mysql_handle.h"

#include <iostream>
#include <string>
#include <vector>
#include <cassert>


// include the sql parser
#include "SQLParser.h"
#include "util/sqlhelper.h"
#include "logger/logger.h"
#include "server/horsedb_server.h"
#include "mysql/command.h"
#include "client/Communicator.h"
#include "cfg/config.h"


using namespace std;
using namespace hsql;

namespace horsedb {


class RaftDBCallback:public RaftDBPrxCallback
{

     void callback_appendEntries(horsedb::Int32 ret,  const horsedb::AppendEntriesRes& tRes)
	 {
		 cout<< "callback_appendEntries"<<endl;
		 return ;

	 }  
    // virtual void callback_AppendEntries_exception(horsedb::Int32 ret);

    // virtual void callback_requestVote(horsedb::Int32 ret,  const horsedb::RequestVoteRes& tRes);   
    // virtual void callback_requestVote_exception(horsedb::Int32 ret);

    // virtual void callback_preVote(horsedb::Int32 ret,  const horsedb::RequestVoteRes& tRes);
    // virtual void callback_preVote_exception(horsedb::Int32 ret);

    // virtual void callback_requestVote(horsedb::Int32 ret,  const horsedb::RequestVoteRes& tRes);
    // virtual void callback_requestVote_exception(horsedb::Int32 ret);

    // virtual void callback_timeoutNow(horsedb::Int32 ret,  const horsedb::TimeoutNowRes& tRes);
    // virtual void callback_timeoutNow_exception(horsedb::Int32 ret);


};


    void MysqlHandle::initialize()
	{
		
		cout << "MysqlHandle::initialize: " << std::this_thread::get_id() << endl;
	}

    	/**
	 *
	 */
	void MysqlHandle::handle(const shared_ptr<TC_EpollServer::RecvContext> &data)
	{
		//int iRet=-1;
        
		do
		{
			try
			{			
			
				shared_ptr<TC_EpollServer::SendContext> send = data->createSendContext();

				LOG_INFO("SocketHandle::handle : "+data->ip()+":"+ TC_Common::tostr(data->port())  );
				vector<char>  &reqBuff=data->buffer();

				string sReqBuff;
				//string sReqBuff(&reqBuff[0],reqBuff.size());
				sReqBuff.assign(reqBuff.begin(), reqBuff.end());
				cout<<"sReqBuff.size()="<<sReqBuff.size()<<endl;
				TC_EpollServer* epollServer=getEpollServer();
				cout<<"data->uid()="<<data->uid()<<endl;
				TC_EpollServer::Connection *con=epollServer->getNetThreadOfFd(data->fd())->getConnectionPtr(data->uid());
				if (!con)
				{
					cout<<"error,con is null"<<endl;
					break;
				}
				
				if (!con->getThirdConData().Is<SocketContext>()
					||con->getThirdConData().IsNull())//
				{
					cout<<"error,Any is null"<<endl;
					break;
				}
				
				SocketContext &tSocketContext=con->getThirdConData().AnyCast<SocketContext>();
				if (!tSocketContext._defaultDB.empty())
				{
					cout<<"======had use DB:"<<tSocketContext._defaultDB<<endl;
				}
				
				if (!con->getThirdAuthState())
				{
					int   clientPacketid = MysqlPacket::getHeaderPacketID(sReqBuff);
					cout<<"clientPacketid="<<clientPacketid<<endl;
					
					if (tSocketContext._packetIDIsReset)
					{
						tSocketContext._lastPacketID = clientPacketid;

						cout<<"set  pack id:"<<clientPacketid<<endl;
						tSocketContext._packetIDIsReset = false;
					}

					MysqlPacket::skipHeader(sReqBuff);
					AuthResponse &authResponse=tSocketContext._AuthResponse;
					if(authResponse._authPluginData.empty())
					{
						if(tSocketContext._AuthRequest._authPluginData.empty())
						{
							cout<<"error,_AuthRequest._authPluginData is NULL"<<endl;
							break;

						}

						
						SocketContext::initAuthResponse(tSocketContext._AuthRequest._capabilities,authResponse);
						cout<<"init,authResponse._clientCapabilities="<<authResponse._clientCapabilities<<endl;
						int err = MysqlPacket::unpacket2AuthRes(sReqBuff, authResponse);
						if (err) 
						{
							cout<<"error,unpacket2AuthRes failed"<<endl;
							break;
						}
						if (!(authResponse._clientCapabilities & CLIENT_PROTOCOL_41))
						{
							SocketContext::sendData(tSocketContext,send,this,string("\xff\xd7\x07" "4.0 protocol is not supported"));
							cout<<"error,4.0 protocol is not supported,authResponse._clientCapabilities="<<authResponse._clientCapabilities<<endl;
						}
						if (authResponse._clientCapabilities & CLIENT_COMPRESS) 
						{
							tSocketContext._isClientCompressed = 1;
							cout<<"client compressed"<<endl;
						}
						if (authResponse._clientCapabilities & CLIENT_MULTI_STATEMENTS) 
						{
							tSocketContext._isMultiStmtSet = 1;
						}

						
						if (authResponse._database.size())
						{
							tSocketContext._defaultDB=authResponse._database;
							cout<<"======connect use ,db ="<<tSocketContext._defaultDB<<endl;
						}
						if ((authResponse._clientCapabilities & CLIENT_PLUGIN_AUTH) && (strcmp(authResponse._authPluginName.c_str(), "mysql_native_password") != 0))
						{
							string sDest;
							MysqlPacket::appendAuthSwitch(sDest, "mysql_native_password",tSocketContext._AuthRequest._authPluginData);
							SocketContext::sendData(tSocketContext, send,this, sDest);
							cout<<"client not mysql_native_password"<<endl;
							
							break;
						}


					}
					else
					{
						tSocketContext._AuthResponse._authPluginData=sReqBuff;
						cout<<" 2nd round auth"<<endl;
					}

					string clientCharset=MysqlPacket::charsetGetName(authResponse._charset);
					if (clientCharset.empty()) 
					{
						authResponse._charset = MysqlPacket::charsetGetNum("utf8");
					}

					tSocketContext._charsetCode=authResponse._charset;
					tSocketContext._charset=clientCharset;
					tSocketContext._charsetClient=clientCharset;
					tSocketContext._charsetResults=clientCharset;
					tSocketContext._charsetConnection=clientCharset;

					SocketContext::sendOK(tSocketContext, send,this);

					con->setThirdAuthState(1);
				
					cout<<"========"<<clientPacketid<<endl;

				}
				else//exe sql
				{
					//test proxy
					try
					{
						

						AppendEntriesReq tReq;
						AppendEntriesRes tRes;
						
						
						// typedef shared_ptr<RaftDBCallback> RaftDBCallbackPtr;//RaftDBPrxCallbackPtr
						// RaftDBCallbackPtr callback(new RaftDBCallback());


						// tReq.groupID="groupid is 1";
           				//  g_app._pPrx->async_appendEntries(callback,tReq);

						// horsedb::TarsOutputStream<BufferWriter> os;
						// tReq.writeTo(os);


						// cout<<"========rpc,AppendEntries,ret="<<0<<",tRes.term="<< tRes.term<<endl;
						
					}
					catch(const std::exception& e)
					{
						std::cerr <<"proxy error:" <<e.what()  << '\n';
					}
					
					

					int   clientPacketid = MysqlPacket::getHeaderPacketID(sReqBuff);
					cout<<"exe sql,clientPacketid="<<clientPacketid<<endl;
					tSocketContext._lastPacketID = clientPacketid;					

					MysqlPacket::skipHeader(sReqBuff);
					uint8_t command;
					MysqlPacket::getInt8(sReqBuff,&command);
					cout<<"exe sql,command="<<(int)command<<endl;

					string sSQL;
					MysqlPacket::getLenStr(sReqBuff,sReqBuff.length(),sSQL);
					cout<<"exe sql,sSQL="<<sSQL<<endl;
					if (command==COM_INIT_DB)
					{
						tSocketContext._defaultDB=sSQL;
						SocketContext::sendOK(tSocketContext, send,this);
						break;
					}
					else if (command==COM_FIELD_LIST)
					{
						//SocketContext::sendOK(tSocketContext, send,this);
						Command::adminShowFields(tSocketContext, send,this);
						
						break;
					}
					else if (command==COM_QUIT)
					{
						
						cout<<"recv client close cmd"<<endl;
						break;
					}
					
					else
					{
						
						if (sSQL.find("@@version_comment")!=sSQL.npos)
						{
							Command::adminSelectVersionComment(tSocketContext, send,this);
							break;
						}
						
						auto &config=Config::getInstance()->getConfig();
						hsql::SQLParserResult result;
						hsql::SQLParser::parse(sSQL, &result);
						if (result.isValid()) 
						{
							cout<<"Parsed successfully!"<<endl;
							cout<<"Number of statements: "<<result.size()<<endl;

							for (auto i = 0u; i < result.size(); ++i) 
							{
								// Print a statement summary.
								hsql::printStatementInfo(result.getStatement(i));
								auto* statement =result.getStatement(i);

								switch (statement->type())
								{
									case kStmtShow:
									{
										const ShowStatement* showSt = static_cast<const hsql::ShowStatement*>(statement);
										if (showSt->type==hsql::kShowDatabases)
										{
											cout<<"kShowDatabases:"<<sSQL<<endl;
											vector<string> vDatabase;
											g_app._db->ShowDB(vDatabase);
											Command::adminShowDatabases(tSocketContext, send,vDatabase,this);
											//SocketContext::sendOK(tSocketContext, send,this);
											return ;
											
											
										}
										else if (showSt->type==hsql::kShowTables)
										{
											cout<<"kShowTables:"<<sSQL<<endl;
											if (tSocketContext._bSkipTable)//mysql连接的时候简单处理
											{
												SocketContext::sendOK(tSocketContext, send,this);
												tSocketContext._bSkipTable=false;
											}
											else
											{
												vector<string> vTable;
												g_app._table->getDBTables(vTable,tSocketContext._defaultDB);
												Command::adminShowTables(tSocketContext, send,vTable,this);
											}
						
											
											return ;
											
										}
										
										
									}
									break;

									case kStmtCreate:
									{
										const CreateStatement* createSt = static_cast<const hsql::CreateStatement*>(statement);
										if (createSt->columns)
										{
											cout<<"*(createSt->columns).size()= "<<(*(createSt->columns)).size()<<endl;
										}
										
										g_app._table->create(createSt,tSocketContext._defaultDB);
										
										if (!g_app._bRaft)
										{
											SocketContext::sendOK(tSocketContext, send,this);
										}
										else
										{
											g_app._smImp.add_cp_task(tSocketContext, send,this);
										}
										
										return ;

									}


										

										break;
									case kStmtInsert:
									{
										const InsertStatement* insertSt = static_cast<const hsql::InsertStatement*>(statement);
										cout<<"*(insertSt->columns).size()= "<<(*(insertSt->columns)).size()<<endl;
										g_app._table->insertTable(insertSt,tSocketContext._defaultDB);
										
										if (!g_app._bRaft)
										{
											SocketContext::sendOKFull(tSocketContext,send,this,1,0,SERVER_STATUS_AUTOCOMMIT,0);
										}
										else
										{
											g_app._smImp.add_cp_task(tSocketContext, send,this);
										}

										return ;

									}
										
										break;
										
									case kStmtSelect:
									{
										const SelectStatement* selectSt = static_cast<const hsql::SelectStatement*>(statement);
										vector<string> vCloumnName;
										vector<map<string,string>> mData;

										g_app._table->selectTable(selectSt,vCloumnName,mData,tSocketContext._defaultDB);

										Command::selectRowData(tSocketContext, send,this,vCloumnName,mData);
										return ;
										

									}
									break;

									
									default:
										break;
								}

								
							}

							
							
							
						} 
						else 
						{
							fprintf(stderr, "Given string is not a valid SQL query.\n");
							fprintf(stderr, "%s (L%d:%d)\n",result.errorMsg(),result.errorLine(),result.errorColumn());
							
						}	


						
						Command::adminSelectVersionComment(tSocketContext, send,this);
						
						
						
					}
					

					

				}

				

			}
			catch (exception &ex)
			{
				cerr << "MysqlHandle::handle ex:" << ex.what() << endl;
				close(data);
			}
		} while (0);
	}

	/**
	 * [handleClose description]
	 * @param data [description]
	 */
	void MysqlHandle::handleClose(const shared_ptr<TC_EpollServer::RecvContext> &data)
	{
		try
		{

			cout << "MysqlHandle::handleClose : " << data->ip() << ":" << data->port();
		}
		catch (exception &ex)
		{
			cerr << "MysqlHandle::handle ex:" << ex.what() << endl;
			close(data);
		}
	}




}
	
