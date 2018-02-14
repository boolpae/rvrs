// rvrs.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//


#include "CallReceiver.h"
#include "VRCManager.h"
#include "VDCManager.h"
#include "WorkTracer.h"
#include "STTDeliver.h"
#include "configuration.h"
#include "rvrs.h"

#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/RollingFileAppender.hh>

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
    
    int max_size = -1, max_backup = 0;
    std::string traceName;
    std::string log_max;
    traceName = config->getConfig("rvrs.trace_name", "worktrace.trc");
    log_max = config->getConfig("rvrs.trace_max", "1MiB");
    max_backup = config->getConfig("rvrs.trace_backup", 5);
    max_size = std::stoul(log_max.c_str()) * itfact::common::convertUnit(log_max);
    log4cpp::Appender *appender = new log4cpp::RollingFileAppender("default", traceName, max_size, max_backup);
	log4cpp::Layout *layout = new log4cpp::PatternLayout();
    try {
        ((log4cpp::PatternLayout *) layout)->setConversionPattern("[%d{%Y-%m-%d %H:%M:%S.%l}] %-6p %m%n");
        appender->setLayout(layout);
    } catch (log4cpp::ConfigureFailure &e) {
        if (layout)
            delete layout;
        layout = new log4cpp::BasicLayout();
        appender->setLayout(layout);
    }

    log4cpp::Category &tracerLog = log4cpp::Category::getInstance(std::string("WorkTracer"));
    tracerLog.addAppender(appender);
    
	logger->info("Realtime Voice Relay Server ver %d.%d.%d", RVRS_VERSION_MAJ, RVRS_VERSION_MIN, RVRS_VERSION_BLD);
	logger->info("=========================================");
	logger->info("Gearhost IP  :  %s", config->getConfig("rvrs.gearhost", "127.0.0.1").c_str());
	logger->info("Gearhost Port:  %d", config->getConfig("rvrs.gearport", 4730));
	logger->info("Call Rcv Port:  %d", config->getConfig("rvrs.callport", 7000));
	logger->info("Gearhost Port:  %d", config->getConfig("rvrs.channel_count", 200));
	logger->info("Voice Playtime: %d", config->getConfig("rvrs.playtime", 3));
	logger->info("Gearhost Port:  %d", config->getConfig("rvrs.udp_bport", 10000));
	logger->info("Call Rcv Port:  %d", config->getConfig("rvrs.udp_eport", 11000));

	WorkTracer::instance();
    WorkTracer::instance()->setLogger(&tracerLog);
    
    deliver = STTDeliver::instance(logger);

	VRCManager* vrcm = VRCManager::instance(config->getConfig("rvrs.gearhost", "127.0.0.1"), config->getConfig("rvrs.gearport", 4730), deliver, logger);
	VDCManager* vdcm = VDCManager::instance(config->getConfig("rvrs.channel_count", 200), config->getConfig("rvrs.udp_bport", 10000), config->getConfig("rvrs.udp_eport", 11000), config->getConfig("rvrs.playtime", 3), vrcm, logger);
    
    if (!vrcm) {
        logger->error("MAIN - ERROR (Failed to get VRCManager instance)");
        VDCManager::release();
        STTDeliver::release();
        WorkTracer::release();
        delete config;
        return -1;
    }

	rcv = CallReceiver::instance(vdcm, vrcm, logger);
    rcv->setNumOfExecutor(config->getConfig("rvrs.callexe_count", 5));

	if (!rcv->init(config->getConfig("rvrs.callport", 7000))) {
        goto FINISH;
	}

    logger->debug("input waiting... quit");
	while (1) {
		std::cin >> input;
		if (!input.compare("quit")) break;
	}


	vdcm->outputVDCStat();
	vrcm->outputVRCStat();

FINISH:

	logger->debug("MAIN FINISH!");

	vdcm->release();
	vrcm->release();
	rcv->release();

    STTDeliver::release();
	WorkTracer::release();
    delete config;

    return 0;
}

