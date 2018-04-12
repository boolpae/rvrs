#pragma once

#include <stdint.h>

#include <map>
#include <mutex>
#include <string>
#include <queue>
#include <thread>

#define SOCKET int
#define closesocket close

#include <log4cpp/Category.hh>

using namespace std;

class VFClient;

class VFCManager
{
private:
	static VFCManager* m_instance;
    
	string m_sGearHost;
	uint16_t m_nGearPort;
    int m_GearTimeout;
    
    SOCKET m_nSockGearman;

    log4cpp::Category *m_Logger;

	map< string, VFClient* > m_mWorkerTable;
	queue< string > m_qVFQue;

	mutable std::mutex m_mxMap;
	mutable std::mutex m_mxQue;
    
public:
	static VFCManager* instance(const std::string gearHostIp, const uint16_t gearHostPort, int geartimout, log4cpp::Category *logger);
	static void release();

	void outputVFCStat();

	string& getGearHost() { return m_sGearHost; }
	uint16_t getGearPort() { return m_nGearPort; }
    
    int pushItem(std::string line);
    int popItem(std::string& line);

private:
	VFCManager(int geartimeout, log4cpp::Category *logger);
	virtual ~VFCManager();

	bool connectGearman();
    void disconnectGearman();
    size_t getWorkerCount();
	size_t getWorkerCountFromString(std::string &gearResult);
	void setGearHost(string host) { m_sGearHost = host; }
	void setGearPort(uint16_t port) { m_nGearPort = port; }
    void syncWorkerVFClient();
    
    static void thrdFuncVFCManager(VFCManager* mgr);
};

