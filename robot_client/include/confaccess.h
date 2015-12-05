#ifndef _CONF_ACCESS_H_
#define _CONF_ACCESS_H_

#include <vector>
#include <string>

class CConfAccess
{
public:
    static CConfAccess* GetConfInstance();

    bool Load(const std::string& fileName, const std::string& programName);

    bool GetValue( const char* session, const char* key, std::string& value, const char* defaultValue = "" );

    void SetProgram( const std::string& programName ) { _programName = programName; }

    std::string GetProgramName() { return _programName; }

    int GetIQLevel();

    int GetRobotIdRangeStart();

    int GetRobotIdRangeEnd();

    int GetRobotNum();

    std::string GetSessionKey();

    std::string GetGameType();

    std::string GetGameName();

    int GetMatchId();

    int GetMaxPlayerNum();

    bool GetIsMatch();

    int GetPercentage();

    std::string GetIP();

    int GetPort();

    int GetHeartBeatTime();

    int GetVerifyTime();

    int GetInitGameTime();

    int GetSendActiveMsgDelayTime();

    int GetSendPassiveMsgDelayTime();

    int GetProgramExitTime();

    int GetQueryRoomStateTime();

    std::string GetLogConfFilePath();

private:
    class CSession
    {
    public:
        CSession(){}
        ~CSession(){}

        bool operator==(const CSession& rhs) const
        {
            return _session == rhs._session;
        }

        typedef std::pair<std::string, std::string> KeyValue;

        std::string             _session;
        std::vector<KeyValue>   _attrVct;
    };


    typedef std::vector<CSession> SessionVector;
    typedef std::vector<CSession::KeyValue> KeyValueVector;

    CConfAccess(){};
    CConfAccess(const CConfAccess &);
    void operator=(const CConfAccess &);

    static CConfAccess* _pConfInstance;

    SessionVector _sessionVct;
    static std::string _programName;
};

#endif /*_CONF_ACCESS_H_*/