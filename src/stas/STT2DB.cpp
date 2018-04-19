
#include "STT2DB.h"

#include <iconv.h>

#include <string.h>

STT2DB* STT2DB::m_instance = nullptr;


STT2DB::STT2DB(log4cpp::Category *logger)
: m_bLiveFlag(true), m_Logger(logger)
{
	m_Logger->debug("STT2DB Constructed.");
}

STT2DB::~STT2DB()
{
    ConnectionPool_stop(m_pool);
    ConnectionPool_free(&m_pool);
    URL_free(&m_url);
    m_thrd.join();
        
	m_Logger->debug("STT2DB Destructed.");
}

void STT2DB::thrdMain(STT2DB * s2d)
{
	std::lock_guard<std::mutex> *g;// (m_mxQue);
	RTSTTQueItem* item;

    int ret=0;
    char *utf_buf = NULL;
    size_t in_size, out_size;
    iconv_t it;
    char *input_buf_ptr = NULL;
    char *output_buf_ptr = NULL;

    Connection_T con;
    con = ConnectionPool_getConnection(s2d->m_pool);
    

	while (s2d->m_bLiveFlag) {
		while (!s2d->m_qRtSttQue.empty()) {
			g = new std::lock_guard<std::mutex>(s2d->m_mxQue);
			item = s2d->m_qRtSttQue.front();
			s2d->m_qRtSttQue.pop();
			delete g;

            in_size = item->getSTTValue().size();
            out_size = item->getSTTValue().size() * 2;
            utf_buf = (char *)malloc(out_size);
            
            if (utf_buf) {
                memset(utf_buf, 0, out_size);

                input_buf_ptr = (char *)item->getSTTValue().c_str();
                output_buf_ptr = utf_buf;

                it = iconv_open("UTF-8", "EUC-KR");

                ret = iconv(it, &input_buf_ptr, &in_size, &output_buf_ptr, &out_size);
                 
                iconv_close(it);
                
            }
            else {
                utf_buf = nullptr;
                ret = -1;
            }
            
            // insert rtstt to db
            TRY
            {
                Connection_execute(con, "INSERT INTO RTSTT_DATA (IDX,CALL_ID,SPKNO,SPOS,EPOS,STT,REGTIME) VALUES (%d,'%s','%c',%lu,%lu,'%s',now())",
                item->getDiaIdx(), item->getCallId().c_str(),((item->getSpkNo() == 1)?'R':'L'), item->getBpos(), item->getEpos(), ((ret == -1) ? item->getSTTValue().c_str() : utf_buf));
            }
            CATCH(SQLException)
            {
                s2d->m_Logger->error("STT2DB::insertCallInfo - SQLException -- %s", Exception_frame.message);
                Connection_close(con);
                con = ConnectionPool_getConnection(s2d->m_pool);
            }
            END_TRY;

            if (utf_buf) free(utf_buf);
			delete item;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
    Connection_close(con);
}

STT2DB* STT2DB::instance(std::string dbtype, std::string dbhost, std::string dbport, std::string dbuser, std::string dbpw, std::string dbname, std::string charset, log4cpp::Category *logger)
{
    if (m_instance) return m_instance;
    
    m_instance = new STT2DB(logger);
    
    std::string sUrl = dbtype + "://" + dbuser + ":" + dbpw + "@" + dbhost + ":" + dbport + "/" + dbname + "?charset=" + charset;
    m_instance->m_url = URL_new(sUrl.c_str());
    m_instance->m_pool = ConnectionPool_new(m_instance->m_url);
    
    TRY
    {
        ConnectionPool_start(m_instance->m_pool);
        ConnectionPool_setReaper(m_instance->m_pool, 10);
    }
    CATCH(SQLException)
    {
        logger->error("STT2DB::instance - SQLException -- %s", Exception_frame.message);
        
        ConnectionPool_free(&m_instance->m_pool);
        URL_free(&m_instance->m_url);
        
        delete m_instance;
        m_instance = nullptr;
    }
    END_TRY;
    
    m_instance->m_thrd = std::thread(STT2DB::thrdMain, m_instance);

    return m_instance;
}

void STT2DB::release()
{
    if (m_instance) {
        m_instance->m_bLiveFlag = false;

        delete m_instance;
        m_instance = nullptr;
    }
}

int STT2DB::insertCallInfo(std::string callid, time_t stime)
{
    Connection_T con;
    struct tm resultT;
    
    localtime_r(&stime, &resultT);

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            m_Logger->error("STT2DB::insertCallInfo - can't get connection from pool");
            return 1;
        }
        else if ( !Connection_ping(con) ) {
            m_Logger->error("STT2DB::insertCallInfo - inactive connection from pool");
            Connection_close(con);
            return 2;
        }
        Connection_execute(con, "INSERT INTO CALL_LIST (CALL_ID, STIME, FLAG, REGTIME) VALUES ('%s', '%04d-%02d-%02d %02d:%02d:%02d', 'I', now())",
        callid.c_str(), resultT.tm_year+1900, resultT.tm_mon+1, resultT.tm_mday, resultT.tm_hour, resultT.tm_min, resultT.tm_sec);
    }
    CATCH(SQLException)
    {
        m_Logger->error("STT2DB::insertCallInfo - SQLException -- %s", Exception_frame.message);
    }
    FINALLY
    {
        Connection_close(con);
    }
    END_TRY;
    return 0;
}

int STT2DB::updateCallInfo(std::string callid, time_t stime, bool end)
{
    Connection_T con;
    struct tm resultT;
    
    localtime_r(&stime, &resultT);

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            m_Logger->error("STT2DB::updateCallInfo - can't get connection from pool");
            return 1;
        }
        else if ( !Connection_ping(con) ) {
            m_Logger->error("STT2DB::updateCallInfo - inactive connection from pool");
            Connection_close(con);
            return 2;
        }
        if (!end) {
            Connection_execute(con, "UPDATE CALL_LIST SET ETIME='%04d-%02d-%02d %02d:%02d:%02d' WHERE CALL_ID='%s' AND FLAG='I'",
            resultT.tm_year+1900, resultT.tm_mon+1, resultT.tm_mday, resultT.tm_hour, resultT.tm_min, resultT.tm_sec, callid.c_str());
        }
        else {
            Connection_execute(con, "UPDATE CALL_LIST SET FLAG='E' WHERE CALL_ID='%s' AND FLAG='I'",
            callid.c_str());
        }
    }
    CATCH(SQLException)
    {
        m_Logger->error("STT2DB::updateCallInfo - SQLException -- %s", Exception_frame.message);
    }
    FINALLY
    {
        Connection_close(con);
    }
    END_TRY;
    return 0;
}
#if 0
int STT2DB::insertRtSTTData(uint32_t idx, std::string callid, uint8_t spkno, uint64_t spos, uint64_t epos, std::string stt)
{
    Connection_T con;

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            printf("STT2DB::insertRtSTTData - can't get connection from pool\n");
            return 1;
        }
        else if ( !Connection_ping(con) ) {
            printf("STT2DB::insertRtSTTData - inactive connection from pool\n");
            Connection_close(con);
            return 2;
        }
        Connection_execute(con, "INSERT INTO RTSTT_DATA (IDX,CALL_ID,SPKNO,SPOS,EPOS,STT,REGTIME) VALUES (%d,'%s','%c',%lu,%lu,'%s',now())",
        idx, callid.c_str(),((spkno == 1)?'R':'L'), spos/160, epos/160, stt.c_str());
    }
    CATCH(SQLException)
    {
        printf("STT2DB::insertRtSTTData - SQLException -- %s\n", Exception_frame.message);
    }
    FINALLY
    {
        Connection_close(con);
    }
    END_TRY;
    return 0;
}
#endif
void STT2DB::insertRtSTTData(uint32_t idx, std::string callid, uint8_t spkno, uint64_t spos, uint64_t epos, std::string stt)
{
	insertRtSTTData(new RTSTTQueItem(idx, callid, spkno, stt, spos, epos));
}

void STT2DB::insertRtSTTData(RTSTTQueItem * item)
{
	std::lock_guard<std::mutex> g(m_mxQue);
	m_qRtSttQue.push(item);
}

void STT2DB::insertBatchTask()
{
    // insert
}

int STT2DB::getBatchTask()
{
	std::lock_guard<std::mutex> g(m_mxDb);
    
    // select, update
    
    return 0;
}

void STT2DB::deleteBatchTask()
{
    // delete
}

RTSTTQueItem::RTSTTQueItem(uint32_t idx, std::string callid, uint8_t spkno, std::string sttvalue, uint64_t bpos, uint64_t epos)
	:m_nDiaIdx(idx), m_sCallId(callid), m_nSpkNo(spkno), m_sSTTValue(sttvalue), m_nBpos(bpos), m_nEpos(epos)
{
}

RTSTTQueItem::~RTSTTQueItem()
{
}
