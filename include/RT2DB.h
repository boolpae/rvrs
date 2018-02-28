#pragma once

#include <string>

#include <log4cpp/Category.hh>

#include <zdb.h>

class RT2DB {
private:
    static RT2DB* m_instance;
    
    URL_T m_url;
    ConnectionPool_T m_pool;

    log4cpp::Category *m_Logger;
    
private:
    RT2DB();
    
public:
    virtual ~RT2DB();
    
    static RT2DB* instance(std::string dbtype, std::string dbhost, std::string dbport, std::string dbuser, std::string dbpw, std::string dbname);
    static void release();
    
    void setLogger(log4cpp::Category *logger) { m_Logger = logger; }
    
    // for Realtime Call Siganl
    int insertCallInfo(std::string callid, time_t stime);
    int updateCallInfo(std::string callid, time_t stime);
    int insertRtSTTData(uint32_t idx, std::string callid, uint8_t spkno, uint64_t spos, uint64_t epos, std::string stt);
};