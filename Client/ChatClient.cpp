#include <iostream>
#include <string>
#include <map>
#include <functional>
#include "ChatClient.h"
#include "public.h"

#include <map>

//----------------全局变量------------------------------
volatile bool isLoginSuccess = false;
volatile bool isResSuccess = false;
volatile bool isReqFriendList = false;

volatile bool isChatSend = false;

volatile bool isFriendIdExist = false;
volatile bool isSendAddFriend = false;

//服务器响应所有的好友请求是否请求成功
volatile bool isShowAllRequest = false;


// TcpClient绑定回调函数，当连接或者断开服务器时调用
void ChatClient::onConnection(const muduo::net::TcpConnectionPtr &con)
{
	/*
	连接服务器成功后，开启和服务器的交互通信功能
	*/
	if (con->connected()) // 和服务器连接成功
	{
		LOG_INFO << "connect server success!";
		// 启动线程专门处理用户的输入操作
		_pool.start(1);
		_pool.run(bind(&ChatClient::userClient, this, con));
	}
	else // 和服务器连接失败
	{
		LOG_INFO << "connect server fail!";
	}
}

// TcpClient绑定回调函数，当有数据接收时调用
void ChatClient::onMessage(const muduo::net::TcpConnectionPtr &con,
	muduo::net::Buffer *buf,
	muduo::Timestamp time)
{
	/*
	负责接收服务器响应的消息，在客户端进行显示
	*/
	muduo::string msg(buf->retrieveAllAsString());
	json js = json::parse(msg.c_str());

	_myJSON = js;
	if (js["msgid"] == MSG_LOGIN_ACK)//处理服务器响应的登录请求
	{
		dealServerLogin(js,con);
		sem_post(&_semLogin);
	}
	else if (js["msgid"] == MSG_REG_ACK)
	{
		dealServerRes(js, con);
		sem_post(&_semRes);
	}
	else if (js["msgid"] == MSG_REQUEST_FRIENDLIST_ACK)//请求好友列表
	{
		sem_post(&_semFriendList);
		if (js["code"] == ACK_SUCCESS)
		{
			isReqFriendList = true;
			//保存好友列表信息
			std::map<int, muduo::string> tmp = js["friendlist"];
			_myfriendMap = tmp;
		}
		else
		{
			isReqFriendList = false;
		}
	}
	else if (js["msgid"] == MSG_ONE_CHAT_ACK)//聊天确认
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
	else if (js["msgid"] == MSG_ONE_CHAT)//有新的聊天消息发送了过来
	{
		std::cout << js["username"] << ":" << js["chatmsg"] << std::endl;
	}
	else if (js["msgid"] == MSG_LOGOUT_ACK)//退出登录的服务器确认操作
	{
		sem_post(&_semLogout);
	}
	else if (js["msgid"] == MSG_REQUEST_ONLINE_FRIEND_ACK)//请求在线好友列表信息
	{
		_myfriendMap.clear();
		std::map<int, muduo::string> _tmp = js["onlinefriendlist"];
		_myfriendMap = _tmp;
		sem_post(&_semOnLineFriend);//默认成功得到Map映射
	}
	else if (js["msgid"] == MSG_ADD_FRIEND_EXIST_ACK)//添加好友服务器确认消息
	{
		if (js["code"] == ACK_SUCCESS)//判断是否成功发送好友请求
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
		if (js["code"] == ACK_SUCCESS)//判断是否成功发送好友请求
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

// 处理用户的输入操作
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

//初次启动客户端显示的菜单
void ChatClient::showOption()
{
	using namespace std;
	cout << "------------------------" << endl;
	cout << "1,Login" << endl;
	cout << "2,Register" << endl;
	cout << "3,Exit" << endl;
	cout << "------------------------" << endl;
}

//处理客户端登录请求
void ChatClient::dealLogin(const muduo::net::TcpConnectionPtr &con)
{
	/*
	登录操作
	*/
	std::string username;
	std::string password;
	//#1封装json格式的字符串
	//#1.1消息类型
	json js;
	js["msgid"] = MSG_LOGIN;//
	//#1.2用户名
	std::cout << "please input your username: ";
	std::cin >> username;
	js["username"] = username;
	_myUsername = username;
	//#1.3密码
	std::cout << "please input your password: ";
	std::cin >> password;
	js["userpwd"] = password;
	//#2封装完毕发送给服务器
	con->send(js.dump());
	std::cout << "now is waiting server return...";

	//#3登录提交数据等待服务器响应对信号量_semLogin进行P操作
	sem_wait(&_semLogin);

	//#4如果登录成功进入聊天界面，如果失败接着进入userClient函数
	if (isLoginSuccess)
	{
		//进入登录成功的页面
		showLoginSuccessFun(js, con);
		/*_pool.start(1);
		_pool.run(bind(&ChatClient::TestClientWith, this, con));*/

	}
	else
	{
		//登陆失败继续初始化界面
		userClient(con);
	}
}

//处理客户端注册请求
void ChatClient::dealRes(const muduo::net::TcpConnectionPtr &con)
{
	/*
	注册操作
	*/
	std::string username;
	std::string password;
	//#1封装json格式的字符串
	//#1.1消息类型
	json js;
	js["msgid"] = MSG_REG;//
	//#1.2用户名
	LOG_INFO << "please input your username";
	std::cin >> username;
	js["username"] = username;
	//#1.3密码
	LOG_INFO << "please input your password";
	std::cin >> password;
	js["userpwd"] = password;
	//#2封装完毕发送给服务器
	con->send(js.dump());
	LOG_INFO << "now is waiting server return...\n";

	//#3登录提交数据等待服务器响应对信号量_semLogin进行P操作
	sem_wait(&_semRes);

	//#4无论注册成功或者失败都进行重新启动客户端的操作
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


//------------------------登录成功-----------------------------

//显示主功能区域
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

//功能派发函数
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

//显示所有的好友
void ChatClient::showAllfriend(const muduo::net::TcpConnectionPtr &con)
{
	//#1 封装请求好友列表json 并发送
	json sendJs;
	sendJs["msgid"] = MSG_REQUEST_FRIENDLIST;

	//当前用户的UserID
	sendJs["id"] = userID;
	con->send(sendJs.dump());

	sem_wait(&_semFriendList);
	//#2 服务器返回之后得到好友列表并且显示好友列表
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

//** 在显示在线好友列表的基础上   和指定好友进行聊天
void ChatClient::chatwithonefriend(const muduo::net::TcpConnectionPtr &con)
{
	TestClientWith(con);
}

//注销当前用户
void ChatClient::logout(const muduo::net::TcpConnectionPtr &con)
{
	json js;
	js["msgid"] = MSG_LOGOUT;
	js["id"] = userID;
	con->send(js.dump());
	sem_wait(&_semLogout);
	//这里可以判断服务器是否修改，时间原因之后工作

	//回到主界面
	userClient(con);
}

//只显示当前在线的好友
void ChatClient::showOnlineFriend(const muduo::net::TcpConnectionPtr &con)
{
	//#1封装json字符串，向服务器请求消息
	json js;
	js["msgid"] = MSG_REQUEST_ONLINE_FRIEND;
	js["id"] = userID;
	con->send(js.dump());
	sem_wait(&_semOnLineFriend);

	//#2接受服务器的响应
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

//添加好友请求
void ChatClient::addfriend(const muduo::net::TcpConnectionPtr &con)
{
	//#1首先根据用户输入的ID在数据库中查询是否有此用户存在
	json js;
	js["msgid"] = MSG_ADD_FRIEND_EXIST;
	js["id"] = userID;//userID will add friend
	int friendid = 0;
	
	std::cout << "please input id that you will add:";
	std::cin >> friendid;
	js["friendid"] = friendid;
	con->send(js.dump());
	sem_wait(&_semAddFriend);
	if (isFriendIdExist)//查询到用户所输入的ID
	{
		//打印搜索的用户的信息
		std::cout << "id:" << js["friendid"] << "<->" << "username:" << _myJSON["friendname"] << std::endl;
		//获取用户的验证消息
		std::string verifymsg;
		std::cout << "please input your verifymsg to your friend:";
		//std::cin >> verifymsg;
		char tmpstr[1024];
		getchar();//吃掉回车
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
	else//数据库中不存在此用户
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

//显示所有的Request这里目前指的是好友请求
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





//----------------直接聊天-----------------------------

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
		js["id"] = id;//要发送给好友的ID
		js["fromid"] = userID;//自己的ID
		con->send(js.dump());
		sem_wait(&_semChat);
		if (!isChatSend)
		{
			std::cout << "send failed!" << std::endl;
		}
	}
}


//向没有在线的好友留言



//创建群组
void createGroup(const muduo::net::TcpConnectionPtr &con)
{

}

//显示所有已加入的群组
void showAllGroup(const muduo::net::TcpConnectionPtr &con)
{

}

//群组聊天
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