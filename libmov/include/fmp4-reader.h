/*
 * @Author: kay1ess 
 * @Date: 2021-10-16 19:42:01 
 * @Last Modified by: kay1ess
 * @Last Modified time: 2021-10-18 23:11:07
 */

#ifndef _fmp4_reader_h_
#define _fmp4_reader_h_

#include "mov-reader.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mov_reader_t fmp4_reader_t;


fmp4_reader_t* fmp4_reader_create(const struct mov_buffer_t* buffer, void* params);

void fmp4_reader_destroy(fmp4_reader_t* mov);


int fmp4_read_init_segment(struct mov_reader_t* reader, struct mov_reader_trackinfo_t *ontrack, void* param, size_t size);


int fmp4_read_normal_segment(struct mov_reader_t* reader, void* buffer, size_t bytes, mov_reader_onread onread, void* param, size_t size);


#ifdef __cplusplus
}
#endif

#endif