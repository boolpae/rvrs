#ifndef __SCHD4DB_H__
#define __SCHD4DB_H__

/* Scheduler for Database
 * 
 */

class Schd4DB {
public:
    static Schd4DB* instance();
    virtual ~Schd4DB();

private:
    Schd4DB();
    
private:
    static Schd4DB* m_instance;

};


#endif //__Schd4DB_H__