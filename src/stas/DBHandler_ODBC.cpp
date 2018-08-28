
#include "DBHandler_ODBC.h"

#include "ItfOdbcPool.h"
#include "stas.h"

#include <iconv.h>
#include <string.h>
#include <stdio.h>

#include <sql.h>
#include <sqlext.h>


static void extract_error(const char *fn, SQLHANDLE handle, SQLSMALLINT type)
{
    SQLINTEGER i = 0;
    SQLINTEGER NativeError;
    SQLCHAR SQLState[ 7 ];
    SQLCHAR MessageText[256];
    SQLSMALLINT TextLength;
    SQLRETURN ret;
    log4cpp::Category *logger = config->getLogger();

    fprintf(stderr, "\nThe driver reported the following error %s\n", fn);
    do
    {
        ret = SQLGetDiagRec(type, handle, ++i, SQLState, &NativeError,
                            MessageText, sizeof(MessageText), &TextLength);
        if (SQL_SUCCEEDED(ret)) {
            logger->error("%s:%ld:%ld:%s",
                        SQLState, (long) i, (long) NativeError, MessageText);
        }
    }
    while( ret == SQL_SUCCESS );
}

DBHandler* DBHandler::m_instance = nullptr;

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
    while(!m_bLiveFlag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (m_pSolDBConnPool) {
        delete m_pSolDBConnPool;
    }

    if (m_bInterDBUse && m_pInterDBConnPool) {
        delete m_pInterDBConnPool;
    }

    if (m_thrd.joinable()) m_thrd.detach();

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
    // char sSpk[2];
    char sqlbuff[1024];
    int nIdx;

    int lenStt, lenIdx, lenCallid, /*lenSpk,*/ lenStart, lenEnd;
    char sttValue[501];
    char callId[256];
    int nStart, nEnd;

    SQLRETURN retcode;

    SQLSMALLINT NumParams;

    logger = config->getLogger();
    PConnSet connSet = s2d->m_pSolDBConnPool->getConnection();//ConnectionPool_getConnection(s2d->m_pool);

    // sprintf(sqlbuff, "INSERT INTO JOB_DATA (idx,call_id,pos_start,pos_end,value,spk) VALUES (?,?,?,?,?)");
    // printf("QUERY<%s>\n", sqlbuff);

    // retcode = SQLPrepare(connSet->stmt, (SQLCHAR*)sqlbuff, SQL_NTS);
    // printf("SQLPrepare RET(%d)\n", retcode);

    retcode = SQLBindParameter(connSet->stmt, 1, SQL_PARAM_INPUT,
                        SQL_C_SLONG, SQL_INTEGER, 0, 0,
                        (SQLPOINTER)&nIdx, sizeof(nIdx), (SQLLEN*)&lenIdx);
    retcode = SQLBindParameter(connSet->stmt, 2, SQL_PARAM_INPUT,
                        SQL_C_CHAR, SQL_VARCHAR, 255, 0,
                        (SQLPOINTER)callId, sizeof(callId), (SQLLEN*)&lenCallid);
    retcode = SQLBindParameter(connSet->stmt, 3, SQL_PARAM_INPUT,
                        SQL_C_SLONG, SQL_INTEGER, 0, 0,
                        (SQLPOINTER)&nStart, sizeof(nStart), (SQLLEN*)&lenStart);
    retcode = SQLBindParameter(connSet->stmt, 4, SQL_PARAM_INPUT,
                        SQL_C_SLONG, SQL_INTEGER, 0, 0,
                        (SQLPOINTER)&nEnd, sizeof(nEnd), (SQLLEN*)&lenEnd);
    retcode = SQLBindParameter(connSet->stmt, 5, SQL_PARAM_INPUT,
                        SQL_C_CHAR, SQL_VARCHAR, 500, 0,
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

#if 0
            // sprintf(sqlbuff, "INSERT INTO JOB_DATA (idx,call_id,spk,pos_start,pos_end,value) VALUES (%d,'%s','%c',%lu,%lu,'%s')",
            //     item->getDiaIdx(), item->getCallId().c_str(), cSpk, item->getBpos(), item->getEpos(), ((ret == -1) ? item->getSTTValue().c_str() : utf_buf));
            lenStt = item->getSTTValue().size();//strlen(input_buf_ptr);
            lenIdx = 11;
            sprintf(sqlbuff, "INSERT INTO JOB_DATA (idx,call_id,spk,pos_start,pos_end,value) VALUES (%d,'%s','%c',%lu,%lu,?)",
                item->getDiaIdx(), item->getCallId().c_str(), cSpk, item->getBpos(), item->getEpos());//, input_buf_ptr);
            printf("QUERY<%s>\n", sqlbuff);
#if 0
            retcode = SQLBindParameter(connSet->stmt, 1, SQL_PARAM_INPUT,
                               SQL_C_LONG, SQL_INTEGER, 11, 0,
                               (SQLPOINTER)&nIdx, 11, (SQLLEN*)&lenIdx);
#endif
            retcode = SQLBindParameter(connSet->stmt, 1, SQL_PARAM_INPUT,
                               SQL_C_CHAR, SQL_VARCHAR, 500, 0,
                               (SQLPOINTER)sttValue, 500, (SQLLEN*)&lenStt);
            printf("SQLBindParameter RET(%d)\n", retcode);

            retcode = SQLPrepare(connSet->stmt, (SQLCHAR*)sqlbuff, SQL_NTS);
            printf("SQLPrepare RET(%d)\n", retcode);
            retcode = SQLNumParams(connSet->stmt, &NumParams);
            printf("SQLNumParams RET(%d) NumParams(%d)\n", retcode, NumParams);
#endif
            sprintf(sqlbuff, "INSERT INTO TBL_JOB_DATA (IDX,SPK,CALL_ID,POS_START,POS_END,VALUE) VALUES (?,'%c',?,?,?,?)", cSpk);
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
            sprintf(sttValue, "%s", "utf_buf");//item->getSTTValue().c_str());
            // lenSpk = strlen(sSpk);
            lenCallid = strlen(callId);
            lenStt = strlen(sttValue);
            
            printf("nIdx(%d, %d), nStart(%d), nEnd(%d) callId(%s) sttValue (%s), len(%d)\n", 
                 nIdx, lenIdx, nStart, nEnd, callId, utf_buf, lenStt);

            retcode = SQLExecute(connSet->stmt);//SQLExecDirect (connSet->stmt, (SQLCHAR*)sqlbuff, SQL_NTS);//
            //printf("SQLExecute RET(%d)\n", retcode);
            //retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

            if (retcode < 0) {
                extract_error("SQLExecute()", connSet->stmt, SQL_HANDLE_STMT);
                //s2d->m_Logger->error("DBHandler::insertCallInfo - SQLException -- %s", Exception_frame.message);
                connSet = s2d->m_pSolDBConnPool->reconnectConnection(connSet);

                if (connSet) {
                    retcode = SQLBindParameter(connSet->stmt, 1, SQL_PARAM_INPUT,
                                        SQL_C_SLONG, SQL_INTEGER, 0, 0,
                                        (SQLPOINTER)&nIdx, sizeof(nIdx), (SQLLEN*)&lenIdx);
                    retcode = SQLBindParameter(connSet->stmt, 2, SQL_PARAM_INPUT,
                                        SQL_C_CHAR, SQL_VARCHAR, 255, 0,
                                        (SQLPOINTER)callId, sizeof(callId), (SQLLEN*)&lenCallid);
                    retcode = SQLBindParameter(connSet->stmt, 3, SQL_PARAM_INPUT,
                                        SQL_C_SLONG, SQL_INTEGER, 0, 0,
                                        (SQLPOINTER)&nStart, sizeof(nStart), (SQLLEN*)&lenStart);
                    retcode = SQLBindParameter(connSet->stmt, 4, SQL_PARAM_INPUT,
                                        SQL_C_SLONG, SQL_INTEGER, 0, 0,
                                        (SQLPOINTER)&nEnd, sizeof(nEnd), (SQLLEN*)&lenEnd);
                    retcode = SQLBindParameter(connSet->stmt, 5, SQL_PARAM_INPUT,
                                        SQL_C_CHAR, SQL_VARCHAR, 500, 0,
                                        (SQLPOINTER)sttValue, sizeof(sttValue), (SQLLEN*)&lenStt);

                }
                else {
                    if (DBHandler::getInstance() && connSet)
                        s2d->m_pSolDBConnPool->restoreConnection(connSet);

                    s2d->m_bLiveFlag = true;
                    logger->error("DBHandler::thrdMain() finish! - for SQLExecute ERROR\n");
                    return;
                }
            }
            retcode = SQLCloseCursor(connSet->stmt);
            if (utf_buf) free(utf_buf);
			delete item;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
        //printf("DBHandler::thrdMain()\n");
	}

    if (DBHandler::getInstance() && connSet)
        s2d->m_pSolDBConnPool->restoreConnection(connSet);

    s2d->m_bLiveFlag = true;
    logger->debug("DBHandler::thrdMain() finish!\n");
}

DBHandler* DBHandler::instance(std::string dsn, int connCount=10)
{
    if (m_instance) return m_instance;
    
    m_instance = new DBHandler(dsn, connCount);
    
    m_instance->m_pSolDBConnPool = new ItfOdbcPool(dsn.c_str());
    
    if (m_instance->m_pSolDBConnPool->createConnections(connCount))
    {
        m_instance->m_thrd = std::thread(DBHandler::thrdMain, m_instance);
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
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        //sprintf(sqlbuff, "SELECT CS_CD,CT_CD,STAT FROM TBL_CS_LIST WHERE CS_CD='%s'", counselorcode.c_str());
        sprintf(sqlbuff, "SELECT COUNT(CS_CD) FROM TBL_CS_LIST WHERE CS_CD='%s'", counselorcode.c_str());
        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);
#if 0
        if (retcode == SQL_NO_DATA) {
            ret = 0;
	    }
	    else if (retcode == SQL_SUCCESS){
            ret = 1;
        }
#endif
        while (SQLFetch(connSet->stmt) == SQL_SUCCESS) 
        {
            int resCnt=0, siCnt;

            SQLGetData(connSet->stmt, 1, SQL_C_SHORT, &ret, 0, (SQLLEN *)&siCnt);
            break;
        }
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

        retcode = SQLCloseCursor(connSet->stmt);
        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("DBHandler::insertCallInfo - SQL[INSERT INTO TBL_CS_LIST (CS_CD,CT_CD,CALL_ID,STAT,REG_DTM) VALUES ('%s','1','%s',now())]", counselorcode.c_str(), callid.c_str());
        }
        else {
            extract_error("DBHandler::insertCallInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            ret = 1;
        }
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

        retcode = SQLCloseCursor(connSet->stmt);
        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("UPDATE TBL_CS_LIST SET STAT='%c' WHERE CALL_ID='%s'",
                (end)?'E':'I', callid.c_str());
        }
        else {
            extract_error("DBHandler::updateCallInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            ret = 1;
        }

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

        retcode = SQLCloseCursor(connSet->stmt);
        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("DBHandler::updateCallInfo - SQL[UPDATE TBL_CS_LIST SET STAT='%c', CALL_ID='%s' WHERE CS_CD='%s']", (end)?'E':'I', callid.c_str(), counselorcode.c_str());
        }
        else {
            extract_error("DBHandler::updateCallInfo(2) - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            ret = 1;
        }

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
        sprintf(sqlbuff, "INSERT INTO TBL_JOB_INFO (CALL_ID,PATH_NM,FILE_NM,REG_DTM,STATE) VALUES ('%s','%s','%s',now(),'I')",
            callId.c_str(), downloadPath.c_str(), filename.c_str());

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        retcode = SQLCloseCursor(connSet->stmt);
        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("INSERT INTO TBL_JOB_INFO (CALL_ID,PATH_NM,FILE_NM,REG_DTM,STATE) VALUES ('%s','%s','%s',now(),'I')",
                callId.c_str(), downloadPath.c_str(), filename.c_str());
        }
        else {
            extract_error("DBHandler::insertTaskInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            ret = 1;
        }

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
int DBHandler::updateTaskInfo(std::string callid, std::string counselorcode, char state, const char *tbName, const char *errcode)
{
    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        //sprintf(sqlbuff, "UPDATE TBL_JOB_INFO SET STATE='%c' WHERE CALL_ID='%s' AND CS_CODE='%s'",
        //    state, callid.c_str(), counselorcode.c_str());
        if (errcode && strlen(errcode)) {
            sprintf(sqlbuff, "UPDATE %s SET STATE='%c',ERR_CD='%s' WHERE CALL_ID='%s'",
                tbName, state, errcode, callid.c_str());
        }
        else {
            sprintf(sqlbuff, "UPDATE %s SET STATE='%c' WHERE CALL_ID='%s'",
                tbName, state, callid.c_str());
        }

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        retcode = SQLCloseCursor(connSet->stmt);
        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("DBHandler::updateTaskInfo() - Query<%s>", sqlbuff);
        }
        else {
            extract_error("DBHandler::updateTaskInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            ret = 1;
        }

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("DBHandler::updateTaskInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

int DBHandler::updateTaskInfo(std::string callid, std::string counselorcode, std::string regdate, char state, const char *tbName, const char *errcode)
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
            sprintf(sqlbuff, "UPDATE %s SET STATE='%c',ERR_CD='%s' WHERE CALL_ID='%s' AND REG_DTM='%s'",
                tbName, state, errcode, callid.c_str(), regdate.c_str());
        }
        else {
            sprintf(sqlbuff, "UPDATE %s SET STATE='%c' WHERE CALL_ID='%s' AND REG_DTM='%s'",
                tbName, state, callid.c_str(), regdate.c_str());
        }


        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        retcode = SQLCloseCursor(connSet->stmt);
        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("DBHandler::updateTaskInfo2() - Query<%s>", sqlbuff);
        }
        else {
            extract_error("DBHandler::updateTaskInfo2() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            ret = 1;
        }

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
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        //sprintf(sqlbuff, "SELECT CALL_ID,CS_CODE,PATHNAME,FILE_NAME FROM TBL_JOB_INFO WHERE CALL_ID='%s' AND FILE_NAME='%s'",
        sprintf(sqlbuff, "SELECT COUNT(CALL_ID) FROM TBL_JOB_INFO WHERE CALL_ID='%s' AND FILE_NM='%s'",
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
        while (SQLFetch(connSet->stmt) == SQL_SUCCESS) 
        {
            int resCnt=0, siCnt;

            SQLGetData(connSet->stmt, 1, SQL_C_SHORT, &ret, 0, (SQLLEN *)&siCnt);
            break;
        }

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
                SQLGetData(connSet->stmt, 5, SQL_C_CHAR, rxtx, sizeof(rxtx)-1, (SQLLEN *)&siRxtx);

                JobInfoItem *item = new JobInfoItem(std::string(callid), std::to_string(counselorcode), std::string(path), std::string(filename), std::string(regdate), std::string(rxtx), std::string(tableName));
                v.push_back(item);
            }
        }
        else if (retcode < 0) {
            extract_error("DBHandler::getTaskInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
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

void DBHandler::updateAllTask2Fail()
{
    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        sprintf(sqlbuff, "UPDATE TBL_JOB_INFO SET STATE='X' WHERE REG_DTM >= concat(date(now()), ' 00:00:00') and REG_DTM <= concat(date(now()), ' 23:59:59') and STATE='U' and DATE_SUB(now(), INTERVAL 1 HOUR) > REG_DTM");

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);
        retcode = SQLCloseCursor(connSet->stmt);
        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("DBHandler::updateAllTask2Fail() - Query<%s>", sqlbuff);
        }
        else {
            extract_error("DBHandler::updateAllTask2Fail() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
        }

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
