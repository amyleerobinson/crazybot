#ifndef _UTILS_H
#define _UTILS_H

#include "CJSON.h"
#include "CSocket.h"

// Some useful macros
#define SAFE_DELETE(x) if((x)) {delete (x); (x)=NULL;}
#define SAFE_DELETE_ARRAY(x) if((x)) {delete[] (x); (x)=NULL;}

namespace Utils
{
	Json::Value APICall(std::string url, std::string extradata = "");
}

#endif