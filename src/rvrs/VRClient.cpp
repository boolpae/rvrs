
#include "VRClient.h"
#include "VRCManager.h"
#include "WorkTracer.h"
#include "STTDeliver.h"
#include "RT2DB.h"

#include <thread>
#include <iostream>
#include <fstream>

#include <string.h>


// For Gearman
#include <libgearman/gearman.h>

VRClient::VRClient(VRCManager* mgr, string& gearHost, uint16_t gearPort, int gearTimeout, string& fname, string& callid, uint8_t jobType, uint8_t noc, STTDeliver *deliver, log4cpp::Category *logger, RT2DB* r2d)
	: m_sGearHost(gearHost), m_nGearPort(gearPort), m_nGearTimeout(gearTimeout), m_sFname(fname), m_sCallId(callid), m_nLiveFlag(1), m_cJobType(jobType), m_nNumofChannel(noc), m_deliver(deliver), m_Logger(logger), m_r2d(r2d)
{
	m_Mgr = mgr;
	m_thrd = std::thread(VRClient::thrdMain, this);
	//thrd.detach();
	//printf("\t[DEBUG] VRClinet Constructed.\n");
    m_Logger->debug("VRClinet Constructed.");
}


VRClient::~VRClient()
{
	QueItem* item;
	while (!m_qRTQue.empty()) {
		item = m_qRTQue.front();
		m_qRTQue.pop();

		delete[] item->voiceData;
		delete item;
	}

	//printf("\t[DEBUG] VRClinet Destructed.\n");
    m_Logger->debug("VRClinet Destructed.");
}

void VRClient::finish()
{
	m_nLiveFlag = 0;
}

#define BUFLEN (16000 * 10 + 64)  //Max length of buffer

typedef struct _posPair {
    uint64_t bpos;
    uint64_t epos;
} PosPair;

void VRClient::thrdMain(VRClient* client) {

	QueItem* item;
	std::lock_guard<std::mutex> *g;
    gearman_client_st *gearClient;
    gearman_return_t ret;
    void *value = NULL;
    size_t result_size;
    gearman_return_t rc;
    PosPair stPos;
    std::vector< PosPair > vPos;
    
    char* pEndpos=NULL;
    std::size_t start,end;

    
    char buf[BUFLEN];
    uint16_t nHeadLen=0;
    
    for (int i=0; i<client->m_nNumofChannel; i++) {
        stPos.bpos = 0;
        stPos.epos = 0;
        vPos.push_back( stPos );
    }
    
    
    gearClient = gearman_client_create(NULL);
    if (!gearClient) {
        //printf("\t[DEBUG] VRClient::thrdMain() - ERROR (Failed gearman_client_create - %s)\n", client->m_sCallId.c_str());
        client->m_Logger->error("VRClient::thrdMain() - ERROR (Failed gearman_client_create - %s)", client->m_sCallId.c_str());

        WorkTracer::instance()->insertWork(client->m_sCallId, client->m_cJobType, WorkQueItem::PROCTYPE::R_FREE_WORKER);

        client->m_thrd.detach();
        delete client;
    }
    
    ret= gearman_client_add_server(gearClient, client->m_sGearHost.c_str(), client->m_nGearPort);
    if (gearman_failed(ret))
    {
        //printf("\t[DEBUG] VRClient::thrdMain() - ERROR (Failed gearman_client_add_server - %s)\n", client->m_sCallId.c_str());
        client->m_Logger->error("VRClient::thrdMain() - ERROR (Failed gearman_client_add_server - %s)", client->m_sCallId.c_str());

        WorkTracer::instance()->insertWork(client->m_sCallId, client->m_cJobType, WorkQueItem::PROCTYPE::R_FREE_WORKER);

        client->m_thrd.detach();
        delete client;
    }
    

	// m_cJobType에 따라 작업 형태를 달리해야 한다. 
	if (client->m_cJobType == 'R') {
        uint32_t diaNumber=1;   // DB 실시간 STT 테이블에 저장될 호(Call)단위 Index 값
		// 실시간의 경우 통화가 종료되기 전까지 Queue에서 입력 데이터를 받아 처리
		// FILE인 경우 기존과 동일하게 filename을 전달하는 방법 이용
        if (client->m_nGearTimeout) {
            gearman_client_set_timeout(gearClient, client->m_nGearTimeout);
        }
        
#if 0 // for DEBUG
		std::string filename = client->m_sCallId + std::string("_") + std::to_string(client->m_nNumofChannel) + std::string(".pcm");
		std::ofstream pcmFile;
		pcmFile.open(filename, ios::out | ios::app | ios::binary);
#endif
		while (client->m_nLiveFlag)
		{
			while (!client->m_qRTQue.empty()) {
				g = new std::lock_guard<std::mutex>(client->m_mxQue);
				item = client->m_qRTQue.front();
				client->m_qRTQue.pop();
				delete g;

                vPos[item->spkNo -1].epos += item->lenVoiceData;
				// queue에서 가져온 item을 STT 하는 로직을 아래에 코딩한다.
				// call-id + item->spkNo => call-id for rt-stt
                memset(buf, 0, sizeof(buf));
                if (!item->flag) {
                    sprintf(buf, "%s_%d|%s|", client->m_sCallId.c_str(), item->spkNo, "LAST");
                }
                else {
                    sprintf(buf, "%s_%d|%s|", client->m_sCallId.c_str(), item->spkNo, "NOLA");
                }
                nHeadLen = strlen(buf);
                memcpy(buf+nHeadLen, (const void*)item->voiceData, item->lenVoiceData);
                
                value= gearman_client_do(gearClient, client->m_sFname.c_str(), NULL, 
                                                (const void*)buf, (nHeadLen + item->lenVoiceData),
                                                &result_size, &rc);
#if 0 // for DEBUG
				if (pcmFile.is_open()) {
					pcmFile.write((const char*)buf+nHeadLen, item->lenVoiceData);
				}
#endif
                if (gearman_success(rc))
                {
                    // Make use of value
                    if (value) {
                        // std::cout << "DEBUG : value(" << (char *)value << ") : size(" << result_size << ")" << std::endl;
                        pEndpos = strchr((char*)value, '|');
                        if (pEndpos) {
                            sscanf(pEndpos, "|%lu|%lu", &start, &end);
                            //client->m_Logger->debug("VRClient::thrdMain(%s) - start_pos(%lu), end_pos(%lu).", client->m_sCallId.c_str(), start, end);
                            *pEndpos = 0;
                        }
                        // to DB
                        if (client->m_r2d) {
                            client->m_r2d->insertRtSTTData(diaNumber, client->m_sCallId, item->spkNo, pEndpos ? start : vPos[item->spkNo -1].bpos/160, pEndpos ? end : vPos[item->spkNo -1].epos/160, std::string((const char*)value));
                        }
                        //STTDeliver::instance(client->m_Logger)->insertSTT(client->m_sCallId, std::string((const char*)value), item->spkNo, vPos[item->spkNo -1].bpos, vPos[item->spkNo -1].epos);
                        // to STTDeliver(file)
                        if (client->m_deliver) {
                            client->m_deliver->insertSTT(client->m_sCallId, std::string((const char*)value), item->spkNo, pEndpos ? start : vPos[item->spkNo -1].bpos/160, pEndpos ? end : vPos[item->spkNo -1].epos/160);
                        }
                        free(value);
                        
                        diaNumber++;
                    }
                }
                else if (gearman_failed(rc)){
                    client->m_Logger->error("VRClient::thrdMain(%s) - failed gearman_client_do(). [%d : %d], timeout(%d)", client->m_sCallId.c_str(), vPos[item->spkNo -1].bpos, vPos[item->spkNo -1].epos, client->m_nGearTimeout);
                }
                
                vPos[item->spkNo -1].bpos = vPos[item->spkNo -1].epos + 1;

				if (!item->flag) {	// 호가 종료되었음을 알리는 flag, 채널 갯수와 flag(0)이 들어온 갯수를 비교해야한다.
					//printf("\t[DEBUG] VRClient::thrdMain(%s) - final item delivered.\n", client->m_sCallId.c_str());
                    client->m_Logger->debug("VRClient::thrdMain(%s, %d) - final item delivered.", client->m_sCallId.c_str(), item->spkNo);
					if (!(--client->m_nNumofChannel)) {
						client->m_Mgr->removeVRC(client->m_sCallId);
						if ( item->voiceData != NULL ) delete[] item->voiceData;
						delete item;

						break;
					}
				}

				delete[] item->voiceData;
				delete item;
				// 예외 발생 시 처리 내용 : VDCManager의 removeVDC를 호출할 수 있어야 한다. - 이 후 VRClient는 item->flag(0)에 대해서만 처리한다.
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
#if 0 // for DEBUG
		if (pcmFile.is_open()) pcmFile.close();
#endif
	}
	// 파일(배치)를 위한 작업 수행 시
	else {
		client->m_Mgr->removeVRC(client->m_sCallId);
	}
    
    gearman_client_free(gearClient);
    vPos.clear();

	WorkTracer::instance()->insertWork(client->m_sCallId, client->m_cJobType, WorkQueItem::PROCTYPE::R_FREE_WORKER);

	client->m_thrd.detach();
	delete client;
}

void VRClient::insertQueItem(QueItem* item)
{
	std::lock_guard<std::mutex> g(m_mxQue);
	m_qRTQue.push(item);
}