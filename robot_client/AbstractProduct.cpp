#include "AbstractProduct.h"
#include "OGLordRobotAI.h"
#include "AIUtils.h"
#include "PBGameDDZ.pb.h"
#include "connect.pb.h"
#include "org_room2client.pb.h"
#include "message.pb.h"
#include "log.h"
#include <sstream>

using namespace PBGameDDZ;
using namespace YLYQ;
using namespace Protocol;
using namespace connect;
using namespace message;
using namespace org_room2client;
using namespace AIUtils;

void sleep_ms( unsigned int secs )
{
    struct timeval tval;
    tval.tv_sec = secs / 1000;
    tval.tv_usec = (secs * 1000) % 1000000;
    select(0, NULL ,NULL, NULL, &tval);
}

//请求认证回应
bool GetVerifyAckInfo::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
    //message VerifyAck {
    //    required int32 result = 1;
    //    optional string gameName = 2;       // 如果正在游戏中，返回游戏名称，客户端进行初始化，自动快速开始
    //}
    VerifyAck verifyAck;
    if (!verifyAck.ParseFromString(msg))
    {
        ERROR("parse verify ack pb message error.");
        //cout << "parse verify ack pb message error." << endl;
        return false;
    }
    int result = verifyAck.result();
    if (message::SUCCESS == result)
    {
        //认证成功，开始初始化游戏
        robot.SetStatus(VERIFIED);
        DEBUG("Robot %d verify successed, result is: %d.", robot.GetRobotId(), robot.GetStatus());
    }
    else
    {
        DEBUG("Robot %d verify failed, result is: %d.", robot.GetRobotId(), result);
    }
    return false;
}

//初始化游戏回应
bool GetInitGameAckInfo::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
    //message InitGameAck {
    //    required int32 result = 1;
    //}
    InitGameAck initGameAck;
    initGameAck.ParseFromString(msg);
    int result = initGameAck.result();
    if (message::SUCCESS == result)
    {
        //初始化成功，开始查询报名条件
        robot.SetStatus(INITGAME);
        DEBUG("Robot %d Game Init successed, robot status is: %d.", robot.GetRobotId(), robot.GetStatus());
    }
    else
    {
        DEBUG("Robot %d Game Init failed, result is: %d.", robot.GetRobotId(), result);
    }
    return false;
}

//查询报名条件
bool GetSignUpCondAckInfo::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
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
    OrgRoomDdzSignUpConditionAck orgRoomDdzSignUpConditionAck;
    orgRoomDdzSignUpConditionAck.ParseFromString(msg);
    int result = orgRoomDdzSignUpConditionAck.result();
    if (message::SUCCESS != result)
    {
        DEBUG("Robot %d sign up condition failed, result is: %d.", robot.GetRobotId(), result);
        return false;
    }
    if (!orgRoomDdzSignUpConditionAck.has_limit())
    {
        DEBUG("Robot %d can sign up for free.", robot.GetRobotId());
        robot.SetCost(0);
        robot.SetStatus(CANSINGUP);
    }
    else
    {
        bool cond = orgRoomDdzSignUpConditionAck.limit().enable();
        if (cond)
        {
            DEBUG("Robot %d can sign up.", robot.GetRobotId());
            robot.SetStatus(CANSINGUP);
            int costSize = orgRoomDdzSignUpConditionAck.costlist_size();
            int index = 0;
            for (index = 0; index < costSize; ++index)
            {
                if (orgRoomDdzSignUpConditionAck.costlist(index).enable())
                {
                    robot.SetCost(orgRoomDdzSignUpConditionAck.costlist(index).id());
                    DEBUG("Found enable cost in costlist, id is: %d.", robot.GetCost())
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
            DEBUG("Robot %d cann't sign up.", robot.GetRobotId());
        }
    }
    return false;
}

//报名结果回应
bool GetSignUpAckInfo::operation( OGLordRobotAI & robot, const string & msg, string& serializedStr ){
    //message OrgRoomDdzSignUpAck {
    //    required int32 result = 1;
    //    message CostGoods {
    //        required string name = 1;   // 消耗物品名称 chips-筹码, vipPoints-竞技点
    //        required int32 count = 2;   // 消耗物品数量
    //    }
    //    repeated CostGoods costList = 2;
    //    optional int32 userCount = 3;   // 已报名人数
    //}
    OrgRoomDdzSignUpAck orgRoomDdzSignUpAck;
    orgRoomDdzSignUpAck.ParseFromString(msg);
    int result = orgRoomDdzSignUpAck.result();
    if (message::SUCCESS == result)
    {
        //报名成功，开始等待游戏
        robot.SetStatus(SIGNUPED);
        DEBUG("Robot %d sign up succssed, robot status is: %d.", robot.GetRobotId(), robot.GetStatus());
    }
    else
    {
        DEBUG("Robot %d sign up failed, result is: %d.", robot.GetRobotId(), result);
    }
    return false;
}

/*++++++++++++++++++游戏阶段 开始++++++++++++++++++++++*/
//游戏开始
bool GetGameStartInfo::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
    // 游戏开始
    //message GameStartNtf {
    //    required string gameName = 1;       // 游戏服务通道号
    //    required int32 basicScore   = 2;    // 底分
    //    repeated UserInfo userinfo  = 3;    // 用户信息
    //    optional MatchInfo matchInfo = 4;   // 如果是游戏场，该字段不用
    //}
    robot.SetStatus(GAMMING);
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
        DEBUG("total userNum: %d, robot id: %d, net id: %d.", iUserNum, robotId, netId);
        if (netId == robotId)
        {
            break;
         }
    }

    if (iUserNum == index)
    {
        DEBUG("Doesn't find robot name.");
    }
    else
    {
        robot.SetAiSeat(index);
        DEBUG("Set robot seat successed, seat is: %d.", index);
    }
    return false;
}


//发牌
bool InitHardCard::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
    // 发牌
    //message DealCardNtf {
    //    required int32 headerseat = 1;          // 第一个叫分座位号
    //    repeated HandCardList cards = 2;        // 玩家手牌
    //}
    robot.SetStatus(GAMMING);
    DealCardNtf dealCardNtf;
    if (!dealCardNtf.ParseFromString(msg))
    {
        ERROR("parse pb message DealCardNtf error.");
        return false;
    }
    int aiSeat = robot.GetAiSeat();
    DEBUG("my seat is: %d.", aiSeat);
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
        DEBUG("hearderSeat is: %d, I'm is the first to call score, my score is: %d.", hearderSeat, myScore);
        if (!callScoreReq.SerializeToString(&serializedStr))
        {
            ERROR("callScoreReq serialize failed.");
        }
        else
        {
            DEBUG("callScoreReq serialize successe.");
        }
        return true;
    }
    return false;
}

//收到叫分通知
bool GetCallScoreInfo::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
    // 叫分
    //message UserCallScoreNtf {
    //    required int32 seatno = 1;                  // 座位号
    //    required int32 seatnext = 2[default=-1];    // 下一个叫分座位，-1叫分结束
    //    required int32 score = 3;                   // 叫分值(1/2/3), 0-不叫
    //}
    robot.SetStatus(GAMMING);
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

    DEBUG("seatno: %d, seatnext: %d, score: %d, my seat: %d.", seatNo, seatNext, score, aiSeat);
    if (-1 == seatNext)
    {
        //停止叫分
        return false;
    }
    if (seatNext != aiSeat)
    {
        //没轮到自己，不叫
        robot.RbtInCallScore(seatNo, score);
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
            DEBUG("Choose to call score, score is: %d.", myScore);
        }
        if (!callScoreReq.SerializeToString(&serializedStr))
        {
            ERROR("callScoreReq serialize failed.");
        }
        else
        {
            DEBUG("callScoreReq serialize successe.");
        }
        return true;
    }
    return false;
}

//收到地主信息
bool GetLordInfo::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
    // 地主确定
    //message LordSetNtf {
    //    required int32 seatlord = 1;    // 地主座位号
    //    required int32 callscore = 2;   // 地主叫分
    //}
    robot.SetStatus(GAMMING);
    LordSetNtf lordSetNtf;
    if (!lordSetNtf.ParseFromString(msg))
    {
        ERROR("parse pb message LordSetNtf error.");
        return false;
    }
    int seatLord = lordSetNtf.seatlord();
    robot.SetLordSeat(seatLord);
    DEBUG("Set lord info %d over.", seatLord);
    return false;
}

//收到底牌
bool GetBaseCardInfo::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
    // 发底牌
    //message SendBaseCardNtf {
    //    repeated int32 basecards = 1;   // 底牌数据
    //}
    robot.SetStatus(GAMMING);
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
    DEBUG("Base card info:");
    printCardInfo(vecBaseCard);
    robot.RbtInSetLord(seatLord, vecBaseCard);
    DEBUG("Set base card over.");

    //判断自己是不是地主
    if (robot.GetLordSeat() == robot.GetAiSeat())
    {
        DEBUG("I am lord, so it's my turn to take out first card.");
        vector<int> vecTackOutCard;
        robot.RbtOutGetTakeOutCard(vecTackOutCard);
        DEBUG("My (lord) take out cards is:");
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
        else
        {
            DEBUG("Take out cards over, serialized string size is: %d.", serializedStr.length());
        }
        return true;
    }
    return false;
}

//收到出牌通知
bool GetTakeOutCardInfo::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
    // 出牌
    //message TakeoutCardNtf {
    //    required int32 seatno = 1;      // 出牌座位号
    //    required int32 seatnext = 2;    // 下一个出牌座位
    //    repeated int32 cards = 3;       // 出牌数据
    //    required int32 cardtype = 4;    // 类型
    //    required int32 multiple = 5;    // 当前倍数(炸弹产生的倍数)
    //}
    robot.SetStatus(GAMMING);
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
    DEBUG("seatno: %d, seatnext: %d, cards num: %d, my seat: %d.", seatno, seatnext, cardsNum, aiSeat);
    vector<int> vecOppTackOutCard;
    for (int index = 0; index < cardsNum; ++index)
    {
        vecOppTackOutCard.push_back(takeoutCardNtf.cards(index));
    }
    DEBUG("Current card info is:");
    printCardInfo(vecOppTackOutCard);
    robot.RbtInTakeOutCard(seatno, vecOppTackOutCard);


    if (seatnext == aiSeat)
    {
        //出牌
        DEBUG("It's my turn to take out card.");
        vector<int> vecTackOutCard;
        robot.RbtOutGetTakeOutCard(vecTackOutCard);
        DEBUG("My take out cards is:");
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
        else
        {
            DEBUG("Take out cards over, serialized string size is: %d.", serializedStr.length());
        }
        return true;
    }
    else
    {
        DEBUG("It not my turn to take out card.");
    }
    return false;
}

//游戏结束
bool GetGameOverInfo::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
    // 游戏结束通知
    //message GameOverNtf {
    //    required int32 reason = 1[default=2];   // 结束原因：1-强制结束，2-达到最大游戏盘数
    //}
    GameOverNtf gameOverNtf;
    if (!gameOverNtf.ParseFromString(msg))
    {
        ERROR("parse pb message GameOverNtf error.");
        return false;
    }
    int reason = gameOverNtf.reason();
    robot.RbtResetData();
    robot.SetStatus(INITGAME);
    DEBUG("Receved game over notify, reset robot to sign up condation.");
    return false;
}
//
//游戏结果
bool GetGameResultInfo::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
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
    OrgRoomDdzGameResultNtf orgRoomDdzGameResultNtf;
    if (!orgRoomDdzGameResultNtf.ParseFromString(msg))
    {
        ERROR("parse pb message OrgRoomDdzGameResultNtf error.");
        return false;
    }
    robot.RbtResetData();
    robot.SetStatus(INITGAME);
    DEBUG("Receved game result notify, reset robot to sign up condation.");
    return false;
}

//获取叫分结果
bool GetCallScoreResultInfo::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
    //message CallScoreAck {
    //    required int32 result = 1;
    //}
    robot.SetStatus(GAMMING);
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
        DEBUG("Get call score result, call score successed.");
    }
    return false;
}

//获取出牌结果
bool GetTakeOutCardResultInfo::operation( OGLordRobotAI& robot, const string& msg, string& serializedStr ){
    //message TakeoutCardAck {
    //    required int32 result = 1;
    //}
    robot.SetStatus(GAMMING);
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
        DEBUG("Get take out card result, take out card successed.");
    }
    return false;
}

/*++++++++++++++++++游戏阶段 结束++++++++++++++++++++++*/

