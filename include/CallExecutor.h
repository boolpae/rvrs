#pragma once

#include <stdint.h>
#include <queue>
#include <mutex>

#ifdef WIN32
#include<winsock2.h>
#else

#include <netinet/in.h>

#define SOCKET int
#define closesocket close

#endif

#include <log4cpp/Category.hh>

using namespace std;

class QueueItem {
	uint16_t m_nNum;
public:
	QueueItem(uint16_t num, SOCKET sockfd, struct sockaddr_in si, uint16_t psize, uint8_t* packet);
	virtual ~QueueItem();

	SOCKET m_sockfd;
	struct sockaddr_in m_si;
	uint16_t m_packetSize;
	uint8_t* m_packet;

};

class VDCManager;
class VRCManager;

class CallExecutor
{
	uint16_t m_nNum;
	queue< QueueItem* > m_Que;
	static bool ms_bThrdRun;

	mutable std::mutex m_mxQue;
    
    VDCManager *m_vdcm;
    VRCManager *m_vrcm;
    
    log4cpp::Category *m_Logger;

public:
	CallExecutor(uint16_t num, VDCManager *vdcm, VRCManager *vrcm, log4cpp::Category *logger);
	virtual ~CallExecutor();

	static void thrdMain(CallExecutor* exe);
	static void thrdFinish() { ms_bThrdRun = false; }

	void pushPacket( SOCKET sockfd, struct sockaddr_in si, uint16_t psize, uint8_t* packet );
	QueueItem* popPacket();

	uint16_t getExecNum() { return m_nNum; }
};

