#ifndef _CWEBSOCKET_H // The usual include guard
#define _CWEBSOCKET_H

#define NOMINMAX

// Enable some WebSocket++ C++11 support
#define _WEBSOCKETPP_CPP11_STL_
#define _WEBSOCKETPP_NOEXCEPT_TOKEN

#include <queue>
#include <string>
// Multithreading!
#include <thread>
#include <mutex>

#include "Utils.h"

// Include the websocket files
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/server.hpp>

using namespace websocketpp;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
typedef websocketpp::config::asio::message_type::ptr srv_message_ptr;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

class CWebSocket // Our websocket client
{
	websocketpp::client<config::asio_client>* wsock;
	websocketpp::client<config::asio_client>::connection_ptr con_ptr;
	std::queue<std::string> msg_queue; // Received messages go here
	bool is_open, stay_open, is_init;
	std::mutex *mtx;

	std::thread *run_thread;

	// The message handler!
	void on_message(websocketpp::connection_hdl hdl, message_ptr msg);
	// The handler for websocket opening
	void on_open(connection_hdl hdl);
	// The function for the connection thread
	//void con_loop(std::string message);
public:
	CWebSocket();
	~CWebSocket();
	void Init();
	bool OpenSocket(std::string address);
	bool CloseSocket();
	bool SendMsg(std::string message);
	std::string GetNextMessage();
	std::mutex *GetMutex();
};

struct WSSConData
{
	connection_hdl Con_Hdl;
	websocketpp::server<config::asio>::connection_ptr Con;
	Json::Value Data;
};

struct WSSMsgData
{
	websocketpp::server<config::asio>::connection_ptr Con_Sender;
	std::string Message;
};

class CWebSocketServer
{
	websocketpp::server<config::asio>* wsocksrv;
	std::vector<WSSConData> open_connections;
	std::queue<WSSMsgData> msg_queue; // Received messages go here

	std::mutex* mtx;

	bool is_listening;
	unsigned short con_count;

	// The message handler!
	void on_message(websocketpp::connection_hdl hdl, srv_message_ptr msg);
	// The handler for connection opening
	void on_open(connection_hdl hdl);
	// The handler for connection closing
	void on_close(connection_hdl hdl);

public:
	CWebSocketServer();
	~CWebSocketServer();
	void Init();
	bool SendMsg(std::string message, std::string recipient = "");
	Json::Value GetUsrData(std::string user);
	WSSMsgData GetNextMessage();
};

#endif