#include "ChatServer.h"
#include "SingleService.h"
#include <thread>

/*
��ChatServer���յ������ӻ������ж�ʱ������øûص�����������������Ϣ��װ��
TcpConnection��ͨ���������ݽ���
*/
void ChatServer::onConnection(const muduo::net::TcpConnectionPtr &con)
{
	// muduoʹ��ʾ������
	/*LOG_INFO << "ChatServer:" << con->peerAddress().toIpPort() << "->"
		<< con->localAddress().toIpPort() << " state:"
		<< (con->connected() ? "UP" : "DOWN");
	if (!con->connected())
	{
		LOG_INFO << con->
	}
	LOG_INFO << "onConnection tid:" << pthread_self();*/
	if (con->connected())
	{
		LOG_INFO << "one client success!"<<con->
	}
	
}

/*
��ChatServer�Ѵ������ӽ��յ�������ʱ������øûص��������������ݷ�װ��
Buffer��ͨ���������ݽ���
*/
void ChatServer::onMessage(const muduo::net::TcpConnectionPtr &con,
	muduo::net::Buffer *buf,
	muduo::Timestamp time)
{
	muduo::string msg(buf->retrieveAllAsString());
	LOG_INFO << msg;
	json js = json::parse(msg);
	int msgid = js["msgid"];
	LOG_INFO << msgid;
	//std::unordered_map<int, Handler> _handlerMap;
	App().handler()[js["msgid"].get<int>()](con, js, time);

	if (js["msgid"] == MSG_LOGIN)
	{
		App().login(con, js, time);
	}
	else if (js["msgid"] == MSG_REG)
	{
		App().reg(con, js, time);
	}
}