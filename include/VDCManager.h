#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <mutex>

class VDClient;
class VDCManager
{
	static VDCManager* ms_instance;

	uint16_t m_nTotalChannelCount;

	std::vector< VDClient* > m_vClients;

	mutable std::mutex m_mxVec;

public:
	static VDCManager* instance();
	static void release();

	int16_t requestVDC(std::string& callid, uint8_t noc, std::vector< uint16_t > &vPorts);
	void removeVDC(std::string& callid);

	void outputVDCStat();

private:
	VDCManager();
	virtual ~VDCManager();
};

