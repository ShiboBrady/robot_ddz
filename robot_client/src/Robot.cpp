#include "Robot.h"
#include "message.pb.h"
#include "log.h"
#include <iostream>
using namespace std;
bool Robot::RobotProcess(int msgId, const string& msg, string& result)
{
    //处理消息
    product_ = factory_.createProduct(msgId);
    if (NULL == product_)
    {
        cout << "Doesn't need to process this kind of message. msgId: " << msgId << endl;
        return false;
    }
    YLYQ::Protocol::message::Message message;
    if (!message.ParseFromString(msg))
    {
        cout << "Parse message pb error." << endl;
        return false;
    }
    if (!message.has_body())
    {
        cout << "Doesn't has body info." << endl;
        return false;
    }
    string bodyMsg = message.body();
    bool ret =  product_->operation(*this, bodyMsg, result);
    delete product_;
    return ret;
}

