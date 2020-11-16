#pragma once

#include <stdint.h>
int rtsp2rtp_ps(const char* rtspUrl, uint32_t ssrc, char* ip, uint16_t port);
