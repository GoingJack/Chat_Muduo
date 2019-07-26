
#include "public.h"
#include <unordered_map>
#include <memory>

// ����������ҵ��ʵ��
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