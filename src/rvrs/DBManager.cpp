
#include "DBManager.h"



DBManager* DBManager::m_instance = nullptr;


DBManager::DBManager()
{
    
}

DBManager::~DBManager()
{
    if (m_thrd.joinable()) {
        m_thrd.join();
    }
}

void DBManager::thrdMain(DBManager *mgr)
{
    
}

DBManager* DBManager::instance()
{
    if (m_instance) return m_instance;
    
    m_instance = new DBManager();
    return m_instance;
}

void DBManager::release()
{
    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }
}

int DBManager::insertCallInfo()
{
    return 0;
}

int DBManager::updateCallInfo()
{
    return 0;
}

int DBManager::insertRtSTTData()
{
    return 0;
}
