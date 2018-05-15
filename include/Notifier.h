
#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__


#include <thread>


class VFCManager;
class STT2DB;

class Notifier {
public:
    static Notifier* instance(VFCManager *vfcm, STT2DB *stt2db);
    ~Notifier();
    int startWork();
    void stopWork() { m_LiveFlag = false; }
    
private:
    Notifier(VFCManager *vfcm, STT2DB *stt2db);
    static void thrdFunc(Notifier *noti);
    
private:
    static Notifier* m_instance;
    std::thread m_thrdNoti;
    
    VFCManager *m_vfcm;
    STT2DB *m_stt2db;
    bool m_LiveFlag;
};



#endif // __NOTIFIER_H__