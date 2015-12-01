#pragma once
#include <vector>

namespace robot
{
const int ALL_CARDS_NUM = 54;

const int CARD_POINT_NUM = 16;

const int PLAYER_NUM = 3;

const int SOLO_CHAIN_CHARGE_LEN[8] = {7, 9, 11, 13, 15, 17, 18, 18};

const int LAIZI_TOTAL_COUNT = 4;

enum RobotStatus
{
    INIT = 0,   //刚初始化
    VERIFIED,   //验证通过
    INITGAME,   //游戏初始化通过
    CANSINGUP,  //可以报名
    SIGNUPED,   //报名成功
    GAMMING,    //正在游戏中
    EXITTING,   //正在退出游戏
    KEEPPLAY,   //准备断线续玩
    OTHER,      //闲置状态
};

//消息种类定义
enum msgID {
    NOTIFY_SWITCH_SCENE             = 5062, //进入游戏场景
    NOTIFY_STARTGAME                = 5050, //游戏开始
    NOTIFY_DEALCARD                 = 5055, // 发牌
    NOTIFY_BASECARD                 = 5056, // 发底牌
    NOTIFY_CALLSCORE                = 5057, // 叫分
    NOTIFY_SETLORD                  = 5058, // 地主确定
    NOTIFY_TAKEOUT                  = 5059, // 出牌
    NOTIFY_TRUST                    = 5053, // 进入托管
    NOTIFY_GAMEOVER                 = 5060, // 结束游戏
    MSGID_DDZ_GAME_RESULT_NTF       = 2100, // 游戏结果
    MSGID_DDZ_MATCH_OVER_NTF        = 2103, // 比赛结束

    MSGID_CALLSCORE_REQ             = 5001, // 叫分请求
    MSGID_CALLSCORE_ACK             = 5002, // 叫分请求结果

    MSGID_TAKEOUT_REQ               = 5003, // 出牌请求
    MSGID_TAKEOUT_ACK               = 5004, // 出牌请求结果

    MSGID_TRUST_CANCEL_REQ          = 5007, // 解除托管请求
    MSGID_TRUST_CANCEL_ACK          = 5008, // 解除托管请求结果

    MSGID_VERIFY_REQ                = 11,   //身份验证请求
    MSGID_VERIFY_ACK                = 12,   //身份验证请求应答

    MSGID_INIT_GAME_REQ             = 13,   //初始化游戏请求
    MSGID_INIT_GAME_ACK             = 14,   //初始化游戏请求应答

    MSGID_DDZ_SIGN_UP_CONDITION_REQ = 2011, // 查询报名条件请求
    MSGID_DDZ_SIGN_UP_CONDITION_ACK = 2012, // 查询报名条件应答

    MSGID_DDZ_SIGN_UP_REQ           = 2013, // 报名请求
    MSGID_DDZ_SIGN_UP_ACK           = 2014, // 报名应答

    MSGID_DDZ_CANCEL_SIGN_UP_REQ    = 2015, //取消报名请求
    MSGID_DDZ_CANCEL_SIGN_UP_ACK    = 2016, //取消报名请求应答

    MSGID_KEEP_REQ                  = 5009, // 断线续玩
    MSGID_KEEP_ACK                  = 5010, // 断线续玩应答

    MSGID_HEARTBEAT_NTF             = 10,   //心跳消息
};

enum CardPoint
{
	CARD_3 = 0,
	CARD_4,
	CARD_5,
	CARD_6,
	CARD_7,
	CARD_8,
	CARD_9,
	CARD_T,
	CARD_J,
	CARD_Q,
	CARD_K,
	CARD_A,
	CARD_2,
	BLACK_JOKER,
	RED_JOKER,		//14
	CARD_LZ,		//癞子
};


const char POINT_CHAR[CARD_POINT_NUM] = {'3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K', 'A', '2', 'X', 'D','L'};

enum HandType
{
	NOTHING,
	TRIO_CHAIN_PAIR, // 飞机带对子翅膀
	TRIO_CHAIN_SOLO, // 飞机带单牌翅膀
	TRIO_CHAIN, // 三顺(飞机不带翅膀)
	PAIR_CHAIN, // 双顺
	SOLO_CHAIN, // 单顺
	TRIO_PAIR, // 三带对
	TRIO_SOLO, // 三带单
	TRIO, // 三张
	PAIR, // 对子
	SOLO, // 单牌
	FOUR_DUAL_PAIR,  // 四带对
	FOUR_DUAL_SOLO,  // 四带单
	LZBOMB,	//癞子炸弹
	//RUANBOMB，//软炸,之后有时间填上
	BOMB,  // 炸弹
	NUKE,  // 火箭
	LZ,//癞子
};

enum Position
{
	LORD,
	UPFARMER,
	DOWNFARMER,
};

struct CardsInfo
{
	int points[CARD_POINT_NUM];
	int total;  //总共有多少张牌
	int control;
};

enum HandProp
{
	NORMAL,
	HIGHEST,
	UNI_HIGHEST,
};

struct Hand
{
	HandType type;  // 牌型
	CardPoint keyPoint;  // 套牌标签
	int len;
	CardPoint kicker[5];
	HandProp prob;
	int laizi_num;
	std::vector<int> m_vecReplaceCards;
};

struct HandsMapSummary
{
	int realHandsCount; //总套数，不包括炸弹
	int unChargeHandsCount; //非冲锋套数
	int extraBombCount; //非控制牌组成的炸弹数
	int twoControlsHandsCount; //有多张控制牌的套数
	int effectiveHandsCount; //有效套数
	int handsTypeCount; //牌型种类，不包括炸弹，单顺区分长度
	int soloCount; //单牌数
	CardPoint lowestSolo; //最小单牌
	CardPoint sencondLowestSolo; //次小单牌
};

struct HandsMapSummaryLvl
{
	int realHandsCount; //非炸弹套数和
	int bombCount; //炸弹数
	int effectiveHandsCount; //有效套数
	int handsTypeCount; //牌型种类，不包括炸弹，单顺区分长度
	int soloCount; //单牌数
	CardPoint lowestSolo; //最小单牌
	CardPoint sencondLowestSolo; //次小单牌
};

}
