#pragma once

#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <boost/bind.hpp>
#include <muduo/base/ThreadPool.h>
#include "json.hpp"
#include <semaphore.h>


using json = nlohmann::json;


/*
�ͻ���ʵ�֣�����C++ muduo�����
*/
class ChatClient
{
public:
	ChatClient(muduo::net::EventLoop *loop,
		const muduo::net::InetAddress &addr)
		:_client(loop, addr, "ChatClient")
	{
		// ���ÿͻ���TCP���ӻص��ӿ�
		_client.setConnectionCallback(bind(&ChatClient::onConnection,
			this, _1));

		// ���ÿͻ��˽������ݻص��ӿ�
		_client.setMessageCallback(bind(&ChatClient::onMessage,
			this, _1, _2, _3));

		//���õ�¼�ȴ���������Ӧ��Ĭ���ź����ĳ�ֵΪ0
		sem_init(&_semLogin, false, 0);

		//���õ�¼�ȴ���������Ӧ��Ĭ���ź����ĳ�ֵΪ0
		sem_init(&_semRes, false, 0);
	}
	// ���ӷ�����
	void connect()
	{
		_client.connect();
	}
private:
	// TcpClient�󶨻ص������������ӻ��߶Ͽ�������ʱ����
	void onConnection(const muduo::net::TcpConnectionPtr &con);
	// TcpClient�󶨻ص��������������ݽ���ʱ����
	void onMessage(const muduo::net::TcpConnectionPtr &con,
		muduo::net::Buffer *buf,
		muduo::Timestamp time);
	// �ͻ���������棬�ڵ������߳��н����û�������з��Ͳ���
	void userClient(const muduo::net::TcpConnectionPtr &con);
	muduo::net::TcpClient _client;
	muduo::ThreadPool _pool;

	//��¼�ȴ���������Ӧ���ź���
	sem_t _semLogin;

	//ע��ȴ���������Ӧ���ź���
	sem_t _semRes;

	//�����ͻ��˵Ĳ˵�����
	void showOption();

	//��¼ҵ����
	void dealLogin(const muduo::net::TcpConnectionPtr &con);

	//ע��ҵ����
	void dealRes(const muduo::net::TcpConnectionPtr &con);

	//��¼ҵ����������صĽ������
	void dealServerLogin(json &js, const muduo::net::TcpConnectionPtr &con);

	//ע��ҵ����������صĽ������
	void dealServerRes(json &js, const muduo::net::TcpConnectionPtr &con);

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	void showLoginSuccessFun(json &js);
};