
#ifndef __VFCLIENT_H__
#define __VFCLIENT_H__

#include <thread>
#include <stdint.h>

class VFCManager;

class VFClient {
public:
    VFClient(VFCManager* mgr, std::string gearHost, uint16_t gearPort, int gearTimeout, uint64_t numId);
    virtual ~VFClient();
    
    void startWork();
    void stopWork() { m_LiveFlag = false; }

private:
    static void thrdFunc(VFCManager* mgr, VFClient* clt);

private:
    std::thread m_thrd;
    
    bool m_LiveFlag;
    VFCManager *m_mgr;

	std::string m_sGearHost;
	uint16_t m_nGearPort;
    int m_nGearTimeout;
    uint64_t m_nNumId;
};


#endif // __VFCLIENT_H__