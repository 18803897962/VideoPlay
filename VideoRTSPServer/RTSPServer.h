#pragma once
#include<string>
#include<map>
#include"Socket.h"
#include"MyThread.h"
#include"Queue.h"
#include"RTPHelper.h"
#include"MediaFile.h"
class RTSPServer;
class RTSPSession;
typedef void(*RTSPPLAYCB)(RTSPServer*, RTSPSession&);//�ص�����
//void PlayCallBack(RTSPServer* thiz, RTSPSession& session);

class RTSPRequest {//������
public:
	RTSPRequest();
	RTSPRequest(const RTSPRequest& rsp);
	RTSPRequest& operator=(const RTSPRequest& rsp);
	~RTSPRequest();
	void SetMethod(const MyBuffer& method);
	void SetUrl(const MyBuffer& url);
	void SetSession(const MyBuffer& session);
	void SetSequence(const MyBuffer& seq);
	void SetClientPort(int ports[]);
	int method() const;//ʹ��int��������MyBuffer��ԭ���ǽ���switch��Ч�ʸ��ߣ��������ַ����Ƚ�
	const MyBuffer& url() const;
	const MyBuffer& session() const;
	const MyBuffer& sequence() const;
	const MyBuffer& port(int index = 0) const;
private:
	int m_method;//���� 0:OPTIONS 1:DESCRIBE 2:SETUP 3:PLAY 4:TEARDOWM Ĭ��-1
	MyBuffer m_url;
	MyBuffer m_session;
	MyBuffer m_seq;
	MyBuffer m_client_port[2];
};

class RTSPReply {//�ظ���
public:
	RTSPReply();
	RTSPReply(const RTSPReply& rsp);
	RTSPReply& operator=(const RTSPReply& rsp);
	~RTSPReply();
	MyBuffer toBuffer();
	void SetOptions(const MyBuffer& option);
	void SetSequence(const MyBuffer& seq);
	void SetSdp(const MyBuffer& sdp);
	void SetClientPort(const MyBuffer& port0, const MyBuffer& port1);
	void SetServerPort(const MyBuffer& port0, const MyBuffer& port1);
	void SetSession(const MyBuffer& session);
	void SetMethod(int method);
	int GetMethod();
private:
	int m_method;//���� 0:OPTIONS 1:DESCRIBE 2:SETUP 3:PLAY 4:TEARDOWM
	int m_client_port[2];
	int m_server_port[2];
	MyBuffer m_sdp;
	MyBuffer m_options;
	MyBuffer m_session;
	MyBuffer m_seq;
};

class RTSPSession {//�Ự��
public:
	RTSPSession();
	RTSPSession(const ESocket& client);
	RTSPSession(const RTSPSession& session);
	RTSPSession& operator=(const RTSPSession& session);
	~RTSPSession() {}
	int PickRequestAndReply(RTSPPLAYCB cb,RTSPServer* thiz);//������������ظ�
	EAddress GetUDPAddress();
private:
	MyBuffer PickOneLine(MyBuffer& buffer);
	void Pick(MyBuffer& buffer);
	RTSPRequest AnalyseRequest(const MyBuffer& buffer);
	RTSPReply MakeReply(const RTSPRequest& request);
private:
	MyBuffer m_id;//�Ựid
	ESocket m_client;//��Ӧ�Ŀͻ���
	unsigned short m_port;
};


class RTSPServer:public ThreadFuncBase
{
public:
	RTSPServer();
	~RTSPServer();
	int Init(const std::string& strIP = "0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
protected:
	//����0���������ظ�����ʾ��ֹ��������ʾ����
	int threadWorker();
	int ThreadSession();//����session
	static void PlayCallBack(RTSPServer* thiz, RTSPSession& session);//�ص�����
	void UDPWorker(EAddress& client_addr);
private:
	ESocket m_socket;
	int m_status;//0δ��ʼ�� 1��ʼ����� 2�������� 3�ر�
	EAddress m_addr;
	CMyThread m_threadMain;
	MyThreadPoor m_pool;//�̳߳����ڴ���session
	//std::map<std::string, RTSPSession> m_mapSessions;
	static SocketIniter m_initer;
	CQueue<RTSPSession> m_lstsession;//�̰߳�ȫ���� ���ڴ�ſͻ���
	RTPHelper m_helper;
	MediaFile m_h264;
};

