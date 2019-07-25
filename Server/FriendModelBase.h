#pragma once

#include "MySQL.h"

// Model层的抽象类
class FriendModelBase
{
public:
	// 增加好友信息
	virtual void add(int userid, int friendid) = 0;
};

// Friend表的Model层操作
class FriendModel : public FriendModelBase
{
public:
	void add(int userid, int friendid)
	{
	
	}
};
