#include "DelFile.h"



bool del_file(const char* dirpath)
{
	
	bool ret = false;
	log("  dir path = %s",dirpath);
	string dirfile(dirpath);
	string file =  dirfile.substr(0,2).append("/").append(dirpath);
	log("del file is  %s ",file.c_str());
	if(0 ==  remove(file.c_str()))
		ret = true;
	else
		log(" remove %s faile ",file.c_str());
	return ret;
						
}
