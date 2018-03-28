
#ifndef __VFCLIENT_H__
#define __VFCLIENT_H__


class VFClient {
public:
    static VFClient* instance();
    virtual ~VFClient();

private:
    VFClient();

private:
    static VFClient* m_instance;
};


#endif // __VFCLIENT_H__