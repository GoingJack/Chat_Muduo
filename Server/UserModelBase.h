#pragma once

#include "UserDO.h"
#include "MySQL.h"
#include <map>
#include <vector>


// Model��ĳ�����
class UserModelBase
{
public:
	// �����û�
	virtual bool add(UserDO &user) = 0;
	// �û���¼���
	virtual bool login(UserDO &user) = 0;
	//�û���״̬��Ϊ������
	virtual bool exit(UserDO &user) = 0;
	//�����ݿ��л�ȡ�����б�
	virtual bool friendlist(const int userid, std::map<int, muduo::string>&res) = 0;
	//�����ݸ�����Щ�˳���¼�û��ĵ�¼״̬
	virtual bool logout(const int userid) = 0;
	//�����ݿ��л�ȡ���ߵĺ����б�
	virtual bool onlinefriendlist(const int userid, std::map<int, muduo::string>&res) = 0;
	//�����ݿ��в�ѯ����
	virtual bool finduser(const int userid,muduo::string &username) = 0;
	//��Request���������صĺ���������Ϣ
	virtual bool addfriend(const int userid, const int friendid, muduo::string &verifymsg) = 0;

	//���ͺ��������б�����
	virtual bool requestList(const int userid, std::map<int, muduo::string>&_resmap) = 0;

	//���ܺ�������
	virtual bool acceptRequest(const int userid, const int friendid)=0;

	//��ȡ�û�����ͨ��IDֵ
	virtual muduo::string getUsernameFromId(const int userid) = 0;
};

// User���Model�����
class UserModel : public UserModelBase
{
private:
	char sql[1024];
public:
	UserModel() { memset(sql, 0, 1024); }
	// ��дadd�ӿڷ�����ʵ�������û�����
	bool add(UserDO &user)
	{
		// ��֯sql���
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

	// ��дlogin�ӿڷ�����ʵ�ּ���¼�û����û��������Ƿ���ȷ
	bool login(UserDO &user)
	{
		// ��֯sql���
		sprintf(sql, "select id from User where name='%s' and password='%s'",
			user.getName().c_str(),
			user.getPwd().c_str());

		MySQL mysql;
		if (mysql.connect())
		{
			// ע��MYSQL_RESָ����Զ��Ϊ�գ������Ƿ��ѯ������
			MYSQL_RES *res = mysql.query(sql);
			if (res != nullptr)
			{
				MYSQL_ROW row = mysql_fetch_row(res);
				// MYSQL_ROW��char**��ͨ��row�Ƿ�Ϊnullptr���жϲ�ѯ�Ƿ񷵻ؽ��
				if (row != nullptr)
				{
					LOG_INFO << "login success => sql:" << sql;
					user.setID(atoi(row[0]));
					mysql_free_result(res);


					//���õ�¼��״̬
					memset(sql, 0, 1024);
					sprintf(sql, "update User set state = 'ONLINE' where id = %d", user.getID());
					return mysql.update(sql);
				}
			}
		}
		LOG_INFO << "login error => sql:" << sql;
		return false;
	}

	// ��дexit�ӿڷ������޸�user���״̬
	bool exit(UserDO &user)
	{
		// ��֯sql���
		sprintf(sql, "update User set state = 'OFFLINE' where id = %d",
			user.getID());

		MySQL mysql;
		if (mysql.connect())
		{
			// ע��MYSQL_RESָ����Զ��Ϊ�գ������Ƿ��ѯ������
			if (mysql.update(sql))
			{
				LOG_INFO << "client offline exception update User success!" << user.getID();
				return true;
			}
			
		}
		LOG_INFO << "client offline exception update User error!" << user.getID();
		return false;
	}
	//�����ݿ��л�ȡ�����б�
	bool friendlist(const int userid,std::map<int,muduo::string>&_resmap)
	{
		// ��֯sql���
		sprintf(sql, "select  id,name from User where id in ( select friendid from Friend where userid = %d)",
			userid
			);

		MySQL mysql;
		if (mysql.connect())
		{
			// ע��MYSQL_RESָ����Զ��Ϊ�գ������Ƿ��ѯ������
			MYSQL_RES *res = mysql.query(sql);
			if (res != nullptr)
			{
				// MYSQL_ROW��char**��ͨ��row�Ƿ�Ϊnullptr���жϲ�ѯ�Ƿ񷵻ؽ��
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

	//���ͺ��������б�����
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

	//���ܺ�������
	bool acceptRequest(const int userid, const int friendid)
	{
		sprintf(sql, "delete from Request where id = %d and fromid = %d", friendid, userid);
		MySQL mysql;
		if (mysql.connect())
		{
			if (mysql.update(sql))//ɾ��һ����¼ ���������¼
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