#include <cstdio>
#include "ChatClient.h"





int main(int argc, char** argv)
{
	if (argc < 3)
	{
		LOG_ERROR << "run command invalid! argc must be 3!" << argc;
		return -1;
	}

	LOG_INFO << "ChatClient run";

	muduo::net::EventLoop loop;
	muduo::net::InetAddress serverAddr(argv[1], atoi(argv[2]));
	ChatClient client(&loop, serverAddr);
	client.connect();
	loop.loop();

	return 0;
}

//单服务器连接代码
//int main()
//{
//	LOG_INFO << "ChatClient run";
//
//	muduo::net::EventLoop loop;
//	muduo::net::InetAddress serverAddr(9999);
//	ChatClient client(&loop, serverAddr);
//	client.connect();
//	loop.loop();
//
//    return 0;
//}