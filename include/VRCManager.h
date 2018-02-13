#pragma once

#include <stdint.h>

#include <vector>
#include <map>
#include <mutex>
#include <string>

#ifdef WIN32
#include<winsock2.h>
#else

#define SOCKET int
#define closesocket close

#endif

using namespace std;

class VRClient;

class VRCManager
{
	static VRCManager* ms_instance;

	string m_sGearHost;
	uint16_t m_nGearPort;
	SOCKET m_nSockGearman;

	map< string, VRClient* > m_mWorkerTable;

	mutable std::mutex m_mxMap;

public:
	static VRCManager* instance();
	static void release();

	int16_t requestVRC(string& callid, uint8_t jobType, uint8_t noc);
	void removeVRC(string& callid);
	void removeAllVRC();

	void outputVRCStat();

	VRClient* getVRClient(string& callid);

	string& getGearHost() { return m_sGearHost; }

	uint16_t getGearPort() { return m_nGearPort; }

private:
	VRCManager();
	virtual ~VRCManager();

	bool connect2Gearman();
	bool getGearmanFnames(std::vector< std::string > &vFnames);
	void getFnamesFromString(std::string &gearResult, std::vector< std::string > &vFnames);
	void setGearHost(string host) { m_sGearHost = host; }
	void setGearPort(uint16_t port) { m_nGearPort = port; }
};

