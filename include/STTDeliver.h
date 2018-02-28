﻿#pragma once

#include <stdint.h>

#include <string>
#include <queue>
#include <thread>
#include <mutex>

#include <log4cpp/Category.hh>

class RT2DB;

class STTQueItem {
	std::string m_sCallId;
	uint8_t m_cJobType;		// 'R': 실기간, 'F': 파일
	uint8_t m_nSpkNo;		// 1 ~ 9 : 통화자 구분값, 0 : 작업타입이 File인 경우
	std::string m_sFilename;
	std::string m_sSTTValue;

	uint64_t m_nBpos;
	uint64_t m_nEpos;

public:
	STTQueItem(std::string callid, uint8_t jobtype, uint8_t spkno, std::string sttvalue, uint64_t bpos, uint64_t epos);
	STTQueItem(std::string callid, uint8_t jobtype, std::string filename, std::string& sttvalue);
	virtual ~STTQueItem();

	std::string& getCallId() { return m_sCallId; }
	uint8_t getJobType() { return m_cJobType; }
	uint8_t getSpkNo() { return m_nSpkNo; }
	std::string& getFilename() { return m_sFilename; }
	std::string& getSTTValue() { return m_sSTTValue; }
    uint64_t getBpos() { return m_nBpos; }
    uint64_t getEpos() { return m_nEpos; }
};

class STTDeliver
{
	static STTDeliver* ms_instance;
	
	bool m_bLiveFlag;

	std::queue< STTQueItem* > m_qSttQue;
	std::thread m_thrd;
	mutable std::mutex m_mxQue;
    
    log4cpp::Category *m_Logger;
    
    RT2DB* m_rt2db;

public:
	static STTDeliver* instance(log4cpp::Category *logger, RT2DB* rt2db);
	static void release();

	void insertSTT(std::string callid, std::string stt, uint8_t spkNo, uint64_t bpos, uint64_t epos);		// for Realtime
	void insertSTT(std::string callid, std::string& stt, std::string filename);	// for FILE, BATCH

private:
	STTDeliver(log4cpp::Category *logger, RT2DB* rt2db);
	virtual ~STTDeliver();

	static void thrdMain(STTDeliver* dlv);

	void insertSTT(STTQueItem* item);
};

