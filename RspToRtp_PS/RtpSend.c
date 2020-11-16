#include <stdint.h>
#include<string.h>
#include "RtpSend.h"
#pragma pack (1)
#pragma comment(lib,"ws2_32.lib")

#define PS  96
#define UDP_MAX_SIZE 1400
#define RTP_HEADER_LEN 12

typedef struct rtp_header {
	/* little-endian */
	/* byte 0 */
	uint8_t csrc_len : 4;  /* bit: 0~3 */
	uint8_t extension : 1;  /* bit: 4 */
	uint8_t padding : 1;  /* bit: 5*/
	uint8_t version : 2;  /* bit: 6~7 */
	/* byte 1 */
	uint8_t payload_type : 7;  /* bit: 0~6 */
	uint8_t marker : 1;  /* bit: 7 */
	/* bytes 2, 3 */
	uint16_t seq_no;
	/* bytes 4-7 */
	uint32_t timestamp;
	/* bytes 8-11 */
	uint32_t ssrc;
} rtp_header_t; /* 12 bytes */

#define SEND_BUF_SIZE               1500

uint8_t SENDBUFFER[SEND_BUF_SIZE];

int rtp_send(struct RtcClient* client, uint8_t *data, int len)
{
	rtp_header_t* rtp_hdr = (rtp_header_t *)SENDBUFFER;
	rtp_hdr->csrc_len = 0;
	rtp_hdr->extension = 0;
	rtp_hdr->padding = 0;
	rtp_hdr->version = 2;
	rtp_hdr->payload_type = PS;
	// rtp_hdr->marker = (pstStream->u32PackCount - 1 == i) ? 1 : 0;   /* 该包为一帧的结尾则置为1, 否则为0. rfc 1889 没有规定该位的用途 */
	rtp_hdr->timestamp = htonl(0);
	rtp_hdr->ssrc = htonl(client->ssrc);

	int packetNum = (len + UDP_MAX_SIZE - 1) / UDP_MAX_SIZE;
	int lastPackSize = len - (packetNum - 1)*UDP_MAX_SIZE;

	int packetIndex;
	for (packetIndex = 0; packetIndex < packetNum - 1; packetIndex++)
	{
		rtp_hdr->marker = 0;
		rtp_hdr->seq_no = htons(client->seq_num++); //序列号，每发送一个RTP包增1
		memcpy(SENDBUFFER + RTP_HEADER_LEN, data + packetIndex * UDP_MAX_SIZE, UDP_MAX_SIZE);
		int bytes = UDP_MAX_SIZE + RTP_HEADER_LEN;
		sendto(client->uiFdsocket, SENDBUFFER, bytes, 0, (struct  sockaddr*)&client->remoteAddr, sizeof(struct sockaddr_in));
	}

	rtp_hdr->marker = 1;
	rtp_hdr->seq_no = htons(client->seq_num++); //序列号，每发送一个RTP包增1
	memcpy(SENDBUFFER + RTP_HEADER_LEN, data + (packetNum - 1) * UDP_MAX_SIZE, lastPackSize);
	int bytes = lastPackSize + RTP_HEADER_LEN;
	sendto(client->uiFdsocket, SENDBUFFER, bytes, 0, (struct  sockaddr*)&client->remoteAddr, sizeof(struct sockaddr_in));

	return 0;
}