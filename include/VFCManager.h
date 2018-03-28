
#ifndef __VFCMANAGER_H__
#define __VFCMANAGER_H__

/* Voice File Client Manager
 * 
 */


class VFCManager {
public:
    virtual ~VFCManager();
    static VFCManager* instance();

private:
    VFCManager();
    
private:
    static VFCManager* m_instance;
};


#endif // __VFCMANAGER_H__