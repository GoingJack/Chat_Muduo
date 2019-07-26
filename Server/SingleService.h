#pragma once

#include "CommonServiceBase.h"
#include "UserModelBase.h"
#include "FriendModelBase.h"
#include "GroupModelBase.h"

#include "public.h"
#include <unordered_map>
// ����������ҵ��ʵ��
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
		//��userid��һ��Ĭ��ֵ
		
		string name = js["username"];
		string pwd = js["userpwd"];

		UserDO user;
		
		user.setName(name);
		user.setPassword(pwd);

		if (userModelPtr->login(user)) // ��¼�ɹ�
		{
			LOG_INFO << "one client is login success!name->" << name << " pwd->"
				<< pwd;

			json js;
			js["msgid"] = MSG_LOGIN_ACK;
			js["code"] = ACK_SUCCESS;
			// ��¼�ɹ������û���id����
			js["id"] = user.getID();
			// ����¼�ɹ���id��con�������������Դ���ͻ����쳣�Ͽ����
			//_connMap.insert(user.getID(), con);
			_connMap[user.getID()] = con;
			//����װ�õ�js�ַ�������
			con->send(js.dump());
		}
		else // ��¼ʧ��
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

		if (userModelPtr->add(user)) // ע��������ݳɹ�
		{
			LOG_INFO << "reg service success!name->" << name << " pwd->"
				<< pwd ;

			json js;
			js["msgid"] = MSG_REG_ACK;
			js["code"] = ACK_SUCCESS;
			con->send(js.dump());
		}
		else // ע���������ʧ��
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
		//��con��id��key-value���ҶϿ����������userid���������ݿ��е�stateֵΪOFFLINE
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

	std::unordered_map<int, muduo::net::TcpConnectionPtr> _connMap;//���Ӻ��û�ID��������



	SingleService()
		: userModelPtr(new UserModel())
		, friendModelPtr(new FriendModel())
		, groupModelPtr(new GroupModel())
	{}
};

// ȫ�ֽӿڣ�����SingleService�����Ψһʵ��
static SingleService& App()
{
	return SingleService::getInstance();
}