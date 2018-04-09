#pragma once

#include <stdint.h>

#include <vector>
#include <map>
#include <mutex>
#include <string>

#define SOCKET int
#define closesocket close

#include <log4cpp/Category.hh>

using namespace std;

class VFClient;

class VFCManager
{
	static VFCManager* m_instance;

	string m_sGearHost;
	uint16_t m_nGearPort;
    int m_GearTimeout;
    
    SOCKET m_nSockGearman;

	map< string, VFClient* > m_mWorkerTable;

	mutable std::mutex m_mxMap;
    
    log4cpp::Category *m_Logger;

	mutable std::mutex m_mxQue;
    
public:
	static VFCManager* instance(const std::string gearHostIp, const uint16_t gearHostPort, int geartimout, log4cpp::Category *logger);
	static void release();

	int16_t requestVFC(string& callid, uint8_t jobType, uint8_t noc);
	void removeVFC(string callid);
	void removeAllVFC();

	void outputVFCStat();

	VFClient* getVFClient(string& callid);

	string& getGearHost() { return m_sGearHost; }

	uint16_t getGearPort() { return m_nGearPort; }
    
    int addVFC(string callid, string fname, uint8_t jobtype, uint8_t noc);

private:
	VFCManager(int geartimeout, log4cpp::Category *logger);
	virtual ~VFCManager();

	bool connectGearman();
    void disconnectGearman();
	bool getGearmanFnames(std::vector< std::string > &vFnames);
	void getFnamesFromString(std::string &gearResult, std::vector< std::string > &vFnames);
	void setGearHost(string host) { m_sGearHost = host; }
	void setGearPort(uint16_t port) { m_nGearPort = port; }
};

