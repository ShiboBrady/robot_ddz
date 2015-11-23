#pragma once
/***********************************************\
斗地主游戏服务内置机器人接口 : COGLordRobotInterface
创建 : 2012-12-28 wangjian
COGLordRobotInterface 规定了斗地主游戏服务内置机器人的对外输入和输出接口

对外接口:
    1. 叫分阶段 : 游戏开始后一直到开始出牌之间的 AI逻辑处理 和 数据记录
        a):RbtInInitCard()             输入发牌信息
        b):RbtInNtfCardInfo()          输入各个座位玩家的牌信息
        c):RbtInCallScore()            输入玩家叫分信息
        d):RbtOutGetCallScore()        输出叫分策略
        e):RbtInSetLord()              输入确定地主信息
        f):RbtOutGetGrabLord()         输出抢地主策略
        g):RbtInSetGrabLord()          输入玩家抢地主信息
    2. 出牌阶段 : 游戏打牌阶段的 出牌AI逻辑 和 数据记录
        a):RbtInTakeOutCard()          输入玩家出牌信息
        b):RbtOutGetTakeOutCard()      输出出牌策略
    3. 重置操作 : 外部控制通过此部分接口传入座位和牌信息 ,重置/新建机器人对象
                   机器人需要通过这几个接口重新生成可以继续游戏的策略
        a):RbtInSetSeat()              输入游戏座位信息
        b):RbtInSetCard()              输入手牌信息
        b):RbtInTakeOutRecord()        输入游戏出牌记录
    4. 其他
        a):RbtOutGetLastError()        输出错误

强调 : 机器人只涉及牌逻辑,与游戏桌、玩家信息无关
        机器人必须不依赖任何类库
        区分各个机器人通过机器人在游戏中的座位号0,1,2
        机器人逻辑需要自我校验,所有外部参数都需要校验合法性
        机器人逻辑尽量跨平台
        所有以bool为返回值的接口 返回值true表示处理成功，FALSE表示处理失败，其他值无意义
\***********************************************/
#include <vector>
// 斗地主机器人基类 设计全被动式 所有操作都由外部游戏服务来调用 对外提供public接口 private为内部
class COGLordRobotAIInterface
{
public:
    // 【叫分阶段】
    // 收到发牌消息
    virtual bool RbtInInitCard( int argSeat,                        // 自己的座位号
                                std::vector<int> argHandCard        // 发送的手牌
                                ) = 0;

    // 输入机器人智商级别
    virtual bool RbtInSetLevel( int argLevel) = 0;                  // 机器人智商级别

    // 输入各个座位玩家的牌信息
    virtual bool RbtInNtfCardInfo(  std::vector<std::vector<int> > argHandCard
                                    // 各个座位玩家的手牌 0,1,2,3个元素分别表示第0,1,2号座位以及底牌的内容。
                                    ) = 0;

    // 收到玩家叫分信息
    virtual bool RbtInCallScore( int argSeat,                       // 座位号
                                 int argCallScore                   // 叫的分数
                                ) = 0;

    // 请求给出叫分策略
    virtual bool RbtOutGetCallScore( int &callScore,                 // 返回值引用
                                     int & delay                         // 延时时间
                                    ) = 0;

    // 输入玩家抢地主信息
    virtual bool RbtInSetGrabLord( int argSeat                      // 座位号
                                    ) = 0;

    // 输出抢地主策略
    virtual bool RbtOutGetGrabLord( bool &grabLord,                  // true：抢地主 FALSE：不抢
                                    int & delay                      // 延时时间
                                    ) = 0;
    virtual bool RbtInShowCard( int argShowSeat,                    // 明牌玩家座位
                                std::vector<int> argHandCard        // 玩家手牌
                                ) = 0;
    virtual bool RbtOutShowCard(bool &showCard, int &delay) = 0;                // 是否明牌 true:明牌 false:不明牌

    //输出加倍策略
    virtual bool RbtOutDoubleGame(bool &Double_game, int &delay) = 0;                // 是否明牌 true:明牌 false:不明牌


    // 收到确定地主信息
    virtual bool RbtInSetLord( int argLordSeat,                     // 地主座位
                               std::vector<int> argReceiceCard      // 地主收到的底牌
                              ) = 0;


    // 【出牌阶段】
    // 收到玩家牌信息
    virtual bool RbtInTakeOutCard( int argSeat,                     // 座位号
                                   std::vector<int> argCards        // 牌内容
                                  ) = 0;

    // 请求给出出牌策略
    virtual bool RbtOutGetTakeOutCard(std::vector<int> &vecCards,    // 返回的牌
                                      int & delay                    // 延时时间
                                      ) = 0;


    // 【重置机器人】
    // 用于第二盘时复用机器人AI 以及断线续完
    // 重置机器人信息
    virtual bool RbtResetData() = 0;

    // 设置地主和玩家座位
    virtual bool RbtInSetSeat( int argMySeat,                       // 自己的座位
                               int argLordSeat                      // 地主的座位
                              ) = 0;

    // 设置手牌和底牌
    virtual bool RbtInSetCard( std::vector<int> argInitCard,        // 自己初始化的牌
                               std::vector<int> argReceiveCard      // 地主收到的底牌
                              ) = 0;
    // 设置出牌记录
    virtual bool RbtInTakeOutRecord( std::vector<int> argTakeOutSeat,               // 历史出牌-座位记录
                                     std::vector<std::vector<int> > argTakeOutRecord // 历史出牌-牌记录
                                    ) = 0;

    // 【其他】
    // 返回错误信息
    virtual bool RbtOutGetLastError( int &errorCode) = 0;           // 返回当前错误码
};
