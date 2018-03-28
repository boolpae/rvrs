
#ifndef __NOTIFIER_H__
#define __NOTIFIER_H__


class Notifier {
public:
    static Notifier* instance();
    ~Notifier();
    
private:
    Notifier();
    
private:
    static Notifier* m_instance;
};



#endif // __NOTIFIER_H__