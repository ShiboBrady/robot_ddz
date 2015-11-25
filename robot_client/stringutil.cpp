#include "stringutil.h"
#include <sstream>

namespace StringUtil
{
    std::string Trim(const std::string& s) 
    {
        static const char* whiteSpace = " \t\r\n";

        // test for null string
        if(s.empty())
            return s;

        // find first non-space character
        std::string::size_type b = s.find_first_not_of(whiteSpace);
        if(b == std::string::npos) // No non-spaces
            return "";

        // find last non-space character
        std::string::size_type e = s.find_last_not_of(whiteSpace);

        // return the remaining characters
        return std::string(s, b, e - b + 1);
    }


    std::string Int2String( int input )
    {
        std::ostringstream outputStream;
        outputStream << input;
        return outputStream.str();
    }


    int String2Int( const std::string& srcStr )
    {
        std::istringstream inputStream(srcStr);
        int dst;
        inputStream >> dst;
        return dst;
    }

    std::vector<std::string> SplitString(const std::string& strInput, const std::string& strSeparator)
    {
        std::vector<std::string> vecResult;

        size_t last = 0;
        size_t index = strInput.find_first_of(strSeparator, last);
        while (std::string::npos != index)
        {
            std::string strElement = strInput.substr(last, index - last);
            if (!strElement.empty())
            {
                vecResult.push_back(strElement);
            }
            last = index + 1;
            index = strInput.find_first_of(strSeparator, last);
        }
        std::string strElement = strInput.substr(last, index - last);
        if (!strElement.empty())
        {
            vecResult.push_back(strElement);
        }

        return vecResult;
    }

    void ParseRequestString(const std::string& strInput, std::map<std::string, std::string>& mapResult)
    {
        std::vector<std::string> vec = SplitString(strInput, "&");
        size_t maxVec = vec.size();
        for (size_t iIndex = 0; iIndex < maxVec; iIndex++)
        {
            std::vector<std::string> vecTmp = SplitString(vec[iIndex], "=");
            if (2 == vecTmp.size())
            {
                mapResult.insert(std::make_pair<std::string, std::string>(vecTmp[0], vecTmp[1]));
            }
            else if (1 == vecTmp.size())
            {
                mapResult.insert(std::make_pair<std::string, std::string>(vecTmp[0], ""));
            }
            else
            {
                continue;
            }
        }
    }

    bool Replace( std::string& srcStr, const std::string& subStr, const std::string& replaceStr )
    {
        std::string::size_type startPos = srcStr.find( subStr );
        if ( std::string::npos == startPos )
        {
            return false;
        }
        srcStr.replace( startPos, subStr.size(), replaceStr );
        return true;
    }
}