#include "AbstractProduct.h"
#include "OGLordRobotAI.h"
#include "PBGameDDZ.pb.h"
#include "AIUtils.h"

using namespace PBGameDDZ;
using namespace AIUtils;

AbstractProduct::AbstractProduct(){
}


AbstractProduct::~AbstractProduct(){
}

//游戏开始
GetGameStartInfo::GetGameStartInfo(){
}


GetGameStartInfo::~GetGameStartInfo(){
}


string GetGameStartInfo::operation( OGLordRobotAI& robot, const string& msg ){
    // 游戏开始
    //message GameStartNtf {
    //    required string gameName = 1;       // 游戏服务通道号
    //    required int32 basicScore   = 2;    // 底分
    //    repeated UserInfo userinfo  = 3;    // 用户信息
    //    optional MatchInfo matchInfo = 4;   // 如果是游戏场，该字段不用
    //}
    string serializedStr;
    GameStartNtf gameStartNtf;
    if (!gameStartNtf.ParseFromString(msg))
    {
        cout << "parse pb message GameStartNtf error." << endl;
        return serializedStr;
    }
    int index = 0;
    int iUserNum = gameStartNtf.userinfo_size();
    cout << gameStartNtf.gamename() << endl;
    cout << iUserNum << endl;
    string aiName = robot.GetAiName();
    for (index = 0; index < iUserNum; ++index)//寻找自己的座位号:0-2
    {
        cout << "total userNum: " << iUserNum << ", robot name: " << aiName << ", netname:" << gameStartNtf.userinfo(index).username() << endl;
        if (gameStartNtf.userinfo(index).username() == aiName)
        {
            break;
         }
    }

    if (iUserNum == index)
    {
        cout << "Doesn't find robot name." << endl;
    }
    else
    {
        robot.SetAiSeat(index);
        cout << "Set robot seat successed, seat is: " << index << endl;
    }
    return serializedStr;
}


//发牌
InitHardCard::InitHardCard(){
}


InitHardCard::~InitHardCard(){
}


string InitHardCard::operation( OGLordRobotAI& robot, const string& msg ){
    // 发牌
    //message DealCardNtf {
    //    required int32 headerseat = 1;          // 第一个叫分座位号
    //    repeated HandCardList cards = 2;        // 玩家手牌
    //}
    string serializedStr;
    DealCardNtf dealCardNtf;
    if (!dealCardNtf.ParseFromString(msg))
    {
        cout << "parse pb message DealCardNtf error." << endl;
        return serializedStr;
    }
    int aiSeat = robot.GetAiSeat();
    cout << "my seat is: " << aiSeat << endl;
    if (-1 == aiSeat)
    {
        cout << "Not init seat info." << endl;
        return serializedStr;
    }
    int hearderSeat = dealCardNtf.headerseat();
    int cardsSize = dealCardNtf.cards(aiSeat).cards_size();//查看自己的那手牌信息

    vector<int> vecHandCard;
    for (int index = 0; index < cardsSize; ++index)//获取自己的那手牌
    {
        vecHandCard.push_back(dealCardNtf.cards(aiSeat).cards(index));
    }
    robot.RbtInInitCard(aiSeat, vecHandCard);
    cout << "Init hand card successed, hand card info is: " << endl;
    printCardInfo(vecHandCard);
    return serializedStr;
}

//收到叫分通知
GetCallScoreInfo::GetCallScoreInfo(){
}


GetCallScoreInfo::~GetCallScoreInfo(){
}


string GetCallScoreInfo::operation( OGLordRobotAI& robot, const string& msg ){
    // 叫分
    //message UserCallScoreNtf {
    //    required int32 seatno = 1;                  // 座位号
    //    required int32 seatnext = 2[default=-1];    // 下一个叫分座位，-1叫分结束
    //    required int32 score = 3;                   // 叫分值(1/2/3), 0-不叫
    //}
    string serializedStr;
    UserCallScoreNtf userCallScoreNtf;
    if (!userCallScoreNtf.ParseFromString(msg))
    {
        cout << "parse pb message UserCallScoreNtf error." << endl;
        return serializedStr;
    }
    int seatNo = userCallScoreNtf.seatno();
    int seatNext = userCallScoreNtf.seatnext();
    int score = userCallScoreNtf.score();
    int aiSeat = robot.GetAiSeat();

    userCallScoreNtf.Clear();
    userCallScoreNtf.set_seatno(aiSeat);
    if (-1 == seatNext)
    {
        //停止叫分，不叫
        userCallScoreNtf.set_seatnext(-1);
        userCallScoreNtf.set_score(0);
    }
    else if ((seatNo + 1) % 3 != aiSeat)
    {
        //没轮到自己，不叫
        robot.RbtInCallScore(seatNo, score);
        userCallScoreNtf.set_seatnext((seatNo + 1) % 3);
        userCallScoreNtf.set_score(0);
    }
    else
    {
        int myScore = 0;
        robot.RbtOutGetCallScore(myScore);
        int curScore = robot.GetCurScore();
        cout << "My score is: " << myScore << " Cur score is: " << curScore << endl;

        if (score >= myScore)
        {
            //目前分数比自己的大，不叫
            robot.RbtInCallScore(seatNo, score);
            userCallScoreNtf.set_seatnext((aiSeat + 1) % 3);
            userCallScoreNtf.set_score(0);
            cout << "Doesn't choose call score, curScore is: " << curScore << endl;
        }
        else if (score == 0 && robot.GetCurScore() >= myScore)
        {
            //上一个用户没叫，且上上一个用户叫的分比自己的高，不叫
            userCallScoreNtf.set_seatnext((aiSeat + 1) % 3);
            userCallScoreNtf.set_score(0);
            cout << "Doesn't choose call score, curScore is: " << curScore << endl;
        }
        else
        {
            //叫分
            userCallScoreNtf.set_seatnext((aiSeat + 1) % 3);
            userCallScoreNtf.set_score(myScore);
            cout << "Choose call score." << endl;
        }
    }
    userCallScoreNtf.SerializeToString(&serializedStr);
    cout << "callScore info: " << serializedStr << endl;
    return serializedStr;
}

//收到地主信息
GetLordInfo::GetLordInfo(){
}


GetLordInfo::~GetLordInfo(){
}


string GetLordInfo::operation( OGLordRobotAI& robot, const string& msg ){
    // 地主确定
    //message LordSetNtf {
    //    required int32 seatlord = 1;    // 地主座位号
    //    required int32 callscore = 2;   // 地主叫分
    //}
    string serializedStr;
    LordSetNtf lordSetNtf;
    if (!lordSetNtf.ParseFromString(msg))
    {
        cout << "parse pb message LordSetNtf error." << endl;
        return serializedStr;
    }
    int seatLord = lordSetNtf.seatlord();
    robot.SetLordSeat(seatLord);
    cout << "Set lord info successed, " << seatLord << endl;
    return serializedStr;
}

//收到底牌
GetBaseCardInfo::GetBaseCardInfo(){
}


GetBaseCardInfo::~GetBaseCardInfo(){
}


string GetBaseCardInfo::operation( OGLordRobotAI& robot, const string& msg ){
    // 发底牌
    //message SendBaseCardNtf {
    //    repeated int32 basecards = 1;   // 底牌数据
    //}
    string serializedStr;
    SendBaseCardNtf sendBaseCardNtf;
    if (!sendBaseCardNtf.ParseFromString(msg))
    {
        cout << "parse pb message SendBaseCardNtf error." << endl;
        return serializedStr;
    }
    int baseCardSize = sendBaseCardNtf.basecards_size();
    int seatLord = robot.GetLordSeat();
    vector<int> vecBaseCard;
    for (int index = 0; index < baseCardSize; ++index)
    {
        vecBaseCard.push_back(sendBaseCardNtf.basecards(index));
    }
    robot.RbtInSetLord(seatLord, vecBaseCard);
    cout << "Set base card successed." << endl;
    printCardInfo(vecBaseCard);
    return serializedStr;
}

//收到出牌通知
GetTakeOutCardInfo::GetTakeOutCardInfo(){
}


GetTakeOutCardInfo::~GetTakeOutCardInfo(){
}


string GetTakeOutCardInfo::operation( OGLordRobotAI& robot, const string& msg ){
    // 出牌
    //message TakeoutCardNtf {
    //    required int32 seatno = 1;      // 出牌座位号
    //    required int32 seatnext = 2;    // 下一个出牌座位
    //    repeated int32 cards = 3;       // 出牌数据
    //    required int32 cardtype = 4;    // 类型
    //    required int32 multiple = 5;    // 当前倍数(炸弹产生的倍数)
    //}
    string serializedStr;
    TakeoutCardNtf takeoutCardNtf;
    if (!takeoutCardNtf.ParseFromString(msg))
    {
        cout << "parse pb message TakeoutCardNtf error." << endl;
        return serializedStr;
    }
    int seatno = takeoutCardNtf.seatno();
    int seatnext = takeoutCardNtf.seatnext();
    int cardsNum = takeoutCardNtf.cards_size();
    vector<int> vecOppTackOutCard;
    for (int index = 0; index < cardsNum; ++index)
    {
        vecOppTackOutCard.push_back(takeoutCardNtf.cards(index));
    }
    cout << "Current card info is:" << endl;
    printCardInfo(vecOppTackOutCard);
    robot.RbtInTakeOutCard(seatno, vecOppTackOutCard);

    int aiSeat = robot.GetAiSeat();
    if (seatnext == aiSeat)
    {
        cout << "It's my turn to take out card." << endl;
        vector<int> vecTackOutCard;
        robot.RbtOutGetTakeOutCard(vecTackOutCard);
        cout << "My take out cards is:" << endl;
        printCardInfo(vecTackOutCard);

        takeoutCardNtf.Clear();
        //出牌
        takeoutCardNtf.set_seatno(aiSeat);
        takeoutCardNtf.set_seatnext((aiSeat + 1) % 3);
        for (int iIndex = 0; iIndex != vecTackOutCard.size(); ++iIndex)
        {
            takeoutCardNtf.add_cards(vecTackOutCard[iIndex]);
        }
        //都有哪些类型?
        takeoutCardNtf.set_cardtype(1);
        takeoutCardNtf.set_multiple(1);
        takeoutCardNtf.SerializeToString(&serializedStr);
        //JsonFormat.searialJsonMsg();
        cout << "Take out card successed!" << endl;
    }
    else
    {
        cout << "It's not my turn to take out card, card info is:" << endl;
    }
    return serializedStr;
}

//游戏结束
GetGameOverInfo::GetGameOverInfo(){
}


GetGameOverInfo::~GetGameOverInfo(){
}


string GetGameOverInfo::operation( OGLordRobotAI& robot, const string& msg ){
    // 游戏结束通知
    //message GameOverNtf {
    //    required int32 reason = 1[default=2];   // 结束原因：1-强制结束，2-达到最大游戏盘数
    //}
    //robot->RbtOutGetTakeOutCard(vecCards);
    string serializedStr;
    GameOverNtf gameOverNtf;
    if (!gameOverNtf.ParseFromString(msg))
    {
        cout << "parse pb message GameOverNtf error." << endl;
        return serializedStr;
    }
    int reason = gameOverNtf.reason();
    robot.RbtResetData();
    cout << "Receved game over notify successed." << endl;
    return serializedStr;
}

//获取叫分结果
GetCallScoreResultInfo::GetCallScoreResultInfo(){
}


GetCallScoreResultInfo::~GetCallScoreResultInfo(){
}


string GetCallScoreResultInfo::operation( OGLordRobotAI& robot, const string& msg ){
    //message CallScoreAck {
    //    required int32 result = 1;
    //}
    //robot->RbtOutGetTakeOutCard(vecCards);
    string serializedStr;
    CallScoreAck callScoreAck;
    if (!callScoreAck.ParseFromString(msg))
    {
        cout << "parse pb message CallScoreAck error." << endl;
        return serializedStr;
    }
    int result = callScoreAck.result();
    cout << "Get call score result successed, call score result is: " << result << endl;
    return serializedStr;
}

//获取出牌结果
GetTakeOutCardResultInfo::GetTakeOutCardResultInfo(){
}


GetTakeOutCardResultInfo::~GetTakeOutCardResultInfo(){
}


string GetTakeOutCardResultInfo::operation( OGLordRobotAI& robot, const string& msg ){
    //message TakeoutCardAck {
    //    required int32 result = 1;
    //}
    //robot->RbtOutGetTakeOutCard(vecCards);
    string serializedStr;
    TakeoutCardAck takeoutCardAck;
    if (!takeoutCardAck.ParseFromString(msg))
    {
        cout << "parse pb message TakeoutCardAck error." << endl;
        return serializedStr;
    }
    int result = takeoutCardAck.result();
    cout << "Get take out result successed, " << result << endl;
    return serializedStr;
}


