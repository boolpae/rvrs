
#include "DBHandler_ODBC.h"

#include "ItfOdbcPool.h"
#include "stas.h"

#include <iconv.h>
#include <string.h>
#include <stdio.h>

#include <sql.h>
#include <sqlext.h>


static int extract_error(const char *fn, SQLHANDLE handle, SQLSMALLINT type)
{
    SQLINTEGER i = 0;
    SQLINTEGER NativeError;
    SQLCHAR SQLState[ 7 ];
    SQLCHAR MessageText[256];
    SQLSMALLINT TextLength;
    SQLRETURN ret;
    log4cpp::Category *logger = config->getLogger();

    do
    {
        ret = SQLGetDiagRec(type, handle, ++i, SQLState, &NativeError,
                            MessageText, sizeof(MessageText), &TextLength);
    fprintf(stderr, "\nDEBUG - The driver reported the following error %s, ret(%d)\n", fn, ret);
        if (SQL_SUCCEEDED(ret)) {
            logger->error("%s:%ld:%ld:%s",
                        SQLState, (long) i, (long) NativeError, MessageText);
        }
        else if (ret < 0) {
            NativeError = 2006;
        }
    }
    while( ret == SQL_SUCCESS );

    return NativeError;
}

DBHandler* DBHandler::m_instance = nullptr;
#ifdef USE_UPDATE_POOL
bool DBHandler::m_bThrdMain = false;
bool DBHandler::m_bThrdUpdate = false;
#endif

DBHandler::DBHandler(std::string dsn,int connCount)
: m_sDsn(dsn), m_nConnCount(connCount), m_bLiveFlag(true), m_bInterDBUse(false)
{
    m_Logger = config->getLogger();
    m_pSolDBConnPool = nullptr;
    m_pInterDBConnPool = nullptr;
	m_Logger->debug("DBHandler Constructed.\n");
}

DBHandler::~DBHandler()
{
    m_bLiveFlag = false;
    while(
        !m_bThrdMain
#ifdef USE_UPDATE_POOL
        || !m_bThrdUpdate
#endif
        ) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (m_pSolDBConnPool) {
        delete m_pSolDBConnPool;
    }

    if (m_bInterDBUse && m_pInterDBConnPool) {
        delete m_pInterDBConnPool;
    }

    if (m_thrd.joinable()) m_thrd.detach();
#ifdef USE_UPDATE_POOL
    if (m_thrdUpdate.joinable()) m_thrdUpdate.detach();
#endif
    DBHandler::m_instance = nullptr;
	m_Logger->debug("DBHandler Destructed.\n");
}

void DBHandler::thrdMain(DBHandler * s2d)
{
	std::lock_guard<std::mutex> *g;
	RTSTTQueItem* item;
    log4cpp::Category *logger;

    int ret=0;
    char *utf_buf = NULL;
    size_t in_size, out_size;
    iconv_t it;
    char *input_buf_ptr = NULL;
    char *output_buf_ptr = NULL;
    char cSpk='N';
    char sRxTx[8];
    char sqlbuff[1024];
    int nIdx;

    SQLLEN lenStt, lenIdx, lenCallid, /*lenSpk,*/ lenStart, lenEnd;
    char sttValue[2048];
    char callId[256];
    int nStart, nEnd;

    SQLRETURN retcode;

    SQLSMALLINT NumParams;

    logger = config->getLogger();
    PConnSet connSet = s2d->m_pSolDBConnPool->getConnection();//ConnectionPool_getConnection(s2d->m_pool);

    it = iconv_open("UTF-8", "EUC-KR");

    // sprintf(sqlbuff, "INSERT INTO JOB_DATA (idx,call_id,pos_start,pos_end,value,spk) VALUES (?,?,?,?,?)");
    // printf("QUERY<%s>\n", sqlbuff);

    // retcode = SQLPrepare(connSet->stmt, (SQLCHAR*)sqlbuff, SQL_NTS);
    // printf("SQLPrepare RET(%d)\n", retcode);

    retcode = SQLBindParameter(connSet->stmt, 1, SQL_PARAM_INPUT,
                        SQL_C_SLONG, SQL_INTEGER, 0, 0,
                        (SQLPOINTER)&nIdx, sizeof(SQLINTEGER), (SQLLEN*)&lenIdx);
    retcode = SQLBindParameter(connSet->stmt, 2, SQL_PARAM_INPUT,
                        SQL_C_CHAR, SQL_VARCHAR, sizeof(callId), 0,
                        (SQLPOINTER)callId, sizeof(callId), (SQLLEN*)&lenCallid);
    retcode = SQLBindParameter(connSet->stmt, 3, SQL_PARAM_INPUT,
                        SQL_C_SLONG, SQL_INTEGER, 0, 0,
                        (SQLPOINTER)&nStart, sizeof(SQLINTEGER), (SQLLEN*)&lenStart);
    retcode = SQLBindParameter(connSet->stmt, 4, SQL_PARAM_INPUT,
                        SQL_C_SLONG, SQL_INTEGER, 0, 0,
                        (SQLPOINTER)&nEnd, sizeof(SQLINTEGER), (SQLLEN*)&lenEnd);
    retcode = SQLBindParameter(connSet->stmt, 5, SQL_PARAM_INPUT,
                        SQL_C_CHAR, SQL_VARCHAR, sizeof(sttValue), 0,
                        (SQLPOINTER)sttValue, sizeof(sttValue), (SQLLEN*)&lenStt);
    // retcode = SQLBindParameter(connSet->stmt, 6, SQL_PARAM_INPUT,
    //                     SQL_C_CHAR, SQL_CHAR, 1, 0,
    //                     (SQLPOINTER)sSpk, sizeof(sSpk), (SQLLEN*)&lenSpk);
    // printf("SQLBindParameter RET(%d)\n", retcode);

    // retcode = SQLNumParams(connSet->stmt, &NumParams);
    // printf("SQLNumParams RET(%d) NumParams(%d)\n", retcode, NumParams);

	while (s2d->m_bLiveFlag) {
		while (!s2d->m_qRtSttQue.empty()) {
			g = new std::lock_guard<std::mutex>(s2d->m_mxQue);
			item = s2d->m_qRtSttQue.front();
			s2d->m_qRtSttQue.pop();
			delete g;

            in_size = item->getSTTValue().size();
            out_size = in_size * 2 + 1;
            utf_buf = (char *)malloc(out_size);

            if (utf_buf) {
                memset(utf_buf, 0, out_size);

                input_buf_ptr = (char *)item->getSTTValue().c_str();
                output_buf_ptr = utf_buf;

                // it = iconv_open("UTF-8", "EUC-KR");

                ret = iconv(it, &input_buf_ptr, &in_size, &output_buf_ptr, &out_size);
                 
                // iconv_close(it);
                
            }
            else {
                utf_buf = nullptr;
                ret = -1;
            }

            // insert rtstt to db
            switch(item->getSpkNo()) {
                case 1:
                    cSpk = 'R';
                    sprintf(sRxTx, "%s", "RX");
                    break;
                case 2:
                    cSpk = 'L';
                    sprintf(sRxTx, "%s", "TX");
                    break;
                default:
                    cSpk = 'N';
                    sprintf(sRxTx, "%s", "MN");
            }

            sprintf(sqlbuff, "INSERT INTO TBL_JOB_DATA (IDX,SPK,CALL_ID,POS_START,POS_END,VALUE,RCD_TP) VALUES (?,'%c',?,?,?,?,'%s')", cSpk, sRxTx);
            //printf("QUERY<%s>\n", sqlbuff);

            retcode = SQLPrepare(connSet->stmt, (SQLCHAR*)sqlbuff, SQL_NTS);
            //printf("SQLPrepare RET(%d)\n", retcode);
            if (retcode < 0) {
                extract_error("SQLPrepare()", connSet->stmt, SQL_HANDLE_STMT);
            }

            nIdx = item->getDiaIdx();
            nStart = item->getBpos();
            nEnd = item->getEpos();
            // sprintf(sSpk, "%c", cSpk);
            sprintf(callId, "%s", item->getCallId().c_str());
            sprintf(sttValue, "%s", utf_buf);//item->getSTTValue().c_str());
            // lenSpk = strlen(sSpk);
            lenCallid = strlen(callId);
            lenStt = strlen(sttValue);
            
            // printf("nIdx(%d, %d), nStart(%d), nEnd(%d) callId(%s) sttValue (%s), len(%d,%d) inout_size(%d,%d)\n", 
            //      nIdx, lenIdx, nStart, nEnd, callId, utf_buf, lenCallid, lenStt, in_size, out_size);

            retcode = SQLExecute(connSet->stmt);//SQLExecDirect (connSet->stmt, (SQLCHAR*)sqlbuff, SQL_NTS);//
            //printf("SQLExecute RET(%d)\n", retcode);
            //retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

            if (retcode < 0) {
                int odbcret = extract_error("SQLExecute()", connSet->stmt, SQL_HANDLE_STMT);
                //s2d->m_Logger->error("DBHandler::insertCallInfo - SQLException -- %s", Exception_frame.message);
                // connSet = s2d->m_pSolDBConnPool->reconnectConnection(connSet);

                if ((odbcret == 2006) && s2d->m_pSolDBConnPool->reconnectConnection(connSet)) {
                    retcode = SQLBindParameter(connSet->stmt, 1, SQL_PARAM_INPUT,
                                        SQL_C_SLONG, SQL_INTEGER, 0, 0,
                                        (SQLPOINTER)&nIdx, sizeof(SQLINTEGER), (SQLLEN*)&lenIdx);
                    retcode = SQLBindParameter(connSet->stmt, 2, SQL_PARAM_INPUT,
                                        SQL_C_CHAR, SQL_VARCHAR, sizeof(callId), 0,
                                        (SQLPOINTER)callId, sizeof(callId), (SQLLEN*)&lenCallid);
                    retcode = SQLBindParameter(connSet->stmt, 3, SQL_PARAM_INPUT,
                                        SQL_C_SLONG, SQL_INTEGER, 0, 0,
                                        (SQLPOINTER)&nStart, sizeof(SQLINTEGER), (SQLLEN*)&lenStart);
                    retcode = SQLBindParameter(connSet->stmt, 4, SQL_PARAM_INPUT,
                                        SQL_C_SLONG, SQL_INTEGER, 0, 0,
                                        (SQLPOINTER)&nEnd, sizeof(SQLINTEGER), (SQLLEN*)&lenEnd);
                    retcode = SQLBindParameter(connSet->stmt, 5, SQL_PARAM_INPUT,
                                        SQL_C_CHAR, SQL_VARCHAR, sizeof(sttValue), 0,
                                        (SQLPOINTER)sttValue, sizeof(sttValue), (SQLLEN*)&lenStt);

                }
#if 0
                else {
                    if (DBHandler::getInstance() && connSet)
                        s2d->m_pSolDBConnPool->restoreConnection(connSet);

                    s2d->m_bLiveFlag = true;
                    logger->error("DBHandler::thrdMain() finish! - for SQLExecute ERROR\n");
                    return;
                }
#endif
            }
            retcode = SQLCloseCursor(connSet->stmt);
            if (utf_buf) free(utf_buf);
			delete item;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
        //printf("DBHandler::thrdMain()\n");
	}

    iconv_close(it);

    if (DBHandler::getInstance() && connSet)
        s2d->m_pSolDBConnPool->restoreConnection(connSet);

    m_bThrdMain = false;
    logger->debug("DBHandler::thrdMain() finish!\n");
}

#ifdef USE_UPDATE_POOL
void DBHandler::thrdUpdate(DBHandler *s2d)
{
	std::lock_guard<std::mutex> *g;
	UpdateInfoItem* item;
    log4cpp::Category *logger;

    time_t rawtime;
    struct tm * timeinfo;
    char timebuff [32];
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    logger = config->getLogger();
    PConnSet connSet = s2d->m_pSolDBConnPool->getConnection();//ConnectionPool_getConnection(s2d->m_pool);

	while (s2d->m_bLiveFlag) {
		while (!s2d->m_qUpdateInfoQue.empty()) {
			g = new std::lock_guard<std::mutex>(s2d->m_mxUpdateQue);
			item = s2d->m_qUpdateInfoQue.front();
			s2d->m_qUpdateInfoQue.pop();
			delete g;

            time (&rawtime);
            timeinfo = localtime (&rawtime);

            strftime (timebuff,sizeof(timebuff),"%F %T",timeinfo);
            //sprintf(sqlbuff, "UPDATE TBL_JOB_INFO SET STATE='%c' WHERE CALL_ID='%s' AND CS_CODE='%s'",
            //    state, callid.c_str(), counselorcode.c_str());
            if (item->getErrCode().size()) {
                // sprintf(sqlbuff, "UPDATE %s SET STATE='%c',ERR_CD='%s' WHERE CALL_ID='%s' AND RCD_TP='%s'",
                //     tbName, state, errcode, callid.c_str(), rxtx.c_str());
                sprintf(sqlbuff, "CALL PROC_JOB_STATISTIC_DAILY('%s','%s','%s','%d','%d','%d','%c','%s','%s')",
                    item->getCallId().c_str(), item->getRxTx().c_str(), item->getServerName().c_str(), item->getPlayLength(), item->getFileSize(), item->getWorkingTime(), item->getState(), item->getErrCode().c_str(), timebuff);
            }
            else {
                // sprintf(sqlbuff, "UPDATE %s SET STATE='%c',FILE_SIZE=%d,REC_LENGTH=%d,WORKING_TIME=%d WHERE CALL_ID='%s' AND RCD_TP='%s'",
                //     tbName, state, fsize, plen, wtime, callid.c_str(), rxtx.c_str());
                sprintf(sqlbuff, "CALL PROC_JOB_STATISTIC_DAILY('%s','%s','%s','%d','%d','%d','%c','','%s')",
                    item->getCallId().c_str(), item->getRxTx().c_str(), item->getServerName().c_str(), item->getPlayLength(), item->getFileSize(), item->getWorkingTime(), item->getState(), timebuff);
            }

            retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

            if SQL_SUCCEEDED(retcode) {
                logger->debug("DBHandler::thrdUpdate() - Query<%s>", sqlbuff);
            }
            else {
                int odbcret = extract_error("DBHandler::thrdUpdate() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
                if (odbcret == 2006) {
                    s2d->m_pSolDBConnPool->reconnectConnection(connSet);
                }
                ret = 1;
            }
			delete item;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
        //printf("DBHandler::thrdMain()\n");
	}

    if (DBHandler::getInstance() && connSet) {
        retcode = SQLCloseCursor(connSet->stmt);
        s2d->m_pSolDBConnPool->restoreConnection(connSet);
    }

    m_bThrdUpdate = false;
    logger->debug("DBHandler::thrdUpdate() finish!\n");
}
#endif

DBHandler* DBHandler::instance(std::string dsn, int connCount=10)
{
    if (m_instance) return m_instance;
    
    m_instance = new DBHandler(dsn, connCount);
    
    m_instance->m_pSolDBConnPool = new ItfOdbcPool(dsn.c_str());
    
    if (m_instance->m_pSolDBConnPool->createConnections(connCount))
    {
        m_instance->m_thrd = std::thread(DBHandler::thrdMain, m_instance);
        m_bThrdMain = true;
#ifdef USE_UPDATE_POOL
        m_instance->m_thrdUpdate = std::thread(DBHandler::thrdUpdate, m_instance);
        m_bThrdUpdate = true;
#endif
    }
    else
    {
        config->getLogger()->error("DBHandler::instance - error: cant't get connection\n");
        
        delete m_instance;
        m_instance = nullptr;
    }

    return m_instance;
}

void DBHandler::release()
{
    if (m_instance) {
        delete m_instance;
    }
}

DBHandler* DBHandler::getInstance()
{
    if (m_instance) {
        return m_instance;
    }
    return nullptr;
}

int DBHandler::searchCallInfo(std::string counselorcode)
{
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0, siCnt;
    char sqlbuff[512];
    SQLRETURN retcode;
    RETCODE rc = SQL_SUCCESS;

    if (connSet)
    {
        //sprintf(sqlbuff, "SELECT CS_CD,CT_CD,STAT FROM TBL_CS_LIST WHERE CS_CD='%s'", counselorcode.c_str());
        sprintf(sqlbuff, "SELECT CS_CD FROM TBL_CS_LIST WHERE CS_CD='%s'", counselorcode.c_str());
        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);
#if 0
        if (retcode == SQL_NO_DATA) {
            ret = 0;
	    }
	    else if (retcode == SQL_SUCCESS){
            ret = 1;
        }
#endif
        if SQL_SUCCEEDED(retcode) {
        while (1)//(SQLFetch(connSet->stmt) == SQL_SUCCESS) 
        {
            rc = SQLFetch( connSet->stmt );
            if (rc == SQL_NO_DATA_FOUND)
            {
                printf("End of data.\n" );
                break;
            }
            ret++;

            // SQLGetData(connSet->stmt, 1, SQL_C_SLONG, &ret, 0, (SQLLEN *)&siCnt);

            break;
        }
        }
        else {
            int odbcret = extract_error("DBHandler::insertCallInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            if (odbcret == 2006) {
                m_pSolDBConnPool->reconnectConnection(connSet);
            }
            ret = -1;

        }
        printf("DEBUG(searchCallInfo) - SQL(%s), ret(%d)\n", sqlbuff, ret);
        retcode = SQLCloseCursor(connSet->stmt);
        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        // error
        m_Logger->error("DBHandler::searchCallInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

int DBHandler::insertCallInfo(std::string counselorcode, std::string callid)
{
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    char sqlbuff[512];
    SQLRETURN retcode;
    int ret=0;

    if (connSet)
    {
        sprintf(sqlbuff, "INSERT INTO TBL_CS_LIST (CS_CD,CT_CD,CALL_ID,STAT,REG_DTM) VALUES ('%s','1','%s','I',now())",
            counselorcode.c_str(), callid.c_str());

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("DBHandler::insertCallInfo - SQL[INSERT INTO TBL_CS_LIST (CS_CD,CT_CD,CALL_ID,STAT,REG_DTM) VALUES ('%s','1','%s',now())]", counselorcode.c_str(), callid.c_str());
        }
        else {
            int odbcret = extract_error("DBHandler::insertCallInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            if (odbcret == 2006) {
                m_pSolDBConnPool->reconnectConnection(connSet);
            }
            ret = 1;
        }
        retcode = SQLCloseCursor(connSet->stmt);
        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("DBHandler::insertCallInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

int DBHandler::updateCallInfo(std::string callid, bool end)
{
    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        if (!end) {
            sprintf(sqlbuff, "UPDATE TBL_CS_LIST SET STAT='I' WHERE CALL_ID='%s'",
            callid.c_str());
        }
        else {
            sprintf(sqlbuff, "UPDATE TBL_CS_LIST SET STAT='E' WHERE CALL_ID='%s'",
            callid.c_str());
        }

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("UPDATE TBL_CS_LIST SET STAT='%c' WHERE CALL_ID='%s'",
                (end)?'E':'I', callid.c_str());
        }
        else {
            int odbcret = extract_error("DBHandler::updateCallInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            if (odbcret == 2006) {
                m_pSolDBConnPool->reconnectConnection(connSet);
            }
            ret = 1;
        }
        retcode = SQLCloseCursor(connSet->stmt);

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("DBHandler::updateCallInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

int DBHandler::updateCallInfo(std::string counselorcode, std::string callid, bool end)
{
    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        if (!end) {
            sprintf(sqlbuff, "UPDATE TBL_CS_LIST SET STAT='I', CALL_ID='%s' WHERE CS_CD='%s'",
            callid.c_str(), counselorcode.c_str());
            //m_Logger->debug("DBHandler::updateCallInfo - SQL[UPDATE CS_LIST SET status='I', call_id='%s' WHERE counselor_code='%s']",callid.c_str(), counselorcode.c_str());
        }
        else {
            sprintf(sqlbuff, "UPDATE TBL_CS_LIST SET STAT='E', CALL_ID='%s' WHERE CS_CD='%s'",
            callid.c_str(), counselorcode.c_str());
             //m_Logger->debug("DBHandler::updateCallInfo - SQL[UPDATE CS_LIST SET status='E', call_id='%s' WHERE counselor_code='%s']",callid.c_str(), counselorcode.c_str());
        }

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("DBHandler::updateCallInfo - SQL[UPDATE TBL_CS_LIST SET STAT='%c', CALL_ID='%s' WHERE CS_CD='%s']", (end)?'E':'I', callid.c_str(), counselorcode.c_str());
        }
        else {
            int odbcret = extract_error("DBHandler::updateCallInfo(2) - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            if (odbcret == 2006) {
                m_pSolDBConnPool->reconnectConnection(connSet);
            }
            ret = 1;
        }
        retcode = SQLCloseCursor(connSet->stmt);

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("DBHandler::updateCallInfo(2) - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

void DBHandler::insertSTTData(uint32_t idx, std::string callid, uint8_t spkno, uint64_t spos, uint64_t epos, std::string &stt)
{
	insertSTTData(new RTSTTQueItem(idx, callid, spkno, stt, spos, epos));
}

void DBHandler::insertSTTData(RTSTTQueItem * item)
{
	std::lock_guard<std::mutex> g(m_mxQue);
	m_qRtSttQue.push(item);
}

int DBHandler::insertFullSttData(std::string callid, std::string &stt)
{
    return 0;
}

void DBHandler::insertBatchTask()
{
    // insert
}

int DBHandler::getBatchTask()
{
	std::lock_guard<std::mutex> g(m_mxDb);
    
    // select, update
    
    return 0;
}

void DBHandler::deleteBatchTask()
{
    // delete
}

// VFCLient모듈에서 사용되는 api로서 해당 task를 작업하기 직전 DB에 task 정보를 등록할 때 사용
// args: call_id, counselor_code etc
int DBHandler::insertTaskInfo(std::string downloadPath, std::string filename, std::string callId)
{
    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        sprintf(sqlbuff, "INSERT INTO TBL_JOB_INFO (CALL_ID,SV_NM,PATH_NM,FILE_NM,REG_DTM,STATE) VALUES ('%s','DEFAULT','%s','%s',now(),'I')",
            callId.c_str(), downloadPath.c_str(), filename.c_str());

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("INSERT INTO TBL_JOB_INFO (CALL_ID,SV_NM,PATH_NM,FILE_NM,REG_DTM,STATE) VALUES ('%s','DEFAULT','%s','%s',now(),'I')",
                callId.c_str(), downloadPath.c_str(), filename.c_str());
        }
        else {
            int odbcret = extract_error("DBHandler::insertTaskInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            if (odbcret == 2006) {
                m_pSolDBConnPool->reconnectConnection(connSet);
            }
            ret = 1;
        }
        retcode = SQLCloseCursor(connSet->stmt);

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("DBHandler::insertTaskInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

int DBHandler::insertTaskInfoRT(std::string downloadPath, std::string filename, std::string callId, std::string counselcode)
{
    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        sprintf(sqlbuff, "INSERT INTO TBL_JOB_INFO (CALL_ID,SV_NM,CS_CD,PATH_NM,FILE_NM,REG_DTM,STATE) VALUES ('%s','DEFAULT','%s','%s','%s',now(),'U')",
            callId.c_str(), counselcode.c_str(), downloadPath.c_str(), filename.c_str());

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("INSERT INTO TBL_JOB_INFO (CALL_ID,SV_NM,CS_CD,PATH_NM,FILE_NM,REG_DTM,STATE) VALUES ('%s','DEFAULT','%s','%s','%s',now(),'U')",
                callId.c_str(), counselcode.c_str(), downloadPath.c_str(), filename.c_str());
        }
        else {
            int odbcret = extract_error("DBHandler::insertTaskInfoRT() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            if (odbcret == 2006) {
                m_pSolDBConnPool->reconnectConnection(connSet);
            }
            ret = 1;
        }
        retcode = SQLCloseCursor(connSet->stmt);

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("DBHandler::insertTaskInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

// VFClient모듈에서 사용되는 api로서 해당 task 작업 종료 후 상태 값을 update할 때 사용
// args: call_id, counselor_code, task_stat etc
int DBHandler::updateTaskInfo(std::string callid, std::string rxtx, std::string counselorcode, char state, int fsize, int plen, int wtime, const char *tbName, const char *errcode, const char *svr_nm)
{
    int ret=0;
#ifdef USE_UPDATE_POOL
	std::lock_guard<std::mutex> g(m_mxUpdateQue);
	m_qUpdateInfoQue.push(new UpdateInfoItem(callid, rxtx, counselorcode, state, fsize, plen, wtime, tbName, errcode, svr_nm));
#else
    // for strftime
    time_t rawtime;
    struct tm * timeinfo;
    char timebuff [32];
    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        time (&rawtime);
        timeinfo = localtime (&rawtime);

        strftime (timebuff,sizeof(timebuff),"%F %T",timeinfo);
        //sprintf(sqlbuff, "UPDATE TBL_JOB_INFO SET STATE='%c' WHERE CALL_ID='%s' AND CS_CODE='%s'",
        //    state, callid.c_str(), counselorcode.c_str());
        if (errcode && strlen(errcode)) {
            // sprintf(sqlbuff, "UPDATE %s SET STATE='%c',ERR_CD='%s' WHERE CALL_ID='%s' AND RCD_TP='%s'",
            //     tbName, state, errcode, callid.c_str(), rxtx.c_str());
            sprintf(sqlbuff, "CALL PROC_JOB_STATISTIC_DAILY('%s','%s','%s','%d','%d','%d','%c','%s','%s')",
                callid.c_str(), rxtx.c_str(), svr_nm, plen, fsize, wtime, state, errcode, timebuff);
        }
        else {
            // sprintf(sqlbuff, "UPDATE %s SET STATE='%c',FILE_SIZE=%d,REC_LENGTH=%d,WORKING_TIME=%d WHERE CALL_ID='%s' AND RCD_TP='%s'",
            //     tbName, state, fsize, plen, wtime, callid.c_str(), rxtx.c_str());
            sprintf(sqlbuff, "CALL PROC_JOB_STATISTIC_DAILY('%s','%s','%s','%d','%d','%d','%c','','%s')",
                callid.c_str(), rxtx.c_str(), svr_nm, plen, fsize, wtime, state, timebuff);
        }

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("DBHandler::updateTaskInfo() - Query<%s>", sqlbuff);
        }
        else {
            int odbcret = extract_error("DBHandler::updateTaskInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            if (odbcret == 2006) {
                m_pSolDBConnPool->reconnectConnection(connSet);
            }
            ret = 1;
        }
        retcode = SQLCloseCursor(connSet->stmt);

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("DBHandler::updateTaskInfo - can't get connection from pool");
        ret = -1;
    }
#endif
    return ret;
}

int DBHandler::updateTaskInfo(std::string callid, std::string rxtx, std::string counselorcode, std::string regdate, char state, int fsize, int plen, int wtime, const char *tbName, const char *errcode, const char *svr_nm)
{
    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        // sprintf(sqlbuff, "UPDATE TBL_JOB_INFO SET STATE='%c' WHERE CALL_ID='%s' AND CS_CODE='%s' AND REG_DTM='%s'",
        //     state, callid.c_str(), counselorcode.c_str(), regdate.c_str());
        if (errcode && strlen(errcode)) {
            sprintf(sqlbuff, "UPDATE %s SET STATE='%c',ERR_CD='%s',SV_NM='%s' WHERE CALL_ID='%s' AND RCD_TP='%s' AND REG_DTM='%s'",
                tbName, state, errcode, svr_nm, callid.c_str(), rxtx.c_str(), regdate.c_str());
        }
        else {
            sprintf(sqlbuff, "UPDATE %s SET STATE='%c',FILE_SIZE=%d,REC_LENGTH=%d,WORKING_TIME=%d,SV_NM='%s' WHERE CALL_ID='%s' AND RCD_TP='%s' AND REG_DTM='%s'",
                tbName, state, fsize, plen, wtime, svr_nm, callid.c_str(), rxtx.c_str(), regdate.c_str());
        }

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("DBHandler::updateTaskInfo2() - Query<%s>", sqlbuff);
        }
        else {
            int odbcret = extract_error("DBHandler::updateTaskInfo2() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            if (odbcret == 2006) {
                m_pSolDBConnPool->reconnectConnection(connSet);
            }
            ret = 1;
        }
        retcode = SQLCloseCursor(connSet->stmt);

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("DBHandler::updateTaskInfo2 - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

// VFClient모듈에서 사용되는 api로서 해당 task에 대해 이전에 작업한 내용인지 아닌지 확인하기 위해 사용
// args: call_id, counselor_code etc
int DBHandler::searchTaskInfo(std::string downloadPath, std::string filename, std::string callId)
{
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0, siCnt=SQL_NTS;
    char sqlbuff[512];
    SQLRETURN retcode;
    RETCODE rc = SQL_SUCCESS;
    // char callid[33];

    if (connSet)
    {
        //sprintf(sqlbuff, "SELECT CALL_ID,CS_CODE,PATHNAME,FILE_NAME FROM TBL_JOB_INFO WHERE CALL_ID='%s' AND FILE_NAME='%s'",
        sprintf(sqlbuff, "SELECT CALL_ID FROM TBL_JOB_INFO WHERE CALL_ID='%s' AND FILE_NM='%s'",
            callId.c_str(), filename.c_str());

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

#if 0
        if (retcode == SQL_NO_DATA) {
            m_Logger->error("DBHandler::searchTaskInfo - retcode(SQL_NO_DATA, %d)", retcode);
            ret = 0;
	    }
	    else if (retcode == SQL_SUCCESS){
            m_Logger->error("DBHandler::searchTaskInfo - retcode(SQL_SUCCESS, %d)", retcode);
            ret = 1;
        }
#endif
        if SQL_SUCCEEDED(retcode) {
        while (1)//(SQLFetch(connSet->stmt) == SQL_SUCCESS) 
        {
            rc = SQLFetch( connSet->stmt );
            if (rc == SQL_NO_DATA_FOUND)
            {
                printf("End of data.\n" );
                break;
            }
            ret++;
            // SQLGetData(connSet->stmt, 1, SQL_C_SLONG, &ret, 0, (SQLLEN *)&siCnt);
            break;
        }
        }
        else {
            int odbcret = extract_error("DBHandler::searchTaskInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            if (odbcret == 2006) {
                m_pSolDBConnPool->reconnectConnection(connSet);
            }
            ret = -1;
        }
        printf("DEBUG(searchTaskInfo) - ret(%d)\n", ret);

        retcode = SQLCloseCursor(connSet->stmt);
        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("DBHandler::searchTaskInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

int DBHandler::getTaskInfo(std::vector< JobInfoItem* > &v, int availableCount, const char *tableName) 
{
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;
    
    char callid[256];
    int counselorcode;
    char path[500];
    char filename[256];
    char regdate[24];
    char rxtx[8];
    int siCallId, siCCode, siPath, siFilename, siRxtx, siRegdate;

    //m_Logger->debug("BEFORE DBHandler::getTaskInfo - ConnectionPool_size(%d), ConnectionPool_active(%d), availableCount(%d)", ConnectionPool_size(m_pool), ConnectionPool_active(m_pool), availableCount);
    
    if (connSet)
    {
        sprintf(sqlbuff, "SELECT CALL_ID,CS_CD,PATH_NM,FILE_NM,REG_DTM,RCD_TP FROM %s WHERE STATE='I' ORDER BY REG_DTM ASC LIMIT %d", tableName, availableCount);
        //m_Logger->debug("BEFORE DBHandler::getTaskInfo - SQL(%s)", sqlbuff);
        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if (retcode == SQL_SUCCESS) {
            while (SQLFetch(connSet->stmt) == SQL_SUCCESS) 
            {
                memset(callid, 0, sizeof(callid));
                memset(path, 0, sizeof(path));
                memset(filename, 0, sizeof(filename));

                SQLGetData(connSet->stmt, 1, SQL_C_CHAR, callid, sizeof(callid)-1, (SQLLEN *)&siCallId);
                SQLGetData(connSet->stmt, 2, SQL_C_SLONG, &counselorcode, 0, (SQLLEN *)&siCCode);
                SQLGetData(connSet->stmt, 3, SQL_C_CHAR, path, sizeof(path)-1, (SQLLEN *)&siPath);
                SQLGetData(connSet->stmt, 4, SQL_C_CHAR, filename, sizeof(filename)-1, (SQLLEN *)&siFilename);
                SQLGetData(connSet->stmt, 5, SQL_C_CHAR, regdate, sizeof(regdate)-1, (SQLLEN *)&siRegdate);
                SQLGetData(connSet->stmt, 6, SQL_C_CHAR, rxtx, sizeof(rxtx)-1, (SQLLEN *)&siRxtx);

                JobInfoItem *item = new JobInfoItem(std::string(callid), std::to_string(counselorcode), std::string(path), std::string(filename), std::string(regdate), std::string(rxtx), std::string(tableName));
                v.push_back(item);
            }
        }
        else if (retcode < 0) {
            int odbcret = extract_error("DBHandler::getTaskInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            if (odbcret == 2006) {
                m_pSolDBConnPool->reconnectConnection(connSet);
            }
            ret = retcode;
        }
        retcode = SQLCloseCursor(connSet->stmt);
        m_pSolDBConnPool->restoreConnection(connSet);

        ret = v.size();
    }
    else
    {
        m_Logger->error("DBHandler::getTaskInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

int DBHandler::getTimeoutTaskInfo(std::vector< JobInfoItem* > &v) 
{
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;
    
    char callid[256];
    int counselorcode;
    char path[500];
    char filename[256];
    char regdate[24];
    char rxtx[8];
    int siCallId, siCCode, siPath, siFilename, siRxtx, siRegdate;

    //m_Logger->debug("BEFORE DBHandler::getTimeoutTaskInfo - ConnectionPool_size(%d), ConnectionPool_active(%d), availableCount(%d)", ConnectionPool_size(m_pool), ConnectionPool_active(m_pool), availableCount);
    
    if (connSet)
    {
        sprintf(sqlbuff, "SELECT CALL_ID,CS_CD,PATH_NM,FILE_NM,REG_DTM,RCD_TP FROM TBL_JOB_INFO WHERE REG_DTM >= concat(date(now()), ' 00:00:00') and REG_DTM <= concat(date(now()), ' 23:59:59') and STATE='U' and DATE_SUB(now(), INTERVAL 1 HOUR) > REG_DTM");
        //m_Logger->debug("BEFORE DBHandler::getTaskInfo - SQL(%s)", sqlbuff);
        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if (retcode == SQL_SUCCESS) {
            while (SQLFetch(connSet->stmt) == SQL_SUCCESS) 
            {
                memset(callid, 0, sizeof(callid));
                memset(path, 0, sizeof(path));
                memset(filename, 0, sizeof(filename));

                SQLGetData(connSet->stmt, 1, SQL_C_CHAR, callid, sizeof(callid)-1, (SQLLEN *)&siCallId);
                SQLGetData(connSet->stmt, 2, SQL_C_SLONG, &counselorcode, 0, (SQLLEN *)&siCCode);
                SQLGetData(connSet->stmt, 3, SQL_C_CHAR, path, sizeof(path)-1, (SQLLEN *)&siPath);
                SQLGetData(connSet->stmt, 4, SQL_C_CHAR, filename, sizeof(filename)-1, (SQLLEN *)&siFilename);
                SQLGetData(connSet->stmt, 5, SQL_C_CHAR, regdate, sizeof(regdate)-1, (SQLLEN *)&siRegdate);
                SQLGetData(connSet->stmt, 6, SQL_C_CHAR, rxtx, sizeof(rxtx)-1, (SQLLEN *)&siRxtx);

                JobInfoItem *item = new JobInfoItem(std::string(callid), std::to_string(counselorcode), std::string(path), std::string(filename), std::string(regdate), std::string(rxtx));
                v.push_back(item);
            }
        }
        else if (retcode < 0) {
            int odbcret = extract_error("DBHandler::getTimeoutTaskInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            if (odbcret == 2006) {
                m_pSolDBConnPool->reconnectConnection(connSet);
            }
            ret = retcode;
        }
        retcode = SQLCloseCursor(connSet->stmt);
        m_pSolDBConnPool->restoreConnection(connSet);

        ret = v.size();
    }
    else
    {
        m_Logger->error("DBHandler::getTimeoutTaskInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

void DBHandler::updateAllTask2Fail2()
{
    std::vector< JobInfoItem* > vItems;
    std::vector< JobInfoItem* >::iterator iter;
    JobInfoItem *jobInfo = nullptr;

    getTimeoutTaskInfo(vItems);

    if(vItems.size()) {
        PConnSet connSet = m_pSolDBConnPool->getConnection();
        char sqlbuff[512];
        SQLRETURN retcode;
        time_t rawtime;
        struct tm * timeinfo;
        char timebuff [32];

        if (connSet)
        {
            time (&rawtime);
            timeinfo = localtime (&rawtime);
            strftime (timebuff,sizeof(timebuff),"%F %T",timeinfo);

            for(iter= vItems.begin(); iter != vItems.end(); iter++) {
                jobInfo = (*iter);
                sprintf(sqlbuff, "CALL PROC_JOB_STATISTIC_DAILY('%s','%s','DEFAULT','0','0','0','X','TM_OUT','%s')",
                    jobInfo->getCallId().c_str(), jobInfo->getRxTxType().c_str(), timebuff);
                retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

                if SQL_SUCCEEDED(retcode) {
                    m_Logger->debug("DBHandler::updateAllTask2Fail2() - Query<%s>", sqlbuff);
                }
                else {
                    int odbcret = extract_error("DBHandler::updateAllTask2Fail2() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
                    if (odbcret == 2006) {
                        m_pSolDBConnPool->reconnectConnection(connSet);
                    }
                }

                delete jobInfo;
            }
            retcode = SQLCloseCursor(connSet->stmt);
            m_pSolDBConnPool->restoreConnection(connSet);
        }

        vItems.clear();
    } 
}
void DBHandler::updateAllTask2Fail()
{
    // 추후(필요 시) getTimeoutTaskInfo()를 활용하고 CALL PROCEDURE 형태로 바꾸어야 한다.

    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        sprintf(sqlbuff, "UPDATE TBL_JOB_INFO SET STATE='X',ERR_CD='E10200' WHERE REG_DTM >= concat(date(now()), ' 00:00:00') and REG_DTM <= concat(date(now()), ' 23:59:59') and STATE='U' and DATE_SUB(now(), INTERVAL 1 HOUR) > REG_DTM");

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);
        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("DBHandler::updateAllTask2Fail() - Query<%s>", sqlbuff);
        }
        else {
            int odbcret = extract_error("DBHandler::updateAllTask2Fail() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            if (odbcret == 2006) {
                m_pSolDBConnPool->reconnectConnection(connSet);
            }
        }
        retcode = SQLCloseCursor(connSet->stmt);

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("DBHandler::updateAllTask2Fail - can't get connection from pool");
    }

}

void DBHandler::setInterDBEnable(std::string dsn, int connCount=10)
{
    if (m_bInterDBUse) return;
    
    m_pInterDBConnPool = new ItfOdbcPool(dsn.c_str());
    
    if (m_pInterDBConnPool->createConnections(connCount))
    {
        m_bInterDBUse = true;
    }
    else
    {
        m_Logger->error("DBHandler::setInterDBEnable - failed to create InterDB ConnectionPool");
        
        delete m_pInterDBConnPool;
        
    }

}

void DBHandler::setInterDBDisable()
{
    if (m_bInterDBUse) {
        delete m_pInterDBConnPool;
        m_pInterDBConnPool = nullptr;
        m_bInterDBUse = false;
    }
}

RTSTTQueItem::RTSTTQueItem(uint32_t idx, std::string callid, uint8_t spkno, std::string &sttvalue, uint64_t bpos, uint64_t epos)
	:m_nDiaIdx(idx), m_sCallId(callid), m_nSpkNo(spkno), m_sSTTValue(sttvalue), m_nBpos(bpos), m_nEpos(epos)
{
}

RTSTTQueItem::~RTSTTQueItem()
{
}

JobInfoItem::JobInfoItem(std::string callid, std::string counselorcode, std::string path, std::string filename, std::string regdate, std::string rxtx, std::string tableName)
: m_callid(callid), m_counselorcode(counselorcode), m_path(path), m_filename(filename), m_regdate(regdate), m_rxtx(rxtx), m_tableName(tableName)
{

}

JobInfoItem::~JobInfoItem()
{

}

#ifdef USE_UPDATE_POOL
UpdateInfoItem::UpdateInfoItem(std::string callid, std::string rxtx, std::string counselorcode, char state, int fsize, int plen, int wtime, const char *tbName, const char *errcode, const char *svr_nm)
: callid(callid), rxtx(rxtx), counselorcode(counselorcode), state(state), fsize(fsize), plen(plen), wtime(wtime), tbName(tbName), errcode(errcode), svr_nm(svr_nm)
{

}

UpdateInfoItem::~UpdateInfoItem()
{

}
#endif
