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
#include "RobotCenter.h"
#include "confaccess.h"

class NetLib
{
public:
    NetLib();
    ~NetLib();
    void connect();
    static void server_msg_cb(struct bufferevent* bev, void* arg);
    static void event_cb(struct bufferevent *bev, short event, void *arg);
    static void cmd_msg_cb(int fd, short events, void* arg);
    static void heart_beat_time_cb(int fd, short events, void* arg);
    static void init_time_cb(int fd, short events, void* arg);
    static void verify_time_cb(int fd, short events, void* arg);
    static void init_game_time_cb(int fd, short events, void* arg);
    static void sign_up_time_cb(int fd, short events, void* arg);
    void start();

private:
    typedef struct MsgNode
    {
        MsgNode(NetLib* netlib = NULL, int index = 0):netlib(netlib), index(index){}
        NetLib* netlib;
        int index;
    }msgNode, *pMsgNode;

    typedef struct BevNode
    {
        BevNode(NetLib* netlib = NULL, 
                std::vector<struct bufferevent*>* bev = NULL, 
                std::map<RobotCenter*, int>* unInitList = NULL,
                std::map<RobotCenter*, int>* unVerifyList = NULL, 
                std::map<RobotCenter*, int>* unInitGameList = NULL,
                std::map<RobotCenter*, int>* unSignUpList = NULL)
            :netlib(netlib),
             bev(bev),
             unInitList(unInitList),
             unInitGameList(unInitGameList),
             unVerifyList(unVerifyList),
             unSignUpList(unSignUpList){}
        NetLib* netlib;
        std::vector<struct bufferevent*>* bev;
        std::map<RobotCenter*, int>* unInitList;
        std::map<RobotCenter*, int>* unInitGameList;
        std::map<RobotCenter*, int>* unVerifyList;
        std::map<RobotCenter*, int>* unSignUpList;        
    }bevNode, *pBevNode;

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
    int signUpTime_;

    //libevent基础数据结构
    struct event_base* base;
    std::vector<struct bufferevent*> bev;
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

    //报名的定时器
    struct event ev_timer_sign_in;
    struct timeval timerEventSignIn;

    //网络连接序列号与机器人的对应关系
    std::map<int, RobotCenter> robotCenter;

    //队列：未进行任何操作的机器人列表
    std::map<RobotCenter*, int> unInitList;

    //队列：未验证通过的机器人队列
    std::map<RobotCenter*, int> unVerifyList;

    //队列：未初始化游戏的机器人列表
    std::map<RobotCenter*, int> unInitGameList;

    //队列：未报名成功的机器人队列
    std::map<RobotCenter*, int> unSignUpList;    

    bool Init();
};

#endif /*_NETLIB_H_*/