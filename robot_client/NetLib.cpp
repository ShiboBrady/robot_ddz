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
    cout << ip_ << " " << port_ << endl;

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
            bufferevent_enable(insertResult.first->first, EV_READ | EV_WRITE);
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
    message.head().set_version(1);
    message.head().set_sequence(1);
    message.head().set_timestamp(1);
    string serializedStr;
    message.SerializeToString(&serializedStr);
    char 4ByteMsgLen[4], 4ByteMsgId[4];
    sprintf(4ByteMsgLen, "%4d", serializedStr.length());
    sprintf(4ByteMsgId, "%4d", msgId);
    return str(4ByteMsgLen) + 4ByteMsgId + serializedStr;
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

    char msgLen[5] = {0};
    size_t len = 0;
    int iMsgLen = 0;
    int msgId = 0;
    int dataLength = evbuffer_get_length(bufferevent_get_input(bev));
    cout << "Reveive data length is: " << dataLength << endl;
    while (dataLength > 0)
    {
        //读取消息长度
        memset(msgLen, '\0', 5);
        len = bufferevent_read(bev, msgLen, 4);
        if (4 != len)
        {
            cout << "Doesn't has 4 byte len info." << endl;
            break;
        }

        msgLen[len] = '\0';
        iMsgLen = ::atoi(msgLen);
        if (0 == iMsgLen)
        {
            cout << "Error! Convent data length failed." << endl;
            break;
        }
        cout << "Msg len: " << msgLen << endl;

        //读取msgid
        memset(msgLen, '\0', 5);
        len = bufferevent_read(bev, msgLen, 4);
        if (4 != len)
        {
            cout << "Doesn't has 4 byte MsgId info." << endl;
            break;
        }

        msgLen[len] = '\0';
        msgId = ::atoi(msgLen);
        if (0 == msgId)
        {
            cout << "Error! Convent msgId failed." << endl;
            break;
        }
        cout << "MsgId is: " << msgId << endl;

        //读取消息体
        char* msg = new char[iMsgLen + 1];
        len = bufferevent_read(bev, msg, iMsgLen);
        msg[len] = '\0';
        cout << "Receive " << msg << " from server" << endl;

        string strRet = (it->second).RobotProcess(msgId, msg);

        if (0 != strRet.length())
        {
            if (MSGID_CALLSCORE_REQ == msgId || MSGID_TAKEOUT_REQ == msgId)
            {
                strRet = SerializeMsg(msgId, strRet);
                //把消息发送给服务器端
                bufferevent_write(bev, strRet.c_str(), strRet.length());
                cout << "Send " << strRet << " to server." << endl;
            }
        }
        delete [] msg;
        dataLength = evbuffer_get_length(bufferevent_get_input(bev));
        cout << "Data still has length: " << dataLength << endl;
    }
    cout << "Read data this time over." << endl;
}

void NetLib::heart_beat_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);
    HeartbeatNtf heartbeatNtf;
    heartbeatNtf.set_rev("robot");
    string serializedStr;
    heartbeatNtf.SerializeToString(&serializedStr);
    serializedStr = SerializeMsg(msgId, serializedStr);
    std::map<struct bufferevent*, OGLordRobotAI>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        bufferevent_write(it->first, serializedStr.c_str(), serializedStr.length());
    }
    cout << "Send once heard beat." << endl;
    event_add(&(netlib->ev_timer_heart_beat), &(netlib->timerEventHeartBeat));/*重新添加定时器*/
}

void NetLib::init_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);
    char msg[] = "init robot\n";
    std::map<struct bufferevent*, OGLordRobotAI>::iterator it;

    Verify verify;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        if (INIT == (it->second).GetStatus())
        {
            verify.Clear();
            verify.set_userid((it->second).GetRobotId());
            verify.set_sessionKey(str("session~") + str((it->second).GetRobotId()));
            string serializedStr;
            verify.SerializeToString(&serializedStr);
            serializedStr = SerializeMsg(MSGID_VERIFY_REQ, serializedStr);
            bufferevent_write(it->first, serializedStr.c_str(), serializedStr.length());
        }
    }
    cout << "send init robot successed." << endl;
    event_add(&(netlib->ev_timer_init), &(netlib->timerEventInit));/*重新添加定时器*/
}

void NetLib::verify_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);
    char msg[] = "verify\n";
    std::map<struct bufferevent*, OGLordRobotAI>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        bufferevent_write(it->first, msg, strlen(msg));
    }
    cout << "send verify successed, pthread Id: " << (unsigned)pthread_self() << endl;
    event_add(&(netlib->ev_timer_verify), &(netlib->timerEventVerify));/*重新添加定时器*/
}

void NetLib::init_game_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);
    char msg[] = "game init\n";
    std::map<struct bufferevent*, OGLordRobotAI>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        bufferevent_write(it->first, msg, strlen(msg));
    }
    cout << "send init game successed, pthread Id: " << (unsigned)pthread_self() << endl;
    event_add(&(netlib->ev_timer_init_game), &(netlib->timerEventInitGame));/*重新添加定时器*/
}

void NetLib::sign_up_cond_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);

    event_add(&(netlib->ev_timer_sign_in_cond), &(netlib->timerEventSignInCond));/*重新添加定时器*/
}

void NetLib::sign_up_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);
    //vector<struct bufferevent*>* allBev = aBevNode->bev;
    //std::map<OGLordRobotAI*, int>* allUnSignUpRobot = aBevNode->unSignUpList;

    ////遍历未报名机器人队表
    //for (map<OGLordRobotAI*, int>::iterator it = allUnSignUpRobot->begin(); it != allUnSignUpRobot->end(); ++it)
    //{
    //    RobotCenter* robotCenter = it->first;
    //    int index = it->second;
    //    struct bufferevent* bev = (*allBev)[index];
    //}

    //查询进场条件

    char msg[] = "sign up\n";
    std::map<struct bufferevent*, OGLordRobotAI>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        bufferevent_write(it->first, msg, strlen(msg));
    }
    cout << "send sign up req successed, pthread Id: " << (unsigned)pthread_self() << endl;
    event_add(&(netlib->ev_timer_sign_in), &(netlib->timerEventSignIn));/*重新添加定时器*/
}

void NetLib::event_cb(struct bufferevent *bev, short event, void *arg)
{

    if (event & BEV_EVENT_EOF)
        printf("connection closed\n");
    else if (event & BEV_EVENT_ERROR)
        printf("some other error\n");
    else if( event & BEV_EVENT_CONNECTED)
    {
        printf("the client has connected to server\n");
        return ;
    }

    //这将自动close套接字和free读写缓冲区
    bufferevent_free(bev);

    struct event *ev = (struct event*)arg;
    event_free(ev);
}

void NetLib::cmd_msg_cb(int fd, short events, void* arg)
{
    char msg[1024];

    int ret = read(fd, msg, sizeof(msg));
    if( ret < 0 )
    {
        perror("read fail ");
        exit(1);
    }

    struct bufferevent* bev = (struct bufferevent*)arg;

    //把终端的消息发送给服务器端
    bufferevent_write(bev, msg, ret);
}

bool NetLib::Init()
{
    std::string strIp;
    std::string strPort;
    std::string strRobotNum;
    std::string strRobotIQLevel;
    std::string strRobotIdStart;
    std::string strRobotIdEnd;
    std::string strHeartBeatTime;
    std::string strInitTime;
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
    bResult = confAccess->GetValue("server", "ip", strIp, "127.0.0.1");
    bResult = confAccess->GetValue("server", "port", strPort, "9999");
    bResult = confAccess->GetValue("timer", "heartBeat", strHeartBeatTime, "30");
    bResult = confAccess->GetValue("timer", "init", strInitTime, "30");
    bResult = confAccess->GetValue("timer", "verify", strVerifyTime, "30");
    bResult = confAccess->GetValue("timer", "initGame", strInitGameTime, "30");
    bResult = confAccess->GetValue("timer", "signUpCond", strSignUpCondTime, "30");
    bResult = confAccess->GetValue("timer", "signUp", strSignUpTime, "30");
    if (bResult)
    {
        ip_ = strIp;
        port_ = ::atoi(strPort.c_str());
        robotNum_ = ::atoi(strRobotNum.c_str());
        robotIQLevel_ = ::atoi(strRobotIQLevel.c_str());
        robotIdStart_ = ::atoi(strRobotIdStart.c_str());
        robotIdEnd_ = ::atoi(strRobotIdEnd.c_str());
        heartBeatTime_ = ::atoi(strHeartBeatTime.c_str());
        initTime_ = ::atoi(strInitTime.c_str());
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

    //机器人初始化的定时器初始化
    timerEventInit.tv_sec = initTime_;
    timerEventInit.tv_usec = 0;

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

    //添加机器人初始化定时器
    evtimer_set(&ev_timer_init, init_time_cb, this);
    event_add(&ev_timer_init, &timerEventInit);
    cout << "Robot init timer started!" << endl;

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
    cout << "Sign up timer started!" << endl;

    //添加报名定时器
    evtimer_set(&ev_timer_sign_in, sign_up_time_cb, this);
    event_add(&ev_timer_sign_in, &timerEventSignIn);
    cout << "Sign up timer started!" << endl;
}


