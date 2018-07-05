
#include "stas.h"
#include "Schd4DB.h"

#ifndef USE_ODBC
#include "STT2DB.h"
#else
#include "STT2DB_ODBC.h"
#endif

#include "VFCManager.h"
#include "HAManager.h"

#include <chrono>

Schd4DB* Schd4DB::m_instance = nullptr;

Schd4DB::Schd4DB(STT2DB *sttdb, VFCManager *vfcmgr)
: m_bLiveFlag(true), m_sttdb(sttdb), m_vfcmgr(vfcmgr)
{
    
}

Schd4DB::~Schd4DB()
{
    if (m_thrd.joinable()) m_thrd.join();

    m_instance = nullptr;
}

Schd4DB* Schd4DB::instance(STT2DB *sttdb, VFCManager *vfcmgr)
{
    if (!m_instance) {
        m_instance = new Schd4DB(sttdb, vfcmgr);

        m_instance->m_thrd = std::thread(Schd4DB::thrdFuncSchd4DB, m_instance);
    }
    return m_instance;
}

void Schd4DB::release()
{
	if (m_instance) {
        m_instance->m_thrd.detach();
        m_instance->m_bLiveFlag = false;
		delete m_instance;
	}
}

void Schd4DB::thrdFuncSchd4DB(Schd4DB *schd)
{
    log4cpp::Category *logger = config->getLogger();
    HAManager *ham = HAManager::getInstance();
    std::vector< JobInfoItem* > v;
    JobInfoItem *item;

    if (ham && !ham->getHAStat()) {
        logger->debug("Schd4DB::thrdFuncSchd4DB() - Waiting... for Standby Mode");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    while(schd->m_bLiveFlag) {
        // STT2DB의 api를 이용하여 새로운 task를 확인
        // 새로운 task를 VFCManager의 큐에 등록

        // get items from DB
        if (schd->m_sttdb->getTaskInfo(v) > 0) {
            for( std::vector< JobInfoItem* >::iterator iter = v.begin(); iter != v.end(); iter++) {
                item = *iter;

                logger->debug("thrdFuncSchd4DB (%s, %s)", item->getPath().c_str(), item->getFilename().c_str());
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
