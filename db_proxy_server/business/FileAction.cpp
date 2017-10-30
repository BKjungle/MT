/*================================================================
*     Copyright (c) 2014年 lanhu. All rights reserved.
*   
*   文件名称：FileAction.cpp
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年12月31日
*   描    述：
*
================================================================*/
#include "FileAction.h"
#include "FileModel.h"
#include "IM.File.pb.h"
#include "../ProxyConn.h"
#include "../DBPool.h"

namespace DB_PROXY {

    
    void hasOfflineFile(CImPdu* pPdu, uint32_t conn_uuid)
    {
    	/*
        IM::File::IMFileHasOfflineReq msg;
        IM::File::IMFileHasOfflineRsp msgResp;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            CImPdu* pPduRes = new CImPdu;
            
            uint32_t nUserId = msg.user_id();
            CFileModel* pModel = CFileModel::getInstance();
            list<IM::BaseDefine::OfflineFileInfo> lsOffline;
            pModel->getOfflineFile(nUserId, lsOffline);
            msgResp.set_user_id(nUserId);
            for (list<IM::BaseDefine::OfflineFileInfo>::iterator it=lsOffline.begin();
                 it != lsOffline.end(); ++it) {
                IM::BaseDefine::OfflineFileInfo* pInfo = msgResp.add_offline_file_list();
    //            *pInfo = *it;
                pInfo->set_from_user_id(it->from_user_id());
                pInfo->set_task_id(it->task_id());
                pInfo->set_file_name(it->file_name());
                pInfo->set_file_size(it->file_size());
				pInfo->set_file_md5(it->file_md5());			// add md5 
				pInfo->set_status(1);							// add 6/16
            }
            
            log("userId=%u, count=%u", nUserId, msgResp.offline_file_list_size());
            
            msgResp.set_attach_data(msg.attach_data());
            pPduRes->SetPBMsg(&msgResp);
            pPduRes->SetSeqNum(pPdu->GetSeqNum());
            pPduRes->SetServiceId(IM::BaseDefine::SID_FILE);
            pPduRes->SetCommandId(IM::BaseDefine::CID_FILE_HAS_OFFLINE_RES);
            CProxyConn::AddResponsePdu(conn_uuid, pPduRes);
						
			// ---- update status 1 to 2 
			CDBConn* pDBConn = CDBManager::getInstance()->GetDBConn("teamtalk_slave");
			string strSql = "update IMTransmitFile set status=2 where toId="+int2string(nUserId) + " and status=1 ";
			mysql_ping(pDBConn->GetMysql());
			if (mysql_real_query(pDBConn->GetMysql(), strSql.c_str(),strSql.length())) {
					log("mysql_real_query failed: %s, sql: %s", mysql_error(pDBConn->GetMysql()), strSql.c_str());
					
				}else
					{
					  int ret = mysql_affected_rows(pDBConn->GetMysql());
					  log(" mysql_addected  %d row ",ret);
					}
			CDBManager::getInstance()->RelDBConn(pDBConn);
        }
        else
        {
            log("parse pb failed");
        }
        */
        
        IM::File::IMFileHasOfflineReq msg;
        IM::File::IMFileHasOfflineRsp msgResp;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            CImPdu* pPduRes = new CImPdu;
            
            uint32_t nUserId = msg.user_id();
            CFileModel* pModel = CFileModel::getInstance();
            list<IM::BaseDefine::OfflineFileInfo> lsOffline;// 待下载文件
			list<IM::BaseDefine::OfflineFileInfo> lsSent;// 待通知文件
            pModel->getOfflineFile(nUserId, lsOffline);// get file that need to download
            pModel->getHasSentFile(nUserId, lsSent);	// --------get peer has downloaded file 
            
            msgResp.set_user_id(nUserId);
            for (list<IM::BaseDefine::OfflineFileInfo>::iterator it=lsOffline.begin();
                 it != lsOffline.end(); ++it) {
                IM::BaseDefine::OfflineFileInfo* pInfo = msgResp.add_offline_file_list();//自动增
    
                pInfo->set_from_user_id(it->from_user_id());
                pInfo->set_task_id(it->task_id());
                pInfo->set_file_name(it->file_name());
                pInfo->set_file_size(it->file_size());
				pInfo->set_file_md5(it->file_md5());			// add md5 
				pInfo->set_status(IM::BaseDefine::FILE_NEED_DOWNLOAD);							// add 6/16
            }
				 int cut =0 ;
			log("userId=%u,need file count=%u", nUserId,cut= msgResp.offline_file_list_size());
			
			for(auto it = lsSent.begin();it != lsSent.end(); ++it){   // ------add 10.16
				IM::BaseDefine::OfflineFileInfo* pInfo = msgResp.add_offline_file_list();//自动增
    
                pInfo->set_from_user_id(it->from_user_id());
                pInfo->set_task_id(it->task_id());
                pInfo->set_file_name(it->file_name());
                pInfo->set_file_size(it->file_size());
				pInfo->set_file_md5(it->file_md5());			
				pInfo->set_status(IM::BaseDefine::FILE_NEED_DISPLAY);// notify peer has downloaded		
			}
            
            log("userId=%u,need display count=%u ,and all list.size = %u", nUserId, msgResp.offline_file_list_size() - cut,msgResp.offline_file_list_size());
            
            msgResp.set_attach_data(msg.attach_data());
            pPduRes->SetPBMsg(&msgResp);
            pPduRes->SetSeqNum(pPdu->GetSeqNum());
            pPduRes->SetServiceId(IM::BaseDefine::SID_FILE);
            pPduRes->SetCommandId(IM::BaseDefine::CID_FILE_HAS_OFFLINE_RES);
            CProxyConn::AddResponsePdu(conn_uuid, pPduRes);


			CDBConn* pDBConn = CDBManager::getInstance()->GetDBConn("teamtalk_slave");
			//---- update status 1 to 2   // 当第二版完成时，此处删除，因为 必须等到client返回接受信息，才能改数据库状态
			string strSql = "update IMTransmitFile set status=2 where toId="+int2string(nUserId) + " and status=1 ";
			mysql_ping(pDBConn->GetMysql());
			if (mysql_real_query(pDBConn->GetMysql(), strSql.c_str(),strSql.length())) {
					log("mysql_real_query 1 to 2 failed: %s, sql: %s", mysql_error(pDBConn->GetMysql()), strSql.c_str());
				
				}else
					{
					  int ret = mysql_affected_rows(pDBConn->GetMysql());
					  log(" mysql_affected 1 to 2 %d row ",ret);
					}
			//--- update status 4 to 5
			 strSql = "update IMTransmitFile set status=5 where fromId="+int2string(nUserId) + " and status=4 ";
			mysql_ping(pDBConn->GetMysql());
			if (mysql_real_query(pDBConn->GetMysql(), strSql.c_str(),strSql.length())) {
					log("mysql_real_query  4 to 5 failed: %s, sql: %s", mysql_error(pDBConn->GetMysql()), strSql.c_str());
				
				}else
					{
					  int ret = mysql_affected_rows(pDBConn->GetMysql());
					  log(" mysql_affected 1 to 2 %d row ",ret);
					}
			
        	CDBManager::getInstance()->RelDBConn(pDBConn);
			
        }
        else
        {
            log("parse pb failed");
        }
        
    }
    
    void addOfflineFile(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::File::IMFileAddOfflineReq msg;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nUserId = msg.from_user_id();
            uint32_t nToId = msg.to_user_id();
            string strTaskId = msg.task_id();
            string strFileName = msg.file_name();
            uint32_t nFileSize = msg.file_size();
			string file_md5 = msg.file_md5();				//add md5 
			uint32_t status = msg.status();					//add status 	
            CFileModel* pModel = CFileModel::getInstance();
            pModel->addOfflineFile(nUserId, nToId, strTaskId, strFileName, nFileSize,file_md5,status);
            log("fromId=%u, toId=%u, taskId=%s, fileName=%s, fileSize=%u, status=%d ", nUserId, nToId, strTaskId.c_str(), strFileName.c_str(), nFileSize,msg.status());
        }
    }
    
    void delOfflineFile(CImPdu* pPdu, uint32_t conn_uuid)
    {
        IM::File::IMFileDelOfflineReq msg;
        if(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
        {
            uint32_t nUserId = msg.from_user_id();
            uint32_t nToId = msg.to_user_id();
            string strTaskId = msg.task_id();
            CFileModel* pModel = CFileModel::getInstance();
            pModel->delOfflineFile(nUserId, nToId, strTaskId);
            log("fromId=%u, toId=%u, taskId=%s", nUserId, nToId, strTaskId.c_str());
        }
    }
	

	// add 6.22
		void notifyClientAck(CImPdu* pPdu,uint32_t conn_uuid)
		{
			IM::BaseDefine::InfoNotify  infonotify;
			CHECK_PB_PARSE_MSG(infonotify.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
			CFileModel* pModel = CFileModel::getInstance();
			for(int i = 0;i< infonotify.offline_file_list_size(); i++)
					{
						IM::BaseDefine::OfflineFileInfo fileinfo = infonotify.offline_file_list(i);
						pModel->ClientAckChangeStatus(fileinfo.task_id(),fileinfo.status());
						log("ClientAckChangeStatus file_name = %s, size = %d , status = %d  ",fileinfo.file_name().c_str(),fileinfo.file_size(),fileinfo.status());
			
					}
		}

	void changeFileStatus(CImPdu* pPdu,uint32_t conn_uuid)
		{
			IM::BaseDefine::InfoNotify  infonotify;
			CHECK_PB_PARSE_MSG(infonotify.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
			CFileModel* pModel = CFileModel::getInstance();
			for(int i = 0;i< infonotify.offline_file_list_size(); i++)
					{
						IM::BaseDefine::OfflineFileInfo fileinfo = infonotify.offline_file_list(i);
						pModel->ChangeFileStatus(fileinfo.task_id(),fileinfo.status());
						log(" FileServerChangeFileStatus file_name =%s, size =%d,status = %d",fileinfo.file_name().c_str(),fileinfo.file_size(),fileinfo.status());
			
					}
		
		}
};
