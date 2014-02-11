#include "CFramework.h"

CFramework::CFramework()
{
	is_init = false;
	Socket = new CSocket;
	JSON = new CJSON;
	WSocket = new CWebSocket;
	Stats = new CStats;
	ws_thread = NULL;
	keep_open = true;
}

CFramework::~CFramework()
{
	SAFE_DELETE(JSON);
	SAFE_DELETE(Socket);
	SAFE_DELETE(Stats);
	WSocket->CloseSocket();
	keep_open = false;
	ws_thread->join();
	SAFE_DELETE(WSocket);
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

	//Init stats updating
	Stats->InitAutoUpdaters();

	ws_thread = new std::thread(&CFramework::ws_open_loop, this);

	std::this_thread::sleep_for(std::chrono::seconds(8)); // Wait until WebSocket opens

	WSocket->SendMsg("{\"cmd\":\"talk\",\"params\":[\"/msg crazyman4865 hello :3\"]}");
}