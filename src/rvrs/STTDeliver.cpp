

#include "STTDeliver.h"

#include <thread>
#include <iostream>
#include <fstream>

STTDeliver* STTDeliver::ms_instance = NULL;

STTDeliver::STTDeliver(log4cpp::Category *logger)
	: m_bLiveFlag(true), m_Logger(logger)
{
	std::cout << "\t[DEBUG] STTDeliver Constructed." << std::endl;
}


STTDeliver::~STTDeliver()
{
	std::cout << "\t[DEBUG] STTDeliver Destructed." << std::endl;
}

void STTDeliver::thrdMain(STTDeliver * dlv)
{
	std::lock_guard<std::mutex> *g;// (m_mxQue);
	STTQueItem* item;
	std::string sttFilename;

	while (dlv->m_bLiveFlag) {
		while (!dlv->m_qSttQue.empty()) {
			g = new std::lock_guard<std::mutex>(dlv->m_mxQue);
			item = dlv->m_qSttQue.front();
			dlv->m_qSttQue.pop();
			delete g;

			// item으로 로직 수행
			if (item->getJobType() == 'R') {
				sttFilename = item->getCallId();
				sttFilename += "_";
				sttFilename += std::to_string(item->getSpkNo());
				sttFilename += ".stt";
			}
			else {
				sttFilename = item->getCallId();
				sttFilename += ".stt";
			}
			std::ofstream sttresult(sttFilename, std::ios::out | std::ios::app);
			if (sttresult.is_open()) {
                if (item->getJobType() == 'R') {
                    sttresult << std::to_string(item->getBpos()) << std::endl;
                }
				sttresult << item->getSTTValue();
                if (item->getJobType() == 'R') {
                    sttresult << std::to_string(item->getEpos()) << std::endl;
                }
				sttresult.close();
			}

			delete item;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

// desc:	spkNo의 값이 0인 경우 실시간 STT 결과값이 아님(이 경우 jobtype의 값도 'F' 이어야 함)
//			실시간 STT 결과인 경우 jobtype : 'R' 이며 spkNo의 값도 1 이상의 값이어야 함
void STTDeliver::insertSTT(std::string callid, std::string stt, uint8_t spkNo, uint64_t bpos, uint64_t epos)
{
	insertSTT(new STTQueItem(callid, uint8_t('R'), spkNo, stt, bpos, epos));
}

void STTDeliver::insertSTT(std::string callid, std::string& stt, std::string filename)
{
	insertSTT(new STTQueItem(callid, uint8_t('F'), filename, stt));
}

void STTDeliver::insertSTT(STTQueItem * item)
{
	std::lock_guard<std::mutex> g(m_mxQue);
	m_qSttQue.push(item);
}

STTDeliver* STTDeliver::instance(log4cpp::Category *logger)
{
	if (ms_instance) return ms_instance;

	ms_instance = new STTDeliver(logger);

	ms_instance->m_thrd = std::thread(STTDeliver::thrdMain, ms_instance);

	return ms_instance;
}

void STTDeliver::release()
{
	ms_instance->m_bLiveFlag = false;

	ms_instance->m_thrd.join();

	delete ms_instance;
	ms_instance = NULL;
}

STTQueItem::STTQueItem(std::string callid, uint8_t jobtype, uint8_t spkno, std::string sttvalue, uint64_t bpos, uint64_t epos)
	:m_sCallId(callid), m_cJobType(jobtype), m_nSpkNo(spkno), m_sFilename(""), m_sSTTValue(sttvalue), m_nBpos(bpos), m_nEpos(epos)
{
}

STTQueItem::STTQueItem(std::string callid, uint8_t jobtype, std::string filename, std::string& sttvalue)
	:m_sCallId(callid), m_cJobType(jobtype), m_nSpkNo(0), m_sFilename(filename), m_sSTTValue(sttvalue)
{
}

STTQueItem::~STTQueItem()
{
}
