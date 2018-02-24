
#include "HAManager.h"



HAManager* HAManager::m_instance = nullptr;
bool HAManager::m_bHAState = false;


HAManager::HAManager()
: m_thrd(nullptr), m_bLiveFlag(false)
{
    
}

HAManager::~HAManager()
{
    if (m_thrd) {
        m_thrd->join();
    }
}

HAManager* HAManager::instance()
{
    if ( m_instance ) return m_instance;
    
    m_instance = new HAManager();
    
    return m_instance;
}

void HAManager::release()
{
    if ( m_instance ) {
        delete m_instance;
        m_instance = nullptr;
    }
}

void HAManager::threMain(HAManager* mgr)
{
    while (mgr->m_bLiveFlag) {
        
    }
}

int HAManager::start()
{
    int ret=0;
    
    m_thrd = new std::thread(HAManager::threMain, this);
    m_bLiveFlag = true;
    
    return ret;
}