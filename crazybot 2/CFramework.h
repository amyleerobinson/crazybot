#ifndef _CFRAMEWORK_H
#define _CFRAMEWORK_H

#include "crazybot/CJSON.h"
#include "crazybot/CSocket.h"
#include "crazybot/CWebSocket.h"
#include "crazybot/CStats.h"
#include "crazybot/CMsgProc.h"

#include <fstream>

class CFramework // All bot operations will be handled by this class
{
	CJSON* JSON;
	CSocket* Socket;
	CWebSocket* WSocket;
	CStats* Stats;
	CMsgProc* MsgProc;
	bool is_init;
	std::string auth_token;

	// Websocket section
	std::thread *ws_thread;
	std::thread *msg_thread;
	client<config::asio_client>::connection_ptr con_ptr;
	bool keep_open;

	void ws_open_loop();
	void msg_loop();

	// Framework section
	std::string Auth(std::string user, std::string pass);
	std::thread *acc_u_thread;
	std::thread *cube_u_thread;
	std::thread *points_u_thread;
	bool force_acc_update;

	void update_accuracy_loop();
	void update_cubes_loop();
	void update_points_loop();
public:
	CFramework();
	~CFramework();
	void Init();
};

#endif
