/*
 * @Author: kay 
 * @Date: 2021-09-29 16:17:08 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-19 20:49:00
 */


#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <string>
#include <vector>
#include <memory>
#include "logging.hpp"
#include "code.hpp"


typedef struct m4s {
    std::string name;
    float duration;
    bool is_header;
    std::string url;
    m4s(std::string name, float duration, bool is_header, std::string url) : name(name), duration(duration), is_header(is_header), url(url) {}
} m4s_t;


class Playlist {
public:
    Playlist(std::string& url): url_(url), is_vod_(false), is_end_(false), version_(-1), target_duration_(0.0) {
        if (url_.find_first_of("http://") == 0 || url_.find_first_of("https://") == 0)
            local_path_ = false;
        else
            local_path_ = true;
    }
    ~Playlist();
    int Fetch();
    int Update();
    int Parse();
    std::string m3u8_content();
    bool is_vod();
    bool is_end();
    float target_duration();
    std::shared_ptr<m4s_t> last_header();
    std::shared_ptr<m4s_t> current_header();
    std::vector<std::shared_ptr<m4s_t>> m4s_list();

private:
    int version_;
    float target_duration_;
    bool is_vod_;
    bool is_end_;
    bool local_path_;
    std::string url_;
    std::string base_url_;
    std::vector<std::shared_ptr<m4s_t>> m4s_list_;
    std::string raw_m3u8_;
    std::string last_m4s_name_;
    std::shared_ptr<m4s_t> cur_header_;
    std::shared_ptr<m4s_t> last_header_;
};


#endif