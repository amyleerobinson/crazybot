#include "CFramework.h"

CFramework::CFramework()
{
	is_init = false;
	Socket = new CSocket;
	JSON = new CJSON;
	WSocket = new CWebSocket;
	Stats = new CStats;
	MsgProc = new CMsgProc();
	ws_thread = NULL;
	msg_thread = NULL;
	acc_u_thread = NULL;
	cube_u_thread = NULL;
	points_u_thread = NULL;
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
	SAFE_DELETE(WSocket);
	SAFE_DELETE(MsgProc);
}

void CFramework::ws_open_loop()
{
	char *buf = new char[256];
	std::string uid = auth_token.substr(0, 5);
	std::string token = auth_token.substr(8);
	std::string chat = "ws://eyewire.org/chat";

	sprintf(buf, "{\"uid\":%s,\"token\":\"%s\"}", uid.c_str(), token.c_str());

	while (keep_open)
	{
		std::thread thr(&CWebSocket::OpenSocket,WSocket,chat);

		// Leave some time for it to connect
		std::this_thread::sleep_for(std::chrono::seconds(5));
		// Send auth message
		WSocket->SendMsg(buf);

		thr.join(); // Wait for the socket to close
	}

	delete []buf;
}

void CFramework::msg_loop()
{
	std::thread *acc_update;

	while (keep_open)
	{
		std::string ws_message = WSocket->GetNextMessage();
		std::string proc_message = MsgProc->GetNextReply();
		if (ws_message != "")
			MsgProc->ProcessMsg(JSON->Parse(ws_message));
		if (proc_message != "")
		{
			if (proc_message != "ACCURACY_UPDATE")
				WSocket->SendMsg(proc_message);
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

	ws_thread = new std::thread(&CFramework::ws_open_loop, this);
	msg_thread = new std::thread(&CFramework::msg_loop, this);
	acc_u_thread = new std::thread(&CFramework::update_accuracy_loop, this);
	cube_u_thread = new std::thread(&CFramework::update_cubes_loop, this);
	points_u_thread = new std::thread(&CFramework::update_points_loop, this);

	std::this_thread::sleep_for(std::chrono::seconds(8)); // Wait until WebSocket opens

	WSocket->SendMsg("{\"cmd\":\"talk\",\"params\":[\"/msg crazyman4865 hello :3\"]}");
}
