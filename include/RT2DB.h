#pragma once

#include <stdint.h>

#include <string>
#include <queue>
#include <thread>
#include <mutex>

#include <log4cpp/Category.hh>

#include <zdb.h>


class RTSTTQueItem {
    uint32_t m_nDiaIdx;
	std::string m_sCallId;
	uint8_t m_nSpkNo;		// 1 ~ 9 : 통화자 구분값, 0 : 작업타입이 File인 경우
	std::string m_sSTTValue;

	uint64_t m_nBpos;
	uint64_t m_nEpos;

public:
	RTSTTQueItem(uint32_t idx, std::string callid, uint8_t spkno, std::string sttvalue, uint64_t bpos, uint64_t epos);
	virtual ~RTSTTQueItem();

	uint32_t getDiaIdx() { return m_nDiaIdx; }
	std::string& getCallId() { return m_sCallId; }
	uint8_t getSpkNo() { return m_nSpkNo; }
	std::string& getSTTValue() { return m_sSTTValue; }
    uint64_t getBpos() { return m_nBpos; }
    uint64_t getEpos() { return m_nEpos; }
};

class RT2DB {
private:
    static RT2DB* m_instance;
    
	bool m_bLiveFlag;

	std::queue< RTSTTQueItem* > m_qRtSttQue;
	std::thread m_thrd;
	mutable std::mutex m_mxQue;
    
    URL_T m_url;
    ConnectionPool_T m_pool;

    log4cpp::Category *m_Logger;
    
private:
    RT2DB(log4cpp::Category *logger);
	static void thrdMain(RT2DB* r2d);
	void insertRtSTTData(RTSTTQueItem* item);
    
public:
    virtual ~RT2DB();
    
    static RT2DB* instance(std::string dbtype, std::string dbhost, std::string dbport, std::string dbuser, std::string dbpw, std::string dbname, std::string charset, log4cpp::Category *logger);
    static void release();
    
    void setLogger(log4cpp::Category *logger) { m_Logger = logger; }
    
    // for Realtime Call Siganl
    int insertCallInfo(std::string callid, time_t stime);
    int updateCallInfo(std::string callid, time_t stime, bool end=false);
    void insertRtSTTData(uint32_t idx, std::string callid, uint8_t spkno, uint64_t spos, uint64_t epos, std::string stt);
};