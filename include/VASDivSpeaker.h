#ifndef _VASDIVSPEAKER_H_
#define _VASDIVSPEAKER_H_

#include <string>

#include <libgearman/gearman.h>

class STT2DB;
class STT2File;
class JobInfoItem;

class VASDivSpeaker {
private:
    STT2DB *m_hDB;
    STT2File *m_hFile;
    JobInfoItem *m_jobItem;

public:
    VASDivSpeaker(STT2DB *db, STT2File *file, JobInfoItem *item);
    virtual ~VASDivSpeaker();

    int startWork(gearman_client_st *gearClient, std::string &funcname, std::string &unseg);
};

#endif // _VASDIVSPEAKER_H_