#ifndef _BASE_H_
#define _BASE_H_	

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>
#include <stdint.h>	
#include <signal.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <json/json.h>
#include <curl/curl.h>

struct Conf{
	int fd;
	int robotid;
	char* Message_Key ;
	char* Api_Key ;
	char* Url ;
	char* Secret ;
};
bool _IsBlock(int error_code)
	{
		return ( (error_code == EINPROGRESS) || (error_code == EWOULDBLOCK) );
	}



#endif
