#include "ChatServer.h"
#include "ClusterService.h"
#include <thread>

/*
当ChatServer接收到新连接或连接中断时，会调用该回调函数，并把连接信息封装在
TcpConnection中通过参数传递进来
*/
void ChatServer::onConnection(const muduo::net::TcpConnectionPtr &con)
{
	// muduo使用示例代码
	if (con->connected())//有新的客户端连接
	{
		LOG_INFO << "one client success" << con->peerAddress().toIpPort();
	}
	else//客户端异常断开
	{
		LOG_INFO << "one client success" << con->peerAddress().toIpPort();
		App().exitChlient(con);
	}
	
}

/*
当ChatServer已存在连接接收到新数据时，会调用该回调函数，并把数据封装在
Buffer中通过参数传递进来
*/
void ChatServer::onMessage(const muduo::net::TcpConnectionPtr &con,
	muduo::net::Buffer *buf,
	muduo::Timestamp time)
{
	muduo::string msg(buf->retrieveAllAsString());
	json js = json::parse(msg);
	int msgid = js["msgid"];
	App().handler()[js["msgid"].get<int>()](con, js, time);
}