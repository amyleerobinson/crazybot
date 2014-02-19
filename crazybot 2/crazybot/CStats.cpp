#include "CStats.h"

CStats::CStats()
{
	Socket = new CSocket();
	JSON = new CJSON();
}

CStats::~CStats()
{
	SAFE_DELETE(Socket);
	SAFE_DELETE(JSON);
}

void CStats::Init()
{
	std::ifstream openfile("config/cubes.txt");
	// Read previously stored data
	std::string cbdata((std::istreambuf_iterator<char>(openfile)), std::istreambuf_iterator<char>());
	cubesdata = JSON->Parse(cbdata);
	
	openfile.close();
	openfile.open("config/points.txt");
	std::string ptdata((std::istreambuf_iterator<char>(openfile)), std::istreambuf_iterator<char>());
	ptsdata = JSON->Parse(ptdata);

	return;
}

void CStats::UpdateAccuracy()
{
	std::ifstream openfile("config/accuracy.txt");
	// Read previously stored data
	std::string accdata((std::istreambuf_iterator<char>(openfile)), std::istreambuf_iterator<char>());
	Json::Value acc = JSON->Parse(accdata);
	std::string ip = "18.4.45.12";

	openfile.close();
	openfile.open("config/userdb.txt");

	std::string usrdata((std::istreambuf_iterator<char>(openfile)), std::istreambuf_iterator<char>());
	Json::Value userdb = JSON->Parse(usrdata);

	openfile.close();

	for (auto player : userdb)
	{
		std::string usr = player["usr"].asString();

		if (usr == "")
			continue;

		Json::Value usrstats = Utils::APICall("1.0/player/" + usr + "/stats");

		std::cout << "Updating accuracy for: " << usr << "." << std::endl;

		if (usrstats["last"]["accuracy"]["enddate"].asString() != "")
		{
			acc[usr][usrstats["last"]["accuracy"]["enddate"].asString()]["tp"] = usrstats["last"]["accuracy"]["tp"];
			acc[usr][usrstats["last"]["accuracy"]["enddate"].asString()]["fp"] = usrstats["last"]["accuracy"]["fp"];
			acc[usr][usrstats["last"]["accuracy"]["enddate"].asString()]["fn"] = usrstats["last"]["accuracy"]["fn"];
		}
	}

	std::string fileoutput = JSON->Stringify(acc);

	std::ofstream accfileo("config/accuracy.txt", std::ios::trunc);
	accfileo << fileoutput;
	accfileo.close();
}

void CStats::UpdateCubes()
{
	std::ifstream openfile;
	std::string ip = "18.4.45.12";

	openfile.open("config/specialapi.txt");

	std::string specapi((std::istreambuf_iterator<char>(openfile)), std::istreambuf_iterator<char>());
	Json::Value special_api = JSON->Parse(specapi);

	openfile.close();

	time_t timer = time(NULL) - 3600 * 7;
	std::string date = Utils::MakeDate(timer);
	std::string dateTomorrow = Utils::MakeDate(timer + 3600 * 24);

	std::string callstr = "request=5&timespan=custom&startTime=" + date + "&endTime=" + dateTomorrow;

	char *buf = new char[5];
	_itoa(strlen(callstr.c_str()), buf, 10);
	std::string len = buf;

	delete [] buf;

	std::string request;

	// Sorry, I can't show you these
	request += "POST " + special_api["url"].asString() + " HTTP/1.1\r\n";
	request += "Host: " + special_api["host"].asString() + "\r\n";
	request += "Content-type: application/x-www-form-urlencoded\r\n";
	request += "Connection: close\r\n";
	request += "Content-length: " + len + "\r\n\r\n";
	request += callstr;

	SOCKET sock = Socket->CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	std::string response = Socket->SendData(sock, ip, 80, request);

	Socket->DeleteSocket(sock);

	int end = response.find("[");
	response.erase(0, end);

	Json::Value cubedata = JSON->Parse(response);

	for (auto user : cubedata)
	{
		cubesdata[user["username"].asString()][date] = user["cubes"].asString();
	}

	Json::FastWriter writer;

	std::string out = writer.write(cubesdata);

	std::ofstream fileo("config/cubes.txt", std::ios::trunc);
	fileo << out;
	fileo.close();
}

void CStats::FullUpdateCubes()
{
	std::ifstream openfile("config/cubes.txt");
	// Read previously stored data
	std::string cbdata((std::istreambuf_iterator<char>(openfile)), std::istreambuf_iterator<char>());
	Json::Value cb = JSON->Parse(cbdata);
	std::string ip = "18.4.45.12";

	openfile.close();
	openfile.open("config/specialapi.txt");

	std::string specapi((std::istreambuf_iterator<char>(openfile)), std::istreambuf_iterator<char>());
	Json::Value special_api = JSON->Parse(specapi);

	openfile.close();

	time_t timer = time(NULL);
	timer -= (3600 * 7);

	std::string prevdate;

	do {
		std::string date = Utils::MakeDate(timer);
		timer -= (3600 * 24);
		prevdate = Utils::MakeDate(timer);

		std::string callstr = "request=5&timespan=custom&startTime="+prevdate+"&endTime="+date;

		std::cout << date << std::endl;

		char *buf = new char[5];
		_itoa(strlen(callstr.c_str()), buf, 10);
		std::string len = buf;

		delete [] buf;

		std::string request;

		// Sorry, I can't show you these
		request += "POST " + special_api["url"].asString() + " HTTP/1.1\r\n";
		request += "Host: " + special_api["host"].asString() + "\r\n";
		request += "Content-type: application/x-www-form-urlencoded\r\n";
		request += "Connection: close\r\n";
		request += "Content-length: " + len + "\r\n\r\n";
		request += callstr;

		SOCKET sock = Socket->CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		std::string response = Socket->SendData(sock, ip, 80, request);

		Socket->DeleteSocket(sock);

		int end = response.find("[");
		response.erase(0, end);

		Json::Value cubedata = JSON->Parse(response);

		for (auto user : cubedata)
		{
			cb[user["username"].asString()][prevdate] = user["cubes"].asString();
		}

	} while (prevdate != "2012-05-01");

	Json::FastWriter writer;

	std::string out = writer.write(cb);

	std::ofstream fileo("config/cubes.txt", std::ios::trunc);
	fileo << out;
	fileo.close();
}

void CStats::UpdatePoints()
{
	std::ifstream openfile;
	std::string ip = "18.4.45.12";

	openfile.open("config/specialapi.txt");

	std::string specapi((std::istreambuf_iterator<char>(openfile)), std::istreambuf_iterator<char>());
	Json::Value special_api = JSON->Parse(specapi);

	openfile.close();

	time_t timer = time(NULL) - 3600 * 7;
	std::string date = Utils::MakeDate(timer);
	std::string dateTomorrow = Utils::MakeDate(timer + 3600 * 24);

	std::string callstr = "request=8&timespan=custom&startTime=" + date + "&endTime=" + dateTomorrow;

	char *buf = new char[5];
	_itoa(strlen(callstr.c_str()), buf, 10);
	std::string len = buf;

	delete[] buf;

	std::string request;

	// Sorry, I can't show you these
	request += "POST " + special_api["url"].asString() + " HTTP/1.1\r\n";
	request += "Host: " + special_api["host"].asString() + "\r\n";
	request += "Content-type: application/x-www-form-urlencoded\r\n";
	request += "Connection: close\r\n";
	request += "Content-length: " + len + "\r\n\r\n";
	request += callstr;

	SOCKET sock = Socket->CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	std::string response = Socket->SendData(sock, ip, 80, request);

	Socket->DeleteSocket(sock);

	int end = response.find("[");
	response.erase(0, end);

	Json::Value pointsdata = JSON->Parse(response);

	for (auto user : pointsdata)
	{
		ptsdata[user["username"].asString()][date] = user["points"].asString();
	}

	Json::FastWriter writer;

	std::string out = writer.write(ptsdata);

	std::ofstream fileo("config/points.txt", std::ios::trunc);
	fileo << out;
	fileo.close();
}

void CStats::FullUpdatePoints()
{
	std::ifstream openfile("config/points.txt");
	// Read previously stored data
	std::string ptdata((std::istreambuf_iterator<char>(openfile)), std::istreambuf_iterator<char>());
	Json::Value pt = JSON->Parse(ptdata);
	std::string ip = "18.4.45.12";

	openfile.close();
	openfile.open("config/specialapi.txt");

	std::string specapi((std::istreambuf_iterator<char>(openfile)), std::istreambuf_iterator<char>());
	Json::Value special_api = JSON->Parse(specapi);

	openfile.close();

	time_t timer = time(NULL);
	timer -= (3600 * 7);

	std::string prevdate;

	do {
		std::string date = Utils::MakeDate(timer);
		timer -= (3600 * 24);
		prevdate = Utils::MakeDate(timer);

		std::string callstr = "request=8&timespan=custom&startTime=" + prevdate + "&endTime=" + date;

		std::cout << date << std::endl;

		char *buf = new char[5];
		_itoa(strlen(callstr.c_str()), buf, 10);
		std::string len = buf;

		delete[] buf;

		std::string request;

		// Sorry, I can't show you these
		request += "POST " + special_api["url"].asString() + " HTTP/1.1\r\n";
		request += "Host: " + special_api["host"].asString() + "\r\n";
		request += "Content-type: application/x-www-form-urlencoded\r\n";
		request += "Connection: close\r\n";
		request += "Content-length: " + len + "\r\n\r\n";
		request += callstr;

		SOCKET sock = Socket->CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		std::string response = Socket->SendData(sock, ip, 80, request);

		Socket->DeleteSocket(sock);

		int end = response.find("[");
		response.erase(0, end);

		Json::Value pointdata = JSON->Parse(response);

		for (auto user : pointdata)
		{
			if (user["points"].asString() != "" )
				pt[user["username"].asString()][prevdate] = user["points"].asString();
		}

	} while (prevdate != "2012-05-01");

	Json::FastWriter writer;

	std::string out = writer.write(pt);

	std::ofstream fileo("config/points.txt", std::ios::trunc);
	fileo << out;
	fileo.close();
}

void CStats::UpdateRaces()
{
	Json::Value special_api = JSON->Parse(Utils::ReadFile("config/specialapi.txt"));
	Json::Value races = JSON->Parse(Utils::ReadFile("config/races.txt"));
	std::string ip = "18.4.45.12";

	unsigned short count = 0;

	for (auto race : races["races"])
	{
		if (race["finished"] == true || race["started"] == false)
		{
			count++;
			continue;
		}

		std::string startdate = race["times"]["starttime"].asString().substr(0, 10) + " ";
		std::string starthour = race["times"]["starttime"].asString().substr(11, 8);
		while (starthour.find('-') != std::string::npos)
		{
			starthour.replace(starthour.find('-'),1,":");
		}
		startdate += starthour;

		std::string enddate = race["times"]["endtime"].asString().substr(0, 10) + " ";
		std::string endhour = race["times"]["endtime"].asString().substr(11, 8);
		while (endhour.find('-') != std::string::npos)
		{
			endhour.replace(endhour.find('-'), 1,":");
		}
		enddate += endhour;

		std::string request;


		if (race["track"]["points"].asBool() == true)
		{
			request = "";
			std::string callstr = "request=8&timespan=custom&startTime=" + startdate + "&endTime=" + enddate;

			char *buf = new char[5];
			_itoa(strlen(callstr.c_str()), buf, 10);
			std::string len = buf;
			request += "POST " + special_api["url"].asString() + " HTTP/1.1\r\n";
			request += "Host: " + special_api["host"].asString() + "\r\n";
			request += "Content-type: application/x-www-form-urlencoded\r\n";
			request += "Connection: close\r\n";
			request += "Content-length: " + len + "\r\n\r\n";
			request += callstr;

			SOCKET sock = Socket->CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			std::string response = Socket->SendData(sock, ip, 80, request);

			Socket->DeleteSocket(sock);

			int end = response.find("[");
			response.erase(0, end);

			Json::Value pointdata = JSON->Parse(response);

			for (auto competitor : pointdata)
			{
				races["races"][count]["competitors"][competitor["username"].asString()]["username"] = competitor["username"].asString();
				races["races"][count]["competitors"][competitor["username"].asString()]["points"] = competitor["points"].asString();
			}
		}
		if (race["track"]["cubes"].asBool() == true)
		{
			request = "";
			std::string callstr = "request=5&timespan=custom&startTime=" + startdate + "&endTime=" + enddate;

			char *buf = new char[5];
			_itoa(strlen(callstr.c_str()), buf, 10);
			std::string len = buf;
			request += "POST " + special_api["url"].asString() + " HTTP/1.1\r\n";
			request += "Host: " + special_api["host"].asString() + "\r\n";
			request += "Content-type: application/x-www-form-urlencoded\r\n";
			request += "Connection: close\r\n";
			request += "Content-length: " + len + "\r\n\r\n";
			request += callstr;

			SOCKET sock = Socket->CreateSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

			std::string response = Socket->SendData(sock, ip, 80, request);

			Socket->DeleteSocket(sock);

			int end = response.find("[");
			response.erase(0, end);

			Json::Value cubedata = JSON->Parse(response);

			for (auto competitor : cubedata)
			{
				races["races"][count]["competitors"][competitor["username"].asString()]["username"] = competitor["username"].asString();
				races["races"][count]["competitors"][competitor["username"].asString()]["cubes"] = competitor["cubes"].asString();
			}
		}
		count++;

	}
	Json::FastWriter writer;

	std::string out = writer.write(races);

	std::ofstream fileo("config/races.txt", std::ios::trunc);
	fileo << out;
	fileo.close();
}