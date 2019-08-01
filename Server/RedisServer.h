#pragma once

#include <hiredis/hiredis.h>
#include <string>
#include <muduo/base/Logging.h>
#include <functional>
#include <hiredis/async.h>
// ����redis server�������ַ��Ϣ
static std::string redisHost = "127.0.0.1";
static unsigned short port = 6379;

class RedisServer
{
public:
	// ��ʼ������redis server��context�����Ļ���
	RedisServer() 
		:_context(nullptr) 
		,_contextPublish(nullptr)
	{}
	// �ͷź�redis server�����õ�context�����Ļ���
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
	// ����redis server
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
	// ����ͨ��
	void subscribe(std::string channel)
	{
		// ֻ�������������������redis server��Ӧ��Ϣ�������notifyMsg�߳���ռ��Ӧ��Դ
		if (REDIS_ERR == redisAppendCommand(this->_context, "SUBSCRIBE %s", channel.c_str()))
		{
			LOG_ERROR << "subscribe [" << channel << "] error!";
			return;
		}
		// redisBufferWrite����ѭ�����ͻ�������ֱ�����������ݷ�����ϣ�done����Ϊ1��
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
	// ȡ�����ĵ�ͨ��
	void unsubscribe(std::string channel)
	{
		// ֻ�������������������redis server��Ӧ��Ϣ�������notifyMsg�߳���ռ��Ӧ��Դ
		if (REDIS_ERR == redisAppendCommand(this->_context, "UNSUBSCRIBE %s", channel))
		{
			LOG_ERROR << "subscribe [" << channel << "] error!";
			return;
		}
		// redisBufferWrite����ѭ�����ͻ�������ֱ�����������ݷ�����ϣ�done����Ϊ1��
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
	// ���ͨ����Ϣ��������ڵ������߳��д�����ͨ��û����Ϣ��redisGetReply�������߳�
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
	//������Ϣ
	void publishMsg(const int id,muduo::string content)
	{
		// ֻ�������������������redis server��Ӧ��Ϣ�������notifyMsg�߳���ռ��Ӧ��Դ
		//if (REDIS_ERR == redisAppendCommand(this->_contextPublish, "PUBLISH %d %s", id, content.c_str()))
		//{
		//	LOG_ERROR << "publish [" << id << ":" << content << "] error!";
		//	return;
		//}
		//// redisBufferWrite����ѭ�����ͻ�������ֱ�����������ݷ�����ϣ�done����Ϊ1��
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
	
	// ����ͨ����Ϣ�ϱ��Ļص���������RedisServer����ж��ĵ���Ϣ��������ͨ��Ԥ�����õ�handler�ϱ������
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