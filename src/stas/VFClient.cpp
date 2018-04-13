
#include "stas.h"
#include "VFClient.h"
#include "VFCManager.h"

#include <thread>

// For Gearman
#include <libgearman/gearman.h>

#include <string.h>

VFClient::VFClient(VFCManager* mgr, std::string gearHost, uint16_t gearPort, int gearTimeout)
: m_LiveFlag(true), m_mgr(mgr), m_sGearHost(gearHost), m_nGearPort(gearPort), m_nGearTimeout(gearTimeout)
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
    
    log4cpp::Category *logger = config->getLogger();

#if 0
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
        if(mgr->popItem(line)) {
            memset(buf, 0, sizeof(buf));
            buflen = 0;
            
            logger->debug("line(%s)", line.c_str());
#if 0
            value= gearman_client_do(gearClient, "vr_stt", NULL, 
                                            (const void*)buf, buflen,
                                            &result_size, &rc);
            if (gearman_success(rc)) {
                // Make use of value
                if (value) {
                }
            }
            else if (gearman_failed(rc)) {
                //client->m_Logger->error("VRClient::thrdMain(%s) - failed gearman_client_do(). [%d : %d], timeout(%d)", client->m_sCallId.c_str(), vPos[item->spkNo -1].bpos, vPos[item->spkNo -1].epos, client->m_nGearTimeout);
            }
#endif
        }
        else {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        
        }
        logger->debug("VFClient::thrdFunc() working...");
    }

    gearman_client_free(gearClient);
}