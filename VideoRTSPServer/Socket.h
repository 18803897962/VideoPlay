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

class ESocket {
public:
	ESocket(bool isTcp = true) 
		:m_socket(new CSocket(isTcp)) {}
	ESocket(const ESocket& esock) 
		:m_socket(esock.m_socket){}
	ESocket& operator=(const ESocket& esock) {
		if (this != &esock) {
			m_socket = esock.m_socket;
		}
	}
	~ESocket() {
		m_socket.reset();
	}
	operator SOCKET() {
		return *m_socket;
	}
private:
	std::shared_ptr<CSocket> m_socket;
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