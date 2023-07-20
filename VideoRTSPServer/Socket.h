#pragma once
#include<WinSock2.h>
#include <memory>
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
	EAddress() { 
		m_port = -1; 
		memset(&m_addr, 0, sizeof(sockaddr_in)); 
		m_addr.sin_family = AF_INET;
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
private:
	std::string m_IP;
	short m_port;
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
	int Listen(int backlog = 5) {
		return listen(*m_socket, backlog);
	}
	ESocket Accept(EAddress& addr) {
		int len=addr.Size();
		SOCKET s = accept(*m_socket,addr,&len);
		return ESocket(s, m_isTCP);
	}
	int Connect(const EAddress& addr) {
		return connect(*m_socket, addr, addr.Size());
	}
	int Recv(MyBuffer& buffer) {
		//TODO:待优化
		return recv(*m_socket, buffer, buffer.size(), 0);
	}
	int Send(const MyBuffer& buffer) {
		//TODO:待优化
		return send(*m_socket, buffer, buffer.size(), 0);
	}
	int Close() {
		m_socket.reset();
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

class MyBuffer :public std::string {
public:
	MyBuffer(size_t size = 0) :std::string() {//调用string的构造函数
		if (size > 0) {
			resize(size);
			memset(this, 0, size);
		}
	}
	MyBuffer(void* buffer, size_t size) :std::string() {
		memcpy((void*)c_str(), buffer, size);
	}
	MyBuffer(const char* str) {
		resize(strlen(str));
		memcpy((void*)c_str(), str, strlen(str));
	}
	~MyBuffer() {
		std::string::~basic_string();
	}
	operator char* () const {
		return (char*)c_str();
	}
	operator const char* () const {
		return (char*)c_str();
	}
	operator BYTE* () const {
		return (BYTE*)c_str();
	}
	operator void* () const {
		return (void*)c_str();
	}
	void Update(void* buffer, size_t size) {
		resize(size);
		memcpy((void*)c_str(), buffer, size);
	}
};