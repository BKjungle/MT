#ifndef __DEL__TASK__H__
#define __DEL__TASK__H__
#include<iostream>
#include<stdio.h> 
#include<stdlib.h> 
#include<dirent.h> 
#include<string.h> 
#include<sys/stat.h> 
#include "../base/slog/slog_api.h"
#include "../base/util.h"
using namespace std;

void del_file(const char * dirpath);

#endif 
