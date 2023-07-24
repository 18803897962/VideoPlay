#pragma once
#include"base.h"
#include"Socket.h"
class RTPHeader {
public:
	//λ��ÿ���ֽ��ڣ���������˳��ӵ�λ����λ����˳��
	unsigned short csrccount : 4;//��Լ��Դ���� 4bit
	unsigned short extension : 1;//��չλ 1bit
	unsigned short padding : 1;//���λ 1bit
	unsigned short version : 2;//�汾�� 2bit :��ʾλ��
	unsigned short pytype : 7;//�������� 7bit
	unsigned short mark : 1;//��־λ 1bit
	unsigned short serial;//���к� 2�ֽ�
	unsigned timestamp;//ʱ��� 4�ֽ�
	unsigned ssrc;//ͬ����Դ��һ��rtp�Ựֻ����һ��ͬ����Դ
	unsigned csrc[15];//��Լ��Դ ���15��
public:
	RTPHeader();
	RTPHeader(const RTPHeader& header);
	RTPHeader& operator= (const RTPHeader & header);
	operator MyBuffer();
	//~RTPHeader();
	//operator MyBuffer();
};

class RTPFrame {
public:
	RTPHeader m_head;//ͷ��
	MyBuffer m_pyload;//���ݺ���
	operator MyBuffer();
};

class RTPHelper
{
public:
	RTPHelper();
	~RTPHelper();
	int SendMediaFrame(RTPFrame& rtpframe, MyBuffer& frame, const EAddress& client_addr);
private:
	int SendFrame(const MyBuffer& frame, const EAddress& client_addr);
	int GetFrameSepSize(const MyBuffer& frame);
	DWORD m_timestamp;
	ESocket m_udp;
	FILE* m_file;//���ڵ��Ե�����ļ�
};

