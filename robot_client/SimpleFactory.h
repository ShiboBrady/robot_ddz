#ifndef _SIMPLEFACTORY_H_
#define _SIMPLEFACTROY_H_

#include "AbstractProduct.h"

//消息种类定义
enum msgID {
    NOTIFY_STARTGAME                = 5050, //游戏开始
    NOTIFY_DEALCARD                 = 5055, // 发牌
    NOTIFY_BASECARD                 = 5056, // 发底牌
    NOTIFY_CALLSCORE                = 5057, // 叫分
    NOTIFY_SETLORD                  = 5058, // 地主确定
    NOTIFY_TAKEOUT                  = 5059, // 出牌
    NOTIFY_GAMEOVER                 = 5060, // 结束游戏

    MSGID_CALLSCORE_REQ             = 5001, // 叫分请求
    MSGID_CALLSCORE_ACK             = 5002, // 叫分请求结果
    MSGID_TAKEOUT_REQ               = 5003, // 出牌请求
    MSGID_TAKEOUT_ACK               = 5004, // 出牌请求结果

    MSGID_VERIFY_REQ                = 11,   //身份验证请求
    MSGID_VERIFY_ACK                = 12,   //身份验证请求应答

    MSGID_INIT_GAME_REQ             = 13,   //初始化游戏请求
    MSGID_INIT_GAME_ACK             = 14,   //初始化游戏请求应答

    MSGID_DDZ_SIGN_UP_CONDITION_REQ = 2011, // 查询报名条件请求
    MSGID_DDZ_SIGN_UP_CONDITION_ACK = 2012, // 查询报名条件应答

    MSGID_DDZ_SIGN_UP_REQ           = 2013, // 报名请求，对应消息 OrgRoomDdzSignUpReq
    MSGID_DDZ_SIGN_UP_ACK           = 2014, // 报名应答，对应消息 OrgRoomDdzSignUpAck
};

class SimpleFactory{
public:
    SimpleFactory(){};
    ~SimpleFactory(){};
    AbstractProduct* createProduct(int type);
};

#endif

