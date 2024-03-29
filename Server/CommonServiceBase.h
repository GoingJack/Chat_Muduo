#pragma once

#include <muduo/net/TcpServer.h>
#include <muduo/base/Logging.h>
#include <boost/bind.hpp>
#include <unordered_map>
#include <functional>
#include "public.h"
#include "json.hpp"
using json = nlohmann::json;

/*
通用的服务处理抽象基类，保留统一接口，可以同时扩展单机服务器和集群服务器的派生类实现
*/
class CommonServiceBase
{	
public:
	CommonServiceBase()
	{
		// 绑定用户事件处理接口函数
		_handlerMap.insert({ MSG_LOGIN, bind(&CommonServiceBase::login, this, _1, _2, _3) });
		_handlerMap.insert({ MSG_REG, bind(&CommonServiceBase::reg, this, _1, _2, _3) });
		_handlerMap.insert({ MSG_ONE_CHAT, bind(&CommonServiceBase::oneChat, this, _1, _2, _3) });
		_handlerMap.insert({ MSG_REQUEST_FRIENDLIST ,bind(&CommonServiceBase::getFriendlist,this,_1,_2,_3) });
		_handlerMap.insert({ MSG_LOGOUT,bind(&CommonServiceBase::logout,this,_1,_2,_3) });
		_handlerMap.insert({ MSG_REQUEST_ONLINE_FRIEND,bind(&CommonServiceBase::onlinefriendlist,this,_1,_2,_3) });
		_handlerMap.insert({ MSG_ADD_FRIEND_EXIST,bind(&CommonServiceBase::findUserWithId,this,_1,_2,_3) });
		_handlerMap.insert({ MSG_ADD_FRIEND,bind(&CommonServiceBase::commitAddFriendRequest,this,_1,_2,_3) });
		_handlerMap.insert({ MSG_SHOW_ALL_REQUEST,bind(&CommonServiceBase::requestList,this,_1,_2,_3) });
		_handlerMap.insert({ MSG_ACK_ADD_FRIEND,bind(&CommonServiceBase::ackAddFriend,this,_1,_2,_3) });
		_handlerMap.insert({ MSG_CHAT_GROUP,bind(&CommonServiceBase::chatWithGroup,this,_1,_2,_3) });
		// 继续给handler绑定更多的接口函数
	}

	// login service
	virtual void login(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;

	// register service
	virtual void reg(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;

	// add friend service
	virtual void addFriend(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;

	// add group service
	virtual void addGroup(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;

	// one to one chat service
	virtual void oneChat(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;

	// group chat service
	virtual void groupChat(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;

	//friend list
	virtual void getFriendlist(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;

	//logout
	virtual void logout(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;

	//request online friendlist
	virtual void onlinefriendlist(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;
	//find a user is exist on User
	virtual void findUserWithId(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;
	//insert into Request for add friend
	virtual void commitAddFriendRequest(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;

	//request request list
	virtual void requestList(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;
	//ack add friend
	virtual void ackAddFriend(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;

	//chat with group
	virtual void chatWithGroup(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time) = 0;

	using Handler = std::function<void(const muduo::net::TcpConnectionPtr&, json&, muduo::Timestamp)>;
	std::unordered_map<int, Handler> handler()const { return _handlerMap; }
protected:
	std::unordered_map<int, Handler> _handlerMap;
};