#include "Utils.h"

Json::Value Utils::APICall(std::string url, std::string extradata)
{
	CJSON *JSON = new CJSON();
	CSocket *Socket = new CSocket();
	std::string ip = "18.4.45.12";

	SOCKET sock = Socket->CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	std::string request;

	request += "GET /" + url + " HTTP/1.0\r\n";
	request += "Host: www.eyewire.org\r\n";
	request += "Connection: close\r\n";
	request += extradata;
	request += "\r\n";

	std::string response = Socket->SendData(sock, ip, 80, request);

	Socket->DeleteSocket(sock);

	int pos = response.find("Connection: close\r\n\r\n");
	response.erase(0, pos + strlen("Connection: close\r\n\r\n"));
	pos = response.find("{");
	response.erase(0, pos);

	Json::Value ret = JSON->Parse(response);

	SAFE_DELETE(JSON);
	SAFE_DELETE(Socket);

	return ret;
}