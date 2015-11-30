#ifndef _CONF_ACCESS_H_
#define _CONF_ACCESS_H_

#include <vector>
#include <string>

class CConfAccess
{
public:
    static CConfAccess* GetConfInstance();

    bool Load(const std::string& fileName);

    bool GetValue( const char* session, const char* key, std::string& value, const char* defaultValue = "" );

private:
    class CSession
    {
    public:
        CSession(){}
        ~CSession(){}

        bool operator==(const CSession& rhs) const
        {
            return _session == rhs._session;
        }

        typedef std::pair<std::string, std::string> KeyValue;

        std::string             _session;
        std::vector<KeyValue>   _attrVct;
    };


    typedef std::vector<CSession> SessionVector;
    typedef std::vector<CSession::KeyValue> KeyValueVector;

    CConfAccess(){};
    CConfAccess(const CConfAccess &);
    void operator=(const CConfAccess &);

    static CConfAccess* _pConfInstance;

    SessionVector _sessionVct;
};

#endif /*_CONF_ACCESS_H_*/