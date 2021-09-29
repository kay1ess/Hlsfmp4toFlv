/*
 * @Author: kay 
 * @Date: 2021-09-29 16:17:16 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-11 16:15:28
 */


#include "playlist.h"
#include <cpr/cpr.h>
#include <iostream>
#include <string.h>


Playlist::~Playlist()
{
    m4s_list_.clear();
}


int Playlist::fetch()
{
    if (!local_path_)
    {
        cpr::Response res = cpr::Get(cpr::Url{url_});
        if (res.status_code != 200)
        {
            log_error("url: %s response code(%ld) != 200", url_.c_str(), res.status_code);
            return -1;
        }
        log_info("fetch %s ok", url_.c_str());
        raw_m3u8_ = res.text;
    }
    else
    {

    }

    return 0;
}


int Playlist::update()
{
    return 0;
}


int Playlist::parse()
{
    if (raw_m3u8_.size() <= 7)
        return -1;
    size_t s = 0, p = 0;
    std::string_view tmp;
    bool next_need_m4s = false;
    float next_m4s_duration = 0;
    bool discount =false;

    while (true)
    {
        p = raw_m3u8_.find_first_of('\n', s);
        if (p == std::string::npos)
        {
            break;
        }
        tmp = raw_m3u8_.substr(s, p - s);

        // First line must start with '#EXTM3U'
        if (s == 0 && strncmp("#EXTM3U", tmp.data(), 7) != 0)
        {
            log_error("not a m3u8");
            return -1;
        }
        s = p + 1;

        if (next_need_m4s)
        {
            if (tmp.find_first_of("http://") == 0 || tmp.find_first_of("https://"))
            {
                auto index = tmp.find_last_of('/');
                m4s_list_.push_back(std::make_shared<m4s_t>(std::string(tmp, index + 1, tmp.size()), next_m4s_duration, false, std::string(tmp, 0, index + 1)));
            }
            else
            {
                m4s_list_.push_back(std::make_shared<m4s_t>(std::string(tmp), next_m4s_duration, false, std::string("")));
            }
            next_need_m4s = false;
            continue;
        }


        if (strncmp("#EXT-X-VERSION:", tmp.data(), 15) == 0 && tmp.size() > 15)
        {
            std::string t(tmp.data(), 15, p - s - 15);
            try
            {
                version_ = std::stoi(t);
            }
            catch (...)
            {
                log_error("invalid version");
                return -1;
            }
            log_info("found #EXT-X-VERSION:%s", t.c_str());
            continue;
        }
        if (strncmp("#EXT-X-TARGETDURATION:", tmp.data(), 22) == 0)
        {
            std::string t(tmp.data(), 22, p - s - 22);
            try
            {
                target_duration_ = std::stol(t);
            }
            catch (...)
            {
                log_error("invalid target_duration_");
                return -1;
            }
            log_info("found #EXT-X-TARGETDURATION:%s", t.c_str());
            continue;
        }
        if (strncmp("#EXT-X-MAP:URI=", tmp.data(), 15) == 0)
        {
            discount = false;
            std::string t(tmp.data(), 16, p - s - 16);
            std::string name, url;
            if (t.find_first_of("https://") == 0 || t.find_first_of("https://") == 0)
            {
                auto index = t.find_last_of('/');
                name = std::string(t, index + 1, t.size());
                url = std::string(t, 0, index + 1);    // include last '/'
            }
            else
            {
                name = t;
            }
            if (discount)
            {
                log_info("header file will update");
                last_header_ = std::make_shared<m4s_t>(name, 0, true, url);
                discount = false;
            }
            else
            {
                cur_header_ = std::make_shared<m4s_t>(name, 0, true, url);
                log_info("found header file, url=%s name=%s", cur_header_->url.c_str(), cur_header_->name.c_str());
                m4s_list_.push_back(cur_header_);
            }
            continue;
        }
        if (strncmp("#EXT-X-PLAYLIST-TYPE:", tmp.data(), 21) == 0)
        {
            std::string t(tmp.data(), 21, p - s - 21);
            if (strncmp("VOD", t.data(), 3) == 0)
            {
                is_vod_ = true;
            }
            else
            {
                is_vod_ = false;
            }
            log_info("playlist is vod=%d", is_vod_);
            continue;
        }
        if (strncmp("#EXT-X-DISCONTINUITY", tmp.data(), 20) == 0)
        {
            log_info("found #EXT-X-DISCONTINUITY metadata will change");
            discount = true;
            continue;
        }
        if (strncmp("#EXTINF:", tmp.data(), 8) == 0)
        {
            std::string t(tmp.data(), 8, p - s - 8);
            try
            {
                next_m4s_duration = std::stol(t);
            }
            catch (...)
            {
                log_error("invalid #EXTINF:%s", t.c_str());
                return -1;
            }
            next_need_m4s = true;
            continue;
        }
        
        if (strncmp("#EXT-X-ENDLIST", tmp.data(), 14) == 0)
        {
            log_info("found #EXT-X-ENDLIST");
            is_end_ = true;
            break;
        }
    }
    
    return 0;
}


std::string Playlist::m3u8_content()
{
    return raw_m3u8_;
}


bool Playlist::is_vod()
{
    return is_vod_;
}


bool Playlist::is_end()
{
    return is_end_ || is_vod_;
}


float Playlist::target_duration()
{
    return target_duration_;
}


std::shared_ptr<m4s_t> Playlist::last_header()
{
    return last_header_;
}



std::shared_ptr<m4s_t> Playlist::current_header()
{
    return cur_header_;
}



std::vector<std::shared_ptr<m4s_t>> Playlist::m4s_list()
{
    return m4s_list_;
}