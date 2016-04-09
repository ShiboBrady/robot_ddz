#ifndef __TCPEVENTSERVER_H__
#define __TCPEVENTSERVER_H__
#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <string>
#include <map>
#include <memory>
#include <functional>
#include "Connection.h"
#include "MsgPackage.h"

class TcpEventServer
{
public:
    typedef void(*eventFunc)(int, short, void*);
    typedef std::map<int, ConnctionPtr> connType;
    typedef std::function<void(const ConnctionPtr& conn)> ConnectionFunp;
    typedef std::function<void(const ConnctionPtr& conn)> OnReadFunp;
    typedef std::function<void(const ConnctionPtr& conn, bool isErrorOccurence)> DisconnectFunp;
    typedef std::function<void()> SingalFunp;
    TcpEventServer();
    ~TcpEventServer() {}

    connType& GetConnection() { return mapConn_; }
    static void singal_cb(int signo, short event, void *arg);

    int CreateConnection();
    void Init(const std::string& ip, int port);
    void Start();
    void Stop(MessagePackagePtr& msgNode);
    void CloseConnection(const ConnctionPtr& conn);
    bool AddSignalEvent(int signo, SingalFunp func);
    bool AddTimerEvent(eventFunc func, MsgNode* msgNode, bool once);
    bool DelTimerEvent(MsgNode* msgNode);
    bool SendHeartBeatMsg(int iSecond, const std::string& strHeartBeatMsg);

    void SetConnctionCallback(ConnectionFunp connectionCallback) { connectionCallback_ = connectionCallback; }
    void SetOnReadCallback(OnReadFunp onReadCallback) { onReadCallback_ = onReadCallback; };
    void SetDisconnectionCallback(DisconnectFunp disconnectionCallback) { disconnectionCallback_ = disconnectionCallback; }

private:
    std::string ip_;
    int port_;
    struct event_base* base_;
    struct sockaddr_in server_addr_;
    connType mapConn_;
    ConnectionFunp connectionCallback_;
    OnReadFunp onReadCallback_;
    DisconnectFunp disconnectionCallback_;
    SingalFunp signalCallback_;

    static void EventCb(struct bufferevent *bev, short event, void *arg);
    static void ReadEventCb(struct bufferevent* bev, void* arg);
    static void heartBeatTimeCb(int fd, short events, void* arg);
};

#endif /*__TCPEVENTSERVER_H__*/