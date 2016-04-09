#include "EventProcess.h"
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
#include <sys/types.h>
#include <algorithm>
#include <boost/any.hpp>  

#include "stringutil.h"
#include "log.h"
#include "RobotConfig.h"
#include "confaccess.h"
#include "RobotBase.h"
#include "RobotHeader.h"
#include "RobotTask.h"
#include "Connection.h"
#include "MsgPackage.h"

using namespace std;
using namespace robot;
void RetryToCreateConnectionForHeaderRobot(int fd, short events, void* arg);

EventProcess::Entry::~Entry() {
    ConnectionPtr conn = weakConn_.lock();
    if (conn) {
        DEBUG("Will close connect for timeout robot %d.", conn->robot_->GetRobotId());
        eventProcess_->DelRobot(conn);
    }
}

EventProcess::EventProcess()
    :confAccess_(CConfAccess::GetConfInstance()),
     headerRobot_(0),
     connectionBuckets_(30) {
    initParam();  //初始化参数
}

bool EventProcess::initParam() {
    ip_ = confAccess_->GetIP();
    DEBUG("Ip is %s.", ip_.c_str());

    port_ = confAccess_->GetPort();
    DEBUG("Port is %d.", port_);

    robotNum_ = confAccess_->GetRobotNum();
    DEBUG("Robot num is %d.", robotNum_);

    robotIQLevel_ = confAccess_->GetIQLevel();
    DEBUG("Robot IQLevel is %d.", robotIQLevel_);

    int robotIdStart = confAccess_->GetRobotIdRangeStart();
    DEBUG("robotIdStart is %d.", robotIdStart);

    int robotIdEnd = confAccess_->GetRobotIdRangeEnd();
    DEBUG("robotIdEnd is %d.", robotIdEnd);

    if (robotIdStart <= 0 || robotIdEnd <= 0 || robotNum_ <= 0) {
        ERROR("robotid or robot num small than zero.");
        return false;
    }

    if (robotIdEnd < robotIdStart) {
        ERROR("robot id range is wrong.");
        return false;
    }

    if ((robotIdEnd - robotIdStart) < robotNum_) {
        robotNum_ = robotIdEnd - robotIdStart + 1;
        DEBUG("robotNum is changed to %d.", robotNum_);
    }

    headerRobotId_ = robotIdStart;
    DEBUG("Header robot Id is %d.", headerRobotId_);
    headerRobot_ = std::shared_ptr<RobotBase>(new RobotHeader(headerRobotId_)); //任务机器人初始化
    DEBUG("Header robot %d created.", headerRobotId_);
    headerRobot_->SetTimer(std::bind(&TcpEventServer::AddTimerEvent, &this->tcpEventServer_, placeholders::_1, placeholders::_2, placeholders::_3));
    headerRobot_->SetAddRobotCallback(std::bind(&EventProcess::AddRobot, this, placeholders::_1, false));
    headerRobot_->AddDisconnectCallback(std::bind(&EventProcess::HeaderRobotDisconnect, this, placeholders::_1));
    headerRobot_->DelTimer(std::bind(&TcpEventServer::DelTimerEvent, &this->tcpEventServer_, placeholders::_1));

    if (1 == robotNum_) {
        DEBUG("Only have header robot in program.");
    } else {
        taskRobotIdStart_ = robotIdStart + 1;
        taskRobotIdEnd_ = robotIdStart + robotNum_ - 1;
        DEBUG("Task robot id range is %d ~ %d.", taskRobotIdStart_, taskRobotIdEnd_);
        for (int robotIdIndex = taskRobotIdStart_; robotIdIndex <= taskRobotIdEnd_; ++robotIdIndex) { //任务机器人初始化
            shared_ptr<RobotBase> oneTaskRobot(new RobotTask(robotIdIndex, robotIQLevel_));
            oneTaskRobot->SetTimer(std::bind(&TcpEventServer::AddTimerEvent, &this->tcpEventServer_, placeholders::_1, placeholders::_2, placeholders::_3));
            oneTaskRobot->DelTimer(std::bind(&TcpEventServer::DelTimerEvent, &this->tcpEventServer_, placeholders::_1));
            oneTaskRobot->AddDisconnectCallback(std::bind(&EventProcess::DelRobot, this, placeholders::_1));
            oneTaskRobot->AddDisconnectAndDelRobotCallback(std::bind(&EventProcess::DelRobotAndCloseConnection, this, placeholders::_1));
            taskRobotList_.push_back(oneTaskRobot);
            DEBUG("Task robot %d created.", robotIdIndex);
        }
    }

    robotIQLevel_ = confAccess_->GetIQLevel();
    DEBUG("Robot IQ is %d.", robotIQLevel_);

    heartBeatTime_ = confAccess_->GetHeartBeatTime();
    DEBUG("Heart beat time is %d.", heartBeatTime_);

    tcpEventServer_.Init(ip_, port_);
    DEBUG("Tcp server Init.");

    MessagePackagePtr msgNode(new MsgNode);
    if (!headerRobot_->GetHeartBeatMsg(msgNode)) {
        ERROR("Get Heart beat msg error!");
        return false;
    }
    strHeartBeatMsg_ = msgNode->GetMsg();
    DEBUG("Heart beat msg have been set.");

    tcpEventServer_.SetConnctionCallback(std::bind(&EventProcess::ConnectEvent, this, placeholders::_1));
    tcpEventServer_.SetOnReadCallback(std::bind(&EventProcess::OnReadEvent, this, placeholders::_1));
    tcpEventServer_.SetDisconnectionCallback(std::bind(&EventProcess::CloseEvent, this, placeholders::_1, placeholders::_2));
    tcpEventServer_.AddSignalEvent(SIGINT, std::bind(&EventProcess::SignalEvent, this));

    connectionBuckets_.push_back(Bucket());
    return true;
}

void EventProcess::Start() {
    InitTimingWheelTimer(); //初始化TimmingWheel定时器
    InitHeaderRobotConnection(); //初始化机器人
    RobotKeepPlayTest(); //检测是否有需要断线续玩的机器人
    tcpEventServer_.SendHeartBeatMsg(heartBeatTime_, strHeartBeatMsg_); //初始化心跳发送器
    tcpEventServer_.Start(); //开始监听
}

bool EventProcess::RobotKeepPlayTest() {
    DEBUG("Begin to test robot is need keep play.");
    AddRobot(taskRobotList_.size(), true);
    DEBUG("Add all robot for need keep play test.");
}

bool EventProcess::InitTimingWheelTimer() {
    MsgNode* msgNode = new MsgNode;
    msgNode->SetTimer(10, 0);
    msgNode->SetObjectPoint(this);
    tcpEventServer_.AddTimerEvent(TimingWheelOnTime, msgNode, false);
}

bool EventProcess::InitHeaderRobotConnection() {
    int fd = tcpEventServer_.CreateConnection();
    if (-1 == fd) {
        ERROR("Header robot create connection failed, will retry.");
        RebuildConnection();
        return false;
    }
    auto& connmap = tcpEventServer_.GetConnection();
    auto findRet = connmap.find(fd);
    if (connmap.end() == findRet) {
        ERROR("Cannot find fd for header robot.");
        return false;
    }
    auto& conn = findRet->second;
    headerRobot_->RobotInit();
    conn->robot_ = headerRobot_;
    DEBUG("Header Robot connection is created.");
    return true;
}

void EventProcess::ConnectEvent(const ConnectionPtr& conn) {
    //新建一根连接
    if (conn->robot_->GetRobotId() != headerRobotId_) {
        EntryPtr entry(new Entry(conn, this));
        connectionBuckets_.back().insert(entry);
        WeakEntryPtr weakEntry(entry);
        conn->setContext(weakEntry);
        DEBUG("Robot %d create a connection.", conn->robot_->GetRobotId());
    }
    MessagePackagePtr msgNode(new MsgNode());
    msgNode->SetConn(conn);
    if (!conn->robot_->SendVerifyReq(msgNode)) {
        ERROR("Robot %d send verify msg error.", conn->robot_->GetRobotId());
        return;
    }
    DEBUG("Robot send a verify msg.");
}

void EventProcess::OnReadEvent(const ConnectionPtr& conn) {
    if (conn->robot_->GetRobotId() != headerRobotId_) {
        WeakEntryPtr weakEntry(boost::any_cast<WeakEntryPtr>(conn->getContext()));
        EntryPtr entry(weakEntry.lock());
        if (entry) {
            connectionBuckets_.back().insert(entry);
            DEBUG("Robot %d received a msg.", conn->robot_->GetRobotId());
        }
    }
    char msgLen[4] = {0};
    size_t len = 0;
    int iMsgLen = 0;
    int msgId = 0;
    int dataLength = conn->GetReadBufferLen();
    while (dataLength > 0) {
        memset(msgLen, '\0', 4);//读取msg length
        len = conn->GetReadBufferData(msgLen, 4);
        if (4 != len) {
            ERROR("Doesn't has 4 byte len info.");
            break;
        }
        int *pMsgLen = (int*)msgLen;
        iMsgLen = ntohl(*pMsgLen);
        if (0 == iMsgLen) {
            ERROR("Error! Convent data length failed.");
            break;
        }
        
        memset(msgLen, '\0', 4);//读取msgid
        len = conn->GetReadBufferData(msgLen, 4);
        if (4 != len) {
            ERROR("Doesn't has 4 byte MsgId info.");
            break;
        }
        pMsgLen = (int*)msgLen;
        msgId = ntohl(*pMsgLen);

        char* msg = new char[iMsgLen + 1];//读取消息体
        len = conn->GetReadBufferData(msg, iMsgLen);
        DEBUG("Receive %d byte from server for robot %d in message %d.", len, conn->robot_->GetRobotId(), msgId);
        
        dataLength = conn->GetReadBufferLen();//本次剩余未读字节
        DEBUG("Data still has length %d.", dataLength);

        string strMsg;
        strMsg.append(msg, iMsgLen);
        delete [] msg;
        MessagePackagePtr msgNode(new MsgNode);
        msgNode->SetMsg(strMsg);
        msgNode->SetMsgId(msgId);
        msgNode->SetConn(conn);
        int result = conn->robot_->RobotProcess(msgNode);
        if (robot::CLOSE_CONNECTION == result) {
            DEBUG("One robot disconnected, break this turn, still has %d byte in buffer.", dataLength);
            break;
        }
    }
}

void EventProcess::CloseEvent(const ConnectionPtr& conn, bool isErrorOccurence) {
    //断开一根连接
    int robotId = conn->robot_->GetRobotId();
    if (robotId == headerRobotId_) {
        DEBUG("Header robot disconnected, will repair connection.", conn->robot_->GetRobotId());
        conn->robot_->DeleteTimer(); //删除查询房间状态定时器
        RebuildConnection();
    } else {
        DEBUG("task tobot %d disconnected.", conn->robot_->GetRobotId());
        MoveRobotFromUntaskListToTaskList(conn);
    }
}

void EventProcess::RebuildConnection() {
    MsgNode* msgNode = new MsgNode();
    msgNode->SetTimer(10, 0);
    msgNode->SetObjectPoint(this);
    tcpEventServer_.AddTimerEvent(RetryToCreateConnectionForHeaderRobot, msgNode, true);
}

void EventProcess::SignalEvent() {
    DEBUG("Receved SIGINI signal, program will exit...");
    MessagePackagePtr ExitTimer(new MsgNode);
    ExitTimer->SetTimer(2, 0); //暂定2
    tcpEventServer_.Stop(ExitTimer);
}

bool EventProcess::AddRobot(int iRobotNum, bool isTestKeepPlay) {
    DEBUG("Will find %d robot for game.", iRobotNum);
    while (iRobotNum && taskRobotList_.size()) {
        int fd = tcpEventServer_.CreateConnection();
        if (-1 == fd) {
            ERROR("Task robot create connection failed.");
            return false;
        }
        auto& connmap = tcpEventServer_.GetConnection();
        auto findRet = connmap.find(fd);
        if (connmap.end() == findRet) {
            ERROR("Cannot find fd for task robot.");
            return false;
        }
        auto& conn = findRet->second;
        MoveRobotFromTaskListToUntaskList(conn);
        if (isTestKeepPlay) {
            conn->robot_->SetNeedTestKeepPlay(true);
        }
        iRobotNum--;
    }
    if (0 == iRobotNum) {
        DEBUG("Found enough robot.");
    } else {
        DEBUG("lack %d robot, and will wait for next game.", iRobotNum);
    }
    return true;
}

bool EventProcess::DelRobot(const ConnctionPtr& conn) {
    DEBUG("Begin to disconnected a connection for robot %d.", conn->robot_->GetRobotId());
    if (!conn->BIsBufferExist()) {
        ERROR("When robot %d assign to disconnect, connection doesn\'t exist.", conn->robot_->GetRobotId());
        return false;
    }
    MoveRobotFromUntaskListToTaskList(conn);
    tcpEventServer_.CloseConnection(conn);
    return true;
}

bool EventProcess::DelRobotAndCloseConnection(const ConnctionPtr& conn) {
    DEBUG("Begin to disconnected a connection and delete robot %d.", conn->robot_->GetRobotId());
    if (!conn->BIsBufferExist()) {
        ERROR("When robot %d assign to disconnect, connection doesn\'t exist.", conn->robot_->GetRobotId());
        return false;
    }
    tcpEventServer_.CloseConnection(conn);
    MoveRobotFromUntaskList(conn);
    return true;
}

void EventProcess::MoveRobotFromTaskListToUntaskList(const ConnctionPtr& conn) {
    conn->robot_ = taskRobotList_.front();
    conn->robot_->RobotInit();
    taskRobotList_.pop_front();
    int robotId = conn->robot_->GetRobotId();
    untaskRobotList_.push_back(conn->robot_);
    DEBUG("Has remore robot %d from task list, and add to untask list.", robotId);
    DEBUG("Task Robot %d connection is created.", robotId);
    DEBUG("Task list size : %d, Untask list size : %d.", taskRobotList_.size(), untaskRobotList_.size());
}

void EventProcess::MoveRobotFromUntaskListToTaskList(const ConnctionPtr& conn) {
    taskRobotList_.push_back(conn->robot_);
    int robotId = conn->robot_->GetRobotId();
    DEBUG("Has add robot %d to task list.", robotId);
    auto findRet = find_if (untaskRobotList_.begin(), untaskRobotList_.end(), [robotId](shared_ptr<RobotBase> r1){ return r1->GetRobotId() == robotId; });
    if (untaskRobotList_.end() != findRet) {
        untaskRobotList_.erase(findRet);
        DEBUG("Has remore robot %d from untask list.", robotId);
    }
    DEBUG("Task list size : %d, Untask list size : %d.", taskRobotList_.size(), untaskRobotList_.size());
}

void EventProcess::MoveRobotFromUntaskList(const ConnctionPtr& conn) {
    int robotId = conn->robot_->GetRobotId();
    auto findRet = find_if (untaskRobotList_.begin(), untaskRobotList_.end(), [robotId](shared_ptr<RobotBase> r1){ return r1->GetRobotId() == robotId; });
    if (untaskRobotList_.end() != findRet) {
        untaskRobotList_.erase(findRet);
        DEBUG("Has remore robot %d from untask list.", robotId);
    }
}

bool EventProcess::HeaderRobotDisconnect(const ConnctionPtr& conn) {
    //robot parse msg error.
    DEBUG("Header robot parse msg error action.");
    return true;
}

void EventProcess::InitRobotTimeCb(int fd, short events, void* arg) {
    MsgNode* oneMsgNode = static_cast<MsgNode*>(arg);
    EventProcess* eventProcess = static_cast<EventProcess*>(oneMsgNode->GetObjectPoint());
    eventProcess->InitHeaderRobotConnection(); //初始化headerrobot.
    delete oneMsgNode;
}

void EventProcess::TimingWheelOnTime(int fd, short events, void* arg) {
    MsgNode* oneMsgNode = static_cast<MsgNode*>(arg);
    EventProcess* eventProcess = static_cast<EventProcess*>(oneMsgNode->GetObjectPoint());
    eventProcess->connectionBuckets_.push_back(Bucket());
    DEBUG("TimmingWheel forward a cell.");
}

void RetryToCreateConnectionForHeaderRobot(int fd, short events, void* arg) {
    MsgNode* oneMsgNode = static_cast<MsgNode*>(arg);
    EventProcess* eventProcess = static_cast<EventProcess*>(oneMsgNode->GetObjectPoint());
    if (nullptr == eventProcess) {
        ERROR("Get EventProcess point failed.");
        return;
    }
    eventProcess->InitHeaderRobotConnection();
    DEBUG("Retry create connection for header robot once.");
    delete oneMsgNode;
}
