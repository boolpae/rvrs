#pragma once

#include <stdint.h>
#include <string>
#include <queue>
#include <thread>
#include <mutex>

#include <log4cpp/Category.hh>

using namespace std;

class VRCManager;
class STTDeliver;

typedef struct _queItem {
	uint8_t flag;	// 통화 시작과 통화 중: 1, 마지막 데이터 또는 통화 종료: 0
	uint8_t spkNo;	// 통화자 번호(송신자 0, 수신자 1)
	uint32_t lenVoiceData;	// voiceData 길이
	uint8_t* voiceData;	// 디코딩된 3초 분량의 음성 데이터
} QueItem;

class VRClient
{
	VRCManager* m_Mgr;

	string m_sGearHost;
	uint16_t m_nGearPort;
	string m_sFname;
	string m_sCallId;
	volatile uint8_t m_nLiveFlag;	// Threading Class로서 객체 삭제를 thread에서 수행하도록 설계
	uint8_t m_cJobType;	// VRClient의 작업 타입(파일:F, 실시간:R)
	uint8_t m_nNumofChannel;

	queue< QueItem* > m_qRTQue;
	std::thread m_thrd;
	mutable std::mutex m_mxQue;
    
    STTDeliver *m_deliver;
    log4cpp::Category *m_Logger;

public:

public:
	VRClient(VRCManager* mgr, string& gearHost, uint16_t gearPort, string& fname, string& callid, uint8_t jobType, uint8_t noc, STTDeliver *deliver, log4cpp::Category *logger);
	void finish();

	string& getFname() { return m_sFname; }
	string& getCallId() { return m_sCallId; }

	void insertQueItem(QueItem* item);

private:
	virtual ~VRClient();
	static void thrdMain(VRClient* client);
};

