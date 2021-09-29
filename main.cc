/*
 * @Author: kay1ess
 * @Date: 2021-09-28 21:07:36 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-11 16:18:12
 */


#include <iostream>
#include <cpr/cpr.h>
#include "logging.hpp"
#include "playlist.h"


int main()
{
    std::string url = "http://10.23.172.197:8088/fmp4/live_36772780_32381134/vod.m3u8";
    Playlist m(url);
    m.fetch();
    m.parse();

    for (auto & it : m.m4s_list())
    {
        log_info("name:%s duration:%.2f is_header:%d url:%s", it->name.c_str(), it->duration, it->is_header, it->url.c_str());
    }
    
}