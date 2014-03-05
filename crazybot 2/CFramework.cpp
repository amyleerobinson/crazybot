#include "CFramework.h"

CFramework::CFramework()
{
	is_init = false;
	Socket = new CSocket;
	JSON = new CJSON;
	WSocket = new CWebSocket;
	WSServer = new CWebSocketServer;
	Stats = new CStats;
	MsgProc = new CMsgProc();
	ws_thread = NULL;
	msg_thread = NULL;
	acc_u_thread = NULL;
	cube_u_thread = NULL;
	points_u_thread = NULL;
	races_u_thread = NULL;
	srv_thread = NULL;
	keep_open = true;
	force_acc_update = false;
}

CFramework::~CFramework()
{
	SAFE_DELETE(JSON);
	SAFE_DELETE(Socket);
	SAFE_DELETE(Stats);
	WSocket->CloseSocket();
	keep_open = false;
	ws_thread->join();
	msg_thread->join();
	acc_u_thread->join();
	cube_u_thread->join();
	points_u_thread->join();
	races_u_thread->join();
	srv_thread->join();
	SAFE_DELETE(WSocket);
	SAFE_DELETE(WSServer);
	SAFE_DELETE(MsgProc);
}

void CFramework::ws_open_loop()
{
	char buf[256];
	std::string uid = auth_token.substr(0, 5);
	std::string token = auth_token.substr(8);
	std::string chat = "ws://eyewire.org/chat";

	sprintf_s(buf, "{\"uid\":%s,\"token\":\"%s\"}", uid.c_str(), token.c_str());

	while (keep_open)
	{
		std::thread thr(&CWebSocket::OpenSocket,WSocket,chat);

		// Leave some time for it to connect
		std::this_thread::sleep_for(std::chrono::seconds(5));
		// Send auth message
		WSocket->SendMsg(buf);

		thr.join(); // Wait for the socket to close
	}
}

void CFramework::msg_loop()
{
	while (keep_open)
	{
		std::string ws_message = WSocket->GetNextMessage();
		std::string proc_message = MsgProc->GetNextReply();
		if (ws_message != "")
			MsgProc->ProcessMsg(JSON->Parse(ws_message));
		if (proc_message != "")
		{
			if (proc_message != "ACCURACY_UPDATE")
			{
				WSocket->SendMsg(proc_message);
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
			else
				force_acc_update = true;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(150));
	}
}

void CFramework::update_accuracy_loop()
{
	while (keep_open)
	{
		std::this_thread::sleep_for(std::chrono::minutes(1));

		std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm * ptm = std::localtime(&tt);

		if (!(ptm->tm_hour == 6 && ptm->tm_min >= 50 && ptm->tm_min < 52) && !force_acc_update)
			continue;

		if ( !force_acc_update )
			WSocket->SendMsg("{\"cmd\":\"talk\",\"params\":[\"Starting scheduled accuracy stats fetch, getting latest data...\"]}");

		force_acc_update = false;

		Stats->UpdateAccuracy();
		WSocket->SendMsg("{\"cmd\":\"talk\",\"params\":[\"Accuracy stats fetch done. Results can be found at\"]}");
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		WSocket->SendMsg("{\"cmd\":\"talk\",\"params\":[\"http://crazyman4865.com/crazybot/accuracy.php\"]}");
	}
}

void CFramework::update_cubes_loop()
{
	while (keep_open)
	{
		std::this_thread::sleep_for(std::chrono::minutes(1));

		Stats->UpdateCubes();
	}
}

void CFramework::update_points_loop()
{
	while (keep_open)
	{
		std::this_thread::sleep_for(std::chrono::minutes(1));

		Stats->UpdatePoints();
	}
}

void CFramework::update_races_loop()
{
	while (keep_open)
	{
		std::this_thread::sleep_for(std::chrono::minutes(1));

		Stats->UpdateRaces();

		Json::Value races = JSON->Parse(Utils::ReadFile("config/races.txt"));

		time_t timer = time(NULL);
		timer -= (3600 * 8);
		unsigned short count = 0;

		for (auto race : races["races"])
		{
			// Split the date into separate parts

			std::vector<std::string> startDate = Utils::split(race["times"]["starttime"].asString(), '-');
			std::vector<std::string> endDate = Utils::split(race["times"]["endtime"].asString(), '-');

			// time_t - stores time in seconds from Jan 1, 1970
			time_t rawtime1, rawtime2;
			// tm - struct for easier management of date and time
			tm *startTime = new tm, *endTime = new tm;
			// fill in the tm structures
			startTime->tm_year = Utils::toInt(startDate[0]) - 1900; startTime->tm_mon = Utils::toInt(startDate[1]) - 1;
			startTime->tm_mday = Utils::toInt(startDate[2]); startTime->tm_hour = Utils::toInt(startDate[3]);
			startTime->tm_min = Utils::toInt(startDate[4]); startTime->tm_sec = Utils::toInt(startDate[5]);

			endTime->tm_year = Utils::toInt(endDate[0]) - 1900; endTime->tm_mon = Utils::toInt(endDate[1]) - 1;
			endTime->tm_mday = Utils::toInt(endDate[2]); endTime->tm_hour = Utils::toInt(endDate[3]);
			endTime->tm_min = Utils::toInt(endDate[4]); endTime->tm_sec = Utils::toInt(endDate[5]);

			// Create the raw time from the structs above
			rawtime1 = mktime(startTime);
			rawtime2 = mktime(endTime);

			if (timer > rawtime1 && !races["races"][count]["started"].asBool())
			{
				races["races"][count]["started"] = true;
				char buf[150];
				endTime->tm_hour += 1;
				std::strftime(buf, 150, "%c", endTime);
				std::string endString = buf;
				std::string tracking = "";
				if (race["track"]["cubes"].asBool())
					tracking += "cubes";
				if (race["track"]["points"].asBool())
					tracking += " and points";
				MsgProc->PublicMessage(race["name"].asString() + " just started. It will track " + tracking + " and will end on " + endString + " EST.");
				MsgProc->PublicMessage("Leaderboard for the race can be found at http://crazyman4865.com/crazybot/races.php?id=" + Utils::toString(race["id"].asInt()) + " (updates every minute)");
			}
			if (timer > rawtime2 && !races["races"][count]["finished"].asBool())
			{
				races["races"][count]["finished"] = true;
				MsgProc->PublicMessage(race["name"].asString() + " just ended. Leaderboard can be found at http://crazyman4865.com/crazybot/races.php?id=" + Utils::toString(race["id"].asInt()));
				MsgProc->PublicMessage("Retroactive points will be tracked over the course of the next hour.");
			}
			if (timer > rawtime2 + 3600 && !races["races"][count]["retrofinish"].asBool())
				races["races"][count]["retrofinish"] = true;
			count++;

			delete startTime; delete endTime;
		}

		Json::FastWriter writer;

		std::string out = writer.write(races);

		std::ofstream fileo("config/races.txt", std::ios::trunc);
		fileo << out;
		fileo.close();
	}
}

void CFramework::srv_run_loop()
{
	while (keep_open)
		WSServer->Init();
}

std::string CFramework::Auth(std::string user, std::string pass)
{
	SOCKET sock = Socket->CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	char *buf = new char[256];
	std::string request;

	// No peeking ;)
	sprintf(buf, "username=%s&password=%s", user.c_str(), pass.c_str());
	std::string str = buf;
	int len = strlen(buf);
	sprintf(buf, "Content-length: %d\r\n\r\n", len);

	request += "POST /1.0/internal/account/authenticate/standard/ HTTP/1.1\r\n";
	request += "Host: www.eyewire.org\r\n";
	request += "Content-type: application/x-www-form-urlencoded\r\n";
	request += buf;
	request += str;

	std::string resp = Socket->SendData(sock, "18.4.45.12", 80, request);

	int start = resp.find("Set-Cookie: authentication-token=") + strlen("Set-Cookie: authentication-token=");
	int end = resp.find(";", start);
	std::string auth_cookie = resp.substr(start, end - start);

	Socket->DeleteSocket(sock);
	delete [] buf;

	return auth_cookie;
}

void CFramework::Init()
{
	// Grab bot login info
	std::ifstream fstr;
	fstr.open("config/user.txt");

	char buf[256];
	fstr.getline(buf, 256);

	Json::Value val = JSON->Parse(buf);

	// Auth
	auth_token = Auth(val["usr"].asString(), val["pw"].asString());

	// Init WebSocket
	WSocket->Init();

	Stats->Init();

	// Did someone say threads?
	ws_thread = new std::thread(&CFramework::ws_open_loop, this);
	msg_thread = new std::thread(&CFramework::msg_loop, this);
	acc_u_thread = new std::thread(&CFramework::update_accuracy_loop, this);
	cube_u_thread = new std::thread(&CFramework::update_cubes_loop, this);
	points_u_thread = new std::thread(&CFramework::update_points_loop, this);
	races_u_thread = new std::thread(&CFramework::update_races_loop, this);
	srv_thread = new std::thread(&CFramework::srv_run_loop, this);

	std::this_thread::sleep_for(std::chrono::seconds(8)); // Wait until WebSocket opens

	WSocket->SendMsg("{\"cmd\":\"talk\",\"params\":[\"/msg crazyman4865 hello :3\"]}");
}
