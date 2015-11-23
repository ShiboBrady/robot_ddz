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

RobotCenter NetLib::robotCenter = RobotCenter();

NetLib::NetLib(string ip, int port)
    :ip(ip),
     port(port)
{
    base = event_base_new();
    bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    memset(&server_addr, 0, sizeof(server_addr) );
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_aton(ip.c_str(), &server_addr.sin_addr);
    connect();
}

NetLib::~NetLib()
{
}

void NetLib::connect()
{
    bufferevent_socket_connect(bev, (struct sockaddr *)&server_addr, sizeof(server_addr));
    bufferevent_setcb(bev, server_msg_cb, NULL, event_cb, NULL);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);
    struct event* ev_cmd = event_new(base, STDIN_FILENO,
                                     EV_READ | EV_PERSIST,
                                     cmd_msg_cb, (void*)bev);


    event_add(ev_cmd, NULL);
}

void NetLib::start()
{
    event_base_dispatch(base);
}

void NetLib::server_msg_cb(struct bufferevent* bev, void* arg)
{
    char msgLen[4];
    size_t len = bufferevent_read(bev, msgLen, 4);
    msgLen[len] = '\0';
    int iMsgLen = ::atoi(msgLen);
    cout << "Msg len: " << msgLen << endl;

    char *msg = (char*)calloc(iMsgLen + 1, sizeof(char));

    len = bufferevent_read(bev, msg, iMsgLen);
    msg[len] = '\0';

    cout << "Receive " << msg << " from server" << endl;

    string strRet = robotCenter.RobotProcess(msg);
    //if ("" != strRet);
    //{
    //    //把消息发送给服务器端
    //    bufferevent_write(bev, strRet.c_str(), strRet.length());
    //    cout << "Send " << strRet << " to server." << endl;
    //}
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
