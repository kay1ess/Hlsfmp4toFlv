/*
 * @Author: kay1ess
 * @Date: 2021-09-28 21:07:36 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-19 21:53:51
 */


#include <iostream>
#include <cpr/cpr.h>
#include "logging.hpp"
#include "playlist.h"
#include "mp4parse.h"


int main() {
    std::string url = "http://10.23.172.197:8088/fmp4/live_36772780_32381134/vod.m3u8";
    Playlist m(url);
    m.Fetch();
    m.Parse();

    io_buffer *buf = new io_buffer(1024);
    Mp4Parse parse;

    for (auto & it : m.m4s_list()) {
        log_info("name:%s duration:%.2f is_header:%d url:%s", it->name.c_str(), it->duration, it->is_header, it->url.c_str());
        cpr::Response r = cpr::Get(cpr::Url{it->url});
        if (r.status_code == 200) {
            printf("%d\n", r.downloaded_bytes);
            buf->write((const uint8_t*)r.text.data(), r.text.size());
            int ret = parse.Parse(buf, it->is_header);
            assert(ret == 0);
        } else {
            log_error("url: %s download failed status=%d", it->url.c_str(), r.status_code);
            assert(0);
        }
        
    }
    
}