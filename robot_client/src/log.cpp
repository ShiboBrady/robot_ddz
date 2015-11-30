#include "log.h"
#include <sstream>
#include <memory>
#include <stdarg.h>
#include <log4cplus/logger.h>
#include <log4cplus/loggingmacros.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/configurator.h>
#include <log4cplus/configurator.h>

using namespace std;
using namespace log4cplus;
using namespace log;

static Logger global_pLogger;

CLog* CLog::pInstance = NULL;

CLog* CLog::Initialize(const string& strLogFile)
{
    if (NULL == pInstance)
    {
        pInstance = new CLog(strLogFile);
    }
    return pInstance;
}

CLog::CLog(const string& strLogFile)
{
    PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(strLogFile));
    global_pLogger = Logger::getRoot();
}

/*
CLog::CLog(const string& strLogFile)
{
    //PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT("/data/log/log4cplus.properties"));

    //Logger global_pLogger = Logger::getRoot();

    //定义一个文件Appender
    SharedAppenderPtr oAppender(new FileAppender(LOG4CPLUS_TEXT(strLogFile)));

    //Layout对象
    string strPattern = "[%D{%Y-%m-%d %H:%M:%S}]%m%n";
    auto_ptr<Layout> pLayout(new PatternLayout(strPattern));

    //将Layout绑定到appender对象
    oAppender->setLayout(pLayout);

    //Logger对象
    global_pLogger = Logger::getInstance(LOG4CPLUS_TEXT("LoggerName"));

    //将需要appender对象绑定到到Logger对象上
    global_pLogger.addAppender(oAppender);
}
*/

void log::DEBUG_LOG(const std::string& strFile, int iLine, const char* format, ...)
{
    char logInfo[MAX_LINE];
    va_list st;
    va_start(st, format);
    vsnprintf(logInfo, MAX_LINE, format, st);
    stringstream ssLogData;
    ssLogData << "[" << strFile << ":" << iLine << "] " << logInfo;
    LOG4CPLUS_DEBUG(global_pLogger, ssLogData.str());
    va_end(st);
}

void log::ERROR_LOG(const std::string& strFile, int iLine, const char* format, ...)
{
    char logInfo[MAX_LINE];
    va_list st;
    va_start(st, format);
    vsnprintf(logInfo, MAX_LINE, format, st);
    stringstream ssLogData;
    ssLogData << "[" << strFile << ":" << iLine << "] " << logInfo;
    LOG4CPLUS_ERROR(global_pLogger, ssLogData.str());
    va_end(st);
}

void log::WARN_LOG(const std::string& strFile, int iLine, const char* format, ...)
{
    char logInfo[MAX_LINE];
    va_list st;
    va_start(st, format);
    vsnprintf(logInfo, MAX_LINE, format, st);
    stringstream ssLogData;
    ssLogData << "[" << strFile << ":" << iLine << "] " << logInfo;
    LOG4CPLUS_WARN(global_pLogger, ssLogData.str());
    va_end(st);
}

