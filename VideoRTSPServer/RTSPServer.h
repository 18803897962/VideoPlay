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
typedef void(*RTSPPLAYCB)(RTSPServer*, RTSPSession&);//回调函数
//void PlayCallBack(RTSPServer* thiz, RTSPSession& session);

class RTSPRequest {//请求类
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
	int method() const;//使用int而不是用MyBuffer的原因是进行switch的效率更高，而无需字符串比较
	const MyBuffer& url() const;
	const MyBuffer& session() const;
	const MyBuffer& sequence() const;
	const MyBuffer& port(int index = 0) const;
private:
	int m_method;//方法 0:OPTIONS 1:DESCRIBE 2:SETUP 3:PLAY 4:TEARDOWM 默认-1
	MyBuffer m_url;
	MyBuffer m_session;
	MyBuffer m_seq;
	MyBuffer m_client_port[2];
};

class RTSPReply {//回复类
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
	int m_method;//方法 0:OPTIONS 1:DESCRIBE 2:SETUP 3:PLAY 4:TEARDOWM
	int m_client_port[2];
	int m_server_port[2];
	MyBuffer m_sdp;
	MyBuffer m_options;
	MyBuffer m_session;
	MyBuffer m_seq;
};

class RTSPSession {//会话类
public:
	RTSPSession();
	RTSPSession(const ESocket& client);
	RTSPSession(const RTSPSession& session);
	RTSPSession& operator=(const RTSPSession& session);
	~RTSPSession() {}
	int PickRequestAndReply(RTSPPLAYCB cb,RTSPServer* thiz);//解析请求包并回复
	EAddress GetUDPAddress();
private:
	MyBuffer PickOneLine(MyBuffer& buffer);
	void Pick(MyBuffer& buffer);
	RTSPRequest AnalyseRequest(const MyBuffer& buffer);
	RTSPReply MakeReply(const RTSPRequest& request);
private:
	MyBuffer m_id;//会话id
	ESocket m_client;//对应的客户端
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
	//返回0继续，返回负数表示终止，其它表示警告
	int threadWorker();
	int ThreadSession();//处理session
	static void PlayCallBack(RTSPServer* thiz, RTSPSession& session);//回调函数
	void UDPWorker(EAddress& client_addr);
private:
	ESocket m_socket;
	int m_status;//0未初始化 1初始化完成 2正在运行 3关闭
	EAddress m_addr;
	CMyThread m_threadMain;
	MyThreadPoor m_pool;//线程池用于处理session
	//std::map<std::string, RTSPSession> m_mapSessions;
	static SocketIniter m_initer;
	CQueue<RTSPSession> m_lstsession;//线程安全队列 用于存放客户端
	RTPHelper m_helper;
	MediaFile m_h264;
};

