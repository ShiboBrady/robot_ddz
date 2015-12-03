#ifndef _NETLIB_H_
#define _NETLIB_H_

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include "Robot.h"
#include "confaccess.h"

class NetLib
{
public:
    NetLib();
    ~NetLib(){};
    void start();
    void stop();
    void ChangeStatusForSignUp(int robotNum);
    void SendSignUpCondReq(struct bufferevent* bev, Robot& robot);
    void SendSignUpReq(struct bufferevent* bev, Robot& robot);
    bool SerializeMsg( int msgId, const std::string& body, std::string& strRet );
    static void server_msg_cb(struct bufferevent* bev, void* arg);
    static void event_cb(struct bufferevent *bev, short event, void *arg);
    static void cmd_msg_cb(int fd, short events, void* arg);
    static void heart_beat_time_cb(int fd, short events, void* arg);
    static void init_time_cb(int fd, short events, void* arg);
    static void verify_time_cb(int fd, short events, void* arg);
    static void init_game_time_cb(int fd, short events, void* arg);
    static void sign_up_cond_time_cb(int fd, short events, void* arg);
    static void sign_up_time_cb(int fd, short events, void* arg);
    static void delay_send_msg_time_cb(int fd, short events, void* arg);
    static void query_room_state_time_cb(int fd, short events, void* arg);
    static void quick_game_time_cb(int fd, short events, void* arg);

private:

    typedef struct MsgNode{
        MsgNode(struct bufferevent* bev, const std::string& msg, int msgId, int robotId)
            :bev_(bev), msg_(msg), msgId_(msgId), robotId_(robotId){}
        struct bufferevent* bev_;
        std::string msg_;
        int msgId_;
        int robotId_;
        struct event ev_timer_delay_;
    }*pMsgNode, msgNode;

    std::string ip_;
    int port_;
    int robotNum_;
    int robotIQLevel_;
    int robotIdStart_;
    int robotIdEnd_;
    int heartBeatTime_;
    int initTime_;
    int verifyTime_;
    int initGameTime_;
    int signUpCondTime_;
    int signUpTime_;
    int delaySendActiveMsgTime_;
    int delaySendPassiveMsgTime_;
    int exitTime_;
    int quickGameTime_;
    int isMatch_;
    int roomStateTime_;

    //libevent基础数据结构
    struct event_base* base;
    std::map<struct bufferevent*, Robot> bevToRobot;   //负责出牌的机器人
    std::map<struct bufferevent*, Robot> headerRobot;  //负责调度的机器人
    struct sockaddr_in server_addr;

    //心跳的定时器
    struct event ev_timer_heart_beat;
    struct timeval timerEventHeartBeat;

    //验证身份的定时器
    struct event ev_timer_verify;
    struct timeval timerEventVerify;

    //初始化游戏的定时器
    struct event ev_timer_init_game;
    struct timeval timerEventInitGame;

    //查询报名条件的定时器
    struct event ev_timer_sign_in_cond;
    struct timeval timerEventSignInCond;

    //报名的定时器
    struct event ev_timer_sign_in;
    struct timeval timerEventSignIn;

    //首次叫分和出牌时延时发送消息的定时器
    //struct event ev_timer_delay_active_msg;
    struct timeval timerEventDelayActiveMsg;

    //收到叫分和出牌通知后延时发送消息的定时器
    //struct event ev_timer_delay_passive_msg;
    struct timeval timerEventDelayPassiveMsg;

    //在收到退出信号时，这个时间以后退出程序
    struct timeval timerEventExit;

    //查询房间状态的定时器
    struct event ev_timer_room_state;
    struct timeval timerEventRoomState;

    //快速比赛场定时器
    struct event ev_timer_quick_game;
    struct timeval timerEventQuickGame;

    void connect();
    bool Init();
    void InitTimer();
};

#endif /*_NETLIB_H_*/