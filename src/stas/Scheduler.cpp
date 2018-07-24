
#include "stas.h"
#include "Scheduler.h"

#ifndef USE_ODBC
#include "DBHandler.h"
#else
#include "DBHandler_ODBC.h"
#endif

#include "VFCManager.h"
#include "HAManager.h"

#include <chrono>

Scheduler* Scheduler::m_instance = nullptr;

Scheduler::Scheduler(DBHandler *sttdb, VFCManager *vfcmgr)
: m_bLiveFlag(true), m_sttdb(sttdb), m_vfcmgr(vfcmgr)
{
    
}

Scheduler::~Scheduler()
{
    if (m_thrd.joinable()) m_thrd.join();

    m_instance = nullptr;
}

Scheduler* Scheduler::instance(DBHandler *sttdb, VFCManager *vfcmgr)
{
    if (!m_instance) {
        m_instance = new Scheduler(sttdb, vfcmgr);

        m_instance->m_thrd = std::thread(Scheduler::thrdFuncScheduler, m_instance, vfcmgr);
    }
    return m_instance;
}

void Scheduler::release()
{
	if (m_instance) {
        m_instance->m_thrd.detach();
        m_instance->m_bLiveFlag = false;
		delete m_instance;
	}
}

void Scheduler::thrdFuncScheduler(Scheduler *schd, VFCManager *vfcm)
{
    log4cpp::Category *logger = config->getLogger();
    HAManager *ham = HAManager::getInstance();
    std::vector< JobInfoItem* > v;
    JobInfoItem *item;

    if (ham && !ham->getHAStat()) {
        logger->debug("Scheduler::thrdFuncScheduler() - Waiting... for Standby Mode");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    while(schd->m_bLiveFlag) {
        // DBHandler의 api를 이용하여 새로운 task를 확인
        // 새로운 task를 VFCManager의 큐에 등록

        // get items from DB
        if (schd->m_sttdb->getTaskInfo(v, vfcm->getAvailableCount()) > 0) {
            for( std::vector< JobInfoItem* >::iterator iter = v.begin(); iter != v.end(); iter++) {
                item = *iter;

                logger->debug("thrdFuncScheduler (%s, %s)", item->getPath().c_str(), item->getFilename().c_str());
                // put item to VFCMgr's Queue
                if (schd->m_vfcmgr->pushItem(item/*item->getPath()+"/"+item->getFilename()*/) > 0) {
                    schd->m_sttdb->updateTaskInfo(item->getCallId(), item->getCounselorCode(), 'U');
                }
            }

            v.clear();
        }

        // config에 설정된 sec 시간 간격으로 확인
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}
