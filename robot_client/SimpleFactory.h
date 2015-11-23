#ifndef _SIMPLEFACTORY_H_
#define _SIMPLEFACTROY_H_

#include <stdio.h>
#include "AbstractProduct.h"

//消息种类定义
enum msgID {
    NOTIFY_STARTGAME              = 5050, //游戏开始
    NOTIFY_DEALCARD               = 5055, // 发牌
    NOTIFY_BASECARD               = 5056, // 发底牌
    NOTIFY_CALLSCORE              = 5057, // 叫分
    NOTIFY_SETLORD                = 5058, // 地主确定
    NOTIFY_TAKEOUT                = 5059, // 出牌
    NOTIFY_GAMEOVER               = 5060, // 结束游戏

    MSGID_CALLSCORE_REQ           = 5001, // 叫分请求
    MSGID_CALLSCORE_ACK           = 5002, // 叫分请求结果
    MSGID_TAKEOUT_REQ             = 5003, // 出牌请求
    MSGID_TAKEOUT_ACK             = 5004, // 出牌请求结果
};


class AbstractFactory{

public:
    AbstractFactory();
    virtual ~AbstractFactory();

public:
    virtual AbstractProduct* createProduct(int type) = 0;
};


class SimpleFactory:public AbstractFactory{

public:
    SimpleFactory();
    ~SimpleFactory();

public:
    AbstractProduct* createProduct(int type);
};

#endif

