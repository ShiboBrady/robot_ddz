#ifndef _EVENTPROCESS_H_
#define _EVENTPROCESS_H_

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <list>
#include "Robot.h"
#include "TcpEventServer.h"

class CConfAccess;
class EventProcess : public TcpEventServer
{
public:
    EventProcess();
    ~EventProcess(){};

protected:
    virtual void Event(std::shared_ptr<Conn> conn, bool isErrorOccurence);
    virtual void ReadEvent(std::shared_ptr<Conn> conn);
    virtual void ReConnentEvent(std::shared_ptr<Conn> conn, int id);

private:
    void Verify(int robotId, int fd);
    void GameInit(int robotId, int fd);
    void InitParams();
    bool CheckConnCount();
    void InitRobot();
    void InitTimer();
    void InitSignal();
    bool SerializeMsg( int msgId, const string& body, string& strRet );
    void SendMsg(int msgId, const string& strRet, std::shared_ptr<Conn> conn);
    void SendQueryRoomStatusReq(std::shared_ptr<Conn> conn, bool isTimer);
    void ChangeStatusForRobot(int robotNum);
    void SendReqForRobot(std::shared_ptr<Conn> conn);

    static void signal_cb(int signo, short event, void *arg);
    static void heart_beat_time_cb(int fd, short events, void* arg);
    static void delay_send_msg_time_cb(int fd, short events, void* arg);
    static void query_room_state_time_cb(int fd, short events, void* arg);

    std::string ip_;
    int port_;
    int robotNum_;
    int robotIQLevel_;
    int robotIdStart_;
    int robotIdEnd_;
    int heartBeatTime_;
    int verifyTime_;
    int initGameTime_;
    int delaySendActiveMsgTime_;
    int delaySendPassiveMsgTime_;
    int exitTime_;
    int roomStateTime_;
    int matchId_;
    bool isMatch_;
    bool isTimeTrial_;

    CConfAccess* confAccess_;
    std::queue<pair<int, int> > taskQueue_;   //选择入场机器人队列
    int headerRobot_;             //负责调度的机器人

    MsgNode* HeartBeatTimer;
    MsgNode* VerifyTimer;
    MsgNode* InitGameTimer;
    MsgNode* QueryRoomStateTimer;
};

#endif /*_EVENTPROCESS_H_*/