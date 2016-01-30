#include <fstream>
#include <algorithm>
#include "stringutil.h"
#include "confaccess.h"
#include <iostream>
using namespace std;
#define MAX_LINE    1024

CConfAccess* CConfAccess::_pConfInstance = NULL;

std::string CConfAccess::_programName;

CConfAccess* CConfAccess::GetConfInstance()
{
    if (NULL != CConfAccess::_pConfInstance)
        return _pConfInstance;

    _pConfInstance = new CConfAccess();
}

bool CConfAccess::Load(const std::string& fileName, const std::string& programName)
{
    if (0 == programName.length())
    {
        return false;
    }
    _programName = programName;
    cout << "ProgramName is " << _programName << endl;

    std::ifstream initFile(fileName.c_str());

    if ( !initFile )
    {
        return false;
    }

    char line[MAX_LINE] = {0};
    std::string fullLine, command;

    while (initFile.getline(line, MAX_LINE))
    {
        fullLine = line;

        /* if the line contains a # then it is a comment
            if we find it anywhere other than the beginning, then we assume
            there is a command on that line, and it we don't find it at all
            we assume there is a command on the line (we test for valid
            command later) if neither is true, we continue with the next line
        */
        std::string::size_type length = fullLine.find('#');
        if (length > 0)
        {
            command = fullLine.substr(0, length);
        }
        else
        {
            continue;
        }

        std::string::size_type leftPos = 0, rightPos = 0;
        // check the command and handle it
        if ( std::string::npos != (length = command.find('=')) )
        {
            if ( _sessionVct.empty() )
            {
                //no session head
                return false;
            }
            CSession::KeyValue keyValue;
            keyValue.first = StringUtil::Trim(command.substr(0, length));
            keyValue.second = StringUtil::Trim(command.substr(length + 1, command.size() - length));
            _sessionVct.back()._attrVct.push_back( keyValue );
            cout << "Add key [" << keyValue.first << "] for session [" << _sessionVct.back()._session << "], value is: ["\
                << keyValue.second << "]" << endl;
        }
        else if ( std::string::npos != (leftPos = command.find('['))
            && std::string::npos != (rightPos = command.find(']'))
            && leftPos < rightPos )
        {
             CSession ss ;
             ss._session = StringUtil::Trim( command.substr(leftPos + 1, rightPos - leftPos - 1) );
             if ( _sessionVct.end() != std::find(_sessionVct.begin(), _sessionVct.end(), ss) )
             {
                 //session exist
                 continue;
             }
             else
             {
                 //push back session object
                 _sessionVct.push_back( ss );
                 cout << "Add session: "  << ss._session << endl;
             }
        }
        else
        {
            continue;
        }
    }
    return true;
}

bool CConfAccess::GetValue( const char* session, const char* key, std::string& value, const char* defaultValue )
{
    if ( !session || !key )
    {
        return false;
    }
    bool result = false;
    value = defaultValue;

    CSession ss;
    ss._session = session;
    SessionVector::const_iterator iterSession = std::find( _sessionVct.begin(), _sessionVct.end(), ss );
    if ( _sessionVct.end() != iterSession )
    {
        const KeyValueVector &attrVct = iterSession->_attrVct;

        for ( KeyValueVector::const_iterator iter = attrVct.begin(); iter != attrVct.end(); ++iter )
        {
            if ( iter->first == key )
            {
                result = true;
                value = iter->second;
                break;
            }
        }

    }
    else
    {
        cout << "Doesn\'t find key [" << key << "] in session: [" << session << "]" << endl;
    }
    return result;
}

int CConfAccess::GetIQLevel()
{
    std::string strRobotIQLevel;
    GetValue(_programName.c_str(), "IQLevel", strRobotIQLevel, "0");
    return ::atoi(strRobotIQLevel.c_str());
}

int CConfAccess::GetRobotIdRangeStart()
{
    std::string strRobotIdStart;
    GetValue(_programName.c_str(), "robotIdStart", strRobotIdStart, "110001");
    return ::atoi(strRobotIdStart.c_str());
}

int CConfAccess::GetRobotIdRangeEnd()
{
    std::string strRobotIdEnd;
    GetValue(_programName.c_str(), "robotIdEnd", strRobotIdEnd, "110001");
    return ::atoi(strRobotIdEnd.c_str());
}

int CConfAccess::GetRobotNum()
{
    std::string strRobotNum;
    GetValue(_programName.c_str(), "robotNum", strRobotNum, "1");
    return ::atoi(strRobotNum.c_str());
}

std::string CConfAccess::GetSessionKey()
{
    std::string strSessionKey;
    GetValue(_programName.c_str(), "sessionKey", strSessionKey, "session_");
    return StringUtil::Trim(strSessionKey);
}

std::string CConfAccess::GetGameType()
{
    std::string strGameType;
    GetValue(_programName.c_str(), "type", strGameType, "ddz");
    return StringUtil::Trim(strGameType);
}

std::string CConfAccess::GetGameName()
{
    std::string strGameName;
    GetValue(_programName.c_str(), "name", strGameName, "org_ddz_match_mf_01");
    return StringUtil::Trim(strGameName);
}

int CConfAccess::GetMatchId()
{
    std::string strMatchId;
    GetValue(_programName.c_str(), "matchid", strMatchId, "1000");
    return ::atoi(strMatchId.c_str());
}

int CConfAccess::GetMaxPlayerNum()
{
    std::string strMaxPlayerNum;
    GetValue(_programName.c_str(), "playerNum", strMaxPlayerNum, "3");
    return ::atoi(strMaxPlayerNum.c_str());
}

bool CConfAccess::GetIsMatch()
{
    std::string strIsMatch;
    GetValue(_programName.c_str(), "isMatch", strIsMatch, "0");
    int iIsMatch = ::atoi(strIsMatch.c_str());
    return iIsMatch > 0 ? true : false;
}

bool CConfAccess::GetIsTimeTrial()
{
    std::string strIsTimeTrial;
    GetValue(_programName.c_str(), "isTimeTrial", strIsTimeTrial, "0");
    int iIsTimeTrial = ::atoi(strIsTimeTrial.c_str());
    return iIsTimeTrial > 0 ? true : false;
}

int CConfAccess::GetLeftTimeForTimeTrial()
{
    std::string strLeftTime;
    GetValue(_programName.c_str(), "timeLeft", strLeftTime, "300");
    return ::atoi(strLeftTime.c_str());
}

int CConfAccess::GetPercentage()
{
    std::string strPercentage;
    GetValue(_programName.c_str(), "percentage", strPercentage, "100");
    return ::atoi(strPercentage.c_str());
}

std::string CConfAccess::GetIP()
{
    std::string strIp;
    GetValue(_programName.c_str(), "ip", strIp, "192.168.1.88");
    return StringUtil::Trim(strIp);
}

int CConfAccess::GetPort()
{
    std::string strPort;
    GetValue(_programName.c_str(), "port", strPort, "4001");
    return ::atoi(strPort.c_str());
}

int CConfAccess::GetHeartBeatTime()
{
    std::string strHeartBeatTime;
    GetValue(_programName.c_str(), "heartBeat", strHeartBeatTime, "30");
    return ::atoi(strHeartBeatTime.c_str());
}

int CConfAccess::GetVerifyTime()
{
    std::string strVerifyTime;
    GetValue(_programName.c_str(), "verify", strVerifyTime, "3");
    return ::atoi(strVerifyTime.c_str());
}

int CConfAccess::GetInitGameTime()
{
    std::string strInitGameTime;
    GetValue(_programName.c_str(), "initGame", strInitGameTime, "5");
    return ::atoi(strInitGameTime.c_str());
}

int CConfAccess::GetSendActiveMsgDelayTime()
{
    std::string strSendActiveMsgDelayTime;
    GetValue(_programName.c_str(), "activeMsgDelay", strSendActiveMsgDelayTime, "5");
    return ::atoi(strSendActiveMsgDelayTime.c_str());
}

int CConfAccess::GetSendPassiveMsgDelayTime()
{
    std::string strSendPassiveMsgDelayTime;
    GetValue(_programName.c_str(), "passiveMsgDelay", strSendPassiveMsgDelayTime, "2");
    return ::atoi(strSendPassiveMsgDelayTime.c_str());
}

int CConfAccess::GetProgramExitTime()
{
    std::string strExitTime;
    GetValue(_programName.c_str(), "exit", strExitTime, "1");
    return ::atoi(strExitTime.c_str());
}

int CConfAccess::GetQueryRoomStateTime()
{
    std::string strQueryRoomStateTime;
    GetValue(_programName.c_str(), "roomstate", strQueryRoomStateTime, "10");
    return ::atoi(strQueryRoomStateTime.c_str());
}

std::string CConfAccess::GetLogConfFilePath()
{
    std::string strLogconfFilePath;
    cout << "programe name length: " << _programName.length() << endl;
    bool ret = GetValue(_programName.c_str(), "logConf", strLogconfFilePath, "../configure/log4cplus.properties");
    if (!ret)
    {
        cout << "Get value failed by programName: [" << _programName.c_str() << "] for key: logConf." << endl;
        cout << "_programName is: " << _programName << endl;
    }
    return StringUtil::Trim(strLogconfFilePath);
}


