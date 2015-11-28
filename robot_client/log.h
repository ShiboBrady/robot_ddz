#ifndef _LOG_H_
#define _LOG_H_

#include <string>
#ifndef MAX_LINE
#define MAX_LINE 1024
#endif
#define DEBUG(format, ...) log::DEBUG_LOG((__FILE__), (__LINE__), format, ## __VA_ARGS__);
#define ERROR(format, ...) log::ERROR_LOG((__FILE__), (__LINE__), format, ## __VA_ARGS__);
#define WARN(format, ...) log::WARN_LOG((__FILE__), (__LINE__), format, ## __VA_ARGS__);

namespace log
{

void DEBUG_LOG(const std::string& strFile, int iLine, const char* format, ...);
void ERROR_LOG(const std::string& strFile, int iLine, const char* format, ...);
void WARN_LOG(const std::string& strFile, int iLine, const char* format, ...);

class CLog
{
public:
    static CLog* Initialize(const std::string& strLogFile);

private:
    CLog(const std::string& strLogFile);
    static CLog* pInstance;
};

}
#endif

