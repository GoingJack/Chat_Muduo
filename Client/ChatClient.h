#pragma once

#include <muduo/net/TcpClient.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>
#include <boost/bind.hpp>
#include <muduo/base/ThreadPool.h>
#include "json.hpp"
#include <semaphore.h>
#include <vector>


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

		//���õ�¼�ȴ���������Ӧ��Ĭ���ź����ĳ�ֵΪ0
		sem_init(&_semFriendList, false, 0);

		//����ע���ȴ���������Ӧ��Ĭ���ź����ĳ�ֵΪ0
		sem_init(&_semLogout, false, 0);

		//���������û��ȴ���������Ӧ��Ĭ���ź����ĳ�ֵΪ0
		sem_init(&_semOnLineFriend, false, 0);

		//��������ȴ���������Ӧ��Ĭ���ź����ĳ�ֵΪ0
		sem_init(&_semChat, false, 0);

		//������Ӻ��ѵȴ���������Ӧ��Ĭ���ź����ĳ�ֵΪ0
		sem_init(&_semAddFriend, false, 0);

		//������ʾ��������ȴ���������Ӧ��Ĭ���ź����ĳ�ֵΪ0
		sem_init(&_semShowAllRequest, false, 0);

		//��ʼ��ͬ����߾ܾ���������ʹ�õ��ź���Ϊ0
		sem_init(&_semAckAddFriend, false, 0);
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

	//���� - ��¼�ɹ�ʱ���ر��û���id
	int userID;

	//ע��ҵ����������صĽ������
	void dealServerRes(json &js, const muduo::net::TcpConnectionPtr &con);

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	void showUserUI();

	void showLoginSuccessFun(json &js, const muduo::net::TcpConnectionPtr &con);

	void showAllfriend(const muduo::net::TcpConnectionPtr &con);

	void chatwithonefriend(const muduo::net::TcpConnectionPtr &con);

	//�ȴ����������غ����б��ź���
	sem_t _semFriendList;
	
	//�洢�����б��id-name   MAPӳ��
	std::map<int,muduo::string> _myfriendMap;

	//ֻ��ʾ��ǰ���ߵĺ���
	void showOnlineFriend(const muduo::net::TcpConnectionPtr &con);

	//�ȴ����������û��б���ź���
	sem_t _semOnLineFriend;


	//
	void TestClientWith(const muduo::net::TcpConnectionPtr &con);
	sem_t _semChat;

	//------------------------��¼�ɹ�-----------------------------
	//��ӡ��¼�ɹ���ʾ�Ĳ˵�
	void showLoginSuccessMenu();

	//��¼�ɹ�����ת������
	void dealLoginSuccessMain();

	//ע����ǰ�û�
	void logout(const muduo::net::TcpConnectionPtr &con);
	//ע��ʹ�õ��ź���
	sem_t _semLogout;

	//��Ӻ���
	void addfriend(const muduo::net::TcpConnectionPtr &con);
	//��Ӻ��ѵȴ���������Ӧ���ź���
	sem_t _semAddFriend;

	//�ö��Ա��������Ҫ�õ����������͹�����js��ʽ�Ķ�������������ֱ�Ӷ���һ��json���󱣴�(OnMessage����)ÿ�η��������͹�������
	json _myJSON;

	//��ʾ���е�Request����Ŀǰָ���Ǻ�������
	void showAllRequest(const muduo::net::TcpConnectionPtr &con);
	//�������еĺ��������б�֮����ź���
	sem_t _semShowAllRequest;

	//ͬ����߾ܾ���������ʹ�õ��ź���
	sem_t _semAckAddFriend;
};