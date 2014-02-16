#include "CMsgProc.h"

CMsgProc::CMsgProc()
{
	JSON = new CJSON();
}

CMsgProc::~CMsgProc()
{
	SAFE_DELETE(JSON);
}

bool CMsgProc::ProcessMsg(Json::Value Message)
{
	std::ifstream usrfile("config/userdb.txt");
	std::string usrdata((std::istreambuf_iterator<char>(usrfile)), std::istreambuf_iterator<char>());
	Json::Reader reader;
	Json::Value usrdb;
	reader.parse(usrdata, usrdb);

	usrfile.close();

	std::string MsgType = Message["cmd"].asString();
	if (MsgType == "msg")
	{
		std::string Sender = Message["origin"]["username"].asString();
		std::string Msg = Message["params"]["msg"].asString();

		if (usrdb[Sender]["usr"].asString() == "")
		{
			std::cout << "New user added to DB: " << Sender << std::endl;
			usrdb[Sender]["usr"] = Sender;
			usrdb[Sender]["uid"] = Message["origin"]["id"];
		}

		if (Msg == "forceupdate accuracy" && Sender == "crazyman4865")
		{
			msg_queue.push("ACCURACY_UPDATE");
			msg_queue.push("{\"cmd\":\"talk\",\"params\":[\"" + Sender + " forced accuracy stats update. Starting update...\"]}");
		}
	}
	else if (MsgType == "alert")
	{
		std::string Sender = Message["params"]["attr"]["username"].asString();
		std::string SubType = Message["params"]["type"].asString();
		if ((SubType == "award" || SubType == "connect" || SubType == "disconnect") && usrdb[Sender]["usr"].asString() == "")
		{
			std::cout << "New user added to DB: " << Sender << std::endl;
			usrdb[Sender]["usr"] = Sender;
			usrdb[Sender]["uid"] = Message["params"]["attr"]["uid"];
		}
	}
	else
		return false;

	std::string fileoutput = JSON->Stringify(usrdb);

	std::ofstream usrfileo("config/userdb.txt", std::ios::trunc);
	usrfileo << fileoutput;
	usrfileo.close();

	return true;
}

std::string CMsgProc::GetNextReply()
{
	if (msg_queue.empty())
		return "";
	
	std::string msg = msg_queue.front();
	msg_queue.pop();

	return msg;
}

// {"cmd":"msg","params":{"msg":"sample msg","scope":"global"},"origin":{"id":12345,"username":"some_user","rank":1,"css":{"color":"#00FF00","text - shadow":"#00FF00 0px 0px 7px"}}}
// {"cmd":"alert", "params" : {"type":"award", "attr" : {"uid":"12345", "username" : "some_user","trailblazer":"0","points":"123","retroactive":"0","timestamp":"2014-02-16T07:19:36.542Z"}}}
// {"cmd":"alert","params":{"type":"connect","attr":{"uid":12345,"username":"some_user"}}}
