#ifndef _LOG_H_
#define _LOG_H_

#include <string>
#ifndef MAX_LINE
#define MAX_LINE 1024
#endif
#define DEBUG(format, ...) mylog4c::DEBUG_LOG((__FILE__), (__LINE__), format, ## __VA_ARGS__);
#define INFO(format, ...) mylog4c::INFO_LOG((__FILE__), (__LINE__), format, ## __VA_ARGS__);
#define ERROR(format, ...) mylog4c::ERROR_LOG((__FILE__), (__LINE__), format, ## __VA_ARGS__);
#define WARN(format, ...) mylog4c::WARN_LOG((__FILE__), (__LINE__), format, ## __VA_ARGS__);

namespace mylog4c
{
    void DEBUG_LOG(const std::string& strFile, int iLine, const char* format, ...);
    void INFO_LOG(const std::string& strFile, int iLine, const char* format, ...);
    void ERROR_LOG(const std::string& strFile, int iLine, const char* format, ...);
    void WARN_LOG(const std::string& strFile, int iLine, const char* format, ...);
}

class CLog
{
public:
    static CLog* Initialize(const std::string& strLogFile);

private:
    CLog(const std::string& strLogFile);
    static CLog* pInstance;
};

#endif

