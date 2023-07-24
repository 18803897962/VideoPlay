#pragma once
#include<WinSock2.h>
#include <memory>
#include"base.h"
#pragma comment(lib, "ws2_32.lib")//將网路api函数导入工程文件
class CSocket
{
public:
	CSocket(bool isTcp = true) {
		m_sock = INVALID_SOCKET;
		if (isTcp) {
			m_sock = socket(PF_INET, SOCK_STREAM, 0);
		}
		else{
			m_sock = socket(PF_INET, SOCK_DGRAM, 0);
		}
	}
	CSocket(SOCKET sock) {
		m_sock = sock;
	}
	void Close() {
		if (m_sock != INVALID_SOCKET) {
			SOCKET tmp = m_sock;
			m_sock = INVALID_SOCKET;
			closesocket(tmp);
		}
	}
	operator SOCKET() {
		return m_sock;
	}
	~CSocket() {
		Close();
	}
private:
	SOCKET m_sock;
};


class EAddress {
public:
	EAddress():m_IP(std::string()),m_port(0) { 
		memset(&m_addr, 0, sizeof(sockaddr_in)); 
		m_addr.sin_family = AF_INET;
	}
	EAddress(const std::string& strIP, unsigned short port) {
		m_IP = strIP;
		m_port = port;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(port);
		m_addr.sin_addr.s_addr = inet_addr(strIP.c_str());
	}
	~EAddress() {}
	EAddress(const EAddress& addr) {
		m_IP = addr.m_IP;
		m_port = addr.m_port;
		memcpy(&m_addr, &addr.m_addr, sizeof(sockaddr_in));
	}
	EAddress& operator=(const EAddress& addr) {
		if (&addr != this) {
			m_IP = addr.m_IP;
			m_port = addr.m_port;
			memcpy(&m_addr, &addr.m_addr, sizeof(sockaddr_in));
		}
		return *this;
	}
	EAddress& operator=(unsigned short port) {
		m_port = port;
		m_addr.sin_port = htons(port);
		return *this;
	}
	operator const sockaddr* () const {
		return (sockaddr*)&m_addr;
	}
	operator sockaddr* (){
		return (sockaddr*)&m_addr;
	}
	operator const sockaddr_in* () const {
		return &m_addr;
	}
	operator sockaddr_in* () {
		return &m_addr;
	}
	void Update(const std::string& strIP, short port) {
		m_IP = strIP;
		m_port = port;
		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(port);
		m_addr.sin_addr.s_addr = inet_addr(strIP.c_str());
	}
	int Size() const { 
		return sizeof(sockaddr_in); 
	}
	const std::string IP() const{
		return m_IP;
	}
	const unsigned short port() const{
		return m_port;
	}
	void Fresh() {
		m_IP = inet_ntoa(m_addr.sin_addr);
	}
private:
	std::string m_IP;
	unsigned short m_port;
	sockaddr_in m_addr;
};


class ESocket {
public:
	ESocket(bool isTcp = true) 
		:m_socket(new CSocket(isTcp)),
		m_isTCP(isTcp){}
	ESocket(const ESocket& esock) 
		:m_socket(esock.m_socket),
		m_isTCP(esock.m_isTCP){}
	ESocket& operator=(const ESocket& esock) {
		if (this != &esock) {
			m_socket = esock.m_socket;
			m_isTCP = esock.m_isTCP;
		}
		return *this;
	}
	~ESocket() {
		m_socket.reset();
	}
	ESocket(SOCKET sock,bool isTCP):m_socket(new CSocket(sock)),m_isTCP(isTCP){}
	int Bind(const EAddress& addr) {
		if (m_socket == nullptr) {
			m_socket.reset(new CSocket(m_isTCP));
		}
		return bind(*m_socket,addr,addr.Size());
	}
	int Listen(int backlog = 3) {
		return listen(*m_socket, backlog);
	}
	ESocket Accept(EAddress& addr) {
		int len=addr.Size();
		if (m_socket == nullptr) return ESocket(INVALID_SOCKET,true);
		SOCKET server = *m_socket;//当无客户端连接时，返回一个空
		if (server == INVALID_SOCKET) return ESocket(INVALID_SOCKET, true);
		SOCKET s = accept(*m_socket,(sockaddr*)addr,&len);
		return ESocket(s, m_isTCP);
	}
	int Connect(const EAddress& addr) {
		return connect(*m_socket, addr, addr.Size());
	}
	int Recv(MyBuffer& buffer) {
		//TODO:待优化
		int ret = recv(*m_socket, (char*)buffer.c_str(), buffer.size(), 0);
		if (ret <= 0) printf("error code=%d\r\n", WSAGetLastError());
		return ret;
	}
	int Send(const MyBuffer& buffer) {
		size_t index = 0;//已发送的数据量
		char* pData = buffer;
		while (index < buffer.size()) {
			int ret = send(*m_socket, (char*)buffer + index, buffer.size() - index, 0);
			if (ret < 0) return ret;
			if (ret == 0) break;//对方关闭
			index += ret;
		}
		return index;
	}
	int Close() {
		m_socket.reset();
		return 0;
	}
	operator SOCKET() {
		return *m_socket;
	}
private:
	std::shared_ptr<CSocket> m_socket;
	bool m_isTCP;
};

class SocketIniter {//用于初始化套接字环境
public:
	SocketIniter() {
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);
	}
	~SocketIniter() {
		WSACleanup();
	}
};

