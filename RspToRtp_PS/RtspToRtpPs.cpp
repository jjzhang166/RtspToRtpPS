// RspToRtp_PS.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
//#include <map>
#include "RtspToRtpPs.h"
#include <thread>

Rtsp2Ps::Rtsp2Ps(std::string  rtspUrl, uint32_t ssrc, std::string ip, uint16_t port)
{
	m_isWorking = false;
	m_rtspUrl = rtspUrl;
	m_pclient = new RtpClient(ssrc, ip, port);
}


Rtsp2Ps::~Rtsp2Ps()
{

}

int Rtsp2Ps::freeRes() 
{
	return 0;
}

int Rtsp2Ps::startReMux()
{
	int ret = 0;
	AVStream *in_stream;
	AVDictionary *opt = NULL;
	av_dict_set(&opt, "rtsp_transport", "tcp", 0);
	AVFormatContext *ifmt_ctx = NULL;
	struct ps_muxer_t* m_ps = NULL;
	// 初始化网络
	//avformat_network_init();
	// 打开文件
	if ((ret = avformat_open_input(&ifmt_ctx, m_rtspUrl.c_str(), 0, &opt)) < 0) {
		fprintf(stderr, "Could not open input file '%s'", m_rtspUrl);
		avformat_close_input(&ifmt_ctx);
		return -1;
	}

	if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
		fprintf(stderr, "Failed to retrieve input stream information");
		avformat_close_input(&ifmt_ctx);
		return -1;
	}
	av_dump_format(ifmt_ctx, 0, m_rtspUrl.c_str(), 0);

	std::thread  th = std::thread(rtsp2mpeg_ps, ifmt_ctx, this);
	th.detach();

	//muxThread.join();
	return 0;
}

int Rtsp2Ps::stopReMux()
{
	m_isWorking = false;
	return 0;
}

int Rtsp2Ps::rtsp2mpeg_ps(AVFormatContext *ifmt_ctx, void* param)
{
	AVStream *in_stream;
	int ret = 0;
	AVPacket pkt;

	ps_muxer_func_t handler;
	handler.alloc = ps_alloc;
	handler.write = ps_write;
	handler.free = ps_free;

	struct ps_muxer_t* ps = ps_muxer_create(&handler, param);
	Rtsp2Ps* rtsp2Ps = (Rtsp2Ps*)param;
	rtsp2Ps->m_isWorking = true;


	for (unsigned int i = 0; i < ifmt_ctx->nb_streams; i++) {
		in_stream = ifmt_ctx->streams[i];
		int codecid = avCodecIdToStreamId(in_stream->codec->codec_id);
		ps_muxer_add_stream((struct ps_muxer_t*)ps, codecid, NULL, 0);
	}

	//实现REMUXING
	av_init_packet(&pkt);
	while (rtsp2Ps->m_isWorking) {
		ret = av_read_frame(ifmt_ctx, &pkt);
		if (ret < 0)
			break;

		if (pkt.stream_index > 2)
		{
			continue;
		}

		int codecid = avCodecIdToStreamId(ifmt_ctx->streams[pkt.stream_index]->codec->codec_id);

		ps_muxer_input(ps, codecid, pkt.flags ? 0x01 : 0x00, pkt.pts, pkt.dts, pkt.data, pkt.size);

		printf("ret = %d\n", ret);
		av_packet_unref(&pkt);
		//printf("size %d, dts %d  duration  %d \r\n", pkt.size, pkt.dts, pkt.duration);
	}

	rtsp2Ps->m_isWorking = false;
	avformat_close_input(&ifmt_ctx);
	ps_muxer_destroy(ps);
	return 0;
}

void * Rtsp2Ps::ps_alloc(void * param, size_t bytes)
{
	Rtsp2Ps* pRtsp2Ps = (Rtsp2Ps*)param;
	return pRtsp2Ps->m_buffer;
}

void Rtsp2Ps::ps_free(void * param, void * packet)
{
}

void Rtsp2Ps::ps_write(void * param, int stream, void * packet, size_t bytes)
{
	Rtsp2Ps* pRtsp2Ps = (Rtsp2Ps*)param;

	pRtsp2Ps->rtp_send((uint8_t*)packet, bytes);
	printf("len = %d\n", bytes);
}

int Rtsp2Ps::avCodecIdToStreamId(AVCodecID codec)
{
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
	return codecid;
}

int Rtsp2Ps::rtp_send(uint8_t *data, int len)
{
	rtp_header_t* rtp_hdr = (rtp_header_t *)m_pclient->SENDBUFFER;
	rtp_hdr->csrc_len = 0;
	rtp_hdr->extension = 0;
	rtp_hdr->padding = 0;
	rtp_hdr->version = 2;
	rtp_hdr->payload_type = PS;
	// rtp_hdr->marker = (pstStream->u32PackCount - 1 == i) ? 1 : 0;   /* 该包为一帧的结尾则置为1, 否则为0. rfc 1889 没有规定该位的用途 */
	rtp_hdr->timestamp = htonl(0);
	rtp_hdr->ssrc = htonl(m_pclient->rtpSsrc);

	int packetNum = (len + UDP_MAX_SIZE - 1) / UDP_MAX_SIZE;
	int lastPackSize = len - (packetNum - 1)*UDP_MAX_SIZE;

	int packetIndex;
	for (packetIndex = 0; packetIndex < packetNum - 1; packetIndex++)
	{
		rtp_hdr->marker = 0;
		rtp_hdr->seq_no = htons(m_pclient->seq_num++); //序列号，每发送一个RTP包增1
		memcpy(m_pclient->SENDBUFFER + RTP_HEADER_LEN, data + packetIndex * UDP_MAX_SIZE, UDP_MAX_SIZE);
		int bytes = UDP_MAX_SIZE + RTP_HEADER_LEN;
		sendto(m_pclient->uiFdsocket, (char*)m_pclient->SENDBUFFER, bytes, 0, (struct sockaddr*)&m_pclient->remoteAddr, sizeof(struct sockaddr_in));
	}

	rtp_hdr->marker = 1;
	rtp_hdr->seq_no = htons(m_pclient->seq_num++); //序列号，每发送一个RTP包增1
	memcpy(m_pclient->SENDBUFFER + RTP_HEADER_LEN, data + (packetNum - 1) * UDP_MAX_SIZE, lastPackSize);
	int bytes = lastPackSize + RTP_HEADER_LEN;
	sendto(m_pclient->uiFdsocket, (char*)m_pclient->SENDBUFFER, bytes, 0, (struct sockaddr*)&m_pclient->remoteAddr, sizeof(struct sockaddr_in));

	return 0;
}