#ifndef _CSOCKET_H_  // Basic include guard
#define _CSOCKET_H_

#include <string> // We all know string > char *
#include <map>
#include <vector>

struct socket_data // Must store open socket data somewhere
{
	std::string ip;
	std::string port;
};

class CSocket // This class will handle all our communications using the socket interface. Yay!
{
	std::map<int, socket_data> socket_list; // We'll store open socket data in a map for fast lookup
	std::vector<int> socket_ids; // And in case you "forget" your socket ID, we'll store a master list here, just in case
public: // Enough private stuff!
	CSocket(); // Constructor! No idea what we'll use it for yet
	int OpenSocket(std::string address, std::string port); // Our function for opening sockets
	bool CloseSocket(int socketid); // After we opened our socket, something must be able to close it
	std::string SendData(int socketid, std::string data); // A function for sending info to the other side!
	socket_data GetSocketData(int socketid); // If you need to know where you have connected to, ask this guy
	std::vector<int> GetSocketList(); // Lost socket IDs can be found here!
};

#endif