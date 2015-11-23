#include "OGLordRobotAI.h"
#include "AIUtils.h"
#include <string.h>

using namespace std;
using namespace AIUtils;

OGLordRobotAI::OGLordRobotAI( const string& robotName )
    :aiName(robotName),
     lordSeat(-1),
     aiSeat(-1),
     myScore(0),
     curCaller(0),
     curScore(0),
     curHandSeat(-1)
{
    curHand.type = NOTHING;
}

OGLordRobotAI::~OGLordRobotAI(void)
{
}

bool OGLordRobotAI::SetSelfSeat( int argSeat )
{
    aiSeat = argSeat;
    return true;
}

bool OGLordRobotAI::RbtInInitCard( vector<int> argHandCard )
{
	aiCardsVec = argHandCard;
    cardVecToPointArr(argHandCard, playerInfo.points);
    playerInfo.total = 17;

    //初始化记牌器
	fill(remainPoints, remainPoints + (CARD_POINT_NUM - 2), 4);
	remainPoints[BLACK_JOKER] = 1;
	remainPoints[RED_JOKER] = 1;

    //按照炸弹，飞机，顺子，连对的顺序组织手中的牌
    int cardsCopy[CARD_POINT_NUM];
	copy(playerInfo.points, playerInfo.points + CARD_POINT_NUM, cardsCopy);
    splitCardsToHandsKind2(cardsCopy, false, handsMap);

    //计算自己要叫得分
    RbtOutGetCallScore(myScore);
	return true;
}

bool OGLordRobotAI::RbtInSetLord( int argLordSeat )
{
    lordSeat = argLordSeat;
    downSeat = (lordSeat + 1) % 3;
	upSeat = (lordSeat + 2) % 3;
    updateAiPosition();
    return true;
}

bool OGLordRobotAI::RbtSetBaseCard( vector<int> argBaseCard )
{
    playerInfo.total = 20;
	for (int i=0; i<3; ++i)
	{
		aiCardsVec.push_back(argBaseCard[i]);
		playerInfo.points[cardToPoint(argBaseCard[i])]++;
	}

    //按照炸弹，飞机，顺子，连对的顺序更新自己手中的牌
    int cardsCopy[CARD_POINT_NUM];
	copy(playerInfo.points, playerInfo.points + CARD_POINT_NUM, cardsCopy);
    splitCardsToHandsKind2(cardsCopy, false, handsMap);
}

bool OGLordRobotAI::RbtOutGetCallScore(int &callScore)
{
    CardsInfo& aiCards = playerInfo;
    int* aiPoints = aiCards.points;
    int scores = 0;
    bool isCallLord = false;
    if (aiPoints[BLACK_JOKER] == 1 || aiPoints[RED_JOKER] == 1)
    {
        if (aiPoints[BLACK_JOKER] == 1 && aiPoints[RED_JOKER] != 1)//小王加3分
        {
            scores += 3;
        }
        else if (aiPoints[BLACK_JOKER] != 1 && aiPoints[RED_JOKER] == 1)//大王加4分
        {
            scores += 4;
        }
        else//火箭加8分
        {
            scores += 8;
        }
    }

    scores += 2 * aiPoints[CARD_2];//一个2加2分

    for (int index = CARD_3; index < CARD_2; ++ index)//一个炸弹加6分
    {
        if (4 == aiPoints[index])
        {
            scores += 6;
        }
    }

    if (scores >= 7)//分数大于等于7分时叫三分
    {
        callScore = 3;
        isCallLord = true;
    }
    else if (scores < 7 && scores >= 5)//大于等于5分时叫二分
    {
        callScore = 2;
        isCallLord = true;
    }
    else if (scores < 5 && scores >= 3)//大于等于3分时叫一分
    {
        callScore = 1;
        isCallLord = true;
    }
    else
    {
        //小于3分不叫地主
        callScore = 0;
    }
    return isCallLord;
}

bool OGLordRobotAI::RbtInTakeOutCard( int argSeat, std::vector<int> argCards)
{
	Hand hand;
	if (argCards.size() == 0)
	{
		hand.type = NOTHING;
	}
	else
	{
		int points[CARD_POINT_NUM];
		cardVecToPointArr(argCards, points);
		getHand(points, &hand);
	}
	history.push_back(make_pair(argSeat, hand));
	if (hand.type != NOTHING)
	{
		curHand = hand;
		curHandSeat = argSeat;
	}

	for (int i=0; i<3; ++i)
	{
		printPoints(playerInfo.points, '\n');
	}
	return true;
}

bool OGLordRobotAI::RbtOutGetTakeOutCard(std::vector<int> &vecCards)
{
	Hand hand;
	takeOutLvl0(hand);
	takeOutHand(hand, vecCards);
	cout << "take out hand: ";
	printHand(hand);
	cout << endl;
	for (unsigned i=0; i<vecCards.size(); ++i)
	{
		cout << vecCards[i] << " ";
	}
	cout << endl;

    //复检
	Hand checkHand;
	int checkPoints[CARD_POINT_NUM];
	cardVecToPointArr(vecCards, checkPoints);
	getHand(checkPoints, &checkHand);
	if (isFree())//第一局或者上盘没人接
	{
		if (checkHand.type == NOTHING)
		{
			findLowestHand(hand);
			takeOutHand(hand, vecCards);
		}
	}
	else
	{
		if (checkHand.type != NOTHING && !isHandHigherThan(checkHand, curHand))//判断出的牌是否比前一个人大
		{
			hand.type = NOTHING;
			takeOutHand(hand, vecCards);
		}
	}

	//cout << "remain cards after takeout: ";
	//for (unsigned i=0; i<aiCardsVec.size(); ++i)
	//{
	//	cout << aiCardsVec[i] << " ";
	//}
	//cout << endl;
	return true;
}

bool OGLordRobotAI::RbtResetData()
{
    aiSeat = -1;
    myScore = -1;
	lordSeat = -1;
	downSeat = -1;
	upSeat = -1;
	//leftoverCards.swap(vector<int>());
	//aiCardsVec.swap(vector<int>());
    memset((char*)&playerInfo, 0, sizeof(playerInfo));
	int remainPoints[CARD_POINT_NUM];
	std::map<HandType, std::vector<Hand> > handsMap;
	memset((char*)&summary, 0, sizeof(summary));
	uniHighHandCount.clear();
	highHandCount.clear();
	goodLowHandCount.clear();
	handsTrioCount = -1;
	controlNum = -1;
	maxOppNumber = -1;
	minOppNumber = -1;
	curScore = -1;
	curCaller = -1;
    memset((char*)&curHand, 0, sizeof(curHand));
	curHandSeat = -1;
	//callHistory.swap(vector<std::pair<int, int> >());
	//history.swap(vector<std::pair<int, Hand> >());
	return true;
}

void OGLordRobotAI::takeOutHand(Hand &hand, std::vector<int> &takeOutCards)
{
	int points[CARD_POINT_NUM] = {0};
	handToPointsArray(hand, points);
	takeOutCards.clear();
	for (vector<int>::iterator it=aiCardsVec.begin(); it!=aiCardsVec.end();)
	{
		int point = cardToPoint(*it);
		if (points[point] > 0)
		{
			--points[point];
			takeOutCards.push_back(*it);
			it = aiCardsVec.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void OGLordRobotAI::takeOutLvl0( Hand &hand )
{
	//sortHandMap();
	if (curHandSeat == aiSeat//要么是自己是地主，要么是出牌后没有要
		|| (aiPosition == LORD && curHandSeat < 0))
	{
		findLowestHand(hand);
	}
	else
	{
		findHigherHand(hand);
	}
}

void OGLordRobotAI::updateAiPosition()
{
	if (lordSeat == aiSeat)
	{
		aiPosition = LORD;
	}
	else
	{
		int diff = aiSeat - lordSeat;
		if (diff == 1 || diff == -2)
		{
			aiPosition = DOWNFARMER;
		}
		else
		{
			aiPosition = UPFARMER;
		}
	}
}

void OGLordRobotAI::findLowestHand( Hand &hand )
{
	hand.type = NOTHING;
	for (map<HandType, vector<Hand> >::iterator it=handsMap.begin(); it!=handsMap.end(); ++it)
	{
		vector<Hand> &hands = it->second;
		if (!hands.empty())
		{
			if (hand.type == NOTHING)
			{
				hand = hands[0];
			}
			else
			{
				if (hand.type == BOMB
					|| hands[0].keyPoint < hand.keyPoint
					|| (hands[0].keyPoint == hand.keyPoint
						&& getHandCount(hands[0]) > getHandCount(hand)))
				{
					hand = hands[0];
				}
			}
		}
	}
}

void OGLordRobotAI::findHigherHand(Hand &hand)
{
	findHigherHandNotBomb(hand);
	if (hand.type == NOTHING && (curHand.type != BOMB || curHand.type != NUKE))
	{
		vector<Hand> &bombs = handsMap[BOMB];
		if (!bombs.empty())
		{
			hand = bombs.front();
		}
	}
}

bool OGLordRobotAI::isFree()
{
	return (aiPosition == LORD && curHandSeat == -1) || curHandSeat == aiSeat;
}

void OGLordRobotAI::findHigherHandNotBomb( Hand &hand )
{
	hand.type = NOTHING;
	vector<Hand> &hands = handsMap[curHand.type];
	if (!hands.empty())
	{
		for (unsigned i=0; i<hands.size(); ++i)
		{
			if (isChain(curHand.type))
			{
				if (hands[i].keyPoint > curHand.keyPoint
					&& hands[i].len == curHand.len)
				{
					hand = hands[i];
					break;
				}
			}
			else
			{
				if (hands[i].keyPoint > curHand.keyPoint)
				{
					hand = hands[i];
					break;
				}
			}
		}
	}
}


