#include "SimpleFactory.h"

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
        case MSGID_VERIFY_ACK:
            temp = new GetVerifyAckInfo();
            break;
        case MSGID_INIT_GAME_ACK:
            temp = new GetInitGameAckInfo();
            break;
        case MSGID_DDZ_SIGN_UP_CONDITION_ACK:
            temp = new GetSignUpCondAckInfo();
            break;
        case MSGID_DDZ_SIGN_UP_ACK:
            temp = new GetSignUpAckInfo();
            break;
        default:
            break;
    }
    return temp;
}

