#include "AbstractProduct.h"
#include "OGLordRobotAI.h"
#include "Robot.h"
#include "AIUtils.h"
#include "PBGameDDZ.pb.h"
#include "connect.pb.h"
#include "org_room2client.pb.h"
#include "message.pb.h"
#include "log.h"
#include "RobotConfig.h"
#include "confaccess.h"
#include "stringutil.h"

using namespace robot;
using namespace PBGameDDZ;
using namespace YLYQ;
using namespace Protocol;
using namespace connect;
using namespace message;
using namespace org_room2client;
using namespace AIUtils;

AbstractProduct* SimpleFactory::createProduct(int type){
    AbstractProduct* temp = NULL;
    switch(type)
    {
        case robot::NOTIFY_SWITCH_SCENE:
            temp = new GetEnterGameSceneInfo();
            break;
        case robot::NOTIFY_STARTGAME:
            temp = new GetGameStartInfo();
            break;
        case robot::NOTIFY_DEALCARD:
            temp = new InitHardCard();
            break;
        case robot::NOTIFY_CALLSCORE:
            temp = new GetCallScoreInfo();
            break;
        case robot::NOTIFY_SETLORD:
            temp = new GetLordInfo();
            break;
        case robot::NOTIFY_BASECARD:
            temp = new GetBaseCardInfo();
            break;
        case robot::NOTIFY_TAKEOUT:
            temp = new GetTakeOutCardInfo();
            break;
        case robot::NOTIFY_TRUST:
            temp = new GetTrustInfo();
            break;
        case robot::NOTIFY_GAMEOVER:
            temp = new GetGameOverInfo();
            break;
        case robot::MSGID_DDZ_GAME_RESULT_NTF:
            temp = new GetGameResultInfo();
            break;
        case robot::MSGID_DDZ_MATCH_OVER_NTF:
            temp = new GetCompetitionOverInfo();
            break;
        case robot::MSGID_CALLSCORE_ACK:
            temp = new GetCallScoreResultInfo();
            break;
        case robot::MSGID_TAKEOUT_ACK:
            temp = new GetTakeOutCardResultInfo();
            break;
        case robot::MSGID_VERIFY_ACK:
            temp = new GetVerifyAckInfo();
            break;
        case robot::MSGID_INIT_GAME_ACK:
            temp = new GetInitGameAckInfo();
            break;
        case robot::MSGID_DDZ_SIGN_UP_CONDITION_ACK:
            temp = new GetSignUpCondAckInfo();
            break;
        case robot::MSGID_DDZ_SIGN_UP_ACK:
            temp = new GetSignUpAckInfo();
            break;
        case robot::MSGID_TRUST_CANCEL_ACK:
            temp = new GetTrustResultInfo();
            break;
        case robot::MSGID_DDZ_CANCEL_SIGN_UP_ACK:
            temp = new GetCancleSignUpResultInfo();
            break;
        case robot::MSGID_KEEP_ACK:
            temp = new GetKeepPlayInfo();
            break;
        default:
            break;
    }
    return temp;
}


//请求认证回应
bool GetVerifyAckInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    //message VerifyAck {
    //    required int32 result = 1;
    //    optional string gameName = 2;       // 如果正在游戏中，返回游戏名称，客户端进行初始化，自动快速开始
    //}
    INFO("===================GetVerifyAckInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (INIT != myRobot.GetStatus())
    {
        DEBUG("Robot %d doesn't in init status.", robot.GetRobotId());
        return false;
    }
    VerifyAck verifyAck;
    if (!verifyAck.ParseFromString(msg))
    {
        ERROR("parse verify ack pb message error.");
        return false;
    }
    int result = verifyAck.result();
    if (message::SUCCESS == result)
    {
        //认证成功，开始初始化游戏
        if (verifyAck.has_gamename())
        {
            //有gamename字段，说明需要进行断线续玩操作.
            string strName;
            CConfAccess* confAccess = CConfAccess::GetConfInstance();
            confAccess->GetValue("game", "name", strName, "org_ddz_match");
            strName = StringUtil::Trim(strName);
            string strGameName = verifyAck.gamename();

            if (strName == strGameName)
            {
                //该机器人正在本场进行比赛，需要进行断线续玩
                myRobot.SetStatus(KEEPPLAY);
                INFO("Set robot %d to KEEPPLAY status.", robot.GetRobotId());

                //发送断线续玩请求
                //message KeepPlayingReq {
                //    required int32 rev = 1;         // reserved
                //}
                KeepPlayingReq keepPlayingReq;
                keepPlayingReq.set_rev(1);
                if (!keepPlayingReq.SerializeToString(&serializedStr))
                {
                    ERROR("KeepPlayingReq serialize failed.");
                    return false;
                }

                if (!keepPlayingReq.IsInitialized())
                {
                    ERROR("keepPlayingReq isn't a protobuf packet, length is: %d.", serializedStr.length());
                    return false;
                }
                INFO("Robot %d will send a keepPlayingReq.", robot.GetRobotId());
                return true;
            }
            else
            {
                //该机器人正在其它场进行比赛，不用
                myRobot.SetStatus(OTHER);
                INFO("Set robot %d to OTHER status.", robot.GetRobotId());
            }
        }
        else
        {
            myRobot.SetStatus(VERIFIED);
            INFO("Robot %d verify successed, status is: VERIFIED.", robot.GetRobotId());
        }
    }
    else
    {
        ERROR("Robot %d verify failed, result is: %d.", robot.GetRobotId(), result);
    }
    return false;
}

//初始化游戏回应
bool GetInitGameAckInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    //message InitGameAck {
    //    required int32 result = 1;
    //}
    INFO("===================GetInitGameAckInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (VERIFIED != myRobot.GetStatus())
    {
        DEBUG("Robot %d doesn't in verified status.", robot.GetRobotId());
        return false;
    }
    InitGameAck initGameAck;
    if (!initGameAck.ParseFromString(msg))
    {
        ERROR("Parse InitGameAck protobuf error.");
        return false;
    }
    int result = initGameAck.result();
    if (message::SUCCESS == result)
    {
        //初始化成功，开始查询报名条件
        myRobot.SetStatus(INITGAME);
        INFO("Robot %d Game Init successed, robot status is: INITGAME.", robot.GetRobotId());
    }
    else
    {
        DEBUG("Robot %d Game Init failed, result is: %d.", robot.GetRobotId(), result);
    }
    return false;
}

//查询报名条件
bool GetSignUpCondAckInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
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
    INFO("===================GetSignUpCondAckInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (INITGAME != myRobot.GetStatus())
    {
        DEBUG("Robot %d doesn't in game init status.", robot.GetRobotId());
        return false;
    }
    OrgRoomDdzSignUpConditionAck orgRoomDdzSignUpConditionAck;
    if (!orgRoomDdzSignUpConditionAck.ParseFromString(msg))
    {
        ERROR("Parse OrgRoomDdzSignUpConditionAck protobuf error.");
        return false;
    }
    int result = orgRoomDdzSignUpConditionAck.result();
    if (message::SUCCESS != result)
    {
        DEBUG("Robot %d sign up condition failed, result is: %d.", robot.GetRobotId(), result);
        return false;
    }
    if (!orgRoomDdzSignUpConditionAck.has_limit())
    {
        INFO("Robot %d can sign up for free, status is: CANSINGUP.", robot.GetRobotId());
        myRobot.SetCost(0);
        myRobot.SetStatus(CANSINGUP);
    }
    else
    {
        bool cond = orgRoomDdzSignUpConditionAck.limit().enable();
        if (cond)
        {
            INFO("Robot %d can sign up, status is: CANSINGUP.", robot.GetRobotId());
            myRobot.SetStatus(CANSINGUP);
            int costSize = orgRoomDdzSignUpConditionAck.costlist_size();
            int index = 0;
            for (index = 0; index < costSize; ++index)
            {
                if (orgRoomDdzSignUpConditionAck.costlist(index).enable())
                {
                    myRobot.SetCost(orgRoomDdzSignUpConditionAck.costlist(index).id());
                    INFO("Found enable cost in costlist, id is: %d.", myRobot.GetCost());
                    if (orgRoomDdzSignUpConditionAck.costlist(index).signed_())
                    {
                        INFO("Robit %d has already sign up, status is: SIGNUPED.", robot.GetRobotId());
                        myRobot.SetStatus(SIGNUPED);
                    }
                    break;
                }
            }
            if (costSize == index)
            {
                ERROR("Doesn't found enable cost in costlist.");
            }
        }
        else
        {
            DEBUG("Robot %d can't sign up.", robot.GetRobotId());
        }
    }
    return false;
}

//报名结果回应
bool GetSignUpAckInfo::operation( Robot & myRobot, const string & msg, string& serializedStr ){
    //message OrgRoomDdzSignUpAck {
    //    required int32 result = 1;
    //    message CostGoods {
    //        required string name = 1;   // 消耗物品名称 chips-筹码, vipPoints-竞技点
    //        required int32 count = 2;   // 消耗物品数量
    //    }
    //    repeated CostGoods costList = 2;
    //    optional int32 userCount = 3;   // 已报名人数
    //}
    INFO("===================GetSignUpAckInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (CANSINGUP != myRobot.GetStatus())
    {
        DEBUG("Robot %d doesn't in can sign up status.", robot.GetRobotId());
        return false;
    }
    OrgRoomDdzSignUpAck orgRoomDdzSignUpAck;
    if (!orgRoomDdzSignUpAck.ParseFromString(msg))
    {
        ERROR("Parse OrgRoomDdzSignUpAck protobuf msg error.");
        return false;
    }
    int result = orgRoomDdzSignUpAck.result();
    if (message::SUCCESS == result)
    {
        //报名成功，开始等待游戏
        myRobot.SetStatus(SIGNUPED);
        INFO("Robot %d sign up succssed, robot status is: SIGNUPED.", robot.GetRobotId(), myRobot.GetStatus());
    }
    else
    {
        if (508 == result)
        {
            INFO("Robit %d has already sign up.", robot.GetRobotId());
            myRobot.SetStatus(SIGNUPED);
        }
        else
        {
            DEBUG("Robot %d sign up failed, result is: %d.", robot.GetRobotId(), result);
        }
    }
    return false;
}

/*++++++++++++++++++游戏阶段 开始++++++++++++++++++++++*/
//进入游戏场景
bool GetEnterGameSceneInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    // 进入游戏场景通知
    //message GameSwitchSceneNtf {
    //    required string gameName = 1;
    //    required bool isMatch    = 2;     // 是否为游戏场，true为是
    //}
    INFO("===================GetEnterGameSceneInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    GameSwitchSceneNtf gameSwitchSceneNtf;
    if (!gameSwitchSceneNtf.ParseFromString(msg))
    {
        ERROR("Parse GameSwitchSceneNtf protobuf msg error.");
        return false;
    }
    myRobot.SetStatus(GAMMING);
    robot.RbtResetData();
    INFO("Set robot %d to GAMMING status.", robot.GetRobotId());
    return false;
}

//游戏开始
bool GetGameStartInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    // 游戏开始
    //message GameStartNtf {
    //    required string gameName = 1;       // 游戏服务通道号
    //    required int32 basicScore   = 2;    // 底分
    //    repeated UserInfo userinfo  = 3;    // 用户信息
    //    optional MatchInfo matchInfo = 4;   // 如果是游戏场，该字段不用
    //}
    INFO("===================GetGameStartInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        ERROR("Robot %d doesn't in gamming status.", robot.GetRobotId());
        return false;
    }
    GameStartNtf gameStartNtf;
    if (!gameStartNtf.ParseFromString(msg))
    {
        ERROR("parse pb message GameStartNtf error.");
        return false;
    }
    int index = 0;
    int iUserNum = gameStartNtf.userinfo_size();
    int robotId = robot.GetRobotId();
    for (index = 0; index < iUserNum; ++index)//寻找自己的座位号:0-2
    {
        int netId = ::atoi(gameStartNtf.userinfo(index).username().c_str());
        //DEBUG("total userNum: %d, robot id: %d, net id: %d.", iUserNum, robotId, netId);
        if (netId == robotId)
        {
            break;
        }
    }

    if (iUserNum == index)
    {
        ERROR("Doesn't find robot name.");
    }
    else
    {
        robot.SetAiSeat(index);
        INFO("Set robot %d seat successed, seat is: %d.", robotId, index);
    }
    return false;
}


//发牌
bool InitHardCard::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    // 发牌
    //message DealCardNtf {
    //    required int32 headerseat = 1;          // 第一个叫分座位号
    //    repeated HandCardList cards = 2;        // 玩家手牌
    //}
    INFO("===================InitHardCard START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        ERROR("Robot %d doesn't in gamming status.", robot.GetRobotId());
        return false;
    }
    DealCardNtf dealCardNtf;
    if (!dealCardNtf.ParseFromString(msg))
    {
        ERROR("parse pb message DealCardNtf error.");
        return false;
    }
    int aiSeat = robot.GetAiSeat();
    //DEBUG("Robot %d's seat is: %d.", robot.GetRobotId(), aiSeat);
    if (-1 == aiSeat)
    {
        ERROR("Not init seat info.");
        return false;
    }
    int hearderSeat = dealCardNtf.headerseat();
    int cardsSize = dealCardNtf.cards(aiSeat).cards_size();//查看自己的那手牌信息

    vector<int> vecHandCard;
    for (int index = 0; index < cardsSize; ++index)//获取自己的那手牌
    {
        vecHandCard.push_back(dealCardNtf.cards(aiSeat).cards(index));
    }
    robot.RbtInInitCard(aiSeat, vecHandCard);
    DEBUG("Init hand card successed, hand card info is: ");
    printCardInfo(vecHandCard);

    //判断自己是不是第一个叫分座位号
    if (hearderSeat == aiSeat)
    {
        int myScore = 0;
        robot.RbtOutGetCallScore(myScore);
        CallScoreReq callScoreReq;
        callScoreReq.set_score(myScore);
        INFO("hearderSeat is: %d, I'm is the first to call score, my score is: %d.", hearderSeat, myScore);
        if (!callScoreReq.SerializeToString(&serializedStr))
        {
            ERROR("callScoreReq serialize failed.");
            return false;
        }

        if (!callScoreReq.IsInitialized())
        {
            ERROR("CallScoreReq isn't a protobuf packet, length is: %d.", serializedStr.length());
            return false;
        }

        return true;
    }
    return false;
}

//收到叫分通知
bool GetCallScoreInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    // 叫分
    //message UserCallScoreNtf {
    //    required int32 seatno = 1;                  // 座位号
    //    required int32 seatnext = 2[default=-1];    // 下一个叫分座位，-1叫分结束
    //    required int32 score = 3;                   // 叫分值(1/2/3), 0-不叫
    //}
    INFO("===================GetCallScoreInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        ERROR("Robot %d doesn't in gamming status.", robot.GetRobotId());
        return false;
    }
    UserCallScoreNtf userCallScoreNtf;
    if (!userCallScoreNtf.ParseFromString(msg))
    {
        ERROR("parse pb message UserCallScoreNtf error.");
        return false;
    }
    int seatNo = userCallScoreNtf.seatno();
    int seatNext = userCallScoreNtf.seatnext();
    int score = userCallScoreNtf.score();
    int aiSeat = robot.GetAiSeat();

    //DEBUG("seatno: %d, seatnext: %d, score: %d, my seat: %d.", seatNo, seatNext, score, aiSeat);
    if (-1 == seatNext)
    {
        //停止叫分
        return false;
    }
    if (seatNext != aiSeat)
    {
        //没轮到自己，不叫
        robot.RbtInCallScore(seatNo, score);
        return false;
    }
    else
    {
        int myScore = 0;
        robot.RbtOutGetCallScore(myScore);
        int curScore = robot.GetCurScore();
        DEBUG("My score is: %d, current score is: %d.", myScore, curScore);

        CallScoreReq callScoreReq;
        if (score >= myScore)
        {
            //目前分数比自己的大，不叫
            callScoreReq.set_score(0);
            DEBUG("Doesn't choose call score, curScore is: %d.", curScore);
        }
        else if (score == 0 && robot.GetCurScore() >= myScore)
        {
            //上一个用户没叫，且上上一个用户叫的分比自己的高，不叫
            callScoreReq.set_score(0);
            DEBUG("Doesn't choose call score, curScore is: %d.", curScore);
        }
        else
        {
            //叫分
            callScoreReq.set_score(myScore);
            INFO("Robot %d choose to call score, score is: %d.", robot.GetRobotId(), myScore);
        }

        if (!callScoreReq.SerializeToString(&serializedStr))
        {
            ERROR("callScoreReq serialize failed.");
            return false;
        }
        if (!callScoreReq.IsInitialized())
        {
            ERROR("CallScoreReq isn't a protobuf packet, length is: %d.", serializedStr.length());
            return false;
        }
        return true;
    }
}

//收到地主信息
bool GetLordInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    // 地主确定
    //message LordSetNtf {
    //    required int32 seatlord = 1;    // 地主座位号
    //    required int32 callscore = 2;   // 地主叫分
    //}
    INFO("===================GetLordInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        ERROR("Robot %d doesn't in gamming status.", robot.GetRobotId());
        return false;
    }
    LordSetNtf lordSetNtf;
    if (!lordSetNtf.ParseFromString(msg))
    {
        ERROR("parse pb message LordSetNtf error.");
        return false;
    }
    int seatLord = lordSetNtf.seatlord();
    robot.SetLordSeat(seatLord);
    INFO("Lord seat is %d.", seatLord);
    return false;
}

//收到底牌
bool GetBaseCardInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    // 发底牌
    //message SendBaseCardNtf {
    //    repeated int32 basecards = 1;   // 底牌数据
    //}
    INFO("===================GetBaseCardInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        ERROR("Robot %d doesn't in gamming status.", robot.GetRobotId());
        return false;
    }
    SendBaseCardNtf sendBaseCardNtf;
    if (!sendBaseCardNtf.ParseFromString(msg))
    {
        ERROR("parse pb message SendBaseCardNtf error.");
        return false;
    }
    int baseCardSize = sendBaseCardNtf.basecards_size();
    int seatLord = robot.GetLordSeat();
    vector<int> vecBaseCard;
    for (int index = 0; index < baseCardSize; ++index)
    {
        vecBaseCard.push_back(sendBaseCardNtf.basecards(index));
    }
    INFO("Base card info:");
    printCardInfo(vecBaseCard);
    robot.RbtInSetLord(seatLord, vecBaseCard);

    //判断自己是不是地主
    if (robot.GetLordSeat() == robot.GetAiSeat())
    {
        INFO("I am lord, so it's my turn to take out first card.");
        vector<int> vecTackOutCard;
        robot.RbtOutGetTakeOutCard(vecTackOutCard);
        printCardInfo(vecTackOutCard);

        TakeoutCardReq takeoutCardReq;
        for (int iIndex = 0; iIndex != vecTackOutCard.size(); ++iIndex)
        {
            takeoutCardReq.add_cards(vecTackOutCard[iIndex]);
        }
        if (!takeoutCardReq.SerializeToString(&serializedStr))
        {
            ERROR("Take cout card req serialize failed!");
        }
        if (!takeoutCardReq.IsInitialized())
        {
            ERROR("takeoutCardReq isn't a protobuf packet, length is: %d.", serializedStr.length());
            return false;
        }
        return true;
    }
    return false;
}

//收到出牌通知
bool GetTakeOutCardInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    // 出牌
    //message TakeoutCardNtf {
    //    required int32 seatno = 1;      // 出牌座位号
    //    required int32 seatnext = 2;    // 下一个出牌座位
    //    repeated int32 cards = 3;       // 出牌数据
    //    required int32 cardtype = 4;    // 类型
    //    required int32 multiple = 5;    // 当前倍数(炸弹产生的倍数)
    //}
    INFO("===================GetTakeOutCardInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        ERROR("Robot %d doesn't in gamming status.", robot.GetRobotId());
        return false;
    }
    TakeoutCardNtf takeoutCardNtf;
    if (!takeoutCardNtf.ParseFromString(msg))
    {
        ERROR("parse pb message TakeoutCardNtf error.");
        return false;
    }
    int seatno = takeoutCardNtf.seatno();
    int seatnext = takeoutCardNtf.seatnext();
    int cardsNum = takeoutCardNtf.cards_size();
    int aiSeat = robot.GetAiSeat();
    vector<int> vecOppTackOutCard;
    for (int index = 0; index < cardsNum; ++index)
    {
        vecOppTackOutCard.push_back(takeoutCardNtf.cards(index));
    }
    DEBUG("Current take out card info is:");
    printCardInfo(vecOppTackOutCard);
    robot.RbtInTakeOutCard(seatno, vecOppTackOutCard);

    if (seatnext == aiSeat)
    {
        //出牌
        DEBUG("It's my turn to take out card, my take out cards is:");
        vector<int> vecTackOutCard;
        robot.RbtOutGetTakeOutCard(vecTackOutCard);
        printCardInfo(vecTackOutCard);

        TakeoutCardReq takeoutCardReq;
        for (int iIndex = 0; iIndex != vecTackOutCard.size(); ++iIndex)
        {
            takeoutCardReq.add_cards(vecTackOutCard[iIndex]);
        }
        if (!takeoutCardReq.SerializeToString(&serializedStr))
        {
            ERROR("Take cout card req serialize failed!");
            return false;
        }

        if (!takeoutCardReq.IsInitialized())
        {
            ERROR("takeoutCardReq isn't a protobuf packet, length is: %d.", serializedStr.length());
            return false;
        }

        return true;
    }
    else
    {
        INFO("It not my turn to take out card.");
    }
    return false;
}

//托管通知
bool GetTrustInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    // 托管
    //message TrustNtf {
    //    required int32 seatno = 1;
    //}
    INFO("===================GetTrustInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        ERROR("Robot %d doesn't in gamming status.", robot.GetRobotId());
        return false;
    }
    TrustNtf trustNtf;
    if (!trustNtf.ParseFromString(msg))
    {
        ERROR("parse pb message TrustNtf error.");
        return false;
    }
    int seatNo = trustNtf.seatno();
    //判断是否是自己被进入托管
    int mySeatNo = robot.GetAiSeat();
    if (mySeatNo == seatNo)
    {
        //说明上一局出牌错误，需要先将上一局出得牌恢复，然后发送取消托管请求
        robot.RecoveryHandCards();//恢复手牌记录

        // 托管解除
        //message TrustLiftReq {
        //    required int32 rev = 1;         // reserved
        //}
        TrustLiftReq trustLiftReq;
        trustLiftReq.set_rev(1);
        if (!trustLiftReq.SerializeToString(&serializedStr))
        {
            ERROR("TrustLiftReq serialize failed!");
            return false;
        }
        if (!trustLiftReq.IsInitialized())
        {
            ERROR("TrustLiftReq is not a legal protobuf packect.");
            return false;
        }
        INFO("robot %d send a TrustLiftReq.", robot.GetRobotId());
        return true;
    }
    return false;
}

//游戏结束
bool GetGameOverInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    // 游戏结束通知
    //message GameOverNtf {
    //    required int32 reason = 1[default=2];   // 结束原因：1-强制结束，2-达到最大游戏盘数
    //}
    INFO("===================GetGameOverInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        INFO("Robot %d doesn't in gamming status, will set it to game init status.", robot.GetRobotId());
    }
    GameOverNtf gameOverNtf;
    if (!gameOverNtf.ParseFromString(msg))
    {
        ERROR("parse pb message GameOverNtf error.");
        return false;
    }
    int reason = gameOverNtf.reason();
    robot.RbtResetData();
    myRobot.SetStatus(INITGAME);
    INFO("Receved game over notify, reset robot to sign up condation.");
    return false;
}

//游戏结果
bool GetGameResultInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
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
    INFO("===================GetGameResultInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        INFO("Robot %d doesn't in gamming status, and will set it to gamming status.", robot.GetRobotId());
        myRobot.SetStatus(GAMMING);
    }
    OrgRoomDdzGameResultNtf orgRoomDdzGameResultNtf;
    if (!orgRoomDdzGameResultNtf.ParseFromString(msg))
    {
        ERROR("parse pb message OrgRoomDdzGameResultNtf error.");
        return false;
    }
    robot.RbtResetData();
    INFO("Receved game result notify, robot waitting for next game.");
    return false;
}

//比赛结束
bool GetCompetitionOverInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
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
    INFO("===================GetCompetitionOverInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        INFO("Robot %d doesn't in gamming status, and will set it to game init status.", robot.GetRobotId());
    }
    OrgRoomDdzMatchOverNtf orgRoomDdzMatchOverNtf;
    robot.RbtResetData();
    myRobot.SetStatus(INITGAME);
    INFO("Receved competition notify, reset robot to sign up condation.");
    return false;
}

//获取叫分结果
bool GetCallScoreResultInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    //message CallScoreAck {
    //    required int32 result = 1;
    //}
    INFO("===================GetCallScoreResultInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        ERROR("Robot %d doesn't in gamming status.", robot.GetRobotId());
        return false;
    }
    CallScoreAck callScoreAck;
    if (!callScoreAck.ParseFromString(msg))
    {
        ERROR("parse pb message CallScoreAck error.");
        return false;
    }
    int result = callScoreAck.result();

    if (0 != result)
    {
        ERROR("Get call score result, call score failed, result is: %d.", result);
    }
    else
    {
        INFO("Get call score result, call score successed.");
    }
    return false;
}

//获取出牌结果
bool GetTakeOutCardResultInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    //message TakeoutCardAck {
    //    required int32 result = 1;
    //}
    INFO("===================GetTakeOutCardResultInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        ERROR("Robot %d doesn't in gamming status.", robot.GetRobotId());
        return false;
    }
    TakeoutCardAck takeoutCardAck;
    if (!takeoutCardAck.ParseFromString(msg))
    {
        ERROR("parse pb message TakeoutCardAck error.");
        return false;
    }
    int result = takeoutCardAck.result();
    if (0 != result)
    {
        ERROR("Get take out card result, take out card failed, result is: %d.", result);
        robot.RecoveryHandCards();//恢复手牌记录
    }
    else
    {
        INFO("Get take out card result, take out card successed.");
    }
    return false;
}

//获取解除托管结果
bool GetTrustResultInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    // 托管解除
    //message TrustLiftAck {
    //    required int32 result = 1;
    //}
    INFO("===================GetTrustResultInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (GAMMING != myRobot.GetStatus())
    {
        ERROR("Robot %d doesn't in gamming status.", robot.GetRobotId());
        return false;
    }
    TrustLiftAck trustLiftAck;
    if (!trustLiftAck.ParseFromString(msg))
    {
        ERROR("parse pb message TrustLiftAck error.");
        return false;
    }
    int result = trustLiftAck.result();
    if (0 == result)
    {
        INFO("robot %d requst cancle trust successed.", robot.GetRobotId());
    }
    else
    {
        ERROR("robot %d requst cancle trust failed.", robot.GetRobotId());
    }
    return false;
}

//获取取消报名结果
bool GetCancleSignUpResultInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    //message OrgRoomDdzCancelSignUpAck {
    //    required int32 result = 1;
    //    message revertGoods {
    //        required string name = 1;   // 消耗物品名称 chips-筹码, vipPoints-竞技点
    //        required int32 count = 2;   // 消耗物品数量
    //    }
    //    // 返还的物品
    //    repeated revertGoods revertList = 2;
    //}
    INFO("===================GetCancleSignUpResultInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    OrgRoomDdzCancelSignUpAck orgRoomDdzCancelSignUpAck;
    if (!orgRoomDdzCancelSignUpAck.ParseFromString(msg))
    {
        ERROR("parse pb message OrgRoomDdzCancelSignUpAck error.");
    }
    int result = orgRoomDdzCancelSignUpAck.result();
    if (0 == result)
    {
        INFO("Robot %d cancle sign up successed.", robot.GetRobotId());
    }
    else
    {
        ERROR("Robot %d cancle sign up failed.", robot.GetRobotId());
    }
    return false;
}

//获取断线续玩应答
bool GetKeepPlayInfo::operation( Robot& myRobot, const string& msg, string& serializedStr ){
    // 断线续玩
    //message KeepPlayingAck {
    //    required int32 result = 1;
    //    // 游戏实时信息
    //    message GameInfo {
    //        required int32 status = 1;      // 游戏当前状态
    //        required int32 seatlord = 2;    // 地主的座位号
    //        required int32 seatactive = 3;  // 当前活动玩家座位号
    //        required int32 multiple = 4;    // 当前倍数, 基本倍数, 如果有踢的需要各自累加倍数
    //        required int32 maxcallscore = 5;// 当前最大叫分
    //        repeated int32 basecards = 6;   // 底牌数据
    //    }

    //    // 玩家动态信息
    //    message PlayerInfo {
    //        required bool trust = 1;          // 托管状态
    //        required int32 trustsurplus = 2;  // 解除托管剩余次数
    //        required int32 callscore = 3;     // 叫分值
    //        repeated int32 cards = 4;         // 手牌内容
    //        repeated int32 lastcards = 5;     // 最后一手牌
    //        required UserInfo detailinfo = 6; // 用户详细信息
    //    }

    //    required GameInfo gameinfo = 2;     // 游戏信息
    //    repeated PlayerInfo playerinfo = 3; // 玩家信息
    //    required int32 ready = 4;           // 准备超时时间
    //    required int32 callscore = 5;       // 叫分超时时间
    //    required int32 takeout = 6;         // 出牌超时时间
    //    required int32 settle = 7;          // 结算框显示时间（超时后自动准备）
    //    required string gameChannel = 8;    // 游戏服务通道号
    //    required int32 basicScore   = 9;    // 底分
    //    optional MatchInfo matchInfo = 10;  // 如果是游戏场，该字段不用
    //}
    INFO("===================GetKeepPlayInfo START=================");
    OGLordRobotAI& robot = myRobot.GetRobot();
    if (KEEPPLAY != myRobot.GetStatus())
    {
        ERROR("Robot %d doesn't in keep play status.", robot.GetRobotId());
        return false;
    }
    KeepPlayingAck keepPlayingAck;
    keepPlayingAck.ParseFromString(msg);
    int result = keepPlayingAck.result();
    if (0 != result)
    {
        ERROR("Get keep play req failed, reslut code is: %d.", result);
        myRobot.SetStatus(INIT);
        return false;
    }
    int index = 0;
    int robotId = robot.GetRobotId();
    int iPlayerInfoSize = keepPlayingAck.playerinfo_size();
    for (index = 0; index < iPlayerInfoSize; ++index)
    {
        int iNetId = ::atoi(keepPlayingAck.playerinfo(index).detailinfo().username().c_str());
        INFO("total userNum: %d, robot id: %d, net id: %d.", iPlayerInfoSize, robotId, iNetId);
        if (iNetId == robotId)
        {
            break;
        }
    }
    if (iPlayerInfoSize == index)
    {
        ERROR("Doesn't find robot name.");
        return false;
    }
    else
    {
        robot.SetAiSeat(index);
        INFO("Set robot seat successed, seat is: %d.", index);
    }
    int iAiSeat = robot.GetAiSeat();
    int iLordSeat = keepPlayingAck.gameinfo().seatlord();
    vector<int> vecHandCards;
    vector<int> vecBaseCards;
    int iBaseCardsSize = keepPlayingAck.gameinfo().basecards_size();
    for (index = 0; index < iBaseCardsSize; ++index)
    {
        vecBaseCards.push_back(keepPlayingAck.gameinfo().basecards(index));
    }
    INFO("Robot %d current base cards is:");
    printCardInfo(vecBaseCards);

    int iHandsCardsSize = keepPlayingAck.playerinfo(iAiSeat).cards_size();
    for (index = 0; index < iHandsCardsSize; ++index)
    {
        vecHandCards.push_back(keepPlayingAck.playerinfo(iAiSeat).cards(index));
    }
    INFO("Robot %d current hands cards is:");
    printCardInfo(vecHandCards);

    //注意先后顺序
    robot.RbtResetData();//重置
    robot.RbtInSetSeat(iAiSeat, iLordSeat);//设置座位信息
    robot.RbtInSetCard(vecHandCards, vecBaseCards);//设置手牌信息
    myRobot.SetStatus(GAMMING);

    //处于托管状态，需发送取消托管请求
    TrustLiftReq trustLiftReq;
    trustLiftReq.set_rev(1);
    if (!trustLiftReq.SerializeToString(&serializedStr))
    {
        ERROR("TrustLiftReq serialize failed!");
        return false;
    }
    if (!trustLiftReq.IsInitialized())
    {
        ERROR("TrustLiftReq is not a legal protobuf packect.");
        return false;
    }
    INFO("robot %d send a TrustLiftReq.", robot.GetRobotId());
    return true;
}

/*++++++++++++++++++游戏阶段 结束++++++++++++++++++++++*/
