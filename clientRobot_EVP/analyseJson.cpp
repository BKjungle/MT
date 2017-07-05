#include "analyseJson.h"

using namespace std;

bool ForText(char ** str,Json::Value & root)
{	
	free(*str);//add 4.27
	string text = root["text"].asString();
	*str = (char*) malloc(text.length()+1 );
	if( NULL == *str)
		{
			log("malloc faile ");
			return false;
		}
	memset(*str,0,text.length()+1);
 	memcpy(*str,text.c_str(),text.length());
 	  log("  Text  ok");
 	return true;
}
bool ForUrl(char ** str,Json::Value & root)
{
	free(*str);//add 4.27
	string text = root["text"].asString();
	string url = root["url"].asString();  
	string all = text+"\n"+url;
	*str= (char*) malloc(all.length()+1 );//kai内存
	if( NULL == *str)
		{
			log("malloc faile ");
			return false;
		}
	memset(*str,0,all.length() +1);
	memcpy(*str,all.c_str(),all.length());
	log("  Url ok  ");
	return true;
}
bool ForNewsMenu(char ** str,Json::Value & root)
{	
	free(*str);//add 4.27
	char *temp1 = "article";
	if( menu == root["code"].asInt())
			temp1 = "name";
		
	string total = "" ;
	string text = root["text"].asString();
	Json::Value array = root["list"];
	int num = array.size();
	total += text + "\n" ;
	for(int i = 0;i<num;i++)
	{
		total += ("  "+ array[i][temp1].asString() + " 详情->" + \
		array[i]["detailurl"].asString()) ;
	}
	
	*str = (char *)malloc(total.length()+1);
	if(NULL == *str)
		{
			log("malloc faile ");
			return false;
		}
	memset(*str,0,total.length()+1);
	memcpy(*str,total.c_str(),total.length());
	log("NewsorMenu ok ");
	return true;
}
bool ForMenu(char ** str,Json::Value & root)
{
	return true;
}


bool ForPoemSong(char ** str,Json::Value & root)
{
	free(*str);//add 4.27
	string Waring("诗歌歌曲暂不支持的~");
	*str = (char *)malloc(Waring.length()+1);
	if(NULL == *str)
		{
			log("malloc faile ");
			return false;
		}
	memset(*str,0,Waring.length()+1);
	memcpy(*str,Waring.c_str(),Waring.length());
	log("PoemSong ok ");
	
	return true;
}