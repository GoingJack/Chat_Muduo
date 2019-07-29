#pragma once

#include "GroupDO.h"
#include "MySQL.h"

// Model��ĳ�����
class GroupModelBase
{
public:
	// ���Ӻ�����Ϣ
	virtual void add(GroupDO &grooup) = 0;

	//��ȡ����gid���е����е�uid
	virtual bool getAllUidFromGid(vector<int> &vec, const int gid) = 0;
	//����ָ����gid��ȡ���groupname
	virtual bool getGroupNameFromGid(muduo::string &name, const int gid) = 0;
};

// Group���Model�����
class GroupModel : public GroupModelBase
{
public:
	GroupModel() { memset(sql, 0, 1024); }
	void add(GroupDO &group)
	{
		
	}
	virtual bool getAllUidFromGid(vector<int> &vec, const int gid)
	{
		sprintf(sql, "select userid from GroupUser where groupid = %d", gid);
		MySQL mysql;
		if (mysql.connect())
		{
			MYSQL_RES *res = mysql.query(sql);
			MYSQL_ROW row;
			while (row = mysql_fetch_row(res))
			{
				vec.push_back(atoi(row[0]));
			}
			return true;
		}
	}
	//����ָ����gid��ȡ���groupname
	virtual bool getGroupNameFromGid(muduo::string &name,const int gid)
	{
		MySQL mysql;
		sprintf(sql, "select groupname from AllGroup where id = %d", gid);
		if (mysql.connect())
		{
			MYSQL_RES *res = mysql.query(sql);
			if (res != nullptr)
			{
				MYSQL_ROW row;
				row = mysql_fetch_row(res);
				name = row[0];
				return true;
			}
		}
	}

	
private:
	char sql[1024];
};