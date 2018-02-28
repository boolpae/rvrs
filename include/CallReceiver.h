#pragma once

#include <stdint.h>

#ifdef WIN32
#include<winsock2.h>
#else

#define SOCKET int
#define closesocket close

#endif

#include <log4cpp/Category.hh>

class VDCManager;
class VRCManager;
class CallExecutor;
class RT2DB;

class CallReceiver
{
	static CallReceiver* m_instance;

	SOCKET m_nSockfd;

	uint16_t m_nNumofExecutor;
    
    VDCManager *m_vdcm;
    VRCManager *m_vrcm;
    
    log4cpp::Category *m_Logger;
    
    RT2DB* m_rt2db;

public:
	static CallReceiver* instance(VDCManager *vdcm, VRCManager *vrcm, log4cpp::Category *logger, RT2DB* rt2db);
	static void release();

	void setNumOfExecutor(uint16_t num) { m_nNumofExecutor = num; }
	bool init(uint16_t port);
	SOCKET getSockfd() { return m_nSockfd; }

private:
	CallReceiver(VDCManager *vdcm, VRCManager *vrcm, log4cpp::Category *logger, RT2DB* rt2db);
	virtual ~CallReceiver();

	static void thrdMain(CallReceiver* rcv);

};

