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

#include <log4cpp/Category.hh>

using namespace std;

class VRClient;
class STTDeliver;
class RT2DB;

class VRCManager
{
	static VRCManager* ms_instance;

	string m_sGearHost;
	uint16_t m_nGearPort;
    int m_GearTimeout;
    
    SOCKET m_nSockGearman;

	map< string, VRClient* > m_mWorkerTable;

	mutable std::mutex m_mxMap;
    
    STTDeliver *m_deliver;
    
    log4cpp::Category *m_Logger;
    RT2DB* m_r2d;

public:
	static VRCManager* instance(const std::string gearHostIp, const uint16_t gearHostPort, int geartimout, STTDeliver *deliver, log4cpp::Category *logger, RT2DB* r2d);
	static void release();

	int16_t requestVRC(string& callid, uint8_t jobType, uint8_t noc);
	void removeVRC(string& callid);
	void removeAllVRC();

	void outputVRCStat();

	VRClient* getVRClient(string& callid);

	string& getGearHost() { return m_sGearHost; }

	uint16_t getGearPort() { return m_nGearPort; }

private:
	VRCManager(int geartimeout, STTDeliver *deliver, log4cpp::Category *logger, RT2DB* r2d);
	virtual ~VRCManager();

	bool connect2Gearman();
	bool getGearmanFnames(std::vector< std::string > &vFnames);
	void getFnamesFromString(std::string &gearResult, std::vector< std::string > &vFnames);
	void setGearHost(string host) { m_sGearHost = host; }
	void setGearPort(uint16_t port) { m_nGearPort = port; }
};

