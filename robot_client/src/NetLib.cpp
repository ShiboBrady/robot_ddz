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
#include "confaccess.h"

using namespace std;
using namespace YLYQ;
using namespace Protocol;
using namespace message;
using namespace connect;
using namespace org_room2client;
using namespace robot;

NetLib::NetLib()
    :confAccess(CConfAccess::GetConfInstance()),
     ip_(confAccess->GetIP()),
     port_(confAccess->GetPort()),
     robotNum_(confAccess->GetRobotNum()),
     robotIQLevel_(confAccess->GetIQLevel()),
     robotIdStart_(confAccess->GetRobotIdRangeStart()),
     robotIdEnd_(confAccess->GetRobotIdRangeEnd()),
     heartBeatTime_(confAccess->GetHeartBeatTime()),
     verifyTime_(confAccess->GetVerifyTime()),
     initGameTime_(confAccess->GetInitGameTime()),
     delaySendActiveMsgTime_(confAccess->GetSendActiveMsgDelayTime()),
     delaySendPassiveMsgTime_(confAccess->GetSendPassiveMsgDelayTime()),
     exitTime_(confAccess->GetProgramExitTime()),
     roomStateTime_(confAccess->GetQueryRoomStateTime()),
     matchId_(confAccess->GetMatchId()),
     isMatch_(confAccess->GetIsMatch()),
     isTimeTrial_(confAccess->GetIsTimeTrial())
{
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
            insertResult = bevToRobot.insert(make_pair(bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE), \
                    Robot(robotIdStart_ + index, robotIQLevel_)));
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
    OrgRoomDdzCancelSignUpReq orgRoomDdzCancelSignUpReq;
    orgRoomDdzCancelSignUpReq.set_matchid(matchId_);
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

void NetLib::ChangeStatusForRobot(int robotNum)
{
    if (isMatch_)
    {
        INFO("Will set %d robot for match.", robotNum);
    }
    else
    {
        INFO("Will set %d robot for game.", robotNum);
    }

    //从队列中取出机器人，再将取出的机器人重新入队
    int queueSize = int(taskQueue.size());
    while (queueSize && robotNum)
    {
        --queueSize;
        struct bufferevent* abev = taskQueue.front();
        taskQueue.pop();

        //查找消息对应的机器人
        map<struct bufferevent*, Robot>::iterator it = bevToRobot.find(abev);
        if ((bevToRobot.end()) == it)
        {
            //没找到
            continue;
        }

        //找到
        if (WAITSIGNUP == (it->second).GetStatus() && NULL != it->first)
        {
            INFO("Choose robot %d.", (it->second).GetRobot().GetRobotId());
            SendReqForRobot(it->first, it->second);
            --robotNum;
        }
        taskQueue.push(abev); //重新入队
    }

    if (0 == robotNum)
    {
        INFO("Find enough robot.");
    }
    else
    {
        INFO("Doesn't find enough robot, also need %d robot, waitting for next time.", robotNum);
    }

}

void NetLib::SendReqForRobot(struct bufferevent* bev, Robot& robot)
{
    string strSend;
    if (isMatch_)
    {
        string serializedStr;
        OrgRoomDdzSignUpConditionReq orgRoomDdzSignUpConditionReq;
        orgRoomDdzSignUpConditionReq.set_matchid(matchId_);
        orgRoomDdzSignUpConditionReq.SerializeToString(&serializedStr);
        SerializeMsg(robot::MSGID_DDZ_SIGN_UP_CONDITION_REQ, serializedStr, strSend);
    }
    else
    {
        string serializedStr;
        OrgRoomDdzQuickStartReq orgRoomDdzQuickStartReq;
        orgRoomDdzQuickStartReq.set_roomid(matchId_);
        orgRoomDdzQuickStartReq.SerializeToString(&serializedStr);
        SerializeMsg(robot::MSGID_DDZ_QUICK_START_REQ, serializedStr, strSend);
    }
    if (NULL != bev)
    {
        bufferevent_write(bev, strSend.c_str(), strSend.length());
        INFO("Robot %d send a request", robot.GetRobot().GetRobotId());
        robot.SetStatus(INITGAME);
    }
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
    map<struct bufferevent*, Robot>::iterator it;
    do
    {
        it = (netlib->headerRobot).find(bev);
        if ((netlib->headerRobot).end() != it)
        {
            //说明是header robot.
            break;
        }

        it = (netlib->bevToRobot).find(bev);
        if ((netlib->bevToRobot).end() == it)
        {
            ERROR("Cannot find robot.");
            return;
        }
        //走到这里说明是出牌机器人
    }while(0);
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

        //本次剩余未读字节
        dataLength = evbuffer_get_length(bufferevent_get_input(bev));
        DEBUG("Data still has length %d.", dataLength);

        string strMsg;
        strMsg.append(msg, iMsgLen);
        delete [] msg;
        string strRet;
        bool result = (it->second).RobotProcess(msgId, strMsg, strRet);
        if (result)
        {
            //需要发送数据
            netlib->SendMsg(msgId, strRet, bev, it->second);
        }
    }
    //DEBUG("Read data this time over.");
}

void NetLib::SendMsg(int msgId, const string& strRet, struct bufferevent* bev, Robot& robot)
{
    int msgIdBak = msgId;
    switch (msgId)
    {
        case robot::NOTIFY_CALLSCORE:
            msgId = robot::MSGID_CALLSCORE_REQ; //叫分
            break;
        case robot::NOTIFY_DEALCARD:
            msgId = robot::MSGID_CALLSCORE_REQ; //叫分
            break;
        case robot::NOTIFY_TAKEOUT:
            msgId = robot::MSGID_TAKEOUT_REQ; //出牌
            break;
        case robot::NOTIFY_BASECARD:
            msgId = robot::MSGID_TAKEOUT_REQ; //出牌
            break;
        case robot::NOTIFY_TRUST:
            msgId = robot::MSGID_TRUST_CANCEL_REQ; //取消托管
            break;
        case robot::MSGID_KEEP_ACK:
            msgId = robot::MSGID_TRUST_CANCEL_REQ; //取消托管
            break;
        case robot::MSGID_INIT_GAME_ACK:
            msgId = robot::MSGID_KEEP_REQ; //断线续玩
            break;
        case robot::NOTIFY_SWITCH_SCENE:
            msgId = robot::MSGID_READY_REQ; //准备完毕，可以发牌
            break;
        case robot::MSGID_DDZ_SIGN_UP_CONDITION_ACK:
            msgId = robot::MSGID_DDZ_SIGN_UP_REQ; //发送报名请求
            break;
        case robot::MSGID_DDZ_ROOM_STAT_ACK:
            ChangeStatusForRobot(::atoi(strRet.c_str()));
            return;
        case robot::MSGID_DDZ_SIGN_UP_ACK://定时赛中可以发送报名请求
            ChangeStatusForRobot(::atoi(strRet.c_str()));
            return;
    }

    string strSend;
    bool result = SerializeMsg(msgId, strRet, strSend);
    if (!result)
    {
        ERROR("Serialize message to be send failed.");
    }
    else
    {
        int robotId = robot.GetRobot().GetRobotId();
        if (robot::NOTIFY_DEALCARD == msgIdBak || robot::NOTIFY_BASECARD == msgIdBak)
        {
            //添加主动消息延时发送消息的定时器
            pMsgNode oneMsgNode = new msgNode(bev, strSend, msgId, robotId);
            evtimer_set(&(oneMsgNode->ev_timer_delay_), delay_send_msg_time_cb, oneMsgNode);
            event_add(&(oneMsgNode->ev_timer_delay_), &(timerEventDelayActiveMsg));
            DEBUG("Add a timer, will delay send active message: %d to server for robot %d.", msgId, robotId);
        }
        else if (robot::NOTIFY_CALLSCORE == msgIdBak || robot::NOTIFY_TAKEOUT == msgIdBak)
        {
            //添加被动消息延时发送消息的定时器
            pMsgNode oneMsgNode = new msgNode(bev, strSend, msgId, robotId);
            evtimer_set(&(oneMsgNode->ev_timer_delay_), delay_send_msg_time_cb, oneMsgNode);
            event_add(&(oneMsgNode->ev_timer_delay_), &(timerEventDelayPassiveMsg));
            DEBUG("Add a timer, will delay send passive message: %d to server for robot %d.", msgId, robotId);
        }
        else
        {
            //无需延迟，直接发送
            int iSendResult = bufferevent_write(bev, strSend.c_str(), strSend.length());
            DEBUG("Send immediate message: %d to server for robot %d, send result: %d.", msgId, robotId, iSendResult);
        }
    }
}

void NetLib::query_room_state_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);
    //message OrgRoomDdzRoomStatReq {
    //    repeated int32 roomIds = 1;     // 房间/比赛 ID 列表
    //}
    INFO("***************** query_room_state_time_cb START ******************");
    if (0 == int((netlib->headerRobot).size()))
    {
        //选定负责调度的那个机器人
        std::map<struct bufferevent*, Robot>::iterator it;
        for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
        {
            if (WAITSIGNUP == (it->second).GetStatus())
            {
                break;
            }
        }
        if (it == (netlib->bevToRobot).end())
        {
            INFO("Doesn't has valuable robot for header robot, waiting for next time.");
            event_add(&(netlib->ev_timer_room_state), &(netlib->timerEventRoomState));/*重新添加定时器*/
            return;
        }
        INFO("Choose robot %d as header robot.", (it->second).GetRobot().GetRobotId());
        (it->second).SetStatus(HEADER);
        (netlib->headerRobot).insert(make_pair(it->first, it->second));
        INFO("Remove robot %d from the woker robot.", (it->second).GetRobot().GetRobotId());
        (netlib->bevToRobot).erase(it);

        //将除了调度机器人以外的其他机器人加入任务队列中
        while ((netlib->taskQueue).size()) //清空
        {
            (netlib->taskQueue).pop();
        }
        for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
        {
           netlib->taskQueue.push(it->first);
        }
        INFO("Add robot in task queue, size is: %d.", int(netlib->taskQueue.size()));
    }

    std::map<struct bufferevent*, Robot>::iterator it = (netlib->headerRobot).begin();
    if (HEADER != (it->second).GetStatus())
    {
        INFO("Robot %d isn't a header.", (it->second).GetRobot().GetRobotId());
        return;
    }
    INFO("Header robot\'s status is normal.");
    string strSend;
    if (netlib->isTimeTrial_)
    {
        //定时赛
        string serializedStr;
        OrgRoomDdzSignUpConditionReq orgRoomDdzSignUpConditionReq;
        orgRoomDdzSignUpConditionReq.set_matchid(netlib->matchId_);
        orgRoomDdzSignUpConditionReq.SerializeToString(&serializedStr);
        netlib->SerializeMsg(robot::MSGID_DDZ_SIGN_UP_CONDITION_REQ, serializedStr, strSend);
        INFO("Begin to query time trial %d status.", netlib->matchId_);
    }
    else
    {
        //游戏场和非定时赛
        string serializedStr;
        OrgRoomDdzRoomStatReq orgRoomDdzRoomStatReq;
        orgRoomDdzRoomStatReq.add_roomids(netlib->matchId_);
        orgRoomDdzRoomStatReq.SerializeToString(&serializedStr);
        netlib->SerializeMsg(robot::MSGID_DDZ_ROOM_STAT_REQ, serializedStr, strSend);
        INFO("Begin to query room %d status.", netlib->matchId_);
    }
    if (NULL != it->first)
    {
        bufferevent_write(it->first, strSend.c_str(), strSend.length());
        INFO("Header robot %d send query requery once.", (it->second).GetRobot().GetRobotId());
    }
    else
    {
        ERROR("Header robot is not avaliable, please reboot program.");
    }

    event_add(&(netlib->ev_timer_room_state), &(netlib->timerEventRoomState));/*重新添加定时器*/
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
    it = (netlib->headerRobot).begin();
    if (it != (netlib->headerRobot).end() && NULL != it->first)
    {
        bufferevent_write(it->first, strSend.c_str(), strSend.length());
    }
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        if (NULL != it->first)
        {
            bufferevent_write(it->first, strSend.c_str(), strSend.length());
        }
    }
    DEBUG("Send once heard beat.");
    event_add(&(netlib->ev_timer_heart_beat), &(netlib->timerEventHeartBeat));/*重新添加定时器*/
}

void NetLib::verify_time_cb(int fd, short events, void* arg)
{
    NetLib* netlib = static_cast<NetLib*>(arg);
    bool bHasUnviryfiedRobot = false;
    VerifyReq verifyReq ;
    std::map<struct bufferevent*, Robot>::iterator it;
    for (it = (netlib->bevToRobot).begin(); it != (netlib->bevToRobot).end(); ++it)
    {
        if (INIT == (it->second).GetStatus())
        {
            string robotId = StringUtil::Int2String((it->second).GetRobot().GetRobotId());
            string sessionkey = netlib->confAccess->GetSessionKey();
            INFO("robot id is %s, session key is: %s.", robotId.c_str(), sessionkey.c_str());
            verifyReq.Clear();
            verifyReq.set_userid(robotId);
            verifyReq.set_sessionkey(sessionkey + robotId);
            string serializedStr;
            verifyReq.SerializeToString(&serializedStr);
            string strSend;
            netlib->SerializeMsg(robot::MSGID_VERIFY_REQ, serializedStr, strSend);
            if (NULL != it->first)
            {
                bufferevent_write(it->first, strSend.c_str(), strSend.length());
                DEBUG("Robot %d send verify once.", (it->second).GetRobot().GetRobotId());
            }
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

    string strType = netlib->confAccess->GetGameType();
    string strName = netlib->confAccess->GetGameName();
    INFO("type: %s, name: %s.", strType.c_str(), strName.c_str());

    InitGameReq initGameReq;
    initGameReq.set_type(strType);
    initGameReq.set_name(strName);
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
            if (NULL != it->first)
            {
                bufferevent_write(it->first, strSend.c_str(), strSend.length());
                DEBUG("Robot %d send init game once.", (it->second).GetRobot().GetRobotId());
            }
            bHasUnInitedRobot = true;
        }
    }
    if (bHasUnInitedRobot)
    {
        event_add(&(netlib->ev_timer_init_game), &(netlib->timerEventInitGame));/*重新添加定时器*/
    }
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
    map<struct bufferevent*, Robot>::iterator it = (netlib->headerRobot).find(bev);
    if (((netlib->headerRobot).end()) != it)
    {
        bufferevent_free(bev);
        ERROR("Header robot %d disconnected.", (it->second).GetRobot().GetRobotId());
        bev = NULL;
        (netlib->headerRobot).erase(it);
        return;
    }

    it = (netlib->bevToRobot).find(bev);
    if ((netlib->bevToRobot).end() != it)
    {
        bufferevent_free(bev);
        ERROR("Robot %d disconnected.", (it->second).GetRobot().GetRobotId());
        bev = NULL;
        (netlib->bevToRobot).erase(it);
    }
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

    //延时发送主动消息定时器初始化
    timerEventDelayActiveMsg.tv_sec = delaySendActiveMsgTime_;
    timerEventDelayActiveMsg.tv_usec = 0;

    //延时发送被动消息定时器初始化
    timerEventDelayPassiveMsg.tv_sec = delaySendPassiveMsgTime_;
    timerEventDelayPassiveMsg.tv_usec = 0;

    //程序退出定时器初始化
    timerEventExit.tv_sec = exitTime_;
    timerEventExit.tv_usec = 0;

    //查询房间定时器初始化
    timerEventRoomState.tv_sec = roomStateTime_;
    timerEventRoomState.tv_usec = 0;

    //+++++++++++++++++++++++++++//
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

    //添加查询房间状态定时器
    evtimer_set(&ev_timer_room_state, query_room_state_time_cb, this);
    event_add(&ev_timer_room_state, &timerEventRoomState);
    DEBUG("Query room status timer started!");
}


