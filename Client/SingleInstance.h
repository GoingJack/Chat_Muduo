
#include "public.h"
#include <unordered_map>
#include <memory>

// 单机服务器业务实现
class SingleService 
{
public:
	static SingleService& getInstance()
	{
		static SingleService instance;
		return instance;
	}
	SingleService(const SingleService&) = delete;
	SingleService& operator=(const SingleService&) = delete;

	

	
private:
	std::unique_ptr<UserModelBase> userModelPtr;
	std::unique_ptr<FriendModelBase> friendModelPtr;
	std::unique_ptr<GroupModelBase> groupModelPtr;

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