#ifndef _MAKETESTDATA_H_
#define _MAKETESTDATA_H_
#include "PBGameDDZ.pb.h"
#include <iostream>
#include <string>
#include <map>

void PromptForGameStartNtf(PBGameDDZ::UserInfo* u1, const std::string& userName);
void MakeTestData(std::map<int, std::string>& mapMsg);
void makeJsonData(std::map<int, std::string>& mapMsg);
void makeSendData(std::map<int, std::string>& mapMsg);

#endif /*_MAKETESTDATA_H_*/