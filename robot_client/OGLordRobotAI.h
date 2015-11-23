#ifndef OGLordRobotAI_h__
#define OGLordRobotAI_h__

#pragma once
#include "Robot.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <numeric>
#include <iostream>
using namespace std;

class OGLordRobotAI
{
public:
	OGLordRobotAI( const string& robotName );
	~OGLordRobotAI(void);

    //【叫分阶段】
    // 设置自己的座位号
    bool SetSelfSeat( int argSeat );

	// 设置自己的手牌
	bool RbtInInitCard( vector<int> argHandCard );

	// 给出自己的叫分情况
	bool RbtOutGetCallScore( int &callScore );

	// 收到确定地主信息
	bool RbtInSetLord( int argLordSeat );

    // 设置自己的底牌
    bool RbtSetBaseCard( vector<int> argBaseCard );

	//【出牌阶段】
    // 收到玩家牌信息
    bool RbtInTakeOutCard( int argSeat, std::vector<int> argCards);

	// 请求给出出牌策略
	bool RbtOutGetTakeOutCard( vector<int> &vecTakeOutCards );


	// 【重置机器人】
	// 用于第二盘时复用机器人AI 以及断线续完
	// 重置机器人信息
	bool RbtResetData();

    bool isFree();

    void findLowestHand(Hand &hand);

	void findHigherHand(Hand &hand);

    void findHigherHandNotBomb( Hand &hand );

    void updateAiPosition();

    void takeOutLvl0(Hand &hand);

    void takeOutHand(Hand &hand, std::vector<int> &takeOutCards);

    string aiName;//自己的名字

	int aiSeat;//自己的座位

    Position aiPosition;//自己的角色

    int myScore;//自己的叫分

	int lordSeat;//地主座位

	int downSeat;//地主下方座位

	int upSeat;//地主上方座位

	std::vector<int> leftoverCards;//底牌

	std::vector<int> aiCardsVec;//自己手中的牌，内容是0-53

	CardsInfo playerInfo;//自己的手牌信息，以结构体的形式保存，其中牌的信息是CARD_3--RED_JOKER

	int remainPoints[CARD_POINT_NUM];//记牌器

	std::map<HandType, std::vector<Hand> > handsMap;//当前手中牌所组成的类型，比如:炸弹是哪些，飞机是哪些

	HandsMapSummary summary;//套牌信息

	std::map<HandType, int> uniHighHandCount;

	std::map<HandType, int> highHandCount;

	std::map<HandType, int> goodLowHandCount;

	int handsTrioCount;

	CardPoint lowestControl;

	int controlNum;//抢地主的依据

	int maxOppNumber;//对手手中的牌，地主需要统计两个农民的手牌数量，农民需要知道一个地主的手牌数量

	int minOppNumber;

	int curScore;//当前的叫分数

	int curCaller;

	Hand curHand;//当前出牌的信息

	int curHandSeat;//当前出牌的人

	std::vector<std::pair<int, int> > callHistory;

	std::vector<std::pair<int, Hand> > history;//出牌历史记录

	int errCode;
};
#endif // OGLordRobotAI_h__
