#include "Rtsp2RtpPS.h"

//将一个H264文件（或其他格式）转为PS文件

int main(int argc, char **argv)
{
	//rtsp2rtp_ps("rtsp://admin:abc12345@10.18.35.19:554/cam/realmonitor?channel=1&subtype=0", 100000, "127.0.0.1",  8088);
	rtsp2rtp_ps("rtsp://60.164.243.246:554/181695447999916-0-0", 100000, "127.0.0.1", 8088);

	return 0;
}