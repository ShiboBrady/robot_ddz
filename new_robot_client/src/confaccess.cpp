#include "confaccess.h"
#include <iostream>
#include <algorithm>
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <cstdio>
using namespace std;
using namespace rapidjson;
#define MAX_LINE    65536
#define psln(x) std::cout << #x << " = " << (x) << std::endl
#define pnoitem(x) std::cout << " Doesn't has value: " << (x) << std::endl
#define pformaterr(x) std::cout << #x << " format error." << std::endl;

CConfAccess* CConfAccess::_pConfInstance = NULL;

std::string CConfAccess::_programName;

CConfAccess* CConfAccess::GetConfInstance() {
    if (NULL != CConfAccess::_pConfInstance)
        return _pConfInstance;

    _pConfInstance = new CConfAccess();
    return _pConfInstance;
}

bool CConfAccess::Load(const std::string& fileName, const std::string& programName) {
    FILE* fp = fopen(fileName.c_str(), "r");
    char readBuffer[MAX_LINE];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    Document document;
    document.ParseStream(is);
    if (document.HasParseError()) {
        rapidjson::ParseErrorCode code = document.GetParseError();
        psln(code);
        return false;
    }
    _programName = programName;
    Value::ConstMemberIterator itrRoot = document.FindMember(_programName.c_str());
    if (itrRoot != document.MemberEnd()) {
        Value::ConstMemberIterator itrServer = itrRoot->value.FindMember("server");
        if (itrServer != itrRoot->value.MemberEnd()) {
            Value::ConstMemberIterator itrItem = itrServer->value.FindMember("ip");
            if (itrItem != itrServer->value.MemberEnd()) {
                if (itrItem->value.IsString()) {
                    strIp_ = itrItem->value.GetString();
                    psln(strIp_);
                }  else {
                    pformaterr(strIp_);
                }
            } else {
                pnoitem("ip");
                return false;
            }
            itrItem = itrServer->value.FindMember("port");
            if (itrItem != itrServer->value.MemberEnd()) {
                if (itrItem->value.IsInt()) {
                    iPort_ = itrItem->value.GetInt();
                    psln(iPort_);
                } else {
                    pformaterr(iPort_);
                }
            } else { 
                pnoitem("port");
                return false;
            }
        }
        Value::ConstMemberIterator itrRobot = itrRoot->value.FindMember("robot");
        if (itrRobot != itrRoot->value.MemberEnd()) {
            Value::ConstMemberIterator itrItem = itrRobot->value.FindMember("IQLevel");
            if (itrItem != itrRobot->value.MemberEnd()) {
                if (itrItem->value.IsInt()) {
                    iRobotIQLevel_ =  itrItem->value.GetInt();
                    psln(iRobotIQLevel_);
                }  else {
                    pformaterr(iRobotIQLevel_);
                }
            } else { 
                pnoitem("IQLevel");
                return false;
            }
            itrItem = itrRobot->value.FindMember("robotIdStart");
            if (itrItem != itrRobot->value.MemberEnd()) {
                if (itrItem->value.IsInt()) {
                    iRobotIdStart_ = itrItem->value.GetInt();
                    psln(iRobotIdStart_);
                } else {
                    pformaterr(iRobotIdStart_);
                }
            } else { 
                pnoitem("robotIdStart");
                return false;
            }
            itrItem = itrRobot->value.FindMember("robotIdEnd");
            if (itrItem != itrRobot->value.MemberEnd()) {
                if (itrItem->value.IsInt()) {
                    iRobotIdEnd_ = itrItem->value.GetInt();
                    psln(iRobotIdEnd_);
                } else {
                    pformaterr(iRobotIdEnd_);
                }
            } else { 
                pnoitem("robotIdEnd");
                return false;
            }
            itrItem = itrRobot->value.FindMember("robotNum");
            if (itrItem != itrRobot->value.MemberEnd()) {
                if (itrItem->value.IsInt()) {
                    iRobotNum_ = itrItem->value.GetInt();
                    psln(iRobotNum_);
                } else {
                    pformaterr(iRobotNum_);
                }
            } else { 
                pnoitem("robotNum");
                return false;
            }
        }
        Value::ConstMemberIterator itrGame = itrRoot->value.FindMember("game");
        if (itrGame != itrRoot->value.MemberEnd()) {
            Value::ConstMemberIterator itrItem = itrGame->value.FindMember("room_name");
            if (itrItem != itrGame->value.MemberEnd()) {
                if (itrItem->value.IsString()) {
                    strRoomName_ = itrItem->value.GetString();
                    psln(strRoomName_);
                } else {
                    pformaterr(strRoomName_);
                }
            } else { 
                pnoitem("room_name");
                return false;
            }
            itrItem = itrGame->value.FindMember("type");
            if (itrItem != itrGame->value.MemberEnd()) {
                if (itrItem->value.IsString()) {
                    strType_ = itrItem->value.GetString();
                    psln(strType_);
                } else {
                    pformaterr(strType_);
                }
            } else { 
                pnoitem("type");
                return false;
            }
            itrItem = itrGame->value.FindMember("sessionKey");
            if (itrItem != itrGame->value.MemberEnd()) {
                if (itrItem->value.IsString()) {
                    strSessionKey_ = itrItem->value.GetString();
                    psln(strSessionKey_);
                } else {
                    pformaterr(strSessionKey_);
                }
            } else { 
                pnoitem("sessionKey");
                return false;
            }
            itrItem = itrGame->value.FindMember("matchid");
            if (itrItem != itrGame->value.MemberEnd()) {
                if (itrItem->value.IsInt()) {
                    iMatchId_ = itrItem->value.GetInt();
                    psln(iMatchId_);
                } else {
                    pformaterr(iMatchId_);
                }
            } else { 
                pnoitem("matchid");
                return false;
            }
            itrItem = itrGame->value.FindMember("name");
            if (itrItem != itrGame->value.MemberEnd()) {
                if (itrItem->value.IsString()) {
                    strName_ = itrItem->value.GetString();
                    psln(strName_);
                } else {
                    pformaterr(strName_);
                }
            } else { 
                pnoitem("name");
                return false;
            }
            itrItem = itrGame->value.FindMember("minPlayerNum");
            if (itrItem != itrGame->value.MemberEnd()) {
                if (itrItem->value.IsInt()) {
                    iMinPlayerNum_ = itrItem->value.GetInt();
                    psln(iMinPlayerNum_);
                } else {
                    pformaterr(iMinPlayerNum_);
                }
            } else { 
                pnoitem("minPlayerNum");
                return false;
            }
            itrItem = itrGame->value.FindMember("playerNum");
            if (itrItem != itrGame->value.MemberEnd()) {
                if (itrItem->value.IsInt()) {
                    iPlayerNum_ = itrItem->value.GetInt();
                    psln(iPlayerNum_);
                } else {
                    pformaterr(iPlayerNum_);
                }
            } else { 
                pnoitem("playerNum");
                return false;
            }
            itrItem = itrGame->value.FindMember("logConf");
            if (itrItem != itrGame->value.MemberEnd()) {
                if (itrItem->value.IsString()) {
                    strLogConf_ = itrItem->value.GetString();
                    psln(strLogConf_);
                } else {
                    pformaterr(strLogConf_);
                }
            } else { 
                pnoitem("logConf");
                return false;
            }
        }
        Value::ConstMemberIterator itrSwitch = itrRoot->value.FindMember("switch");
        if (itrSwitch != itrRoot->value.MemberEnd()) {
            Value::ConstMemberIterator itrItem = itrSwitch->value.FindMember("isMatch");
            if (itrItem != itrSwitch->value.MemberEnd()) {
                if (itrItem->value.IsBool()) {
                    bIsMatch_ = itrItem->value.GetBool();
                    psln(bIsMatch_);
                } else {
                    pformaterr(bIsMatch_);
                }
            } else { 
                pnoitem("isMatch");
                return false;
            }
            itrItem = itrSwitch->value.FindMember("isCheckKeepPlay");
            if (itrItem != itrSwitch->value.MemberEnd()) {
                if (itrItem->value.IsBool()) {
                    bIsCheckKeepPlay_ = itrItem->value.GetBool();
                    psln(bIsCheckKeepPlay_);
                } else {
                    pformaterr(bIsCheckKeepPlay_);
                }
            } else { 
                pnoitem("isCheckKeepPlay");
                return false;
            }
        }
        Value::ConstMemberIterator itrTimer = itrRoot->value.FindMember("timer");
        if (itrTimer != itrRoot->value.MemberEnd()) {
            Value::ConstMemberIterator itrItem = itrTimer->value.FindMember("heartBeat");
            if (itrItem != itrTimer->value.MemberEnd()) {
                if (itrItem->value.IsInt()) {
                    iHeartBeatTime_ = itrItem->value.GetInt();
                    psln(iHeartBeatTime_);
                } else {
                    pformaterr(iHeartBeatTime_);
                }
            } else { 
                pnoitem("heartBeat");
                return false;
            }
            itrItem = itrTimer->value.FindMember("activeMsgDelay");
            if (itrItem != itrTimer->value.MemberEnd()) {
                if (itrItem->value.IsInt()) {
                    iActiveMsgDelay_ = itrItem->value.GetInt();
                    psln(iActiveMsgDelay_);
                } else {
                    pformaterr(iActiveMsgDelay_);
                }
            } else { 
                pnoitem("activeMsgDelay");
                return false;
            }
            itrItem = itrTimer->value.FindMember("passiveMsgDelay");
            if (itrItem != itrTimer->value.MemberEnd()) {
                if (itrItem->value.IsInt()) {
                    iPassiveMsgDelay_ = itrItem->value.GetInt();
                    psln(iPassiveMsgDelay_);
                } else {
                    pformaterr(iPassiveMsgDelay_);
                }
            } else { 
                pnoitem("passiveMsgDelay");
                return false;
            }
            itrItem = itrTimer->value.FindMember("roomstate");
            if (itrItem != itrTimer->value.MemberEnd()) {
                if (itrItem->value.IsInt()) {
                    iRoomState_ = itrItem->value.GetInt();
                    psln(iRoomState_);
                } else {
                    pformaterr(iRoomState_);
                }
            } else { 
                pnoitem("roomstate");
                return false;
            }
        }
    } else {
        return false;
    }
    return true;
}
