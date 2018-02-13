// SmartVRS.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//


#ifdef WIN32

#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#endif

#include "CallReceiver.h"
#include "VRCManager.h"
#include "VDCManager.h"
#include "WorkTracer.h"
#include "STTDeliver.h"

#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;

int main(int argc, char** argv)
{
	string input;
	vector<uint16_t> vPorts;
	vector<uint16_t>::iterator iter;
    
    CallReceiver* rcv=NULL;
#ifdef WIN32
	WSADATA wsa;

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");
#endif
	WorkTracer::instance();
    STTDeliver::instance();

	VDCManager* vdcm = VDCManager::instance();
	VRCManager* vrcm = VRCManager::instance();
    
    if (!vrcm) {
        printf("\t[DEBUG] MAIN - ERROR (Failed to get VRCManager instance)\n");
        VDCManager::release();
        WorkTracer::release();
        goto FINISH;
    }

	rcv = CallReceiver::instance();

	std::this_thread::sleep_for(std::chrono::seconds(2));
#if 0
	input = std::string("quit");

	std::this_thread::sleep_for(std::chrono::seconds(2));
	vrcm->requestVRC(input, 'R', 2);
	vdcm->requestVDC(input, 2, vPorts);
	for (iter = vPorts.begin(); iter != vPorts.end(); iter++) {
		printf("\t[DEBUG] UDP PORT(%d)\n", *iter);
	}

	vdcm->outputVDCStat();
	vrcm->outputVRCStat();
#endif
	printf("\t[DEBUG] input waiting...stop\n");

	if (rcv->init(7777)) {
		while (1) {
			std::cin >> input;
			if (!input.compare("stop")) break;
			else {
				if (vrcm->requestVRC(input, 'R', 2)) {
					printf("\t[DEBUG] MAIN - failed requestVRC\n");
				}
				else
				{
					vPorts.clear();
					if (vdcm->requestVDC(input, 2, vPorts)) {
						printf("\t[DEBUG] MAIN - failed requestVDC\n");
						vrcm->removeVRC(input);
					}
					else {
						for (iter = vPorts.begin(); iter != vPorts.end(); iter++) {
							printf("\t[DEBUG] UDP PORT(%d)\n", *iter);
						}
					}
				}

				vdcm->outputVDCStat();
				vrcm->outputVRCStat();
			}
		}
	}

	//std::this_thread::sleep_for(std::chrono::seconds(2));
	//vrcm->removeVRC(input);
	printf("\t[DEBUG] input waiting...quit\n");
	while (1) {
		std::cin >> input;
		if (!input.compare("quit")) break;
	}


	vdcm->outputVDCStat();
	vrcm->outputVRCStat();
	printf("\t[DEBUG] MAIN\n");

	vdcm->release();
	vrcm->release();
	rcv->release();

    STTDeliver::release();
	WorkTracer::release();

	//std::this_thread::sleep_for(std::chrono::seconds(10));
FINISH:
#ifdef WIN32
	WSACleanup();
#endif

    return 0;
}

