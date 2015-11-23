//#include <windows.h>
#include <vector>
#include <string.h>
using namespace std;

//#include "OGService.h"
#include "OGLordRobotAIInterface.h"
#include "OGLordRbtAINull.h"


COGLordRbtAINull::COGLordRbtAINull()
{
    m_nScore= 0;
    for (int i= 0 ; i< CARDS_SUM ; i++)
    {
        m_szHandCards[i] = 0;
    }
    m_nSeat = 0;
    m_nBiggerSeat = 0;
}

COGLordRbtAINull::~COGLordRbtAINull()
{

}

// finish
bool COGLordRbtAINull::RbtInInitCard( int argSeat, std::vector<int> argHandCard )
{
    for(int i= 0 ; i< CARDS_SUM ; i++){
        m_szHandCards[i] = 0;
    }
    m_nSeat = argSeat;
    for (int i= 0 ; i< argHandCard.size() ; i++){
        if (argHandCard[i]< (CARDS_SUM) && argHandCard[i]>= 0){
            m_szHandCards[argHandCard[i]] = 1;
        }
    }
    return true;
}

// finish
bool COGLordRbtAINull::RbtInCallScore( int argSeat, int argCallScore )
{
    if (argCallScore != 0)
    {
        m_nScore = argCallScore;
    }
    return true;
}

// finish
bool COGLordRbtAINull::RbtOutGetCallScore( int &argCallScore ,int &delay)
{
    if (m_nScore){
        argCallScore = 0;
    }else{
        argCallScore = 1;
    }
    return true;
}

// finish
bool COGLordRbtAINull::RbtInSetLord( int argLordSeat, std::vector<int> argReceiceCard )
{
    //gsWriteLog(L"RbtNULL SetLord lordSeat:%d , mySeat:%d , receiceCardSize:%d", argLordSeat, m_nSeat, argReceiceCard.size());
    if (argLordSeat == m_nSeat){
        for (int i= 0 ; i< argReceiceCard.size() ; i++){
            if (argReceiceCard[i]< (CARDS_SUM) && argReceiceCard[i]>= 0){
                m_szHandCards[argReceiceCard[i]] = 1;
            }
        }
    }
    m_nBiggerSeat = argLordSeat;
    return true;
}

// finish
bool COGLordRbtAINull::RbtInTakeOutCard( int argSeat, std::vector<int> argCards )
{
    if (argCards.size() != 0 )
    {
        m_nBiggerSeat = argSeat;
    }
    for (int i= 0; i< argCards.size(); i++ ){
        if (argCards[i] < CARDS_SUM && argCards[i]>= 0){
            m_szHandCards[argCards[i]] = 0;
        }
    }
    return true;
}

// finish
bool COGLordRbtAINull::RbtOutGetTakeOutCard( std::vector<int> &argCards ,int &delay )
{
    argCards.clear();
    if (m_nBiggerSeat != m_nSeat)
    {
        return true;
    }
    for (int j= 0 ; j< 13 ;j++)
    {
        for (int i= 0; i< 4; i++)
        {
            int nIndex = i*13 + j;
            if (m_szHandCards[nIndex] > 0)
            {
                argCards.push_back(nIndex);
                return true;
            }
        }
    }
    if (m_szHandCards[52] > 0)
    {
        argCards.push_back(52);
        return true;
    }
    if (m_szHandCards[53] > 0)
    {
        argCards.push_back(53);
        return true;
    }

    return true;
}

// finish
bool COGLordRbtAINull::RbtResetData()
{
    m_nScore= 0;
    for(int i= 0 ; i< CARDS_SUM ;i++){
        m_szHandCards[i] = 0;
    }
    m_nSeat = 0;
    m_nBiggerSeat = 0;
    return true;
}

// finish
bool COGLordRbtAINull::RbtInSetSeat( int argMySeat, int argLordSeat )
{
    m_nSeat = argMySeat;
    return true;
}

// unrealized 在断线续完s
bool COGLordRbtAINull::RbtInSetCard( std::vector<int> argInitCard, std::vector<int> argReceiveCard )
{
    return true;
}

// unrealized 在断线续玩时候使用
bool COGLordRbtAINull::RbtInTakeOutRecord( std::vector<int> argTakeOutSeat ,std::vector<std::vector<int> > argTakeOutRecord )
{
    return true;
}

bool COGLordRbtAINull::RbtInSetLevel(int argLevel) {return true;}

bool COGLordRbtAINull::RbtInNtfCardInfo(std::vector<std::vector<int> > argHandCard){return true;}

bool COGLordRbtAINull::RbtInSetGrabLord(int argSeat){return true;}

bool COGLordRbtAINull::RbtInCallScore( int argSeat,                        // 座位号
                                 int argCallScore,                   // 叫的分数
                                 int & delay                         // 延时时间
                                ){return true;}
bool COGLordRbtAINull::RbtOutGetCallScore( int &callScore                 // 返回值引用
                                    ){return true;};

bool COGLordRbtAINull::RbtOutGetGrabLord(bool &grabLord ,int &delay) {return true;}

bool COGLordRbtAINull::RbtOutGetLastError(int &errorCode){return true;}

        // 输入玩家明牌信息
    bool COGLordRbtAINull::RbtInShowCard( int argShowSeat,                    // 明牌玩家座位
                                std::vector<int> argHandCard        // 玩家手牌
                                ){return true;}

    // 输出明牌策略
    bool COGLordRbtAINull::RbtOutShowCard(bool &showCard, int &delay){return true;}     // 是否明牌 true:明牌 false:不明牌
