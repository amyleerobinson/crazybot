#ifndef _CSOCKET_H_  // Basic include guard
#define _CSOCKET_H_

#include <string> // We all know string > char *
#include <map>
#include <vector>

#include <WinSock2.h> // Socket goodness!
// NOTE: This code doesn't support continuous connections yet
class CSocket // This class will handle all our communications using the socket interface. Yay!
{
	WSAData wsaData;
	// Not storing sockets anymore, you'll have to do that yourself!
public: // Enough private stuff!
	CSocket(); // Constructor!
	~CSocket(); // And a destructor
	SOCKET CreateSocket(int AddrFamily, int SocketType, int Protocol); // Our function for opening sockets
	bool DeleteSocket(SOCKET socketid); // After we opened our socket, something must be able to close it
	std::string SendData(SOCKET socketid,std::string address,USHORT port, std::string data); // A function for sending info to the other side!
};

#endif