
#include "VFCManager.h"

#include <vector>

#include <unistd.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define CONN_GEARMAN_PER_CALL

using namespace std;

VFCManager* VFCManager::m_instance = nullptr;

VFCManager::VFCManager(int geartimeout, log4cpp::Category *logger)
	: m_sGearHost("127.0.0.1"), m_nGearPort(4730), m_GearTimeout(geartimeout), m_nSockGearman(0), m_Logger(logger)
{
	//printf("\t[DEBUG] VFCManager Constructed.\n");
    m_Logger->debug("VFCManager Constructed.");
}


VFCManager::~VFCManager()
{
    disconnectGearman();
	removeAllVFC();

	//printf("\t[DEBUG] VFCManager Destructed.\n");
    m_Logger->debug("VFCManager Destructed.");
}

bool VFCManager::connectGearman()
{
	struct sockaddr_in addr;

	if (m_nSockGearman) closesocket(m_nSockGearman);

	if ((m_nSockGearman = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {//소켓 생성
		perror("VFCManager::connectGearman() - socket :");
        m_Logger->error("VFCManager::connectGearman() - socket : %d", errno);
		m_nSockGearman = 0;
		return false;
	}

	memset(&addr, 0x00, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(m_sGearHost.c_str());
	addr.sin_port = htons(m_nGearPort);

	if (::connect(m_nSockGearman, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("VFCManager::connectGearman() - connect :");
        m_Logger->error("VFCManager::connectGearman() - connect : %d", errno);
		closesocket(m_nSockGearman);
		m_nSockGearman = 0;
		return false;
	}

	return true;
}

void VFCManager::disconnectGearman()
{
	if (m_nSockGearman > 0) {
		closesocket(m_nSockGearman);
        m_nSockGearman = 0;
	}
}

#define RECV_BUFF_LEN 512
bool VFCManager::getGearmanFnames(std::vector<std::string> &vFnames)
{
	char recvBuf[RECV_BUFF_LEN];
	int recvLen = 0;
	std::string sReq = "status\r\n";
	std::string sRes = "";
	int rec = 0;

	struct timeval tv;
	fd_set rfds;
	int selVal;

	RECONNECT:
	if (!m_nSockGearman && !connectGearman()) {
		//printf("\t[DEBUG] VFCManager::getGearmanFnames() - error connect to GearHost.\n");
        m_Logger->error("VFCManager::getGearmanFnames() - error connect to GearHost.");
		return false;
	}

	if ( ::send(m_nSockGearman, sReq.c_str(), sReq.length(), 0) <= 0) {
		perror("VFCManager::getGearmanFnames() - send :");
        m_Logger->error("VFCManager::getGearmanFnames() - send : %d", errno);
		if (++rec > 3) {
			//printf("\t[DEBUG] VFCManager::getGearmanFnames() - error Reconnect count 3 exceeded.\n");
            disconnectGearman();
            m_Logger->warn("VFCManager::getGearmanFnames() - error Reconnect count 3 exceeded.");
			return false;
		}
		goto RECONNECT;
	}

	while (1) {
		tv.tv_sec = 0;	// for debug
		tv.tv_usec = 500000;
		FD_ZERO(&rfds);
		FD_SET(m_nSockGearman, &rfds);

		selVal = select(m_nSockGearman+1, &rfds, NULL, NULL, &tv);

        if (selVal > 0) {
            recvLen = ::recv(m_nSockGearman, recvBuf, RECV_BUFF_LEN-1, 0);
            if (recvLen <= 0) {
                perror("VFCManager::getGearmanFnames() - send :");
                disconnectGearman();
                m_Logger->error("VFCManager::getGearmanFnames() - send : %d", errno);
                return false;
            }
            recvBuf[recvLen] = 0;
            sRes.append(recvBuf);

            if ( !strncmp( (sRes.c_str() + (sRes.length() - 2)), ".\n", 2) ) {	// 마지막 문장이 ".\r\n" 인 경우 루프 탈출
                break;
            }
        }
        else if (selVal == 0) {
            m_Logger->error("VFCManager::getGearmanFnames() - recv timeout : reconnect(%d)", rec);
            if (++rec > 3) {
                disconnectGearman();
                //printf("\t[DEBUG] VFCManager::getGearmanFnames() - error Reconnect count 3 exceeded.\n");
                m_Logger->warn("VFCManager::getGearmanFnames() - error Reconnect count 3 exceeded.");
                return false;
            }
            goto RECONNECT;
        }
	}
	
	getFnamesFromString(sRes, vFnames);

	//printf("\t[DEBUG] - Gearman STATUS <<\n%s\n>>\n", sRes.c_str());
    //m_Logger->debug("\n --- Gearman STATUS --- \n%s ---------------------- \n", sRes.c_str());
	return true;
}

void VFCManager::getFnamesFromString(std::string & gearResult, std::vector<std::string>& vFnames)
{
	std::string token;
	char fname[64];
	int c;
	int r;
	int w;
	size_t pos, npos;

	pos = npos = 0;

	while ((npos = gearResult.find("\n", pos)) != string::npos) {

		token = gearResult.substr(pos, npos - pos);

		if (!strncmp(token.c_str() + (token.length() - 1), ".", 1)) {
			break;
		}

		sscanf(token.c_str(), "%s\t%d\t%d\t%d", fname, &c, &r, &w);

		if ((!strncmp(fname, "vr_stt", 6)) && (c == 0) && (r == 0)) {
			vFnames.push_back(std::string(fname));
		}

		pos = npos + 1;
	}

}

VFCManager* VFCManager::instance(const std::string gearHostIp, const uint16_t gearHostPort, int geartimeout, log4cpp::Category *logger)
{
	if (m_instance) return m_instance;

	m_instance = new VFCManager(geartimeout, logger);

	// for DEV
	m_instance->setGearHost(gearHostIp);//);("192.168.229.135")
	m_instance->setGearPort(gearHostPort);
    
#ifndef CONN_GEARMAN_PER_CALL   // 항시 연결인 경우 사용
    if (!m_instance->connectGearman()) {
        //printf("\t[DEBUG] RCManager::instance() - ERROR (Failed to connect gearhost)\n");
        logger->error("VFCManager::instance() - ERROR (Failed to connect gearhost)");
        delete m_instance;
        m_instance = NULL;
    }
#endif

	return m_instance;
}

void VFCManager::release()
{
	if (m_instance) {
		delete m_instance;
		m_instance = NULL;
	}
}

// return: 성공(0), 실패(0이 아닌 값)
int16_t VFCManager::requestVFC(string& callid, uint8_t jobType, uint8_t noc = 1)
{
	int16_t res = 0;
#if 0
	VRClient* client;
	vector< string > vFnames;
	vector< string >::iterator iter;

	std::lock_guard<std::mutex> g(m_mxQue);

	// 1. vFnames에 실시간STT 처리를 위한 worker의 fname 가져오기 &vFnames
#ifndef CONN_GEARMAN_PER_CALL   // Gearmand과 항시 연결일 경우에 사용
	if (!getGearmanFnames(vFnames))
#else   // 요청 시 마다 Gearman에 연결
    if (!connectGearman() || !getGearmanFnames(vFnames))
#endif
    {
		//printf("\t[DEBUG] VFCManager::requestVRC() - error Failed to get gearman status\n");
        m_Logger->error("VFCManager::requestVRC() - error Failed to get gearman status");
		return int16_t(3);	// Gearman으로부터 Fn Name 가져오기 실패
	}
    
#ifdef CONN_GEARMAN_PER_CALL
    disconnectGearman();
#endif
	// DEBUG
	// vFnames.push_back(callid);

	for (iter = vFnames.begin(); iter != vFnames.end(); iter++) {
		//if (!m_mWorkerTable.count(*iter)) break;
		if (m_mWorkerTable.count(*iter) == 0) break;
	}

	if (iter != vFnames.end()) {
		client = new VRClient(m_instance, this->m_sGearHost, this->m_nGearPort, this->m_GearTimeout, *iter, callid, jobType, noc, m_deliver, m_Logger, m_s2d, m_is_save_pcm, m_pcm_path); // or VRClient(this);

		if (client) {
			std::lock_guard<std::mutex> g(m_mxMap);
			m_mWorkerTable[*iter] = client;
		}
		else {
			res = 1;	// 실시간 STT 처리를 위한 VRClient 인스턴스 생성에 실패
		}
	}
	else {
		res = 2;	// 실시간 STT 처리를 위한 가용한 worker가 없음
	}
#endif

	return res;
}

void VFCManager::removeVFC(string callid)
{
	//m_vWorkerTable.erase(find(m_vWorkerTable.begin(), m_vWorkerTable.end(), fname));
#if 0
	VRClient* client = NULL;
	map< string, VRClient* >::iterator iter;

	std::lock_guard<std::mutex> g(m_mxMap);

	for (iter = m_mWorkerTable.begin(); iter != m_mWorkerTable.end(); iter++) {
		if (!((VRClient*)(iter->second))->getCallId().compare(callid)) {
			client = (VRClient*)iter->second;
			break;
		}
	}

	//delete client;
	if (client) {
		client->finish();
		m_mWorkerTable.erase(iter);
	}
#endif
}

void VFCManager::removeAllVFC()
{
#if 0
	VRClient* client = NULL;
	map< string, VRClient* >::iterator iter;

	for (iter = m_mWorkerTable.begin(); iter != m_mWorkerTable.end(); iter++) {
		client = (VRClient*)iter->second;
		client->finish();
	}
	m_mWorkerTable.clear();
#endif
}

void VFCManager::outputVFCStat()
{
#if 0
	//VRClient* client = NULL;
	map< string, VRClient* >::iterator iter;

	for (iter = m_mWorkerTable.begin(); iter != m_mWorkerTable.end(); iter++) {
		//client = (VRClient*)iter->second;
		//printf("\t[DEBUG] VFCManager::outputVRCStat() - VRClient(%s)\n", iter->first.c_str());
        m_Logger->debug("VFCManager::outputVRCStat() - VRClient(%s, %s)", iter->first.c_str(), iter->second->getCallId().c_str());
	}
    
    if ( m_mWorkerTable.size() )
        m_Logger->info("VFCManager::outputVRCStat() - Current working VRClient count(%d)", m_mWorkerTable.size());
#endif
}

VFClient* VFCManager::getVFClient(string& callid)
{
#if 0
	//m_vWorkerTable.erase(find(m_vWorkerTable.begin(), m_vWorkerTable.end(), fname));
	VRClient* client = NULL;
	map< string, VRClient* >::iterator iter;

	for (iter = m_mWorkerTable.begin(); iter != m_mWorkerTable.end(); iter++) {
		if (!((VRClient*)(iter->second))->getCallId().compare(callid)) {
			client = (VRClient*)iter->second;
			break;
		}
	}

	return client;
#endif
    return nullptr;
}

// for HA
int VFCManager::addVFC(string callid, string fname, uint8_t jobtype, uint8_t noc)
{
	int16_t res = 0;
#if 0
	VRClient* client;

    client = new VRClient(this, this->m_sGearHost, this->m_nGearPort, this->m_GearTimeout, fname, callid, jobtype, noc, m_deliver, m_Logger, m_s2d, m_is_save_pcm, m_pcm_path); // or VRClient(this);

    if (client) {
        std::lock_guard<std::mutex> g(m_mxMap);
        m_mWorkerTable[fname] = client;
    }
    else {
        res = 1;	// 실시간 STT 처리를 위한 VRClient 인스턴스 생성에 실패
    }
#endif
    return res;
}