#pragma once

#include <thread>

class DBManager {
private:
    static DBManager* m_instance;
    
    std::thread m_thrd;
    
private:
    DBManager();
    
    static void thrdMain(DBManager* mgr);
    
public:
    virtual ~DBManager();
    
    DBManager* instance();
    void release();
    
    // for Realtime Call Siganl
    int insertCallInfo();
    int updateCallInfo();
    int insertRtSTTData();
};