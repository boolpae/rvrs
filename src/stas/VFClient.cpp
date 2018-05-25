
#include "stas.h"
#include "VFClient.h"
#include "VFCManager.h"
#include "STT2DB.h"
#include "STT2File.h"

#include <thread>

// For Gearman
#include <libgearman/gearman.h>

#include <string.h>

#include <boost/algorithm/string.hpp>
#include <sstream>

VFClient::VFClient(VFCManager* mgr, std::string gearHost, uint16_t gearPort, int gearTimeout, uint64_t numId)
: m_LiveFlag(true), m_mgr(mgr), m_sGearHost(gearHost), m_nGearPort(gearPort), m_nGearTimeout(gearTimeout), m_nNumId(numId)
{
    
}

VFClient::~VFClient()
{
    if (m_thrd.joinable()) m_thrd.join();
}

void VFClient::startWork()
{
    m_thrd = std::thread(VFClient::thrdFunc, m_mgr, this);
}

void VFClient::thrdFunc(VFCManager* mgr, VFClient* client)
{
    gearman_client_st *gearClient;
    gearman_return_t ret;
    void *value = NULL;
    size_t result_size;
    gearman_return_t rc;

    char buf[256];
    int buflen=0;
    
    std::string line;

    JobInfoItem* item;
    
    STT2DB *stt2db = STT2DB::getInstance();
    STT2File *stt2file = STT2File::getInstance();
    log4cpp::Category *logger = config->getLogger();

    if (!stt2db) {
        logger->error("VFClient::thrdFunc(%ld) - Failed to get STT2DB instance", client->m_nNumId);
        return;
    }

#if 1
    gearClient = gearman_client_create(NULL);
    if (!gearClient) {
        //printf("\t[DEBUG] VRClient::thrdMain() - ERROR (Failed gearman_client_create - %s)\n", client->m_sCallId.c_str());
        //client->m_Logger->error("VRClient::thrdMain() - ERROR (Failed gearman_client_create - %s)", client->m_sCallId.c_str());

        //WorkTracer::instance()->insertWork(client->m_sCallId, client->m_cJobType, WorkQueItem::PROCTYPE::R_FREE_WORKER);

        client->m_thrd.detach();
        delete client;
        return;
    }
    
    ret= gearman_client_add_server(gearClient, client->m_sGearHost.c_str(), client->m_nGearPort);
    if (gearman_failed(ret))
    {
        //printf("\t[DEBUG] VRClient::thrdMain() - ERROR (Failed gearman_client_add_server - %s)\n", client->m_sCallId.c_str());
        //client->m_Logger->error("VRClient::thrdMain() - ERROR (Failed gearman_client_add_server - %s)", client->m_sCallId.c_str());

        //WorkTracer::instance()->insertWork(client->m_sCallId, client->m_cJobType, WorkQueItem::PROCTYPE::R_FREE_WORKER);
        gearman_client_free(gearClient);
        client->m_thrd.detach();
        delete client;
        return;
    }

    if (client->m_nGearTimeout) {
        gearman_client_set_timeout(gearClient, client->m_nGearTimeout);
    }
#endif

    while(client->m_LiveFlag) {
        if(item = mgr->popItem()) {
            memset(buf, 0, sizeof(buf));
            buflen = 0;
            
            line = item->getPath() + "/" + item->getFilename();
            logger->debug("VFClient::thrdFunc(%ld) - line(%s)", client->m_nNumId, line.c_str());
#if 1
            value= gearman_client_do(gearClient, "vr_stt", NULL, 
                                            (const void*)line.c_str(), line.size(),
                                            &result_size, &rc);
            if (gearman_success(rc)) {
                // Make use of value
                if (value) {
                    //std::cout << "STT RESULT <<\n" << (const char*)value << "\n>>" << std::endl;

                    value= gearman_client_do(gearClient, "vr_text", NULL, 
                                                    (const void*)value, result_size,
                                                    &result_size, &rc);
                    if (gearman_success(rc)) {
                        // Make use of value
                        if (value) {
                            uint32_t diaNumber=0;
                            std::string strValue((const char*)value);
                            std::istringstream iss(strValue);
                            std::vector<std::string> strs;

                            //std::cout << "STT RESULT <<\n" << (const char*)value << "\n>>" << std::endl;
                            while(std::getline(iss, line)) {
                                boost::split(strs, line, boost::is_any_of(","));
                                //std::cout << "[1] : " << strs[0] << " [2] : " << strs[1] << " [3] : " << strs[2] << std::endl;

                                // to DB
                                if (stt2db) {
                                    diaNumber++;
                                    stt2db->insertRtSTTData(diaNumber, item->getCallId(), 0, std::stoi(strs[0].c_str()+4), std::stoi(strs[1].c_str()+4), strs[2]);
                                }

                                // to STTDeliver(file)
                                if (stt2file) {
                                    //stt2file->insertSTT(item->getCallId(), strs[2], 0, std::stoi(strs[0].c_str()+4), std::stoi(strs[1].c_str()+4));
                                    stt2file->insertSTT(item->getCallId(), strs[2], item->getCallId());
                                }
                            }
                        }
                        stt2db->updateTaskInfo(item->getCallId(), item->getCounselorCode(), 'Y');
                    }
                    else if (gearman_failed(rc)) {
                        logger->error("VFClient::thrdFunc(%ld) - failed gearman_client_do(vr_text). [%s : %s], timeout(%d)", client->m_nNumId, item->getCallId().c_str(), item->getFilename().c_str(), client->m_nGearTimeout);
                        stt2db->updateTaskInfo(item->getCallId(), item->getCounselorCode(), 'X');
                    }

                }
                else {
                    logger->info("VFClient::thrdFunc(%ld) - Success to get gearman(vr_stt) but empty result.  [%s : %s]", client->m_nNumId, item->getCallId().c_str(), item->getFilename().c_str());
                    stt2db->updateTaskInfo(item->getCallId(), item->getCounselorCode(), 'Y');
                }
            }
            else if (gearman_failed(rc)) {
                logger->error("VFClient::thrdFunc(%ld) - failed gearman_client_do(vr_stt). [%s : %s], timeout(%d)", client->m_nNumId, item->getCallId().c_str(), item->getFilename().c_str(), client->m_nGearTimeout);
                stt2db->updateTaskInfo(item->getCallId(), item->getCounselorCode(), 'X');
            }
#endif
            delete item;
            item = nullptr;
        }
        else {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        
        }
        //logger->debug("VFClient::thrdFunc() working...");
    }
#if 1
    gearman_client_free(gearClient);
#endif
}