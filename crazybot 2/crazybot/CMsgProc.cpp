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
	std::ofstream chatlog("config/chat.log", std::ios::app );
	std::string usrdata((std::istreambuf_iterator<char>(usrfile)), std::istreambuf_iterator<char>());
	Json::Reader reader;
	Json::Value usrdb;
	reader.parse(usrdata, usrdb);

	// Get time of the message
	time_t *rawCurTime = new time_t;
	time(rawCurTime);
	tm *curTime = localtime(rawCurTime);

	usrfile.close();

	std::string MsgType = Message["cmd"].asString();
	if (MsgType == "msg")
	{
		std::string Sender = Message["origin"]["username"].asString();
		std::string Msg = Message["params"]["msg"].asString();
		std::string Scope = Message["params"]["scope"].asString();

		if (usrdb[Sender]["usr"].asString() == "" && Sender != "")
		{
			std::cout << "New user added to DB: " << Sender << std::endl;
			usrdb[Sender]["usr"] = Sender;
			usrdb[Sender]["uid"] = Message["origin"]["id"];
		}

		if (Msg == "forceupdate accuracy" && Sender == "crazyman4865")
		{
			msg_queue.push("ACCURACY_UPDATE");
			PublicMessage( Sender + " forced accuracy stats fetch. Getting latest data...");
		}

		// Chat logging

		if (Scope == "global")
		{
			// Timestamp of message
			std::string timestamp = "[";
			timestamp += ctime(rawCurTime);
			timestamp.erase(timestamp.length() - 1, 1);
			timestamp += "]";

			// Reconstruct the message
			std::string chatmsg = "<" + Sender + "> " + Msg;

			// Write message to log
			chatlog << timestamp << " " << chatmsg << '\n';
		}

		// Command processing
		std::vector<std::string> args;

		// Sample message format
		//registerRace 2014-02-17-00-00-00 2014-02-18-00-00-00 points,cubes "Cube Race"

		int start = Msg.find("\"");
		int end;
		std::string name;
		if (start != std::string::npos)
			end = Msg.find("\"", start + 1);
		if (end != std::string::npos)
			name = Msg.substr(start + 1, end-start-1);

		// Split the message
		args = Utils::split(Msg, ' ');
		if (args.size() == 0)
			return false;

		if ((Message["origin"]["rank"].asInt() == 9 || Sender == "crazyman4865" ) && args[0]=="registerRace")
		{
			Json::Value newrace;
			bool points = false;
			bool cubes = false;
			// Our "short" regex pattern to match YYYY-MM-DD-HH-MM-SS
			std::regex pattern("201[4-9]-([0][1-9]|[1][0-2])-([0-2][1-9]|3[01])-([01][0-9]|2[0-3])-[0-5][0-9]-[0-5][0-9]");
			if (std::regex_match(args[1], pattern) && std::regex_match(args[2],pattern))
			{
				// Split the date into separate parts
				std::vector<std::string> startDate = Utils::split(args[1], '-');
				std::vector<std::string> endDate = Utils::split(args[2], '-');

				// time_t - stores time in seconds from Jan 1, 1970
				time_t rawtime1, rawtime2;
				// tm - struct for easier management of date and time
				tm *startTime = new tm, *endTime = new tm;
				// fill in the tm structures
				startTime->tm_year = Utils::toInt(startDate[0])-1900; startTime->tm_mon = Utils::toInt(startDate[1])-1;
				startTime->tm_mday = Utils::toInt(startDate[2]); startTime->tm_hour = Utils::toInt(startDate[3]);
				startTime->tm_min = Utils::toInt(startDate[4]); startTime->tm_sec = Utils::toInt(startDate[5]);
				
				endTime->tm_year = Utils::toInt(endDate[0])-1900; endTime->tm_mon = Utils::toInt(endDate[1])-1;
				endTime->tm_mday = Utils::toInt(endDate[2]); endTime->tm_hour = Utils::toInt(endDate[3]);
				endTime->tm_min = Utils::toInt(endDate[4]); endTime->tm_sec = Utils::toInt(endDate[5]);

				// Create the raw time from the structs above
				rawtime1 = mktime(startTime);
				rawtime2 = mktime(endTime);

				// Is the start time earlier than the end time?
				if (rawtime1 < rawtime2)
				{
					// Start filling in the JSON descriptor of the race
					newrace["times"]["starttime"] = args[1];
					newrace["times"]["endtime"] = args[2];

					bool track = false;

					// Figure out what we're tracking
					if (args[3].find("points") != std::string::npos)
					{
						newrace["track"]["points"] = true;
						track = true;
					}
					if (args[3].find("cubes") != std::string::npos)
					{
						newrace["track"]["cubes"] = true;
						track = true;
					}

					if (!track)
						PrivateMessage(Sender, "Error: Invalid tracking data!");
					else
					{
						newrace["name"] = name;

						Json::Value racefile = JSON->Parse(Utils::ReadFile("config/races.txt"));
						newrace["id"] = racefile["highraceid"].asInt();
						newrace["finished"] = false;
						newrace["started"] = false;
						racefile["highraceid"] = racefile["highraceid"].asInt() + 1;
						racefile["races"].append(newrace);
						
						Json::FastWriter writer;

						std::string out = writer.write(racefile);

						std::ofstream fileo("config/races.txt", std::ios::trunc);
						fileo << out;
						fileo.close();

						char buf[150];
						std::strftime(buf, 150, "%c", startTime);
						
						std::string startString = buf;
						std::strftime(buf, 150, "%c", endTime);
						std::string endString = buf;

						std::string msg = "New race \"" + name + "\" added. Starts at " + startString + ".";

						PrivateMessage(Sender, msg);
						Sleep(500);
						msg = "Ends at " + endString + ". Will track " + args[3] + ".";
						PrivateMessage(Sender, msg);
					}
				}
				else
					PrivateMessage(Sender, "Error: End date is earlier than start date!");
				delete startTime; delete endTime;
			}
			else
				PrivateMessage(Sender, "Error: Invalid date format. Expected YYYY-MM-DD-HH-MM-SS.");
		}
	}
	else if (MsgType == "alert")
	{
		std::string Sender = Message["params"]["attr"]["username"].asString();
		std::string SubType = Message["params"]["type"].asString();

		if ((SubType == "award" || SubType == "connect" || SubType == "disconnect") && usrdb[Sender]["usr"].asString() == "" && Sender != "")
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

	chatlog.close();

	SAFE_DELETE(rawCurTime);

	return true;
}

void CMsgProc::PublicMessage(std::string msg)
{
	msg_queue.push("{\"cmd\":\"talk\",\"params\":[\"" + msg + "\"]}");
}

void CMsgProc::PrivateMessage(std::string Receiver, std::string msg)
{
	msg_queue.push("{\"cmd\":\"talk\",\"params\":[\"/msg " + Receiver + " " + msg + "\"]}");
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
