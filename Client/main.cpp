#include <cstdio>
#include "ChatClient.h"

int main()
{
	LOG_INFO << "ChatClient run";

	muduo::net::EventLoop loop;
	muduo::net::InetAddress serverAddr(9999);
	ChatClient client(&loop, serverAddr);
	client.connect();
	loop.loop();

    return 0;
}