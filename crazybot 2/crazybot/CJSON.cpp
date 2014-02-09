#include "CJSON.h"

CJSON::CJSON()
{

}

CJSON::~CJSON()
{

}

Json::Value CJSON::Parse(std::string json)
{
	// I hope you have properly formatted JSON otherwise this will crash the bot
	Json::Value val;
	Json::Reader read;

	read.parse(json, val);
	return val;
}

std::string CJSON::Stringify(Json::Value data)
{
	Json::StyledWriter writer;
	std::string val;

	writer.write(data);

	return val;
}