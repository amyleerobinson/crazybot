#include "CSocket.h"

// Time to add the code for all the functions we defined in the class!

CSocket::CSocket()
{
	WSAStartup(MAKEWORD(2, 2), &wsaData); // We'll use WinSock 2.2
}

CSocket::~CSocket()
{
	WSACleanup(); // We're done using WinSock
}

SOCKET CSocket::CreateSocket(int AddrFamily, int SocketType, int Protocol)
{
	SOCKET sock;

	sock = socket(AddrFamily, SocketType, Protocol); // Open the socket

	return sock; // You'll have to check if the socket has opened yourself
}

bool CSocket::DeleteSocket(SOCKET socketid)
{
	int retval = closesocket(socketid);
	return (retval ? true : false);
}

std::string CSocket::SendData(SOCKET socketid, std::string address, USHORT port, std::string data)
{
	sockaddr_in *serveraddr;

	serveraddr = new sockaddr_in;

	// Fill in connection info
	serveraddr->sin_family = AF_INET;
	serveraddr->sin_addr.s_addr = inet_addr(address.c_str());
	serveraddr->sin_port = htons(port);

	int result = connect(socketid, (sockaddr*)serveraddr, sizeof(*serveraddr)); // Attempt connection
	if (result == SOCKET_ERROR)
	{
		return ""; // Return nothing if connection failed
	}

	result = send(socketid, data.c_str(), data.length(), 0); // Send the data
	if (result < 0)
		return ""; // Send failed, return empty string

	std::string response = "";
	int response_len;
	char buffer[1000000]; // A one million character string response buffer
	do
	{
		response_len = recv(socketid, buffer, 1000000, 0);
		if (response_len > 0)
			response += std::string(buffer).substr(0, response_len);
		// NOTE: I hope you have fast internet and good connection since download lag
		// isn't handled here
	} while (response_len > 0);

	return response; // Return the valid response
}