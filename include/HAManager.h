#ifndef __HAMANAGER_H__
#define __HAMANAGER_H__

#include <thread>

class HAManager {
private:
    static HAManager* m_instance;
    static bool m_bHAState; // Active: true, Standby: false
    
    std::thread m_thrd;
    bool m_bLiveFlag;
    
public:
    
private:
    HAManager();
    
    static void threMain(HAManager* mgr);
    
public:
    virtual ~HAManager();
    
    static HAManager* instance();
    static void release();
    
    int start();
};


#endif // __HAMANAGER_H__