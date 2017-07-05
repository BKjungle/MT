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
#include "Base64.h"
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <json/json.h>
#include <curl/curl.h>
using namespace std;

class CRobotConn : public CImConn
{
public:
	CRobotConn( string AesKey,char * _APIkey,char * _URL,int id,string _secret);
	virtual ~CRobotConn();

	virtual void HandlePdu(CImPdu* pPdu);
	Json::Value root;
    Json::Value person;
    Json::FastWriter writer;
    CAes data; 
    string APIkey;
	string URL;
	string secret;
	int ID;
public:
	void SendAndGet_Turing(CImPdu* pPdu);
	void SwitchP2P(CImPdu* pPdu);
	void msgdataAck(uint32_t from_id,uint32_t msg_id);
	void msgReadAck(uint32_t from_id,uint32_t msg_id);
private:
	
};
struct MemoryStruct {
  char *memory;
  int handle;
  int fromid;
  size_t size;
  int Id;
};

#endif /* LOGINCONN_H_ */
