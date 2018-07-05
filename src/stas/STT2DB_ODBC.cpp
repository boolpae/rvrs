
#include "STT2DB_ODBC.h"

#include "ItfOdbcPool.h"
#include "stas.h"

#include <iconv.h>
#include <string.h>
#include <stdio.h>

#include <sql.h>
#include <sqlext.h>


void extract_error(const char *fn, SQLHANDLE handle, SQLSMALLINT type)
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

STT2DB* STT2DB::m_instance = nullptr;

STT2DB::STT2DB(std::string dsn,int connCount)
: m_sDsn(dsn), m_nConnCount(connCount), m_bLiveFlag(true), m_bInterDBUse(false)
{
    m_Logger = config->getLogger();
    m_pSolDBConnPool = nullptr;
    m_pInterDBConnPool = nullptr;
	m_Logger->debug("STT2DB Constructed.\n");
}

STT2DB::~STT2DB()
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

    STT2DB::m_instance = nullptr;
	m_Logger->debug("STT2DB Destructed.\n");
}

void STT2DB::thrdMain(STT2DB * s2d)
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
            sprintf(sqlbuff, "INSERT INTO JOB_DATA (idx,spk,call_id,pos_start,pos_end,value) VALUES (?,'%c',?,?,?,?)", cSpk);
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
            sprintf(sttValue, "%s", item->getSTTValue().c_str());
            // lenSpk = strlen(sSpk);
            lenCallid = strlen(callId);
            lenStt = strlen(sttValue);
            
            // printf("nIdx(%d, %d), cSpk(%s, %d), nStart(%d), nEnd(%d) callId(%s) sttValue (%s), len(%d)\n", 
            //     nIdx, lenIdx, sSpk, lenSpk, nStart, nEnd, callId, sttValue, lenStt);

            retcode = SQLExecute(connSet->stmt);//SQLExecDirect (connSet->stmt, (SQLCHAR*)sqlbuff, SQL_NTS);//
            //printf("SQLExecute RET(%d)\n", retcode);
            //retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

            if (retcode < 0) {
                extract_error("SQLExecute()", connSet->stmt, SQL_HANDLE_STMT);
                //s2d->m_Logger->error("STT2DB::insertCallInfo - SQLException -- %s", Exception_frame.message);
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
                    if (STT2DB::getInstance() && connSet)
                        s2d->m_pSolDBConnPool->restoreConnection(connSet);

                    s2d->m_bLiveFlag = true;
                    logger->error("STT2DB::thrdMain() finish! - for SQLExecute ERROR\n");
                    return;
                }
            }

            if (utf_buf) free(utf_buf);
			delete item;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
        //printf("STT2DB::thrdMain()\n");
	}

    if (STT2DB::getInstance() && connSet)
        s2d->m_pSolDBConnPool->restoreConnection(connSet);

    s2d->m_bLiveFlag = true;
    logger->debug("STT2DB::thrdMain() finish!\n");
}

STT2DB* STT2DB::instance(std::string dsn, int connCount=10)
{
    if (m_instance) return m_instance;
    
    m_instance = new STT2DB(dsn, connCount);
    
    m_instance->m_pSolDBConnPool = new ItfOdbcPool(dsn.c_str());
    
    if (m_instance->m_pSolDBConnPool->createConnections(connCount))
    {
        m_instance->m_thrd = std::thread(STT2DB::thrdMain, m_instance);
    }
    else
    {
        config->getLogger()->error("STT2DB::instance - error: cant't get connection\n");
        
        delete m_instance;
        m_instance = nullptr;
    }

    return m_instance;
}

void STT2DB::release()
{
    if (m_instance) {
        delete m_instance;
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
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        sprintf(sqlbuff, "SELECT counselor_code,center_code,status FROM CS_LIST WHERE counselor_code='%s'", counselorcode.c_str());
        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if (retcode == SQL_NO_DATA) {
            ret = 0;
	    }
	    else if (retcode == SQL_SUCCESS){
            ret = 1;
        }

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        // error
        m_Logger->error("STT2DB::searchCallInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

int STT2DB::insertCallInfo(std::string counselorcode, std::string callid)
{
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    char sqlbuff[512];
    SQLRETURN retcode;
    int ret=0;

    if (connSet)
    {
        sprintf(sqlbuff, "INSERT INTO CS_LIST (counselor_code,call_id,status,reg_dttm) VALUES ('%s','%s','I',now())",
            counselorcode.c_str(), callid.c_str());

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("STT2DB::insertCallInfo - SQL[INSERT INTO CS_LIST (counselor_code,call_id,reg_dttm) VALUES ('%s','%s',now())]", counselorcode.c_str(), callid.c_str());
        }
        else {
            extract_error("STT2DB::insertCallInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            ret = 1;
        }
        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("STT2DB::insertCallInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

int STT2DB::updateCallInfo(std::string callid, bool end)
{
    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        if (!end) {
            sprintf(sqlbuff, "UPDATE CS_LIST SET status='I' WHERE call_id='%s'",
            callid.c_str());
        }
        else {
            sprintf(sqlbuff, "UPDATE CS_LIST SET status='E' WHERE call_id='%s'",
            callid.c_str());
        }

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("UPDATE CS_LIST SET status='%c' WHERE call_id='%s'",
                (end)?'E':'I', callid.c_str());
        }
        else {
            extract_error("STT2DB::updateCallInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            ret = 1;
        }

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("STT2DB::updateCallInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

int STT2DB::updateCallInfo(std::string counselorcode, std::string callid, bool end)
{
    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        if (!end) {
            sprintf(sqlbuff, "UPDATE CS_LIST SET status='I', call_id='%s' WHERE counselor_code='%s'",
            callid.c_str(), counselorcode.c_str());
            //m_Logger->debug("STT2DB::updateCallInfo - SQL[UPDATE CS_LIST SET status='I', call_id='%s' WHERE counselor_code='%s']",callid.c_str(), counselorcode.c_str());
        }
        else {
            sprintf(sqlbuff, "UPDATE CS_LIST SET status='E', call_id='%s' WHERE counselor_code='%s'",
            callid.c_str(), counselorcode.c_str());
             //m_Logger->debug("STT2DB::updateCallInfo - SQL[UPDATE CS_LIST SET status='E', call_id='%s' WHERE counselor_code='%s']",callid.c_str(), counselorcode.c_str());
        }

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("STT2DB::updateCallInfo - SQL[UPDATE CS_LIST SET status='%c', call_id='%s' WHERE counselor_code='%s']", (end)?'E':'I', callid.c_str(), counselorcode.c_str());
        }
        else {
            extract_error("STT2DB::updateCallInfo(2) - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            ret = 1;
        }

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("STT2DB::updateCallInfo(2) - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

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
    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        sprintf(sqlbuff, "INSERT INTO JOB_INFO (call_id, pathname, filename, reg_dttm) VALUES ('%s', '%s', '%s', now())",
            callId.c_str(), downloadPath.c_str(), filename.c_str());

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("INSERT INTO JOB_INFO (call_id, pathname, filename, reg_dttm) VALUES ('%s', '%s', '%s', now())",
                callId.c_str(), downloadPath.c_str(), filename.c_str());
        }
        else {
            extract_error("STT2DB::insertTaskInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            ret = 1;
        }

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("STT2DB::insertTaskInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

// VFClient모듈에서 사용되는 api로서 해당 task 작업 종료 후 상태 값을 update할 때 사용
// args: call_id, counselor_code, task_stat etc
int STT2DB::updateTaskInfo(std::string callid, std::string counselorcode, char state)
{
    // Connection_T con;
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        sprintf(sqlbuff, "UPDATE JOB_INFO SET STATE='%c' WHERE call_id='%s' and counselor_code='%s'",
            state, callid.c_str(), counselorcode.c_str());

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if SQL_SUCCEEDED(retcode) {
            m_Logger->debug("STT2DB::updateTaskInfo() - Query<%s>", sqlbuff);
        }
        else {
            extract_error("STT2DB::updateTaskInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            ret = 1;
        }

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("STT2DB::updateTaskInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

// VFClient모듈에서 사용되는 api로서 해당 task에 대해 이전에 작업한 내용인지 아닌지 확인하기 위해 사용
// args: call_id, counselor_code etc
int STT2DB::searchTaskInfo(std::string downloadPath, std::string filename, std::string callId)
{
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;

    if (connSet)
    {
        sprintf(sqlbuff, "SELECT call_id,counselor_code,pathname,filename FROM JOB_INFO WHERE call_id = '%s' and filename = '%s'",
            callId.c_str(), filename.c_str());

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if (retcode == SQL_NO_DATA) {
            ret = 0;
	    }
	    else if (retcode == SQL_SUCCESS){
            ret = 1;
        }

        m_pSolDBConnPool->restoreConnection(connSet);
    }
    else
    {
        m_Logger->error("STT2DB::searchTaskInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

int STT2DB::getTaskInfo(std::vector< JobInfoItem* > &v) 
{
    PConnSet connSet = m_pSolDBConnPool->getConnection();
    int ret=0;
    char sqlbuff[512];
    SQLRETURN retcode;
    
    char callid[256];
    int counselorcode;
    char path[500];
    char filename[256];
    int siCallId, siCCode, siPath, siFilename;

    // m_Logger->debug("BEFORE STT2DB::getTaskInfo - ConnectionPool_size(%d), ConnectionPool_active(%d)", ConnectionPool_size(m_pool), ConnectionPool_active(m_pool));
    if (connSet)
    {
        sprintf(sqlbuff, "SELECT call_id,counselor_code,pathname,filename FROM JOB_INFO WHERE state = 'N' or state = 'X'");

        retcode = SQLExecDirect(connSet->stmt, (SQLCHAR *)sqlbuff, SQL_NTS);

        if (retcode == SQL_SUCCESS) {
            while (SQLFetch(connSet->stmt) == SQL_SUCCESS) 
            {
                memset(callid, 0, sizeof(callid));
                memset(path, 0, sizeof(path));
                memset(filename, 0, sizeof(filename));

                SQLGetData(connSet->stmt, 1, SQL_C_CHAR, callid, 255, (SQLLEN *)&siCallId);
                SQLGetData(connSet->stmt, 2, SQL_C_SLONG, &counselorcode, 0, (SQLLEN *)&siCCode);
                SQLGetData(connSet->stmt, 3, SQL_C_CHAR, path, 500, (SQLLEN *)&siPath);
                SQLGetData(connSet->stmt, 4, SQL_C_CHAR, filename, 255, (SQLLEN *)&siFilename);

                JobInfoItem *item = new JobInfoItem(std::string(callid), std::to_string(counselorcode), std::string(path), std::string(filename));
                v.push_back(item);
            }
        }
        else if (retcode < 0) {
            extract_error("STT2DB::getTaskInfo() - SQLExecDirect()", connSet->stmt, SQL_HANDLE_STMT);
            ret = retcode;
        }

        m_pSolDBConnPool->restoreConnection(connSet);

        ret = v.size();
    }
    else
    {
        m_Logger->error("STT2DB::getTaskInfo - can't get connection from pool");
        ret = -1;
    }

    return ret;
}

void STT2DB::setInterDBEnable(std::string dsn, int connCount=10)
{
    if (m_bInterDBUse) return;
    
    m_pInterDBConnPool = new ItfOdbcPool(dsn.c_str());
    
    if (m_pInterDBConnPool->createConnections(connCount))
    {
        m_bInterDBUse = true;
    }
    else
    {
        m_Logger->error("STT2DB::setInterDBEnable - failed to create InterDB ConnectionPool");
        
        delete m_pInterDBConnPool;
        
    }

}

void STT2DB::setInterDBDisable()
{
    if (m_bInterDBUse) {
        delete m_pInterDBConnPool;
        m_pInterDBConnPool = nullptr;
        m_bInterDBUse = false;
    }
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
