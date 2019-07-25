#pragma once

#include "GroupDO.h"
#include "MySQL.h"

// Model层的抽象类
class GroupModelBase
{
public:
	// 增加好友信息
	virtual void add(GroupDO &grooup) = 0;
};

// Group表的Model层操作
class GroupModel : public GroupModelBase
{
public:
	void add(GroupDO &group)
	{
		
	}
};