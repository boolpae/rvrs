
#include "Notifier.h"


Notifier* Notifier::m_instance = nullptr;



Notifier::Notifier()
{
    
}

Notifier::~Notifier()
{
    
}

Notifier* Notifier::instance()
{
    if (!m_instance) {
        m_instance = new Notifier();
    }
    
    return m_instance;
}