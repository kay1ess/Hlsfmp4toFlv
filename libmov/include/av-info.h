/*
 * @Author: kay 
 * @Date: 2021-10-20 12:12:13 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-20 12:15:05
 */

#ifndef _av_info_h
#define _av_info_h

#include "mov-format.h"
#include <stdlib.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline int keyframe(const uint8_t* data, size_t bytes, uint32_t object) {
    size_t nalu_length = 4, len = 0;
    while (bytes >= nalu_length + 1)
    {
        len = 0;
        for (size_t i = 0; i < nalu_length; i++)
            len = (len << 8) | data[i];

        if (len + nalu_length > bytes)
            return false; // invalid

        uint8_t nalu_type = (MOV_OBJECT_H264 == object) ? (data[nalu_length] & 0x1f) : ((data[nalu_length] >> 1) & 0x3f);
        if ((MOV_OBJECT_H264 == object) ? (5 == nalu_type) : (19 == nalu_type))
            return 1;

        bytes -= nalu_length + len;
        data += nalu_length + len;
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

#endif