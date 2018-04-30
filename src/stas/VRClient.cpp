
#include "VRClient.h"
#include "VRCManager.h"
#include "WorkTracer.h"
#include "STTDeliver.h"
#include "RT2DB.h"
#include "HAManager.h"

#include <thread>
#include <iostream>
#include <fstream>

#include <string.h>

#include <boost/algorithm/string/replace.hpp>

// For Gearman
#include <libgearman/gearman.h>

#ifdef FAD_FUNC

#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sndfile.h>

#include <unistd.h>
#include <sys/mman.h>

#include <string.h>
#include <stdlib.h>

#include <fvad.h>



#define WAVE_FORMAT_UNKNOWN      0X0000;
#define WAVE_FORMAT_PCM          0X0001;
#define WAVE_FORMAT_MS_ADPCM     0X0002;
#define WAVE_FORMAT_IEEE_FLOAT   0X0003;
#define WAVE_FORMAT_ALAW         0X0006;
#define WAVE_FORMAT_MULAW        0X0007;
#define WAVE_FORMAT_IMA_ADPCM    0X0011;
#define WAVE_FORMAT_YAMAHA_ADPCM 0X0016;
#define WAVE_FORMAT_GSM          0X0031;
#define WAVE_FORMAT_ITU_ADPCM    0X0040;
#define WAVE_FORMAT_MPEG         0X0050;
#define WAVE_FORMAT_EXTENSIBLE   0XFFFE;



typedef struct
{
	unsigned char ChunkID[4];    // Contains the letters "RIFF" in ASCII form
	unsigned int ChunkSize;      // This is the size of the rest of the chunk following this number
	unsigned char Format[4];     // Contains the letters "WAVE" in ASCII form
} RIFF;

//-------------------------------------------
// [Channel]
// - streo     : [left][right]
// - 3 channel : [left][right][center]
// - quad      : [front left][front right][rear left][reat right]
// - 4 channel : [left][center][right][surround]
// - 6 channel : [left center][left][center][right center][right][surround]
//-------------------------------------------
typedef struct
{
	unsigned char  ChunkID[4];    // Contains the letters "fmt " in ASCII form
	unsigned int   ChunkSize;     // 16 for PCM.  This is the size of the rest of the Subchunk which follows this number.
	unsigned short AudioFormat;   // PCM = 1
	unsigned short NumChannels;   // Mono = 1, Stereo = 2, etc.
	unsigned int   SampleRate;    // 8000, 44100, etc.
	unsigned int   AvgByteRate;   // SampleRate * NumChannels * BitsPerSample/8
	unsigned short BlockAlign;    // NumChannels * BitsPerSample/8
	unsigned short BitPerSample;  // 8 bits = 8, 16 bits = 16, etc
} FMT;


typedef struct
{
	char          ChunkID[4];    // Contains the letters "data" in ASCII form
	unsigned int  ChunkSize;     // NumSamples * NumChannels * BitsPerSample/8
} DATA;


typedef struct
{
	RIFF Riff;
	FMT	 Fmt;
	DATA Data;
} WAVE_HEADER;

#endif // FAD_FUNC

VRClient::VRClient(VRCManager* mgr, string& gearHost, uint16_t gearPort, int gearTimeout, string& fname, string& callid, uint8_t jobType, uint8_t noc, STTDeliver *deliver, log4cpp::Category *logger, RT2DB* r2d, bool is_save_pcm, string pcm_path, size_t framelen)
	: m_sGearHost(gearHost), m_nGearPort(gearPort), m_nGearTimeout(gearTimeout), m_sFname(fname), m_sCallId(callid), m_nLiveFlag(1), m_cJobType(jobType), m_nNumofChannel(noc), m_deliver(deliver), m_Logger(logger), m_r2d(r2d), m_is_save_pcm(is_save_pcm), m_pcm_path(pcm_path), m_framelen(framelen)
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

#define WAV_HEADER_SIZE 44
#define WAV_BUFF_SIZE 19200
#define MM_SIZE (WAV_HEADER_SIZE + WAV_BUFF_SIZE)

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
#ifndef FAD_FUNC    
    char* pEndpos=NULL;
    std::size_t start,end;
#endif
    
    char buf[BUFLEN];
    uint16_t nHeadLen=0;
    
#ifdef FAD_FUNC
    uint8_t *cpBuf = NULL;
    size_t posBuf = 0;
    std::vector<uint8_t> vBuff[2];
    size_t sframe[2];
    size_t eframe[2];

    WAVE_HEADER wh;

    vBuff[0].reserve(16044);
    vBuff[1].reserve(16044);
#endif // FAD_FUNC
    
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
        return;
    }
    
    ret= gearman_client_add_server(gearClient, client->m_sGearHost.c_str(), client->m_nGearPort);
    if (gearman_failed(ret))
    {
        //printf("\t[DEBUG] VRClient::thrdMain() - ERROR (Failed gearman_client_add_server - %s)\n", client->m_sCallId.c_str());
        client->m_Logger->error("VRClient::thrdMain() - ERROR (Failed gearman_client_add_server - %s)", client->m_sCallId.c_str());

        WorkTracer::instance()->insertWork(client->m_sCallId, client->m_cJobType, WorkQueItem::PROCTYPE::R_FREE_WORKER);

        client->m_thrd.detach();
        delete client;
        return;
    }
    

	// m_cJobType에 따라 작업 형태를 달리해야 한다. 
	if (client->m_cJobType == 'R') {
        uint32_t diaNumber=1;   // DB 실시간 STT 테이블에 저장될 호(Call)단위 Index 값
#ifndef FAD_FUNC
        const char* srcBuff;
        const char* dstBuff;
        uint32_t srcLen, dstLen;
        string tmpStt[2];
        uint32_t sttIdx;
        
        tmpStt[0] = "";
        tmpStt[1] = "";
#else
        int mmfd=0;
        uint8_t *pmmap;
        SF_INFO in_info = {0};
        SNDFILE *in_sf = NULL;
        Fvad *vad = NULL;
        int vadres;
        double *buf0 = NULL;
        int16_t *buf1 = NULL;
        
        memset(&wh, 0, sizeof(WAVE_HEADER));
        memcpy(wh.Riff.ChunkID, "RIFF", 4);
        memcpy(wh.Riff.Format, "WAVE", 4);

        memcpy(wh.Fmt.ChunkID, "fmt ", 4);
        wh.Fmt.ChunkSize = 16;
        wh.Fmt.AudioFormat = 1;
        wh.Fmt.NumChannels = 1;
        wh.Fmt.SampleRate = 8000;
        wh.Fmt.AvgByteRate = 8000 * 1 * 2;//unsigned int   AvgByteRate;   // SampleRate * NumChannels * BitsPerSample/8
        wh.Fmt.BlockAlign = 1 * 2;//unsigned short BlockAlign;    // NumChannels * BitsPerSample/8
        wh.Fmt.BitPerSample = 16;//unsigned short BitPerSample;  // 8 bits = 8, 16 bits = 16, etc

        memcpy(wh.Data.ChunkID, "data", 4);// char          ChunkID[4];    // Contains the letters "data" in ASCII form
        wh.Data.ChunkSize = 8000 * 1 * 2;//unsigned int  ChunkSize;     // NumSamples * NumChannels * BitsPerSample/8
        
        sprintf(buf, "/dev/shm/%s.mm", client->m_sCallId.c_str());

        mmfd = open(buf, O_RDWR|O_CREAT, 0666);
        if (mmfd < 0) {
            //printf("\t[DEBUG] VRClient::thrdMain() - ERROR (Failed gearman_client_add_server - %s)\n", client->m_sCallId.c_str());
            client->m_Logger->error("VRClient::thrdMain() - ERROR (Failed open(%s))", buf);

            WorkTracer::instance()->insertWork(client->m_sCallId, client->m_cJobType, WorkQueItem::PROCTYPE::R_FREE_WORKER);

            client->m_thrd.detach();
            delete client;
            return;
        }
        ftruncate(mmfd, MM_SIZE);

        pmmap = (uint8_t *)mmap(0, MM_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mmfd, 0);
        if(pmmap == (void *)-1)
        {
            client->m_Logger->error("VRClient::thrdMain() - ERROR (Failed mmap(%d))", mmfd);

            WorkTracer::instance()->insertWork(client->m_sCallId, client->m_cJobType, WorkQueItem::PROCTYPE::R_FREE_WORKER);

            client->m_thrd.detach();
            delete client;
            return;
        }

        memcpy(pmmap, &wh, sizeof(WAVE_HEADER));

        in_sf = sf_open_fd(mmfd, SFM_READ, &in_info, true);
        if (!in_sf) {
            client->m_Logger->error("VRClient::thrdMain() - ERROR (Failed sf_open_fd(%d))", mmfd);

            WorkTracer::instance()->insertWork(client->m_sCallId, client->m_cJobType, WorkQueItem::PROCTYPE::R_FREE_WORKER);

            munmap(pmmap, MM_SIZE);
            if (mmfd>0) close(mmfd);

            client->m_thrd.detach();
            delete client;
            return;
        }

        vad = fvad_new();
        if (!vad || (fvad_set_sample_rate(vad, in_info.samplerate) < 0)) {
            client->m_Logger->error("VRClient::thrdMain() - ERROR (Failed fvad_new, fvad_set_sample_rate(%s))", buf);

            WorkTracer::instance()->insertWork(client->m_sCallId, client->m_cJobType, WorkQueItem::PROCTYPE::R_FREE_WORKER);

            munmap(pmmap, MM_SIZE);
            if (mmfd>0) close(mmfd);

            client->m_thrd.detach();
            delete client;
            return;
        }

#endif // FAD_FUNC

		// 실시간의 경우 통화가 종료되기 전까지 Queue에서 입력 데이터를 받아 처리
		// FILE인 경우 기존과 동일하게 filename을 전달하는 방법 이용
        if (client->m_nGearTimeout) {
            gearman_client_set_timeout(gearClient, client->m_nGearTimeout);
        }
        
#if 0 // for DEBUG
		std::string filename = client->m_pcm_path + "/" + client->m_sCallId + std::string("_") + std::to_string(client->m_nNumofChannel) + std::string(".pcm");
		std::ofstream pcmFile;
        if (client->m_is_save_pcm)
            pcmFile.open(filename, ios::out | ios::app | ios::binary);
#endif

#ifdef FAD_FUNC
        // open file(mmap);
        // write wav heaer to file(mmap);
        vBuff[0].clear();
        vBuff[1].clear();
        sframe[0] = 0;
        sframe[1] = 0;
        eframe[0] = 0;
        eframe[1] = 0;

        buf0 = (double *)malloc(client->m_framelen * sizeof *buf0);
        buf1 = (int16_t *)malloc(client->m_framelen * sizeof *buf1);
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
                else if (item->flag == 2) {
                    sprintf(buf, "%s_%d|%s|", client->m_sCallId.c_str(), item->spkNo, "FIRS");
                }
                else {
                    sprintf(buf, "%s_%d|%s|", client->m_sCallId.c_str(), item->spkNo, "NOLA");
                }
                nHeadLen = strlen(buf);

#ifdef FAD_FUNC                
                if (vBuff[item->spkNo -1].size() == 0) {
                    for(int i=0; i<nHeadLen; i++) {
                        vBuff[item->spkNo-1].push_back(buf[i]);
                    }
                }
                
                // write pcm data to opened file(mmap);
                memset(pmmap+WAV_HEADER_SIZE, 0, WAV_BUFF_SIZE);
                memcpy(pmmap+WAV_HEADER_SIZE, (const void*)item->voiceData, item->lenVoiceData);
#endif // FAD_FUNC
                
                memcpy(buf+nHeadLen, (const void*)item->voiceData, item->lenVoiceData);
                
                if (client->m_is_save_pcm) {
                    std::string spker = (item->spkNo == 1)?std::string("r"):std::string("l");
                    std::string filename = client->m_pcm_path + "/" + client->m_sCallId + std::string("_") + /*std::to_string(client->m_nNumofChannel)*/spker + std::string(".pcm");
                    std::ofstream pcmFile;

                    pcmFile.open(filename, ios::out | ios::app | ios::binary);
                    if (pcmFile.is_open()) {
                        pcmFile.write((const char*)buf+nHeadLen, item->lenVoiceData);
                        pcmFile.close();
                    }
                }
                
#ifdef FAD_FUNC
                cpBuf = (uint8_t *)item->voiceData;
                posBuf = 0;
                // check vad!, by loop()
                // if finish check vad and vBuff is no empty, send buff to VR by gearman
                // vadres == 1 vBuff[item->spkNo-1].push_back();
                // vadres == 0 and vBuff[item->spkNo-1].size() > 0 then send buff to gearman
                while ((item->lenVoiceData > client->m_framelen*2) && (sf_read_double(in_sf, buf0, client->m_framelen) == (ssize_t)client->m_framelen)) {

                    eframe[item->spkNo-1] += 20;
                    // Convert the read samples to int16
                    for (size_t i = 0; i < client->m_framelen; i++)
                        buf1[i] = buf0[i] * INT16_MAX;

                    vadres = fvad_process(vad, buf1, client->m_framelen);
                    if (vadres < 0) {
                        continue;
                    }

                    if (vadres > 0) {
                        for(size_t i=0; i<(client->m_framelen*2); i++) {
                            vBuff[item->spkNo-1].push_back(cpBuf[i]);
                            
                        }
                    }
                    
                    if (!vadres && (vBuff[item->spkNo-1].size()<=nHeadLen)) {
                        // start ms
                        sframe[item->spkNo-1] = eframe[item->spkNo-1] - 20;
                    }

                    if (!vadres && (vBuff[item->spkNo-1].size()>nHeadLen)) {
                        // send buff to gearman
                        client->m_Logger->debug("VRClient::thrdMain(%s) - send buffer buff_len(%lu), spos(%lu), epos(%lu)", client->m_sCallId.c_str(), vBuff[item->spkNo-1].size(), sframe[item->spkNo-1], eframe[item->spkNo-1]);
                        value= gearman_client_do(gearClient, client->m_sFname.c_str(), NULL, 
                                                        (const void*)&vBuff[item->spkNo-1][0], vBuff[item->spkNo-1].size(),
                                                        &result_size, &rc);
                        if (gearman_success(rc))
                        {
                            // Make use of value
                            if (value) {
                                // std::cout << "DEBUG : value(" << (char *)value << ") : size(" << result_size << ")" << std::endl;
                                //client->m_Logger->debug("VRClient::thrdMain(%s) - sttIdx(%d)\nsrc(%s)\ndst(%s)", client->m_sCallId.c_str(), sttIdx, srcBuff, dstBuff);

                                // to DB
                                if (client->m_r2d) {
                                    client->m_r2d->insertRtSTTData(diaNumber, client->m_sCallId, item->spkNo, sframe[item->spkNo -1], eframe[item->spkNo -1], boost::replace_all_copy(std::string((const char*)value), "\n", " "));
                                }
                                //STTDeliver::instance(client->m_Logger)->insertSTT(client->m_sCallId, std::string((const char*)value), item->spkNo, vPos[item->spkNo -1].bpos, vPos[item->spkNo -1].epos);
                                // to STTDeliver(file)
                                if (client->m_deliver) {
                                    client->m_deliver->insertSTT(client->m_sCallId, boost::replace_all_copy(std::string((const char*)value), "\n", " "), item->spkNo, sframe[item->spkNo -1], eframe[item->spkNo -1]);
                                }

                                free(value);
                                
                                diaNumber++;
                            }
                        }
                        else if (gearman_failed(rc)){
                            client->m_Logger->error("VRClient::thrdMain(%s) - failed gearman_client_do(). [%lu : %lu], timeout(%d)", client->m_sCallId.c_str(), sframe[item->spkNo -1], eframe[item->spkNo -1], client->m_nGearTimeout);
                        }

                        // and clear buff, set msg header
                        vBuff[item->spkNo-1].clear();

                        for(size_t i=0; i<nHeadLen; i++) {
                            vBuff[item->spkNo-1].push_back(buf[i]);
                        }
                    }
                    
                    posBuf += (client->m_framelen*2);
                    item->lenVoiceData = item->lenVoiceData - posBuf;
                    cpBuf = (uint8_t *)(item->voiceData + posBuf);

                }

                sf_seek(in_sf, 0L, SEEK_SET);
#else // FAD_FUNC

                value= gearman_client_do(gearClient, client->m_sFname.c_str(), NULL, 
                                                (const void*)buf, (nHeadLen + item->lenVoiceData),
                                                &result_size, &rc);
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
#if 1
                        sttIdx = 0;
                        srcBuff = tmpStt[item->spkNo-1].c_str();
                        srcLen = strlen(srcBuff);
                        dstBuff = (const char*)value;
                        dstLen = strlen(dstBuff);
                        //client->m_Logger->debug("VRClient::thrdMain(%s) - sttIdx(%d)\nsrc(%s)\ndst(%s)", client->m_sCallId.c_str(), sttIdx, srcBuff, dstBuff);

                        if (srcLen <= dstLen) {
                            for(sttIdx=0; sttIdx<srcLen; sttIdx++) {
                                if (!memcmp(srcBuff, dstBuff, srcLen-sttIdx)) {
                                    break;
                                }
                            }
                            sttIdx = srcLen-sttIdx;
                            while(sttIdx) {
                                if ((dstBuff[sttIdx] == ' ') || (dstBuff[sttIdx] == '\n')) {
                                    sttIdx++;
                                    break;
                                }
                                sttIdx--;
                            }
                            
                        }

                        //client->m_Logger->debug("VRClient::thrdMain(%s) - sttIdx(%d)\nsrc(%s)\ndst(%s)", client->m_sCallId.c_str(), sttIdx, srcBuff, dstBuff);

                        if ((!sttIdx || (sttIdx < dstLen)) && strlen(dstBuff+sttIdx)) {
                            // to DB
                            if (client->m_r2d) {
                                client->m_r2d->insertRtSTTData(diaNumber, client->m_sCallId, item->spkNo, pEndpos ? start : vPos[item->spkNo -1].bpos/160, pEndpos ? end : vPos[item->spkNo -1].epos/160, boost::replace_all_copy(std::string((const char*)dstBuff+sttIdx), "\n", " "));
                            }
                            //STTDeliver::instance(client->m_Logger)->insertSTT(client->m_sCallId, std::string((const char*)value), item->spkNo, vPos[item->spkNo -1].bpos, vPos[item->spkNo -1].epos);
                            // to STTDeliver(file)
                            if (client->m_deliver) {
                                client->m_deliver->insertSTT(client->m_sCallId, boost::replace_all_copy(std::string((const char*)dstBuff+sttIdx), "\n", " "), item->spkNo, pEndpos ? start : vPos[item->spkNo -1].bpos/160, pEndpos ? end : vPos[item->spkNo -1].epos/160);
                            }
                            
                        }

                        tmpStt[item->spkNo-1].clear();
                        tmpStt[item->spkNo-1] = (const char*)value;

#else
                        // to DB
                        if (client->m_r2d) {
                            client->m_r2d->insertRtSTTData(diaNumber, client->m_sCallId, item->spkNo, pEndpos ? start : vPos[item->spkNo -1].bpos/160, pEndpos ? end : vPos[item->spkNo -1].epos/160, std::string((const char*)value));
                        }
                        //STTDeliver::instance(client->m_Logger)->insertSTT(client->m_sCallId, std::string((const char*)value), item->spkNo, vPos[item->spkNo -1].bpos, vPos[item->spkNo -1].epos);
                        // to STTDeliver(file)
                        if (client->m_deliver) {
                            client->m_deliver->insertSTT(client->m_sCallId, std::string((const char*)value), item->spkNo, pEndpos ? start : vPos[item->spkNo -1].bpos/160, pEndpos ? end : vPos[item->spkNo -1].epos/160);
                        }
#endif
                        free(value);
                        
                        diaNumber++;
                    }
                }
                else if (gearman_failed(rc)){
                    client->m_Logger->error("VRClient::thrdMain(%s) - failed gearman_client_do(). [%lu : %lu], timeout(%d)", client->m_sCallId.c_str(), vPos[item->spkNo -1].bpos, vPos[item->spkNo -1].epos, client->m_nGearTimeout);
                }
                
                vPos[item->spkNo -1].bpos = vPos[item->spkNo -1].epos + 1;

#endif  // FAD_FUNC

				if (!item->flag) {	// 호가 종료되었음을 알리는 flag, 채널 갯수와 flag(0)이 들어온 갯수를 비교해야한다.
					//printf("\t[DEBUG] VRClient::thrdMain(%s) - final item delivered.\n", client->m_sCallId.c_str());
                    client->m_Logger->debug("VRClient::thrdMain(%s, %d) - final item delivered.", client->m_sCallId.c_str(), item->spkNo);

#ifdef FAD_FUNC
                    // last vBuff
                    if (vBuff[item->spkNo-1].size()) {
                        // send buff to gearman
                        value= gearman_client_do(gearClient, client->m_sFname.c_str(), NULL, 
                                                        (const void*)&vBuff[item->spkNo-1][0], vBuff[item->spkNo-1].size(),
                                                        &result_size, &rc);
                        if (gearman_success(rc))
                        {
                            // Make use of value
                            if (value) {
                                // std::cout << "DEBUG : value(" << (char *)value << ") : size(" << result_size << ")" << std::endl;
                                //client->m_Logger->debug("VRClient::thrdMain(%s) - sttIdx(%d)\nsrc(%s)\ndst(%s)", client->m_sCallId.c_str(), sttIdx, srcBuff, dstBuff);

                                if (client->m_r2d) {
                                    client->m_r2d->insertRtSTTData(diaNumber, client->m_sCallId, item->spkNo, sframe[item->spkNo -1], eframe[item->spkNo -1], boost::replace_all_copy(std::string((const char*)value), "\n", " "));
                                }
                                //STTDeliver::instance(client->m_Logger)->insertSTT(client->m_sCallId, std::string((const char*)value), item->spkNo, vPos[item->spkNo -1].bpos, vPos[item->spkNo -1].epos);
                                // to STTDeliver(file)
                                if (client->m_deliver) {
                                    client->m_deliver->insertSTT(client->m_sCallId, boost::replace_all_copy(std::string((const char*)value), "\n", " "), item->spkNo, sframe[item->spkNo -1], eframe[item->spkNo -1]);
                                }

                                free(value);
                                
                                diaNumber++;
                            }
                        }
                        else if (gearman_failed(rc)){
                            client->m_Logger->error("VRClient::thrdMain(%s) - failed gearman_client_do(). [%d : %d], timeout(%d)", client->m_sCallId.c_str(), sframe[item->spkNo -1], eframe[item->spkNo -1], client->m_nGearTimeout);
                        }

                        // and clear buff, set msg header
                        vBuff[item->spkNo-1].clear();

                    }
#endif

					if (!(--client->m_nNumofChannel)) {
						client->m_Mgr->removeVRC(client->m_sCallId);
						if ( item->voiceData != NULL ) delete[] item->voiceData;
						delete item;

                        if (client->m_r2d) {
                            client->m_r2d->updateCallInfo(client->m_sCallId, time(NULL), true);
                        }
#if 0
                        HAManager::getInstance()->deleteSyncItem(client->m_sCallId);
#else
                        // HA
                        if (HAManager::getInstance())
                            HAManager::getInstance()->insertSyncItem(false, client->m_sCallId, std::string("remove"), 1, 1);

#endif
						break;
					}
				}

				delete[] item->voiceData;
				delete item;
				// 예외 발생 시 처리 내용 : VDCManager의 removeVDC를 호출할 수 있어야 한다. - 이 후 VRClient는 item->flag(0)에 대해서만 처리한다.
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
        
#ifdef FAD_FUNC
        if (buf0) free(buf0);
        if (buf1) free(buf1);
        
        fvad_free(vad);

        munmap(pmmap, MM_SIZE);
        if (mmfd>0) close(mmfd);

        sprintf(buf, "/dev/shm/%s.mm", client->m_sCallId.c_str());
        unlink(buf);

        std::vector<uint8_t>().swap(vBuff[0]);
        std::vector<uint8_t>().swap(vBuff[1]);
#endif

#if 0 // for DEBUG
		if (client->m_is_save_pcm && pcmFile.is_open()) pcmFile.close();
#endif
	}
	// 파일(배치)를 위한 작업 수행 시
	else {
		client->m_Mgr->removeVRC(client->m_sCallId);
	}

    gearman_client_free(gearClient);
    std::vector< PosPair >().swap(vPos);

	WorkTracer::instance()->insertWork(client->m_sCallId, client->m_cJobType, WorkQueItem::PROCTYPE::R_FREE_WORKER);

	client->m_thrd.detach();
	delete client;
}

void VRClient::insertQueItem(QueItem* item)
{
	std::lock_guard<std::mutex> g(m_mxQue);
	m_qRTQue.push(item);
}