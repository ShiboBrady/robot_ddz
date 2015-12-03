#ifndef _ABSTRACTPRODUCT_H_
#define _ABSTRACTPRODUCT_H_
#include <string>

using namespace std;
class Robot;
class AbstractProduct;

class SimpleFactory{
public:
    SimpleFactory(){};
    ~SimpleFactory(){};
    AbstractProduct* createProduct(int type);
};

class AbstractProduct{

public:
    AbstractProduct(){};
    virtual ~AbstractProduct(){};

public:
    virtual bool operation( Robot& myRobot, const string& msg, string& retMsg ) = 0;
};

//请求认证回应
class GetVerifyAckInfo:public AbstractProduct{

public:
    GetVerifyAckInfo(){};
    virtual ~GetVerifyAckInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//初始化游戏回应
class GetInitGameAckInfo:public AbstractProduct{

public:
    GetInitGameAckInfo(){};
    virtual ~GetInitGameAckInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//查询报名条件回应
class GetSignUpCondAckInfo:public AbstractProduct{

public:
    GetSignUpCondAckInfo(){};
    virtual ~GetSignUpCondAckInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//报名回应
class GetSignUpAckInfo:public AbstractProduct{

public:
    GetSignUpAckInfo(){};
    virtual ~GetSignUpAckInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//查询房间状态回应
class GetRoomStateAckInfo:public AbstractProduct{

public:
    GetRoomStateAckInfo(){};
    virtual ~GetRoomStateAckInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//快速开始游戏请求回应
class GetQuickGameAckInfo:public AbstractProduct{

public:
    GetQuickGameAckInfo(){};
    virtual ~GetQuickGameAckInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};



/*++++++++++++++++++游戏阶段 开始++++++++++++++++++++++*/
//进入游戏场景
class GetEnterGameSceneInfo:public AbstractProduct{

public:
    GetEnterGameSceneInfo(){};
    virtual ~GetEnterGameSceneInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//游戏开始
class GetGameStartInfo:public AbstractProduct{

public:
    GetGameStartInfo(){};
    virtual ~GetGameStartInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//发牌
class InitHardCard:public AbstractProduct{

public:
    InitHardCard(){};
    virtual ~InitHardCard(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//收到叫分通知
class GetCallScoreInfo:public AbstractProduct{

public:
    GetCallScoreInfo(){};
    ~GetCallScoreInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//收到地主信息
class GetLordInfo:public AbstractProduct{

public:
    GetLordInfo(){};
    ~GetLordInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//收到底牌
class GetBaseCardInfo:public AbstractProduct{

public:
    GetBaseCardInfo(){};
    ~GetBaseCardInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//收到出牌通知
class GetTakeOutCardInfo:public AbstractProduct{

public:
    GetTakeOutCardInfo(){};
    ~GetTakeOutCardInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//收到托管通知
class GetTrustInfo:public AbstractProduct{

public:
    GetTrustInfo(){};
    ~GetTrustInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};


//游戏结束
class GetGameOverInfo:public AbstractProduct{

public:
    GetGameOverInfo(){};
    ~GetGameOverInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//游戏结果通知
class GetGameResultInfo:public AbstractProduct{

public:
    GetGameResultInfo(){};
    ~GetGameResultInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//比赛结束通知
class GetCompetitionOverInfo:public AbstractProduct{

public:
    GetCompetitionOverInfo(){};
    ~GetCompetitionOverInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//获取准备完毕结果
class GetReadyResultInfo:public AbstractProduct{

public:
    GetReadyResultInfo(){};
    ~GetReadyResultInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};


//获取叫分结果
class GetCallScoreResultInfo:public AbstractProduct{

public:
    GetCallScoreResultInfo(){};
    ~GetCallScoreResultInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//获取出牌结果
class GetTakeOutCardResultInfo:public AbstractProduct{

public:
    GetTakeOutCardResultInfo(){};
    ~GetTakeOutCardResultInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//获取取消托管结果
class GetTrustResultInfo:public AbstractProduct{

public:
    GetTrustResultInfo(){};
    ~GetTrustResultInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//获取取消报名结果
class GetCancleSignUpResultInfo:public AbstractProduct{

public:
    GetCancleSignUpResultInfo(){};
    ~GetCancleSignUpResultInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};

//获取断线续玩应答
class GetKeepPlayInfo:public AbstractProduct{

public:
    GetKeepPlayInfo(){};
    ~GetKeepPlayInfo(){};

public:
    bool operation( Robot& myRobot, const string& msg, string& retMsg );
};


/*++++++++++++++++++游戏阶段 结束++++++++++++++++++++++*/
#endif

