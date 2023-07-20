#pragma once
#include<string>
#include<map>
#include"Socket.h"
#include"MyThread.h"
class RTSPRequest {//������
public:
	RTSPRequest();
	RTSPRequest(const RTSPRequest& rsp);
	RTSPRequest& operator=(const RTSPRequest& rsp);
	~RTSPRequest();
private:
	int m_method;//���� 0:OPTIONS 1:DESCRIBE 2:SETUP 3:PLAY 4:TEARDOWM
};
class RTSPReply {//�ظ���
public:
	RTSPReply();
	RTSPReply(const RTSPReply& rsp);
	RTSPReply& operator=(const RTSPReply& rsp);
	~RTSPReply();
private:
	int m_method;//���� 0:OPTIONS 1:DESCRIBE 2:SETUP 3:PLAY 4:TEARDOWM
};
class RTSPSession {
public:
	RTSPSession();
	RTSPSession(const RTSPSession& session);
	RTSPSession& operator=(const RTSPSession& session);
	~RTSPSession();
};
class RTSPServer:public ThreadFuncBase
{
public:
	RTSPServer() :m_socket(true) ,m_status(0){}
	~RTSPServer() {}
	int Init(const std::string& strIP = "0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
protected:
	int ThreadWorker();
	RTSPRequest AnalyseRequest(const std::string& data);//���������
	RTSPReply MakeReply(const RTSPRequest& request);//��������õ��ظ���
	int ThreadSession();//����session
private:
	ESocket m_socket;
	int m_status;//0δ��ʼ�� 1��ʼ����� 2�������� 3�ر�
	CMyThread m_threadMain;
	MyThreadPoor m_pool;//�̳߳����ڴ���session
	std::map<std::string, RTSPSession> m_mapSessions;
	static SocketIniter m_initer;
};

