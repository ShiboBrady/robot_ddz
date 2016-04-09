#include "RobotHeader.h"
#include <iostream>
#include "log.h"
#include "TcpEventServer.h"
#include "OGLordRobotAI.h"
#include "AIUtils.h"
#include "RobotConfig.h"
#include "stringutil.h"
#include "MsgPackage.h"
#include "Connection.h"

#include "PBGameDDZ.pb.h"
#include "connect.pb.h"
#include "org_room2client.pb.h"
#include "message.pb.h"

using namespace std;
using namespace robot;
using namespace PBGameDDZ;
using namespace YLYQ;
using namespace Protocol;
using namespace connect;
using namespace message;
using namespace org_room2client;
using namespace AIUtils;

void QueryRoomStatusTimer(int fd, short events, void* arg);

RobotHeader::RobotHeader(int iRobotId)
    :RobotBase(iRobotId),
     QueryRoomStateTimer_(NULL) {
    AddMessageProcessFunc(robot::MSGID_DDZ_ROOM_NEED_ROBOT_ACK, std::bind(&RobotHeader::RecvQueryRoomStatusAck, this, placeholders::_1));
    AddMessageProcessFunc(robot::MSGID_DDZ_SIGN_UP_CONDITION_ACK, std::bind(&RobotHeader::RecvQuerySignUpCondAck, this, placeholders::_1));

    roomStateTime_ = confAccess_->GetQueryRoomStateTime();
    DEBUG("Query room state time is %d.", roomStateTime_);
}

RobotHeader::~RobotHeader() {
    delete QueryRoomStateTimer_;
}

bool RobotHeader::AnalysisAndDecision(std::shared_ptr<MsgNode>& msgNode) {
    DEBUG("Header robot analysis msg.");
    if (msgNode->GetMsgNeedSend()) { //是否是要发送消息
        SendMsg(msgNode);
        DEBUG("Header robot send a request.");
    } else if (msgNode->GetNeedRobotNum()) { //需要加派机器人
        addRobotCallback_(msgNode->GetNeedRobotNum());
        DEBUG("Header robot add %d robot.", msgNode->GetNeedRobotNum());
    } else {
        DEBUG("Header robot doesn\'t have action.");
    }
    return true;
}

inline bool RobotHeader::NextActionProcess( std::shared_ptr<MsgNode>& msgNode ) {
    BeginToQueryRoomStatus(msgNode); //开启定时查询房间状态定时器
    return true;
}

inline void RobotHeader::RobotInit() {
    status_ = INIT;
    QueryRoomStateTimer_ = NULL;
    needKeepPlay_ = false;
}

void RobotHeader::DeleteTimer() {
    if (NULL == QueryRoomStateTimer_) {
        return;
    }
    delTimerFunc_(QueryRoomStateTimer_);
    delete QueryRoomStateTimer_;
    QueryRoomStateTimer_ = NULL;
}

bool RobotHeader::HeaderRobotAction(std::shared_ptr<MsgNode>& msgNode) {
    return isTimeTrail_ ? SendQuerySignUpCondReq(msgNode) : SendQueryRoomStatusReq(msgNode);
}

bool RobotHeader::BeginToQueryRoomStatus(std::shared_ptr<MsgNode>& msgNode) {
    QueryRoomStateTimer_ = new MsgNode;
    QueryRoomStateTimer_->SetTimer(roomStateTime_, 0);
    QueryRoomStateTimer_->SetObjectPoint(this);
    QueryRoomStateTimer_->SetConn(msgNode->GetConn());
    timerFunc_(QueryRoomStatusTimer, QueryRoomStateTimer_, false);
    DEBUG("Have been set query room status timer, time interval is %d.", roomStateTime_);
    return true;
}

bool RobotHeader::SendQueryRoomStatusReq(std::shared_ptr<MsgNode>& msgNode) { //发送查询房间状态消息
    INFO("=================== SendQueryRoomStatusReq START =================");
    // OrgRoomDdzRoomStatReq orgRoomDdzRoomStatReq;
    // orgRoomDdzRoomStatReq.add_roomids(matchId_);
    // if (!SerializeSendMsg(&orgRoomDdzRoomStatReq, robot::MSGID_DDZ_ROOM_STAT_REQ, msgNode)) {
    //     ERROR("Robot %d send Query room status string serialize failed.", robotId_);
    //     return false;
    // }
    // INFO("Begin to query room %d status.", matchId_);
    // SendMsg(msgNode);
    OrgRoomDdzNeedRobotReq orgRoomDdzNeedRobotReq;
    orgRoomDdzNeedRobotReq.set_roomid(matchId_);
    if (!SerializeSendMsg(&orgRoomDdzNeedRobotReq, robot::MSGID_DDZ_ROOM_NEED_ROBOT_REQ, msgNode)) {
        ERROR("Robot %d send Query room status string serialize failed.", robotId_);
        return false;
    }
    INFO("Begin to query room %d status.", matchId_);
    SendMsg(msgNode);
    INFO("=================== SendQueryRoomStatusReq END =================");
    return true;
}

bool RobotHeader::RecvQueryRoomStatusAck(std::shared_ptr<MsgNode>& msgNode) {
    //message OrgRoomDdzRoomStatAck {
    //    required int32 result = 1;
    //    message RoomStat {
    //        required int32 roomId = 1;  // 比赛/比赛 ID
    //        required int32 userCount = 2;// 人数
    //    }
    //    repeated RoomStat stat = 2;
    //}
    INFO("=================== RecvQueryRoomStatusAck START =================");
    INFO("Message for robot %d.", robotId_);
    // OrgRoomDdzRoomStatAck orgRoomDdzRoomStatAck;
    // if (!DeserializePbMsg(&orgRoomDdzRoomStatAck, msgNode->GetMsg())) {
    //     ERROR("parse query room status ack pb message error.");
    //     return false;
    // }
    // int result = orgRoomDdzRoomStatAck.result();
    // if (0 != result) {
    //     ERROR("Request query room failed, result is: %d.", result);
    //     return false;
    // }
    // int roomId = orgRoomDdzRoomStatAck.stat(0).roomid();
    // int userCount = orgRoomDdzRoomStatAck.stat(0).usercount();
    // int minPlayerNum = confAccess_->GetMinPlayNumNeekCheck();
    // int maxPlayerNum = confAccess_->GetMaxPlayerNum();
    // if (roomId != matchId_) {
    //     ERROR("Room id %d is not mater with matchId %d", roomId, matchId_);
    //     return true;
    // }

    // if (userCount < minPlayerNum) {
    //     INFO("Current room has %d User, lowest player check num is: %d.", userCount, minPlayerNum);
    //     return true;
    // }

    // int needRobotNum = 0;
    // if (isMatch_) { //比赛场
    //     if (userCount >= maxPlayerNum) {
    //         INFO("There is enough user signed up, no need robot.");
    //         return true;
    //     }

    //     if (isTimeTrail_) { //定时赛
    //         needRobotNum = maxPlayerNum - userCount;
    //     } else { //非定时赛
    //         int percentage = confAccess_->GetPercentage();
    //         int need = maxPlayerNum - userCount;
    //         needRobotNum =  (100 >= percentage ? need : int(need * (percentage / 100.00) + 0.5)) ; //需要按照百分比选择机器人数
    //         if (1 == need && 0 == needRobotNum) {
    //             needRobotNum = 1;
    //         }
    //     }
    //     INFO("Match room, Limit player num is: %d, current room user is: %d, need enter robot num is: %d.", \
    //         maxPlayerNum, userCount, needRobotNum);
    // } else { //游戏场
    //     if (0 == minPlayerNum) {
    //         needRobotNum = maxPlayerNum; //当最低限制人数为0时，就加派指定个数的机器人
    //     } else {
    //         int remainder = userCount % 3;
    //         switch (remainder) {
    //             case 1: needRobotNum = 2; break;
    //             case 2: needRobotNum = 1; break;
    //             default: return true;
    //         }
    //     }
    //     INFO("Game room, Current room user is: %d, need enter robot num is: %d.", userCount, needRobotNum);
    // }
    // msgNode->SetNeedRobotNum(needRobotNum);
    int minPlayerNum = confAccess_->GetMinPlayNumNeekCheck();
    int maxPlayerNum = confAccess_->GetMaxPlayerNum();
    if (0 == minPlayerNum) { //没有限制，根据maxplayer的数量加派机器人
        if (maxPlayerNum <= 0) {
            ERROR("Configure file max player num is not ilegal.");
            msgNode->SetNeedRobotNum(0);
        }
        msgNode->SetNeedRobotNum(maxPlayerNum);
        return true;
    }
    OrgRoomDdzNeedRobotAck orgRoomDdzNeedRobotAck;
    if (!DeserializePbMsg(&orgRoomDdzNeedRobotAck, msgNode->GetMsg())) {
        ERROR("parse query room status ack pb message error.");
        return false;
    }
    int result = orgRoomDdzNeedRobotAck.result();
    if (0 != result) {
        ERROR("Request query room failed, result is: %d.", result);
        return false;
    }
    if (!orgRoomDdzNeedRobotAck.has_robots()) {
        DEBUG("Doesn't has robots num info.");
        return true;
    }
    int iNeedRobotNum = orgRoomDdzNeedRobotAck.robots();
    DEBUG("Need robot num is %d.", iNeedRobotNum);
    if (iNeedRobotNum < 0) {
        ERROR("Robot num isn\'t corrent.");
        return true;
    }
    msgNode->SetNeedRobotNum(iNeedRobotNum);
    INFO("=================== RecvQueryRoomStatusAck END =================");
    return true;
}

bool RobotHeader::SendQuerySignUpCondReq(std::shared_ptr<MsgNode>& msgNode) //发送查询报名条件
{
    INFO("=================== SendQuerySignUpCondReq START =================");
    OrgRoomDdzSignUpConditionReq orgRoomDdzSignUpConditionReq;
    orgRoomDdzSignUpConditionReq.set_matchid(matchId_);
    if (!SerializeSendMsg(&orgRoomDdzSignUpConditionReq, robot::MSGID_DDZ_SIGN_UP_CONDITION_REQ, msgNode)) {
        ERROR("Robot %d send query sign up cond string serialize failed.");
        return false;
    }
    SendMsg(msgNode);
    INFO("=================== SendQuerySignUpCondReq END =================");
    return true;
}

bool RobotHeader::RecvQuerySignUpCondAck(std::shared_ptr<MsgNode>& msgNode) {
    //message OrgRoomDdzSignUpConditionAck {
    //    required int32 result = 1;
    //    message Limit {
    //        required bool enable = 1;   // 是否满足条件
    //        optional string desc = 2;   // 条件描述, 大厅配置获取中
    //    }
    //    message Cost {
    //        required int32 id = 1;      // 费用ID
    //        required string desc = 2;   // 费用描述
    //        required bool enable = 3;   // 是否满足
    //        required bool signed = 4;   // 是否已报名
    //    }
    //    optional Limit limit = 2;
    //    repeated Cost costList = 3;
    //    optional int32 sysTime = 4;     // 系统时间, time_t
    //    optional int32 startTime = 5;   // 对于定时赛，返回开赛时间, time_t
    //    optional int32 startSignUpTime = 6; // 开始报名时间, time_t
    //    optional int32 endSignUpTime = 7;   // 结束报名时间, time_t
    //}
    INFO("=================== RecvQuerySignUpCondAck START =================");
    INFO("Message for Header robot.");
    OrgRoomDdzSignUpConditionAck orgRoomDdzSignUpConditionAck;
    if (!DeserializePbMsg(&orgRoomDdzSignUpConditionAck, msgNode->GetMsg())) {
        ERROR("parse InitGame ack pb message error.");
        return false;
    }
    int result = orgRoomDdzSignUpConditionAck.result();
    if (message::SUCCESS != result) {
        ERROR("Header robot sign up condition failed, result is: %d.", result);
        return true;
    }

    int iStartSignUpTime = orgRoomDdzSignUpConditionAck.startsignuptime();//报名开始时间
    int iEndSignUpTime = orgRoomDdzSignUpConditionAck.endsignuptime();//报名结束时间
    int iGameBeginTime = orgRoomDdzSignUpConditionAck.starttime();//下一场比赛开始的时间

    if (!IsTimeToQueryRoomStatus(iStartSignUpTime, iEndSignUpTime, iGameBeginTime)) {
        return true;
    }
    INFO("=================== RecvQuerySignUpCondAck END =================");
    return SendQueryRoomStatusReq(msgNode);
}

bool RobotHeader::IsTimeToQueryRoomStatus(int iStartSignUpTime, int iEndSignUpTime, int iGameBeginTime) {
    char strCurrentTime[128] = {0};
    char strStartTime[128] = {0};
    char strEndTime[128] = {0};
    char strGameBeginTime[128] = {0};

    time_t currentTimer = ::time(NULL);
    time_t startSignUpTimer;
    time_t endSignUpTimer;
    time_t gameBeginTimer;

    struct tm* tmCurrentTime;
    struct tm* tmStartTime;
    struct tm* tmEndTime;
    struct tm* tmGameBeginTime;

    int iCurrentTime = (int)currentTimer;//系统当前时间    

    startSignUpTimer = (time_t)iStartSignUpTime;
    endSignUpTimer = (time_t)iEndSignUpTime;
    gameBeginTimer = (time_t)iGameBeginTime;

    tmStartTime = ::localtime(&startSignUpTimer);
    strftime(strStartTime, sizeof(strStartTime), "%Y-%m-%d %H:%M:%S", tmStartTime);
    INFO("Start sign up time is: %s.", strStartTime);

    tmEndTime = ::localtime(&endSignUpTimer);
    strftime(strEndTime, sizeof(strEndTime), "%Y-%m-%d %H:%M:%S", tmEndTime);
    INFO("End sign up time is: %s.", strEndTime);

    tmCurrentTime = ::localtime(&currentTimer);
    strftime(strCurrentTime, sizeof(strCurrentTime), "%Y-%m-%d %H:%M:%S", tmCurrentTime);
    INFO("Current time is: %s.", strCurrentTime);

    tmGameBeginTime = ::localtime(&gameBeginTimer);
    strftime(strGameBeginTime, sizeof(strGameBeginTime), "%Y-%m-%d %H:%M:%S", tmGameBeginTime);
    INFO("Game will start at: %s.", strGameBeginTime);

    int confleftTime = 300;//暂定为30s
    int curleftTime = iGameBeginTime - iCurrentTime;
    INFO("Check time is: %d second, current left time: %d second", confleftTime, curleftTime);

    if (iCurrentTime >= iStartSignUpTime && iCurrentTime < iEndSignUpTime) { //在定时赛时间范围内
        INFO("It\'s in sign up time trial match time.");
        if (curleftTime > confleftTime) { //说明还不到预先设定的检查时间
            INFO("It\'s not time to check sign up people num."); 
            return false;
        }
        INFO("It\'s time to check sign up people num."); //该查询该场次的报名人数了
        return true;
    } else {
        INFO("It isn\'s in sign up time trial match time.");
        return false;
    }
}

void QueryRoomStatusTimer(int fd, short events, void* arg) {
    MsgNode* oneMsgNode = static_cast<MsgNode*>(arg);
    RobotHeader* eventProcess = static_cast<RobotHeader*>(oneMsgNode->GetObjectPoint());
    if (nullptr == eventProcess) {
        ERROR("Get RobotBase point failed.");
        return;
    }
    shared_ptr<MsgNode> msgNode(new MsgNode);
    msgNode->SetConn(oneMsgNode->GetConn());
    if (!eventProcess->HeaderRobotAction(msgNode)) {
        ERROR("Header send query room status msg error.");
        return;
    }
    DEBUG("Send once query room status requre.");
}