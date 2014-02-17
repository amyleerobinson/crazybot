#ifndef _C_STATS
#define _C_STATS

#include "CSocket.h"
#include "CJSON.h"
#include "Utils.h"

#include<fstream>
#include<thread>
#include<chrono>
#include<ctime>
#include<cstdlib>

class CStats
{
	CSocket *Socket;
	CJSON *JSON;

	Json::Value ptsdata,cubesdata;
public:
	CStats();
	~CStats();
	void Init();
	void UpdateAccuracy();
	void UpdateCubes();
	void UpdatePoints();
	// WARNING: Call this function only if you're prepared to wait a long time
	void FullUpdateCubes();
	void FullUpdatePoints();
};

#endif