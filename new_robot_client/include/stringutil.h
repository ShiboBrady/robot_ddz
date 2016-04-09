#ifndef _STRING_UTIL_H_
#define _STRING_UTIL_H_

#include <string>
#include <map>
#include <vector>

namespace StringUtil
{      
    /**
     *  Returns a string identical to the given string but without leading
     *  or trailing HTABs or spaces.
    **/
    std::string Trim(const std::string& s);

    /**
     *  Convert int type to string type Safety
    **/
    std::string Int2String( int srcInt );


    /**
     *  Convert string type to int type Safety
    **/
    int String2Int( const std::string& srcStr );

    /**
     *  Split string by a symbol
    **/
    std::vector<std::string> SplitString(const std::string& strInput, const std::string& strSeparator);

    /**
     *  Get param from url
     **/
    void ParseRequestString(const std::string& strInput, std::map<std::string, std::string>& mapResult);


    bool Replace( std::string& srcStr, const std::string& subStr, const std::string& replaceStr );

}

#endif // !_STRING_UTIL_H_