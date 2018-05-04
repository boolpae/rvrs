

#include "STT2File.h"

#include <thread>
#include <iostream>
#include <fstream>

#include <iconv.h>
#include <string.h>
#include <unistd.h>

STT2File* STT2File::ms_instance = NULL;

STT2File::STT2File(std::string path, log4cpp::Category *logger)
	: m_bLiveFlag(true), m_sResultPath(path), m_Logger(logger)
{
	m_Logger->debug("STT2File Constructed.");
}


STT2File::~STT2File()
{
	m_Logger->debug("STT2File Destructed.");
}

void STT2File::thrdMain(STT2File * dlv)
{
	std::lock_guard<std::mutex> *g;// (m_mxQue);
	STTQueItem* item;
	std::string sttFilename;
    int ret=0;
    char *utf_buf = NULL;
    size_t in_size, out_size;
    iconv_t it;
    char *input_buf_ptr = NULL;
    char *output_buf_ptr = NULL;

	while (dlv->m_bLiveFlag) {
		while (!dlv->m_qSttQue.empty()) {
			g = new std::lock_guard<std::mutex>(dlv->m_mxQue);
			item = dlv->m_qSttQue.front();
			dlv->m_qSttQue.pop();
			delete g;

            in_size = item->getSTTValue().size();
            out_size = item->getSTTValue().size() * 2;
            utf_buf = (char *)malloc(out_size);
            
            if (utf_buf) {
                memset(utf_buf, 0, out_size);

                input_buf_ptr = (char *)item->getSTTValue().c_str();
                output_buf_ptr = utf_buf;

                it = iconv_open("UTF-8", "EUC-KR");

                ret = iconv(it, &input_buf_ptr, &in_size, &output_buf_ptr, &out_size);
                 
                iconv_close(it);
            }
            else {
                utf_buf = nullptr;
                ret = -1;
            }

			// item으로 로직 수행
			if (item->getJobType() == 'R') {
				sttFilename = dlv->m_sResultPath + "/" + item->getCallId();
                /*
				sttFilename += "_";
				sttFilename += std::to_string(item->getSpkNo());
                 */
				sttFilename += ".stt";
			}
			else {
				sttFilename = item->getCallId();
				sttFilename += ".stt";
			}
			std::ofstream sttresult(sttFilename, std::ios::out | std::ios::app);
			if (sttresult.is_open()) {
                if (item->getJobType() == 'R') {
                    if (item->getSpkNo() == 1) {
                        sttresult << "<< Counselor >> : ";
                    }
                    else {
                        sttresult << "<< Customer >> : ";
                    }
                    sttresult << std::to_string(item->getBpos()) << " - " << std::to_string(item->getEpos()) << std::endl;
                }
				sttresult << ((ret == -1) ? item->getSTTValue() : utf_buf);//item->getSTTValue();
                /*
                if (item->getJobType() == 'R') {
                    sttresult << std::to_string(item->getEpos()) << std::endl;
                }
                 */
                sttresult << std::endl;
				sttresult.close();
			}
            if (utf_buf) free(utf_buf);
			delete item;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
}

// desc:	spkNo의 값이 0인 경우 실시간 STT 결과값이 아님(이 경우 jobtype의 값도 'F' 이어야 함)
//			실시간 STT 결과인 경우 jobtype : 'R' 이며 spkNo의 값도 1 이상의 값이어야 함
void STT2File::insertSTT(std::string callid, std::string stt, uint8_t spkNo, uint64_t bpos, uint64_t epos)
{
	insertSTT(new STTQueItem(callid, uint8_t('R'), spkNo, stt, bpos, epos));
}

void STT2File::insertSTT(std::string callid, std::string& stt, std::string filename)
{
	insertSTT(new STTQueItem(callid, uint8_t('F'), filename, stt));
}

void STT2File::insertSTT(STTQueItem * item)
{
	std::lock_guard<std::mutex> g(m_mxQue);
	m_qSttQue.push(item);
}

STT2File* STT2File::instance(std::string path, log4cpp::Category *logger)
{
	if (ms_instance) return ms_instance;
    
    if ( ::access(path.c_str(), 0) ) {
        std::string cmd = "mkdir -p ";
        cmd += path;
        std::system(cmd.c_str());
        
        if ( ::access(path.c_str(), 0) ) {
            logger->error("STT2File::instance - failed create path : %s", path.c_str());
            return nullptr;
        }
    }

	ms_instance = new STT2File(path, logger);

	ms_instance->m_thrd = std::thread(STT2File::thrdMain, ms_instance);

	return ms_instance;
}

void STT2File::release()
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
