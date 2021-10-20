/*
 * @Author: kay1ess
 * @Date: 2021-09-28 21:07:36 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-20 17:22:20
 */


#include <iostream>
#include <cpr/cpr.h>
#include "logging.hpp"
#include "playlist.h"
#include "mp4parse.h"
#include "sink.h"
#include "utils.h"


int main(int argc, char** argv) {
    if (argc != 3) {
        log_warn("--usage\n [hls2flv] m3u8_url flv_path");
        return -1;
    }

    std::string url = argv[1];
    if (!EndsWith(url, ".m3u8")) {
        log_error("invalid m3u8 url");
        return -1;
    }

    std::string flv_path = argv[2];
    if (!EndsWith(flv_path, ".flv")) {
        log_error("invalid flv path");
        return -1;
    }

    Playlist m(url);
    if (m.Fetch() != 0) {
        log_error("download playlist failed");
        return -1;
    }
    if (m.Parse() != 0) {
        log_error("parse m3u8 failed");
        return -1;
    }

    FlvFileSink sink(flv_path);
    io_buffer *buf = new io_buffer(1024);
    Mp4Parse parse;
    parse.SetSink(&sink);

    for (auto & it : m.m4s_list()) {
        cpr::Response r = cpr::Get(cpr::Url{it->url});
        log_info("url=%s", it->url.c_str());
        if (r.status_code == 200) {
            buf->write((const uint8_t*)r.text.data(), r.text.size());
            int ret = parse.Parse(buf, it->is_header);
            assert(ret == 0);
        } else {
            log_error("url: %s download failed status=%ld", it->url.c_str(), r.status_code);
            assert(0);
        }
    }
}