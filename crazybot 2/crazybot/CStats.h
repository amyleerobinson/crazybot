#ifndef _C_STATS
#define _C_STATS

#include "CSocket.h"
#include "CJSON.h"
#include "Utils.h"

#include<fstream>
#include<thread>
#include<chrono>
#include<ctime>

class CStats
{
	CSocket *Socket;
	CJSON *JSON;
	bool keep_updating;
	bool is_init;

	std::thread *accuracy_updater;
public:
	CStats();
	~CStats();
	void InitAutoUpdaters();
	void UpdateAccuracy();
	void UpdateAccuracyLoop();
};

#endif