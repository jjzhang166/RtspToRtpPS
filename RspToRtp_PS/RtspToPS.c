// RspToRtp_PS.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
//#include <map>
#include "RtspToPS.h"
#ifdef __cplusplus
extern "C" {
#endif
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

static int rtsp_ondemux(void* ps, int codec, const void* data, size_t bytes, uint64_t pts, uint64_t dts, int flags)
{
	printf("[%d] pts: %08lu, dts: %08lu%s\n", codec, (unsigned long)pts, (unsigned long)dts, flags ? " [I]" : "");

	/*static std::map<int, int> streams;
	std::map<int, int>::const_iterator it = streams.find(codec);
	if (streams.end() == it)
	{
		switch (codec)
		{
		case AV_CODEC_ID_PCM_ALAW:
			i = ps_muxer_add_stream((ps_muxer_t*)ps, STREAM_AUDIO_G711A, NULL, 0);
			streams[AV_CODEC_ID_PCM_ALAW] = i;
			break;
		case STREAM_AUDIO_AAC:
			i = ps_muxer_add_stream((ps_muxer_t*)ps, STREAM_AUDIO_AAC, NULL, 0);
			streams[STREAM_AUDIO_AAC] = i;
			return 0;
		case AV_CODEC_ID_H264:
			i = ps_muxer_add_stream((ps_muxer_t*)ps, STREAM_VIDEO_H264, NULL, 0);
			streams[STREAM_VIDEO_H264] = i;
			return 0;
		case AV_CODEC_ID_H265:
			i = ps_muxer_add_stream((ps_muxer_t*)ps, STREAM_VIDEO_H265, NULL, 0);
			streams[AV_CODEC_ID_H265] = i;
			return 0;
		default: return 0;
		}
		streams[codec] = i;
	}
	else
	{
		i = it->second;
	}*/
	int codecid = 0;
	switch (codec)
	{
	case AV_CODEC_ID_PCM_ALAW:
		codecid = STREAM_AUDIO_G711A;
		break;
	case STREAM_AUDIO_AAC:
		codecid = STREAM_AUDIO_AAC;
		break;
	case AV_CODEC_ID_H264:
		codecid = STREAM_VIDEO_H264;
		break;
	case AV_CODEC_ID_H265:
		codecid = STREAM_VIDEO_H265;
		break;
	default:
		break;
	}

	return ps_muxer_input((struct ps_muxer_t*)ps, codecid, flags ? 0x01 : 0x00, pts, dts, data, bytes);
}

int rtsp2mpeg_ps(const char* rtspUrl, struct ps_muxer_func_t handler, void* param)
{
	AVStream *in_stream;

	int ret = 0;

	//FILE* fp = fopen("test.dat", "wb");
	struct ps_muxer_t* ps = ps_muxer_create(&handler, param);


	AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
	AVPacket pkt;

	//av_register_all();
	AVDictionary *opt = NULL;
	av_dict_set(&opt, "rtsp_transport", "tcp", 0);
	if ((ret = avformat_open_input(&ifmt_ctx, rtspUrl, 0, &opt)) < 0) {
		fprintf(stderr, "Could not open input file '%s'", rtspUrl);
		goto end;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		fprintf(stderr, "Failed to retrieve input stream information");
		goto end;
	}
	av_dump_format(ifmt_ctx, 0, rtspUrl, 0);
	int nVideoFramesNum = 0;

	for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
		in_stream = ifmt_ctx->streams[i];
		if (in_stream->codec->codec_id == AV_CODEC_ID_H265)
		{
			ps_muxer_add_stream((struct ps_muxer_t*)ps, STREAM_VIDEO_H265, NULL, 0);
		}
		if (in_stream->codec->codec_id == AV_CODEC_ID_PCM_ALAW)
		{
			ps_muxer_add_stream((struct ps_muxer_t*)ps, STREAM_AUDIO_G711A, NULL, 0);
		}
		if (in_stream->codec->codec_id == AV_CODEC_ID_AAC)
		{
			ps_muxer_add_stream((struct ps_muxer_t*)ps, STREAM_AUDIO_AAC, NULL, 0);
		}
		if (in_stream->codec->codec_id == AV_CODEC_ID_H264)
		{
			ps_muxer_add_stream((struct ps_muxer_t*)ps, STREAM_VIDEO_H264, NULL, 0);
		}
	}
	//实现REMUXING
	while (1) {
		ret = av_read_frame(ifmt_ctx, &pkt);
		if (ret < 0)
			break;

		ret = rtsp_ondemux(ps, ifmt_ctx->streams[pkt.stream_index]->codec->codec_id, pkt.data, pkt.size, pkt.pts, pkt.dts, pkt.flags ? 0x01 : 0x00);

		printf("ret = %d\n", ret);
		av_packet_unref(&pkt);

		//printf("size %d, dts %d  duration  %d \r\n", pkt.size, pkt.dts, pkt.duration);

	}

end:
	ps_muxer_destroy(ps);
	avformat_close_input(&ifmt_ctx);
	return 0;
}
