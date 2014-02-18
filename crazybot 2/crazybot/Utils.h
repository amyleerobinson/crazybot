#ifndef _UTILS_H
#define _UTILS_H

#include "CJSON.h"
#include "CSocket.h"

#include <vector>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <fstream>

// Some useful macros
#define SAFE_DELETE(x) if((x)) {delete (x); (x)=NULL;}
#define SAFE_DELETE_ARRAY(x) if((x)) {delete[] (x); (x)=NULL;}

namespace Utils
{
	Json::Value APICall(std::string url, std::string extradata = "\r\n", std::string method = "GET");
	// Split a string into multiple strings using a delimeter
	std::vector<std::string> split(const std::string &s, char delim);
	// Create a date in YYYY-MM-DD from raw time
	std::string MakeDate(time_t timer);
	// Converts a string to integer
	int toInt(std::string str);
	// Converts an integer to string;
	std::string toString(int num);
	// Reads the full contents of a file
	std::string ReadFile(std::string filename);
}

#endif