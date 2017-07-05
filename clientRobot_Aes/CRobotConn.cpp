
#include "CRobotConn.h"
#include "IM.Other.pb.h"
#include "IM.Login.pb.h"
#include "IM.Message.pb.h"

#include "public_define.h"
#include "slog/slog_api.h"

#include "IM.Server.pb.h"
#include "IM.SwitchService.pb.h"
using namespace std;
using namespace IM::BaseDefine;

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	
	 size_t realsize = size * nmemb;
	 struct MemoryStruct *mem = (struct MemoryStruct *)userp;
	 
	 mem->memory =(char*) realloc(mem->memory, mem->size + realsize + 1);
	 if(mem->memory == NULL) {
	  
	   log("not enough memory (realloc returned NULL)");
	   return 0;
	 }
	 
	 memcpy(&(mem->memory[mem->size]), contents, realsize);
	 mem->size += realsize;
	 mem->memory[mem->size] = 0;
	 
	 return realsize; 
}

size_t ResDate(struct MemoryStruct * me,CAes * aes)
{

    Json::Value  root;  
    Json::Reader reader;
    if (reader.parse(me->memory ,root))  // reader将Json字符串解析到root，root将包含Json里所有子元素  
    {  
        int code = root["code"].asInt();     
        log("code = %d ", code );	
        bool var = false;
        switch (code)
        {
        	case text:
        		var = ForText(&me->memory,root);
        		break;
        	case url:
        		var = ForUrl(&me->memory,root);
  				break;
  			case news:
  			case menu:
  				var = ForNewsMenu(&me->memory,root);
  				break;
        	default:
        		log("code Exception %d ,%s ",code,root["text"].asString().c_str());
        	
        		goto End;
        }
        if(false == var)
        	{
        		log("organize  Res false ");
        		return -1;
        	}
        //------------------------------------------------------------加密
		char* EnEnd ;
		uint32_t nOutLen;
	
		aes->Encrypt((const char*)me->memory, (uint32_t)strlen(me->memory), &EnEnd, nOutLen);
        //------------------------------------------------------------------------组织并返回结果
		IM::Message::IMMsgData msgData;
			
		msgData.set_from_user_id(me->Id);
		msgData.set_to_session_id(me->fromid);
		msgData.set_msg_id(1);//必须设置, msg_server 否则解析失败 	
		msgData.set_create_time(get_tick_count());
		msgData.set_msg_type(IM::BaseDefine::MSG_TYPE_SINGLE_TEXT);		
		msgData.set_msg_data((const char *)EnEnd);//------------------------------------organize ok
			
		CImPdu msgData_pdu;
		
	    msgData_pdu.SetPBMsg(&msgData);
	    msgData_pdu.SetServiceId(IM::BaseDefine::SID_MSG);
	    msgData_pdu.SetCommandId(IM::BaseDefine::CID_MSG_DATA);
	    log(" msgData_pdu.GetLength() = %d ", msgData_pdu.GetLength());
	    
	    if(SOCKET_ERROR == send(me->handle, (char*)msgData_pdu.GetBuffer(), msgData_pdu.GetLength(), 0))
			if ((errno == EINPROGRESS) || (errno == EWOULDBLOCK))
			{
				log("socket send block fd=%d",me->handle);
			}
			else
			{
				log("!!!send failed, error code: %d", errno);
				return -2;
			}
			aes->Free(EnEnd); 
    }else{
    	log("json parse failed "); 
    	return -3;
    }
    	  	
End:
 return 0;
 
}


CRobotConn::CRobotConn( string AesKey,char * _APIkey,char * _URL,int id,string _secret):data(AesKey),APIkey(_APIkey),URL(_URL),ID(id),secret(_secret)
{
	
}

CRobotConn::~CRobotConn()
{
}

void CRobotConn::HandlePdu(CImPdu* pPdu)
{
	switch (pPdu->GetCommandId()) {
        case CID_OTHER_HEARTBEAT:
        	log("CMD: HeartBeat ");
            break;
        case CID_MSG_DATA:
        	log("CMD:  CID_MSG_DATA  goTuring begin");
            SendAndGet_Turing(pPdu);
            log("DoTuring  ok");      
            break;
        case CID_MSG_DATA_ACK:
        	log("CMD: CID_MSG_DATA_ACK");
        	break;
        case CID_LOGIN_RES_USERLOGIN:
        	log("CMD: Login in ");
        	break;
        case CID_SWITCH_P2P_CMD:
        	log("CMD: CID_SWITCH_P2P_CMD");
        	SwitchP2P(pPdu);
        	break;
        case CID_BUDDY_LIST_STATUS_NOTIFY:
        log("CID_BUDDY_LIST_STATUS_NOTIFY. ");
        	break;
        default:
            log("wrong msg, cmd id=%d ", pPdu->GetCommandId());
            break;
	}
}
void CRobotConn::msgdataAck(uint32_t from_id,uint32_t msg_id)
{
	IM::Message::IMMsgDataAck msgAck;
	msgAck.set_user_id(ID);
	msgAck.set_session_id(from_id);
	msgAck.set_msg_id(msg_id);
	msgAck.set_session_type(IM::BaseDefine::SESSION_TYPE_SINGLE);
	CImPdu msgDataAck_pdu;
	msgDataAck_pdu.SetPBMsg(&msgAck);
    msgDataAck_pdu.SetServiceId(IM::BaseDefine::SID_MSG);
    msgDataAck_pdu.SetCommandId(IM::BaseDefine::CID_MSG_DATA_ACK);
    	if(send(m_handle, (char*)msgDataAck_pdu.GetBuffer(), msgDataAck_pdu.GetLength(), 0))
    	log("send msgdataAck ok ");
}
void CRobotConn::msgReadAck(uint32_t from_id,uint32_t msg_id)
{
	IM::Message::IMMsgDataReadAck ReadAck;
	ReadAck.set_user_id(ID);
	ReadAck.set_session_id(from_id);
	ReadAck.set_msg_id(msg_id);
	ReadAck.set_session_type(IM::BaseDefine::SESSION_TYPE_SINGLE);
	CImPdu msgReadAck_pdu;
		
    msgReadAck_pdu.SetPBMsg(&ReadAck);
    msgReadAck_pdu.SetServiceId(IM::BaseDefine::SID_MSG);
    msgReadAck_pdu.SetCommandId(IM::BaseDefine::CID_MSG_READ_ACK);
    
    if(send(m_handle, (char*)msgReadAck_pdu.GetBuffer(), msgReadAck_pdu.GetLength(), 0))
    	log("send MsgReadAck ok ");

}
void CRobotConn::SwitchP2P(CImPdu* pPdu)
{
	IM::SwitchService::IMP2PCmdMsg msg;
    CHECK_PB_PARSE_MSG(msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()));
	string cmd_msg = msg.cmd_msg_data();
	uint32_t from_user_id = msg.from_user_id();
	uint32_t to_user_id = msg.to_user_id();
	

	log("HandleClientP2PCmdMsg, %u->%u, cmd_msg: %s ", from_user_id, to_user_id, cmd_msg.c_str());
}

void CRobotConn::SendAndGet_Turing(CImPdu* pPdu)
{
	IM::Message::IMMsgData msg;
     if (false == msg.ParseFromArray(pPdu->GetBodyData(), pPdu->GetBodyLength()))
	    {
	        log("parse pb msg failed.");
	        return;
	    }
	//----------------------------------------------------取得信息
	char Fromid[10]; 
	sprintf(Fromid,"%0d",msg.from_user_id());//转字符串
	if(IM::BaseDefine::MSG_TYPE_SINGLE_TEXT != msg.msg_type())//-非文本消息丢弃
		{
			log(" not MSG_TYPE_SINGLE_TEXT");
			return ;
		}
	uint32_t msg_id = msg.msg_id();
	const string msgdata =  msg.msg_data();
	
	msgdataAck(msg.from_user_id(), msg_id);//--msgdataAck
	msgReadAck(msg.from_user_id(), msg_id);//--ReadAck
	
	char* ppOutData;//--解密Aes---注意要 free 
	uint32_t nOutLen;
	if(0 == data.Decrypt(msgdata.c_str(), msgdata.length(), &ppOutData, nOutLen))
		log("Decrypt OK  nOutLen = %d,%s  ",nOutLen,ppOutData);
	
	for(int i =0;i<nOutLen;i++)
	{
		if( '\r' == ppOutData[i]  )
			{
				ppOutData[i] = 32;
				ppOutData[i+1] = 32;
			//	break;
			}	
	}
	
	string infof(ppOutData);
	log("infof len =  %d, %c",infof.length(),infof.c_str()[infof.length()-1]);
	//--------------------------------------------Encrypt
	
	char  key[1024] = {0};
	char *Aesdata;//--------------------free ！
	uint32_t AesOutlen; 
	string timestamp = "1492650896";//to_string(time(NULL));
	
	string keyParam = secret+timestamp+APIkey;
	CMd5::MD5_Calculate(keyParam.c_str(),keyParam.length(),key);//一次md5后转32位十六进制的字符串形式
	
	//--------------------------------------------仅md5
	string KKey(key);
	unsigned char  d[16];
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, KKey.c_str(),  KKey.length());
    MD5_Final((unsigned char *)d, &ctx);
	//---------------------------------------------------待加密数据
	string Sourceinfo = "{\"key\":\""+APIkey+"\",\"info\":\""+infof+"\","+"\"userid\":\"" + string(Fromid) + "\"}";

	log("%d , %s ",Sourceinfo.length(),Sourceinfo.c_str());
	//--------------------------------------------------扩内存  打padding
	int num = Sourceinfo.length() + (16 - (Sourceinfo.length()%16) );
	char *E_b = new char[num];
	memset(E_b,0,num);
	memcpy((void *)E_b,(const void * )Sourceinfo.c_str(),Sourceinfo.length());
	int iPaddings = (16 - (Sourceinfo.length()%16));
	memset(E_b+Sourceinfo.length(), iPaddings,iPaddings);//Padding
	int numm = num + 256;
	char *E_d = new char[numm];
	memset(E_d,0,numm);

	if(!aess_encrypt(( char *)E_b,(char*)d,( char *)E_d,num))  //-----加密
    {
    	log("encrypt error\n");
    	return ;
    }
    string dstString = base64_encode(E_d);
    log("Base end  dstString = %s",dstString.c_str());
	//----------------------------------------------------------设置json
	

	person["key"] = Json::Value(APIkey);
	person["timestamp"] = Json::Value(timestamp);
 	person["data"] =Json::Value(dstString);
	std::string json_file = writer.write(person);
	log("-------------------------%s",json_file.c_str());
//-------------------------------------------------------------Libcurl begin
	struct MemoryStruct chunk;
	
    chunk.handle = m_handle; 
    chunk.size = 0;
    chunk.fromid = msg.from_user_id(); 
    chunk.Id = ID;
    chunk.memory = NULL;
    //---------init
	CURL *curl_handle;
  	CURLcode res;
 
    curl_handle = curl_easy_init();
  	
	curl_easy_setopt(curl_handle, CURLOPT_URL,URL.c_str());

	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	curl_easy_setopt(curl_handle, CURLOPT_POST, 1);// set  post 请求 

	curl_easy_setopt(curl_handle, CURLOPT_HEADER, false);//set  输出不包含头
	curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT,10);// time out

	// 设置http发送的内容类型为JSON  
	curl_slist *plist = curl_slist_append(NULL,"Content-Type:application/json;charset=UTF-8");  
	
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, plist);  
	// 设置要POST的JSON数据  
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, json_file.c_str());  
	//  go
	res = curl_easy_perform(curl_handle);

	/* check for errors */ 
	if(res != CURLE_OK) {
	  log("curl_easy_perform())failed：%s \n", curl_easy_strerror(res));
	}
	else {
	 log("%lu bytes retrieval \n",(long)chunk.size);
	}
	///chunk.memory = buff;
	ResDate(&chunk,&data);//组织并返回
	///memset(buff,0,20480);
	///tmpsize = 0;
	curl_slist_free_all(plist);  //free  list 
	curl_easy_cleanup(curl_handle);
	
//----------------------------------------------------------libcurl End
	free(chunk.memory);
	free(E_b);
	free(E_d);	
	data.Free(ppOutData);
	ppOutData = NULL;
}



