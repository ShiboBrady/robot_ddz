#include "makeTestData.h"
#include "GameProtocol.h"
#include <time.h>

using namespace std;
using namespace PBGameDDZ;

void PromptForGameStartNtf(UserInfo* u1, const string& userName)
{
    u1->set_username(userName);
    u1->set_nickname("123");
    u1->set_avatatype(1);
    u1->set_avataid("1");
    u1->set_sex(1);
    u1->set_coins(100);
    u1->set_vippoints(50);
}

void InitHandCard(HandCardList* handCardList[3], int baseCard[3])
{
    int total[54];
    int player[3][17];
    int leftNum = 54;
    int ranNumber;
    for(int i = 0; i < 54; i++)
    {
        total[i] = i;
    }
    for(int i = 0; i < 3; i++)
    {
        for(int j  = 0; j < 17; j++)
        {
            srand((unsigned)time(NULL));
            ranNumber = rand() % leftNum;
            handCardList[i]->add_cards(total[ranNumber]);
            total[ranNumber] = total[leftNum - 1];
            leftNum--;
        }
    }
    for (int i = 0; i < 3; ++i)
    {
        baseCard[i] = total[i];
    }
}

void MakeTestData(map<int, string>& mapMsg)
{
    int baseCard[3];
    //游戏开始
    GameStartNtf gameStartNtf;
    PromptForGameStartNtf(gameStartNtf.add_userinfo(), "110001");
    PromptForGameStartNtf(gameStartNtf.add_userinfo(), "110002");
    gameStartNtf.set_gamename("ddz");
    gameStartNtf.set_basicscore(100);
    string msgGameStart;
    gameStartNtf.SerializeToString(&msgGameStart);
    mapMsg[NOTIFY_STARTGAME] = msgGameStart;
    //cout << "msgGameStart" << msgGameStart << endl;    
    //cout << "=====================================" << endl;

    //发牌
    DealCardNtf dealCardNtf;
    dealCardNtf.set_headerseat(0);
    HandCardList* handCardList[3];
    handCardList[0] = dealCardNtf.add_cards();
    handCardList[1] = dealCardNtf.add_cards();
    handCardList[2] = dealCardNtf.add_cards();
    InitHandCard(handCardList, baseCard);
    string msgDealCard;
    dealCardNtf.SerializeToString(&msgDealCard);
    mapMsg[NOTIFY_DEALCARD] = msgDealCard;
    //cout << "msgDealCard" << msgDealCard << endl;    
    //cout << "=====================================" << endl;

    //叫分
    UserCallScoreNtf userCallScoreNtf;
    userCallScoreNtf.set_seatno(2);
    userCallScoreNtf.set_seatnext(0);
    userCallScoreNtf.set_score(0);
    string msgUserCallScoreNtf;
    userCallScoreNtf.SerializeToString(&msgUserCallScoreNtf);
    mapMsg[NOTIFY_CALLSCORE] = msgUserCallScoreNtf;
    //cout << "msgUserCallScoreNtf" << msgUserCallScoreNtf << endl;    
    //cout << "=====================================" << endl;

    //地主信息
    LordSetNtf lordSetNtf;
    lordSetNtf.set_seatlord(0);
    lordSetNtf.set_callscore(1);
    string msgLordSetNtf;
    lordSetNtf.SerializeToString(&msgLordSetNtf);
    mapMsg[NOTIFY_SETLORD] = msgLordSetNtf;
    //cout << "msgLordSetNtf" << msgLordSetNtf << endl;
    //cout << "=====================================" << endl;    
    
    //底牌
    SendBaseCardNtf sendBaseCardNtf;
    for (int i = 0; i < 3; ++i)
    {
        sendBaseCardNtf.add_basecards(baseCard[i]);
    }
    string msgSendBaseCardNtf;
    sendBaseCardNtf.SerializeToString(&msgLordSetNtf);
    mapMsg[NOTIFY_BASECARD] = msgLordSetNtf;
    //cout << "msgSendBaseCardNtf" << msgSendBaseCardNtf << endl;
    //cout << "=====================================" << endl; 

    //出牌
    TakeoutCardNtf takeoutCardNtf;
    takeoutCardNtf.set_seatno(2);
    takeoutCardNtf.set_seatnext(0);
    //takeoutCardNtf.add_cards(5);
    takeoutCardNtf.set_cardtype(1);
    takeoutCardNtf.set_multiple(1);
    string msgTakeoutCardNtf;
    takeoutCardNtf.SerializeToString(&msgTakeoutCardNtf);
    mapMsg[NOTIFY_TAKEOUT] = msgTakeoutCardNtf;
    //cout << "msgTakeoutCardNtf" << msgTakeoutCardNtf << endl;
    //cout << "=====================================" << endl;    

    //游戏结束
    GameOverNtf gameOverNtf;
    gameOverNtf.set_reason(2);
    string msgGameOverNtf;
    gameOverNtf.SerializeToString(&msgGameOverNtf);
    mapMsg[NOTIFY_GAMEOVER] = msgGameOverNtf;
    //cout << "msgGameOverNtf" << msgGameOverNtf << endl;
    //cout << "=====================================" << endl;    
}

void makeJsonData(map<int, string>& mapMsg)
{
    MakeTestData(mapMsg);
    JsonFormat jsonFormat;
    JsonFormat::payLoadReq jsReq;
    jsReq.msgID = 1;
    jsReq.seq = 1;
    jsReq.channel = "ddz_server_01";
    jsReq.source = "org_ddz_game_cj_01";
    jsReq.ack = false;
    jsReq.body.username = "zhangsan";
    jsReq.body.msgID = 0;
    jsReq.body.headMsg.version = 1;
    jsReq.body.headMsg.timeStamp = 1;
    jsReq.body.headMsg.sequence = 1;
    for (map<int, string>::iterator it = mapMsg.begin(); it != mapMsg.end(); ++it)
    {
        it->second = jsonFormat.searialJsonPush(jsReq.channel, jsReq.source, jsReq.body.username, it->first, jsReq.seq, it->second);
    }
}

void makeSendData(map<int, string>& mapMsg)
{
    makeJsonData(mapMsg);
    for (map<int, string>::iterator it = mapMsg.begin(); it != mapMsg.end(); ++it)
    {   
        char t[4];
        int len = strlen(it->second.c_str());
        sprintf(t, "%4d", len);
        it->second = t + it->second;
    }
}