#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "mpeg-ps-proto.h"
#include "mpeg-ps.h"

#include "libavcodec/avcodec.h"
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
#include <libswscale/swscale.h> 
#include "libavutil/mathematics.h"
#ifdef __cplusplus
}
#endif
#include "rtp.h"
#include <string>

class Rtsp2Ps
{
public:
	Rtsp2Ps(std::string  rtspUrl, uint32_t ssrc, std::string ip, uint16_t port);
	~Rtsp2Ps();

	int startReMux();
	int stopReMux();
private:
	int rtp_send(uint8_t * data, int len);
    static int rtsp2mpeg_ps(AVFormatContext *ifmt_ctx, void* param);
	int freeRes();
	static void* ps_alloc(void* param, size_t bytes);
	static void ps_free(void *param, void* packet);
	static void ps_write(void* param, int stream, void* packet, size_t bytes);
	static int avCodecIdToStreamId(AVCodecID codec);

private:
	std::string m_rtspUrl;
	bool m_isWorking;
	unsigned char m_buffer[1024 * 1024];
	RtpClient* m_pclient;
};

