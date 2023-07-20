#include "RTSPServer.h"

RTSPServer::RTSPServer() :m_socket(true), m_status(0), m_pool(10) {
	m_threadMain.UpdateWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::threadWorker));//threadWorker���ܴ���ͬ��
}

RTSPServer::~RTSPServer(){
	Stop();
}

int RTSPServer::Init(const std::string& strIP, short port){
	m_addr.Update(strIP, port);
	m_socket.Bind(m_addr);
	m_socket.Listen();
	return 0;
}

int RTSPServer::Invoke(){
	m_threadMain.Start();
	return 0;
}

void RTSPServer::Stop(){
	m_socket.Close();
	m_threadMain.Stop();
	m_pool.Stop();
}

int RTSPServer::threadWorker(){
	EAddress client_addr;
	ESocket client = m_socket.Accept(client_addr);
	if (client != INVALID_SOCKET) {//�ַ�һ���Ự
		m_clients.PushBack(client);
		m_pool.dispatchWorker(ThreadWorker(this, (FUNCTYPE)&RTSPServer::ThreadSession));
	}
	return 0;
}

RTSPRequest RTSPServer::AnalyseRequest(const std::string& data)
{
	return RTSPRequest();
}

RTSPReply RTSPServer::MakeReply(const RTSPRequest& request)
{
	return RTSPReply();
}

int RTSPServer::ThreadSession(){//���ÿ���ͻ��˶�����һ���Ự
	//TODO:1����������  2����������  3��Ӧ������
	//TODO:���� �ͻ����׽���Ҫ��Session�������
	ESocket client;//�ٶ��õ��˿ͻ���client
	MyBuffer buffer = MyBuffer(1024 * 16);
	int len = client.Recv(buffer);
	if (len <= 0) {
		//TODO:�������ǰclient
		return -1;
	}
	buffer.resize(len);
	RTSPRequest request = AnalyseRequest(buffer);
	RTSPReply reply = MakeReply(request);
	client.Send(reply.toBuffer());
	return 0;
}
