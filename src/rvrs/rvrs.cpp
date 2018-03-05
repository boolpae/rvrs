// rvrs.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//


#include "CallReceiver.h"
#include "VRCManager.h"
#include "VDCManager.h"
#include "WorkTracer.h"
#include "STTDeliver.h"
#include "configuration.h"
#include "rvrs.h"
#include "RT2DB.h"

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
#include <thread>
#include <chrono>
#include <csignal>

using namespace std;
using namespace itfact::common;

Configuration *config;

volatile bool gRunning = true;

void term_handle(int sig)
{
    gRunning = false;
}

int main(int argc, const char** argv)
{
	string input;
    
    log4cpp::Category *logger;
    RT2DB* rt2db=nullptr;
    CallReceiver* rcv=nullptr;
    STTDeliver* deliver = nullptr;
    
    std::signal(SIGINT, term_handle);
    std::signal(SIGTERM, term_handle);
    //std::signal(SIGABRT, term_handle);
    
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
	logger->info("MPI host IP      :  %s", config->getConfig("rvrs.mpihost", "127.0.0.1").c_str());
	logger->info("MPI host Port    :  %d", config->getConfig("rvrs.mpiport", 4730));
	logger->info("MPI host Timeout :  %d", config->getConfig("rvrs.mpitimeout", 0));
	logger->info("Call Signal Port :  %d", config->getConfig("rvrs.callport", 7000));
	logger->info("Call Channel Cnt :  %d", config->getConfig("rvrs.channel_count", 200));
	logger->info("Voice Playtime   :  %d", config->getConfig("rvrs.playtime", 3));
	logger->info("Voice Begin Port :  %d", config->getConfig("rvrs.udp_bport", 10000));
	logger->info("Voice END Port   :  %d", config->getConfig("rvrs.udp_eport", 11000));

    logger->info("Database USE     :  %s", config->getConfig("database.use", "false").c_str());
    if (!config->getConfig("database.use", "false").compare("true")) {
        logger->info("Database Type    :  %s", config->getConfig("database.type", "mysql").c_str());
        logger->info("Database Addr    :  %s", config->getConfig("database.addr", "localhost").c_str());
        logger->info("Database Port    :  %s", config->getConfig("database.port", "3306").c_str());
        logger->info("Database ID      :  %s", config->getConfig("database.id", "stt").c_str());
        logger->info("Database Name    :  %s", config->getConfig("database.name", "rt_stt").c_str());
        logger->info("Database CharSet :  %s", config->getConfig("database.chset", "utf8").c_str());
        
        rt2db = RT2DB::instance(config->getConfig("database.type", "mysql"),
                                config->getConfig("database.addr", "localhost"),
                                config->getConfig("database.port", "3306"),
                                config->getConfig("database.id", "stt"),
                                config->getConfig("database.pw", "rt_stt"),
                                config->getConfig("database.name", "rt_stt"),
                                config->getConfig("database.chset", "utf8"),
                                logger);
        if (!rt2db) {
            logger->error("MAIN - ERROR (Failed to get RT2DB instance)");
            delete config;
            return -1;
        }
    }
    logger->info("STT Result USE   :  %s", config->getConfig("stt_result.use", "true").c_str());

	WorkTracer::instance();
    WorkTracer::instance()->setLogger(&tracerLog);
    
    if (!config->getConfig("stt_result.use", "false").compare("true")) {
        deliver = STTDeliver::instance(config->getConfig("stt_result.path", "./stt_result"), logger);
    }

	VRCManager* vrcm = VRCManager::instance(config->getConfig("rvrs.mpihost", "127.0.0.1"), config->getConfig("rvrs.mpiport", 4730), config->getConfig("rvrs.mpitimeout", 0), deliver, logger, rt2db);
	VDCManager* vdcm = VDCManager::instance(config->getConfig("rvrs.channel_count", 200), config->getConfig("rvrs.udp_bport", 10000), config->getConfig("rvrs.udp_eport", 11000), config->getConfig("rvrs.playtime", 3), vrcm, logger);
    
    if (!vrcm) {
        logger->error("MAIN - ERROR (Failed to get VRCManager instance)");
        VDCManager::release();
        STTDeliver::release();
        WorkTracer::release();
        delete config;
        return -1;
    }

	rcv = CallReceiver::instance(vdcm, vrcm, logger, rt2db);
    rcv->setNumOfExecutor(config->getConfig("rvrs.callexe_count", 5));

	if (!rcv->init(config->getConfig("rvrs.callport", 7000))) {
        goto FINISH;
	}

#if 0
    logger->debug("input waiting... quit");
	while (1) {
		std::cin >> input;
		if (!input.compare("quit")) break;
	}
#else
    while (gRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
#endif

	vdcm->outputVDCStat();
	vrcm->outputVRCStat();

FINISH:

	logger->debug("MAIN FINISH!");

	vdcm->release();
	vrcm->release();
	rcv->release();

    STTDeliver::release();
	WorkTracer::release();
    RT2DB::release();
    delete config;

    return 0;
}

