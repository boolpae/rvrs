
#include "HAManager.h"



HAManager* HAManager::m_instance = nullptr;
bool HAManager::m_bHAState = false;


HAManager::HAManager()
{
    
}

HAManager::~HAManager()
{
    
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