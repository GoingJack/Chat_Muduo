#pragma once

#include "CommonServiceBase.h"
#include "UserModelBase.h"
#include "FriendModelBase.h"
#include "GroupModelBase.h"

#include "public.h"
#include <unordered_map>
// 单机服务器业务实现
class SingleService : public CommonServiceBase
{
public:
	static SingleService& getInstance()
	{
		static SingleService instance;
		return instance;
	}
	SingleService(const SingleService&) = delete;
	SingleService& operator=(const SingleService&) = delete;

	// login service
	virtual void login(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time)
	{
		//给userid的一个默认值
		
		string name = js["username"];
		string pwd = js["userpwd"];

		UserDO user;
		
		user.setName(name);
		user.setPassword(pwd);

		if (userModelPtr->login(user)) // 登录成功
		{
			LOG_INFO << "one client is login success!name->" << name << " pwd->"
				<< pwd;

			json js;
			js["msgid"] = MSG_LOGIN_ACK;
			js["code"] = ACK_SUCCESS;
			// 登录成功，把用户的id返回
			js["id"] = user.getID();
			// 将登录成功的id和con关联起来，用以处理客户端异常断开情况
			//_connMap.insert(user.getID(), con);
			_connMap[user.getID()] = con;
			//将封装好的js字符串发送
			con->send(js.dump());
		}
		else // 登录失败
		{
			LOG_INFO << "login service  error!!!name->" << name << " pwd->"
				<< pwd  ;

			json js;
			js["msgid"] = MSG_LOGIN_ACK;
			js["code"] = ACK_ERROR;
			con->send(js.dump());
		}
	}

	// register service
	virtual void reg(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time)
	{
		string name = js["username"].get<string>();
		string pwd = js["userpwd"].get<string>();
		
		UserDO user;
		user.setID(-1);
		user.setName(name);
		user.setPassword(pwd);

		if (userModelPtr->add(user)) // 注册插入数据成功
		{
			LOG_INFO << "reg service success!name->" << name << " pwd->"
				<< pwd ;

			json js;
			js["msgid"] = MSG_REG_ACK;
			js["code"] = ACK_SUCCESS;
			con->send(js.dump());
		}
		else // 注册插入数据失败
		{
			LOG_INFO << "reg service error!name->" << name << " pwd->"
				<< pwd ;

			json js;
			js["msgid"] = MSG_REG_ACK;
			js["code"] = ACK_ERROR;
			con->send(js.dump());
		}
	}

	// add friend service
	virtual void addFriend(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time)
	{
	
	}

	// add group service
	virtual void addGroup(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time)
	{

	}

	// one to one chat service
	virtual void oneChat(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time)
	{
		int id = js["id"];//toid
		json jstmp;
		jstmp["msgid"] = MSG_ONE_CHAT_ACK;
		jstmp["code"] = ACK_SUCCESS;
		con->send(jstmp.dump());

		auto it = _connMap.find(js["id"]);

		json msg;
		msg["msgid"] = MSG_ONE_CHAT;
		msg["id"] = js["fromid"];
		msg["chatmsg"] = js["chatmsg"];
		it->second->send(msg.dump());
	}

	// group chat service
	virtual void groupChat(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time)
	{

	}

	// update  user state when client exit
	void exitChlient(const muduo::net::TcpConnectionPtr &con)
	{
		auto it = _connMap.begin();
		//从con和id的key-value中找断开连接请求的userid更新其数据库中的state值为OFFLINE
		for (; it != _connMap.end(); ++it)
		{
			if (it->second == con)
			{
				UserDO user;
				user.setID(it->first);
				userModelPtr->exit(user);
				break;
			}
		}
	}

	// get friend list
	virtual void getFriendlist(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time)
	{
		
		std::map<int, muduo::string> res;

		int id = js["id"];

		json backjs;
		if (userModelPtr->friendlist(id, res))
		{
			backjs["friendlist"] = res;
			backjs["code"] = ACK_SUCCESS;
			backjs["msgid"] = MSG_REQUEST_FRIENDLIST_ACK;
			con->send(backjs.dump());
		}
		else
		{
			backjs["msgid"] = MSG_REQUEST_FRIENDLIST_ACK;
			backjs["code"] = ACK_ERROR;
			con->send(backjs.dump());
		}
	}
	//logout update database
	virtual void logout(const muduo::net::TcpConnectionPtr &con,
		json &js, muduo::Timestamp time)
	{
		int id = js["id"];
		json back;
		back["msgid"] = MSG_LOGOUT_ACK;
		if (userModelPtr->logout(id))
		{
			back["code"] = ACK_SUCCESS;
		}
		else
		{
			back["code"] = ACK_ERROR;
		}
		con->send(back.dump());
	}

private:
	unique_ptr<UserModelBase> userModelPtr;
	unique_ptr<FriendModelBase> friendModelPtr;
	unique_ptr<GroupModelBase> groupModelPtr;

	std::unordered_map<int, muduo::net::TcpConnectionPtr> _connMap;//连接和用户ID关联起来



	SingleService()
		: userModelPtr(new UserModel())
		, friendModelPtr(new FriendModel())
		, groupModelPtr(new GroupModel())
	{}
};

// 全局接口，返回SingleService服务的唯一实例
static SingleService& App()
{
	return SingleService::getInstance();
}