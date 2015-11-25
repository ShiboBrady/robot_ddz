#ifndef _NETLIB_H_
#define _NETLIB_H_

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <string>
#include <vector>
#include <map>
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
    static void sign_up_time_cb(int fd, short events, void* arg);
    void start();

private:
    typedef struct MsgNode
    {
        MsgNode(NetLib* netlib, int index):netlib(netlib), index(index){}
        NetLib* netlib;
        int index;
    }msgNode, *pMsgNode;

    typedef struct BevNode
    {
        BevNode(NetLib* netlib, std::vector<struct bufferevent*> bev):netlib(netlib), bev(bev){}
        NetLib* netlib;
        std::vector<struct bufferevent*> bev;
    }bevNode, *pBevNode;

    std::string ip_;
    int port_;
    int robotNum_;
    int robotIQLevel_;
    int robotIdStart_;
    int robotIdEnd_;
    int heartBeatTime_;
    int signUpTime_;

    //libevent基础数据结构
    struct event_base* base;
    std::vector<struct bufferevent*> bev;
    struct sockaddr_in server_addr;

    //心跳的定时器
    struct event ev_timer_heart_beat;
    struct timeval timerEventHeartBeat;

    //报名的定时器
    struct event ev_timer_sign_in;
    struct timeval timerEventSignIn;

    std::map<int, RobotCenter> robotCenter;

    bool Init();
};

#endif /*_NETLIB_H_*/