
#include "RT2DB.h"

#include <iconv.h>

#include <string.h>

RT2DB* RT2DB::m_instance = nullptr;


RT2DB::RT2DB()
: m_Logger(NULL)
{
    
}

RT2DB::~RT2DB()
{
    ConnectionPool_stop(m_pool);
    ConnectionPool_free(&m_pool);
    URL_free(&m_url);
}

RT2DB* RT2DB::instance(std::string dbtype, std::string dbhost, std::string dbport, std::string dbuser, std::string dbpw, std::string dbname)
{
    if (m_instance) return m_instance;
    
    m_instance = new RT2DB();
    
    std::string sUrl = dbtype + "://" + dbuser + ":" + dbpw + "@" + dbhost + ":" + dbport + "/" + dbname;
    m_instance->m_url = URL_new(sUrl.c_str());
    m_instance->m_pool = ConnectionPool_new(m_instance->m_url);
    
    TRY
    {
        ConnectionPool_start(m_instance->m_pool);
        ConnectionPool_setReaper(m_instance->m_pool, 10);
    }
    CATCH(SQLException)
    {
        printf("SQLException -- %s\n", Exception_frame.message);
        
        ConnectionPool_free(&m_instance->m_pool);
        URL_free(&m_instance->m_url);
        
        delete m_instance;
        m_instance = nullptr;
    }
    END_TRY;

    return m_instance;
}

void RT2DB::release()
{
    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }
}

int RT2DB::insertCallInfo(std::string callid, time_t stime)
{
    Connection_T con;
    struct tm resultT;
    
    localtime_r(&stime, &resultT);

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            printf("RT2DB::insertCallInfo - can't get connection from pool\n");
            return 1;
        }
        else if ( !Connection_ping(con) ) {
            printf("RT2DB::insertCallInfo - inactive connection from pool\n");
            Connection_close(con);
            return 2;
        }
        Connection_execute(con, "INSERT INTO CALL_LIST (CALL_ID, STIME, FLAG, REGTIME) VALUES ('%s', '%04d-%02d-%02d %02d:%02d:%02d', 'S', now())",
        callid.c_str(), resultT.tm_year, resultT.tm_mon+1, resultT.tm_mday, resultT.tm_hour, resultT.tm_min, resultT.tm_sec);
    }
    CATCH(SQLException)
    {
        printf("RT2DB::insertCallInfo - SQLException -- %s\n", Exception_frame.message);
    }
    FINALLY
    {
        Connection_close(con);
    }
    END_TRY;
    return 0;
}

int RT2DB::updateCallInfo(std::string callid, time_t stime)
{
    Connection_T con;
    struct tm resultT;
    
    localtime_r(&stime, &resultT);

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            printf("RT2DB::updateCallInfo - can't get connection from pool\n");
            return 1;
        }
        else if ( !Connection_ping(con) ) {
            printf("RT2DB::updateCallInfo - inactive connection from pool\n");
            Connection_close(con);
            return 2;
        }
        Connection_execute(con, "UPDATE CALL_LIST SET ETIME='%04d-%02d-%02d %02d:%02d:%02d', FLAG='E' WHERE CALL_ID='%s' AND FLAG='S'",
        resultT.tm_year, resultT.tm_mon+1, resultT.tm_mday, resultT.tm_hour, resultT.tm_min, resultT.tm_sec, callid.c_str());
    }
    CATCH(SQLException)
    {
        printf("RT2DB::updateCallInfo - SQLException -- %s\n", Exception_frame.message);
    }
    FINALLY
    {
        Connection_close(con);
    }
    END_TRY;
    return 0;
}

int RT2DB::insertRtSTTData(uint32_t idx, std::string callid, uint8_t spkno, uint64_t spos, uint64_t epos, std::string stt)
{
    Connection_T con;

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            printf("RT2DB::insertRtSTTData - can't get connection from pool\n");
            return 1;
        }
        else if ( !Connection_ping(con) ) {
            printf("RT2DB::insertRtSTTData - inactive connection from pool\n");
            Connection_close(con);
            return 2;
        }
        Connection_execute(con, "INSERT INTO RTSTT_DATA (IDX,CALL_ID,SPKNO,SPOS,EPOS,STT,REGTIME) VALUES (%d,'%s','%c',%lu,%lu,'%s',now())",
        idx, callid.c_str(),((spkno == 1)?'R':'L'), spos/160, epos/160, stt.c_str());
    }
    CATCH(SQLException)
    {
        printf("RT2DB::insertRtSTTData - SQLException -- %s\n", Exception_frame.message);
    }
    FINALLY
    {
        Connection_close(con);
    }
    END_TRY;
    return 0;
}
