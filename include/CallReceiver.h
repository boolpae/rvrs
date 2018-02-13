#pragma once

#include <stdint.h>

#ifdef WIN32
#include<winsock2.h>
#else

#define SOCKET int
#define closesocket close

#endif

class CallExecutor;

class CallReceiver
{
	static CallReceiver* m_instance;

	SOCKET m_nSockfd;

	uint16_t m_nNumofExecutor;

public:
	static CallReceiver* instance();
	static void release();

	void setNumOfExecutor(uint16_t num) { m_nNumofExecutor = num; }
	bool init(uint16_t port);
	SOCKET getSockfd() { return m_nSockfd; }

private:
	CallReceiver();
	virtual ~CallReceiver();

	static void thrdMain(CallReceiver* rcv);

};

