#ifndef _JSONFOEMAT_
#define _JSONFOEMAT_

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <string>

using namespace rapidjson;

class JsonFormat {
public:
    JsonFormat() {
    }
    ~JsonFormat(void) {
    }

    enum JSON_PUSH_TYPE {
        MSG_HEARTBEAT = 0,    // 心跳
        MSG_PUSH = 1,         // 推送消息
        MSG_LISTEN = 2,       // 监听消息
        MSG_SUB = 3,          // 发布消息
        MSG_PUB = 4,          // 订阅消息
        MSG_PUSH_ACK = 11,    // 推送返回
        MSG_LISTEN_ACK = 12,  // 监听返回
        MSG_SUB_ACK = 13,     // 订阅返回
        MSG_PUB_ACK = 14     // 发布返回
    };

    bool parseJsonMsg(const std::string& jsonMsg) {
        if (m_doc.Parse<0>(jsonMsg.c_str()).HasParseError()) {
            return false;
        }
        return true;
    }
    typedef struct Head {
        int32_t version;
        int64_t timeStamp;
        int32_t sequence;
    } Head;
    typedef struct content {
        std::string username;
        int32_t msgID;
        Head headMsg;
        std::string pbMsg;
    } content;
    typedef struct payLoadReq {
        int32_t msgID;
        uint32_t seq;
        std::string channel;
        std::string source;
        bool ack;
        content body;
    } payLoadReq;

    typedef struct payLoadAck {
        uint32_t seq;
        int32_t result;
    } payLoadAck;

    static std::string searialJsonMsg(const int msgNo, const std::string& channel, const std::string& sourceChannel,
        const std::string& username, const uint32_t msgID, const int32_t seq, const std::string& pbMsg);
    static std::string searialJsonPush(const std::string& channel, const std::string& sourceChannel,
        const std::string& username, const uint32_t msgID, const int32_t seq, const std::string& pbMsg);
    static std::string searialJsonRegister(const std::string& channel, const std::string& sourceChannel,
        const std::string& username, const uint32_t msgID, const int32_t seq, const std::string& pbMsg);

    static std::string Bin2Hex(const std::string &strBin, bool bIsUpper = false);
    static std::string Hex2Bin(const std::string &strHex);

    bool parseJsonReq(payLoadReq& jsReq, const std::string& jsonMsg);
    bool parseJsonAck(int32_t& msgID, payLoadAck& jsAck, const std::string& jsonMsg);

    bool isJsonArray(Value& jobj);
    bool isJsonObject(Value& jobj);

    bool getJsonValue(Value& jobj, const char* keyname);
    bool getJsonArray(Value& jobj, const char* keyname);
    bool getJsonObject(Value& jobj, const char* keyname);

    bool getJsonChild(Value& jobj, const char* keyname, const Value& jboj);

    bool decode(std::string& out, const char* keyname);
    bool decode(bool& out, const char* keyname);
    bool decode(int32_t& out, const char* keyname);
    bool decode(uint32_t& out, const char* keyname);

    bool decode(Value& out, const char* keyname, const Value& jobj);
    bool decode(std::string& out, const char* keyname, const Value& jobj);
    bool decode(bool& out, const char* keyname, const Value& jobj);
    bool decode(int32_t& out, const char* keyname, const Value& jobj);
    bool decode(uint32_t& out, const char* keyname, const Value& jobj);
    bool decode(int64_t& out, const char* keyname, const Value& jobj);

private:
    rapidjson::Document m_doc;
};

#endif /*_JSONFOEMAT_*/
