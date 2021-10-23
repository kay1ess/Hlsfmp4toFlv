/*
 * @Author: kay 
 * @Date: 2021-10-19 17:11:22 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-20 12:47:04
 */


#include "mp4parse.h"
#include "mov-format.h"
#include <stdlib.h>
#include <functional>

using namespace std::placeholders;

struct mov_buffer_t buffer_reader =
{
    io_buffer_read,
    NULL,
    io_buffer_seek,
    io_buffer_tell,
};

Mp4Parse::Mp4Parse() : has_header_(false) {

    info = (struct mov_reader_trackinfo_t*)malloc(sizeof(struct mov_reader_trackinfo_t));
    info->onaudio = Mp4Parse::MovAudioInfo;
    info->onvideo = Mp4Parse::MovVideoInfo;
    info->onsubtitle = NULL;

    video_info_ = std::make_shared<video_info_t>();
    audio_info_ = std::make_shared<audio_info_t>();

    memset(frame_, 0, sizeof(frame_));
}

Mp4Parse::~Mp4Parse() {
    if (info)
        free(info);
    if (reader_)
        fmp4_reader_destroy(reader_);
}

void Mp4Parse::SetSink(BaseSink* sink) {
    sink_ = sink;
}

int Mp4Parse::Parse(io_buffer* buf, bool is_header) {
    if (!has_header_ && !is_header)
        return -1;
    if (reader_ == nullptr && buf) {
        reader_ = fmp4_reader_create(&buffer_reader, buf);
    }
    if (is_header) {
        has_header_ = true;
        if (reader_)
            fmp4_reader_destroy(reader_);
        else
            return -1;
        reader_ = fmp4_reader_create(&buffer_reader, buf);
        if (reader_ == NULL)
            return -1;
        fmp4_read_init_segment(reader_, info, this, buf->can_read());
    } else {
        fmp4_read_normal_segment(reader_, frame_, sizeof(frame_), Mp4Parse::MovOnRead, this, buf->can_read());
    }
    return 0;
}

void Mp4Parse::MovAudioInfo(void* param, uint32_t track, uint8_t object, int channel_count, int bit_per_sample, int sample_rate, const void* extra, size_t bytes) {
    Mp4Parse* th = (Mp4Parse*)param;
    th->audio_info_->track = track;
    th->audio_info_->codec_id = object;
    th->audio_info_->channels = channel_count;
    th->audio_info_->bit_per_sample = bit_per_sample;
    th->audio_info_->sample_rate = sample_rate;
    th->audio_info_->track = track;
    if (th->sink_) {
        th->sink_->UpdateInfo(th->video_info_, th->audio_info_);
        int ret = th->sink_->WriteTrackData(extra, bytes, MOV_TRACK_AUDIO);
        assert(ret == 0);
    }
    
}

void Mp4Parse::MovVideoInfo(void* param, uint32_t track, uint8_t object, int width, int height, const void* extra, size_t bytes) {
    Mp4Parse* th = (Mp4Parse*)param;
    th->video_info_->track = track;
    th->video_info_->codec_id = object;
    th->video_info_->width = width;
    th->video_info_->height = height;
    th->video_info_->track = track;
    if (th->sink_) {
        th->sink_->UpdateInfo(th->video_info_, th->audio_info_);
        int ret = th->sink_->WriteTrackData(extra, bytes, MOV_TRACK_VIDEO);
        assert(ret == 0);
    }
}

void Mp4Parse::MovOnRead(void* param, uint32_t track, const void* buffer, size_t bytes, int64_t pts, int64_t dts, int flags) {
    Mp4Parse* th = (Mp4Parse*)param;
    if (th->sink_) {
        th->sink_->WriteData(track, buffer, bytes, pts, dts, flags);
    }
}
