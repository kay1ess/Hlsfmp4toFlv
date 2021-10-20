/*
 * @Author: kay1ess 
 * @Date: 2021-10-20 00:08:40 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-20 16:05:23
 */

#ifndef SINK_H
#define SINK_H

#include <inttypes.h>
#include <stdlib.h>
#include <memory>
#include <string>
#include "logging.hpp"
#include "flv-header.h"
#include "flv-muxer.h"
#include "flv-proto.h"
#include "flv-writer.h"

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


class BaseSink {
public:
    virtual ~BaseSink() {}
    virtual int WriteTrackData(const void* buffer, size_t bytes, track_type track) = 0;
    virtual int WriteData(uint32_t track, const void* buffer, size_t bytes, int64_t pts, int64_t dts, int flags) = 0;
    void UpdateInfo(std::shared_ptr<video_info_t> v, std::shared_ptr<audio_info_t> a);
protected:
    std::shared_ptr<video_info_t> video_info_;
    std::shared_ptr<audio_info_t> audio_info_;
};


class FlvFileSink : public BaseSink {
public:
    FlvFileSink(std::string path);
    ~FlvFileSink();
    int WriteTrackData(const void* buffer, size_t bytes, track_type track) override;
    int WriteData(uint32_t track, const void* buffer, size_t bytes, int64_t pts, int64_t dts, int flags) override;
private:
    void* writer_;
    uint8_t packet_[1*1024*1024];
    struct flv_video_tag_header_t video_tag_;
    struct flv_audio_tag_header_t audio_tag_;
};

#endif