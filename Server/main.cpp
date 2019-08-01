
#if 0
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <hiredis/hiredis.h>
#include "RedisServer.h"
void doTest()
{
	//redis默认监听端口为6387 可以再配置文件中修改
	redisContext* redis = redisConnect("127.0.0.1", 6379);
	if (NULL == redis || redis->err)
	{       // redis为NULL与redis->err是两种不同的错误，若redis->err为true，可使用redis->errstr查看错误信息
		redisFree(redis);
		printf("Connect to redisServer faile\n");
		return;
	}
	printf("Connect to redisServer Success\n");
	const char* command1 = "SUBSCRIBE 1";
	redisReply* reply = (redisReply*)redisCommand(redis, command1);    // 执行命令，结果强转成redisReply*类型
	if (NULL == reply)
	{
		printf("Execut command1 failure\n");
		redisFree(redis);     // 命令执行失败，释放内存
		return;
	}
	//if (!(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str, "OK") == 0))
	//{       // 判断命令执行的返回值
	//	printf("Failed to execute command[%s]\n", command1);
	//	freeReplyObject(reply);
	//	redisFree(redis);
	//	return;
	//}
	freeReplyObject(reply);
	printf("Succeed to execute command[%s]\n", command1);
	// 一切正常，则对返回值进行处理
	while (1)
	{

	}
}
void test()
{
	RedisServer _redisServer;
	if (_redisServer.connect())
	{
		printf("connect success\n");
	}
	_redisServer.subscribe("1");
	while (1)
	{

	}
}
int main()
{
	test();
	return 0;
}
#endif


#include <iostream>
using namespace std;

#include "UserDO.h"
#include "UserModelBase.h"
#include "ChatServer.h"

#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>


int main(int argc, char** argv)
{
	if (argc < 3)
	{
		LOG_ERROR << "run command invalid! argc must be 3!" << argc;
		return -1;
	}

	LOG_INFO << "Servers run";

	muduo::net::EventLoop loop;
	muduo::net::InetAddress serverAddr(argv[1], atoi(argv[2]));
	ChatServer server(&loop, serverAddr);
	server.start();
	loop.loop();

	return 0;
}

//单服务器的
/*
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
*/