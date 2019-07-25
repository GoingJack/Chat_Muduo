#include <iostream>
#include <string>
#include <map>
#include <functional>
#include "ChatClient.h"
#include "public.h"

//----------------ȫ�ֱ���------------------------------
volatile bool isLoginSuccess = false;
volatile bool isResSuccess = false;


// TcpClient�󶨻ص������������ӻ��߶Ͽ�������ʱ����
void ChatClient::onConnection(const muduo::net::TcpConnectionPtr &con)
{
	/*
	���ӷ������ɹ��󣬿����ͷ������Ľ���ͨ�Ź���
	*/
	if (con->connected()) // �ͷ��������ӳɹ�
	{
		LOG_INFO << "connect server success!";
		// �����߳�ר�Ŵ����û����������
		_pool.start(1);
		_pool.run(bind(&ChatClient::userClient, this, con));
	}
	else // �ͷ���������ʧ��
	{
		LOG_INFO << "connect server fail!";
	}
}

// TcpClient�󶨻ص��������������ݽ���ʱ����
void ChatClient::onMessage(const muduo::net::TcpConnectionPtr &con,
	muduo::net::Buffer *buf,
	muduo::Timestamp time)
{
	/*
	������շ�������Ӧ����Ϣ���ڿͻ��˽�����ʾ
	*/
	muduo::string msg(buf->retrieveAllAsString());
	json js = json::parse(msg.c_str());

	if (js["msgid"] == MSG_LOGIN_ACK)//�����������Ӧ�ĵ�¼����
	{
		dealServerLogin(js,con);
		sem_post(&_semLogin);
	}
	else if (js["msgid"] == MSG_REG_ACK)
	{
		dealServerRes(js, con);
		sem_post(&_semRes);
	}
	
}

// �����û����������
void ChatClient::userClient(const muduo::net::TcpConnectionPtr &con)
{
	std::map<int, std::function<void(const muduo::net::TcpConnectionPtr &)>> actionMap;
	actionMap.insert({ 1,bind(&ChatClient::dealLogin,this,std::placeholders::_1) });
	//actionMap.insert({ 2,ChatClient::DealRes });
	actionMap.insert({ 2,bind(&ChatClient::dealRes,this,std::placeholders::_1) });
	int choice = 0;
	for (;;)
	{
		showOption();
		std::cin >> choice;

		auto it = actionMap.find(choice);
		if (it == actionMap.end())
		{
			LOG_INFO << "input number invaild,please try again!";
		}
		else
		{
			it->second(con);
			break;
		}
	}
}

//���������ͻ�����ʾ�Ĳ˵�
void ChatClient::showOption()
{
	using namespace std;
	cout << "------------------------" << endl;
	cout << "1,Login" << endl;
	cout << "2,Register" << endl;
	cout << "3,Exit" << endl;
	cout << "------------------------" << endl;
}

//����ͻ��˵�¼����
void ChatClient::dealLogin(const muduo::net::TcpConnectionPtr &con)
{
	/*
	��¼����
	*/
	std::string username;
	std::string password;
	//#1��װjson��ʽ���ַ���
	//#1.1��Ϣ����
	json js;
	js["msgid"] = MSG_LOGIN;//
	//#1.2�û���
	LOG_INFO << "please input your username";
	std::cin >> username;
	js["username"] = username;
	//#1.3����
	LOG_INFO << "please input your password";
	std::cin >> password;
	js["userpwd"] = password;
	//#2��װ��Ϸ��͸�������
	con->send(js.dump());
	LOG_INFO << "now is waiting server return...";

	//#3��¼�ύ���ݵȴ���������Ӧ���ź���_semLogin����P����
	sem_wait(&_semLogin);

	//#4�����¼�ɹ�����������棬���ʧ�ܽ��Ž���userClient����
	if (isLoginSuccess)
	{
		std::cout << "chat UI" << std::endl;
	}
	else
	{
		userClient(con);
	}
}

//����ͻ���ע������
void ChatClient::dealRes(const muduo::net::TcpConnectionPtr &con)
{
	/*
	ע�����
	*/
	std::string username;
	std::string password;
	//#1��װjson��ʽ���ַ���
	//#1.1��Ϣ����
	json js;
	js["msgid"] = MSG_REG;//
	//#1.2�û���
	LOG_INFO << "please input your username";
	std::cin >> username;
	js["username"] = username;
	//#1.3����
	LOG_INFO << "please input your password";
	std::cin >> password;
	js["userpwd"] = password;
	//#2��װ��Ϸ��͸�������
	con->send(js.dump());
	LOG_INFO << "now is waiting server return...\n";

	//#3��¼�ύ���ݵȴ���������Ӧ���ź���_semLogin����P����
	sem_wait(&_semRes);

	//#4����ע��ɹ�����ʧ�ܶ��������������ͻ��˵Ĳ���
	userClient(con);
}

void ChatClient::dealServerLogin(json &js, const muduo::net::TcpConnectionPtr &con)
{
	if (js["code"] == ACK_SUCCESS)
	{
		LOG_INFO << "login success!";
		isLoginSuccess = true;
	}
	else
	{
		LOG_INFO << "your username or password is error!";
		isLoginSuccess = false;
	}
}

void ChatClient::dealServerRes(json &js, const muduo::net::TcpConnectionPtr &con)
{
	if (js["code"] == ACK_SUCCESS)
	{
		LOG_INFO << "register success!";
		//isResSuccess = true;
	}
	else
	{
		LOG_INFO << "register failed!";
		//isResSuccess = false;
	}
}

void ChatClient::showLoginSuccessFun(json &js)
{
	std::cout << "------------------------------" << std::endl;
	std::cout << "1,ѡ���������" << std::endl;
	std::cout << "2,��Ӻ���" << std::endl;
	std::cout << "3,���������б�" << std::endl;
	std::cout << "4," << std::endl;
	std::cout << "------------------------------" << std::endl;
}

