#ifndef _UTILS_H
#define _UTILS_H

#include "CJSON.h"
#include "CSocket.h"

#include <vector>
#include <string>
#include <sstream>
#include <ctime>

// Some useful macros
#define SAFE_DELETE(x) if((x)) {delete (x); (x)=NULL;}
#define SAFE_DELETE_ARRAY(x) if((x)) {delete[] (x); (x)=NULL;}

namespace Utils
{
	Json::Value APICall(std::string url, std::string extradata = "\r\n", std::string method = "GET");
	std::vector<std::string> split(const std::string &s, char delim);
	std::string MakeDate(time_t timer);
}

#endif