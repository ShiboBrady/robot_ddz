#ifndef _CONF_ACCESS_H_
#define _CONF_ACCESS_H_
#include <string>

class CConfAccess
{
public:
    static CConfAccess* GetConfInstance();

    bool Load(const std::string& fileName, const std::string& programName);

    int GetIQLevel() {return iRobotIQLevel_; }

    int GetRobotIdRangeStart() {return iRobotIdStart_; }

    int GetRobotIdRangeEnd() {return iRobotIdEnd_; }

    int GetRobotNum() {return iRobotNum_; }

    const std::string& GetSessionKey() {return strSessionKey_; }

    const std::string& GetGameType() {return strType_; }

    const std::string& GetGameName() {return strName_; }

    int GetMatchId() {return iMatchId_; }

    int GetMinPlayNumNeekCheck() {return iMinPlayerNum_; }

    int GetMaxPlayerNum() {return iPlayerNum_; }

    bool GetIsMatch() {return bIsMatch_; }

    bool GetIsNeedKeepPlay() {return bIsCheckKeepPlay_; }

    const std::string& GetIP() {return strIp_; }

    int GetPort() {return iPort_; }

    int GetHeartBeatTime() {return iHeartBeatTime_; }

    int GetSendActiveMsgDelayTime() {return iActiveMsgDelay_; }

    int GetSendPassiveMsgDelayTime() {return iPassiveMsgDelay_; }

    int GetQueryRoomStateTime() {return iRoomState_; }

    const std::string& GetLogConfFilePath() {return strLogConf_; }

private:
    CConfAccess(){};
    CConfAccess(const CConfAccess &);
    void operator=(const CConfAccess &);

    std::string strIp_;
    int iPort_;
    int iRobotIQLevel_;
    int iRobotIdStart_;
    int iRobotIdEnd_;
    int iRobotNum_;
    bool bIsMatch_;
    bool bIsCheckKeepPlay_;
    std::string strRoomName_;
    std::string strType_;
    std::string strSessionKey_;
    int iMatchId_;
    std::string strName_;
    int iMinPlayerNum_;
    int iPlayerNum_;
    std::string strLogConf_;
    int iHeartBeatTime_;
    int iRoomState_;
    int iActiveMsgDelay_;
    int iPassiveMsgDelay_;
    
    static CConfAccess* _pConfInstance;
    static std::string _programName;
};

#endif /*_CONF_ACCESS_H_*/