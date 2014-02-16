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
public:
	CStats();
	~CStats();
	void UpdateAccuracy();
};

#endif