#include "GameProtocol.h"
#include <sstream>
#include <time.h>

//#define PROTO_DEBUG
#ifdef PROTO_DEBUG
#include <iostream>
using namespace std;
#endif

bool JsonFormat::isJsonArray(Value& jobj)
{
    if (!jobj.IsNull() && jobj.IsArray()) {
        return true;
    }
    return false;
}

bool JsonFormat::isJsonObject(Value& jobj)
{
    if (!jobj.IsNull() && jobj.IsObject()) {
        return true;
    }
    return false;
}
bool JsonFormat::getJsonValue(Value& out, const char* keyname)
{
    if (keyname && m_doc.HasMember(keyname)) {
        out = m_doc[keyname];
        return true;
    }
    return false;
}

bool JsonFormat::getJsonChild(Value& out, const char* keyname, const Value& jobj)
{
    Document::AllocatorType& allocator = m_doc.GetAllocator();
    for (rapidjson::Value::ConstMemberIterator it = jobj.MemberBegin(); it != jobj.MemberEnd(); ++it) {
        if (!strcmp(keyname, it->name.GetString()) && (it->value.IsObject() || it->value.IsArray())) {
            out.CopyFrom(it->value, allocator);
            return true;
        }
    }
    return false;
}

bool JsonFormat::decode(int32_t& out, const char* keyname)
{
    if (keyname && m_doc.HasMember(keyname)) {
        if (m_doc[keyname].IsNull()) {
            return true;
        }
        if (m_doc[keyname].IsInt()) {
            out = m_doc[keyname].GetInt();
            return true;
        }
    }
    return false;
}
bool JsonFormat::decode(uint32_t& out, const char* keyname)
{
    if (keyname && m_doc.HasMember(keyname)) {
        if (m_doc[keyname].IsNull()) {
            return true;
        }
        if (m_doc[keyname].IsUint()) {
            out = m_doc[keyname].GetUint();
            return true;
        }
    }
    return false;
}
bool JsonFormat::decode(bool& out, const char* keyname)
{
    if (keyname && m_doc.HasMember(keyname)) {
        if (m_doc[keyname].IsNull()) {
            return true;
        }
        if (m_doc[keyname].IsBool()) {
            out = m_doc[keyname].GetBool();
            return true;
        }
    }
    return false;
}
bool JsonFormat::decode(std::string& out, const char* keyname)
{
    if (keyname && m_doc.HasMember(keyname)) {
        if (m_doc[keyname].IsNull()) {
            out = "";
            return true;
        }
        if (m_doc[keyname].IsString()) {
            out = m_doc[keyname].GetString();
            return true;
        }
    }
    return false;
}
bool JsonFormat::decode(Value& out, const char* keyname, const Value& jobj)
{
    if (keyname && jobj.IsObject() && jobj.HasMember(keyname)) {
        if (jobj[keyname].IsNull()) {
            return true;
        }
        if (jobj[keyname].IsArray() || jobj[keyname].IsObject()) {
            return true;
        }
    }
    return false;
}
bool JsonFormat::decode(std::string& out, const char* keyname, const Value& jobj)
{
    if (keyname && jobj.IsObject() && jobj.HasMember(keyname)) {
        if (jobj[keyname].IsNull()) {
            out = "";
            return true;
        }
        if (jobj[keyname].IsString()) {
            out = jobj[keyname].GetString();
            return true;
        }
        return false;
    }
    return false;
}

bool JsonFormat::decode(bool& out, const char* keyname, const Value& jobj)
{
    if (keyname && jobj.IsObject() && jobj.HasMember(keyname)) {
        if (jobj[keyname].IsNull()) {
            return true;
        }
        if (jobj[keyname].IsBool()) {
            out = jobj[keyname].GetBool();
            return true;
        }
    }
    return false;
}
bool JsonFormat::decode(int32_t& out, const char* keyname, const Value& jobj)
{
    if (keyname && jobj.IsObject() && jobj.HasMember(keyname)) {
        if (jobj[keyname].IsNull()) {
            return true;
        }
        if (jobj[keyname].IsInt()) {
            out = jobj[keyname].GetInt();
            return true;
        }
    }
    return false;
}
bool JsonFormat::decode(uint32_t& out, const char* keyname, const Value& jobj)
{
    if (keyname && jobj.IsObject() && jobj.HasMember(keyname)) {
        if (jobj[keyname].IsNull()) {
            return true;
        }
        if (jobj[keyname].IsUint()) {
            out = jobj[keyname].GetUint();
            return true;
        }
    }
    return false;
}
bool JsonFormat::decode(int64_t& out, const char* keyname, const Value& jobj)
{
    if (keyname && jobj.IsObject() && jobj.HasMember(keyname)) {
        if (jobj[keyname].IsNull()) {
            return true;
        }
        if (jobj[keyname].IsInt64()) {
            out = jobj[keyname].GetInt64();
            return true;
        }
    }
    return false;
}

bool JsonFormat::parseJsonReq(JsonFormat::payLoadReq& jsReq, const std::string& jsonMsg)
{
    JsonFormat js;
    if (!js.parseJsonMsg(jsonMsg)) {
        return false;
    }

    if (!js.decode(jsReq.msgID, "msgId")) {
        return false;
    }

    js.decode(jsReq.ack, "ack");

    Value pyLoad;
    if (!js.getJsonValue(pyLoad, "payload")) {
        return false;
    }
    if (!js.isJsonObject(pyLoad)) {
        return false;
    }

    js.decode(jsReq.seq, "seq", pyLoad);

    if (!js.decode(jsReq.channel, "channel", pyLoad)) {
        return false;
    }

    if (!js.decode(jsReq.source, "source", pyLoad)) {
        return false;
    }

    Value content;
    if (!getJsonChild(content, "content", pyLoad)) {
        return false;
    }

    if (!isJsonObject(content)) {
        return false;
    }

    if (!decode(jsReq.body.username, "userId", content)) {
        return false;
    }

    if (!decode(jsReq.body.msgID, "msgId", content)) {
        return false;
    }

    Value head;
    if (!getJsonChild(head, "head", content)) {
        return false;
    }
    if (!isJsonObject(head)) {
        return false;
    }

    decode(jsReq.body.headMsg.version, "version", head);

    decode(jsReq.body.headMsg.timeStamp, "timestamp", head);

    decode(jsReq.body.headMsg.sequence, "sequence", head);

    std::string pbMsg;
    if (!decode(pbMsg, "body", content)) {
        return false;
    }

    if (pbMsg.size() > 0) {
        jsReq.body.pbMsg = Hex2Bin(pbMsg);
    }
    return true;
}

bool JsonFormat::parseJsonAck(int32_t& msgID, payLoadAck& jsAck, const std::string& jsonMsg)
{
    if (!parseJsonMsg(jsonMsg)) {
        return false;
    }

    if (!decode(msgID, "msgId")) {
        return false;
    }

    Value pyLoad;
    if (!getJsonValue(pyLoad, "payload")) {
        return false;
    }
    if (!isJsonObject(pyLoad)) {
        return false;
    }
    if (!decode(jsAck.seq, "seq", pyLoad)) {
        return false;
    }
    if (!decode(jsAck.result, "result", pyLoad)) {
        return false;
    }
    return true;
}

std::string JsonFormat::searialJsonPush(const std::string& channel, const std::string& sourceChannel, const std::string& username, const uint32_t msgID, const int32_t seq, const std::string& pbMsg)
{
    return searialJsonMsg(MSG_PUSH, channel, sourceChannel, username, msgID, seq, pbMsg);
}
std::string JsonFormat::searialJsonRegister(const std::string& channel, const std::string& sourceChannel, const std::string& username, const uint32_t msgID, const int32_t seq, const std::string& pbMsg)
{
    return searialJsonMsg(MSG_LISTEN, channel, sourceChannel, username, msgID, seq, pbMsg);
}
std::string JsonFormat::searialJsonMsg(const int msgNo, const std::string& channel, const std::string& sourceChannel, const std::string& username, const uint32_t msgID, const int32_t seq, const std::string& pbMsg)
{
    Document doc;
    doc.SetObject();
    Document::AllocatorType& allocator = doc.GetAllocator();

    Value head(kObjectType);
    head.AddMember("version", 1, allocator);
    head.AddMember("timestamp", time(NULL), allocator);
    head.AddMember("sequence", seq, allocator);

    Value Content(kObjectType);
    Content.SetObject();
    Value jsUserID(kStringType);
    if (!username.empty()) {
        jsUserID.SetString(username.c_str(), username.size());
    }
    Content.AddMember("userId", jsUserID, allocator);
    Value jsPBMsg(kStringType);
    std::string hexMsg;
    if (!pbMsg.empty()) {
        hexMsg = Bin2Hex(pbMsg);
        jsPBMsg.SetString(hexMsg.c_str(), hexMsg.size());
    }
    Content.AddMember("body", jsPBMsg, allocator);
    Content.AddMember("msgId", msgID, allocator);
    Content.AddMember("head", head, allocator);

    Value payLoad(kObjectType);
    payLoad.SetObject();
    Value jsChannel(kStringType);
    if (!channel.empty()) {
        jsChannel.SetString(channel.c_str(), channel.size());
    }
    Value jsSourceChannel(kStringType);
    if (!sourceChannel.empty()) {
        jsSourceChannel.SetString(sourceChannel.c_str(), sourceChannel.size());
    }
    payLoad.AddMember("seq", seq, allocator);
    payLoad.AddMember("channel", jsSourceChannel, allocator);
    payLoad.AddMember("source", jsChannel, allocator);
    payLoad.AddMember("content", Content, allocator);

    doc.AddMember("msgId", msgNo, allocator);
    if (msgNo == MSG_LISTEN) {
        doc.AddMember("ack", 0, allocator);
    }
    doc.AddMember("payload", payLoad, allocator);

    StringBuffer buffer;
    Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    return buffer.GetString();
}

std::string JsonFormat::Bin2Hex(const std::string &strBin, bool bIsUpper)
{
    std::string strHex;
    strHex.resize(strBin.size() * 2);
    for (size_t i = 0; i < strBin.size(); i++) {
        uint8_t cTemp = strBin[i];
        for (size_t j = 0; j < 2; j++) {
            uint8_t cCur = (cTemp & 0x0f);
            if (cCur < 10) {
                cCur += '0';
            } else {
                cCur += ((bIsUpper ? 'A' : 'a') - 10);
            }
            strHex[2 * i + 1 - j] = cCur;
            cTemp >>= 4;
        }
    }

    return strHex;
}

std::string JsonFormat::Hex2Bin(const std::string &strHex)
{
    if (strHex.size() % 2 != 0) {
        return "";
    }

    std::string strBin;
    strBin.resize(strHex.size() / 2);
    for (size_t i = 0; i < strBin.size(); i++) {
        uint8_t cTemp = 0;
        for (size_t j = 0; j < 2; j++) {
            char cCur = strHex[2 * i + j];
            if (cCur >= '0' && cCur <= '9') {
                cTemp = (cTemp << 4) + (cCur - '0');
            } else if (cCur >= 'a' && cCur <= 'f') {
                cTemp = (cTemp << 4) + (cCur - 'a' + 10);
            } else if (cCur >= 'A' && cCur <= 'F') {
                cTemp = (cTemp << 4) + (cCur - 'A' + 10);
            } else {
                return "";
            }
        }
        strBin[i] = cTemp;
    }

    return strBin;
}

// bool PBFormat::formatData2PB(PBMessage& pMsg, const std::string& pStr)
// {
//     return pMsg.ParseFromString(pStr);
// }
// bool PBFormat::formatPB2Data(std::string& pStr, const PBMessage& pMsg)
// {
//     return pMsg.SerializeToString(&pStr);
// }