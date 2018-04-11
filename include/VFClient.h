
#ifndef __VFCLIENT_H__
#define __VFCLIENT_H__

#include <thread>
#include <stdint.h>

class VFCManager;

class VFClient {
public:
    VFClient(VFCManager* mgr, std::string gearHost, uint16_t gearPort, int gearTimeout);
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
};


#endif // __VFCLIENT_H__