#ifndef _CMSGPROC_H
#define _CMSGPROC_H

#include "CJSON.h"
#include "Utils.h"

#include <queue>
#include <string>
#include <fstream>
#include <iterator>
#include <regex>

class CMsgProc
{
	CJSON* JSON;
	std::queue<std::string> msg_queue;
public:
	CMsgProc();
	~CMsgProc();
	bool ProcessMsg(Json::Value Message);
	void PublicMessage(std::string msg);
	void PrivateMessage(std::string Receiver, std::string msg);
	std::string GetNextReply();
};

#endif
