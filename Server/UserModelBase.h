#pragma once

#include "UserDO.h"
#include "MySQL.h"
#include <map>
#include <vector>


// Model层的抽象类
class UserModelBase
{
public:
	// 增加用户
	virtual bool add(UserDO &user) = 0;
	// 用户登录检查
	virtual bool login(UserDO &user) = 0;
	//用户的状态置为不在线
	virtual bool exit(UserDO &user) = 0;
	//从数据库中获取好友列表
	virtual bool friendlist(const int userid, std::map<int, muduo::string>&res) = 0;
	//从数据更新那些退出登录用户的登录状态
	virtual bool logout(const int userid) = 0;
	//从数据库中获取在线的好友列表
	virtual bool onlinefriendlist(const int userid, std::map<int, muduo::string>&res) = 0;
	//向数据库中查询好友
	virtual bool finduser(const int userid,muduo::string &username) = 0;
	//向Request表中添加相关的好友请求信息
	virtual bool addfriend(const int userid, const int friendid, muduo::string &verifymsg) = 0;

	//发送好友申请列表请求
	virtual bool requestList(const int userid, std::map<int, muduo::string>&_resmap) = 0;

	//接受好友请求
	virtual bool acceptRequest(const int userid, const int friendid)=0;

	//获取用户名字通过ID值
	virtual muduo::string getUsernameFromId(const int userid) = 0;
};

// User表的Model层操作
class UserModel : public UserModelBase
{
private:
	char sql[1024];
public:
	UserModel() { memset(sql, 0, 1024); }
	// 重写add接口方法，实现增加用户操作
	bool add(UserDO &user)
	{
		// 组织sql语句
		sprintf(sql, "insert into User(name,password,state) values('%s', '%s', '%s')",
			user.getName().c_str(),
			user.getPwd().c_str(),
			user.getState().c_str());

		MySQL mysql;
		if (mysql.connect())
		{
			if (mysql.update(sql))
			{
				LOG_INFO << "add User success => sql:" << sql;
				return true;
			}
		}
		LOG_INFO << "add User error => sql:" << sql;
		return false;
	}

	// 重写login接口方法，实现检查登录用户的用户名密码是否正确
	bool login(UserDO &user)
	{
		// 组织sql语句
		sprintf(sql, "select id from User where name='%s' and password='%s'",
			user.getName().c_str(),
			user.getPwd().c_str());

		MySQL mysql;
		if (mysql.connect())
		{
			// 注意MYSQL_RES指针永远不为空，不管是否查询到数据
			MYSQL_RES *res = mysql.query(sql);
			if (res != nullptr)
			{
				MYSQL_ROW row = mysql_fetch_row(res);
				// MYSQL_ROW是char**，通过row是否为nullptr，判断查询是否返回结果
				if (row != nullptr)
				{
					LOG_INFO << "login success => sql:" << sql;
					user.setID(atoi(row[0]));
					mysql_free_result(res);


					//设置登录的状态
					memset(sql, 0, 1024);
					sprintf(sql, "update User set state = 'ONLINE' where id = %d", user.getID());
					return mysql.update(sql);
				}
			}
		}
		LOG_INFO << "login error => sql:" << sql;
		return false;
	}

	// 重写exit接口方法，修改user表的状态
	bool exit(UserDO &user)
	{
		// 组织sql语句
		sprintf(sql, "update User set state = 'OFFLINE' where id = %d",
			user.getID());

		MySQL mysql;
		if (mysql.connect())
		{
			// 注意MYSQL_RES指针永远不为空，不管是否查询到数据
			if (mysql.update(sql))
			{
				LOG_INFO << "client offline exception update User success!" << user.getID();
				return true;
			}
			
		}
		LOG_INFO << "client offline exception update User error!" << user.getID();
		return false;
	}
	//从数据库中获取好友列表
	bool friendlist(const int userid,std::map<int,muduo::string>&_resmap)
	{
		// 组织sql语句
		sprintf(sql, "select  id,name from User where id in ( select friendid from Friend where userid = %d)",
			userid
			);

		MySQL mysql;
		if (mysql.connect())
		{
			// 注意MYSQL_RES指针永远不为空，不管是否查询到数据
			MYSQL_RES *res = mysql.query(sql);
			if (res != nullptr)
			{
				// MYSQL_ROW是char**，通过row是否为nullptr，判断查询是否返回结果
				MYSQL_ROW row;
				while (row = mysql_fetch_row(res))
				{
					_resmap[atoi(row[0])] = std::string(row[1]);
				}
				return true;
			}
			LOG_INFO << "function(get friend list) -> query in database";
			return false;
		}
		LOG_INFO << "function(get friend list) -> connected to database";
		return false;
	}
	bool logout(const int userid)
	{
		sprintf(sql, "update User set state = 'OFFLINE' where id = %d", userid);

		MySQL mysql;
		if (mysql.connect())
		{
			if (mysql.update(sql))
			{
				return true;
			}
		}
		return false;
	}
	bool onlinefriendlist(const int userid, std::map<int, muduo::string>&_mapres)
	{
		sprintf(sql, "select id,name from User where id in (select friendid from Friend where userid = %d) and state = 'ONLINE'"
			, userid);
		MySQL mysql;
		if (mysql.connect())
		{
			MYSQL_RES *res = mysql.query(sql);
			if (res != nullptr)
			{
				MYSQL_ROW row;
				while (row = mysql_fetch_row(res))
				{
					int id = atoi(row[0]);
					_mapres[id] = row[1];
				}
				return true;
			}
		}
		else
			LOG_INFO << "function request online friendlist -> connect to database error!";
		return false;
	}
	bool finduser(const int userid,muduo::string &name)
	{
		sprintf(sql, "select name from User where id = %d", userid);
		MySQL mysql;
		if (mysql.connect())
		{
			MYSQL_RES *res = mysql.query(sql);
			if (res != nullptr)
			{
				MYSQL_ROW row;
				row = mysql_fetch_row(res);
				if (row != nullptr)
				{
					name = row[0];
					return true;
				}
				return false;
			}
			LOG_INFO << "while try to find userid !query failed!";
		}
		LOG_INFO << "while try to find userid !connect to database failed!";
		return false;
	}
	bool addfriend(const int userid, const int friendid, muduo::string &verifymsg)
	{
		sprintf(sql, "insert  Request (id,fromid,type,msg) VALUES(%d,%d,'F','%s')", userid, friendid, verifymsg.c_str());
		MySQL mysql;
		if (mysql.connect())
		{
			if (mysql.update(sql))
			{
				return true;
			}
			else
			{
				return false;
			}
		}
	}

	//发送好友申请列表请求
	bool requestList(const int userid,std::map<int,muduo::string>&_resmap)
	{
		sprintf(sql, "select id,msg from Request where type = 'F' and fromid = %d", userid);
		MySQL mysql;
		if (mysql.connect())
		{
			MYSQL_RES *res = mysql.query(sql);
			if (res != nullptr)
			{
				MYSQL_ROW row;
				while (row = mysql_fetch_row(res))
				{
					int id = atoi(row[0]);
					_resmap[id] = row[1];
				}
				return true;
			}
		}
		LOG_INFO << "mysql connect failed!";
		return false;
	}

	//接受好友请求
	bool acceptRequest(const int userid, const int friendid)
	{
		sprintf(sql, "delete from Request where id = %d and fromid = %d", friendid, userid);
		MySQL mysql;
		if (mysql.connect())
		{
			if (mysql.update(sql))//删除一条记录 添加俩条记录
			{
				sprintf(sql, "insert Friend (userid,friendid) VALUES(%d,%d)", friendid, userid);
				if (mysql.update(sql))
				{
					sprintf(sql, "insert Friend (userid,friendid) VALUES(%d,%d)", userid, friendid);
					if (mysql.update(sql))
						return true;
				}
			}
		}
		return false;
	}
	virtual muduo::string getUsernameFromId(const int userid)
	{
		sprintf(sql, "select name from User where id = %d", userid);
		MySQL mysql;
		if (mysql.connect())
		{
			MYSQL_RES *res = mysql.query(sql);
			MYSQL_ROW row = mysql_fetch_row(res);
			return muduo::string(row[0]);
		}
	}
};