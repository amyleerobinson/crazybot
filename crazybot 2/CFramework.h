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
	CWebSocketServer* WSServer;
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
	std::thread *races_u_thread;
	std::thread *srv_thread;
	bool force_acc_update;

	void update_accuracy_loop();
	void update_cubes_loop();
	void update_points_loop();
	void update_races_loop();
	void srv_run_loop();
public:
	CFramework();
	~CFramework();
	void Init();
};

#endif
