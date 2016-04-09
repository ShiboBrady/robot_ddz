#include "RobotTask.h"
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

void DelaySendMsgTimer(int fd, short events, void* arg);

RobotTask::RobotTask(int iRobotId, int iRobotIQ)
    :RobotBase(iRobotId){
    robot_.RbtInSetLevel(iRobotIQ);
    AddMessageProcessFunc(robot::MSGID_DDZ_SIGN_UP_CONDITION_ACK, std::bind(&RobotTask::RecvQuerySignUpCondAck, this, placeholders::_1));  //查询报名条件
    AddMessageProcessFunc(robot::MSGID_DDZ_SIGN_UP_ACK, std::bind(&RobotTask::RecvSignUpAck, this, placeholders::_1));  //报名信息
    AddMessageProcessFunc(robot::MSGID_DDZ_QUICK_START_ACK, std::bind(&RobotTask::RecvQuickBeginGameAck, this, placeholders::_1));  //快速开始游戏消息
    AddMessageProcessFunc(robot::MSGID_KEEP_ACK, std::bind(&RobotTask::RecvKeepPlayAck, this, placeholders::_1));  //断线续玩消息
    AddMessageProcessFunc(robot::NOTIFY_SWITCH_SCENE, std::bind(&RobotTask::RecvEnterGameSceneNotify, this, placeholders::_1));  //进入游戏场景通知
    AddMessageProcessFunc(robot::NOTIFY_STARTGAME, std::bind(&RobotTask::RecvGameStartNotify, this, placeholders::_1));  //游戏开始通知
    AddMessageProcessFunc(robot::NOTIFY_DEALCARD, std::bind(&RobotTask::RecvInitHardCardNotify, this, placeholders::_1));  //初始化手牌通知
    AddMessageProcessFunc(robot::NOTIFY_CALLSCORE, std::bind(&RobotTask::RecvUserCallScoreNotify, this, placeholders::_1));  //玩家叫分通知
    AddMessageProcessFunc(robot::NOTIFY_SETLORD, std::bind(&RobotTask::RecvGetLordNotify, this, placeholders::_1));  //地主位置通知
    AddMessageProcessFunc(robot::NOTIFY_BASECARD, std::bind(&RobotTask::RecvGetBaseCardNotify, this, placeholders::_1));  //底牌通知
    AddMessageProcessFunc(robot::NOTIFY_TAKEOUT, std::bind(&RobotTask::RecvGetTakeOutCardNotify, this, placeholders::_1));  //出牌通知
    AddMessageProcessFunc(robot::NOTIFY_TRUST, std::bind(&RobotTask::RecvGetTrustNotify, this, placeholders::_1));  //托管通知
    AddMessageProcessFunc(robot::NOTIFY_GAMEOVER, std::bind(&RobotTask::RecvGetGameOverNotify, this, placeholders::_1));  //游戏结束通知
    AddMessageProcessFunc(robot::MSGID_DDZ_GAME_RESULT_NTF, std::bind(&RobotTask::RecvGetGameResultNotify, this, placeholders::_1));  //游戏结果通知
    AddMessageProcessFunc(robot::MSGID_DDZ_MATCH_OVER_NTF, std::bind(&RobotTask::RecvGetCompetitionOverNotify, this, placeholders::_1));  //比赛结束通知
    AddMessageProcessFunc(robot::MSGID_READY_ACK, std::bind(&RobotTask::RecvReadyForGameAck, this, placeholders::_1));  //准备好游戏消息
    AddMessageProcessFunc(robot::MSGID_CALLSCORE_ACK, std::bind(&RobotTask::RecvCallScoreAck, this, placeholders::_1));  //叫分消息
    AddMessageProcessFunc(robot::MSGID_TAKEOUT_ACK, std::bind(&RobotTask::RecvTakeOutCardAck, this, placeholders::_1));  //出牌消息
    AddMessageProcessFunc(robot::MSGID_TRUST_CANCEL_ACK, std::bind(&RobotTask::RecvTrustCancelAck, this, placeholders::_1));  //取消托管消息
    AddMessageProcessFunc(robot::MSGID_DDZ_CANCEL_SIGN_UP_ACK, std::bind(&RobotTask::RecvCancelSignUpAck, this, placeholders::_1));  //取消报名消息

    delaySendActiveMsgTime_ = confAccess_->GetSendActiveMsgDelayTime();

    delaySendPassiveMsgTime_ = confAccess_->GetSendPassiveMsgDelayTime();
}

bool RobotTask::AnalysisAndDecision(std::shared_ptr<MsgNode>& msgNode) {
    if (!msgNode->GetIsNeedKeepPlay()) { //测试阶段，且不需要断线续玩，就断开连接
        disConnectCallback_(msgNode->GetConn());
        return true;
    }
    if (msgNode->GetIsNeedDelRobot()) { //需要删除机器人
        DEBUG("Will delete robot %d.", robotId_);
        disconnectAndDelRobotCallback_(msgNode->GetConn());
        return true;
    }
    if (msgNode->GetMsgNeedSend()) { //需要发送消息
        if (msgNode->GetMsgDelaySecond()) { //需要延时发送
            DEBUG("Robot %d send delay msg.", robotId_);
            MsgNode* delayMsgNode = new MsgNode;
            delayMsgNode->SetMsg(msgNode->GetMsg());
            delayMsgNode->SetMsgId(msgNode->GetMsgId());
            delayMsgNode->SetConn(msgNode->GetConn());
            delayMsgNode->SetTimer(msgNode->GetMsgDelaySecond(), 0);
            delayMsgNode->SetObjectPoint(this);
            timerFunc_(DelaySendMsgTimer, delayMsgNode, true);
        } else { //不需要延时发送
            DEBUG("Robot %d send immediately msg.", robotId_);
            SendMsg(msgNode);
        }
    } else if (msgNode->GetGameIsOver()) { //比赛结束
        DEBUG("Game over, and robot will exit.");
        // delTimerFunc_(ReceiveMsgTimeOutTimer_);
        // bIsTimerInit_ = false;
        disConnectCallback_(msgNode->GetConn());
    }
    return true;
}

inline bool RobotTask::NextActionProcess( std::shared_ptr<MsgNode>& msgNode ) {
    if (KEEPPLAY == status_) {
        needKeepPlay_ = false;
        if (!SendKeepPlayReq(msgNode)) {
            ERROR("Robot %d Send Keep play req failed.", robotId_);
            return false;
        }
        return true;
    }
    return isMatch_ ? SendQuerySignUpCondReq(msgNode) : SendQuickBeginGameReq(msgNode);
}

void RobotTask::RobotInit() {
    status_ = INIT;
    myScore_ = 0;
    needKeepPlay_ = false;
    isTestNeedKeepPlay_ = false;
    robot_.RbtResetData();
}

void RobotTask::DeleteTimer() {

}

//快速开始游戏消息
bool RobotTask::SendQuickBeginGameReq(std::shared_ptr<MsgNode>& msgNode)  //发送快速开始游戏
{
    INFO("=================== SendQuickBeginGameReq START =================");
    OrgRoomDdzQuickStartReq orgRoomDdzQuickStartReq;
    orgRoomDdzQuickStartReq.set_roomid(matchId_);
    if (!SerializeSendMsg(&orgRoomDdzQuickStartReq, robot::MSGID_DDZ_QUICK_START_REQ, msgNode)) {
        ERROR("Robot %d send quick game string serialize failed.");
        return false;
    }
    INFO("=================== SendQuickBeginGameReq END =================");
    return true;
}

bool RobotTask::RecvQuickBeginGameAck(std::shared_ptr<MsgNode>& msgNode) {
    //message OrgRoomDdzQuickStartAck {
    //    required int32 result = 1;
    //}
    INFO("=================== RecvQuickBeginGameAck START =================");
    INFO("Message for robot %d.", robotId_);
    if (INITGAME != status_) {
        if (GAMMING == status_) {
            ERROR("Robot %d is in GAMMING status, but received Quick Begin Game Ack.", robotId_);
            return true;
        } else {
            ERROR("Robot %d doesn't in INITGAME status, robot status is: %s.", robotId_, STATUS_CHAR[status_]);
            return false;
        }
    }
    OrgRoomDdzQuickStartAck orgRoomDdzQuickStartAck;
    if (!DeserializePbMsg(&orgRoomDdzQuickStartAck, msgNode->GetMsg())) {
        ERROR("parse InitGame ack pb message error.");
        return false;
    }

    int result = orgRoomDdzQuickStartAck.result();
    if (0 != result) {
        if (506 == result) {
            ERROR("Robot %d request quick game failed, coin is not enouth.", robotId_, result);
            msgNode->SetNeedDelRobot(true);
            return true;
        } else {
            ERROR("Robot %d request quick game failed, result code is: %d.", robotId_, result);
        }
        return false;
    } else {
        status_ = QUICKGAME;
        INFO("Robot %d request quick game succssed, robot status is: QUICKGAME.", robotId_);
    }
    INFO("=================== RecvQuickBeginGameAck END =================");
    return true;
}

bool RobotTask::SendQuerySignUpCondReq(std::shared_ptr<MsgNode>& msgNode) //发送查询报名条件
{
    INFO("=================== SendQuerySignUpCondReq START =================");
    OrgRoomDdzSignUpConditionReq orgRoomDdzSignUpConditionReq;
    orgRoomDdzSignUpConditionReq.set_matchid(matchId_);
    if (!SerializeSendMsg(&orgRoomDdzSignUpConditionReq, robot::MSGID_DDZ_SIGN_UP_CONDITION_REQ, msgNode)) {
        ERROR("Robot %d send query sign up cond string serialize failed.");
        return false;
    }
    INFO("=================== SendQuerySignUpCondReq END =================");
    return true;
}

bool RobotTask::RecvQuerySignUpCondAck(std::shared_ptr<MsgNode>& msgNode) {
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
    INFO("Message for robot %d.", robotId_);
    if (INITGAME != status_) {
        ERROR("Robot %d doesn't in INITGAME status, robot status is: %s.", robotId_, STATUS_CHAR[status_]);
        return false;
    }
    OrgRoomDdzSignUpConditionAck orgRoomDdzSignUpConditionAck;
    if (!DeserializePbMsg(&orgRoomDdzSignUpConditionAck, msgNode->GetMsg())) {
        ERROR("parse InitGame ack pb message error.");
        return false;
    }
    int result = orgRoomDdzSignUpConditionAck.result();
    if (message::SUCCESS != result) {
        ERROR("Robot %d sign up condition failed, result is: %d.", robotId_, result);
        return false;
    }

    //开始判断是否可以报名
    if (!orgRoomDdzSignUpConditionAck.has_limit()) {
        INFO("Robot %d can sign up for free, has been set to CANSINGUP.", robotId_);
        status_ = CANSINGUP;
    } else {
        if (!orgRoomDdzSignUpConditionAck.limit().enable()) {
            ERROR("Robot %d can't sign up, enable in sign up condition is false.", robotId_);
            return false;
        } else {
            INFO("Robot %d can sign up.", robotId_);
            int costSize = orgRoomDdzSignUpConditionAck.costlist_size();
            int index = 0;
            for (index = 0; index < costSize; ++index) {
                if (orgRoomDdzSignUpConditionAck.costlist(index).enable()) {
                    costId_ = orgRoomDdzSignUpConditionAck.costlist(index).id();
                    INFO("Found enable cost id %d in costlist.", costId_);
                    if (orgRoomDdzSignUpConditionAck.costlist(index).signed_()) {
                        INFO("Robit %d has already sign up, status is: SIGNUPED.", robotId_);
                        status_ = SIGNUPED;
                        return true;
                    }
                    break;
                }
            }
            if (costSize == index) {
                ERROR("Doesn't found enable cost in costlist, Robot %d can't sign up.", robotId_);
                return false;
            } else {
                INFO("Robot %d has been set to CANSINGUP, will send sign up request.", robotId_);
                status_ = CANSINGUP;
            }
        }
    }
    if (!SendSignUpReq(msgNode)) {
        ERROR("Robot %d send sign up req failed.", robotId_);
        return false;
    }
    INFO("=================== RecvQuerySignUpCondAck END =================");
    return true;
}

bool RobotTask::SendSignUpReq(std::shared_ptr<MsgNode>& msgNode)          //发送报名信息
{
    INFO("=================== SendSignUpReq START =================");
    OrgRoomDdzSignUpReq orgRoomDdzSignUpReq;
    orgRoomDdzSignUpReq.set_matchid(matchId_);
    orgRoomDdzSignUpReq.set_costid(costId_);
    if (!SerializeSendMsg(&orgRoomDdzSignUpReq, robot::MSGID_DDZ_SIGN_UP_REQ, msgNode)) {
        return false;
    }
    INFO("=================== SendSignUpReq END =================");
    return true;
}

bool RobotTask::RecvSignUpAck(std::shared_ptr<MsgNode>& msgNode) {
    //message OrgRoomDdzSignUpAck {
    //    required int32 result = 1;
    //    message CostGoods {
    //        required string name = 1;   // 消耗物品名称 chips-筹码, vipPoints-竞技点
    //        required int32 count = 2;   // 消耗物品数量
    //    }
    //    repeated CostGoods costList = 2;
    //    optional int32 userCount = 3;   // 已报名人数
    //}
    INFO("=================== RecvSignUpAck START =================");
    INFO("Message for robot %d.", robotId_);
    if (CANSINGUP != status_) {
        if (GAMMING == status_) {
            ERROR("Robot %d is in GAMMING status, but received Signed Up Ack.", robotId_);
            return true;
        } else {
            ERROR("Robot %d doesn't in can sign up status, robot status is: %s.", robotId_, STATUS_CHAR[status_]);
            return false;
        }
    }
    OrgRoomDdzSignUpAck orgRoomDdzSignUpAck;
    if (!DeserializePbMsg(&orgRoomDdzSignUpAck, msgNode->GetMsg())) {
        ERROR("Parse OrgRoomDdzSignUpAck protobuf msg error.");
        return false;
    }
    int result = orgRoomDdzSignUpAck.result();
    if (0 != result) {
        if (508 == result) {
            INFO("Robit %d has already sign up.", robotId_);
        } else {
            ERROR("Robot %d sign up failed, result is: %d.", robotId_, result);
            return false;
        }
    }
    INFO("Robot %d sign up succssed, robot status is: SIGNUPED.", robotId_);
    status_ = SIGNUPED;
    INFO("=================== RecvSignUpAck END =================");
    return true;
}

bool RobotTask::SendKeepPlayReq(shared_ptr<MsgNode>& msgNode)        //发送断线续玩消息
{
    //发送断线续玩请求
    //message KeepPlayingReq {
    //    required int32 rev = 1;         // reserved
    //}
    INFO("=================== SendKeepPlayReq START =================");
    KeepPlayingReq keepPlayingReq;
    keepPlayingReq.set_rev(1001);
    if (!SerializeSendMsg(&keepPlayingReq, robot::MSGID_KEEP_REQ, msgNode)) {
        ERROR("KeepPlayingReq serialize failed.");
        return false;
    }
    DEBUG("Robot %d will send a keepPlayingReq.", robotId_);
    INFO("=================== SendKeepPlayReq END =================");
    return true;
}

bool RobotTask::RecvKeepPlayAck(std::shared_ptr<MsgNode>& msgNode) {      //断线续玩消息回复
    // 断线续玩
    // message KeepPlayingAck {
    //     required int32 result = 1;
    //     message KeepDetail {
    //         // 游戏实时信息
    //         message GameInfo {
    //             required int32 status = 1;      // 游戏当前状态
    //             required int32 seatlord = 2;    // 地主的座位号
    //             required int32 seatactive = 3;  // 当前活动玩家座位号
    //             required int32 multiple = 4;    // 当前倍数, 基本倍数, 如果有踢的需要各自累加倍数
    //             required int32 maxcallscore = 5;// 当前最大叫分
    //             repeated int32 basecards = 6;   // 底牌数据
    //         }
            
    //         // 玩家动态信息
    //         message PlayerInfo {
    //             required bool trust = 1;          // 托管状态
    //             required int32 trustsurplus = 2;  // 解除托管剩余次数
    //             required int32 callscore = 3;     // 叫分值
    //             repeated int32 cards = 4;         // 手牌内容
    //             repeated int32 lastcards = 5;     // 最后一手牌
    //             required UserInfo detailinfo = 6; // 用户详细信息 
    //         }
            
    //         required GameInfo gameinfo = 1;     // 游戏信息
    //         repeated PlayerInfo playerinfo = 2; // 玩家信息
    //         required int32 ready = 3;           // 准备超时时间
    //         required int32 callscore = 4;       // 叫分超时时间
    //         required int32 takeout = 5;         // 出牌超时时间
    //         required int32 settle = 6;          // 结算框显示时间（超时后自动准备）
    //         required string gameChannel = 7;    // 游戏服务通道号
    //         required int32 basicScore   = 8;    // 底分
    //         optional MatchInfo matchInfo = 9;  // 如果是游戏场，该字段不用
    //     }
    //     optional KeepDetail keepInfo = 2;
    // }
    INFO("=================== RecvKeepPlayAck START =================");
    INFO("Message for robot %d.", robotId_);
    if (KEEPPLAY != status_) {
        ERROR("Robot %d doesn't in KEEPPLAY status, robot status is: %s.", robotId_, STATUS_CHAR[status_]);
        return false;
    }
    KeepPlayingAck keepPlayingAck;
    if (!DeserializePbMsg(&keepPlayingAck, msgNode->GetMsg())) {
        return false;
    }
    int result = keepPlayingAck.result();
    if (0 != result) {
        ERROR("Get keep play req failed, reslut code is: %d. Robot will exit.", result, robotId_);
        status_ = INIT;
        return false;
    }
    int index = 0;
    int iPlayerInfoSize = keepPlayingAck.keepinfo().playerinfo_size();
    for (index = 0; index < iPlayerInfoSize; ++index) {
        int iNetId = ::atoi(keepPlayingAck.keepinfo().playerinfo(index).detailinfo().username().c_str());
        INFO("total userNum: %d, robot id: %d, net id: %d.", iPlayerInfoSize, robotId_, iNetId);
        if (iNetId == robotId_) {
            break;
        }
    }
    if (iPlayerInfoSize == index) {
        ERROR("Doesn't find robot name.");
        return false;
    } else {
        robot_.SetAiSeat(index);
    }
    int iAiSeat = index;
    int iLordSeat = keepPlayingAck.keepinfo().gameinfo().seatlord();
    INFO("robot %d seat is: %d, lord seat is: %d", robotId_, iAiSeat, iLordSeat);
    vector<int> vecHandCards;
    vector<int> vecBaseCards;
    int iBaseCardsSize = keepPlayingAck.keepinfo().gameinfo().basecards_size();
    for (index = 0; index < iBaseCardsSize; ++index) {
        vecBaseCards.push_back(keepPlayingAck.keepinfo().gameinfo().basecards(index));
    }
    INFO("Robot %d current base cards is:", robotId_);
    printCardInfo(vecBaseCards);
    int iHandsCardsSize = keepPlayingAck.keepinfo().playerinfo(iAiSeat).cards_size();
    for (index = 0; index < iHandsCardsSize; ++index) {
        vecHandCards.push_back(keepPlayingAck.keepinfo().playerinfo(iAiSeat).cards(index));
    }
    INFO("Robot %d current hands cards is:", robotId_);
    printCardInfo(vecHandCards);

    //注意先后顺序
    robot_.RbtResetData();//重置
    robot_.RbtInSetSeat(iAiSeat, iLordSeat);//设置座位信息
    robot_.RbtInSetCard(vecHandCards, vecBaseCards);//设置手牌信息
    status_ = GAMMING;

    //处于托管状态，需发送取消托管请求
    if (!SendTrustCancelReq(msgNode)){
        return false;
    }
    INFO("robot %d send a TrustLiftReq.", robotId_);
    INFO("=================== RecvKeepPlayAck END =================");
    return true;
}

bool RobotTask::RecvEnterGameSceneNotify(std::shared_ptr<MsgNode>& msgNode) {
    // 进入游戏场景通知
    //message GameSwitchSceneNtf {
    //    required string gameName = 1;
    //    required bool isMatch    = 2;     // 是否为游戏场，true为是
    //}
    INFO("=================== RecvEnterGameSceneNotify START =================");
    INFO("Message for robot %d.", robotId_);
    GameSwitchSceneNtf gameSwitchSceneNtf;
    if (!DeserializePbMsg(&gameSwitchSceneNtf, msgNode->GetMsg())) {
        ERROR("Deserialize enter game scene notify msg error.");
        return false;
    }
    if (SIGNUPED != status_ && QUICKGAME != status_ && GAMMING != status_ && KEEPPLAY != status_)
    {
        if (INITGAME == status_) {
            ERROR("Robot %d doesn't received QuickGame ack or Signed up ack, but will Set to GAMMING status.", robotId_);
        } else {
            ERROR("Robot %d doesn't in SIGNUPED or QUICKGAME or GAMMING or KEEPPLAY status, status is: %s.", robotId_, STATUS_CHAR[status_]);
            return false;
        }
    }
    status_ = GAMMING;
    robot_.RbtResetData();
    INFO("Set robot %d to GAMMING status.", robotId_);

    if (!SendReadyForGameReq(msgNode)) {
        ERROR("Robot %d send ready for game req failed.", robotId_);
        return false;
    }
    INFO("=================== RecvEnterGameSceneNotify END =================");
    return true;
}

bool RobotTask::RecvGameStartNotify(std::shared_ptr<MsgNode>& msgNode) {
    // 游戏开始
    //message GameStartNtf {
    //    required string gameName = 1;       // 游戏服务通道号
    //    required int32 basicScore   = 2;    // 底分
    //    repeated UserInfo userinfo  = 3;    // 用户信息
    //    optional MatchInfo matchInfo = 4;   // 如果是游戏场，该字段不用
    //}
    INFO("=================== RecvGameStartNotify START =================");
    INFO("Message for robot %d.", robotId_);
    if (GAMMING != status_) {
        ERROR("Robot %d doesn't in gamming status, robot status is %d.", robotId_, STATUS_CHAR[status_]);
        return false;
    }
    GameStartNtf gameStartNtf;
    if (!DeserializePbMsg(&gameStartNtf, msgNode->GetMsg())) {
        ERROR("parse pb message GameStartNtf error.");
        return false;
    }
    int iUserNum = gameStartNtf.userinfo_size();
    for (int index = 0; index < iUserNum; ++index) {//寻找自己的座位号:0-2
        if (::atoi(gameStartNtf.userinfo(index).username().c_str()) == robotId_) {
            robot_.SetAiSeat(index);
            INFO("Set robot %d seat successed, seat is: %d.", robotId_, index);
            return true;
        }
    }
    ERROR("Doesn't find robot name.");
    INFO("=================== RecvGameStartNotify END =================");
    return true;
}

bool RobotTask::RecvInitHardCardNotify(std::shared_ptr<MsgNode>& msgNode) {
    // 发牌
    //message DealCardNtf {
    //    required int32 headerseat = 1;          // 第一个叫分座位号
    //    repeated HandCardList cards = 2;        // 玩家手牌
    //}
    INFO("=================== RecvInitHardCardNotify START =================");
    INFO("Message for robot %d.", robotId_);
    if (GAMMING != status_) {
        ERROR("Robot %d doesn't in gamming status, robot status is %s.", robotId_, STATUS_CHAR[status_]);
        return true;
    }
    DealCardNtf dealCardNtf;
    if (!DeserializePbMsg(&dealCardNtf, msgNode->GetMsg())) {
        ERROR("parse pb message DealCardNtf error.");
        return false;
    }
    int aiSeat = robot_.GetAiSeat();
    if (-1 == aiSeat) {
        ERROR("Not init seat info.");
        return false;
    }
    int hearderSeat = dealCardNtf.headerseat();
    int cardsSize = dealCardNtf.cards(aiSeat).cards_size();//查看自己的那手牌信息

    vector<int> vecHandCard;
    for (int index = 0; index < cardsSize; ++index) {//获取自己的那手牌
        vecHandCard.push_back(dealCardNtf.cards(aiSeat).cards(index));
    }
    robot_.RbtInInitCard(aiSeat, vecHandCard);
    DEBUG("Init hand card successed, hand card info is: ");
    printCardInfo(vecHandCard);

    //判断自己是不是第一个叫分座位号
    if (hearderSeat == aiSeat) {
        robot_.RbtOutGetCallScore(myScore_);
        if (!SendCallScoreReq(msgNode)) {
            ERROR("Send call score req failed.");
            return false;
        }
    }
    INFO("=================== RecvInitHardCardNotify END =================");
    return true;
}

bool RobotTask::RecvUserCallScoreNotify(std::shared_ptr<MsgNode>& msgNode) {
    // 叫分
    //message UserCallScoreNtf {
    //    required int32 seatno = 1;                  // 座位号
    //    required int32 seatnext = 2[default=-1];    // 下一个叫分座位，-1叫分结束
    //    required int32 score = 3;                   // 叫分值(1/2/3), 0-不叫
    //}
    INFO("=================== RecvUserCallScoreNotify START =================");
    INFO("Message for robot %d.", robotId_);
    if (GAMMING != status_) {
        ERROR("Robot %d doesn't in gamming status, robot status is %s.", robotId_, STATUS_CHAR[status_]);
        return true;
    }
    UserCallScoreNtf userCallScoreNtf;
    if (!DeserializePbMsg(&userCallScoreNtf, msgNode->GetMsg())) {
        ERROR("parse pb message UserCallScoreNtf error.");
        return false;
    }
    int seatNo = userCallScoreNtf.seatno();
    int seatNext = userCallScoreNtf.seatnext();
    int score = userCallScoreNtf.score();
    int aiSeat = robot_.GetAiSeat();

    INFO("seatno: %d, seatnext: %d, my seat: %d, score: %d", seatNo, seatNext, aiSeat, score);
    if (-1 == seatNext) {//停止叫分
        return true;
    }
    if (seatNext != aiSeat) {//没轮到自己，不叫
        robot_.RbtInCallScore(seatNo, score);
        return true;
    } else {
        int myScore = 0;
        robot_.RbtOutGetCallScore(myScore);
        int curScore = robot_.GetCurScore();
        if (score >= myScore || (score == 0 && robot_.GetCurScore() >= myScore)) { //目前分数比自己的大 or 上一个用户没叫，且上上一个用户叫的分比自己的高不叫
            myScore_ = 0;
            INFO("Doesn't choose call score, curScore is: %d, my score is: %d", curScore, myScore);
        } else { //叫分
            myScore_ = myScore;
            INFO("Choose to call score, score is: %d.", robotId_, myScore);
        }
        if (!SendCallScoreReq(msgNode)) {
            ERROR("Send call score req failed.");
            return false;
        }
    }
    INFO("=================== RecvUserCallScoreNotify END =================");
    return true;
}

bool RobotTask::RecvGetLordNotify(std::shared_ptr<MsgNode>& msgNode) {
    // 地主确定
    //message LordSetNtf {
    //    required int32 seatlord = 1;    // 地主座位号
    //    required int32 callscore = 2;   // 地主叫分
    //}
    INFO("=================== RecvGetLordNotify START =================");
    INFO("Message for robot %d.", robotId_);
    if (GAMMING != status_) {
        ERROR("Robot %d doesn't in gamming status, robot status is %s.", robotId_, STATUS_CHAR[status_]);
        return true;
    }
    LordSetNtf lordSetNtf;
    if (!DeserializePbMsg(&lordSetNtf, msgNode->GetMsg())) {
        ERROR("parse pb message LordSetNtf error.");
        return false;
    }
    int seatLord = lordSetNtf.seatlord();
    robot_.SetLordSeat(seatLord);
    INFO("Lord seat is %d.", seatLord);
    INFO("=================== RecvGetLordNotify END =================");
    return true;
}

bool RobotTask::RecvGetBaseCardNotify(std::shared_ptr<MsgNode>& msgNode) {
    // 发底牌
    //message SendBaseCardNtf {
    //    repeated int32 basecards = 1;   // 底牌数据
    //}
    INFO("=================== RecvGetBaseCardNotify START =================");
    INFO("Message for robot %d.", robotId_);
    if (GAMMING != status_) {
        ERROR("Robot %d doesn't in gamming status, robot status is %s.", robotId_, STATUS_CHAR[status_]);
        return true;
    }
    SendBaseCardNtf sendBaseCardNtf;
    if (!DeserializePbMsg(&sendBaseCardNtf, msgNode->GetMsg())) {
        ERROR("parse pb message SendBaseCardNtf error.");
        return false;
    }
    int baseCardSize = sendBaseCardNtf.basecards_size();
    int seatLord = robot_.GetLordSeat();
    vector<int> vecBaseCard;
    for (int index = 0; index < baseCardSize; ++index) {
        vecBaseCard.push_back(sendBaseCardNtf.basecards(index));
    }
    INFO("Base card info:");
    printCardInfo(vecBaseCard);
    robot_.RbtInSetLord(seatLord, vecBaseCard);

    //判断自己是不是地主
    if (robot_.GetLordSeat() == robot_.GetAiSeat()) {
        INFO("I am lord, so it's my turn to take out first card.");
        if (!SendTakeOutCardReq(msgNode)) {
            return false;
        }
    }
    INFO("=================== RecvGetBaseCardNotify END =================");
    return true;
}

bool RobotTask::RecvGetTakeOutCardNotify(std::shared_ptr<MsgNode>& msgNode) {
    // 出牌
    //message TakeoutCardNtf {
    //    required int32 seatno = 1;      // 出牌座位号
    //    required int32 seatnext = 2;    // 下一个出牌座位
    //    repeated int32 cards = 3;       // 出牌数据
    //    required int32 cardtype = 4;    // 类型
    //    required int32 multiple = 5;    // 当前倍数(炸弹产生的倍数)
    //}
    INFO("=================== RecvGetTakeOutCardNotify START =================");
    INFO("Message for robot %d.", robotId_);
    if (GAMMING != status_) {
        ERROR("Robot %d doesn't in gamming status, robot status is %s.", robotId_, STATUS_CHAR[status_]);
        return true;
    }
    TakeoutCardNtf takeoutCardNtf;
    if (!DeserializePbMsg(&takeoutCardNtf, msgNode->GetMsg())) {
        ERROR("parse pb message TakeoutCardNtf error.");
        return false;
    }
    int seatno = takeoutCardNtf.seatno();
    int seatnext = takeoutCardNtf.seatnext();
    int cardsNum = takeoutCardNtf.cards_size();
    int aiSeat = robot_.GetAiSeat();
    vector<int> vecOppTackOutCard;
    for (int index = 0; index < cardsNum; ++index) {
        vecOppTackOutCard.push_back(takeoutCardNtf.cards(index));
    }
    INFO("Current take out card info is:");
    printCardInfo(vecOppTackOutCard);
    robot_.RbtInTakeOutCard(seatno, vecOppTackOutCard);
    if (seatno == aiSeat) { //判断上次出牌是否是系统代出的
        INFO("It's my taked out last time.");
        if (!robot_.IsLastTakeOutCards(vecOppTackOutCard)) {
            INFO("Take out error last time.");
            robot_.RemoveExtraCards(vecOppTackOutCard); //将出的牌从自己的牌中去掉
        }
    }
    if (seatnext == aiSeat) { //出牌
        INFO("It's my turn to take out card, my take out cards is:");
        if (!SendTakeOutCardReq(msgNode)) {
            return false;
        }
    }
    INFO("=================== RecvGetTakeOutCardNotify START =================");
    return true;
}

bool RobotTask::RecvGetTrustNotify(std::shared_ptr<MsgNode>& msgNode) {
    // 托管
    //message TrustNtf {
    //    required int32 seatno = 1;
    //}
    INFO("=================== RecvGetTrustNotify START =================");
    INFO("Message for robot %d.", robotId_);
    if (GAMMING != status_) {
        ERROR("Robot %d doesn't in gamming status, robot status is %s.", robotId_, STATUS_CHAR[status_]);
        return true;
    }
    TrustNtf trustNtf;
    if (!DeserializePbMsg(&trustNtf, msgNode->GetMsg())) {
        ERROR("parse pb message TrustNtf error.");
        return false;
    }

    int seatNo = trustNtf.seatno();
    //判断是否是自己被进入托管
    int mySeatNo = robot_.GetAiSeat();
    INFO("Trust seatno: %d, my seatno: %d.", seatNo, mySeatNo);
    if (mySeatNo == seatNo) {//说明上一局出牌错误，需要先将上一局出得牌恢复，然后发送取消托管请求
        robot_.RecoveryHandCards();//恢复手牌记录
        if (!SendTrustCancelReq(msgNode)) {
            return false;
        }
    }
    INFO("=================== RecvGetTrustNotify END =================");
    return true;
}

bool RobotTask::RecvGetGameOverNotify(std::shared_ptr<MsgNode>& msgNode) {
    // 游戏结束通知
    //message GameOverNtf {
    //    required int32 reason = 1[default=2];   // 结束原因：1-强制结束，2-达到最大游戏盘数
    //}
    INFO("=================== RecvGetGameOverNotify START =================");
    INFO("Message for robot %d.", robotId_);
    GameOverNtf gameOverNtf;
    if (!DeserializePbMsg(&gameOverNtf, msgNode->GetMsg())) {
        ERROR("parse pb message gameOverNtf error.");
        return true;
    }
    robot_.RbtResetData();
    status_ = INIT;
    msgNode->SetGameIsOver(true);
    INFO("Receved game over notify.");
    INFO("=================== RecvGetGameOverNotify END =================");
    return true;
}

bool RobotTask::RecvGetGameResultNotify(std::shared_ptr<MsgNode>& msgNode) {
    // 游戏结果(游戏场和比赛场通用)
    //message OrgRoomDdzGameResultNtf {
    //    required int32 seatlord = 1;        // 地主的座位号
    //    required int32 seatwin = 2;         // 胜利座位
    //    required int32 multiple_base = 3;   // 基础倍数(叫分值)
    //    required int32 bombs = 4;           // 普通炸弹个数
    //    required bool  hasrocket = 5;       // 是否有火箭
    //    required bool  hasspring = 6;       // 是否春天
    //    required string guid = 7;
    //    message UserResult {
    //        required string userId = 1;
    //        required int64 chip_change = 2; // 筹码变化
    //        required int64 chip_now = 3;    // 当前筹码
    //        repeated int32 cards = 4;       // 玩家手牌
    //    }
    //    repeated UserResult userresult = 8;
    //    required int32 tax = 9;             // 服务费
    //    required int32 incExp = 10;         // 增加的经验值
    //    required int32 incVipPoints = 11;   // 增加的竞技点
    //}
    INFO("=================== RecvGetGameResultNotify START =================");
    INFO("Message for robot %d.", robotId_);
    robot_.RbtResetData();
    status_ = GAMMING;
    INFO("Receved game result notify, robot waitting for next game.");
    if (!confAccess_->GetIsMatch()) {
        status_ = INIT;
        msgNode->SetGameIsOver(true);
    }
    INFO("=================== RecvGetGameResultNotify END =================");
    return true;
}

bool RobotTask::RecvGetCompetitionOverNotify(std::shared_ptr<MsgNode>& msgNode) {
    //message OrgRoomDdzMatchOverNtf {
    //    required int32 matchId = 1;        // 比赛ID
    //    required int32 selfPlace = 2;      // 自己的名次
    //    required int32 userCount = 3;      // 总人数
    //    message rewardGoods {
    //        required string name = 1;      // 物品名称 chips-筹码, vipPoints-竞技点
    //        required int32 count = 2;      // 物品数量
    //    }
    //    // 奖励的物品
    //    repeated rewardGoods rewardList = 4;
    //}
    INFO("=================== RecvGetCompetitionOverNotify START =================");
    INFO("Message for robot %d.", robotId_);;
    robot_.RbtResetData();
    status_ = INIT;
    msgNode->SetGameIsOver(true);
    INFO("Receved competition notify, robot will exit.");
    INFO("=================== RecvGetCompetitionOverNotify END =================");
    return true;
}

bool RobotTask::SendReadyForGameReq(std::shared_ptr<MsgNode>& msgNode)  {  //发送准备好游戏消息
    //发送准备完毕请求
    //message ReadyReq {
    //    required int32 rev = 1;
    //}
    INFO("=================== SendReadyForGameReq START =================");
    ReadyReq readyReq;
    readyReq.set_rev(1001);
    if (!SerializeSendMsg(&readyReq, robot::MSGID_READY_REQ, msgNode)) {
        return false;
    }
    INFO("=================== SendReadyForGameReq END =================");
    return true;
}

bool RobotTask::RecvReadyForGameAck(std::shared_ptr<MsgNode>& msgNode) {
    //message ReadyAck {
    //    required int32 result = 1;
    //}
    INFO("=================== RecvReadyForGameAck START =================");
    INFO("Message for robot %d.", robotId_);
    if (GAMMING != status_) {
        ERROR("Robot %d doesn't in gamming status, robot status is %s.", robotId_, STATUS_CHAR[status_]);
        return true;
    }
    ReadyAck readyAck;
    if (!DeserializePbMsg(&readyAck, msgNode->GetMsg())) {
        ERROR("parse pb message readyAck error.");
        return false;
    }
    int result = readyAck.result();
    if (0 != result) {
        ERROR("Get ready req failed, result is: %d.", result);
    } else {
        INFO("Get ready req successed.");
    }
    INFO("=================== RecvReadyForGameAck END =================");
    return true;
}

bool RobotTask::SendCallScoreReq(std::shared_ptr<MsgNode>& msgNode)       //发送叫分消息
{
    INFO("=================== SendCallScoreReq START =================");
    CallScoreReq callScoreReq;
    callScoreReq.set_score(myScore_);
    if (!SerializeSendMsg(&callScoreReq, robot::MSGID_CALLSCORE_REQ, msgNode)) {
        return false;
    }
    int iDelayTime = 0;
    if (robot::NOTIFY_DEALCARD == msgNode->GetMsgId()) {
        iDelayTime = delaySendActiveMsgTime_;
    } else {
        srand((int)time(NULL));
        iDelayTime = rand() % (delaySendPassiveMsgTime_ - 1) + 1;
    }
    msgNode->SetMsgDelaySecond(iDelayTime);
    INFO("=================== SendCallScoreReq END =================");
    return true;
}
bool RobotTask::RecvCallScoreAck(std::shared_ptr<MsgNode>& msgNode) {
    //message CallScoreAck {
    //    required int32 result = 1;
    //}
    INFO("=================== RecvCallScoreAck START =================");
    INFO("Message for robot %d.", robotId_);
    if (GAMMING != status_) {
        ERROR("Robot %d doesn't in gamming status, robot status is %s.", robotId_, STATUS_CHAR[status_]);
        return false;
    }
    CallScoreAck callScoreAck;
    if (!DeserializePbMsg(&callScoreAck, msgNode->GetMsg())) {
        ERROR("parse pb message CallScoreAck error.");
        return false;
    }
    int result = callScoreAck.result();

    if (0 != result) {
        ERROR("Get call score result, call score failed, result is: %d.", result);
    } else {
        INFO("Get call score result, call score successed.");
    }
    INFO("=================== RecvCallScoreAck END =================");
    return true;
}

bool RobotTask::SendTakeOutCardReq(std::shared_ptr<MsgNode>& msgNode)     //发送出牌消息
{
    INFO("=================== SendTakeOutCardReq START =================");
    vector<int> vecTackOutCard;
    robot_.RbtOutGetTakeOutCard(vecTackOutCard);
    printCardInfo(vecTackOutCard);

    TakeoutCardReq takeoutCardReq;
    for (int iIndex = 0; iIndex != vecTackOutCard.size(); ++iIndex) {
        takeoutCardReq.add_cards(vecTackOutCard[iIndex]);
    }
    if (!SerializeSendMsg(&takeoutCardReq, robot::MSGID_TAKEOUT_REQ, msgNode)) {
        return false;
    }
    int iDelayTime = 0;
    if (robot::NOTIFY_BASECARD == msgNode->GetMsgId()) {
        iDelayTime = delaySendActiveMsgTime_;
    } else {
        srand((int)time(NULL));
        iDelayTime = rand() % (delaySendPassiveMsgTime_ - 1) + 1;
    }
    msgNode->SetMsgDelaySecond(iDelayTime);
    INFO("=================== SendTakeOutCardReq END =================");
    return true;
}

bool RobotTask::RecvTakeOutCardAck(std::shared_ptr<MsgNode>& msgNode) {
    //message TakeoutCardAck {
    //    required int32 result = 1;
    //}
    INFO("=================== RecvTakeOutCardAck START =================");
    INFO("Message for robot %d.", robotId_);
    if (GAMMING != status_) {
        ERROR("Robot %d doesn't in gamming status, robot status is %s.", robotId_, STATUS_CHAR[status_]);
        return false;
    }
    TakeoutCardAck takeoutCardAck;
    if (!DeserializePbMsg(&takeoutCardAck, msgNode->GetMsg())) {
        ERROR("parse pb message TakeoutCardAck error.");
        return false;
    }
    int result = takeoutCardAck.result();
    if (0 != result) {
        INFO("Get take out card result, take out card failed, result is: %d.", result);
        robot_.RecoveryHandCards();//恢复手牌记录
    } else {
        INFO("Get take out card result, take out card successed.");
    }
    INFO("=================== RecvTakeOutCardAck END =================");
    return true;
}

bool RobotTask::SendTrustCancelReq(std::shared_ptr<MsgNode>& msgNode)     //发送取消托管消息
{
    INFO("=================== SendTrustCancelReq START =================");
    TrustLiftReq trustLiftReq;
    trustLiftReq.set_rev(1);
    if (!SerializeSendMsg(&trustLiftReq, robot::MSGID_TRUST_CANCEL_REQ, msgNode)) {
        return false;
    }
    INFO("=================== SendTrustCancelReq END =================");
    return true;
}

bool RobotTask::RecvTrustCancelAck(std::shared_ptr<MsgNode>& msgNode) {
    // 托管解除
    //message TrustLiftAck {
    //    required int32 result = 1;
    //}
    INFO("=================== RecvTrustCancelAck START =================");
    INFO("Message for robot %d.", robotId_);
    if (GAMMING != status_) {
        ERROR("Robot %d doesn't in gamming status, robot status is %s.", robotId_, STATUS_CHAR[status_]);
        return false;
    }
    TrustLiftAck trustLiftAck;
    if (!DeserializePbMsg(&trustLiftAck, msgNode->GetMsg())) {
        ERROR("parse pb message trustLiftAck error.");
        return false;
    }
    int result = trustLiftAck.result();
    if (0 == result) {
        INFO("robot %d requst cancle trust successed.", robotId_);
    } else {
        ERROR("robot %d requst cancle trust failed.", robotId_);
    }
    INFO("=================== RecvTrustCancelAck END =================");
    return true;
}

bool RobotTask::SendCancelSignUpReq(std::shared_ptr<MsgNode>& msgNode) {
    INFO("=================== SendCancelSignUpReq START =================");
    OrgRoomDdzCancelSignUpReq orgRoomDdzCancelSignUpReq;
    orgRoomDdzCancelSignUpReq.set_matchid(matchId_);
    if (!SerializeSendMsg(&orgRoomDdzCancelSignUpReq, robot::MSGID_DDZ_CANCEL_SIGN_UP_REQ, msgNode)) {
        return false;
    }
    INFO("=================== SendCancelSignUpReq END =================");
    return true;
}

bool RobotTask::RecvCancelSignUpAck(std::shared_ptr<MsgNode>& msgNode) {
    //message OrgRoomDdzCancelSignUpAck {
    //    required int32 result = 1;
    //    message revertGoods {
    //        required string name = 1;   // 消耗物品名称 chips-筹码, vipPoints-竞技点
    //        required int32 count = 2;   // 消耗物品数量
    //    }
    //    // 返还的物品
    //    repeated revertGoods revertList = 2;
    //}
    INFO("=================== RecvCancelSignUpAck START =================");
    INFO("Message for robot %d.", robotId_);
    OrgRoomDdzCancelSignUpAck orgRoomDdzCancelSignUpAck;
    if (!DeserializePbMsg(&orgRoomDdzCancelSignUpAck, msgNode->GetMsg())) {
        ERROR("parse pb message OrgRoomDdzCancelSignUpAck error.");
        return false;
    }

    int result = orgRoomDdzCancelSignUpAck.result();
    if (0 != result) {
        ERROR("Robot %d cancle sign up failed.", robotId_);
        return false;
    } else {
        INFO("Robot %d cancle sign up successed.", robotId_);
    }
    INFO("=================== RecvCancelSignUpAck END =================");
    return true;
}

void DelaySendMsgTimer(int fd, short events, void* arg) {
    MsgNode* oneMsgNode = static_cast<MsgNode*>(arg);
    if (nullptr == oneMsgNode) {
        ERROR("In delay send msg timer, msgnode is null.");
        return;
    }
    RobotTask* eventProcess = static_cast<RobotTask*>(oneMsgNode->GetObjectPoint());
    if (nullptr == eventProcess) {
        ERROR("Get RobotBase point failed.");
        return;
    }
    if (oneMsgNode->GetConn()->BIsBufferExist()) {
        int SendRet = oneMsgNode->GetConn()->AddToWriteBuffer(oneMsgNode->GetMsg());
        DEBUG("Robot %d send delay msg once, msglen is %d, send result is %d.", eventProcess->GetRobotId(), oneMsgNode->GetMsg().length(), SendRet);
    } else {
        ERROR("Connection doesn\'t exist.");
    }
    delete oneMsgNode;
}