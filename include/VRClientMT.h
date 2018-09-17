#pragma once

#include <stdint.h>
#include <string>
#include <queue>
#include <thread>
#include <mutex>

#include <log4cpp/Category.hh>

#ifdef USE_XREDIS
#include "xRedisClient.h"

using namespace xrc;
#endif

using namespace std;

class VRCManager;
class FileHandler;
class DBHandler;

typedef struct _queItem {
	uint8_t flag;	// 통화 시작: 2, 과 통화 중: 1, 마지막 데이터 또는 통화 종료: 0
	uint8_t spkNo;	// 통화자 번호(송신자 0, 수신자 1)
	uint32_t lenVoiceData;	// voiceData 길이
	uint8_t* voiceData;	// 디코딩된 3초 분량의 음성 데이터
} QueItem;

class VRClientMT
{
	VRCManager* m_Mgr;

	string m_sGearHost;
	uint16_t m_nGearPort;
    int m_nGearTimeout;
	string m_sFname;
	string m_sCallId;
	string m_sCounselCode;
	volatile uint8_t m_nLiveFlag;	// Threading Class로서 객체 삭제를 thread에서 수행하도록 설계
	uint8_t m_cJobType;	// VRClient의 작업 타입(파일:F, 실시간:R)
	uint8_t m_nNumofChannel;

	volatile uint8_t m_RxState;
	volatile uint8_t m_TxState;

	queue< QueItem* > m_qRTQue;
	std::thread m_thrd;
	mutable std::mutex m_mxQue;

	queue< QueItem* > m_qRXQue;
	std::thread m_thrdRx;
	mutable std::mutex m_mxRxQue;

	queue< QueItem* > m_qTXQue;
	std::thread m_thrdTx;
	mutable std::mutex m_mxTxQue;
    
    FileHandler *m_deliver;
    log4cpp::Category *m_Logger;
    DBHandler* m_s2d;
    
    bool m_is_save_pcm;
    string m_pcm_path;

    size_t m_framelen;

public:

public:
	VRClientMT(VRCManager* mgr, string& gearHost, uint16_t gearPort, int gearTimeout, string& fname, string& callid, string& counselcode, uint8_t jobType, uint8_t noc, FileHandler *deliver, /*log4cpp::Category *logger,*/ DBHandler* s2d, bool is_save_pcm, string pcm_path, size_t framelen);
	void finish();

	string& getFname() { return m_sFname; }
	string& getCallId() { return m_sCallId; }
	string& getCounselCode() { return m_sCounselCode; }

	void insertQueItem(QueItem* item);

#ifdef USE_XREDIS
	xRedisClient& getXRdedisClient();
#endif

private:
	virtual ~VRClientMT();
	static void thrdMain(VRClientMT* client);
	static void thrdRxProcess(VRClientMT* client);
	static void thrdTxProcess(VRClientMT* client);

	volatile uint32_t ms_diaNumber;
	uint32_t getDiaNumber();

	uint64_t m_totalVoiceDataLen;
	void setTotalVoiceDataLen(uint64_t len) { m_totalVoiceDataLen = len; }
	uint64_t getTotalVoiceDataLen() { return m_totalVoiceDataLen; }

	std::string m_svrnm;
	void setServerName(std::string svr_nm) { m_svrnm = svr_nm; }
	std::string getServerName() { return m_svrnm; }
};

