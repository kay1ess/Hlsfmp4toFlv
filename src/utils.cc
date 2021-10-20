/*
 * @Author: kay 
 * @Date: 2021-10-20 17:14:52 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-20 17:26:14
 */


#include "utils.h"


bool StartsWith(const std::string& str, const std::string& start) {
    int srclen = str.size();
    int startlen = start.size();
    if(srclen >= startlen)
    {
        std::string temp = str.substr(0, startlen);
        if(temp == start)
            return true;
    }
    return false;
}

bool StartsWith(const std::string& str, const char* start) {
    std::string t(start);
    return StartsWith(str, t);
}

bool EndsWith(const std::string& str, const std::string& end)
{
    int srclen = str.size();
    int endlen = end.size();
    if(srclen >= endlen)
    {
        std::string temp = str.substr(srclen - endlen, endlen);
        if(temp == end)
            return true;
    }

    return false;
}