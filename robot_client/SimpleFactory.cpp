#include "SimpleFactory.h"


AbstractFactory::AbstractFactory(){
}


AbstractFactory::~AbstractFactory(){
}


SimpleFactory::SimpleFactory(){
}


SimpleFactory::~SimpleFactory(){
}


AbstractProduct* SimpleFactory::createProduct(int type){
    AbstractProduct* temp = NULL;
    switch(type)
    {
        case NOTIFY_STARTGAME:
            temp = new GetGameStartInfo();
            break;
        case NOTIFY_DEALCARD:
            temp = new InitHardCard();
            break;
        case NOTIFY_CALLSCORE:
            temp = new GetCallScoreInfo();
            break;
        case NOTIFY_SETLORD:
            temp = new GetLordInfo();
            break;
        case NOTIFY_BASECARD:
            temp = new GetBaseCardInfo();
            break;
        case NOTIFY_TAKEOUT:
            temp = new GetTakeOutCardInfo();
            break;
        case NOTIFY_GAMEOVER:
            temp = new GetGameOverInfo();
            break;
        case MSGID_CALLSCORE_ACK:
            temp = new GetCallScoreResultInfo();
            break;
        case MSGID_TAKEOUT_ACK:
            temp = new GetTakeOutCardResultInfo();
            break;
        default:
            break;
    }
    return temp;
}

