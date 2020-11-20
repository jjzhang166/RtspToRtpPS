#pragma once
#pragma pack (1)

#include<stdint.h>
#ifdef WIN32
#include <WINSOCK2.H>
#pragma comment(lib,"ws2_32.lib")
#else
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#endif

#include<string>

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
typedef struct RtpClient {
	int uiFdsocket;
	struct sockaddr_in remoteAddr;
	uint16_t remotePort;
	int seq_num;
	uint32_t rtpSsrc;
	uint8_t SENDBUFFER[SEND_BUF_SIZE];

	//RtpClient() {}

	RtpClient(uint32_t ssrc, std::string ip, uint16_t port)    // ¹¹Ôìº¯Êý
	{
		rtpSsrc = ssrc;
		remotePort = port;
		seq_num = 0;

		memset(&remoteAddr, 0, sizeof(remoteAddr));

		remoteAddr.sin_family = AF_INET;

		remoteAddr.sin_port = htons(port);

		remoteAddr.sin_addr.s_addr = inet_addr(ip.c_str());

		uiFdsocket = socket(AF_INET, SOCK_DGRAM, 0);
	}
}RtpClient;
