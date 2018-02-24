#ifndef __HAMANAGER_H__
#define __HAMANAGER_H__


class HAManager {
private:
    static HAManager* m_instance;
    static bool m_bHAState; // Active: true, Standby: false
    
public:
    
private:
    HAManager();
    
public:
    virtual ~HAManager();
    
    static HAManager* instance();
    static void release();
};


#endif // __HAMANAGER_H__