#ifndef __ROBOTBASE_H__
#define __ROBOTBASE_H__

#include <map>
#include <memory>
#include <google/protobuf/message.h>
#include <functional>
#include "RobotConfig.h"
#include "confaccess.h"

class Conn;
class MsgNode;
class RobotBase {
public:
    enum RobotStatus
    {
        INIT = 0,   //刚初始化
        VERIFIED,   //验证通过
        INITGAME,   //游戏初始化通过
        WAITSIGNUP, //等待报名(等待报名快速开始游戏)
        CANSINGUP,  //可以报名
        SIGNUPED,   //报名成功
        QUICKGAME,  //等待快速开始游戏
        GAMMING,    //正在游戏中
        EXITTING,   //正在退出游戏
        KEEPPLAY,   //准备断线续玩
        OTHER,      //闲置状态
        HEADER,     //调度状态
    };

    typedef std::function<bool(std::shared_ptr<MsgNode>&)> ActionFuncp;
    typedef void(*eventFunc)(int, short, void*);
    typedef std::function<bool(eventFunc, MsgNode*, bool)> TimerFuncp;
    typedef std::function<bool(MsgNode* msgNode)> DelTimerFuncp;
    typedef std::function<bool(int)> AddRobotFunp;
    typedef std::function<bool(std::shared_ptr<Conn> conn)> DisconnectFunp;
    typedef std::function<bool(std::shared_ptr<Conn> conn)> DisconnectAndDelRobotFunp;

    RobotBase(int robotId);
    virtual ~RobotBase();
    
    virtual bool AnalysisAndDecision(std::shared_ptr<MsgNode>& msgNode) = 0;
    virtual bool NextActionProcess( std::shared_ptr<MsgNode>& msgNode ) = 0;
    virtual void RobotInit() = 0;
    virtual void DeleteTimer() = 0;

    static void ReceiveMsgTimeOutTimer(int fd, short events, void* arg);

    bool SetTimer(TimerFuncp func) { timerFunc_ = func; }
    bool DelTimer(DelTimerFuncp delTimerFunc) { delTimerFunc_ = delTimerFunc; }
    bool SetAddRobotCallback(AddRobotFunp addRobotCallback) { addRobotCallback_ = addRobotCallback; };
    void AddDisconnectCallback(DisconnectFunp disConnectCallback) { disConnectCallback_ = disConnectCallback; }
    void AddDisconnectAndDelRobotCallback(DisconnectAndDelRobotFunp disconnectAndDelRobotCallback) { disconnectAndDelRobotCallback_ = disconnectAndDelRobotCallback; }

    bool SendMsg(std::shared_ptr<MsgNode>& msgNode);

    bool GetHeartBeatMsg(std::shared_ptr<MsgNode>& msgNode);                 //获取心跳消息内容

    void SetRobotId( int robotId ) { robotId_ = robotId; }
    int GetRobotId() { return robotId_; }

    void SetNeedTestKeepPlay(bool isTestNeedKeepPlay) { isTestNeedKeepPlay_ = isTestNeedKeepPlay; }
    bool GetIsNeedTestKeepPlay() { return isTestNeedKeepPlay_; }

    int RobotProcess( std::shared_ptr<MsgNode>& msgNode );

    bool SendVerifyReq(std::shared_ptr<MsgNode>& msgNode);                    //验证消息发送
    bool RecvVerifyAck(std::shared_ptr<MsgNode>& msgNode);                    //验证消息回复

    bool SendInitGameReq(std::shared_ptr<MsgNode>& msgNode);                  //初始化游戏消息发送
    bool RecvInitGameAck(std::shared_ptr<MsgNode>& msgNode);                  //初始化游戏消息回复

protected:
    bool DeserializePbMsg(google::protobuf::Message* pbObject, const std::string& strMsg);

    bool SerializePbMsg(google::protobuf::Message* pbObject, std::string& strMsg);

    bool SerializeSendMsg(google::protobuf::Message* pbObject, int msgId, std::shared_ptr<MsgNode>& msgNode );

    bool AddMessageProcessFunc(robot::msgID iMsgId, ActionFuncp func){ 
        mapActionCallback_[iMsgId] = func;
    }

    int robotId_;
    RobotStatus status_;
    CConfAccess* confAccess_;
    bool bIsTimerInit_; //定时器是否初始化
    bool needKeepPlay_;    //是否需要断线续玩
    bool isTestNeedKeepPlay_; //检测是否需要断线续玩
    int matchId_; //比赛id
    bool isTimeTrail_;  //是否是定时赛
    bool isMatch_; //是否是比赛场
    std::map<int, ActionFuncp> mapActionCallback_; //处理消息函数
    TimerFuncp timerFunc_; //定时器回掉函数
    DelTimerFuncp delTimerFunc_; //删除定时器回掉函数
    AddRobotFunp addRobotCallback_; //增加机器人回掉函数
    DisconnectFunp disConnectCallback_; //断开连接回掉函数
    DisconnectAndDelRobotFunp disconnectAndDelRobotCallback_; //断开连接并删除机器人回掉函数
    MsgNode* ReceiveMsgTimeOutTimer_; //接收消息超时定时器
};

#endif /*__ROBOTBASE_H__*/