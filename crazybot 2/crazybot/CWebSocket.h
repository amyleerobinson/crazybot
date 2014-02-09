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

// Include the websocket files
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

using namespace websocketpp;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

class CWebSocket // Our websocket client
{
	websocketpp::client<config::asio_client>* wsock;
	websocketpp::client<config::asio_client>::connection_ptr con_ptr;
	std::queue<std::string> msg_queue; // Received messages go here
	bool is_open;
	bool stay_open;
	std::mutex *mtx;

	std::thread *run_thread;

	// The message handler!
	void on_message(websocketpp::connection_hdl hdl, message_ptr msg);
	// The handler for websocket opening
	void on_open(connection_hdl hdl);
	// The function for the connection thread
	//void con_loop(std::string message);

	friend class CFramework;
public:
	CWebSocket();
	~CWebSocket();
	bool OpenSocket(std::string address);
	bool CloseSocket();
	bool SendMsg(std::string message);
	std::string GetNextMessage();
	std::mutex *GetMutex();
};

#endif