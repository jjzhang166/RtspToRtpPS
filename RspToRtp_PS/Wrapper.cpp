#include "Wrapper.h"
#include <string>
#include <map>
#include <mutex>
#include"RtspToRtpPs.h"

std::map<unsigned int, Rtsp2Ps*> reMuxTask;
std::mutex mutex;

int startReMux(unsigned int taskId, char*  rtspUrl, unsigned int ssrc, char* ip, unsigned short port)
{
	std::lock_guard<std::mutex> lock(mutex);

	if (reMuxTask.find(taskId) != reMuxTask.end())
	{
		return -1;
	}

	Rtsp2Ps* rtsp2Ps = new Rtsp2Ps(ssrc, ip, port);

	reMuxTask[taskId] = rtsp2Ps;

	int ret = rtsp2Ps->startReMux(rtspUrl);
	return ret;
}

int startSend(unsigned int taskId)
{
	std::lock_guard<std::mutex> lock(mutex);

	if (reMuxTask.find(taskId) == reMuxTask.end())
	{
		return -1;
	}

	Rtsp2Ps* rtsp2Ps = reMuxTask[taskId];
	rtsp2Ps->startSend();
	return 0;
}

int stopReMux(unsigned int taskId)
{
	std::lock_guard<std::mutex> lock(mutex);

	if (reMuxTask.find(taskId) == reMuxTask.end())
	{
		return -1;
	}

	Rtsp2Ps* rtsp2Ps = reMuxTask[taskId];
	rtsp2Ps->stopReMux();
	delete[] rtsp2Ps;

	return 0;
}


