#ifndef _NETLIB_H_
#define _NETLIB_H_

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <string>
#include "RobotCenter.h"

class NetLib
{
public:
    NetLib(std::string ip, int port);
    ~NetLib();
    void connect();
    static void server_msg_cb(struct bufferevent* bev, void* arg);
    static void event_cb(struct bufferevent *bev, short event, void *arg);
    static void cmd_msg_cb(int fd, short events, void* arg);
    void start();

private:
    std::string ip;
    int port;
    struct event_base *base;
    struct bufferevent* bev;
    struct sockaddr_in server_addr;
    static RobotCenter robotCenter;
};

#endif /*_NETLIB_H_*/