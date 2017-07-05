/* client.cpp */

//#include <doapi.h>
#include <base.h>

#include "omconn.h"
#include "IM.Login.pb.h"
#include "IM.Message.pb.h"
#include "IM.Other.pb.h"
#include "EncDec.h"
#include "slog/slog_api.h"
#include "CRobotConn.h"
#include "ConfigFileReader.h" 


using namespace std;
using namespace IM::BaseDefine;
void * StartRoutine(void* arg);

#define MAXLINE 2048
#define SERV_PORT 8000
#define SERVER_HEARTBEAT_INTERVAL  15000
#define SERVER_TIMEOUT  30000
#define OPEN_MAX 1
#define ID   8
#define wait_timeout  1000

long long m_last_send_tick = 0 ;
long long m_last_recv_tick = 0;
pthread_t		m_thread_id ;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int sockfd;

int   Ontime()
{
		log(" Ontime ");
		
		long long timer = get_tick_count();
		if (timer > m_last_send_tick + SERVER_HEARTBEAT_INTERVAL) {//心跳检测机制，5秒一次
            IM::Other::IMHeartBeat msg;
            CImPdu pdu;
            pdu.SetPBMsg(&msg);
            pdu.SetServiceId(SID_OTHER);
            pdu.SetCommandId(CID_OTHER_HEARTBEAT);
			m_last_send_tick = get_tick_count();//最后一次发送时间
		   
			int ret = send(sockfd, (char*)pdu.GetBuffer(), pdu.GetLength(), 0);
			if (ret == SOCKET_ERROR)
			{
				log("send error %d \n",ret);
				return -1;
			}
			 log(" send heartbeat ok");
		}
		if (timer> m_last_recv_tick + SERVER_TIMEOUT) {//监测链接，超过半分钟未收到回应--->close
			log("   m_last_recv_tick  timeout ");
			return -2;
		}
		
	return 0;
}

int main(int argc, char *argv[])
{
	struct sockaddr_in servaddr;
	
	char buf[MAXLINE];
	int  ret,m_epfd,flag = 1;
	int nodelay = 1;
	struct epoll_event ev,ep[OPEN_MAX];
	
	//-----------------------------conf
	
	CConfigFileReader config_file("robot.conf");

	char* Message_Key = config_file.GetConfigName("MessageKey");
	char* Api_Key = config_file.GetConfigName("ApiKey");
	char* Url = config_file.GetConfigName("Url");
	char* RobotidStr = config_file.GetConfigName("RobotId");
	char* Secret = config_file.GetConfigName("Secret");
	int robotid  = atoi(RobotidStr); 
	
	//--------------------------------struct 
	struct Conf conf = 
	{
		.fd = -1,
		.robotid = robotid,
		.Message_Key = Message_Key,
		.Api_Key  = Api_Key,
		.Url = Url,
		.Secret = Secret,
	} ;
	
	//socke init-------------------->
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, "192.168.1.182", &servaddr.sin_addr);
	servaddr.sin_port = htons(SERV_PORT);
	
 	//_SetNoBlock------------------>
 	
	ret = fcntl(sockfd, F_SETFL, O_NONBLOCK | fcntl(sockfd, F_GETFL));
	if (ret == SOCKET_ERROR)
	{
		log("_SetNonblock failed, err_code=%d",ret);
		return SOCKET_ERROR;
	}
	
	//_SetNoDelay(m_socket)-------------------->
	
	ret = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&nodelay, sizeof(nodelay));//不使用Nagle算法　
	if (ret == SOCKET_ERROR)
	{
		log("_SetNoDelay failed, err_code=%d",ret);
		return SOCKET_ERROR;
	}
	
	//conn---------------->
	ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	if ( (ret == SOCKET_ERROR) && (!_IsBlock(errno)) )
	{	
		log("connect failed, err_code=%d", errno);
		close(sockfd);
		return errno;
	}
		
	//epoll init---------------------->
	m_epfd = epoll_create(1);
	if (m_epfd == -1)
	{
		log("epoll_create failed  %d",errno);
		close(sockfd);
		return errno;
	}

	ev.events = EPOLLIN | EPOLLOUT | EPOLLET| EPOLLPRI | EPOLLERR  |EPOLLRDHUP;
	ev.data.fd = sockfd;
	if (ret  = epoll_ctl(m_epfd, EPOLL_CTL_ADD, sockfd, &ev) )
	{
		log("epoll_ctl() failed, errno=%d", errno);
		close(sockfd);
		close(m_epfd);
		return 0;
	}
	
	curl_global_init(CURL_GLOBAL_ALL);   //全局一次
	writePid();                            //记录pid
	conf.fd = sockfd;
	//------------------------------------------------------------------------------
	while (1) {
		if(1 == epoll_wait(m_epfd, ep, 1, wait_timeout))
			{
         		//检测对端关闭
	            if (ep[0].events & EPOLLRDHUP)
	            {
	               log("this is  EPOLL RDHUP --------------");
	                 break;
	            }
				//读事件
				if (ep[0].events & EPOLLIN)
				{
					
					ret = pthread_create(&m_thread_id, NULL, StartRoutine, (void*)&conf);
					
					m_last_recv_tick = get_tick_count();
					log("elollin pthread  %d ",ret );
				}
				//写事件
				if (ep[0].events & EPOLLOUT)//触发完读又直接触发写事件
				{
					if(1 == flag)
						{
							IM::Login::IMLoginReq msg;//login
							msg.set_user_name("TTR");
							msg.set_password("a9ce994b8687f9139a78bed98e3940bd");
							msg.set_online_status(IM::BaseDefine::USER_STATUS_ONLINE);
							msg.set_client_type(IM::BaseDefine::CLIENT_TYPE_WINDOWS);
							msg.set_client_version("e.g WIN/2.2");
							
						    CImPdu pdu;
						    pdu.SetPBMsg(&msg);
						    pdu.SetServiceId(IM::BaseDefine::SID_LOGIN);
						    pdu.SetCommandId(IM::BaseDefine::CID_LOGIN_REQ_USERLOGIN);
							int num = send(sockfd, (char*)pdu.GetBuffer(), pdu.GetLength(), 0);
							log("epollout  num = %d\n",num);
							
							flag--;
							
						}		
				}

				if (ep[0].events & (EPOLLPRI | EPOLLERR | EPOLLHUP))
				{
					log("OnClose,EPOLLPRI | EPOLLERR | EPOLLHUP");
					break;
				}
			}
		Ontime();	
	}
	close(sockfd);
	close(m_epfd);
	pthread_mutex_destroy(&mutex);
	return 0;
}

void * StartRoutine(void* arg)
{
	
	Conf * tmp = (Conf*) arg;
	
	pthread_detach(pthread_self());
	log("-------------------------->>>>>>");
	CRobotConn rbt(tmp->Message_Key,tmp->Api_Key,tmp->Url,tmp->robotid,tmp->Secret);
	//CRobotConn rbt("12345678901234567890123456789012","b534d629502f45f2b4f474a2ec113673","http://www.tuling123.com/openapi/api",8,"e1e020e27b834b1a");
	rbt.OnConnect(tmp->fd);
	rbt.OnRead();	
	
}

