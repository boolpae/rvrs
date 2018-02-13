// rvrs.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//


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
    
    CallReceiver* rcv=NULL;

	WorkTracer::instance();
    STTDeliver::instance();

	VDCManager* vdcm = VDCManager::instance();
	VRCManager* vrcm = VRCManager::instance();
    
    if (!vrcm) {
        printf("\t[DEBUG] MAIN - ERROR (Failed to get VRCManager instance)\n");
        VDCManager::release();
        STTDeliver::release();
        WorkTracer::release();
        return -1;
    }

	rcv = CallReceiver::instance();

	if (!rcv->init(7777)) {
        goto FINISH;
	}

	printf("\t[DEBUG] input waiting...quit\n");
	while (1) {
		std::cin >> input;
		if (!input.compare("quit")) break;
	}


	vdcm->outputVDCStat();
	vrcm->outputVRCStat();

FINISH:

	printf("\t[DEBUG] MAIN\n");

	vdcm->release();
	vrcm->release();
	rcv->release();

    STTDeliver::release();
	WorkTracer::release();

    return 0;
}

