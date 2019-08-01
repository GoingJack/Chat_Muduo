
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
	//redisĬ�ϼ����˿�Ϊ6387 �����������ļ����޸�
	redisContext* redis = redisConnect("127.0.0.1", 6379);
	if (NULL == redis || redis->err)
	{       // redisΪNULL��redis->err�����ֲ�ͬ�Ĵ�����redis->errΪtrue����ʹ��redis->errstr�鿴������Ϣ
		redisFree(redis);
		printf("Connect to redisServer faile\n");
		return;
	}
	printf("Connect to redisServer Success\n");
	const char* command1 = "SUBSCRIBE 1";
	redisReply* reply = (redisReply*)redisCommand(redis, command1);    // ִ��������ǿת��redisReply*����
	if (NULL == reply)
	{
		printf("Execut command1 failure\n");
		redisFree(redis);     // ����ִ��ʧ�ܣ��ͷ��ڴ�
		return;
	}
	//if (!(reply->type == REDIS_REPLY_STATUS && strcasecmp(reply->str, "OK") == 0))
	//{       // �ж�����ִ�еķ���ֵ
	//	printf("Failed to execute command[%s]\n", command1);
	//	freeReplyObject(reply);
	//	redisFree(redis);
	//	return;
	//}
	freeReplyObject(reply);
	printf("Succeed to execute command[%s]\n", command1);
	// һ����������Է���ֵ���д���
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

//����������
/*
int main()
{
	LOG_INFO << "main pid:" << getpid();
	int maintid = syscall(SYS_gettid);//��ȡmain�̵߳��߳�ID
	LOG_INFO << "main tid:" << maintid;
	muduo::net::EventLoop loop;
	muduo::net::InetAddress listenAddr(9999);
	ChatServer server(&loop, listenAddr);
	server.start();
	loop.loop();
	
    return 0;
}
*/