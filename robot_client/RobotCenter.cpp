#include "RobotCenter.h"
#include "GameProtocol.h"
#include "AbstractProduct.h"
#include "SimpleFactory.h"

using namespace std;

RobotCenter::RobotCenter( const int robotId, const int robotIQLeve )
    :_Robot(robotId, robotIQLeve)
{
}

RobotCenter::~RobotCenter()
{

}

string RobotCenter::RobotProcess(string msg)
{
    string result;

    //解析json数据
    JsonFormat jsonFormat;
    JsonFormat::payLoadReq jsReq;
    bool parseRet = jsonFormat.parseJsonReq(jsReq, msg);
    if (!parseRet)
    {
        cout << "Parse json data error." << endl;
        return result;
    }

    //判断消息类型
    int msgId = jsReq.body.msgID;
    string pbBody = jsReq.body.pbMsg;

    //处理消息
    AbstractFactory* factory = new SimpleFactory();
    AbstractProduct* product = factory->createProduct(msgId);
    if (NULL == product)
    {
        cout << "Doesn't need to process this kind of message. msgId: " << msgId << endl;
        return result;
    }
    result = product->operation(_Robot, pbBody);
    if ("" != result)
    {
        //序列化消息
        cout << "Before searial: " << result << endl;
        result = jsonFormat.searialJsonPush(jsReq.channel, jsReq.source, jsReq.body.username, jsReq.body.msgID, jsReq.seq, result);
        char tmp[4];
        sprintf(tmp, "%4d", int(result.length()));
        result = string(tmp) + result;
        cout << "After searial: " << result << endl;
    }
    return result;

}


