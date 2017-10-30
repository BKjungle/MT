/*================================================================
*     Copyright (c) 2014年 lanhu. All rights reserved.
*   
*   文件名称：FileModel.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2014年12月31日
*   描    述：
*
#pragma once
================================================================*/
#ifndef __FILEMODEL_H__
#define __FILEMODEL_H__
#include "IM.File.pb.h"
#include "ImPduBase.h"

class CFileModel
{
public:
    virtual ~CFileModel();
    static CFileModel* getInstance();
    
    void getOfflineFile(uint32_t userId, list<IM::BaseDefine::OfflineFileInfo>& lsOffline);
    void addOfflineFile(uint32_t fromId, uint32_t toId, string& taskId, string& fileName, uint32_t fileSize,string& filemd5,uint32_t status);
    void delOfflineFile(uint32_t fromId, uint32_t toId, string& taskId);
		void ClientAckChangeStatus(string  taskId,uint32_t status);	// add 6.21
		void ChangeFileStatus(string taskId,uint32_t status); 		// add 7.14	
		void getHasSentFile(uint32_t userId,list<IM::BaseDefine::OfflineFileInfo>& lsSent);// add 10,19 
    
private:
    CFileModel();
    
private:
    static CFileModel* m_pInstance;
};

#endif /*defined(__FILEMODEL_H__) */
