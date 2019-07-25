#include <iostream>
#include <string>
#include <map>
#include <functional>
#include "ChatClient.h"
#include "public.h"

//----------------全局变量------------------------------
volatile bool isLoginSuccess = false;
volatile bool isResSuccess = false;


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
	LOG_INFO << "please input your username";
	std::cin >> username;
	js["username"] = username;
	//#1.3密码
	LOG_INFO << "please input your password";
	std::cin >> password;
	js["userpwd"] = password;
	//#2封装完毕发送给服务器
	con->send(js.dump());
	LOG_INFO << "now is waiting server return...";

	//#3登录提交数据等待服务器响应对信号量_semLogin进行P操作
	sem_wait(&_semLogin);

	//#4如果登录成功进入聊天界面，如果失败接着进入userClient函数
	if (isLoginSuccess)
	{
		std::cout << "chat UI" << std::endl;
	}
	else
	{
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
	std::cout << "1,选择好友聊天" << std::endl;
	std::cout << "2,添加好友" << std::endl;
	std::cout << "3,好友请求列表" << std::endl;
	std::cout << "4," << std::endl;
	std::cout << "------------------------------" << std::endl;
}

