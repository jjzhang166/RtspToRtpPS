#include "Wrapper.h"
#include <windows.h>

int main(int argc, char **argv)
{
	//rtsp2rtp_ps("rtsp://admin:abc12345@10.18.35.19:554/cam/realmonitor?channel=1&subtype=0", 100000, "127.0.0.1",  8088);
	//rtsp2rtp_ps("rtsp://60.164.243.246:554/181695447999916-0-0", 100000, "127.0.0.1", 8088);
	//char* rtspUrl = "rtsp://60.164.243.246:554/181695447999916-0-0";
	//char* ip = "127.0.0.1";

	//Rtsp2Ps* rtspPs = new Rtsp2Ps(rtspUrl, 10000, ip, 8088);
	//rtspPs->startReMux();
	startReMux(5125555, "rtsp://admin:abc12345@10.18.35.19:554/cam/realmonitor?channel=1&subtype=0", 10000, "127.0.0.1", 8088);
	startSend(5125555);
	Sleep(100000);

	return 0;
}