#include "mov-internal.h"
#include "fmp4-reader.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define DIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))

fmp4_reader_t* fmp4_reader_create(const struct mov_buffer_t* buffer, void* param)
{
	fmp4_reader_t* reader;
	reader = (fmp4_reader_t*)calloc(1, sizeof(*reader));
	if (NULL == reader)
		return NULL;

	// ISO/IEC 14496-12:2012(E) 4.3.1 Definition (p17)
	// Files with no file-type box should be read as if they contained an FTYP box 
	// with Major_brand='mp41', minor_version=0, and the single compatible brand 'mp41'.
	reader->mov.ftyp.major_brand = MOV_BRAND_MP41;
	reader->mov.ftyp.minor_version = 0;
	reader->mov.ftyp.brands_count = 0;
	reader->mov.header = 0;

	reader->mov.io.param = param;
	memcpy(&reader->mov.io.io, buffer, sizeof(reader->mov.io.io));
	return reader;
}

void fmp4_reader_destroy(fmp4_reader_t* reader)
{
	mov_reader_destroy(reader);
}

int fmp4_read_init_segment(fmp4_reader_t* reader, struct mov_reader_trackinfo_t *ontrack, void* param, size_t size)
{
	int i;
	uint32_t j;
	struct mov_t* mov;
	struct mov_track_t* track;
    struct mov_sample_entry_t* entry;
	
	mov = &reader->mov;

	mov_reader_root2(mov, size);

	for (i = 0; i < mov->track_count; i++)
	{
		track = mov->tracks + i;
		mov_index_build(track);
	}

	mov_reader_getinfo(reader, ontrack, param);
	return 0;
}

int fmp4_read_normal_segment(fmp4_reader_t* reader, void* buffer, size_t bytes, mov_reader_onread onread, void* param, size_t size)
{
	struct mov_t* mov;
	struct mov_sample_t smpl;

	mov = &reader->mov;

	mov_reader_root2(mov, size);

	while (mov_reader_read(reader, buffer, bytes, onread, param) > 0)
	{
	};

	return 0;
}

static int mov_fragment_seek_get_duration(struct mov_t* mov)
{
	int i;
	struct mov_track_t* track;
	track = mov->track_count > 0 ? &mov->tracks[0] : NULL;
	if (track && track->frag_capacity < track->frag_count && track->mdhd.timescale)
	{
		mov_buffer_seek(&mov->io, track->frags[track->frag_count - 1].offset);
		mov_reader_root(mov); // moof

		track->mdhd.duration = track->samples[track->sample_count - 1].dts - track->samples[0].dts;
		mov->mvhd.duration = track->mdhd.duration * mov->mvhd.timescale / track->mdhd.timescale;
		
		// clear samples and seek to the first moof
		for (i = 0; i < mov->track_count; i++)
		{
			mov->tracks[i].sample_count = 0;
			mov->tracks[i].sample_offset = 0;
		}
		track->frag_capacity = 0;
	}

	return 0;
}

int mov_fragment_seek_read_mfra(struct mov_t* mov)
{
	uint64_t pos;
	pos = mov_buffer_tell(&mov->io); // for fallback
	mov_buffer_seek(&mov->io, -16);
	mov_reader_root(mov); // mfro
	if (mov->mfro > 0)
	{
		mov_buffer_seek(&mov->io, -((int64_t)mov->mfro));
		mov_reader_root(mov); // mfra
		mov_fragment_seek_get_duration(mov); // for get fmp4 duration
	}
	mov_buffer_seek(&mov->io, pos);
	return mov_buffer_error(&mov->io);
}

int mov_fragment_seek(struct mov_t* mov, int64_t* timestamp)
{
	int i;
	uint64_t clock;
	size_t idx, start, end;
	struct mov_track_t* track;
	struct mov_fragment_t* frag, *prev, *next;

	track = mov->track_count > 0 ? &mov->tracks[0] : NULL;
	if (!track || track->frag_count < 1)
		return -1;

	idx = start = 0;
	end = track->frag_count;
	assert(track->frag_count > 0);
	clock = (uint64_t)(*timestamp) * track->mdhd.timescale / 1000; // mvhd timescale

	while (start < end)
	{
		idx = (start + end) / 2;
		frag = &track->frags[idx];

		if (frag->time > clock)
			end = idx;
		else if (frag->time < clock)
			start = idx + 1;
		else
			break;
	}

	frag = &track->frags[idx];
	prev = &track->frags[idx > 0 ? idx - 1 : idx];
	next = &track->frags[idx + 1 < track->frag_count ? idx + 1 : idx];
	if (DIFF(prev->time, clock) < DIFF(frag->time, clock))
		frag = prev;
	if (DIFF(next->time, clock) < DIFF(frag->time, clock))
		frag = next;

	*timestamp = frag->time * 1000 / track->mdhd.timescale;
	
	// clear samples and seek
	for (i = 0; i < mov->track_count; i++)
	{
		mov->tracks[i].sample_count = 0;
		mov->tracks[i].sample_offset = 0;
	}
	track->frag_capacity = (uint32_t)idx;
	return 0;
}

int mov_fragment_read_next_moof(struct mov_t* mov)
{
	int i;
	struct mov_track_t* track;

	// clear moof samples
	for (i = 0; i < mov->track_count; i++)
	{
		mov->tracks[i].sample_count = 0;
		mov->tracks[i].sample_offset = 0;
	}

	track = mov->track_count > 0 ? &mov->tracks[0] : NULL;
	if (track && track->frag_capacity < track->frag_count)
	{
		mov_buffer_seek(&mov->io, track->frags[track->frag_capacity++].offset);
		mov_reader_root(mov); // moof
		return 0;
	}

	return 1; // eof
}
