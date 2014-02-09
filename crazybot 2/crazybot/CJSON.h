#ifndef _CJSON_H
#define _CJSON_H

#include "json/json.h"

class CJSON // This class will be doing all our JSON work
{
	Json::Value root;
	Json::Reader reader;
	Json::StyledWriter writer;
public:
	CJSON();
	~CJSON();
	Json::Value Parse(std::string json);
	std::string Stringify(Json::Value data);
};

#endif