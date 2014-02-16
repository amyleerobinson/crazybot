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