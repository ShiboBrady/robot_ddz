#include "TcpEventServer.h"
#include <cstdio>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "log.h"
using namespace std;

int Conn::GetReadBufferData(char* buffer, int msgLen)
{
    int len = bufferevent_read(bev_, buffer, msgLen);
    return len;
}

int Conn::AddToWriteBuffer(const char* buffer, int msgLen)
{
    int rtn = -1;
    if (NULL != bev_)
    {
        rtn = bufferevent_write(bev_, buffer, msgLen);
    }
    return rtn;
}

int TcpEventServer::InitConnection(std::string ip, int port, int connCount)
{
    ip_ = ip;
    port_ = port;
    connCount_ = connCount;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    inet_aton(ip_.c_str(), &server_addr.sin_addr);

    base_ = event_base_new();

    int index = 0;
    for (index = 0; index != connCount_; ++index)
    {
        struct bufferevent* bev = bufferevent_socket_new(base_, -1, BEV_OPT_CLOSE_ON_FREE);
        int rtn = bufferevent_socket_connect(bev, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if (rtn != 0)
        {
            //ERROR("Cannot connect to server, robot %d init failed, index is: %d.", robotIdStart_ + index, index);
            bufferevent_free(bev);
            --index;
            continue;
        }
        int fd = bufferevent_getfd(bev);
        evutil_make_socket_nonblocking(fd);
        bufferevent_setcb(bev, ReadEventCb, NULL, EventCb, this);
        bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
        mapConn_.insert(make_pair(fd, shared_ptr<Conn>(new Conn(fd, bev))));
        //DEBUG("Create a connection, index is: %d, init a Robot, Id is: %d.", index, robotIdStart_ + index);
    }
    return index;
}

void TcpEventServer::start()
{
    event_base_dispatch(base_);
}

void TcpEventServer::stop(MsgNode* msgNode)
{
    event_base_loopexit(base_, &(msgNode->timerEvent_));
    delete msgNode;
}

void TcpEventServer::disConnect(std::shared_ptr<Conn> conn)
{
    int fd = conn->fd_;
    bufferevent_disable(conn->bev_, EV_READ | EV_WRITE | EV_PERSIST);
    close(fd);
    bufferevent_free(conn->bev_);
    auto it = mapConn_.find(fd);
    mapConn_.erase(it);
}

bool TcpEventServer::addSignalEvent(void (*ptr)(int, short, void *), int signo)
{
    struct event* signal_event = evsignal_new(base_, signo, ptr, (void*)this);
    event_add(signal_event, NULL);
    return true;
}

bool TcpEventServer::addTimerEvent(void (*ptr)(int, short, void *), MsgNode* msgNode, bool once)
{
    int flag = 0;
    if( !once )
    {
        flag = EV_PERSIST;        
    }

    //新建定时器信号事件
    event_assign(&(msgNode->ev_timer_), base_, -1, flag, ptr, (void*)msgNode);
    evtimer_add(&(msgNode->ev_timer_), &(msgNode->timerEvent_));
    return true;
}

bool TcpEventServer::delTimerEvent(MsgNode* msgNode)
{
    evtimer_del(&(msgNode->ev_timer_));
    delete msgNode;
    return true;
}

void TcpEventServer::EventCb(struct bufferevent *bev, short event, void *arg)
{
    //Event();
    TcpEventServer* tcpServer = static_cast<TcpEventServer*>(arg);
    if (event & BEV_EVENT_EOF)
    {
        DEBUG("Connection closed.");
    }
    else if (event & BEV_EVENT_ERROR)
    {
        ERROR("Some other error.");
    }
    else if( event & BEV_EVENT_CONNECTED)
    {
        DEBUG("One client has connected to server.");
        return ;
    }

    int fd = bufferevent_getfd(bev);
    auto findRet = tcpServer->mapConn_.find(fd);
    if (tcpServer->mapConn_.end() != findRet)
    {
        bufferevent_free(bev);
        ERROR("One client disconnected.");
        bev = NULL;
        tcpServer->Event(findRet->second);
        tcpServer->mapConn_.erase(findRet);
    }
}

void TcpEventServer::ReadEventCb(struct bufferevent* bev, void* arg)
{
    TcpEventServer* tcpServer = static_cast<TcpEventServer*>(arg);
    int fd = bufferevent_getfd(bev);
    auto findRet = tcpServer->mapConn_.find(fd);
    if (tcpServer->mapConn_.end() == findRet)
    {
        //wrong
        return;
    }
    tcpServer->ReadEvent(findRet->second);
}

void TcpEventServer::WriteEventCb(struct bufferevent* bev, void* arg)
{
    //WriteEvent();
}

void TcpEventServer::CloseEventCb(struct bufferevent* bev, void* arg)
{
    //CloseEvent();
}