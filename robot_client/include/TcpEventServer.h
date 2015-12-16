#ifndef __TCPEVENTSERVER_H__
#define __TCPEVENTSERVER_H__
#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <string>
#include <map>
#include <memory>
class Robot;
class Conn
{
    friend class TcpEventServer;
public:
    Conn(int fd = 0, struct bufferevent* bev = NULL)
        :fd_(fd), bev_(bev){}
    ~Conn() {}
    int fd_;
    std::shared_ptr<Robot> robot_;
    int GetReadBufferLen() { evbuffer_get_length(bufferevent_get_input(bev_)); }
    int GetReadBufferData(char* buffer, int len);
    int AddToWriteBuffer(const char* buffer, int len);

private:
    struct bufferevent* bev_;
};
class TcpEventServer;
class MsgNode
{
public:
    MsgNode(int sec = 0, int usec = 0, TcpEventServer* tcpEventServer = NULL, const std::string& msg = "", int msgId = 0, \
            std::shared_ptr<Conn> conn = std::shared_ptr<Conn>(NULL))
            :timerEvent_({sec, usec}), tcpEventServer_(tcpEventServer), msg_(msg), msgId_(msgId), conn_(conn){}
        struct event ev_timer_;
        struct timeval timerEvent_;
        TcpEventServer* tcpEventServer_;
        std::string msg_;
        int msgId_;
        std::shared_ptr<Conn> conn_;
        
};

class TcpEventServer
{
    typedef std::map<int, std::shared_ptr<Conn> > connType;
    typedef std::map<int, std::shared_ptr<Conn> >::iterator connTypeIt;
public:
    TcpEventServer()  {}
    ~TcpEventServer() {}

    int InitConnection(std::string ip, int port, int connCount);
    void start();
    void stop(MsgNode* msgNode);
    void disConnect(std::shared_ptr<Conn> conn);
    bool addSignalEvent(void (*ptr)(int, short, void *), int signo);
    bool addTimerEvent(void (*ptr)(int, short, void *), MsgNode* msgNode, bool once);
    bool delTimerEvent(MsgNode* msgNode);

protected:
    connType mapConn_;
    virtual void Event(std::shared_ptr<Conn> conn) {}
    virtual void ReadEvent(std::shared_ptr<Conn> conn) {}
    virtual void WriteEvent(std::shared_ptr<Conn> conn) {}
    virtual void CloseEvent(std::shared_ptr<Conn> conn) {}

private:    
    std::string ip_;
    int port_;
    int connCount_;
    struct event_base* base_;

    static void EventCb(struct bufferevent *bev, short event, void *arg);
    static void ReadEventCb(struct bufferevent* bev, void* arg);
    static void WriteEventCb(struct bufferevent* bev, void* arg);
    static void CloseEventCb(struct bufferevent* bev, void* arg);
};

#endif /*__TCPEVENTSERVER_H__*/