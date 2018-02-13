#pragma once

#include <stdint.h>
#include <vector>

#define PROTO_AUTH_TOKEN "RT-STT"
#define PROTO_AUTH_TOKEN_LEN 6

#define CALL_BEG_PACKET_LEN 112
#define CALL_END_PACKET_LEN 105
#define CALL_PACKET_HEADER_LEN 8
#define CALL_BEG_PACKET_BODY_LEN (CALL_BEG_PACKET_LEN - CALL_PACKET_HEADER_LEN)
#define CALL_END_PACKET_BODY_LEN (CALL_END_PACKET_LEN - CALL_PACKET_HEADER_LEN)

namespace Protocol {

	class CallSignal
	{
		uint16_t m_nPacketSize;
		uint8_t* m_Packet;

		// 요청 패킷의 크기 : 264 bits
		uint16_t pacSize;		// 패킷 전체 길이 값
		uint8_t pacFlag;		// 호 시작('B'), 호 종료('E')
		uint8_t pacCallId[33];	// unique한 값, 키로 사용되는 값이며 문자열
		uint8_t pacUdpCnt;		// 실시간 음성 전달을 위해 요청한 채널 갯수
        int32_t pacSampleRate;
        uint8_t pacChnCnt;
        uint8_t pacEnc;
		uint8_t pacFingerPrint[65];	// 패킷의 조작 여부를 판별하기 위한 문자열 값(요청하는 노드에서 설정되는 값)
		uint8_t pacRes[3];		// 응답 값 - HTTP 프로토콜의 응답 코드와 동일한 값 사용

	public:
		CallSignal();
		virtual ~CallSignal();

		void init();
		uint16_t parsePacket(uint8_t* packet);

		int16_t makePacket(uint8_t flag);
		int16_t makePacket(uint8_t* packet, uint16_t packetlen);
		int16_t makePacket(uint8_t* packet, uint16_t packetlen, uint16_t resCode);
		int16_t makePacket(uint8_t* packet, uint16_t packetlen, std::vector< uint16_t > &vPorts);

		uint16_t getPacketSize() { return m_nPacketSize; }
		uint8_t* getPacket() { return m_Packet; }

		uint8_t getPacketFlag() { return pacFlag; }
		const char* getCallId() { return (const char*)pacCallId; }
		uint8_t getUdpCnt() { return pacUdpCnt; }

		void setPacFlag(uint8_t flag) { pacFlag = flag; }
		void setPacCallId(uint8_t *callid, uint16_t len);
		void setPacUdpCnt(uint8_t cnt) { pacUdpCnt = cnt; }
        void setPacSampleRate(int32_t rate) { pacSampleRate = rate; }
        void setPacChnCnt(uint8_t cnt) { pacChnCnt = cnt; }
        void setPacEnc(uint8_t enc) { pacEnc = enc; }
		void setFingerPrint(uint8_t *fprint, uint16_t len);

		void printPacketInfo();
	};

}

