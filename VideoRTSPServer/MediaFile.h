#pragma once
#include"Socket.h"
#include"base.h"
class MediaFile
{
public:
	MediaFile();
	~MediaFile();
	int Open(const MyBuffer& path, int nType);//打开文件
	MyBuffer ReadOneFrame();//按照相应type读取一个帧
	void Close();//关闭
	//重置后ReadOneFrame()又会有值返回
	void Reset();//重置
private:
	long FindH264Head();
	MyBuffer ReadH264Frame();//读取一个H264帧
private:
	long m_size;
	FILE* m_file;//文件指针
	MyBuffer m_filepath;//文件路径
	int m_type;//文件类型  96:H264
};

