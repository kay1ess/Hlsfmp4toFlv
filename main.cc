/*
 * @Author: kay1ess
 * @Date: 2021-09-28 21:07:36 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-20 17:22:20
 */


#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <signal.h>
#include <cpr/cpr.h>
#include "logging.hpp"
#include "playlist.h"
#include "mp4parse.h"
#include "sink.h"
#include "utils.h"


std::atomic<bool> quit(false);

void SignalHandler(int signal) {
    if (signal == SIGINT) {
        log_warn("ready to quit!");
        quit = true;
    }
}


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

    // Regiseter Signal
    signal(SIGINT, SignalHandler);

    Playlist m(url);
    FlvFileSink sink(flv_path);
    io_buffer *buf = new io_buffer(1024);
    Mp4Parse parse;
    parse.SetSink(&sink);

    // Download m4s
    std::string last_m4s_url;
    int fail_count = 0;

    while (!quit && m.Update() == 0) {
        size_t i = 0;
        auto m4s_list = m.m4s_list();
        auto it = m4s_list.begin();

        if (last_m4s_url.size() != 0) {
            for (; it != m4s_list.end(); it++) {
                // skip has downloaded m4s
                if ((*it)->url == last_m4s_url) {
                    it++;
                    break;
                }
            }
        }

        for (; it != m4s_list.end(); it++) {
            cpr::Response r = cpr::Get(cpr::Url{(*it)->url});
            log_info("download from url=%s", (*it)->url.c_str());
            if (r.status_code == 200) {
                buf->write((const uint8_t*)r.text.data(), r.text.size());
                int ret = parse.Parse(buf, (*it)->is_header);
                assert(ret == 0);
            } else {
                log_error("url: %s download failed status=%ld", (*it)->url.c_str(), r.status_code);
                break;
            }
            fail_count = 0;
        }

        if (m.m4s_list()[m.m4s_list().size()-1]->url == last_m4s_url && ++fail_count == 10) {
            log_error("playlist fail_count=%d not update", fail_count);
            break;
        }
        last_m4s_url =m4s_list[m.m4s_list().size()-1]->url;

        int update_duration = m.min_duration() * 1000 / 2;
        log_info("update playlist after %dms", update_duration);
        std::this_thread::sleep_for(std::chrono::milliseconds(update_duration));
    }
}