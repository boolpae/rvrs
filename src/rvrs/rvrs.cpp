// rvrs.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//


#include "CallReceiver.h"
#include "VRCManager.h"
#include "VDCManager.h"
#include "WorkTracer.h"
#include "STTDeliver.h"
#include "configuration.h"

#include <log4cpp/Category.hh>

#include <cerrno>
#include <cstdio>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace itfact::common;

Configuration *config;

int main(int argc, const char** argv)
{
	string input;
    
    log4cpp::Category *logger;
    CallReceiver* rcv=NULL;
    STTDeliver* deliver = NULL;
    
    try {
        config = new Configuration(argc, argv);
    } catch (std::exception &e) {
        perror(e.what());
        return -1;
    }
    
    logger = config->getLogger();

	logger->info("Realtime Voice Relay Server v0.0.alpha");
	logger->debug("=========================================");
	logger->debug("Gearhost IP  : %s", config->getConfig("rvrs.gearhost", "192.168.0.220").c_str());
	logger->debug("Gearhost Port: %d", config->getConfig("rvrs.gearport", 4730));
	logger->debug("Call Rcv Port: %d", config->getConfig("rvrs.callport", 7777));
	logger->debug("Gearhost Port: %d", config->getConfig("rvrs.channel_count", 10));
	logger->debug("Gearhost Port: %d", config->getConfig("rvrs.udp_bport", 10000));
	logger->debug("Call Rcv Port: %d", config->getConfig("rvrs.udp_eport", 11000));

	WorkTracer::instance();
    deliver = STTDeliver::instance(logger);

	VRCManager* vrcm = VRCManager::instance(config->getConfig("rvrs.gearhost", "192.168.0.220"), config->getConfig("rvrs.gearport", 4730), deliver, logger);
	VDCManager* vdcm = VDCManager::instance(config->getConfig("rvrs.channel_count", 10), config->getConfig("rvrs.udp_bport", 10000), config->getConfig("rvrs.udp_eport", 11000), vrcm, logger);
    
    if (!vrcm) {
        printf("\t[DEBUG] MAIN - ERROR (Failed to get VRCManager instance)\n");
        VDCManager::release();
        STTDeliver::release();
        WorkTracer::release();
        delete config;
        return -1;
    }

	rcv = CallReceiver::instance(vdcm, vrcm, logger);

	if (!rcv->init(config->getConfig("rvrs.callport", 7777))) {
        goto FINISH;
	}

	printf("\t[DEBUG] input waiting...quit\n");
    logger->debug("input waiting... quit");
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
    delete config;

    return 0;
}

