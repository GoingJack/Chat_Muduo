#pragma once

#include <hiredis/hiredis.h>
#include <string>
#include <muduo/base/Logging.h>
#include <functional>
#include <hiredis/async.h>
// 配置redis server的网络地址信息
static std::string redisHost = "127.0.0.1";
static unsigned short port = 6379;

class RedisServer
{
public:
	// 初始化连接redis server的context上下文环境
	RedisServer() 
		:_context(nullptr) 
		,_contextPublish(nullptr)
	{}
	// 释放和redis server连接用的context上下文环境
	~RedisServer()
	{
		if (this->_context != nullptr)
		{
			redisFree(this->_context);
			this->_context = nullptr;
		}
		if (this->_contextPublish != nullptr)
		{
			redisFree(this->_contextPublish);
			this->_contextPublish = nullptr;
		}
	}
	// 连接redis server
	bool connect()
	{
		this->_context = redisConnect(redisHost.c_str(), port);
		this->_contextPublish = redisConnect(redisHost.c_str(), port);
		if (this->_context == nullptr || this->_context->err)
		{
			LOG_ERROR << "redis server connect1 error! reason:"
				<< this->_context->errstr;
			return false;
		}
		//LOG_INFO << "redis server connect success!";

		if (this->_contextPublish == nullptr || this->_contextPublish->err)
		{
			LOG_ERROR << "redis server connect2 error! reason:"
				<< this->_contextPublish->errstr;
			return false;
		}
		LOG_INFO << "redis server connect success!";
		return true;
	}
	// 订阅通道
	void subscribe(std::string channel)
	{
		// 只负责发送命令，不阻塞接收redis server响应消息，否则和notifyMsg线程抢占响应资源
		if (REDIS_ERR == redisAppendCommand(this->_context, "SUBSCRIBE %s", channel.c_str()))
		{
			LOG_ERROR << "subscribe [" << channel << "] error!";
			return;
		}
		// redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
		int done = 0;
		while (!done)
		{
			if (REDIS_ERR == redisBufferWrite(this->_context, &done))
			{
				LOG_ERROR << "subscribe [" << channel << "] error!";
				return;
			}
		}
		LOG_INFO << "subscribe [" << channel << "] success!";
	}
	/*void subscribe(std::string channel)
	{
		redisReply *reply = (redisReply*)redisCommand(this->_context, "SUBSCRIBE %s", channel.c_str());
		if (reply == nullptr)
		{
			LOG_ERROR << "subscribe [" << channel << "] error!";
			return;
		}
		freeReplyObject(reply);
		LOG_INFO << "subscribe [" << channel << "] success!";
	}*/
	// 取消订阅的通道
	void unsubscribe(std::string channel)
	{
		// 只负责发送命令，不阻塞接收redis server响应消息，否则和notifyMsg线程抢占响应资源
		if (REDIS_ERR == redisAppendCommand(this->_context, "UNSUBSCRIBE %s", channel))
		{
			LOG_ERROR << "subscribe [" << channel << "] error!";
			return;
		}
		// redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
		int done = 0;
		while (!done)
		{
			if (REDIS_ERR == redisBufferWrite(this->_context, &done))
			{
				LOG_ERROR << "subscribe [" << channel << "] error!";
				return;
			}
		}
		LOG_INFO << "subscribe [" << channel << "] success!";
		/*redisReply *reply = (redisReply*)redisCommand(this->_context, "UNSUBSCRIBE %s", channel.c_str());
		if (reply == nullptr)
		{
			LOG_ERROR << "subscribe [" << channel << "] error!";
			return;
		}
		freeReplyObject(reply);
		LOG_INFO << "unsubscribe [" << channel << "] success!";*/
	}
	// 检测通道消息，必须放在单独的线程中处理，通道没有消息，redisGetReply会阻塞线程
	void notifyMsg()
	{
		redisReply *reply;
		while (redisGetReply(this->_context, (void**)&reply) == REDIS_OK)
		{
			LOG_INFO << "redis_ok";
			if (reply != nullptr && reply->element != nullptr)
			{
				if (reply->element[2] != nullptr
					 && reply->element[2]->str != nullptr)
				{
					LOG_INFO << reply->element[2]->str;
					_channelHandler(reply->element[1]->str,reply->element[2]->str);
				}
			}
			else
			{
				LOG_INFO << "redisGetReply no response!";
			}
		}

		freeReplyObject(reply);
	}
	//发布消息
	void publishMsg(const int id,muduo::string content)
	{
		// 只负责发送命令，不阻塞接收redis server响应消息，否则和notifyMsg线程抢占响应资源
		//if (REDIS_ERR == redisAppendCommand(this->_contextPublish, "PUBLISH %d %s", id, content.c_str()))
		//{
		//	LOG_ERROR << "publish [" << id << ":" << content << "] error!";
		//	return;
		//}
		//// redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
		//int done = 0;
		//while (!done)
		//{
		//	if (REDIS_ERR == redisBufferWrite(this->_contextPublish, &done))
		//	{
		//		LOG_ERROR << "publish [" << id << ":" << content << "] error!";
		//		return;
		//	}
		//}
		//LOG_INFO << "publish [" << id << ":" << content << "] success!"; 
		redisReply *reply = (redisReply*)redisCommand(this->_contextPublish, "PUBLISH %d %s", id, content.c_str());

		if (reply == nullptr)
		{
			LOG_ERROR << "publish [" << id << ":" << content << "] error!";
			return;
		}
		freeReplyObject(reply);
		LOG_INFO << "publish [" << id << ":" << content << "] success!";
	}
	
	// 设置通道消息上报的回调函数，当RedisServer检测有订阅的消息发生，则通过预先设置的handler上报服务层
	using channelHandler = std::function<void(std::string,std::string)>;
	void setChannelMsgHandler(channelHandler handler)
	{
		_channelHandler = handler;
	}
private:
	redisContext *_context;
	redisContext *_contextPublish;
	channelHandler _channelHandler;
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
