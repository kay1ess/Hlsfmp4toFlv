/*
 * @Author: kay 
 * @Date: 2021-10-20 17:14:06 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-20 17:25:42
 */

#ifndef UTILS_H
#define UTILS_H

#include <string>

bool StartsWith(const std::string& str, const std::string& start);

bool StartsWith(const std::string& str, const char* start);

bool EndsWith(const std::string& str, const std::string& end);

#endif