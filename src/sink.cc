/*
 * @Author: kay1ess 
 * @Date: 2021-10-20 00:25:07 
 * @Last Modified by: kay1ess
 * @Last Modified time: 2021-10-20 00:31:59
 */

#include "sink.h"

void BaseSink::UpdateInfo(video_info_t* v, audio_info_t* a) {
    if (v) {
        video_info_ = *v;
    }
    if (a) {
        audio_info_ = *a;
    }
}