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
class STT2File;
class STT2DB;

class VRCManager
{
	static VRCManager* ms_instance;

	string m_sGearHost;
	uint16_t m_nGearPort;
    int m_GearTimeout;
    
    SOCKET m_nSockGearman;

	map< string, VRClient* > m_mWorkerTable;

	mutable std::mutex m_mxMap;
    
    STT2File *m_deliver;
    
    log4cpp::Category *m_Logger;
    STT2DB* m_s2d;

	mutable std::mutex m_mxQue;
    
    bool m_is_save_pcm;
    string m_pcm_path;
    size_t m_framelen;

public:
	static VRCManager* instance(const std::string gearHostIp, const uint16_t gearHostPort, int geartimout, STT2File *deliver, log4cpp::Category *logger, STT2DB* s2d, bool is_save_pcm, string pcm_path, size_t framelen);
	static void release();

	int16_t requestVRC(string& callid, uint8_t jobType, uint8_t noc);
	void removeVRC(string callid);
	void removeAllVRC();

	void outputVRCStat();

	VRClient* getVRClient(string& callid);

	string& getGearHost() { return m_sGearHost; }

	uint16_t getGearPort() { return m_nGearPort; }
    
    int addVRC(string callid, string fname, uint8_t jobtype, uint8_t noc);

private:
	VRCManager(int geartimeout, STT2File *deliver, log4cpp::Category *logger, STT2DB* s2d, bool is_save_pcm, string pcm_path, size_t framelen);
	virtual ~VRCManager();

	bool connectGearman();
    void disconnectGearman();
	bool getGearmanFnames(std::vector< std::string > &vFnames);
	void getFnamesFromString(std::string &gearResult, std::vector< std::string > &vFnames);
	void setGearHost(string host) { m_sGearHost = host; }
	void setGearPort(uint16_t port) { m_nGearPort = port; }
};

