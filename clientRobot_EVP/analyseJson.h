#ifndef _ANA_JSON_H_
#define _ANA_JSON_H_

#include <json/json.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "slog/slog_api.h"

enum JsonType{ 
	text = 100000,
	url  = 200000,
	news = 302000,
	menu = 308000,
	song = 313000,
	poem = 314000
	
};
enum ErrorCode{
	keyfault = 40001,
	nullinfo = 40002,
	numover  = 40004,
	fmterror = 40007
};

bool ForText(char ** str,Json::Value & root);
bool ForUrl(char ** str,Json::Value & root);
bool ForNewsMenu(char ** str,Json::Value & root);
bool ForMenu(char ** str,Json::Value & root);
bool ForPoemSong(char ** str,Json::Value & root);
#endif