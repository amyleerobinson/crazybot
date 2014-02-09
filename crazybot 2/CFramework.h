#ifndef _CFRAMEWORK_H
#define _CFRAMEWORK_H

#include "crazybot/CJSON.h"
#include "crazybot/CSocket.h"
#include "crazybot/CWebSocket.h"

#include <fstream>

class CFramework // All bot operations will be handled by this class
{
	CJSON* JSON;
	CSocket* Socket;
	CWebSocket* WSocket;
	bool is_init;
	std::string auth_token;

	// Websocket section
	std::thread *ws_thread;
	client<config::asio_client>::connection_ptr con_ptr;
	bool keep_open;

	void ws_open_loop();

	// Framework section
	std::string Auth(std::string user, std::string pass);
public:
	CFramework();
	~CFramework();
	void Init();
};

#endif