#include "fmp4-reader.h"
#include "mov-format.h"
#include "mpeg4-avc.h"
#include "mpeg4-aac.h"
#include "flv-proto.h"
#include "flv-header.h"
#include "flv-writer.h"
#include "flv-muxer.h"
#include "io-buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

extern "C" const struct mov_buffer_t* mov_file_buffer(void);

struct mov_buffer_t buffer_reader =
{
    io_buffer_read,
    NULL,
    io_buffer_seek,
    io_buffer_tell,
};

static uint8_t s_packet[2 * 1024 * 1024];
static uint8_t s_buffer[4 * 1024 * 1024];
static uint32_t s_aac_track;
static uint32_t s_avc_track;
static uint32_t s_txt_track;
static uint8_t s_video_type;
static struct flv_audio_tag_header_t s_audio_tag;
static struct flv_video_tag_header_t s_video_tag;

static int iskeyframe(const uint8_t* data, size_t bytes)
{
	size_t nalu_length = 4;
	uint32_t len;

	while (bytes >= nalu_length + 1)
	{
		len = 0;
		for (size_t i = 0; i < nalu_length; i++)
			len = (len << 8) | data[i];

		if (len + nalu_length > bytes)
			return 0; // invalid

		uint8_t nalu_type = (FLV_VIDEO_H264 == s_video_type) ? (data[nalu_length] & 0x1f) : ((data[nalu_length] >> 1) & 0x3f);
		if ((FLV_VIDEO_H264 == s_video_type) ? (5 == nalu_type) : (16 <= nalu_type && nalu_type <= 23))
			return 1;

		bytes -= nalu_length + len;
		data += nalu_length + len;
	}

	return 0;
}

static void onread(void* flv, uint32_t track, const void* buffer, size_t bytes, int64_t pts, int64_t dts, int flags)
{
	static int64_t last_video_dts, last_audio_dts;
	if (s_avc_track == track)
	{
		int keyframe = (FLV_VIDEO_H264 == s_video_type || FLV_VIDEO_H265 == s_video_type) ? iskeyframe((const uint8_t*)buffer, bytes) : flags;
		s_video_tag.keyframe = (keyframe ? 1 : 2);
		s_video_tag.avpacket = FLV_AVPACKET;
		s_video_tag.cts = (int32_t)(pts - dts);
		flv_video_tag_header_write(&s_video_tag, s_packet, sizeof(s_packet));
		memcpy(s_packet + 5, buffer, bytes);

		if (last_video_dts == 0)
		{
			last_video_dts = dts;
			dts = 0;
		}
		else
		{
			dts -= last_video_dts;
		}
		printf("[V] pts: %08" PRId64 ", dts: %08" PRId64 " %s\n", pts, dts, keyframe ? " [I]" : "");
		flv_writer_input(flv, FLV_TYPE_VIDEO, s_packet, bytes + 5, (uint32_t)dts);
	}
	else if (s_aac_track == track)
	{
		s_audio_tag.avpacket = FLV_AVPACKET;
		int m = flv_audio_tag_header_write(&s_audio_tag, s_packet, sizeof(s_packet));
		memcpy(s_packet + m, buffer, bytes); // AAC exclude ADTS
		if (last_audio_dts == 0)
		{
			last_audio_dts = dts;
			dts = 0;
		}
		else
		{
			dts -= last_audio_dts;
		}
		printf("[A] pts: %08" PRId64 ", dts: %08" PRId64 "\n", pts, dts);
		flv_writer_input(flv, FLV_TYPE_AUDIO, s_packet, bytes + m, (uint32_t)dts);
	}
	else
	{
		assert(0);
	}
}

static void mov_video_info(void* flv, uint32_t track, uint8_t object, int width, int height, const void* extra, size_t bytes)
{
	s_avc_track = track;
	assert(MOV_OBJECT_H264 == object || MOV_OBJECT_HEVC == object || MOV_OBJECT_AV1 == object || MOV_OBJECT_VP9 == object);
	s_video_type = MOV_OBJECT_H264 == object ? FLV_VIDEO_H264 : (MOV_OBJECT_HEVC == object ? FLV_VIDEO_H265 : FLV_VIDEO_AV1);
	s_video_tag.codecid = s_video_type;
	s_video_tag.keyframe = 1;
	s_video_tag.avpacket = FLV_SEQUENCE_HEADER;
	s_video_tag.cts = 0;
	flv_video_tag_header_write(&s_video_tag, s_packet, sizeof(s_packet));
	memcpy(s_packet + 5, extra, bytes);
	flv_writer_input(flv, FLV_TYPE_VIDEO, s_packet, bytes + 5, 0);
	printf("width=%d height=%d\n", width, height);
}

static void mov_audio_info(void* flv, uint32_t track, uint8_t object, int channel_count, int bit_per_sample, int sample_rate, const void* extra, size_t bytes)
{
	if (MOV_OBJECT_AAC == object || MOV_OBJECT_OPUS == object)
	{
		s_aac_track = track;
		s_audio_tag.codecid = MOV_OBJECT_AAC == object ? FLV_AUDIO_AAC : FLV_AUDIO_OPUS;
		s_audio_tag.rate = 3; // 44k-SoundRate
		s_audio_tag.bits = 1; // 16-bit samples
		s_audio_tag.channels = 1; // Stereo sound
		s_audio_tag.avpacket = FLV_SEQUENCE_HEADER;
		int m = flv_audio_tag_header_write(&s_audio_tag, s_packet, sizeof(s_packet));

		memcpy(s_packet + m, extra, bytes);
		flv_writer_input(flv, FLV_TYPE_AUDIO, s_packet, bytes + m, 0);
		printf("channel_count=%d bit_per_sample=%d sample_rate=%d\n", channel_count, bit_per_sample, sample_rate);
	}
}

static int mov_meta_info(void* flv, int type, const void* data, size_t bytes, uint32_t timestamp)
{
	return flv_writer_input(flv, FLV_TYPE_SCRIPT, data, bytes, 0);
}

void fmp4_2_flv_test(const char* header, const char* m1, const char* m2, const char* m3)
{
	snprintf((char*)s_packet, sizeof(s_packet), "%s.flv", header);
	
	void* flv = flv_writer_create((char*)s_packet);

	io_buffer buf(1024);

	fmp4_reader_t *fmp4 = fmp4_reader_create(&buffer_reader, &buf);

	FILE* fp = fopen(header, "rb");
	size_t size = fread(s_buffer, 1, sizeof(s_buffer), fp);
	buf.write(s_buffer, size);
	fclose(fp);

	struct mov_reader_trackinfo_t info = { mov_video_info, mov_audio_info };
	fmp4_read_init_segment(fmp4, &info, flv, buf.can_read());

	const char* files[] = {m1, m2, m3};

	for (int i=0; i < 3; i++)
	{
		fp = fopen(files[i], "rb");
		size = fread(s_buffer, 1, sizeof(s_buffer), fp);
		buf.write(s_buffer, size);
		fclose(fp);
		memset(s_buffer, 0, sizeof(s_buffer));
		fmp4_read_normal_segment(fmp4, s_buffer, sizeof(s_buffer), onread, flv, buf.can_read());
	}

	flv_writer_destroy(flv);
}


int main(int argc, char** argv)
{
	if (argc != 5)
	{
		printf("userage:\n\t[fmp42flv] header.m4s m1.m4s m2.m4s m3.m4s\n");
		return -1;
	}
	else 
	{
		fmp4_2_flv_test(argv[1], argv[2], argv[3], argv[4]);
		return 0;
	}
}