#ifndef __CONNECTION_H__
#define __CONNECTION_H__
#include <boost/any.hpp>
#include "TcpEventServer.h"

class Conn;
class RobotBase;
class MsgNode;
class Entry;
typedef std::shared_ptr<Conn> ConnectionPtr;
typedef std::weak_ptr<Conn> ConnectionWeakPtr;
class Conn : public std::enable_shared_from_this<Conn>
{
    friend class TcpEventServer;
public:
    Conn(int fd = 0, struct bufferevent* bev = NULL)
        :fd_(fd), bev_(bev){}
    ~Conn() {}
    int fd_;
    std::shared_ptr<RobotBase> robot_;
    boost::any context_;
    void setContext(const boost::any& context) { context_ = context; }
    const boost::any& getContext() const { return context_; }

    int GetReadBufferLen() { return evbuffer_get_length(bufferevent_get_input(bev_)); }
    int GetReadBufferData(char* buffer, int msgLen);
    int AddToWriteBuffer(const std::string& buffer);
    bool BIsBufferExist() { return bev_ != nullptr; } 

private:
    struct bufferevent* bev_;
};

#endif /*__CONNECTION_H__*/