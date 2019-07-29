#include <iostream>
#include <string>
#include <map>
#include <functional>
#include "ChatClient.h"
#include "public.h"

#include <map>

//----------------ȫ�ֱ���------------------------------
volatile bool isLoginSuccess = false;
volatile bool isResSuccess = false;
volatile bool isReqFriendList = false;

volatile bool isChatSend = false;

volatile bool isFriendIdExist = false;
volatile bool isSendAddFriend = false;

//��������Ӧ���еĺ��������Ƿ�����ɹ�
volatile bool isShowAllRequest = false;


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

	_myJSON = js;
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
		std::cout << js["username"] << ":" << js["chatmsg"] << std::endl;
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
	else if (js["msgid"] == MSG_ADD_FRIEND_EXIST_ACK)//��Ӻ��ѷ�����ȷ����Ϣ
	{
		if (js["code"] == ACK_SUCCESS)//�ж��Ƿ�ɹ����ͺ�������
		{
			isFriendIdExist = true;
		}
		else
		{
			isFriendIdExist = false;
		}
		sem_post(&_semAddFriend);
	}
	else if (js["msgid"] == MSG_ADD_FRIEND_ACK)
	{
		if (js["code"] == ACK_SUCCESS)//�ж��Ƿ�ɹ����ͺ�������
		{
			isSendAddFriend = true;
		}
		else
		{
			isSendAddFriend = false;
		}
		sem_post(&_semAddFriend);
	}
	else if (js["msgid"] == MSG_SHOW_ALL_REQUEST_ACK)
	{
		if (js["code"] == ACK_SUCCESS)
		{
			isShowAllRequest = true;
		}
		else
		{
			isShowAllRequest = false;
		}
		sem_post(&_semShowAllRequest);
	}
	else if (js["msgid"] == MSG_ACK_ADD_FRIEND_ACK)
	{
		sem_post(&_semAckAddFriend);
	}
	else if (js["msgid"] == MSG_CHAT_GROUP_ACK)
	{
		sem_post(&_semGroupChat);
	}
	else if (js["msgid"] == MSG_CHAT_GROUP_RECV)
	{
		std::cout << js["groupname"] << "->" << js["username"] << ":" << js["content"] << std::endl;
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
	_myUsername = username;
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
	actionMap.insert({ 3,bind(&ChatClient::showAllRequest,this,std::placeholders::_1) });
	actionMap.insert({ 4,bind(&ChatClient::chatGroup,this,std::placeholders::_1) });
	
	actionMap.insert({ 6,bind(&ChatClient::addfriend,this,std::placeholders::_1) });
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

//** ����ʾ���ߺ����б�Ļ�����   ��ָ�����ѽ�������
void ChatClient::chatwithonefriend(const muduo::net::TcpConnectionPtr &con)
{
	TestClientWith(con);
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
	chatwithonefriend(con);
}

//��Ӻ�������
void ChatClient::addfriend(const muduo::net::TcpConnectionPtr &con)
{
	//#1���ȸ����û������ID�����ݿ��в�ѯ�Ƿ��д��û�����
	json js;
	js["msgid"] = MSG_ADD_FRIEND_EXIST;
	js["id"] = userID;//userID will add friend
	int friendid = 0;
	
	std::cout << "please input id that you will add:";
	std::cin >> friendid;
	js["friendid"] = friendid;
	con->send(js.dump());
	sem_wait(&_semAddFriend);
	if (isFriendIdExist)//��ѯ���û��������ID
	{
		//��ӡ�������û�����Ϣ
		std::cout << "id:" << js["friendid"] << "<->" << "username:" << _myJSON["friendname"] << std::endl;
		//��ȡ�û�����֤��Ϣ
		std::string verifymsg;
		std::cout << "please input your verifymsg to your friend:";
		//std::cin >> verifymsg;
		char tmpstr[1024];
		getchar();//�Ե��س�
		fgets(tmpstr, 1023, stdin);
		tmpstr[strlen(tmpstr) - 1] = 0;
		verifymsg = tmpstr;

		js["verifymsg"] = verifymsg;
		js["msgid"] = MSG_ADD_FRIEND;
		con->send(js.dump());
		sem_wait(&_semAddFriend);
		if (isSendAddFriend)
		{
			std::cout << "send to " << js["username"] << "success!" << std::endl;
		}
		else
		{
			std::cout << "some error occured on server!please try again!" << std::endl;
		}
	}
	else//���ݿ��в����ڴ��û�
	{
		std::cout << "can not find user whose id is " << friendid << std::endl;
		char c;
		while (1)
		{
			std::cout << "please input y/n,y->continue search n->back to last menu";
			
			std::cin >> c;

			if (c == 'y' || c == 'Y' || c == 'n' || c == 'N')
			{
				break;
			}
			else
			{
				std::cout << "invaild input ! try again!" << std::endl;
			}
		}
		if (c == 'y' || c == 'Y')
		{
			addfriend(con);
		}
	}
}

//��ʾ���е�Request����Ŀǰָ���Ǻ�������
void ChatClient::showAllRequest(const muduo::net::TcpConnectionPtr &con)
{
	json js;
	js["msgid"] = MSG_SHOW_ALL_REQUEST;
	js["id"] = userID;
	con->send(js.dump());
	sem_wait(&_semShowAllRequest);
	if (isShowAllRequest)
	{
		std::map<int, muduo::string> requestList = _myJSON["List"];

		auto it = requestList.begin();

		std::cout << "userid" << " " << "msg" << std::endl;
		for (; it != requestList.end(); ++it)
		{
			std::cout << it->first << " " << it->second << std::endl;
		}
		muduo::string op;
		while (true)
		{
			int id;
			std::cout << "please input your op(y->agree,n->refused,end->back):";
			std::cin >> op;
			if (op == "end")
			{
				break;
			}
			else if (op == "y")
			{
				while (true)
				{
					std::cout << "please input your agree's id:";
					std::cin >> id;
					auto _finder = requestList.find(id);
					if (_finder != requestList.end())
					{
						json sendToServer;
						sendToServer["msgid"] = MSG_ACK_ADD_FRIEND;
						sendToServer["type"] = 'y';
						sendToServer["friendid"] = id;
						sendToServer["userid"] = userID;
						con->send(sendToServer.dump());
						sem_wait(&_semAckAddFriend);
						std::cout << "operator success!" << std::endl;
						break;
					}

					std::cout << "your input id is not in request list,please try again.";
				}
			}
			else
			{

			}
		}
	}
	else
	{

	}
}





//----------------ֱ������-----------------------------

void ChatClient::TestClientWith(const muduo::net::TcpConnectionPtr &con)
{
	int id = 0;
	std::cout << "input your friend id,while you input \"end\" to end this session." << std::endl;
	std::cin >> id;
	auto _myFinder = _myfriendMap.find(id);
	if (_myFinder == _myfriendMap.end())
	{
		std::cout << "your input id is not your friend or your friend is OFFLINE!";
		return;
	}
	while (1)
	{
		//std::cout << "input your content:";
		std::string content;
		std::cin >> content;
		if (content == "end")
		{
			break;
		}
		json js;
		js["msgid"] = MSG_ONE_CHAT;
		js["chatmsg"] = content;
		js["id"] = id;//Ҫ���͸����ѵ�ID
		js["fromid"] = userID;//�Լ���ID
		con->send(js.dump());
		sem_wait(&_semChat);
		if (!isChatSend)
		{
			std::cout << "send failed!" << std::endl;
		}
	}
}


//��û�����ߵĺ�������



//����Ⱥ��
void createGroup(const muduo::net::TcpConnectionPtr &con)
{

}

//��ʾ�����Ѽ����Ⱥ��
void showAllGroup(const muduo::net::TcpConnectionPtr &con)
{

}

//Ⱥ������
void ChatClient::chatGroup(const muduo::net::TcpConnectionPtr &con)
{
	json back;
	back["msgid"] = MSG_CHAT_GROUP;
	int gid = 0;
	std::cout << "please input gid that you want to chat with";
	std::cin >> gid;
	back["gid"] = gid;
	back["username"] = _myUsername;
	back["id"] = userID;
	muduo::string content;
	while (true)
	{
		std::cin >> content;
		if (content == "end")
		{
			break;
		}
		back["content"] = content;
		con->send(back.dump());
		sem_wait(&_semGroupChat);
	}
}