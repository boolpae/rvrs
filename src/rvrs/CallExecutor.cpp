
#include "CallExecutor.h"
#include "CallSignal.h"
#include "VRCManager.h"
#include "VDCManager.h"
#include "WorkTracer.h"

#include <thread>
#include <chrono>
#include <vector>

#ifndef WIN32
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

using namespace Protocol;

bool CallExecutor::ms_bThrdRun;

QueueItem::QueueItem(uint16_t num, SOCKET sockfd, struct sockaddr_in si, uint16_t psize, uint8_t* packet)
	: m_nNum(num), m_sockfd(sockfd), m_si(si), m_packetSize(psize)
{
	m_packet = packet;
	printf("\t\t[%d] QueueItem Created!\n", m_nNum);
}

QueueItem::~QueueItem()
{
	delete[] m_packet;
	printf("\t\t[%d] QueueItem Destroyed!\n", m_nNum);
}

CallExecutor::CallExecutor(uint16_t num, VDCManager *vdcm, VRCManager *vrcm, log4cpp::Category *logger)
	: m_nNum(num), m_vdcm(vdcm), m_vrcm(vrcm), m_Logger(logger)
{
	printf("\t[%d] CallExecutor Created!\n", m_nNum);
}


CallExecutor::~CallExecutor()
{
	while (!m_Que.empty()) {
		delete m_Que.front();
		m_Que.pop();
	}

	printf("\t[%d] CallExecutor Destroyed!\n", m_nNum);
}

void CallExecutor::thrdMain(CallExecutor* exe)
{
	uint16_t num = exe->getExecNum();
	QueueItem* item = NULL;
	CallSignal *cs = new CallSignal();
	std::vector< uint16_t > vPorts;
	std::string sCallId;
	uint16_t resReq;

	ms_bThrdRun = true;
	while (ms_bThrdRun) {
		while ((item = exe->popPacket())) {
			//printf("\t[%d] Received packet from %s:%d\n", num, inet_ntoa(item->m_si.sin_addr), ntohs(item->m_si.sin_port));
			printf("\t[%d] Received packet size: %d\n", num, item->m_packetSize);

			// 패킷 파싱 후 호 시작/종료 에 대한 처리 및 응답 패킷 생성 후 응답 로직
			cs->init();
			if ((resReq = cs->parsePacket(item->m_packet)) == 200) {
				
                cs->printPacketInfo();
				
                sCallId = std::string(cs->getCallId());

				if (cs->getPacketFlag() == 'B') {
					WorkTracer::instance()->insertWork(sCallId, 'R', WorkQueItem::PROCTYPE::R_BEGIN_PROC);
					// VDC, VRC 요청
					// 1. VRC 요청 : 성공 시 VDC 요청, 실패 패킷 생성하여 sendto
					WorkTracer::instance()->insertWork(sCallId, 'R', WorkQueItem::PROCTYPE::R_REQ_WORKER);
                    if ((resReq = exe->m_vrcm->requestVRC(sCallId, 'R', cs->getUdpCnt()))) {
						WorkTracer::instance()->insertWork(sCallId, 'R', WorkQueItem::PROCTYPE::R_RES_WORKER);

						if (resReq == 1) {
							// ERRCODE: 500 - 내부 서버 오류
							cs->makePacket(item->m_packet, item->m_packetSize, 500);
						}
						else if (resReq == 2) {
							// ERRCODE: 503 - 서비스를 사용할 수 없음
							cs->makePacket(item->m_packet, item->m_packetSize, 503);
						}
						else if (resReq == 3) {
							// ERRCODE: 504 - Gearman 호스트 연결/통신 실패
							cs->makePacket(item->m_packet, item->m_packetSize, 504);
						}
					}
					// 2. VDC 요청 : 성공 시 성공 패킷 생성하여 sendto, 실패 시 removeVRC 수행 후 실패 패킷 생성하여 sendto
					else {
						WorkTracer::instance()->insertWork(sCallId, 'R', WorkQueItem::PROCTYPE::R_RES_WORKER, 1);
						WorkTracer::instance()->insertWork(sCallId, 'R', WorkQueItem::PROCTYPE::R_REQ_CHANNEL);
						if ((resReq = exe->m_vdcm->requestVDC(sCallId, cs->getUdpCnt(), vPorts))) {
							// ERRCODE: 507 - 용량부족
							WorkTracer::instance()->insertWork(sCallId, 'R', WorkQueItem::PROCTYPE::R_RES_CHANNEL, 0);
                            exe->m_vrcm->removeVRC(sCallId);
							cs->makePacket(item->m_packet, item->m_packetSize, 507);
						}
						else {
							// SUCCESS
							WorkTracer::instance()->insertWork(sCallId, 'R', WorkQueItem::PROCTYPE::R_RES_CHANNEL, 1);
							cs->makePacket(item->m_packet, item->m_packetSize, vPorts);
						}
					}
				}
				else if (cs->getPacketFlag() == 'E') {
					WorkTracer::instance()->insertWork(sCallId, 'R', WorkQueItem::PROCTYPE::R_END_PROC);
                    exe->m_vdcm->removeVDC(sCallId);
					cs->makePacket(item->m_packet, item->m_packetSize, 200);
				}
			}
			else {
				switch (resReq) {
				case 404:	// 패킷 형태 오류 : 잘 못 된 패킷에 대한 응답
					cs->makePacket(item->m_packet, item->m_packetSize);
					break;
				default:
					cs->makePacket(item->m_packet, item->m_packetSize, resReq);
				}
			}

			sendto(item->m_sockfd, (const char*)cs->getPacket(), cs->getPacketSize(), 0, (struct sockaddr *)&item->m_si, sizeof(struct sockaddr_in));

			vPorts.clear();
			delete item;
			item = NULL;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	delete cs;
}

void CallExecutor::pushPacket(SOCKET sockfd, struct sockaddr_in si, uint16_t psize, uint8_t* packet)
{
	std::lock_guard<std::mutex> g(m_mxQue);
	//m_Que.push(packet);
	QueueItem* item = new QueueItem(this->m_nNum, sockfd, si, psize, packet);
	m_Que.push(item);
}

QueueItem* CallExecutor::popPacket()
{
	QueueItem* item;

	std::lock_guard<std::mutex> g(m_mxQue);

	if (m_Que.empty()) return NULL;

	item = m_Que.front();
	m_Que.pop();

	return item;
}
