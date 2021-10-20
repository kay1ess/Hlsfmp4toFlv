/*
 * @Author: kay1ess 
 * @Date: 2021-10-20 00:25:07 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-20 16:16:24
 */

#include <assert.h>
#include "sink.h"
#include "string.h"
#include "mov-format.h"
#include "av-info.h"

void BaseSink::UpdateInfo(std::shared_ptr<video_info_t> v, std::shared_ptr<audio_info_t> a) {
    if (v) {
        video_info_ = v;
    }
    if (a) {
        audio_info_ = a;
    }
}

FlvFileSink::FlvFileSink(std::string path) {
    writer_ = flv_writer_create(path.c_str());
    if (writer_ == NULL) {
        log_error("create flv writer failed");
    }
}

FlvFileSink::~FlvFileSink() {
    if (writer_)
        flv_writer_destroy(writer_);
}

int FlvFileSink::WriteTrackData(const void* buffer, size_t bytes, track_type track) {
    if (bytes > sizeof(packet_)) {
        log_error("write data(%zd) more than %zd", bytes, sizeof(packet_));
        return -1;
    }
    memset(packet_, 0, sizeof(packet_));
    if (track == MOV_TRACK_VIDEO) {
        if (video_info_->codec_id == MOV_OBJECT_H264) {
            video_tag_.codecid = FLV_VIDEO_H264;
        } else if (video_info_->codec_id == MOV_OBJECT_HEVC) {
            video_tag_.codecid = FLV_VIDEO_H265;
        } else if (video_info_->codec_id == MOV_OBJECT_AV1) {
            video_tag_.codecid = FLV_VIDEO_AV1;
        } else {
            log_error("unsupported video codec id=%d", video_info_->codec_id);
            return -1;
        }
        video_tag_.keyframe = 1;
        video_tag_.avpacket = FLV_SEQUENCE_HEADER;
        video_tag_.cts = 0;
        int n = flv_video_tag_header_write(&video_tag_, packet_, sizeof(packet_));
        memcpy(packet_ + n, buffer, bytes);
        flv_writer_input(writer_, FLV_TYPE_VIDEO, packet_, bytes + n, 0);
        return 0;
    }

    if (track == MOV_TRACK_AUDIO) {
        if (audio_info_->codec_id == MOV_OBJECT_AAC) {
            audio_tag_.codecid = FLV_AUDIO_AAC;
        } else {
            log_error("unsupported audio codec id=%d", video_info_->codec_id);
            return -1;
        }
        // https://www.adobe.com/content/dam/acom/en/devnet/flv/video_file_format_spec_v10.pdf Page:6
        audio_tag_.rate = 3;
        audio_tag_.bits = 1;
        audio_tag_.channels = audio_info_->channels;
        audio_tag_.avpacket = FLV_SEQUENCE_HEADER;
        int n = flv_audio_tag_header_write(&audio_tag_, packet_, sizeof(packet_));
        memcpy(packet_ + n, buffer, bytes);
        flv_writer_input(writer_, FLV_TYPE_AUDIO, packet_, bytes + n, 0);
    }
    return 0;
}

int FlvFileSink::WriteData(uint32_t track, const void* buffer, size_t bytes, int64_t pts, int64_t dts, int flags) {
    memset(packet_, 0, sizeof(packet_));
    if (track == video_info_->track) {
        video_tag_.keyframe = keyframe((const uint8_t*)buffer, bytes, video_info_->codec_id) == 1 ? 1 : 2;
        int frame_type = 0;
        video_tag_.avpacket = FLV_AVPACKET;
        video_tag_.cts = (int32_t)(pts - dts);
        log_info("[V] pts=%" PRId64 " dts=%" PRId64 "", pts, dts);
        assert(video_tag_.cts >= 0);
        int n = flv_video_tag_header_write(&video_tag_, packet_, sizeof(packet_));
        memcpy(packet_ + n, buffer, bytes);
        flv_writer_input(writer_, FLV_TYPE_VIDEO, packet_, bytes + n, (uint32_t)dts);
    } else if (track == audio_info_->track) {
        audio_tag_.avpacket = FLV_AVPACKET;
        int n = flv_audio_tag_header_write(&audio_tag_, packet_, sizeof(packet_));
        memcpy(packet_ + n, buffer, bytes);
        flv_writer_input(writer_, FLV_TYPE_AUDIO, packet_, bytes + n, (uint32_t)dts);
        log_info("[A] dts=%" PRId64 "", dts);
    } else {
        assert(0);
    }
    return 0;
}