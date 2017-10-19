

#define LOG_MODULE_IM         "IM"
#include <iostream>
#include <string.h>
#include "slog_api.h"

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)
#define log(fmt, args...)  g_imlog.Info("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##args)
CSLog g_imlog = CSLog(LOG_MODULE_IM);
using namespace std;

int main()
{
	log(" this is a test for log tool");
	return 0;	
}
