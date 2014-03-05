#include "CWebSocket.h"

CWebSocket::CWebSocket()
{
	is_open = false;
	stay_open = true;
	wsock = new client<config::asio_client>;
	mtx = new std::mutex;
	run_thread = NULL;
	is_init = false;
}

CWebSocket::~CWebSocket()
{
	SAFE_DELETE(wsock);
	SAFE_DELETE(mtx);
	if (run_thread)
		run_thread->join();
	SAFE_DELETE(run_thread);
}

void CWebSocket::on_open( connection_hdl hdl)
{
	con_ptr = wsock->get_con_from_hdl(hdl);
}

void CWebSocket::on_message(websocketpp::connection_hdl hdl, message_ptr msg)
{
	std::string message = msg->get_payload();

	mtx->lock(); // We don't want to cause a data race in our message queue
	msg_queue.push(message);
	mtx->unlock();

	if (!stay_open)
		wsock->stop();
}

/*void CWebSocket::con_loop(std::string address)
{
	websocketpp::lib::error_code ec;
	websocketpp::client<config::asio_client>::connection_ptr con = wsock->get_connection(address, ec);

	std::this_thread::sleep_for(std::chrono::seconds(10));

	while (stay_open) // Connection loop
	{
		wsock->connect(con);
		wsock->run();
		wsock->reset();
	}
}*/

void CWebSocket::Init()
{
	// Clear channels
	wsock->clear_access_channels(websocketpp::log::alevel::all);
	wsock->clear_error_channels(websocketpp::log::elevel::all);

	// Init
	wsock->init_asio();

	// Set handlers
	wsock->set_message_handler(websocketpp::lib::bind(&CWebSocket::on_message, this, ::_1, ::_2));
	wsock->set_open_handler(websocketpp::lib::bind(&CWebSocket::on_open, this, ::_1));

	is_init = true;
}

bool CWebSocket::OpenSocket(std::string address)
{
	if (is_open || !is_init)
		return false;

	stay_open = true;

	// Run the WebSocket connection
	websocketpp::lib::error_code ec;
	websocketpp::client<config::asio_client>::connection_ptr con = wsock->get_connection(address, ec);

	is_open = true;
	wsock->connect(con);
	wsock->run();
	wsock->reset();
	is_open = false;

	return true;
}

bool CWebSocket::CloseSocket()
{
	if (is_open)
	{
		stay_open = false;
		return true;
	}
	return false;
}

bool CWebSocket::SendMsg(std::string message)
{
	if (!is_open)
		return false;

	websocketpp::lib::error_code ec;

	wsock->send(con_ptr, message, websocketpp::frame::opcode::text, ec);

	return true;
}

std::string CWebSocket::GetNextMessage()
{
	mtx->lock(); // We want to be sure nothing else is using the queue

	if (msg_queue.empty())
	{
		mtx->unlock();
		return "";
	}
	
	std::string msg = msg_queue.front(); // Fetch the message on the front of the queue
	msg_queue.pop();
	mtx->unlock();

	return msg;
}

std::mutex *CWebSocket::GetMutex()
{
	return mtx;
}

CWebSocketServer::CWebSocketServer()
{
	wsocksrv = new server<config::asio>;
	mtx = new std::mutex;
	is_listening = false;
	con_count = 0;
}

CWebSocketServer::~CWebSocketServer()
{
	SAFE_DELETE(wsocksrv);
	SAFE_DELETE(mtx);
}

void CWebSocketServer::on_open(connection_hdl hdl)
{
	WSSConData newCon;
	newCon.Con = wsocksrv->get_con_from_hdl(hdl);
	
	con_count++;

	open_connections.push_back(newCon);
}

void CWebSocketServer::on_close(connection_hdl hdl)
{
	for (int i = 0; i < open_connections.size(); i++)
	{
		if (open_connections[i].Con == wsocksrv->get_con_from_hdl(hdl))
			open_connections.erase(open_connections.begin()+i);
	}

	con_count--;
}

void CWebSocketServer::on_message(websocketpp::connection_hdl hdl, srv_message_ptr msg)
{
	WSSMsgData Msg_Data;

	Msg_Data.Message = msg->get_payload();
	Msg_Data.Con_Sender = wsocksrv->get_con_from_hdl(hdl);

	msg_queue.push(Msg_Data);
}

void CWebSocketServer::Init()
{
	if (is_listening)
		return;

	try {
		wsocksrv->clear_access_channels(websocketpp::log::alevel::all);
		wsocksrv->clear_error_channels(websocketpp::log::elevel::all);

		wsocksrv->init_asio();

		wsocksrv->set_message_handler(websocketpp::lib::bind(&CWebSocketServer::on_message, this, ::_1, ::_2));
		wsocksrv->set_open_handler(websocketpp::lib::bind(&CWebSocketServer::on_open, this, ::_1));
		wsocksrv->set_close_handler(websocketpp::lib::bind(&CWebSocketServer::on_close, this, ::_1));

		wsocksrv->listen(27014);
		wsocksrv->start_accept();

		std::cout << "WebSocket server started on port 27014" << std::endl;
		is_listening = true;
		wsocksrv->run();

		std::cout << "WebSocket server closed!" << std::endl;

		wsocksrv->reset();

		is_listening = false;
	}
	catch (std::exception &e)
	{
		std::cout << "WebSocket Server Init Error: " << e.what() << std::endl;
	}
	catch (websocketpp::lib::error_code &ec)
	{
		std::cout << "WebSocket Server Init Error: " << ec.message() << std::endl;
	}
	catch (...)
	{
		std::cout << "WebSocket Server Init Error: Unknown Error" << std::endl;
	}
}

bool CWebSocketServer::SendMsg(std::string message, std::string recipient)
{
	if (!recipient.empty())
	{
		for (int i = 0; i < open_connections.size(); i++)
		{
			if (open_connections[i].Data["user"].asString() == recipient)
			{
				websocketpp::lib::error_code ec;

				wsocksrv->send(open_connections[i].Con_Hdl, message, websocketpp::frame::opcode::text, ec);
				return true;
			}
		}
		return false;
	}
	else
	{
		for (int i = 0; i < open_connections.size(); i++)
		{
			websocketpp::lib::error_code ec;

			wsocksrv->send(open_connections[i].Con_Hdl, message, websocketpp::frame::opcode::text, ec);
		}
		return true;
	}
}

Json::Value CWebSocketServer::GetUsrData(std::string user)
{
	Json::Value usr_data;

	for (auto i : open_connections)
	{
		if (i.Data["user"].asString() == user)
		{
			usr_data = i.Data;
			break;
		}
	}

	return usr_data;
}

WSSMsgData CWebSocketServer::GetNextMessage()
{
	WSSMsgData NextMsg;
	NextMsg.Message = "";
	if (msg_queue.empty())
		return NextMsg;
	else
	{
		NextMsg = msg_queue.front();
		msg_queue.pop();

		return NextMsg;
	}
}