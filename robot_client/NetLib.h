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
#include "OGLordRobotAI.h"
#include "confaccess.h"

class NetLib
{
public:
    NetLib();
    ~NetLib();
    void start();
    std::string SerializeMsg( int msgId, const std::string& body );
    static void server_msg_cb(struct bufferevent* bev, void* arg);
    static void event_cb(struct bufferevent *bev, short event, void *arg);
    static void cmd_msg_cb(int fd, short events, void* arg);
    static void heart_beat_time_cb(int fd, short events, void* arg);
    static void init_time_cb(int fd, short events, void* arg);
    static void verify_time_cb(int fd, short events, void* arg);
    static void init_game_time_cb(int fd, short events, void* arg);
    static void sign_up_cond_time_cb(int fd, short events, void* arg);
    static void sign_up_time_cb(int fd, short events, void* arg);
private:
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

    //libevent基础数据结构
    struct event_base* base;
    std::map<struct bufferevent*, OGLordRobotAI> bevToRobot;
    struct sockaddr_in server_addr;

    //心跳的定时器
    struct event ev_timer_heart_beat;
    struct timeval timerEventHeartBeat;

    //初始化机器人的定时器
    struct event ev_timer_init;
    struct timeval timerEventInit;

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

    void connect();
    bool Init();
    void InitTimer();
};

#endif /*_NETLIB_H_*/