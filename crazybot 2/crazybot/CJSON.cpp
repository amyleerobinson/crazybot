#include "CJSON.h"

CJSON::CJSON()
{

}

CJSON::CJSON()
{

}

Json::Value CJSON::Parse(std::string json)
{
	// I hope you have properly formatted JSON otherwise this will crash the bot
	reader.parse(json, root);
	return root;
}

std::string CJSON::Stringify(Json::Value data)
{
	std::string val;
	writer.write(data);

	return val;
}