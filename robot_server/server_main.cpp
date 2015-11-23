#include "makeTestData.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <event.h>
#include <event2/bufferevent.h>

void accept_cb(int fd, short events, void* arg);
void socket_read_cb(bufferevent* bev, void* arg);
void event_cb(struct bufferevent *bev, short event, void *arg);
int tcp_server_init(int port, int listen_num);
using namespace std;

static int iIndex = 0;
map<int, string> mapMsg;
int msgQueue[] = {PBGameDDZ::NOTIFY_STARTGAME, PBGameDDZ::NOTIFY_DEALCARD, PBGameDDZ::NOTIFY_SETLORD, PBGameDDZ::NOTIFY_TAKEOUT, PBGameDDZ::NOTIFY_GAMEOVER};
int main(int argc, char** argv)
{
    makeSendData(mapMsg);
    // for (map<int, string>::iterator it = mapMsg.begin(); it != mapMsg.end(); ++it)
    // {   
    //     printf("%d : %s\n", it->first, it->second.c_str());
    // }
    int listener = tcp_server_init(9999, 10);
    if( listener == -1 )
    {
        perror(" tcp_server_init error ");
        return -1;
    }

    struct event_base* base = event_base_new();

    //添加监听客户端请求连接事件
    struct event* ev_listen = event_new(base, listener, EV_READ | EV_PERSIST,
                                        accept_cb, base);
    event_add(ev_listen, NULL);


    event_base_dispatch(base);
    event_base_free(base);


    return 0;
}

void accept_cb(int fd, short events, void* arg)
{
    evutil_socket_t sockfd;
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    sockfd = ::accept(fd, (struct sockaddr*)&client, &len );
    evutil_make_socket_nonblocking(sockfd);
    printf("accept a client %d\n", sockfd);
    iIndex = 0;
    struct event_base* base = (event_base*)arg;
    bufferevent* bev = bufferevent_socket_new(base, sockfd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(bev, socket_read_cb, NULL, event_cb, arg);
    bufferevent_enable(bev, EV_READ | EV_PERSIST);
}

void socket_read_cb(bufferevent* bev, void* arg)
{
    char msg[4096];
    size_t len = bufferevent_read(bev, msg, sizeof(msg));
    msg[len] = '\0';
    printf("recv the client msg: %s", msg);    
    string strInfo = mapMsg[msgQueue[iIndex]];
    bufferevent_write(bev, strInfo.c_str(), strInfo.length());
    iIndex = (iIndex + 1) % 5;
}

void event_cb(struct bufferevent *bev, short event, void *arg)
{

    if (event & BEV_EVENT_EOF)
        printf("connection closed\n");
    else if (event & BEV_EVENT_ERROR)
        printf("some other error\n");

    //这将自动close套接字和free读写缓冲区
    bufferevent_free(bev);
}


typedef struct sockaddr SA;
int tcp_server_init(int port, int listen_num)
{
    int errno_save;
    evutil_socket_t listener;

    listener = ::socket(AF_INET, SOCK_STREAM, 0);
    if( listener == -1 )
        return -1;

    //允许多次绑定同一个地址。要用在socket和bind之间
    evutil_make_listen_socket_reuseable(listener);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(port);

    if( ::bind(listener, (SA*)&sin, sizeof(sin)) < 0 )
        goto error;

    if( ::listen(listener, listen_num) < 0)
        goto error;


    //跨平台统一接口，将套接字设置为非阻塞状态
    evutil_make_socket_nonblocking(listener);

    return listener;

    error:
        errno_save = errno;
        evutil_closesocket(listener);
        errno = errno_save;

        return -1;
}
