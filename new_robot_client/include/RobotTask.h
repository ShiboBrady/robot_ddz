#ifndef __ROBOTTASK_H__
#define __ROBOTTASK_H__

#include <memory>
#include "RobotBase.h"
#include "OGLordRobotAI.h"

class RobotTask : public RobotBase {
public:
    RobotTask(int iRobotId, int iRobotIQ);
    ~RobotTask() {};
    
    virtual bool AnalysisAndDecision(std::shared_ptr<MsgNode>& msgNode);
    virtual bool NextActionProcess( std::shared_ptr<MsgNode>& msgNode );
    virtual void RobotInit();
    virtual void DeleteTimer();

    bool SendQuerySignUpCondReq(std::shared_ptr<MsgNode>& msgNode);           //查询报名条件发送
    bool RecvQuerySignUpCondAck(std::shared_ptr<MsgNode>& msgNode);           //查询报名条件回复

    bool SendSignUpReq(std::shared_ptr<MsgNode>& msgNode);                    //报名信息发送
    bool RecvSignUpAck(std::shared_ptr<MsgNode>& msgNode);                    //报名信息回复

    bool SendQuickBeginGameReq(std::shared_ptr<MsgNode>& msgNode);            //快速开始游戏消息发送
    bool RecvQuickBeginGameAck(std::shared_ptr<MsgNode>& msgNode);            //快速开始游戏消息回复

    bool SendKeepPlayReq(std::shared_ptr<MsgNode>& msgNode);                  //断线续玩消息发送
    bool RecvKeepPlayAck(std::shared_ptr<MsgNode>& msgNode);                  //断线续玩消息回复

    bool RecvEnterGameSceneNotify(std::shared_ptr<MsgNode>& msgNode);         //收到进入游戏场景通知
    bool RecvGameStartNotify(std::shared_ptr<MsgNode>& msgNode);              //收到游戏开始通知
    bool RecvInitHardCardNotify(std::shared_ptr<MsgNode>& msgNode);           //收到初始化手牌通知
    bool RecvUserCallScoreNotify(std::shared_ptr<MsgNode>& msgNode);          //收到玩家叫分通知
    bool RecvGetLordNotify(std::shared_ptr<MsgNode>& msgNode);                //收到地主位置通知
    bool RecvGetBaseCardNotify(std::shared_ptr<MsgNode>& msgNode);            //收到底牌通知
    bool RecvGetTakeOutCardNotify(std::shared_ptr<MsgNode>& msgNode);         //收到出牌通知
    bool RecvGetTrustNotify(std::shared_ptr<MsgNode>& msgNode);               //收到托管通知
    bool RecvGetGameOverNotify(std::shared_ptr<MsgNode>& msgNode);            //收到游戏结束通知
    bool RecvGetGameResultNotify(std::shared_ptr<MsgNode>& msgNode);          //收到游戏结果通知
    bool RecvGetCompetitionOverNotify(std::shared_ptr<MsgNode>& msgNode);     //收到比赛结束通知

    bool SendReadyForGameReq(std::shared_ptr<MsgNode>& msgNode);              //准备好游戏消息发送
    bool RecvReadyForGameAck(std::shared_ptr<MsgNode>& msgNode);              //准备好游戏消息回复

    bool SendCallScoreReq(std::shared_ptr<MsgNode>& msgNode);                 //出牌消息发送
    bool RecvCallScoreAck(std::shared_ptr<MsgNode>& msgNode);                 //出牌消息回复

    bool SendTakeOutCardReq(std::shared_ptr<MsgNode>& msgNode);               //出牌消息发送
    bool RecvTakeOutCardAck(std::shared_ptr<MsgNode>& msgNode);               //出牌消息回复

    bool SendTrustCancelReq(std::shared_ptr<MsgNode>& msgNode);               //取消托管消息发送
    bool RecvTrustCancelAck(std::shared_ptr<MsgNode>& msgNode);               //取消托管消息回复

    bool RecvCancelSignUpAck(std::shared_ptr<MsgNode>& msgNode);              //取消报名消息回复

private:
    OGLordRobotAI robot_;
    int costId_; //报名需要的费用id
    int myScore_; //此局自己的叫分
    int delaySendActiveMsgTime_;
    int delaySendPassiveMsgTime_;
};

#endif /*__ROBOTTASK_H__*/