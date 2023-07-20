#pragma once
#include<string>
#include<map>
#include"Socket.h"
#include"MyThread.h"
#include"Queue.h"
class RTSPRequest {//请求类
public:
	RTSPRequest();
	RTSPRequest(const RTSPRequest& rsp);
	RTSPRequest& operator=(const RTSPRequest& rsp);
	~RTSPRequest();
private:
	int m_method;//方法 0:OPTIONS 1:DESCRIBE 2:SETUP 3:PLAY 4:TEARDOWM
};

class RTSPReply {//回复类
public:
	RTSPReply();
	RTSPReply(const RTSPReply& rsp);
	RTSPReply& operator=(const RTSPReply& rsp);
	~RTSPReply();
	MyBuffer toBuffer();
private:
	int m_method;//方法 0:OPTIONS 1:DESCRIBE 2:SETUP 3:PLAY 4:TEARDOWM
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
	RTSPServer();
	~RTSPServer();
	int Init(const std::string& strIP = "0.0.0.0", short port = 554);
	int Invoke();
	void Stop();
protected:
	//返回0继续，返回负数表示终止，其它表示警告
	int threadWorker();
	RTSPRequest AnalyseRequest(const std::string& data);//解析请求包
	RTSPReply MakeReply(const RTSPRequest& request);//根据请求得道回复包
	int ThreadSession();//处理session
private:
	ESocket m_socket;
	int m_status;//0未初始化 1初始化完成 2正在运行 3关闭
	EAddress m_addr;
	CMyThread m_threadMain;
	MyThreadPoor m_pool;//线程池用于处理session
	std::map<std::string, RTSPSession> m_mapSessions;
	static SocketIniter m_initer;
	CQueue<ESocket> m_clients;//线程安全队列 用于存放客户端
};

