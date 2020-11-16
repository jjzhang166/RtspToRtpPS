#include "Rtsp2RtpPS.h"
#include "RtspToPS.h"
#include "RtpSend.h"
#include <stdio.h>
#include<stdlib.h>
#include <string.h>

//将一个H264文件（或其他格式）转为PS文件

static void* ps_alloc(void* param, size_t bytes)
{
	static char s_buffer[2 * 1024 * 1024];
	//assert(bytes <= sizeof(s_buffer));

	return s_buffer;
}

static void ps_free(void *param, void* packet)
{
	return;
}

static void ps_write(void* param, int stream, void* packet, size_t bytes)
{
	//fwrite(packet, bytes, 1, (FILE*)param);
	RtcClient* client = (RtcClient*)param;
	rtp_send(client, packet, bytes);
	printf("len = %d\n", bytes);
}

int rtsp2rtp_ps(const char* rtspUrl, uint32_t ssrc, char* ip, uint16_t port)
{
	struct ps_muxer_func_t handler;
	handler.alloc = ps_alloc;
	handler.write = ps_write;
	handler.free = ps_free;

	RtcClient* client = malloc(sizeof(RtcClient));
	client->seq_num = 0;
	client->ssrc = ssrc;

	memset(&(client->remoteAddr), 0, sizeof(client->remoteAddr));

	client->remoteAddr.sin_family = AF_INET;

	client->remoteAddr.sin_port = htons(port);

	client->remoteAddr.sin_addr.s_addr = inet_addr(ip);

	client->uiFdsocket = socket(AF_INET, SOCK_DGRAM, 0);


	//FILE* fp = fopen("test.dat", "wb");
	rtsp2mpeg_ps(rtspUrl, handler, client);

	return 0;
}