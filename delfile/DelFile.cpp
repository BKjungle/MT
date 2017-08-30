#include "DelFile.h"



void del_file(const char* dirpath)
{
	
	int ret = -1;
	log("  dir path = %s",dirpath);
	string dirfile(dirpath);
	string file =  dirfile.substr(0,2).append("/").append(dirpath);
	log("del file is  %s ",file.c_str());
	if(0 ==  remove(file.c_str()))
		log("del file %s  ok",file.c_str());
	else
		log(" remove %s faile ",file.c_str());

						
}
