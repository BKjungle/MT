//
//  transfer_task_manager.cpp
//  im-server-mac-new
//
//  Created by wubenqi on 15/7/16.
//  Copyright (c) 2015年 benqi. All rights reserved.
//

#include "file_server/transfer_task_manager.h"

#include "base/pb/protocol/IM.BaseDefine.pb.h"
#include "base/util.h"

#include "file_server/config_util.h"
#include "file_server/file_client_conn.h"

using namespace IM::BaseDefine;

TransferTaskManager::TransferTaskManager() {
    
}

void TransferTaskManager::OnTimer(uint64_t tick) {
    for (TransferTaskMap::iterator it = transfer_tasks_.begin(); it != transfer_tasks_.end();) {
        BaseTransferTask* task = it->second;
        if (task == NULL) {
            transfer_tasks_.erase(it++);
            continue;
        }
        
        if (task->state() != kTransferTaskStateWaitingUpload &&
            task->state() == kTransferTaskStateTransferDone) {
            long esp = time(NULL) - task->create_time();
            if (esp > ConfigUtil::GetInstance()->GetTaskTimeout()) {
                if (task->GetFromConn()) {
                    FileClientConn* conn = reinterpret_cast<FileClientConn*>(task->GetFromConn());
                    conn->ClearTransferTask();
                }
                if (task->GetToConn()) {
                    FileClientConn* conn = reinterpret_cast<FileClientConn*>(task->GetToConn());
                    conn->ClearTransferTask();
                }
				log(" delete task  before");
                delete task;
				log(" delete task  after ");
                transfer_tasks_.erase(it++);
                continue;
            }
        }
        
        ++it;
    }
}

BaseTransferTask* TransferTaskManager::NewTransferTask(uint32_t trans_mode, const std::string& task_id, uint32_t from_user_id, uint32_t to_user_id, const std::string& file_name, uint32_t file_size,const std::string& file_md5) {
    BaseTransferTask* transfer_task = NULL;
    
    TransferTaskMap::iterator it = transfer_tasks_.find(task_id);
    if (it==transfer_tasks_.end()) {             //˵???????? ??һ?ν?��
        if (trans_mode == IM::BaseDefine::FILE_TYPE_ONLINE) {
            transfer_task = new OnlineTransferTask(task_id, from_user_id, to_user_id, file_name, file_size,file_md5);
        } else if (trans_mode == IM::BaseDefine::FILE_TYPE_OFFLINE) {
            transfer_task = new OfflineTransferTask(task_id, from_user_id, to_user_id, file_name, file_size,file_md5);
        } else {
            log("Invalid trans_mode = %d", trans_mode);
        }
        
        if (transfer_task) {
            transfer_tasks_.insert(std::make_pair(task_id, transfer_task));
        }
    } else {
        log("Task existed by task_id=%s, why?????", task_id.c_str());
    }
    
    return transfer_task;
}

OfflineTransferTask* TransferTaskManager::NewTransferTask(const std::string& task_id, uint32_t to_user_id) {
    OfflineTransferTask* transfer_task = OfflineTransferTask::LoadFromDisk(task_id, to_user_id);
    if (transfer_task) {
        transfer_tasks_.insert(std::make_pair(task_id, transfer_task));
    }
    return transfer_task;
}

bool TransferTaskManager::DeleteTransferTaskByConnClose(const std::string& task_id) {
    bool rv = false;
    
    TransferTaskMap::iterator it = transfer_tasks_.find(task_id);
    if (it!=transfer_tasks_.end()) {
			log("has  find it ");
        BaseTransferTask* transfer_task = it->second;
        if (transfer_task->GetTransMode() == FILE_TYPE_ONLINE) {
            if (transfer_task->GetFromConn() == NULL && transfer_task->GetToConn() == NULL) {
                delete transfer_task;
                transfer_tasks_.erase(it);//  ???ߴ???????ɾ?????????󣬴??????б? ?Ƴ?
                rv = true;
            }
            //
        } else {
            if (transfer_task->state() != kTransferTaskStateWaitingUpload) {//?˴??о?Ӧ???? WaitingDownload 
                delete transfer_task;
                transfer_tasks_.erase(it);
                rv = true;
				log(" has delete transfer_task ok ");
            }
        }
        
        // delete  it->second;
        // transfer_tasks_.erase(it);
    }
    
    return rv;
}


bool TransferTaskManager::DeleteTransferTask(const std::string& task_id) {
    bool rv = false;
    
    TransferTaskMap::iterator it = transfer_tasks_.find(task_id);
    if (it!=transfer_tasks_.end()) {
        delete  it->second;
        transfer_tasks_.erase(it);
    }
    
    return rv;
}
