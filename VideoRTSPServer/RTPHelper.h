#pragma once
#include"base.h"
#include"Socket.h"
class RTPHeader {
public:
	//位域，每个字节内，都由声明顺序从低位到高位安排顺序
	unsigned short csrccount : 4;//特约信源个数 4bit
	unsigned short extension : 1;//拓展位 1bit
	unsigned short padding : 1;//填充位 1bit
	unsigned short version : 2;//版本号 2bit :表示位域
	unsigned short pytype : 7;//荷载类型 7bit
	unsigned short mark : 1;//标志位 1bit
	unsigned short serial;//序列号 2字节
	unsigned timestamp;//时间戳 4字节
	unsigned ssrc;//同步信源，一个rtp会话只能有一个同步信源
	unsigned csrc[15];//特约信源 最多15个
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
	RTPHeader m_head;//头部
	MyBuffer m_pyload;//数据荷载
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
	FILE* m_file;//用于调试的输出文件
};

