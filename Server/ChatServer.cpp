#include "ChatServer.h"
#include "ClusterService.h"
#include <thread>

/*
��ChatServer���յ������ӻ������ж�ʱ������øûص�����������������Ϣ��װ��
TcpConnection��ͨ���������ݽ���
*/
void ChatServer::onConnection(const muduo::net::TcpConnectionPtr &con)
{
	// muduoʹ��ʾ������
	if (con->connected())//���µĿͻ�������
	{
		LOG_INFO << "one client success" << con->peerAddress().toIpPort();
	}
	else//�ͻ����쳣�Ͽ�
	{
		LOG_INFO << "one client success" << con->peerAddress().toIpPort();
		App().exitChlient(con);
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
	json js = json::parse(msg);
	int msgid = js["msgid"];
	App().handler()[js["msgid"].get<int>()](con, js, time);
}