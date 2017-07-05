
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
int CRobotConn::analyse()
{	
	Json::Value  root;  
    Json::Reader reader;

    if (reader.parse(chunk.memory ,root))  // reader将Json字符串解析到root，root将包含Json里所有子元素  
    {  
        int code = root["code"].asInt();     
        log("   ^^^^^^^^^^^^----------------code = %d ", code );
        bool var = false;
        switch (code)
        {
        	case text:
        		var = ForText(&chunk.memory,root);
        		chunk.size = 0;
        		break;
        		
        	case url:
        		var = ForUrl(&chunk.memory,root);
        		chunk.size = 0;
  				break;
  				
  			case news:
  			case menu:
  				var = ForNewsMenu(&chunk.memory,root);
  				chunk.size = 0;
  				break;
  				
  			case poem:
  			case song:
  				var = ForPoemSong(&chunk.memory,root);
  				chunk.size = 0;
  				break;
  				
  			case 40007:
  				log("code 400007 Exception  ,%s ",root["text"].asString().c_str());
  				free(chunk.memory); //本次接受的数据清除，下次接受重新开辟空间
  				chunk.memory = NULL;
  				chunk.size = 0; 
  				return code;
  				
        	default:
        		log("code Exception %d ,%s ",code,root["text"].asString().c_str());
        		
        		
        }
        if(false == var)
        	{
        		log("organize  Res false ");
        		return -1;
        	}
    }else{
    	log("json parse failed "); 
    	return -3;
    }
 
	
	return 0;	
}


size_t CRobotConn::ResDate()
{
		if(NULL == chunk.memory)
			{
				log("post exceed and all is 40007 -************************************************");
				return 0 ;
			}
			
	
        //------------------------------------------------------------加密
		char* EnEnd ;
		uint32_t nOutLen;
		
		data.Encrypt((const char*)chunk.memory, (uint32_t)strlen(chunk.memory), &EnEnd, nOutLen);
        //------------------------------------------------------------------------组织并返回结果
		IM::Message::IMMsgData msgData;
			
		msgData.set_from_user_id(ID);
		msgData.set_to_session_id(fromid);
		msgData.set_msg_id(1);//必须设置, msg_server 否则解析失败 	
		msgData.set_create_time(get_tick_count());
		msgData.set_msg_type(IM::BaseDefine::MSG_TYPE_SINGLE_TEXT);		
		msgData.set_msg_data((const char *)EnEnd);//------------------------------------organize ok
			
		CImPdu msgData_pdu;
		
	    msgData_pdu.SetPBMsg(&msgData);
	    msgData_pdu.SetServiceId(IM::BaseDefine::SID_MSG);
	    msgData_pdu.SetCommandId(IM::BaseDefine::CID_MSG_DATA);
	    log(" msgData_pdu.GetLength() = %d ", msgData_pdu.GetLength());
	    
	    if(SOCKET_ERROR == send(m_handle, (char*)msgData_pdu.GetBuffer(), msgData_pdu.GetLength(), 0))
			if ((errno == EINPROGRESS) || (errno == EWOULDBLOCK))
			{
				log("socket send block fd=%d",m_handle);
			}
			else
			{
				log("!!!send failed, error code: %d", errno);
				return -2;
			}
			data.Free(EnEnd); 
    	  	
 return 0;
 
}

CURLcode CRobotConn::Post(const string & str)
{
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
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, str.c_str());  
	//  go
	res = curl_easy_perform(curl_handle);

	/* check for errors */ 
	if(res != CURLE_OK) {
	  log("curl_easy_perform())failed：%s \n", curl_easy_strerror(res));
	}
	else {
	 log("%lu bytes retrieval \n",(long)chunk.size);
	}

	
	curl_slist_free_all(plist);  //free  list 
	curl_easy_cleanup(curl_handle);
	
	return res;
}

void CRobotConn::Md5_Hex_Md5(unsigned char * d)
{
	char  key[1024] = {0};
	timestamp = to_string(time(NULL));
	//string timestamp = "1492650896";//把此时间固定 特定数据会加密错误
									// 进一步验证得： 有些时间戳对特定待数据  加密错误
	string keyParam = secret+timestamp+APIkey;
	CMd5::MD5_Calculate(keyParam.c_str(),keyParam.length(),key);//一次md5后转32位十六进制的字符串形式
	
	//--------------------------------------------仅md5
	string KKey(key);
	log(" KKKKKey   length  = %d ", KKey.length());
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, KKey.c_str(),  KKey.length());
    MD5_Final((unsigned char *)d, &ctx);		
}

void CRobotConn::EVP_aes_128_cbc_base64(unsigned char * key)
{
	
	Request_Enc.clear();

	int rv;

   	int outl, tmp, i;

   	EVP_CIPHER_CTX ctxx;

   	unsigned char iv[16] = {0};

   	OpenSSL_add_all_algorithms();

    EVP_CIPHER_CTX_init(&ctxx);

    rv = EVP_EncryptInit_ex(&ctxx, EVP_aes_128_cbc(),NULL, key, iv);
    if(rv != 1)
    {  
            log("EVP Encry Error");
            return ; 
    }  

   // unsigned char out[1024] = {0};
	unsigned char* out = new unsigned char[2048];

	memset(out,0,2048*sizeof(unsigned char)); 

    rv = EVP_EncryptUpdate(&ctxx, out, &outl, (unsigned char*)Request.c_str(), Request.length());
    if(rv != 1)
    {  
            log(" EVP_Updata Error");
            return ; 
    }  

    rv = EVP_EncryptFinal_ex(&ctxx, out + outl, &tmp);
    if(rv != 1)
    {  
            log(" EVP_EncryptFinal_ex Error");
            return ; 
    }  

    EVP_CIPHER_CTX_cleanup(&ctxx);

   	//------------------------------------------------------EVPEND
   	string strOut((const char * )out);// add 7.4
   	log("out = %s ", out);
   	
    string tmpReq = base64_encode(strOut);// chg 7.4   原为： （const char*）out
    log(" tmpReq = %s ", tmpReq.c_str());
    Request_Enc = tmpReq;
    log("EVP  Error");
    delete [] out;  // chg  7.4
    log("Base end  Request_Enc = %s",Request_Enc.c_str());
    
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
        	log("CMD:  CID_MSG_DATA req");
            SendAndGet_Turing(pPdu);    
            break;
        case CID_MSG_DATA_ACK:
        	log("CMD: CID_MSG_DATA_ACK");
        	break;
        case CID_LOGIN_RES_USERLOGIN:
        	log("CMD: Login in ");
        	break;
        case CID_SWITCH_P2P_CMD:
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
	int ret = -2;
	int num = 0;
	
	
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
	
 
    chunk.size = 0;
    fromid = msg.from_user_id();
  
    chunk.memory = NULL;
	
	msgdataAck(msg.from_user_id(), msg_id);//--msgdataAck
	msgReadAck(msg.from_user_id(), msg_id);//--ReadAck
	
	char* ppOutData;//--解密Aes---注意要 free 
	uint32_t nOutLen;
	if(0 == data.Decrypt(msgdata.c_str(), msgdata.length(), &ppOutData, nOutLen))
		log("Decrypt OK  nOutLen = %d, %d ,%d ",nOutLen,ppOutData[0],ppOutData[nOutLen-1]);
	 
	for(int i =0;i<nOutLen;i++)//把数据中的 \r\n全部替换成空格，以免影响后续加密
	{
		if( 10 == ppOutData[i] || 13 == ppOutData[i])
			{
				ppOutData[i] = 32;
			}	
	}
	
	string infof(ppOutData);
	log("infof len =  %d, %d",infof.length(),infof.c_str()[infof.length()-1]);
	//-----------组织加密数据
	Request = "{\"key\":\""+APIkey+"\",\"info\":\""+infof+"\","+"\"userid\":\"" + string(Fromid) + "\"}";
	log("%d , %s ",Request.length(),Request.c_str());
	//---------Encrypt
	do
	{
		sleep(1);
		unsigned char  key[16];
		Md5_Hex_Md5(key);
		log(" md5_hex_md5 = %s ",key);

	   	//-----EVP
		EVP_aes_128_cbc_base64(key);
		
		//--设置json
		Json::Value person;
		person["key"] = Json::Value(APIkey);
		person["timestamp"] = Json::Value(timestamp);
	 	person["data"] =Json::Value(Request_Enc); 
		std::string jsonstr = writer.write(person);
		log("-----req  json =  %s",jsonstr.c_str());

		//------Libcurl begin
	
		Post((const string )jsonstr);
		ret =  analyse();
		log("num ==  %d ~~~~~~",num);
	}
	while(ret == 40007 &&  ++num <= 3);
	
	ResDate();
//----------------------------------------------------------libcurl End
	if(chunk.memory != NULL)
		free(chunk.memory);
	data.Free(ppOutData);

}



