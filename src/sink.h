/*
 * @Author: kay1ess 
 * @Date: 2021-10-20 00:08:40 
 * @Last Modified by: kay1ess
 * @Last Modified time: 2021-10-20 00:30:38
 */

#ifndef SINK_H
#define SINK_H

#include <inttypes.h>
#include <stdlib.h>

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
    void UpdateInfo(video_info_t* v, audio_info_t* a);
protected:
    video_info_t video_info_;
    audio_info_t audio_info_;
};

#endif