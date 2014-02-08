#include "CWebSocket.h"

CWebSocket::CWebSocket()
{
	is_open = false;
	stay_open = true;
	wsock = new client<config::asio_client>;
	mtx = new std::mutex;
}

CWebSocket::~CWebSocket()
{
	if (wsock)
		delete wsock;
	if (mtx)
		delete mtx;
}

void CWebSocket::on_open( connection_hdl hdl)
{
	//con_ptr = cl->get_con_from_hdl(hdl);
}

void CWebSocket::on_message(websocketpp::connection_hdl hdl, message_ptr msg)
{
	std::string message = msg->get_payload();

	mtx->lock(); // We don't want to cause a data race in our message queue
	msg_queue.push(message);
	mtx->unlock();
}

void CWebSocket::con_loop(std::string address)
{
	websocketpp::lib::error_code ec;
	websocketpp::client<config::asio_client>::connection_ptr con = wsock->get_connection(address, ec);

	while (stay_open) // Connection loop
	{
		wsock->connect(con);
		wsock->run();
		wsock->reset();
	}
}

bool CWebSocket::OpenSocket(std::string address)
{
	if (is_open)
		return false;


	// Clear channels
	wsock->clear_access_channels(websocketpp::log::alevel::all);
	wsock->clear_error_channels(websocketpp::log::elevel::all);

	// Init
	wsock->init_asio();

	// Set handlers
	wsock->set_message_handler(websocketpp::lib::bind(&CWebSocket::on_message,this,::_1,::_2));
	wsock->set_open_handler(websocketpp::lib::bind(&CWebSocket::on_open,this,::_1));

	stay_open = true;

	std::thread run_thread(&CWebSocket::con_loop, this, address);

	is_open = true;

	return true;
}

bool CWebSocket::CloseSocket()
{
	if (is_open)
	{
		stay_open = true;
		return true;
	}
	return false;
}

std::mutex *CWebSocket::GetMutex()
{
	return mtx;
}
