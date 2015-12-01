#include "NetLib.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "message.pb.h"
#include "connect.pb.h"
#include "org_room2client.pb.h"
#include "stringutil.h"
#include "log.h"
#include "RobotConfig.h"

using namespace std;
using namespace YLYQ;
using namespace Protocol;
using namespace message;
using namespace connect;
using namespace org_room2client;
using namespace robot;

NetLib::NetLib()
{
    if (!Init())
    {
        ERROR("Get params error, please check configure file!");
        ::exit(0);
    }
    memset(&server_addr, 0, sizeof(server_addr) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    inet_aton(ip_.c_str(), &server_addr.sin_addr);
    base = event_init();
    DEBUG("ip: %s, port: %d.", ip_.c_str(), port_);
    connect();
    InitTimer();
}

void NetLib::connect()
{
    int index = 0;
    for (index = 0; index != robotNum_; ++index)
    {
        if (robotIdStart_ + index <= robotIdEnd_)
        {
            pair< std::map<struct bufferevent*, Robot>::iterator, bool> insertResult;
            insertResult = bevToRobot.insert(make_pair(bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE), Robot(robotIdStart_ + index, robotIQLevel_)));
            if (!insertResult.second)
            {
                //插入失败
                ERROR("Insert into bevToRobot failed.");
                ::exit(0);
            }
            bufferevent_socket_connect(insertResult.first->first, (struct sockaddr *)&server_addr, sizeof(server_addr));
            bufferevent_setcb(insertResult.first->first, server_msg_cb, NULL, event_cb, this);
            bufferevent_enable(insertResult.first->first, EV_READ | EV_WRITE);
            DEBUG("Create a connection, index is: %d, init a Robot, Id is: %d.", index, robotIdStart_ + index);
        }
        else
        {
            WARN("Robot Id range is bigger than robot num.");
            break;
        }
    }
    DEBUG("Total init %d robots and connections.", index);
}

void NetLib::start()
{
    event_base_dispatch(base);
}

void NetLib::stop()
{
    //message OrgRoomDdzCancelSignUpReq {
    //    required int32 matchId = 1;     // 比赛 ID
    //}
    string strMatchId;
    CConfAccess* confAccess = CConfAccess::GetConfInstance();
    confAccess->GetValue("game", "matchid", strMatchId, "1000");
    OrgRoomDdzCancelSignUpReq orgRoomDdzCancelSignUpReq;
    orgRoomDdzCancelSignUpReq.set_matchid(::atoi(strMatchId.c_str()));
    string serializedStr;
    orgRoomDdzCancelSignUpReq.SerializeToString(&serializedStr);
    string strSend;
    SerializeMsg(robot::MSGID_DDZ_CANCEL_SIGN_UP_REQ, serializedStr, strSend);

    std::map<struct bufferevent*, Robot>::iterator it;
    for (it = bevToRobot.begin(); it != bevToRobot.end(); ++it)
    {
        if (SIGNUPED == (it->second).GetStatus())
        {
            bufferevent_write(it->first, strSend.c_str(), strSend.length());
            DEBUG("Send unsign req for robot %d.", (it->second).GetRobot().GetRobotId());
            (it->second).SetStatus(EXITTING);
        }
    }
    event_base_loopexit(base, &timerEventExit);
}

bool NetLib::SerializeMsg( int msgId, const string& body, string& strRet )
{
    Message message;
    message.set_body( body );
    message.mutable_head()->set_version(1);     //暂且设为1
    message.mutable_head()->set_sequence(1);    //暂且设为1
    message.mutable_head()->set_timestamp(1);   //暂且设为1
    string serializedStr;
    if (!message.SerializeToString(&serializedStr))
    {
        ERROR("Serialized protobuf msg failed.");
        return false;
    }

    if (!message.IsInitialized())
    {
        ERROR("Isn't a legal protobuf packet.");
        return false;
    }

    int msgLen = int(serializedStr.length());
    msgLen = htonl(msgLen);
    msgId = htonl(msgId);

    strRet.append((char*)&msgLen, sizeof(msgLen));
    strRet.append((char*)&msgId, sizeof(msgId));
    strRet.append(serializedStr.c_str(), (int)serializedStr.length());
    return true;
}

void NetLib::server_msg_cb(struct bufferevent* bev, void* arg)
{
    /*消息的格式
    +----------------+----------+----------------------------+
    +  len(Message)  |   msgId  |           Message          |
    +----------------+----------+-----------+----------------+
    +                           |   Head    |     body       |
    +----------------+----------+-----------+----------------+
    +    4 bytes     |  4 bytes |  12 bytes | len - 12 bytes |
    +----------------+---------------------------------------+
    */

    //获取参数内容
    NetLib* netlib = static_cast<NetLib*>(arg);

    //查找消息对应的机器人
    map<struct bufferevent*, Robot>::iterator it = (netlib->bevToRobot).find(bev);
    if ((netlib->bevToRobot).end() == it)
    {
        ERROR("Cannot find robot.");
        return;
    }

    char msgLen[4] = {0};
    size_t len = 0;
    int iMsgLen = 0;
    int msgId = 0;
    int dataLength = evbuffer_get_length(bufferevent_get_input(bev));
    //DEBUG("Reveive data length is: %d.", dataLength);
    while (dataLength > 0)
    {
        //读取msg length
        memset(msgLen, '\0', 4);
        len = bufferevent_read(bev, msgLen, 4);
        if (4 != len)
        {
            ERROR("Doesn't has 4 byte len info.");
            break;
        }
        int *pMsgLen = (int*)msgLen;
        iMsgLen = ntohl(*pMsgLen);
        if (0 == iMsgLen)
        {
            ERROR("Error! Convent data length failed.");
            break;
        }
        //DEBUG("Msg len: %d.", iMsgLen);

        //读取msgid
        memset(msgLen, '\0', 4);
        len = bufferevent_read(bev, msgLen, 4);
        if (4 != len)
        {
            ERROR("Doesn't has 4 byte MsgId info.");
            break;
        }
        pMsgLen = (int*)msgLen;
        msgId = ntohl(*pMsgLen);
        //DEBUG("MsgId is: %d.", msgId);

        //读取消息体
        char* msg = new char[iMsgLen + 1];
        len = bufferevent_read(bev, msg, iMsgLen);
        DEBUG("Receive %d byte from server for robot %d in message %d.", len, (it->second).GetRobot().GetRobotId(), msgId);

        string strMsg;
        strMsg.append(msg, iMsgLen);
        delete [] msg;
        string strRet;
        bool result = (it->second).RobotProcess(msgId, strMsg, strRet);

        if (result)
        {
            int msgIdBak = msgId;
            if (NOTIFY_CALLSCORE == msgId || NOTIFY_DEALCARD == msgId)
            {
                msgId = MSGID_CALLSCORE_REQ;
            }
            else if (NOTIFY_TAKEOUT == msgId || NOTIFY_BASECARD == msgId)
            {
                msgId = MSGID_TAKEOUT_REQ;
            }
            else if (NOTIFY_TRUST == msgId || MSGID_KEEP_ACK == msgId)
            {
                msgId = MSGID_TRUST_CANCEL_REQ;
            }
            else if (robot::MSGID_VERIFY_ACK == msgId)
            {
                msgId = robot::MSGID_KEEP_REQ;
            }
            string strSend;
            result = netlib->SerializeMsg(msgId, strRet, strSend);
            if (!result)
            {
                ERROR("Serialize message to be send failed.");
            }
            else
            {
                if (NOTIFY_DEALCARD == msgIdBak || NOTIFY_BASECARD == msgIdBak)
                {
                    //添加主动消息延时发送消息的定时器
                    pMsgNode oneMsgNode = new msgNode(bev, strSend, msgId, (it->second).GetRobot().GetRobotId());
                    evtimer_set(&(oneMsgNode->ev_timer_delay_), delay_send_msg_time_cb, oneMsgNode);
                    event_add(&(oneMsgNode->ev_timer_delay_), &(netlib->timerEventDelayActiveMsg));
                    DEBUG("Add a timer, will delay send active message: %d to server for robot %d.", msgId, (it->second).GetRobot().GetRobotId());
                }
                else if (NOTIFY_CALLSCORE == msgIdBak || NOTIFY_TAKEOUT == msgIdBak)
                {
                    //添加被动消息延时发送消息的定时器
                    pMsgNode oneMsgNode = new msgNode(bev, strSend, msgId, (it->second).GetRobot().GetRobotId());
                    evtimer_set(&(oneMsgNode->ev_timer_delay_), delay_send_msg_time_cb, oneMsgNode);
                    event_add(&(oneMsgNode->ev_timer_delay_), &(netlib->timerEventDelayPassiveMsg));
                    DEBUG("Add a timer, will delay send passive message: %d to server for robot %d.", msgId, (it->second).GetRobot().GetRobotId());
                }
                else
                {
                    //无需延迟，直接发送
                    int iSendResult = bufferevent_write(bev, strSend.c_str(), strSend.length());
                    DEBUG("Send immediate message: %d to server for robot %d, send result: %d.", msgId, (it->second).GetRobot().GetRobotId(), iSendResult);
                }
            }
        }
        dataLength = evbuffer_get_length(bufferevent_get_input(bev));
        //DEBUG("Data still has length %d.", dataLength);
    }
    //DEBUG("Read data this time over.");
}

void NetLib::heart_beat_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);

    HeartbeatNtf heartbeatNtf;
    heartbeatNtf.set_rev("robot");  //暂且设为robot
    string serializedStr;
    heartbeatNtf.SerializeToString(&serializedStr);
    string strSend;
    netlib->SerializeMsg(robot::MSGID_HEARTBEAT_NTF, serializedStr, strSend);

    std::map<struct bufferevent*, Robot>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        bufferevent_write(it->first, strSend.c_str(), strSend.length());
    }
    DEBUG("Send once heard beat.");
    event_add(&(netlib->ev_timer_heart_beat), &(netlib->timerEventHeartBeat));/*重新添加定时器*/
}

void NetLib::verify_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);

    string strSessionKey;
    CConfAccess* confAccess = CConfAccess::GetConfInstance();
    bool bResult = confAccess->GetValue("sessionKey", "sessionKey", strSessionKey, "session_");

    bool bHasUnviryfiedRobot = false;
    VerifyReq verifyReq ;
    std::map<struct bufferevent*, Robot>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        if (INIT == (it->second).GetStatus())
        {
            string robotId = StringUtil::Int2String((it->second).GetRobot().GetRobotId());
            verifyReq.Clear();
            verifyReq.set_userid(robotId);
            verifyReq.set_sessionkey(StringUtil::Trim(strSessionKey) + robotId);
            string serializedStr;
            verifyReq.SerializeToString(&serializedStr);
            string strSend;
            netlib->SerializeMsg(robot::MSGID_VERIFY_REQ, serializedStr, strSend);
            bufferevent_write(it->first, strSend.c_str(), strSend.length());
            DEBUG("Robot %d send verify once.", (it->second).GetRobot().GetRobotId());
            bHasUnviryfiedRobot = true;
        }
    }
    if (bHasUnviryfiedRobot)
    {
        event_add(&(netlib->ev_timer_verify), &(netlib->timerEventVerify));/*重新添加定时器*/
    }
}

void NetLib::init_game_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);

    string strType;
    string strName;
    CConfAccess* confAccess = CConfAccess::GetConfInstance();
    confAccess->GetValue("game", "type", strType, "ddz");
    confAccess->GetValue("game", "name", strName, "org_ddz_match");

    InitGameReq initGameReq;
    initGameReq.set_type(StringUtil::Trim(strType).c_str());
    initGameReq.set_name(StringUtil::Trim(strName).c_str());
    string serializedStr;
    initGameReq.SerializeToString(&serializedStr);
    string strSend;
    serializedStr = netlib->SerializeMsg(robot::MSGID_INIT_GAME_REQ, serializedStr, strSend);

    bool bHasUnInitedRobot = false;
    std::map<struct bufferevent*, Robot>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        if (VERIFIED == (it->second).GetStatus())
        {
            bufferevent_write(it->first, strSend.c_str(), strSend.length());
            DEBUG("Robot %d send init game once.", (it->second).GetRobot().GetRobotId());
            bHasUnInitedRobot = true;
        }
    }
    if (bHasUnInitedRobot)
    {
        event_add(&(netlib->ev_timer_init_game), &(netlib->timerEventInitGame));/*重新添加定时器*/
    }
}

void NetLib::sign_up_cond_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);
    string strMatchId;
    CConfAccess* confAccess = CConfAccess::GetConfInstance();
    confAccess->GetValue("game", "matchid", strMatchId, "1000");

    OrgRoomDdzSignUpConditionReq orgRoomDdzSignUpConditionReq;
    orgRoomDdzSignUpConditionReq.set_matchid(::atoi(strMatchId.c_str()));
    string serializedStr;
    orgRoomDdzSignUpConditionReq.SerializeToString(&serializedStr);
    string strSend;
    serializedStr = netlib->SerializeMsg(robot::MSGID_DDZ_SIGN_UP_CONDITION_REQ, serializedStr, strSend);

    std::map<struct bufferevent*, Robot>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        if (INITGAME == (it->second).GetStatus())
        {
            bufferevent_write(it->first, strSend.c_str(), strSend.length());
            DEBUG("Robot %d send init sign up cond req once.", (it->second).GetRobot().GetRobotId());
        }
    }
    event_add(&(netlib->ev_timer_sign_in_cond), &(netlib->timerEventSignInCond));/*重新添加定时器*/
}

void NetLib::sign_up_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);
    string strMatchId;
    CConfAccess* confAccess = CConfAccess::GetConfInstance();
    confAccess->GetValue("game", "matchid", strMatchId, "1000");

    OrgRoomDdzSignUpReq orgRoomDdzSignUpReq;
    orgRoomDdzSignUpReq.set_matchid(::atoi(strMatchId.c_str()));

    std::map<struct bufferevent*, Robot>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        if (CANSINGUP == (it->second).GetStatus())
        {
            orgRoomDdzSignUpReq.set_costid((it->second).GetCost());
            string serializedStr;
            orgRoomDdzSignUpReq.SerializeToString(&serializedStr);
            string strSend;
            serializedStr = netlib->SerializeMsg(robot::MSGID_DDZ_SIGN_UP_REQ, serializedStr, strSend);

            bufferevent_write(it->first, strSend.c_str(), strSend.length());
            DEBUG("Robot %d send init sign up req once.", (it->second).GetRobot().GetRobotId());
        }
    }
    event_add(&(netlib->ev_timer_sign_in), &(netlib->timerEventSignIn));/*重新添加定时器*/
}

void NetLib::delay_send_msg_time_cb(int fd, short events, void* arg)
{
    pMsgNode oneMsgNode = static_cast<pMsgNode>(arg);
    if (NULL != oneMsgNode->bev_)
    {
        int iSendResult = bufferevent_write(oneMsgNode->bev_, (oneMsgNode->msg_).c_str(), (oneMsgNode->msg_).length());
        DEBUG("Send delay message: %d to server for robot %d, write %d size, send result is %d.", \
            oneMsgNode->msgId_, oneMsgNode->robotId_, (oneMsgNode->msg_).length(), iSendResult);
    }
    delete oneMsgNode;
}

void NetLib::event_cb(struct bufferevent *bev, short event, void *arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);
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

    //查找消息对应的机器人
    map<struct bufferevent*, Robot>::iterator it = (netlib->bevToRobot).find(bev);
    int robotId;
    bool exist = false;
    Robot robot = it->second;
    if ((netlib->bevToRobot).end() != it)
    {
        exist = true;
        robotId = it->second.GetRobot().GetRobotId();

        //这将自动close套接字和free读写缓冲区
        bufferevent_free(bev);
        DEBUG("Robot %d disconnected.", (it->second).GetRobot().GetRobotId());
        bev = NULL;
        (netlib->bevToRobot).erase(it);
    }

    if (!exist)
    {
        //没有机器人断开连接
        return;
    }

    //尝试断线续连
    pair< std::map<struct bufferevent*, Robot>::iterator, bool> insertResult;
    insertResult = (netlib->bevToRobot).insert(make_pair(bufferevent_socket_new(netlib->base, -1, BEV_OPT_CLOSE_ON_FREE), robot));
    if (!insertResult.second)
    {
        //插入失败
        ERROR("Insert into bevToRobot for robot %d failed.", robotId);
    }
    else
    {
        bufferevent_socket_connect((insertResult.first)->first, (struct sockaddr *)&(netlib->server_addr), sizeof(netlib->server_addr));
        bufferevent_setcb((insertResult.first)->first, server_msg_cb, NULL, event_cb, arg);
        bufferevent_enable((insertResult.first)->first, EV_READ | EV_WRITE);
        DEBUG("Create connection for robot %d again", robot.GetRobot().GetRobotId());
    }
}

bool NetLib::Init()
{
    std::string strPort;
    std::string strRobotNum;
    std::string strRobotIQLevel;
    std::string strRobotIdStart;
    std::string strRobotIdEnd;
    std::string strHeartBeatTime;
    std::string strVerifyTime;
    std::string strInitGameTime;
    std::string strSignUpCondTime;
    std::string strSignUpTime;
    std::string strDelaySendActiveTime;
    std::string strDelaySendPassiveTime;
    std::string strExitTime;

    bool bResult = false;
    CConfAccess* confAccess = CConfAccess::GetConfInstance();
    bResult = confAccess->GetValue("robot", "robotIdStart", strRobotIdStart, "110001");
    bResult = confAccess->GetValue("robot", "robotIdEnd", strRobotIdEnd, "110001");
    bResult = confAccess->GetValue("robot", "robotNum", strRobotNum, "1");
    bResult = confAccess->GetValue("robot", "IQLevel", strRobotIQLevel, "0");
    bResult = confAccess->GetValue("server", "ip", ip_, "127.0.0.1");
    bResult = confAccess->GetValue("server", "port", strPort, "9999");
    bResult = confAccess->GetValue("timer", "heartBeat", strHeartBeatTime, "30");
    bResult = confAccess->GetValue("timer", "verify", strVerifyTime, "30");
    bResult = confAccess->GetValue("timer", "initGame", strInitGameTime, "30");
    bResult = confAccess->GetValue("timer", "signUpCond", strSignUpCondTime, "30");
    bResult = confAccess->GetValue("timer", "signUp", strSignUpTime, "30");
    bResult = confAccess->GetValue("timer", "activeMsgDelay", strDelaySendActiveTime, "8");
    bResult = confAccess->GetValue("timer", "passiveMsgDelay", strDelaySendPassiveTime, "5");
    bResult = confAccess->GetValue("timer", "exit", strExitTime, "2");
    if (bResult)
    {
        port_ = ::atoi(strPort.c_str());
        robotNum_ = ::atoi(strRobotNum.c_str());
        robotIQLevel_ = ::atoi(strRobotIQLevel.c_str());
        robotIdStart_ = ::atoi(strRobotIdStart.c_str());
        robotIdEnd_ = ::atoi(strRobotIdEnd.c_str());
        heartBeatTime_ = ::atoi(strHeartBeatTime.c_str());
        verifyTime_ = ::atoi(strVerifyTime.c_str());
        initGameTime_ = ::atoi(strInitGameTime.c_str());
        signUpCondTime_ = ::atoi(strSignUpCondTime.c_str());
        signUpTime_ = ::atoi(strSignUpTime.c_str());
        delaySendActiveMsgTime_ = ::atoi(strDelaySendActiveTime.c_str());
        delaySendPassiveMsgTime_ = ::atoi(strDelaySendPassiveTime.c_str());
        exitTime_ = ::atoi(strExitTime.c_str());
        DEBUG("Get configure successed.");
    }
    return bResult;

}

void NetLib::InitTimer()
{
    //心跳协议的定时器初始化
    timerEventHeartBeat.tv_sec = heartBeatTime_;
    timerEventHeartBeat.tv_usec = 0;

    //验证身份的定时器初始化
    timerEventVerify.tv_sec = verifyTime_;
    timerEventVerify.tv_usec = 0;

    //初始化游戏的定时器初始化
    timerEventInitGame.tv_sec = initGameTime_;
    timerEventInitGame.tv_usec = 0;

    //报名的定时器初始化
    timerEventSignInCond.tv_sec = signUpCondTime_;
    timerEventSignInCond.tv_usec = 0;

    //报名的定时器初始化
    timerEventSignIn.tv_sec = signUpTime_;
    timerEventSignIn.tv_usec = 0;

    //延时发送主动消息定时器初始化
    timerEventDelayActiveMsg.tv_sec = delaySendActiveMsgTime_;
    timerEventDelayActiveMsg.tv_usec = 0;

    //延时发送被动消息定时器初始化
    timerEventDelayPassiveMsg.tv_sec = delaySendPassiveMsgTime_;
    timerEventDelayPassiveMsg.tv_usec = 0;

    //程序退出定时器初始化
    timerEventExit.tv_sec = exitTime_;
    timerEventExit.tv_usec = 0;

    //添加心跳协议定时器
    evtimer_set(&ev_timer_heart_beat, heart_beat_time_cb, this);
    event_add(&ev_timer_heart_beat, &timerEventHeartBeat);
    DEBUG("Heart beat timer started!");

    //添加验证身份定时器
    evtimer_set(&ev_timer_verify, verify_time_cb, this);
    event_add(&ev_timer_verify, &timerEventVerify);
    DEBUG("Verify timer started!");

    //添加初始化游戏定时器
    evtimer_set(&ev_timer_init_game, init_game_time_cb, this);
    event_add(&ev_timer_init_game, &timerEventInitGame);
    DEBUG("Init game timer started!");

    //添加查询报名条件定时器
    evtimer_set(&ev_timer_sign_in_cond, sign_up_cond_time_cb, this);
    event_add(&ev_timer_sign_in_cond, &timerEventSignInCond);
    DEBUG("Sign up cond timer started!");

    //添加报名定时器
    evtimer_set(&ev_timer_sign_in, sign_up_time_cb, this);
    event_add(&ev_timer_sign_in, &timerEventSignIn);

    DEBUG("Sign up timer started!");
}


