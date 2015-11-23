#include "AbstractProduct.h"
#include "OGLordRobotAI.h"
#include "PBGameDDZ.pb.h"
//#include "GameProtocol.h"

using namespace PBGameDDZ;
void printIntVector(vector<int>& vecContent);

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
    gameStartNtf.ParseFromString(msg);
    int index = 0;
    int iUserNum = gameStartNtf.userinfo_size();
    cout << gameStartNtf.gamename() << endl;
    cout << iUserNum << endl;
    for (index = 0; index < iUserNum; ++index)//寻找自己的座位号:0-2
    {
        cout << "iUserNum: " << iUserNum << ", aiName: " << robot.aiName << ", netname:" << gameStartNtf.userinfo(index).username() << endl;
        if (gameStartNtf.userinfo(index).username() == robot.aiName)
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
        robot.SetSelfSeat(index);
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
    dealCardNtf.ParseFromString(msg);
    int hearderSeat = dealCardNtf.headerseat();
    int cardsSize = dealCardNtf.cards(robot.aiSeat).cards_size();//查看自己的那手牌信息

    vector<int> vecHandCard;
    for (int index = 0; index < cardsSize; ++index)//获取自己的那手牌
    {
        vecHandCard.push_back(dealCardNtf.cards(robot.aiSeat).cards(index));
    }
    robot.RbtInInitCard(vecHandCard);
    cout << "Init hand card successed, hand card info is: " << endl;
    printIntVector(vecHandCard);

    //if (hearderSeat == robot.aiSeat)
    //{
    //    //叫分
    //    UserCallScoreNtf userCallScoreNtf;
    //    userCallScoreNtf.set_seatno(robot.aiSeat);
    //    userCallScoreNtf.set_seatnext((robot.aiSeat + 1) % 3);
    //    userCallScoreNtf.set_score(robot.myScore);
    //    userCallScoreNtf.SerializeToString(&serializedStr);
    //    cout << "callScore info: " << serializedStr << endl;
    //}
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
    userCallScoreNtf.ParseFromString(msg);
    int seatNo = userCallScoreNtf.seatno();
    int seatNext = userCallScoreNtf.seatnext();
    int score = userCallScoreNtf.score();

    userCallScoreNtf.Clear();
    userCallScoreNtf.set_seatno(robot.aiSeat);
    if (-1 == seatNext)
    {
        //停止叫分，不叫
        userCallScoreNtf.set_seatnext(-1);
        userCallScoreNtf.set_score(0);
    }
    else if ((seatNo + 1) % 3 != robot.aiSeat)
    {
        //没轮到自己，不叫
        robot.curScore = score;
        userCallScoreNtf.set_seatnext((seatNo + 1) % 3);
        userCallScoreNtf.set_score(0);
    }
    else
    {
        if (score >= robot.myScore)
        {
            //目前分数比自己的大，不叫
            robot.curScore = score;
            userCallScoreNtf.set_seatnext((robot.aiSeat + 1) % 3);
            userCallScoreNtf.set_score(0);
        }
        else if (score == 0 && robot.curScore >= robot.myScore)
        {
            //上一个用户没叫，且上上一个用户叫的分比自己的高，不叫
            userCallScoreNtf.set_seatnext((robot.aiSeat + 1) % 3);
            userCallScoreNtf.set_score(0);
        }
        else
        {
            //叫分
            userCallScoreNtf.set_seatnext((robot.aiSeat + 1) % 3);
            userCallScoreNtf.set_score(robot.myScore);
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
    lordSetNtf.ParseFromString(msg);
    int seatLord = lordSetNtf.seatlord();
    robot.RbtInSetLord(seatLord);
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
    if (robot.aiSeat != robot.lordSeat)
    {
        //自己不是地主，不使用底牌
        cout << "Doesn't use base card." << endl;
        return serializedStr;
    }
    SendBaseCardNtf sendBaseCardNtf;
    sendBaseCardNtf.ParseFromString(msg);
    int baseCardSize = sendBaseCardNtf.basecards_size();
    vector<int> vecBaseCard;
    for (int index = 0; index < baseCardSize; ++index)
    {
        vecBaseCard.push_back(sendBaseCardNtf.basecards(index));
    }
    robot.RbtSetBaseCard(vecBaseCard);
    cout << "Set base card successed." << endl;
    printIntVector(vecBaseCard);
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
    takeoutCardNtf.ParseFromString(msg);
    int seatno = takeoutCardNtf.seatno();
    int seatnext = takeoutCardNtf.seatnext();
    int cardsNum = takeoutCardNtf.cards_size();
    vector<int> vecOppTackOutCard;
    for (int index = 0; index < cardsNum; ++index)
    {
        vecOppTackOutCard.push_back(takeoutCardNtf.cards(index));
    }
    cout << "Current card info is:" << endl;
    printIntVector(vecOppTackOutCard);
    if (seatnext == robot.aiSeat)
    {
        vector<int> vecTackOutCard;
        robot.RbtOutGetTakeOutCard(vecTackOutCard);
        takeoutCardNtf.Clear();
        //出牌
        takeoutCardNtf.set_seatno(robot.aiSeat);
        takeoutCardNtf.set_seatnext((robot.aiSeat + 1) % 3);
        for (int iIndex = 0; iIndex != vecTackOutCard.size(); ++iIndex)
        {
            takeoutCardNtf.set_cards(iIndex, vecTackOutCard[iIndex]);
        }
        //都有哪些类型?
        takeoutCardNtf.set_cardtype(1);
        takeoutCardNtf.set_multiple(1);
        takeoutCardNtf.SerializeToString(&serializedStr);
        //JsonFormat.searialJsonMsg();
        cout << "Take out card successed!" << endl;
        printIntVector(vecTackOutCard);
    }
    else
    {
        robot.RbtInTakeOutCard(seatno, vecOppTackOutCard);
        cout << "It's not my turn to take out card." << endl;
        printIntVector(vecOppTackOutCard);
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
    gameOverNtf.ParseFromString(msg);
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
    cout << "Get call score result successed." << endl;
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
    takeoutCardAck.ParseFromString(msg);
    int result = takeoutCardAck.result();
    cout << "Get take out result successed, " << result << endl;
    return serializedStr;
}

void printIntVector(vector<int>& vecContent)
{
    for (vector<int>::iterator it = vecContent.begin(); it != vecContent.end(); ++it)
    {
        cout << POINT_CHAR[(*it) / 4] << " ";
    }
    cout << "" << endl;
}

