#include "RobotCenter.h"
#include "GameProtocol.h"
#include "AbstractProduct.h"
#include "SimpleFactory.h"

using namespace std;

RobotCenter::RobotCenter()
{
    string robotName = "zhangsan";
    mMapRobotList_.insert(make_pair(robotName, OGLordRobotAI(robotName)));
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

    //判断是哪个机器人
    string robotName = jsReq.body.username;
    map<string, OGLordRobotAI>::iterator it = mMapRobotList_.find(robotName);
    if (it == mMapRobotList_.end())
    {
        cout << "There doesn't has robot:" << robotName << endl;
        return result;
    }
    OGLordRobotAI& robot = it->second;

    //判断消息类型
    int msgId = jsReq.body.msgID;
    string pbBody = jsReq.body.pbMsg;

    //处理消息
    AbstractFactory* factory = new SimpleFactory();
    AbstractProduct* product = factory->createProduct(msgId);
    result = product->operation(robot, pbBody);
    if ("" != result)
    {
        //序列化消息
        cout << "Before searial: " << result << endl;
        result = jsonFormat.searialJsonPush(jsReq.channel, jsReq.source, jsReq.body.username, jsReq.body.msgID, jsReq.seq, result);
        cout << "After searial: " << result << endl;
    }
    return result;

}


