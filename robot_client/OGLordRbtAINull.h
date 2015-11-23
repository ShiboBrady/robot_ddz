#pragma once
#include "OGLordRobotAIInterface.h"
/***********************************************\
斗地主无逻辑版机器人 : COGLordRbtAINull
创建 : 2013-05-10 wangjian
COGLordRbtAINull 实现了COGLordRobotInterface , 无智能逻辑,只确保处于领牌时不阻碍游戏进程即可,叫分时固定叫2分.
\***********************************************/

#define CARDS_SUM 54

class COGLordRbtAINull : public COGLordRobotAIInterface
{
public:
    COGLordRbtAINull();
    ~COGLordRbtAINull();

    // 【新设计的对外接口】
public:
    // 收到发牌消息
    bool RbtInInitCard(int argSeat, std::vector<int> argHandCard);

    // 收到玩家叫分信息
    bool RbtInCallScore(int argSeat, int argCallScore);

    // 请求给出叫分策略
    bool RbtOutGetCallScore(int &argCallScore,int &delay);

    // 收到确定地主信息
    bool RbtInSetLord(int argLordSeat, std::vector<int> argReceiceCard);

    // 收到玩家牌信息
    bool RbtInTakeOutCard(int argSeat, std::vector<int> argCards);

    // 请求给出出牌策略
    bool RbtOutGetTakeOutCard(std::vector<int> &argCards ,int &delay);

    // 重置机器人信息
    bool RbtResetData();

    // 设置地主和玩家座位
    bool RbtInSetSeat(int argMySeat, int argLordSeat);

    // 设置手牌和底牌
    bool RbtInSetCard(std::vector<int> argInitCard, std::vector<int> argReceiveCard);

    // 设置出牌记录
    bool RbtInTakeOutRecord(std::vector<int> argTakeOutSeat ,std::vector<std::vector<int> > argTakeOutRecord);

    bool RbtInSetLevel(int argLevel);

    bool RbtInNtfCardInfo(std::vector<std::vector<int> > argHandCard);

    bool RbtInSetGrabLord(int argSeat);

    bool RbtOutGetGrabLord(bool &grabLord ,int &delay);

    bool RbtOutGetLastError(int &errorCode);

    bool RbtOutGetCallScore( int &callScore);                 // 返回值引用
    bool RbtInCallScore( int argSeat,                        // 座位号
                                 int argCallScore,                   // 叫的分数
                                 int & delay                         // 延时时间
                                 );
    bool RbtInShowCard( int argShowSeat,                    // 明牌玩家座位
                                std::vector<int> argHandCard        // 玩家手牌
                                );

    bool RbtOutShowCard(bool &showCard, int &delay);                // 是否明牌 true:明牌 false:不明牌
    bool RbtOutDoubleGame(bool &Double_game, int &delay){Double_game=0;delay=0;return true;}
private:
    int m_szHandCards[CARDS_SUM];    // 手牌信息
    int m_nSeat;                // 玩家座位号
    int m_nBiggerSeat;          // 最大牌座位号(玩家如果是领牌者则出最小牌,如果非领牌者则不出)
    int m_nScore;               // 最大叫分值
};
