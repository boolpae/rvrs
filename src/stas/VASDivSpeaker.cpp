
#include "VASDivSpeaker.h"
#include "STT2DB.h"
#include "STT2File.h"

VASDivSpeaker::VASDivSpeaker(STT2DB *db, STT2File *file, JobInfoItem *item)
: m_hDB(db), m_hFile(file), m_jobItem(item)
{

}

VASDivSpeaker::~VASDivSpeaker()
{

}

int VASDivSpeaker::startWork(gearman_client_st *gearClient, std::string &unseg)
{
    int ret=0;
    gearman_return_t rc;
    void *value = nullptr;
    size_t result_size;
    std::string sValue;

    sValue = m_jobItem->getCallId() + "," + m_jobItem->getPath() + "/" + m_jobItem->getFilename() + "\n" + unseg;

    value= gearman_client_do(gearClient, "vr_text", NULL, 
                                    (const void*)(sValue.c_str()), sValue.size(),
                                    &result_size, &rc);
    if (gearman_success(rc)) {
        sValue = (const char*)value;
        free(value);

        //"화자,시작msec,종료msec,Text"
        // 화자는 spk00, spk01 이라는 값으로 구분되며 갯수가 많은 값이 상담원이 된다. - spk00, spk01 의 갯수를 파악해야핸다.
    }

    return ret;
}