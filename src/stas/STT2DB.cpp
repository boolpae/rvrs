
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
    m_instance->m_bLiveFlag = false;

    ConnectionPool_stop(m_pool);
    ConnectionPool_free(&m_pool);
    URL_free(&m_url);
    if (m_thrd.joinable()) m_thrd.detach();
        
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
                char cSpk='N';

                switch(item->getSpkNo()) {
                    case 1:
                        cSpk = 'R';
                        break;
                    case 2:
                        cSpk = 'L';
                        break;
                    default:
                        cSpk = 'N';
                }

                Connection_execute(con, "INSERT INTO JOB_DATA (idx,call_id,spk,pos_start,pos_end,value) VALUES (%d,'%s','%c',%lu,%lu,'%s')",
                item->getDiaIdx(), item->getCallId().c_str(), cSpk, item->getBpos(), item->getEpos(), ((ret == -1) ? item->getSTTValue().c_str() : utf_buf));
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
    if (STT2DB::getInstance())
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
        //ConnectionPool_setReaper(m_instance->m_pool, 10);
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
        delete m_instance;
        m_instance = nullptr;
    }
}

STT2DB* STT2DB::getInstance()
{
    if (m_instance) {
        return m_instance;
    }
    return nullptr;
}

int STT2DB::searchCallInfo(std::string counselorcode)
{
    Connection_T con;
    int ret=0;

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            m_Logger->error("STT2DB::searchCallInfo - can't get connection from pool");
            restartConnectionPool();
            return -1;
        }
        else if ( !Connection_ping(con) ) {
            m_Logger->error("STT2DB::searchCallInfo - inactive connection from pool");
            Connection_close(con);
            return -2;
        }
        ResultSet_T r = Connection_executeQuery(con, "SELECT counselor_code,center_code,status FROM CS_LIST WHERE counselor_code='%s'", counselorcode.c_str());

        ret = ResultSet_next(r);
    }
    CATCH(SQLException)
    {
        m_Logger->error("STT2DB::searchCallInfo - SQLException -- %s", Exception_frame.message);
    }
    FINALLY
    {
        //Connection_close(con);
        ConnectionPool_returnConnection(m_pool, con);
    }
    END_TRY;
    return ret;
}

int STT2DB::insertCallInfo(std::string counselorcode, std::string callid)
{
    Connection_T con;

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            m_Logger->error("STT2DB::insertCallInfo - can't get connection from pool");
            restartConnectionPool();
            return 1;
        }
        else if ( !Connection_ping(con) ) {
            m_Logger->error("STT2DB::insertCallInfo - inactive connection from pool");
            Connection_close(con);
            return 2;
        }
        Connection_execute(con, "INSERT INTO CS_LIST (counselor_code,call_id,status,reg_dttm) VALUES ('%s','%s','I',now())",
        counselorcode.c_str(), callid.c_str());
        m_Logger->debug("STT2DB::insertCallInfo - SQL[INSERT INTO CS_LIST (counselor_code,call_id,reg_dttm) VALUES ('%s','%s',now())]", counselorcode.c_str(), callid.c_str());
    }
    CATCH(SQLException)
    {
        m_Logger->error("STT2DB::insertCallInfo - SQLException -- %s", Exception_frame.message);
    }
    FINALLY
    {
        //Connection_close(con);
        ConnectionPool_returnConnection(m_pool, con);
    }
    END_TRY;
    return 0;
}

int STT2DB::updateCallInfo(std::string callid, bool end)
{
    Connection_T con;

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            m_Logger->error("STT2DB::updateCallInfo - can't get connection from pool");
            restartConnectionPool();
            return 1;
        }
        else if ( !Connection_ping(con) ) {
            m_Logger->error("STT2DB::updateCallInfo - inactive connection from pool");
            Connection_close(con);
            return 2;
        }
        if (!end) {
            Connection_execute(con, "UPDATE CS_LIST SET status='I' WHERE call_id='%s'",
            callid.c_str());
        }
        else {
            Connection_execute(con, "UPDATE CS_LIST SET status='E' WHERE call_id='%s'",
            callid.c_str());
        }
    }
    CATCH(SQLException)
    {
        m_Logger->error("STT2DB::updateCallInfo - SQLException -- %s", Exception_frame.message);
    }
    FINALLY
    {
        //Connection_close(con);
        ConnectionPool_returnConnection(m_pool, con);
    }
    END_TRY;
    return 0;
}

int STT2DB::updateCallInfo(std::string counselorcode, std::string callid, bool end)
{
    Connection_T con;

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            m_Logger->error("STT2DB::updateCallInfo - can't get connection from pool");
            restartConnectionPool();
            return 1;
        }
        else if ( !Connection_ping(con) ) {
            m_Logger->error("STT2DB::updateCallInfo - inactive connection from pool");
            Connection_close(con);
            return 2;
        }
        if (!end) {
            Connection_execute(con, "UPDATE CS_LIST SET status='I', call_id='%s' WHERE counselor_code='%s'",
            callid.c_str(), counselorcode.c_str());
            m_Logger->debug("STT2DB::updateCallInfo - SQL[UPDATE CS_LIST SET status='I', call_id='%s' WHERE counselor_code='%s']",callid.c_str(), counselorcode.c_str());
        }
        else {
            Connection_execute(con, "UPDATE CS_LIST SET status='E', call_id='%s' WHERE counselor_code='%s'",
            callid.c_str(), counselorcode.c_str());
            m_Logger->debug("STT2DB::updateCallInfo - SQL[UPDATE CS_LIST SET status='E', call_id='%s' WHERE counselor_code='%s']",callid.c_str(), counselorcode.c_str());
        }
    }
    CATCH(SQLException)
    {
        m_Logger->error("STT2DB::updateCallInfo - SQLException -- %s", Exception_frame.message);
    }
    FINALLY
    {
        //Connection_close(con);
        ConnectionPool_returnConnection(m_pool, con);
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

// VFCLient모듈에서 사용되는 api로서 해당 task를 작업하기 직전 DB에 task 정보를 등록할 때 사용
// args: call_id, counselor_code etc
int STT2DB::insertTaskInfo(std::string downloadPath, std::string filename, std::string callId)
{
    Connection_T con;

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            m_Logger->error("STT2DB::insertTaskInfo - can't get connection from pool");
            restartConnectionPool();
            return 1;
        }
        else if ( !Connection_ping(con) ) {
            m_Logger->error("STT2DB::insertTaskInfo - inactive connection from pool");
            Connection_close(con);
            return 2;
        }
        Connection_execute(con, "INSERT INTO JOB_INFO (call_id, pathname, filename, reg_dttm) VALUES ('%s', '%s', '%s', now())",
        callId.c_str(), downloadPath.c_str(), filename.c_str());
    }
    CATCH(SQLException)
    {
        m_Logger->error("STT2DB::insertTaskInfo - SQLException -- %s", Exception_frame.message);
    }
    FINALLY
    {
        //Connection_close(con);
        ConnectionPool_returnConnection(m_pool, con);
    }
    END_TRY;
    return 0;
}

// VFClient모듈에서 사용되는 api로서 해당 task 작업 종료 후 상태 값을 update할 때 사용
// args: call_id, counselor_code, task_stat etc
int STT2DB::updateTaskInfo(std::string callid, std::string counselorcode, char state)
{
    Connection_T con;

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            m_Logger->error("STT2DB::updateTaskInfo - can't get connection from pool");
            restartConnectionPool();
            return 1;
        }
        else if ( !Connection_ping(con) ) {
            m_Logger->error("STT2DB::updateTaskInfo - inactive connection from pool");
            Connection_close(con);
            return 2;
        }
        Connection_execute(con, "UPDATE JOB_INFO SET STATE='%c' WHERE call_id='%s' and counselor_code='%s'",
        state, callid.c_str(), counselorcode.c_str());
    }
    CATCH(SQLException)
    {
        m_Logger->error("STT2DB::updateTaskInfo - SQLException -- %s", Exception_frame.message);
    }
    FINALLY
    {
        //Connection_close(con);
        ConnectionPool_returnConnection(m_pool, con);
    }
    END_TRY;
    return 0;
}

// VFClient모듈에서 사용되는 api로서 해당 task에 대해 이전에 작업한 내용인지 아닌지 확인하기 위해 사용
// args: call_id, counselor_code etc
int STT2DB::searchTaskInfo(std::string downloadPath, std::string filename, std::string callId)
{
    Connection_T con;
    int ret=0;

    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            m_Logger->error("STT2DB::searchTaskInfo - can't get connection from pool");
            restartConnectionPool();
            return -1;
        }
        else if ( !Connection_ping(con) ) {
            m_Logger->error("STT2DB::searchTaskInfo - inactive connection from pool");
            Connection_close(con);
            return -2;
        }
        ResultSet_T r = Connection_executeQuery(con, "SELECT call_id,counselor_code,pathname,filename FROM JOB_INFO WHERE call_id = '%s' and filename = '%s'",
        callId.c_str(), filename.c_str());

        ret = ResultSet_next(r);
    }
    CATCH(SQLException)
    {
        m_Logger->error("STT2DB::searchTaskInfo - SQLException -- %s", Exception_frame.message);
    }
    FINALLY
    {
        //Connection_close(con);
        ConnectionPool_returnConnection(m_pool, con);
    }
    END_TRY;
    return ret;
}

int STT2DB::getTaskInfo(std::vector< JobInfoItem* > &v) 
{
    Connection_T con;
    int ret=0;

    m_Logger->debug("BEFORE STT2DB::getTaskInfo - ConnectionPool_size(%d), ConnectionPool_active(%d)", ConnectionPool_size(m_pool), ConnectionPool_active(m_pool));
    TRY
    {
        con = ConnectionPool_getConnection(m_pool);
        if ( con == NULL) {
            m_Logger->error("STT2DB::getTaskInfo - can't get connection from pool");
            restartConnectionPool();
            return -1;
        }
        else if ( !Connection_ping(con) ) {
            m_Logger->error("STT2DB::getTaskInfo - inactive connection from pool");
            Connection_close(con);
            return -2;
        }
        ResultSet_T r = Connection_executeQuery(con, "SELECT call_id,counselor_code,pathname,filename FROM JOB_INFO WHERE state = 'N' or state = 'X'");

        while (ResultSet_next(r)) 
        {
            std::string callid = ResultSet_getString(r, 1);
            std::string counselorcode = ResultSet_getString(r, 2);
            std::string path = ResultSet_getString(r, 3);
            std::string filename = ResultSet_getString(r, 4);

            JobInfoItem *item = new JobInfoItem(callid, counselorcode, path, filename);
            v.push_back(item);
        }

        ret = v.size();
    }
    CATCH(SQLException)
    {
        m_Logger->error("STT2DB::getTaskInfo - SQLException -- %s", Exception_frame.message);
    }
    FINALLY
    {
        Connection_close(con);
        //ConnectionPool_returnConnection(m_pool, con);
        m_Logger->debug("AFTER STT2DB::getTaskInfo - ConnectionPool_size(%d), ConnectionPool_active(%d)", ConnectionPool_size(m_pool), ConnectionPool_active(m_pool));
    }
    END_TRY;
    return ret;
}

void STT2DB::restartConnectionPool()
{
    m_Logger->debug("STT2DB::restartConnectionPool - size(%d), active(%d)", ConnectionPool_size(m_pool), ConnectionPool_active(m_pool));
    ConnectionPool_stop(m_pool);
    ConnectionPool_start(m_pool);
}

RTSTTQueItem::RTSTTQueItem(uint32_t idx, std::string callid, uint8_t spkno, std::string sttvalue, uint64_t bpos, uint64_t epos)
	:m_nDiaIdx(idx), m_sCallId(callid), m_nSpkNo(spkno), m_sSTTValue(sttvalue), m_nBpos(bpos), m_nEpos(epos)
{
}

RTSTTQueItem::~RTSTTQueItem()
{
}

JobInfoItem::JobInfoItem(std::string callid, std::string counselorcode, std::string path, std::string filename)
: m_callid(callid), m_counselorcode(counselorcode), m_path(path), m_filename(filename)
{

}

JobInfoItem::~JobInfoItem()
{

}