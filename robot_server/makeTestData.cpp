#include "makeTestData.h"
#include "GameProtocol.h"

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

void MakeTestData(map<int, string>& mapMsg)
{
    //游戏开始
    GameStartNtf gameStartNtf;
    PromptForGameStartNtf(gameStartNtf.add_userinfo(), "zhangsan");
    PromptForGameStartNtf(gameStartNtf.add_userinfo(), "lisi");
    gameStartNtf.set_gamename("ddz");
    gameStartNtf.set_basicscore(100);
    string msgGameStart;
    gameStartNtf.SerializeToString(&msgGameStart);
    //cout << "msgGameStart" << msgGameStart << endl;
    mapMsg[NOTIFY_STARTGAME] = msgGameStart;
    //cout << "=====================================" << endl;

    //发牌
    DealCardNtf dealCardNtf;
    dealCardNtf.set_headerseat(0);
    HandCardList *handCardList1 = dealCardNtf.add_cards();
    HandCardList *handCardList2 = dealCardNtf.add_cards();
    HandCardList *handCardList3 = dealCardNtf.add_cards();
    for (int index = 0; index <17; ++index)
    {
        handCardList1->add_cards(index);
    }
    for (int index = 0; index < 17; ++index)
    {
        handCardList2->add_cards(index + 17);
    }
    for (int index = 0; index <17; ++index)
    {
        handCardList3->add_cards(index + 17 + 17);
    }
    string msgDealCard;
    dealCardNtf.SerializeToString(&msgDealCard);
    //cout << "msgDealCard" << msgDealCard << endl;
    mapMsg[NOTIFY_DEALCARD] = msgDealCard;
    //cout << "=====================================" << endl;

    //叫分
    UserCallScoreNtf userCallScoreNtf;

    //地主信息
    LordSetNtf lordSetNtf;
    lordSetNtf.set_seatlord(0);
    lordSetNtf.set_callscore(1);
    string msgLordSetNtf;
    lordSetNtf.SerializeToString(&msgLordSetNtf);
    //cout << "msgLordSetNtf" << msgLordSetNtf << endl;
    //cout << "=====================================" << endl;
    mapMsg[NOTIFY_SETLORD] = msgLordSetNtf;
    
    //底牌
    SendBaseCardNtf sendBaseCardNtf;

    //出牌
    TakeoutCardNtf takeoutCardNtf;
    takeoutCardNtf.set_seatno(2);
    takeoutCardNtf.set_seatnext(0);
    takeoutCardNtf.add_cards(5);
    takeoutCardNtf.set_cardtype(1);
    takeoutCardNtf.set_multiple(1);
    string msgTakeoutCardNtf;
    takeoutCardNtf.SerializeToString(&msgTakeoutCardNtf);
    //cout << "msgTakeoutCardNtf" << msgTakeoutCardNtf << endl;
    //cout << "=====================================" << endl;
    mapMsg[NOTIFY_TAKEOUT] = msgTakeoutCardNtf;

    //游戏结束
    GameOverNtf gameOverNtf;
    gameOverNtf.set_reason(2);
    string msgGameOverNtf;
    gameOverNtf.SerializeToString(&msgGameOverNtf);
    //cout << "msgGameOverNtf" << msgGameOverNtf << endl;
    //cout << "=====================================" << endl;
    mapMsg[NOTIFY_GAMEOVER] = msgGameOverNtf;
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