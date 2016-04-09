#include "TcpEventServer.h"
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
#include "Connection.h"
#include "MsgPackage.h"
using namespace std;

TcpEventServer::TcpEventServer() {  
    
}

void TcpEventServer::Init(const string& ip, int port) {
    base_ = event_base_new();
    memset(&server_addr_, 0, sizeof(server_addr_) );
    server_addr_.sin_family = AF_INET;
    server_addr_.sin_port = htons(port);
    inet_aton(ip.c_str(), &server_addr_.sin_addr);
}

void TcpEventServer::Start() {
    event_base_dispatch(base_);
}

void TcpEventServer::Stop(MessagePackagePtr& msgNode) {
    event_base_loopexit(base_, msgNode->GetTimerP());
}

int TcpEventServer::CreateConnection() {
    struct bufferevent* bev = bufferevent_socket_new(base_, -1, BEV_OPT_CLOSE_ON_FREE);
    if (0 != bufferevent_socket_connect(bev, (struct sockaddr *)&server_addr_, sizeof(server_addr_))) {
        bufferevent_free(bev);
        ERROR("Create connect error.");
        return -1;
    }
    int fd = bufferevent_getfd(bev);
    evutil_make_socket_nonblocking(fd); //非阻塞
    evutil_make_listen_socket_reuseable(fd); //端口复用
    bufferevent_setcb(bev, ReadEventCb, NULL, EventCb, this);
    bufferevent_enable(bev, EV_READ | EV_WRITE | EV_PERSIST);
    mapConn_.insert(make_pair(fd, shared_ptr<Conn>(new Conn(fd, bev))));
    INFO("Successed connected a connection, fd is %d.", fd);
    return fd;
}

void TcpEventServer::CloseConnection(const ConnctionPtr& conn) {
    int fd = conn->fd_;
    bufferevent_disable(conn->bev_, EV_READ | EV_WRITE | EV_PERSIST);
    bufferevent_free(conn->bev_);
    DEBUG("Have close socket %d.", fd);
    conn->bev_ = nullptr;
    mapConn_.erase(fd);
}

bool TcpEventServer::AddSignalEvent(int signo, SingalFunp func) {
    signalCallback_ = func;
    struct event* signal_event = evsignal_new(base_, signo, singal_cb, (void*)this);
    event_add(signal_event, NULL);
    return true;
}

bool TcpEventServer::AddTimerEvent(eventFunc func, MsgNode* msgNode, bool once) {
    int flag = once ? 0 : EV_PERSIST;
    event_assign(msgNode->GetEventP(), base_, -1, flag, func, msgNode); //新建定时器信号事件
    evtimer_add(msgNode->GetEventP(), msgNode->GetTimerP());
    return true;
}

bool TcpEventServer::DelTimerEvent(MsgNode* msgNode) {
    evtimer_del(msgNode->GetEventP());
    return true;
}

bool TcpEventServer::SendHeartBeatMsg(int iSecond, const std::string& strHeartBeatMsg) {
    MsgNode* HeartBeatTimer = new MsgNode;
    HeartBeatTimer->SetTimer(iSecond, 0);
    HeartBeatTimer->SetObjectPoint(this);
    HeartBeatTimer->SetMsg(strHeartBeatMsg);
    AddTimerEvent(heartBeatTimeCb, HeartBeatTimer, false);
}

void TcpEventServer::heartBeatTimeCb(int fd, short events, void* arg) {
    MsgNode* oneMsgNode = static_cast<MsgNode*>(arg);
    TcpEventServer* eventProcess = static_cast<TcpEventServer*>(oneMsgNode->GetObjectPoint());
    for (auto it : eventProcess->mapConn_) {
        it.second->AddToWriteBuffer(oneMsgNode->GetMsg());
    }
    DEBUG("send once heard beat for all connection.");
}

void TcpEventServer::EventCb(struct bufferevent *bev, short event, void *arg) {
    bool isErrorOccurence = false;
    bool isConnectEvent = false;
    TcpEventServer* tcpServer = static_cast<TcpEventServer*>(arg);
    if (event & BEV_EVENT_EOF) {
        DEBUG("Connection closed.");
    } else if (event & BEV_EVENT_ERROR) {
        isErrorOccurence = true;
        ERROR("Some other error.");
    } else if( event & BEV_EVENT_CONNECTED) {
        DEBUG("One client has connected to server.");
        isConnectEvent = true;
    }

    auto findRet = tcpServer->mapConn_.find(bufferevent_getfd(bev));
    if (tcpServer->mapConn_.end() != findRet) {
        if (isConnectEvent) {
            tcpServer->connectionCallback_(findRet->second);
            DEBUG("One client connected.");
        } else {
            bufferevent_disable(bev, EV_READ | EV_WRITE | EV_PERSIST);
            bufferevent_free(bev);
            bev = nullptr;
            tcpServer->disconnectionCallback_(findRet->second, isErrorOccurence);
            tcpServer->mapConn_.erase(findRet);
        }
    } else {
        ERROR("There a event appear ,but doesn\'t has fd.");
    }
}

void TcpEventServer::ReadEventCb(struct bufferevent* bev, void* arg) {
    TcpEventServer* tcpServer = static_cast<TcpEventServer*>(arg);
    auto findRet = tcpServer->mapConn_.find(bufferevent_getfd(bev));
    if (tcpServer->mapConn_.end() == findRet) {
        ERROR("Read event cb error.");
        return;
    }
    DEBUG("Received a msg.");
    tcpServer->onReadCallback_(findRet->second);
}

void TcpEventServer::singal_cb(int signo, short event, void *arg) {
    TcpEventServer* eventProcess = static_cast<TcpEventServer*>(arg);
    eventProcess->signalCallback_();
}
