#include <fstream>
#include <algorithm>
#include "stringutil.h"
#include "confaccess.h"

#define MAX_LINE    1024

CConfAccess* CConfAccess::_pConfInstance = NULL;

CConfAccess* CConfAccess::GetConfInstance()
{
    if (NULL != CConfAccess::_pConfInstance)
        return _pConfInstance;

    _pConfInstance = new CConfAccess();
}

bool CConfAccess::Load(const std::string& fileName)
{
    std::ifstream initFile(fileName.c_str());

    if ( !initFile )
    {
        return false;
    }

    char line[MAX_LINE] = {0};
    std::string fullLine, command;

    while (initFile.getline(line, MAX_LINE))
    {
        fullLine = line;

        /* if the line contains a # then it is a comment
            if we find it anywhere other than the beginning, then we assume
            there is a command on that line, and it we don't find it at all
            we assume there is a command on the line (we test for valid
            command later) if neither is true, we continue with the next line
        */
        std::string::size_type length = fullLine.find('#');
        if (length > 0)
        {
            command = fullLine.substr(0, length);
        }
        else
        {
            continue;
        }

        std::string::size_type leftPos = 0, rightPos = 0;
        // check the command and handle it
        if ( std::string::npos != (length = command.find('=')) )
        {
            if ( _sessionVct.empty() )
            {
                //no session head
                return false;
            }
            CSession::KeyValue keyValue;
            keyValue.first = StringUtil::Trim(command.substr(0, length));
            keyValue.second = StringUtil::Trim(command.substr(length + 1, command.size() - length));
            _sessionVct.back()._attrVct.push_back( keyValue );
        }
        else if ( std::string::npos != (leftPos = command.find('['))
            && std::string::npos != (rightPos = command.find(']'))
            && leftPos < rightPos )
        {
             CSession ss ;
             ss._session = StringUtil::Trim( command.substr(leftPos + 1, rightPos - leftPos - 1) );
             if ( _sessionVct.end() != std::find(_sessionVct.begin(), _sessionVct.end(), ss) )
             {
                 //session exist
                 continue;
             }
             else
             {
                 //push back session object
                 _sessionVct.push_back( ss );
             }
        }
        else
        {
            continue;
        }
    }
    return true;
}

bool CConfAccess::GetValue( const char* session, const char* key, std::string& value, const char* defaultValue )
{
    if ( !session || !key )
    {
        return false;
    }
    bool result = false;
    value = defaultValue;

    CSession ss;
    ss._session = session;
    SessionVector::const_iterator iterSession = std::find( _sessionVct.begin(), _sessionVct.end(), ss );

    if ( _sessionVct.end() != iterSession )
    {
        const KeyValueVector &attrVct = iterSession->_attrVct;

        for ( KeyValueVector::const_iterator iter = attrVct.begin(); iter != attrVct.end(); ++iter )
        {
            if ( iter->first == key )
            {
                result = true;
                value = iter->second;
                break;
            }
        }

    }
    return result;
}
