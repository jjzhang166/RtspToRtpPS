#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	int startReMux(unsigned int taskId, char*  rtspUrl, unsigned int ssrc, char* ip, unsigned short port);
	int startSend(unsigned int taskId);
	int stopReMux(unsigned int taskId);

#ifdef __cplusplus
}
#endif