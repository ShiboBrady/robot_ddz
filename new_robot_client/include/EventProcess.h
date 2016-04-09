#ifndef _EVENTPROCESS_H_
#define _EVENTPROCESS_H_

#include <string>
#include <vector>
#include <map>
#include <queue>
#include <list>
#include <unordered_set>
#include <boost/circular_buffer.hpp>

#include "TcpEventServer.h"

class RobotBase;
class CConfAccess;
class EventProcess {
public:
    EventProcess();
    ~EventProcess(){};
    void Start();
    bool RobotKeepPlayTest();
    bool InitHeaderRobotConnection();

    class Entry {
    public:
        explicit Entry(const ConnectionWeakPtr& weakConn, EventProcess* eventProcess)
            : weakConn_(weakConn),
              eventProcess_(eventProcess) {}

        ~Entry();
        ConnectionWeakPtr weakConn_;
        EventProcess* eventProcess_;
    };
    typedef std::shared_ptr<Entry> EntryPtr;
    typedef std::weak_ptr<Entry> WeakEntryPtr;
    typedef std::unordered_set<EntryPtr> Bucket;
    typedef boost::circular_buffer<Bucket> WeakConnectionList;
private:
    void RebuildConnection();
    bool InitTimingWheelTimer();
    static void TimingWheelOnTime(int fd, short events, void* arg);
    bool RobotGameInit();
    bool initParam();
    bool AddRobot(int iRobotNum, bool isTestKeepPlay = false);
    bool DelRobot(const ConnctionPtr& conn);
    bool DelRobotAndCloseConnection(const ConnctionPtr& conn);
    bool HeaderRobotDisconnect(const ConnctionPtr& conn);
    void MoveRobotFromTaskListToUntaskList(const ConnctionPtr& conn);
    void MoveRobotFromUntaskListToTaskList(const ConnctionPtr& conn);
    void MoveRobotFromUntaskList(const ConnctionPtr& conn);
    void ConnectEvent(const ConnctionPtr& conn);
    void OnReadEvent(const ConnctionPtr& conn);
    void CloseEvent(const ConnctionPtr& conn, bool isErrorOccurence);
    void SignalEvent();
    void InitRobotTimeCb(int fd, short events, void* arg);
    
    std::string ip_;
    int port_;
    int robotIQLevel_;
    int headerRobotId_;
    int taskRobotIdStart_;
    int taskRobotIdEnd_;
    int robotNum_;
    int heartBeatTime_;
    int delaySendActiveMsgTime_;
    int delaySendPassiveMsgTime_;

    TcpEventServer tcpEventServer_; //tcp server
    std::shared_ptr<RobotBase> headerRobot_;
    std::list<std::shared_ptr<RobotBase>> taskRobotList_;
    std::list<std::shared_ptr<RobotBase>> untaskRobotList_;
    CConfAccess* confAccess_;
    std::string strHeartBeatMsg_; //心跳消息，固定
    MsgNode* HeartBeatTimer;
    MsgNode* QueryRoomStateTimer;
    WeakConnectionList connectionBuckets_; //TimingWheelList.
};

#endif /*_EVENTPROCESS_H_*/