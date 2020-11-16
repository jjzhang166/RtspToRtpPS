#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "mpeg-ps-proto.h"
#include "mpeg-ps.h"
#ifdef __cplusplus
}
#endif

int rtsp2mpeg_ps(const char* rtspUrl, struct ps_muxer_func_t handler, void* param);