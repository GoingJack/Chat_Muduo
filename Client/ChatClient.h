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
客户端实现，基于C++ muduo网络库
*/
class ChatClient
{
public:
	ChatClient(muduo::net::EventLoop *loop,
		const muduo::net::InetAddress &addr)
		:_client(loop, addr, "ChatClient")
	{
		// 设置客户端TCP连接回调接口
		_client.setConnectionCallback(bind(&ChatClient::onConnection,
			this, _1));

		// 设置客户端接收数据回调接口
		_client.setMessageCallback(bind(&ChatClient::onMessage,
			this, _1, _2, _3));

		//设置登录等待服务器响应的默认信号量的初值为0
		sem_init(&_semLogin, false, 0);

		//设置登录等待服务器响应的默认信号量的初值为0
		sem_init(&_semRes, false, 0);

		//设置登录等待服务器响应的默认信号量的初值为0
		sem_init(&_semFriendList, false, 0);

		//设置注销等待服务器响应的默认信号量的初值为0
		sem_init(&_semLogout, false, 0);

		//设置在线用户等待服务器响应的默认信号量的初值为0
		sem_init(&_semOnLineFriend, false, 0);

		//设置聊天等待服务器响应的默认信号量的初值为0
		sem_init(&_semChat, false, 0);

		//设置添加好友等待服务器响应的默认信号量的初值为0
		sem_init(&_semAddFriend, false, 0);

		//设置显示所有请求等待服务器响应的默认信号量的初值为0
		sem_init(&_semShowAllRequest, false, 0);

		//初始化同意或者拒绝好友请求使用的信号量为0
		sem_init(&_semAckAddFriend, false, 0);
	}
	// 连接服务器
	void connect()
	{
		_client.connect();
	}
private:
	// TcpClient绑定回调函数，当连接或者断开服务器时调用
	void onConnection(const muduo::net::TcpConnectionPtr &con);
	// TcpClient绑定回调函数，当有数据接收时调用
	void onMessage(const muduo::net::TcpConnectionPtr &con,
		muduo::net::Buffer *buf,
		muduo::Timestamp time);
	// 客户端输入界面，在单独的线程中接收用户输入进行发送操作
	void userClient(const muduo::net::TcpConnectionPtr &con);
	muduo::net::TcpClient _client;
	muduo::ThreadPool _pool;

	//登录等待服务器响应的信号量
	sem_t _semLogin;

	//注册等待服务器响应的信号量
	sem_t _semRes;

	//启动客户端的菜单函数
	void showOption();

	//登录业务处理
	void dealLogin(const muduo::net::TcpConnectionPtr &con);

	//注册业务处理
	void dealRes(const muduo::net::TcpConnectionPtr &con);

	//登录业务服务器返回的结果处理
	void dealServerLogin(json &js, const muduo::net::TcpConnectionPtr &con);

	//保存 - 登录成功时返回本用户的id
	int userID;

	//注册业务服务器返回的结果处理
	void dealServerRes(json &js, const muduo::net::TcpConnectionPtr &con);

	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	void showUserUI();

	void showLoginSuccessFun(json &js, const muduo::net::TcpConnectionPtr &con);

	void showAllfriend(const muduo::net::TcpConnectionPtr &con);

	void chatwithonefriend(const muduo::net::TcpConnectionPtr &con);

	//等待服务器返回好友列表信号量
	sem_t _semFriendList;
	
	//存储好友列表的id-name   MAP映射
	std::map<int,muduo::string> _myfriendMap;

	//只显示当前在线的好友
	void showOnlineFriend(const muduo::net::TcpConnectionPtr &con);

	//等待返回在线用户列表的信号量
	sem_t _semOnLineFriend;


	//
	void TestClientWith(const muduo::net::TcpConnectionPtr &con);
	sem_t _semChat;

	//------------------------登录成功-----------------------------
	//打印登录成功显示的菜单
	void showLoginSuccessMenu();

	//登录成功后功能转发函数
	void dealLoginSuccessMain();

	//注销当前用户
	void logout(const muduo::net::TcpConnectionPtr &con);
	//注销使用的信号量
	sem_t _semLogout;

	//添加好友
	void addfriend(const muduo::net::TcpConnectionPtr &con);
	//添加好友等待服务器响应的信号量
	sem_t _semAddFriend;

	//好多成员函数都需要用到服务器发送过来的js格式的对象所以我这里直接定义一个json对象保存(OnMessage函数)每次服务器发送过来内容
	json _myJSON;

	//显示所有的Request这里目前指的是好友请求
	void showAllRequest(const muduo::net::TcpConnectionPtr &con);
	//请求所有的好友请求列表之后的信号量
	sem_t _semShowAllRequest;

	//同意或者拒绝好友请求使用的信号量
	sem_t _semAckAddFriend;
};