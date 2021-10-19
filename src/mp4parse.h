/*
 * @Author: kay 
 * @Date: 2021-10-19 11:37:13 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-19 21:24:57
 */

#ifndef MP4PARSE_H
#define MP4PARSE_H

#include <stdio.h>
#include "fmp4-reader.h"
#include "io-buffer.h"
#include "logging.hpp"

class Mp4Parse {
protected:
    enum track_type {
        MOV_TRACK_VIDEO = 1,
        MOV_TRACK_AUDIO = 2
    };

    typedef struct {
        int width;
        int height;
        double fps;
        int codec_id;
        uint32_t track;
    } video_info_t;

    typedef struct {
        int channels;
        int codec_id;
        int bit_per_sample;
        int sample_rate;
        uint32_t track;
    } audio_info_t;

public:
    Mp4Parse();
    ~Mp4Parse();

    int Parse(io_buffer* buf, bool is_header=false);

    virtual int WriteData(uint32_t track, const void* buffer, size_t bytes, int64_t pts, int64_t dts, int flags) { 
        log_info("pts=%d dts=%d", pts, dts);
        return 0; 
    }
    virtual int WriteTrackData(const void* buffer, size_t bytes, track_type track) {
        log_info("write track");
        return 0;
    }

protected:
    video_info_t* video_info_;
    audio_info_t* audio_info_;
    struct mov_reader_trackinfo_t* info;

private:
    fmp4_reader_t* reader_;
    const char* flv_path_;
    bool has_header_;
    io_buffer* inner_buf_;
    char frame_[1024*1024*1];

    static void MovVideoInfo(void* param, uint32_t track, uint8_t object, int width, int height, const void* extra, size_t bytes);
    static void MovAudioInfo(void* param, uint32_t track, uint8_t object, int channel_count, int bit_per_sample, int sample_rate, const void* extra, size_t bytes);
    static void MovOnRead(void* param, uint32_t track, const void* buffer, size_t bytes, int64_t pts, int64_t dts, int flags);
};


#endif