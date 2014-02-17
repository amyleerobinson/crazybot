#include "Utils.h"

Json::Value Utils::APICall(std::string url, std::string extradata, std::string method)
{
	CJSON *JSON = new CJSON();
	CSocket *Socket = new CSocket();
	std::string ip = "18.4.45.12";

	SOCKET sock = Socket->CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	std::string request;

	request += method + " /" + url + " HTTP/1.1\r\n";
	request += "Host: www.eyewire.org\r\n";
	request += "Connection: close\r\n";
	request += extradata;

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

std::vector<std::string> Utils::split(const std::string &s, char delim)
{
	std::vector<std::string> elems;
	std::stringstream ss(s);
	std::string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

std::string Utils::MakeDate(time_t timer)
{
	std::tm * ptm = std::localtime(&timer);

	std::string date;

	char *buf = new char[5];
	_itoa((ptm->tm_year + 1900), buf, 10);
	date = buf;
	_itoa((ptm->tm_mon + 1), buf, 10);
	if (strlen(buf) == 1)
		date += "-0";
	else
		date += "-";
	date += buf;
	_itoa((ptm->tm_mday), buf, 10);
	if (strlen(buf) == 1)
		date += "-0";
	else
		date += "-";
	date += buf;

	return date;
}