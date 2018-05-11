#ifndef __SCHD4DB_H__
#define __SCHD4DB_H__

/* Scheduler for Database
 * 
 */
#include <thread>

class STT2DB;
class VFCManager;

class Schd4DB {
public:
    virtual ~Schd4DB();
    static Schd4DB* instance(STT2DB *sttdb, VFCManager *vfcmgr);
	static void release();

private:
    Schd4DB(STT2DB *sttdb, VFCManager *vfcmgr);
    static void thrdFuncSchd4DB(Schd4DB* schd);
    
private:
    static Schd4DB* m_instance;
    bool m_bLiveFlag;

    STT2DB *m_sttdb;
    VFCManager *m_vfcmgr;

	std::thread m_thrd;

};


#endif //__Schd4DB_H__