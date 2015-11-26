#ifndef _ABSTRACTPRODUCT_H_
#define _ABSTRACTPRODUCT_H_
#include <string>
using namespace std;
class OGLordRobotAI;
class AbstractProduct{

public:
    AbstractProduct();
    virtual ~AbstractProduct();

public:
    virtual string operation( OGLordRobotAI& robot, const string& msg ) = 0;
};

//请求认证回应
class GetVerifyAckInfo:public AbstractProduct{

public:
    GetVerifyAckInfo();
    virtual ~GetVerifyAckInfo();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};

//初始化游戏回应
class GetInitGameAckInfo:public AbstractProduct{

public:
    GetInitGameAckInfo();
    virtual ~GetInitGameAckInfo();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};

//查询报名条件回应
class GetSignUpCondAckInfo:public AbstractProduct{

public:
    GetSignUpCondAckInfo();
    virtual ~GetSignUpCondAckInfo();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};

//报名回应
class GetSignUpAckInfo:public AbstractProduct{

public:
    GetSignUpAckInfo();
    virtual ~GetSignUpAckInfo();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};


/*++++++++++++++++++游戏阶段 开始++++++++++++++++++++++*/

//游戏开始
class GetGameStartInfo:public AbstractProduct{

public:
    GetGameStartInfo();
    virtual ~GetGameStartInfo();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};

//发牌
class InitHardCard:public AbstractProduct{

public:
    InitHardCard();
    virtual ~InitHardCard();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};

//收到叫分通知
class GetCallScoreInfo:public AbstractProduct{

public:
    GetCallScoreInfo();
    ~GetCallScoreInfo();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};

//收到地主信息
class GetLordInfo:public AbstractProduct{

public:
    GetLordInfo();
    ~GetLordInfo();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};

//收到底牌
class GetBaseCardInfo:public AbstractProduct{

public:
    GetBaseCardInfo();
    ~GetBaseCardInfo();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};

//收到出牌通知
class GetTakeOutCardInfo:public AbstractProduct{

public:
    GetTakeOutCardInfo();
    ~GetTakeOutCardInfo();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};

//游戏结束
class GetGameOverInfo:public AbstractProduct{

public:
    GetGameOverInfo();
    ~GetGameOverInfo();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};

//获取叫分结果
class GetCallScoreResultInfo:public AbstractProduct{

public:
    GetCallScoreResultInfo();
    ~GetCallScoreResultInfo();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};

//获取出牌结果
class GetTakeOutCardResultInfo:public AbstractProduct{

public:
    GetTakeOutCardResultInfo();
    ~GetTakeOutCardResultInfo();

public:
    string operation( OGLordRobotAI& robot, const string& msg );
};

/*++++++++++++++++++游戏阶段 结束++++++++++++++++++++++*/
#endif

