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

using namespace std;
using namespace YLYQ;
using namespace Protocol;
using namespace message;
using namespace connect;
using namespace org_room2client;

NetLib::NetLib()
{
    if (!Init())
    {
        cout << "Get params error, please check configure file!" << endl;
        ::exit(0);
    }
    memset(&server_addr, 0, sizeof(server_addr) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_);
    inet_aton(ip_.c_str(), &server_addr.sin_addr);
    base = event_init();
    cout << "ip: " << ip_.c_str() << endl;
    cout << "port: " << port_ << endl;

    connect();

    InitTimer();
}

NetLib::~NetLib()
{
}

void NetLib::connect()
{
    int index = 0;
    for (index = 0; index != robotNum_; ++index)
    {
        if (robotIdStart_ + index <= robotIdEnd_)
        {
            pair< std::map<struct bufferevent*, OGLordRobotAI>::iterator, bool> insertResult;
            insertResult = bevToRobot.insert(make_pair(bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE), OGLordRobotAI(robotIdStart_ + index, robotIQLevel_)));
            if (!insertResult.second)
            {
                //插入失败
                cout << "ERROR: insert failed." << endl;
                ::exit(0);
            }
            bufferevent_socket_connect(insertResult.first->first, (struct sockaddr *)&server_addr, sizeof(server_addr));
            bufferevent_setcb(insertResult.first->first, server_msg_cb, NULL, event_cb, this);
            bufferevent_enable(insertResult.first->first, EV_READ | EV_PERSIST);
            cout << "Create a connection, index is: " << index << ", init a Robot, Id is: " << robotIdStart_ + index << endl;
        }
        else
        {
            cout << "Robot Id range is bigger than robot num." << endl;
            break;
        }
    }
    cout << "Total init " << index << " robots and connections." << endl;
}

void NetLib::start()
{
    event_base_dispatch(base);
}

string NetLib::SerializeMsg( int msgId, const string& body )
{
    Message message;
    message.set_body( body.c_str() );
    message.mutable_head()->set_version(1);     //暂且设为1
    message.mutable_head()->set_sequence(1);    //暂且设为1
    message.mutable_head()->set_timestamp(1);   //暂且设为1
    string serializedStr;
    message.SerializeToString(&serializedStr);

    int msgLen = int(serializedStr.length());

    //cout << "Before trans to net byte order: msg length: " << msgLen << endl;
    //cout << "msgId: " << msgId << endl;
    msgLen = htonl(msgLen);
    msgId = htonl(msgId);

    string strRet;
    strRet.append((char*)&msgLen, sizeof(msgLen));
    strRet.append((char*)&msgId, sizeof(msgId));
    strRet.append(serializedStr.c_str(), (int)serializedStr.length());
    return strRet;
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
    map<struct bufferevent*, OGLordRobotAI>::iterator it = (netlib->bevToRobot).find(bev);
    if ((netlib->bevToRobot).end() == it)
    {
        cout << "Error! Cannot find robot." << endl;
        return;
    }

    char msgLen[4] = {0};
    size_t len = 0;
    int iMsgLen = 0;
    int msgId = 0;
    int dataLength = evbuffer_get_length(bufferevent_get_input(bev));
    cout << "Reveive data length is: " << dataLength << endl;
    while (dataLength > 0)
    {
        //读取msg length
        memset(msgLen, '\0', 4);
        len = bufferevent_read(bev, msgLen, 4);
        if (4 != len)
        {
            cout << "Doesn't has 4 byte len info." << endl;
            break;
        }
        int *pMsgLen = (int*)msgLen;
        iMsgLen = ntohl(*pMsgLen);
        if (0 == iMsgLen)
        {
            cout << "Error! Convent data length failed." << endl;
            break;
        }
        cout << "Msg len: " << iMsgLen << endl;

        //读取msgid
        memset(msgLen, '\0', 4);
        len = bufferevent_read(bev, msgLen, 4);
        if (4 != len)
        {
            cout << "Doesn't has 4 byte MsgId info." << endl;
            break;
        }
        pMsgLen = (int*)msgLen;
        msgId = ntohl(*pMsgLen);
        cout << "MsgId is: " << msgId << endl;

        //读取消息体
        char* msg = new char[iMsgLen + 1];
        len = bufferevent_read(bev, msg, iMsgLen);
        cout << "Receive " << len << " byte from server for robot :" << (it->second).GetRobotId()
            << " in message " << msgId << endl;

        string strMsg;
        strMsg.append(msg, iMsgLen);
        delete [] msg;
        string strRet = (it->second).RobotProcess(msgId, strMsg);

        if (0 != strRet.length())
        {
            if (NOTIFY_CALLSCORE == msgId || NOTIFY_TAKEOUT == msgId ||\
                NOTIFY_DEALCARD == msgId || NOTIFY_BASECARD == msgId)
            {
                if (NOTIFY_CALLSCORE == msgId || NOTIFY_DEALCARD == msgId)
                {
                    msgId = MSGID_CALLSCORE_REQ;
                }
                else if (NOTIFY_TAKEOUT == msgId || NOTIFY_BASECARD == msgId)
                {
                    msgId = MSGID_TAKEOUT_REQ;
                }
                string strSend = netlib->SerializeMsg(msgId, strRet);
                //把消息发送给服务器端
                bufferevent_write(bev, strSend.c_str(), strSend.length());
                cout << "Send message: " << msgId << " to server for robot :"
                    << (it->second).GetRobotId() << endl;
            }
        }
        dataLength = evbuffer_get_length(bufferevent_get_input(bev));
        cout << "Data still has length: " << dataLength << endl;
    }
    cout << "Read data this time over." << endl;
}

void NetLib::heart_beat_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);

    HeartbeatNtf heartbeatNtf;
    heartbeatNtf.set_rev("robot");  //暂且设为robot
    string serializedStr;
    heartbeatNtf.SerializeToString(&serializedStr);
    serializedStr = netlib->SerializeMsg(connect::MSGID_HEARTBEAT_NTF, serializedStr);

    std::map<struct bufferevent*, OGLordRobotAI>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        bufferevent_write(it->first, serializedStr.c_str(), serializedStr.length());
    }
    cout << "Send once heard beat." << endl;
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
    std::map<struct bufferevent*, OGLordRobotAI>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        if (INIT == (it->second).GetStatus())
        {
            string robotId = StringUtil::Int2String((it->second).GetRobotId());
            verifyReq.Clear();
            verifyReq.set_userid(robotId);
            verifyReq.set_sessionkey(StringUtil::Trim(strSessionKey) + robotId);
            string serializedStr;
            verifyReq.SerializeToString(&serializedStr);
            serializedStr = netlib->SerializeMsg(connect::MSGID_VERIFY_REQ, serializedStr);
            bufferevent_write(it->first, serializedStr.c_str(), serializedStr.length());
            cout << "Robot " << (it->second).GetRobotId() << " send verify successed." << endl;
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
    serializedStr = netlib->SerializeMsg(connect::MSGID_INIT_GAME_REQ, serializedStr);

    bool bHasUnInitedRobot = false;
    std::map<struct bufferevent*, OGLordRobotAI>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        if (VERIFIED == (it->second).GetStatus())
        {
            bufferevent_write(it->first, serializedStr.c_str(), serializedStr.length());
            cout << "Robot " << (it->second).GetRobotId() << " send init game successed." << endl;
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
    serializedStr = netlib->SerializeMsg(org_room2client::MSGID_DDZ_SIGN_UP_CONDITION_REQ, serializedStr);

    std::map<struct bufferevent*, OGLordRobotAI>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        if (INITGAME == (it->second).GetStatus())
        {
            bufferevent_write(it->first, serializedStr.c_str(), serializedStr.length());
            cout << "Robot " << (it->second).GetRobotId() << " send sign up cond req successed" << endl;
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

    std::map<struct bufferevent*, OGLordRobotAI>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        if (CANSINGUP == (it->second).GetStatus())
        {
            orgRoomDdzSignUpReq.set_costid((it->second).GetCost());
            string serializedStr;
            orgRoomDdzSignUpReq.SerializeToString(&serializedStr);
            serializedStr = netlib->SerializeMsg(org_room2client::MSGID_DDZ_SIGN_UP_REQ, serializedStr);

            bufferevent_write(it->first, serializedStr.c_str(), serializedStr.length());
            cout << "Robot " << (it->second).GetRobotId() << " send sign up req successed" << endl;
        }
    }
    event_add(&(netlib->ev_timer_sign_in), &(netlib->timerEventSignIn));/*重新添加定时器*/
}

void NetLib::event_cb(struct bufferevent *bev, short event, void *arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);
    if (event & BEV_EVENT_EOF)
    {
        cout << "connection closed." << endl;
    }
    else if (event & BEV_EVENT_ERROR)
    {
        printf("some other error\n");
    }
    else if( event & BEV_EVENT_CONNECTED)
    {
        printf("the client has connected to server\n");
        return ;
    }

    //查找消息对应的机器人
    map<struct bufferevent*, OGLordRobotAI>::iterator it = (netlib->bevToRobot).find(bev);
    if ((netlib->bevToRobot).end() != it)
    {
        //这将自动close套接字和free读写缓冲区
        bufferevent_free(bev);
        cout << "Robot " << (it->second).GetRobotId() << " disconnected." << endl;
        (netlib->bevToRobot).erase(it);
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

    bool bResult = false;
    CConfAccess* confAccess = CConfAccess::GetConfInstance();
    bResult = confAccess->GetValue("robot", "robotIdStart", strRobotIdStart, "110001");
    bResult = confAccess->GetValue("robot", "robotIdEnd", strRobotIdEnd, "110001");
    bResult = confAccess->GetValue("robot", "robotNum", strRobotNum, "1");
    bResult = confAccess->GetValue("robot", "IQLevel", strRobotIQLevel, "0");
    bResult = confAccess->GetValue("server", "ip", ip_, "127.0.0.1");
    bResult = confAccess->GetValue("timer", "heartBeat", strHeartBeatTime, "30");
    bResult = confAccess->GetValue("timer", "verify", strVerifyTime, "30");
    bResult = confAccess->GetValue("timer", "initGame", strInitGameTime, "30");
    bResult = confAccess->GetValue("timer", "signUpCond", strSignUpCondTime, "30");
    bResult = confAccess->GetValue("timer", "signUp", strSignUpTime, "30");
    bResult = confAccess->GetValue("server", "port", strPort, "9999");
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
        cout << "Get configure successed." << endl;
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

    //添加心跳协议定时器
    evtimer_set(&ev_timer_heart_beat, heart_beat_time_cb, this);
    event_add(&ev_timer_heart_beat, &timerEventHeartBeat);
    cout << "Heart beat timer started!" << endl;

    //添加验证身份定时器
    evtimer_set(&ev_timer_verify, verify_time_cb, this);
    event_add(&ev_timer_verify, &timerEventVerify);
    cout << "Verify timer started!" << endl;

    //添加初始化游戏定时器
    evtimer_set(&ev_timer_init_game, init_game_time_cb, this);
    event_add(&ev_timer_init_game, &timerEventInitGame);
    cout << "Init game timer started!" << endl;

    //添加查询报名条件定时器
    evtimer_set(&ev_timer_sign_in_cond, sign_up_cond_time_cb, this);
    event_add(&ev_timer_sign_in_cond, &timerEventSignInCond);
    cout << "Sign up cond timer started!" << endl;

    //添加报名定时器
    evtimer_set(&ev_timer_sign_in, sign_up_time_cb, this);
    event_add(&ev_timer_sign_in, &timerEventSignIn);
    cout << "Sign up timer started!" << endl;
}


