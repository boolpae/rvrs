
#include "Schd4DB.h"


Schd4DB* Schd4DB::m_instance = nullptr;

Schd4DB::Schd4DB()
{
    
}

Schd4DB::~Schd4DB()
{
    m_instance = nullptr;
}

Schd4DB* Schd4DB::instance()
{
    if (!m_instance) {
        m_instance = new Schd4DB();
    }
    return m_instance;
}