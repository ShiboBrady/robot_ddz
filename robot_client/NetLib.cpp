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
using namespace std;

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
    base = event_base_new();

    //心跳协议的定时器初始化
    timerEventHeartBeat.tv_sec = heartBeatTime_;
    timerEventHeartBeat.tv_usec = 0;

    //报名的定时器初始化
    timerEventSignIn.tv_sec = signUpTime_;
    timerEventSignIn.tv_usec = 0;

    connect();
}

NetLib::~NetLib()
{
}

void NetLib::connect()
{
    int index = 0;
    for (index = 0; index != robotNum_; ++index)
    {
        bev.push_back(bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE));
        bufferevent_socket_connect(bev[index], (struct sockaddr *)&server_addr, sizeof(server_addr));

        pMsgNode aMsgNode = new msgNode(this, index);
        bufferevent_setcb(bev[index], server_msg_cb, NULL, event_cb, static_cast<void*>(aMsgNode));
        bufferevent_enable(bev[index], EV_READ | EV_PERSIST);
        cout << "Create a connection, index is: " << index << endl;

        if (robotIdStart_ + index <= robotIdEnd_)
        {
            robotCenter.insert(make_pair(index, RobotCenter(robotIdStart_ + index, robotIQLevel_)));
            cout << "Init a robot, id is: " << robotIdStart_ + index << endl;
        }
        else
        {
            cout << "Robot Id range is bigger than robot num." << endl;
            break;
        }
    }
    cout << "Total init " << index << " robots and connections." << endl;

    //添加心跳协议定时器
    //pBevNode aBevNode = new BevNode(this, bev);
    //evtimer_set(&ev_timer_heart_beat, heart_beat_time_cb, (void*)(aBevNode));
    //event_add(&ev_timer_heart_beat, &timerEventHeartBeat);
    //cout << "Heart beat started!" << endl;

    //添加报名定时器
    //evtimer_set(&ev_timer_sign_in, sign_up_time_cb, (void*)(aBevNode));
    //event_add(&ev_timer_sign_in, &timerEventSignIn);
    //cout << "Sign up started!" << endl;

}

void NetLib::start()
{
    event_base_dispatch(base);
}

void NetLib::server_msg_cb(struct bufferevent* bev, void* arg)
{
    //获取参数内容
    pMsgNode aMsgNode = static_cast<pMsgNode>(arg);
    NetLib* netlib = aMsgNode->netlib;
    int index = aMsgNode->index;
    cout << "Msg index: " << index << endl;

    char msgLen[4];
    size_t len = bufferevent_read(bev, msgLen, 4);
    msgLen[len] = '\0';
    int iMsgLen = ::atoi(msgLen);
    cout << "Msg len: " << msgLen << endl;

    char *msg = (char*)calloc(iMsgLen + 1, sizeof(char));

    len = bufferevent_read(bev, msg, iMsgLen);
    msg[len] = '\0';

    cout << "Receive " << msg << " from server" << endl;

    map<int, RobotCenter>::iterator it = (netlib->robotCenter).find(index);
    if ((netlib->robotCenter).end() == it)
    {
        cout << "Cannot find robot." << endl;
    }
    string strRet = (it->second).RobotProcess(msg);
    if ("" != strRet);
    {
        //把消息发送给服务器端
        bufferevent_write(bev, strRet.c_str(), strRet.length());
        cout << "Send " << strRet << " to server." << endl;
    }
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

void NetLib::heart_beat_time_cb(int fd, short events, void* arg)
{
    pBevNode aBevNode = static_cast<pBevNode>(arg);
    NetLib* netlib = aBevNode->netlib;
    vector<struct bufferevent*> allBev = aBevNode->bev;
    char msg[] = "timer wakeup\n";
    for (vector<struct bufferevent*>::iterator it = allBev.begin(); it != allBev.end(); ++it)
    {
        bufferevent_write(*it, msg, strlen(msg));
    }
    cout << "send heard beat successed." << endl;
    event_add(&(netlib->ev_timer_heart_beat), &(netlib->timerEventHeartBeat));/*重新添加定时器*/
}

void NetLib::sign_up_time_cb(int fd, short events, void* arg)
{
    pBevNode aBevNode = static_cast<pBevNode>(arg);
    NetLib* netlib = aBevNode->netlib;
    vector<struct bufferevent*> allBev = aBevNode->bev;

    event_add(&(netlib->ev_timer_sign_in), &(netlib->timerEventSignIn));/*重新添加定时器*/
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
    std::string strSignUpTime;

    bool bResult = false;
    CConfAccess* confAccess = CConfAccess::GetConfInstance();
    bResult = confAccess->GetValue("robot", "robotIdStart", strRobotIdStart, "110001");
    bResult = confAccess->GetValue("robot", "robotIdEnd", strRobotIdEnd, "110001");
    bResult = confAccess->GetValue("robot", "robotNum", strRobotNum, "1");
    bResult = confAccess->GetValue("robot", "IQLevel", strRobotIQLevel, "0");
    bResult = confAccess->GetValue("server", "ip", strIp, "127.0.0.1");
    bResult = confAccess->GetValue("server", "port", strIp, "9999");
    bResult = confAccess->GetValue("timer", "heartBeat", strHeartBeatTime, "30");
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
        signUpTime_ = ::atoi(strSignUpTime.c_str());
    }
    return bResult;

}

