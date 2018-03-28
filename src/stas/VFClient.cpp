
#include "VFClient.h"



VFClient* VFClient::m_instance = nullptr;


VFClient::VFClient()
{
    
}

VFClient::~VFClient()
{
    m_instance = nullptr;
}

VFClient* VFClient::instance()
{
    if (!m_instance) {
        m_instance = new VFClient();
    }
    
    return m_instance;
}