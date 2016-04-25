#ifndef __ROBOTHEADER_H__
#define __ROBOTHEADER_H__

#include "RobotBase.h"

class RobotHeader : public RobotBase {
public:
    RobotHeader(int iRobotId);
    ~RobotHeader();

    virtual bool AnalysisAndDecision(std::shared_ptr<MsgNode>& msgNode);
    virtual bool NextActionProcess( std::shared_ptr<MsgNode>& msgNode );
    virtual void RobotInit();
    virtual void DeleteTimer();

    bool HeaderRobotAction(std::shared_ptr<MsgNode>& msgNode); 

    bool BeginToQueryRoomStatus(std::shared_ptr<MsgNode>& msgNode);

    bool SendQueryRoomStatusReq(std::shared_ptr<MsgNode>& msgNode);           //查询房间状态消息发送
    bool RecvQueryRoomStatusAck(std::shared_ptr<MsgNode>& msgNode);           //查询房间状态消息回复

    bool SendQuerySignUpCondReq(std::shared_ptr<MsgNode>& msgNode);           //查询报名条件发送
    bool RecvQuerySignUpCondAck(std::shared_ptr<MsgNode>& msgNode);           //查询报名条件回复
    
private:
    bool IsTimeToQueryRoomStatus(int iStartSignUpTime, int iEndSignUpTime, int iGameBeginTime);
    int roomStateTime_; //查询房间状态的时间间隔
    MsgNode* QueryRoomStateTimer_; //查询房间状态消息包
};

#endif /*__ROBOTHEADER_H__*/