
#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__


#include <thread>


class VFCManager;

class Notifier {
public:
    static Notifier* instance(VFCManager *vfcm);
    ~Notifier();
    int startWork();
    void stopWork() { m_LiveFlag = false; }
    
private:
    Notifier(VFCManager *vfcm);
    static void thrdFunc(Notifier *noti);
    
private:
    static Notifier* m_instance;
    std::thread m_thrdNot;
    
    VFCManager *m_vfcm;
    bool m_LiveFlag;
};



#endif // __NOTIFIER_H__