/*
 * @Author: kay 
 * @Date: 2021-10-19 11:37:13 
 * @Last Modified by: kay1ess
 * @Last Modified time: 2021-10-20 00:23:23
 */

#ifndef MP4PARSE_H
#define MP4PARSE_H

#include <stdio.h>
#include "fmp4-reader.h"
#include "io-buffer.h"
#include "logging.hpp"
#include "sink.h"

class Mp4Parse {
public:
    Mp4Parse();
    ~Mp4Parse();

    void SetSink(BaseSink* sink);
    int Parse(io_buffer* buf, bool is_header=false);

private:
    fmp4_reader_t* reader_;
    const char* flv_path_;
    bool has_header_;
    char frame_[1024*1024*1];
    video_info_t* video_info_;
    audio_info_t* audio_info_;
    struct mov_reader_trackinfo_t* info;
    BaseSink *sink_;

    static void MovVideoInfo(void* param, uint32_t track, uint8_t object, int width, int height, const void* extra, size_t bytes);
    static void MovAudioInfo(void* param, uint32_t track, uint8_t object, int channel_count, int bit_per_sample, int sample_rate, const void* extra, size_t bytes);
    static void MovOnRead(void* param, uint32_t track, const void* buffer, size_t bytes, int64_t pts, int64_t dts, int flags);
};


#endif