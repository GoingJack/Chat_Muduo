#include <iostream>
using namespace std;

#include "UserDO.h"
#include "UserModelBase.h"
#include "ChatServer.h"

#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>


int main()
{
	LOG_INFO << "main pid:" << getpid();
	int maintid = syscall(SYS_gettid);//获取main线程的线程ID
	LOG_INFO << "main tid:" << maintid;
	muduo::net::EventLoop loop;
	muduo::net::InetAddress listenAddr(9999);
	ChatServer server(&loop, listenAddr);
	server.start();
	loop.loop();
	
    return 0;
}