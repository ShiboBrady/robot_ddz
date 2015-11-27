#ifndef OGLordRobotAI_h__
#define OGLordRobotAI_h__

#pragma once
#include "Robot.h"
#include "AbstractProduct.h"
#include "SimpleFactory.h"

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <numeric>
#include <string>
#include <iostream>

class OGLordRobotAI
{
public:
	OGLordRobotAI( const int robotId, const int IQLevel );
	~OGLordRobotAI(void);

	// 收到发牌消息
	virtual bool RbtInInitCard( int argSeat,                        // 自己的座位号
		std::vector<int> argHandCard        // 发送的手牌
		);

	// 输入机器人智商级别
	virtual bool RbtInSetLevel( int argLevel);                  // 机器人智商级别

	// 输入各个座位玩家的牌信息
	virtual bool RbtInNtfCardInfo(  std::vector<std::vector<int> > argHandCard
		// 各个座位玩家的手牌 0,1,2,3个元素分别表示第0,1,2号座位以及底牌的内容。
		);

	// 收到玩家叫分信息
	virtual bool RbtInCallScore( int argSeat,                       // 座位号
		int argCallScore                   // 叫的分数
		);

	// 请求给出叫分策略
	virtual bool RbtOutGetCallScore( int &callScore                 // 返回值引用
		);

	// 输入玩家抢地主信息
	virtual bool RbtInSetGrabLord( int argSeat                      // 座位号
		);

	// 输出抢地主策略
	virtual bool RbtOutGetGrabLord( bool &grabLord                  // true：抢地主 FALSE：不抢
		);

	// 收到确定地主信息
	virtual bool RbtInSetLord( int argLordSeat,                     // 地主座位
		std::vector<int> argReceiceCard      // 地主收到的底牌
		);


	// 【出牌阶段】
	// 收到玩家牌信息
	virtual bool RbtInTakeOutCard( int argSeat,                     // 座位号
		std::vector<int> argCards        // 牌内容
		);

	// 请求给出出牌策略
	virtual bool RbtOutGetTakeOutCard(std::vector<int> &vecCards    // 返回的牌
		);


	// 【重置机器人】
	// 用于第二盘时复用机器人AI 以及断线续完
	// 重置机器人信息
	virtual bool RbtResetData();

	// 设置地主和玩家座位
	virtual bool RbtInSetSeat( int argMySeat,                       // 自己的座位
		int argLordSeat                      // 地主的座位
		);

	// 设置手牌和底牌
	virtual bool RbtInSetCard( std::vector<int> argInitCard,        // 自己初始化的牌
		std::vector<int> argReceiveCard      // 地主收到的底牌
		);
	// 设置出牌记录
	virtual bool RbtInTakeOutRecord( std::vector<int> argTakeOutSeat,               // 历史出牌-座位记录
		std::vector<std::vector<int> > argTakeOutRecord // 历史出牌-牌记录
		);


	// 【其他】
	// 返回错误信息
	virtual bool RbtOutGetLastError( int &errorCode);           // 返回当前错误码

    //访问数据成员接口
    virtual void SetStatus( RobotStatus status ) { _status = status; };
    virtual RobotStatus GetStatus() { return _status; };

    virtual void SetCost( int costId ) { _costId = costId; };
    virtual int GetCost() { return _costId; };

    virtual void SetRobotId( int robotId ) { this->robotId = robotId; };
    virtual int GetRobotId() { return robotId; };

    virtual void SetAiSeat( int aiSeat ) { this->aiSeat = aiSeat; };
    virtual int GetAiSeat() { return aiSeat; };

    virtual void SetCurScore( int curScore ) { this->curScore = curScore; };
    virtual int GetCurScore() { return curScore; };

    virtual void SetLordSeat( int lordSeat ) { this->lordSeat = lordSeat; };
    virtual int GetLordSeat() { return lordSeat; };

    std::string RobotProcess( int msgId, const std::string& msg );

    void RecoveryHandCards();
private:

	void updateAiPosition();//更新自己的位置，确定是地主或是地主上家还是地主下家

	void sortHandMap();

	void sortHandMapLvl();

	void findLowestHand(Hand &hand);

	void findHigherHand(Hand &hand);

	void findHigherHandNotBomb(Hand &hand);

	void findMostCardsHandNotBomb(Hand &hand);

	void takeOutHand(Hand &hand, std::vector<int> &takeOutCards);

	void takeOutLvl0(Hand &hand);

	void takeOutLvl1(Hand &hand);

	void takeOutHighLvl(Hand &hand);

	bool isFree();

	bool isDanger();

	bool isGood(HandsMapSummary &hmSummary, int controlCount, int minOppNum);

	bool isFirstHalf();

	bool isGoodFarmer();

	void findChargeHandFirst(Hand &hand, bool typeCountFirst);

	void lordTakeOutFree(Hand &hand);

	void lordTakeOutFreeFarmerOnly1Card(Hand &hand);

	void lordTakeOutFreeFarmerOnly2Cards(Hand &hand);

	void lordTakeOutFreeNormal(Hand &hand);

	void lordTakeOutHigher(Hand &hand);

	void lordTakeOutHigherFarmerOnly1Card(Hand &hand);

	void lordTakeOutHigherFarmerOnly2Cards(Hand &hand);

	void lordTakeOutHigherNormal(Hand &hand);

	void lordTakeOutHigherRebuild(Hand &hand);

	void farmerTakeOutLordOnly1Card(Hand &hand);

	void farmerTakeOutLordOnly2Cards(Hand &hand);

	void farmerMustTakeOutLordOnly1Card(Hand &hand);

	void farmerMustTakeOutLordCharge(Hand &hand);

	void goodFarmerOverOtherFarmer(Hand &hand);

	void downGoodFarmerTakeOut(Hand &hand);

	void downBadFarmerTakeOut(Hand &hand);

	void upGoodFarmerTakeOut(Hand &hand);

	void upBadFarmerTakeOut(Hand &hand);

	void downFarmerTakeOutUpFarmerOnly1Card(Hand &hand);

	void findBestHigherHandFromMap(Hand &hand);

	void findBestHigherHandFromPoints(Hand &hand, bool force, bool lordOnly1Card);

	void farmerTakeOutWhenLordTakeOutHighSolo(Hand &hand);

	void refindForTrio(Hand &hand);

	bool isDangerLvl();

	bool isGoodLvl(HandsMapSummary &hmSummary, int minOppNum);

	void findKickerLvl(Hand &hand);

	bool containsHand(std::map<HandType, std::vector<Hand> > &allHands, Hand &hand);

	void lordTakeOutFreeLvl(Hand &hand);

	void lordTakeOutFreeFarmerOnly1CardLvl(Hand &hand);

	void lordTakeOutFreeFarmerOnly2CardsLvl(Hand &hand);

	void lordTakeOutFreeNormalLvl(Hand &hand);

	void lordTakeOutHigherLvl(Hand &hand);

	void lordTakeOutHigherFarmerOnly1CardLvl(Hand &hand);

	void lordTakeOutHigherFarmerOnly2CardsLvl(Hand &hand);

	void lordTakeOutHigherNormalLvl(Hand &hand);

	void lordTakeOutHigherRebuildLvl(Hand &hand);

	void farmerTakeOutLordOnly1CardLvl(Hand &hand);

	void farmerTakeOutLordOnly2CardsLvl(Hand &hand);

	void farmerMustTakeOutLordOnly1CardLvl(Hand &hand);

	void farmerMustTakeOutLordChargeLvl(Hand &hand);

	void goodFarmerOverOtherFarmerLvl(Hand &hand);

	void downGoodFarmerTakeOutLvl(Hand &hand);

	void downBadFarmerTakeOutLvl(Hand &hand);

	void upGoodFarmerTakeOutLvl(Hand &hand);

	void upBadFarmerTakeOutLvl(Hand &hand);

	void downFarmerTakeOutUpFarmerOnly1CardLvl(Hand &hand);

	void countHighHand(std::map<HandType, std::vector<Hand> > &allHands,
						std::map<HandType, int> &uniHighHands,
						std::map<HandType, int> &HighHands,
						std::map<HandType, int> &goodLowHands,
						int &trioCount);

	HandsMapSummary getHandsMapSummaryLvl(std::map<HandType, std::vector<Hand> > &allHands,
						std::map<HandType, int> &uniHighHands,
						std::map<HandType, int> &HighHands,
						std::map<HandType, int> &goodLowHands,
						int trioCount);

	int findOppHighestPoint(HandType type);

	int findOppHighestChain(HandType type, int len);

	int addAllValue(std::map<HandType, int> &handsCount);

	void findChargeHandFirstLvl(Hand &hand, bool typeCountFirst);

	bool mustHighLevel;

    RobotStatus _status;

    std::vector<int> vecLastTakeOutCards;//最后一次出牌纪录

    int _costId;//报名比赛时的费用ID

    int robotId;//机器人id

	int level;//机器人智能等级，目前有0、1

	int aiSeat;//自己的座位

	int lordSeat;//地主座位

	int downSeat;//地主下方座位

	int upSeat;//地主上方座位

	Position aiPosition;

	std::vector<int> leftoverCards;

	std::vector<int> aiCardsVec;//自己手中的牌，以17位的vector保存

	CardsInfo playerInfo[PLAYER_NUM];//玩家的手牌信息，以结构体的形式保存

	int remainPoints[CARD_POINT_NUM];//记牌器

	int otherPoints[CARD_POINT_NUM];//其他人手中的牌

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

	int curScore;

	int curCaller;

	Hand curHand;//当前出牌的信息

	int curHandSeat;//当前出牌的人

	std::vector<std::pair<int, int> > callHistory;

	std::vector<std::pair<int, Hand> > history;

	int errCode;

    SimpleFactory factory;

    AbstractProduct* product;
};
#endif // OGLordRobotAI_h__
