/*
 * RobotConn.h
 *
 *  Created on: 2017.4.6.12:50
 *      Author: xxx
 */

#ifndef ROBOTCONN_H_
#define ROBOTCONN_H_

#include "omconn.h"
#include "EncDec.h"
#include "analyseJson.h"
#include "enc.h"
#include "Base64_.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <json/json.h>
#include <curl/curl.h>
using namespace std;

struct MemoryStruct {
  char *memory;
  size_t size;
};

class CRobotConn : public CImConn
{
public:
	CRobotConn( string AesKey,char * _APIkey,char * _URL,int id,string _secret);
	virtual ~CRobotConn();

	virtual void HandlePdu(CImPdu* pPdu);
    
    Json::FastWriter writer;
    CAes data; 
    string APIkey;
	string URL;
	string secret;
	string timestamp;
	string Request;
	string Request_Enc;
	int ID;
	struct MemoryStruct chunk;
	int fromid;
	int msg_id;
public:
	void SendAndGet_Turing(CImPdu* pPdu);
	void SwitchP2P(CImPdu* pPdu);
	void msgdataAck(uint32_t from_id,uint32_t msg_id);
	void msgReadAck(uint32_t from_id,uint32_t msg_id);
	void Md5_Hex_Md5(unsigned char * d);
	void EVP_aes_128_cbc_base64(unsigned char * key);
	CURLcode Post(const string & str);
	int analyse();
	size_t ResDate();
	
private:
	
};


#endif /* LOGINCONN_H_ */
