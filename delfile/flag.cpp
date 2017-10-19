#include <iostream>
#include <sstream>
#include <stdio.h>  
#include <signal.h>  
#include <unistd.h>  
#include <mysql.h>
#include "ConfigFileReader.h"
#include "DBPool.h"
#include "DelFile.h"

#define mpath  "./offline_file"


using namespace std;

//=====================
MYSQL* 		m_mysql;
int flag  = 0;
bool pDBInit(const char* db_host,unsigned int db_port,const char* db_dbname,
				const char* db_username,const char *db_password );
void timer(int sig)  
{  
		log(" in timer ");
        if(SIGALRM == sig)  
        {  
            log("timer\n");  
            alarm(3);       //we continue set the timer  
            if(1 == flag)
            	{
            		log("del server is busy ");
            		return ;
            	}
            else 
            	flag = 1;
        // -- STR_QUERY
        
        if(0 != chdir(mpath))			// 进入目录
		{
			log(" chdir failed ");
			return ;
		}
        
        time_t timer =  time(NULL);		//时间
        time_t flagtime = timer - 7 * 3600 * 24 ;
        stringstream stream ; 
        string str_flagtime;
        stream << flagtime;
        stream >> str_flagtime;
        string sql_query = "select fileName,taskId from IMTransmitFile where status=0 and created<="+ str_flagtime+ ";";
        //----QUERY
        mysql_ping(m_mysql);

		if (mysql_real_query(m_mysql, sql_query.c_str(), sql_query.length())) {	// 执行
			log("mysql_real_query failed: %s, sql: %s", mysql_error(m_mysql), sql_query.c_str());
			return ;
		}
		
		MYSQL_RES* res = mysql_store_result(m_mysql);	// 取回结果
		if (!res) {
			log("mysql_store_result failed: %s", mysql_error(m_mysql));
			return ;
		}
		
		CResultSet* result_set = new CResultSet(res);
		
	
		if(NULL != result_set )
			{
				
				while(result_set->Next())
				{
					log("7-------------%s",result_set->GetString("taskId"));					
					del_file(result_set->GetString("taskId"));
					log("delete file %s ok ",result_set->GetString("fileName"));
				}
				delete result_set;  // contain mysql_free_result();
				log("-------------88");
			}
        
        }  
  	
  		
  		if(0 != chdir(".."))			// quit目录
		{
			log(" Quit dir failed ");
			return ;
		}
		
  		flag = 0;
		log("flag = 0 ");
        return ;  
}  
  
int main()  
{  
		log(" exe is start----------------  ");
        signal(SIGALRM, timer); //relate the signal and function  
        
		log(" exe is start---------------1  ");
        //mysql 
        CConfigFileReader config_file("del_fileserver.conf");
 
        char* db_host = config_file.GetConfigName("host");
		char* str_db_port = config_file.GetConfigName("port");
		char* db_dbname = config_file.GetConfigName("dbname");
		char* db_username = config_file.GetConfigName("username");
		char* db_password = config_file.GetConfigName("password");

		log(" exe is start---------------2 ");

		if (!db_host || !str_db_port || !db_dbname || !db_username || !db_password ) {
			log("not configure db info  instance: %s", db_dbname);
			return 2;
		}
  		
		log(" exe is start---------------3  ");
  		unsigned int db_port = atoi(str_db_port);
  		if (!pDBInit(db_host , db_port ,db_dbname ,db_username ,db_password)) {//链接指定mysql数据库
			log("init db instance failed: %s");
			return 3;
		}
  		
  		
		log(" exe is start---------------4  ");
  		
        alarm(3);       //trigger the timer  
		log(" after alarm ");
		while(1)
{
 		sleep(10);
}
	 
//        getchar();  
  
		log("return ");
        return 0;  
}  


bool pDBInit(const char* db_host,unsigned int db_port,const char* db_dbname,
				const char* db_username,const char *db_password )
{
	
		log(" pDBInit  start---------------1  ");
	m_mysql = mysql_init(NULL);
	if (!m_mysql) {
		log("mysql_init failed");
		return false;
	}

		log(" pDBInit  start---------------2  ");
	my_bool reconnect = true;
	mysql_options(m_mysql, MYSQL_OPT_RECONNECT, &reconnect);
		log(" pDBInit  start---------------3  ");
	mysql_options(m_mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");

		log(" pDBInit  start--------------4 ");
	if (!mysql_real_connect(m_mysql,db_host, db_username,db_password,db_dbname, db_port, NULL, 0)) {
		log("mysql_real_connect failed: %s", mysql_error(m_mysql));
		return false;
	}


		log(" pDBInit  start---------------5  ");
	
	return true;
}
