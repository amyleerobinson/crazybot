#include "CStats.h"

CStats::CStats()
{
	Socket = new CSocket();
	JSON = new CJSON();
	is_init = false;
}

CStats::~CStats()
{
	SAFE_DELETE(Socket);
	SAFE_DELETE(JSON);
	keep_updating = false;
	accuracy_updater->join();
}

void CStats::InitAutoUpdaters()
{
	if (is_init)
		return;

	accuracy_updater = new std::thread(&CStats::UpdateAccuracyLoop, this);
}

void CStats::UpdateAccuracy()
{
	std::ifstream accfile("config/accuracy.txt");
	// Read previously stored data
	std::string accdata((std::istreambuf_iterator<char>(accfile)), std::istreambuf_iterator<char>());
	Json::Value acc = JSON->Parse(accdata);
	std::string ip = "18.4.45.12";

	accfile.close();

	Json::Value lboard = Utils::APICall("api/v1/stats/top/user/by/points/per/day");

	for (auto player : lboard)
	{
		std::string usr = player["username"].asString();

		Json::Value usrstats = Utils::APICall("1.0/player/" + usr + "/stats");

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

void CStats::UpdateAccuracyLoop()
{
	while (keep_updating)
	{
		std::this_thread::sleep_for(std::chrono::minutes(1));

		std::time_t tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::tm * ptm = std::localtime(&tt);

		if (!(ptm->tm_hour == 6 && ptm->tm_min >= 50 && ptm->tm_min < 52))
			continue;

		UpdateAccuracy();
	}
}