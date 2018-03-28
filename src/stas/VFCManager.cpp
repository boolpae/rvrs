
#include "VFCManager.h"



VFCManager* VFCManager::m_instance = nullptr;


VFCManager::VFCManager()
{
    
}

VFCManager::~VFCManager()
{
    m_instance = nullptr;
}

VFCManager* VFCManager::instance()
{
    if (!m_instance) {
        m_instance = new VFCManager();
    }
    
    return m_instance;
}