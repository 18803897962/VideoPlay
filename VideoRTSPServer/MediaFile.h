#pragma once
#include"Socket.h"
#include"base.h"
class MediaFile
{
public:
	MediaFile();
	~MediaFile();
	int Open(const MyBuffer& path, int nType);//���ļ�
	MyBuffer ReadOneFrame();//������Ӧtype��ȡһ��֡
	void Close();//�ر�
	//���ú�ReadOneFrame()�ֻ���ֵ����
	void Reset();//����
private:
	long FindH264Head();
	MyBuffer ReadH264Frame();//��ȡһ��H264֡
private:
	long m_size;
	FILE* m_file;//�ļ�ָ��
	MyBuffer m_filepath;//�ļ�·��
	int m_type;//�ļ�����  96:H264
};

