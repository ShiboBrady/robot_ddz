#include "RobotBase.h"
#include <iostream>
#include "log.h"
#include "TcpEventServer.h"
#include "OGLordRobotAI.h"
#include "AIUtils.h"
#include "stringutil.h"
#include "MsgPackage.h"
#include "Connection.h"

#include "PBGameDDZ.pb.h"
#include "connect.pb.h"
#include "org_room2client.pb.h"
#include "message.pb.h"

using namespace std;
using namespace robot;
using namespace PBGameDDZ;
using namespace YLYQ;
using namespace Protocol;
using namespace connect;
using namespace message;
using namespace org_room2client;
using namespace AIUtils;

void ReceiveMsgTimeOutTimer(int fd, short events, void* arg);

RobotBase::RobotBase(int robotId)
    :robotId_(robotId),
     status_(INIT),
     confAccess_(CConfAccess::GetConfInstance()),
     bIsTimerInit_(false),
     needKeepPlay_(false),
     isTestNeedKeepPlay_(false) {
    AddMessageProcessFunc(robot::MSGID_VERIFY_ACK, std::bind(&RobotBase::RecvVerifyAck, this, placeholders::_1));
    AddMessageProcessFunc(robot::MSGID_INIT_GAME_ACK, std::bind(&RobotBase::RecvInitGameAck, this, placeholders::_1));

    matchId_ = confAccess_->GetMatchId();
    isMatch_ = confAccess_->GetIsMatch();
    isTimeTrail_ = false; //暂定不检查定时赛
}

RobotBase::~RobotBase() {
}

bool RobotBase::GetHeartBeatMsg(std::shared_ptr<MsgNode>& msgNode) {      //发送心跳消息
    HeartbeatNtf heartbeatNtf;
    heartbeatNtf.set_rev("robot");  //暂且设为robot
    if (!SerializeSendMsg(&heartbeatNtf, robot::MSGID_HEARTBEAT_NTF, msgNode)) {
        ERROR("Generate heart beat msg error.");
        return false;
    }
    return true;
}

int RobotBase::RobotProcess(shared_ptr<MsgNode>& msgNode) {
    if (!msgNode) {
        ERROR("Robot receive a null msgnode.");
        return robot::FAIL;
    }
    auto iterMap = mapActionCallback_.find(msgNode->GetMsgId());
    if (mapActionCallback_.end() == iterMap) {
        return robot::FAIL;
    }
    YLYQ::Protocol::message::Message message;
    if (!DeserializePbMsg(&message, msgNode->GetMsg())) {
        ERROR("parse message pb message error.");
        return robot::FAIL;
    } 
    if (!message.has_body()) {
        ERROR("Doesn't has body info. msgId: %d", msgNode->GetMsgId());
        return robot::FAIL;
    }
    msgNode->SetMsg(message.body());
    if (!iterMap->second(msgNode)) { //消息处理失败
        ERROR("Robot %d process msg error, and will exit.", robotId_);
        status_ = INIT;
        disConnectCallback_(msgNode->GetConn()); //header robot 不做响应，task robot 断开连接
        return robot::CLOSE_CONNECTION;
    }
    if (!AnalysisAndDecision(msgNode)) { //对消息结果处理失败
        ERROR("Robot %d process msg result error.", robotId_);
        return true;
    }
    if (msgNode->GetGameIsOver()) {
        return robot::CLOSE_CONNECTION;
    }

    if (msgNode->GetIsNeedDelRobot()) { //需要删除机器人
        return robot::CLOSE_CONNECTION;
    }
    return true;
}

bool RobotBase::SendMsg(std::shared_ptr<MsgNode>& msgNode) {
    if (msgNode->GetConn()->BIsBufferExist()) {
        int SendRet = msgNode->GetConn()->AddToWriteBuffer(msgNode->GetMsg());
        DEBUG("Robot %d send msg once, msglen is %d, send result is %d.", robotId_, msgNode->GetMsg().length(), SendRet);
        return true;
    } else {
        ERROR("Connection doesn\'t exist.");
        return false;
    }
}

//验证消息
bool RobotBase::SendVerifyReq(shared_ptr<MsgNode>& msgNode) {
    INFO("=================== SendVerifyReq START =================");
    VerifyReq verifyReq;
    string strRobotId = StringUtil::Int2String(robotId_);
    string sessionkey = confAccess_->GetSessionKey();
    verifyReq.set_userid(strRobotId);
    verifyReq.set_sessionkey(sessionkey + strRobotId);
    DEBUG("robotId: %s, sessionkey: %s.", strRobotId.c_str(), sessionkey.c_str());
    if (!SerializeSendMsg(&verifyReq, robot::MSGID_VERIFY_REQ, msgNode)) {
        ERROR("Robot %d send verify string serialize failed.", robotId_);
        return false;
    }
    SendMsg(msgNode);
    DEBUG("Robot %d send verify once.", robotId_);
    INFO("=================== SendVerifyReq END =================");
    return true;
}

bool RobotBase::RecvVerifyAck(shared_ptr<MsgNode>& msgNode) {          //验证消息回复 
    //message VerifyAck {
    //    required int32 result = 1;
    //    optional string gameName = 2;       // 如果正在游戏中，返回游戏名称，客户端进行初始化，自动快速开始
    //}
    INFO("=================== RecvVerifyAck START =================");
    INFO("Message for robot %d.", robotId_);
    if (INIT != status_) { //只有INIT状态的机器人才能收到此消息
        ERROR("Robot %d doesn't in init status, robot status is: %s.", robotId_, STATUS_CHAR[status_]);
        return false;
    }
    VerifyAck verifyAck;
    if (!DeserializePbMsg(&verifyAck, msgNode->GetMsg())) {
        ERROR("parse verify ack pb message error.");
        return false;
    }
    int result = verifyAck.result();
    if (message::SUCCESS == result)  { //认证成功
        DEBUG("need keep play status: %d.", isTestNeedKeepPlay_);
        if (verifyAck.has_gamename()) {
            needKeepPlay_ = true; //有gamename字段，说明需要进行断线续玩操作.
            DEBUG("Robot %d is need keey play.", robotId_);
        } else if (isTestNeedKeepPlay_) {
            msgNode->SetNeedKeepPlay(false); //没有gamename字段，且是属于测试
            DEBUG("Robot %d doesn\'t keep play, and will exit.", robotId_);
            return true;
        }
        status_ = VERIFIED;
        INFO("Robot %d verify successed, status is: VERIFIED. Will send INITGAME request.", robotId_);
        if (!SendInitGameReq(msgNode)) {//发送游戏初始化消息
            ERROR("Robot %d Send Init game msg failed", robotId_);
            return false;
        }
    }
    else {
        ERROR("Robot %d verify failed, result is: %d.", robotId_, result);
        return false;
    }
    INFO("=================== RecvVerifyAck END =================");
    return true;
}

//初始化游戏消息
bool RobotBase::SendInitGameReq(shared_ptr<MsgNode>& msgNode)        //初始化游戏消息
{
    INFO("=================== SendInitGameReq START =================");
    string strType = confAccess_->GetGameType();
    string strName = confAccess_->GetGameName();
    DEBUG("Type: %s, name: %s.", strType.c_str(), strName.c_str());
    InitGameReq initGameReq;
    initGameReq.set_type(strType);
    initGameReq.set_name(strName);
    if (!SerializeSendMsg(&initGameReq, robot::MSGID_INIT_GAME_REQ, msgNode)) {
        ERROR("Robot %d send init game string serialize failed.");
        return false;
    }
    INFO("=================== SendInitGameReq END =================");
    return true;
}

bool RobotBase::RecvInitGameAck(std::shared_ptr<MsgNode>& msgNode){         //初始化游戏消息回复
    //message InitGameAck {
    //    required int32 result = 1;
    //}
    INFO("=================== RecvInitGameAck START =================");
    INFO("Message for robot %d.", robotId_);
    if (VERIFIED != status_) {//只有认证状态的机器人才能收到此消息
        ERROR("Robot %d doesn't in verified status, robot status is: %s.", robotId_, STATUS_CHAR[status_]);
        return false;
    }
    InitGameAck initGameAck;
    if (!DeserializePbMsg(&initGameAck, msgNode->GetMsg())) {
        ERROR("parse InitGame ack pb message error.");
        return false;
    }
    int result = initGameAck.result();
    if (message::SUCCESS != result) { //初始化失败
        ERROR("Robot %d Game Init failed, result is: %d.", robotId_, result);
        return false;
    } else { //初始化成功，判断是断线续玩，还是选择等待。
        INFO("Robot %d Game Init successed.", robotId_);
        if (needKeepPlay_) { //需要进行断线续玩操作.
            // if (!SendKeepPlayReq(msgNode)) {
            //     ERROR("Robot %d Send Keep play req failed.", robotId_);
            //     return false;
            // }
            status_ = KEEPPLAY;
            INFO("Set robot %d to KEEPPLAY status.", robotId_);
        } else { //不需要进行断线续玩操作
            status_ = INITGAME;
            INFO("Set robot %d to GameInit status.", robotId_);
        }
        if (!this->NextActionProcess(msgNode)) { //执行下一步动作
            return false;
        }
    }
    INFO("=================== RecvInitGameAck END =================");
    return true;
}

bool RobotBase::SendCancelSignUpReq(std::shared_ptr<MsgNode>& msgNode) {
    INFO("=================== SendCancelSignUpReq START =================");
    if (SIGNUPED == status_) {
        OrgRoomDdzCancelSignUpReq orgRoomDdzCancelSignUpReq;
        orgRoomDdzCancelSignUpReq.set_matchid(matchId_);
        if (!SerializeSendMsg(&orgRoomDdzCancelSignUpReq, robot::MSGID_DDZ_CANCEL_SIGN_UP_REQ, msgNode)) {
            return false;
        }
        SendMsg(msgNode);
    }
    INFO("=================== SendCancelSignUpReq END =================");
    return true;
}

bool RobotBase::DeserializePbMsg( google::protobuf::Message* pbObject, const std::string& strMsg ) {
    if (!pbObject->ParseFromString(strMsg)) {
        ERROR("Parse pb msg error!");
        return false;
    }
    return true;
}

bool RobotBase::SerializePbMsg( google::protobuf::Message* pbObject, std::string& strMsg ) {
    if (!pbObject->SerializeToString(&strMsg)) {
        ERROR("Serialized pb msg error!");
        return false;
    }
    return true;
}

bool RobotBase::SerializeSendMsg( google::protobuf::Message* pbObject, int msgId, shared_ptr<MsgNode>& msgNode ) {
    string strInbody;
    if (!SerializePbMsg(pbObject, strInbody)) {
        return false;
    }
    Message message;
    message.set_body( strInbody );
    message.mutable_head()->set_version(11111111);     //暂且设为1
    message.mutable_head()->set_sequence(22222222);    //暂且设为1
    message.mutable_head()->set_timestamp(33333333);   //暂且设为1
    string serializedStr;
    if (!SerializePbMsg(&message, serializedStr)) {
        return false;
    }

    int msgLen = int(serializedStr.length());
    msgLen = htonl(msgLen);
    msgId = htonl(msgId);
    string strOutRet;
    strOutRet.append((char*)&msgLen, sizeof(msgLen));
    strOutRet.append((char*)&msgId, sizeof(msgId));
    strOutRet.append(serializedStr.c_str(), (int)serializedStr.length());

    msgNode->SetMsg(strOutRet);
    msgNode->SetMsgNeedSend(true);
    return true;
}

void RobotBase::ReceiveMsgTimeOutTimer(int fd, short events, void* arg) {
    MsgNode* oneMsgNode = static_cast<MsgNode*>(arg);
    RobotBase* eventProcess = static_cast<RobotBase*>(oneMsgNode->GetObjectPoint());
    if (nullptr == eventProcess) {
        ERROR("Get RobotBase point failed.");
        return;
    }
    if (!eventProcess->disConnectCallback_(oneMsgNode->GetConn())) {
        ERROR("Robot %d timeout disconnect failed.", eventProcess->GetRobotId());
        return;
    }
    DEBUG("Robot %d timeout, and will exit.", eventProcess->GetRobotId());
}