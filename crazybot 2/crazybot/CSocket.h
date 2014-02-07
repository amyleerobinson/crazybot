#ifndef _CSOCKET_H_  // Basic include guard
#define _CSOCKET_H_

#include <string> // We all know string > char *
#include <map>
#include <vector>

#include <WinSock2.h> // Socket goodness!

struct socket_data // Must store open socket data somewhere
{
	std::string ip;
	unsigned short port;
};

// NOTE: This code doesn't support continuous connections yet
class CSocket // This class will handle all our communications using the socket interface. Yay!
{
	WSAData wsaData;
	std::map<SOCKET, socket_data> socket_list; // We'll store open socket data in a map for fast lookup
	std::vector<SOCKET> socket_ids; // And in case you "forget" your socket ID, we'll store a master list here, just in case
public: // Enough private stuff!
	CSocket(); // Constructor!
	~CSocket(); // And a destructor
	SOCKET OpenSocket(std::string address, unsigned short port); // Our function for opening sockets
	bool CloseSocket(SOCKET socketid); // After we opened our socket, something must be able to close it
	std::string SendData(SOCKET socketid, std::string data); // A function for sending info to the other side!
	socket_data GetSocketData(SOCKET socketid); // If you need to know where you have connected to, ask this guy
	std::vector<SOCKET> GetSocketList(); // Lost socket IDs can be found here!
};

#endif