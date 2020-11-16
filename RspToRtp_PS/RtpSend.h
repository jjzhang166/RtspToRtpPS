#pragma once
#include <stdint.h>
#ifdef WIN32
#include <WINSOCK2.H>
#else
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#endif

typedef struct RtcClient{
	int uiFdsocket;
	struct sockaddr_in remoteAddr;
	int seq_num;
	uint32_t ssrc;

} RtcClient;

int rtp_send(struct RtcClient* client, uint8_t *data, int len);

