// Don't make me explain all the basic libraries.
#include <iostream>

#include "crazybot/CSocket.h" // We're gonna need this!


// You should really know what this function is for if you plan to
// use/contribute to the code.
int main()
{
	// Time to test out a basic connection
	CSocket *sockmgr = new CSocket();
	SOCKET sock;
	std::string request;

	// Basic HTTP GET request to today's leaderboard
	request += "GET /api/v1/stats/top/user/by/points/per/day?from=0&to=0 HTTP/1.1\r\n";
	request += "Host: www.eyewire.org\r\n";
	request += "\r\n";

	sock = sockmgr->OpenSocket("18.4.45.12", 80); // Open the socket
	// Get the (quite lengthy) response
	std::string response = sockmgr->SendData(sock, request);
	std::cout << response << std::endl;

	std::cin.get();

	sockmgr->CloseSocket(sock); // Close our socket
	delete sockmgr; // Clean up
	return 0;
}