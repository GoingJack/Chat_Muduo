#include <iostream>
#include <string>
#include <map>
#include <functional>
#include "ChatClient.h"
#include "public.h"

//----------------ȫ�ֱ���------------------------------
volatile bool isLoginSuccess = false;
volatile bool isResSuccess = false;
volatile bool isReqFriendList = false;

volatile bool isChatSend = false;


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
	else if (js["msgid"] == MSG_REQUEST_FRIENDLIST_ACK)//��������б�
	{
		sem_post(&_semFriendList);
		if (js["code"] == ACK_SUCCESS)
		{
			isReqFriendList = true;
			//��������б���Ϣ
			std::map<int, muduo::string> tmp = js["friendlist"];
			_myfriendMap = tmp;
		}
		else
		{
			isReqFriendList = false;
		}
	}
	else if (js["msgid"] == MSG_ONE_CHAT_ACK)//����ȷ��
	{
		if (js["code"] == ACK_SUCCESS)
		{
			isChatSend = true;
		}
		else
		{
			isChatSend = false;
		}
		sem_post(&_semChat);
	}
	else if (js["msgid"] == MSG_ONE_CHAT)//���µ�������Ϣ�����˹���
	{
		std::cout << js["id"] << ":" << js["chatmsg"] << std::endl;
	}
	else if (js["msgid"] == MSG_LOGOUT_ACK)//�˳���¼�ķ�����ȷ�ϲ���
	{
		sem_post(&_semLogout);
	}
	else if (js["msgid"] == MSG_REQUEST_ONLINE_FRIEND_ACK)//�������ߺ����б���Ϣ
	{
		_myfriendMap.clear();
		std::map<int, muduo::string> _tmp = js["onlinefriendlist"];
		_myfriendMap = _tmp;
		sem_post(&_semOnLineFriend);//Ĭ�ϳɹ��õ�Mapӳ��
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
		fflush(stdout);
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
	std::cout << "please input your username: ";
	std::cin >> username;
	js["username"] = username;
	//#1.3����
	std::cout << "please input your password: ";
	std::cin >> password;
	js["userpwd"] = password;
	//#2��װ��Ϸ��͸�������
	con->send(js.dump());
	std::cout << "now is waiting server return...";

	//#3��¼�ύ���ݵȴ���������Ӧ���ź���_semLogin����P����
	sem_wait(&_semLogin);

	//#4�����¼�ɹ�����������棬���ʧ�ܽ��Ž���userClient����
	if (isLoginSuccess)
	{
		//�����¼�ɹ���ҳ��
		showLoginSuccessFun(js, con);
		/*_pool.start(1);
		_pool.run(bind(&ChatClient::TestClientWith, this, con));*/

	}
	else
	{
		//��½ʧ�ܼ�����ʼ������
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
		userID = js["id"];
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


//------------------------��¼�ɹ�-----------------------------

//��ʾ����������
void ChatClient::showUserUI()
{
	std::cout << "------------------------------" << std::endl;
	std::cout << "1,show online friend." << std::endl;
	std::cout << "2,show all	friend." << std::endl;
	std::cout << "3,show all	request." << std::endl;
	std::cout << "4,show all	group" << std::endl;
	std::cout << "5,show all	message" << std::endl;
	std::cout << "6,add  friend/group" << std::endl;
	std::cout << "7,logout" << std::endl;
	std::cout << "------------------------------" << std::endl;
}

//�����ɷ�����
void ChatClient::showLoginSuccessFun(json &js, const muduo::net::TcpConnectionPtr &con)
{
	std::map<int, std::function<void(const muduo::net::TcpConnectionPtr &)>> actionMap;
	actionMap.insert({ 1,bind(&ChatClient::showOnlineFriend,this,std::placeholders::_1) });
	actionMap.insert({ 2,bind(&ChatClient::showAllfriend,this,std::placeholders::_1) });

	actionMap.insert({ 7,bind(&ChatClient::logout,this,std::placeholders::_1) });
	int choice = 0;
	for (;;)
	{
		showUserUI();
		std::cin >> choice;
		auto it = actionMap.find(choice);
		if (it == actionMap.end())
		{
			LOG_INFO << "input number invaild,please try again!";
			fflush(stdin);
		}
		else
		{
			it->second(con);
		}
	}
}

//��ʾ���еĺ���
void ChatClient::showAllfriend(const muduo::net::TcpConnectionPtr &con)
{
	//#1 ��װ��������б�json ������
	json sendJs;
	sendJs["msgid"] = MSG_REQUEST_FRIENDLIST;

	//��ǰ�û���UserID
	sendJs["id"] = userID;
	con->send(sendJs.dump());

	sem_wait(&_semFriendList);
	//#2 ����������֮��õ������б�����ʾ�����б�
	if (isReqFriendList)
	{
		std::cout << "--------all friend list---------------" << std::endl;
		std::cout << "id	" << "username	" << std::endl;
		auto it = _myfriendMap.begin();
		while (it != _myfriendMap.end())
		{
			std::cout << it->first << "\t" << it->second << std::endl;
			++it;
		}
		std::cout << "--------------------------------------" << std::endl;
	}
	else
	{
		std::cout << "request friendilist failed" << std::endl;
	}

}

//��ָ�����ѽ�������
void ChatClient::chatwithonefriend(const muduo::net::TcpConnectionPtr &con)
{

}

//ע����ǰ�û�
void ChatClient::logout(const muduo::net::TcpConnectionPtr &con)
{
	json js;
	js["msgid"] = MSG_LOGOUT;
	js["id"] = userID;
	con->send(js.dump());
	sem_wait(&_semLogout);
	//��������жϷ������Ƿ��޸ģ�ʱ��ԭ��֮����

	//�ص�������
	userClient(con);
}

//ֻ��ʾ��ǰ���ߵĺ���
void ChatClient::showOnlineFriend(const muduo::net::TcpConnectionPtr &con)
{
	//#1��װjson�ַ������������������Ϣ
	json js;
	js["msgid"] = MSG_REQUEST_ONLINE_FRIEND;
	js["id"] = userID;
	con->send(js.dump());
	sem_wait(&_semOnLineFriend);

	//#2���ܷ���������Ӧ
	std::cout << "---------online friend list--------------" << std::endl;
	std::cout << "id	" << "username	" << std::endl;
	auto it = _myfriendMap.begin();
	while (it != _myfriendMap.end())
	{
		std::cout << it->first << "\t" << it->second << std::endl;
		++it;
	}
	std::cout << "-----------------------------------------" << std::endl;
}









//----------------ֱ������-----------------------------

void ChatClient::TestClientWith(const muduo::net::TcpConnectionPtr &con)
{
	int id = 0;
	std::cout << "input your friend id" << std::endl;
	std::cin >> id;
	while (1)
	{
		//std::cout << "input your content:";
		std::string content;
		std::cin >> content;
		json js;
		js["msgid"] = MSG_ONE_CHAT;
		js["chatmsg"] = content;
		js["id"] = id;
		js["fromid"] = userID;
		con->send(js.dump());
		sem_wait(&_semChat);
		if (!isChatSend)
		{
			std::cout << "send failed" << std::endl;
		}
	}
}