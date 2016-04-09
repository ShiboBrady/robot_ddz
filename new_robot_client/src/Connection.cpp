#include "Connection.h"
#include <string>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "MsgPackage.h"
#include "log.h"
#include "RobotBase.h"

using namespace std;

/*消息的格式
    +----------------+----------+----------------------------+
    +  len(Message)  |   msgId  |           Message          |
    +----------------+----------+-----------+----------------+
    +                           |   Head    |     body       |
    +----------------+----------+-----------+----------------+
    +    4 bytes     |  4 bytes |  12 bytes | len - 12 bytes |
    +----------------+---------------------------------------+
*/
int Conn::GetReadBufferData(char* buffer, int msgLen) {
    if (nullptr == bev_) { 
        return -1;
    }
    return bufferevent_read(bev_, buffer, msgLen);
}

int Conn::AddToWriteBuffer(const string& buffer) {
    if (nullptr == bev_) {
        return -1;
    }
    return bufferevent_write(bev_, buffer.c_str(), buffer.length());
}
