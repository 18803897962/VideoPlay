#include "MediaFile.h"

MediaFile::MediaFile()
	:m_file(NULL),m_type(-1),m_size(-1)
{}

MediaFile::~MediaFile(){
	Close();
}

int MediaFile::Open(const MyBuffer& path, int nType){
	m_file = fopen(path.c_str(), "rb");
	if (m_file == NULL) {
	//TODO:������ʾ
		return -1;
	}
	m_type = nType;
	fseek(m_file, 0, SEEK_END);
	m_size = ftell(m_file);
	fseek(m_file, 0, SEEK_SET);
	return 0;
}

MyBuffer MediaFile::ReadOneFrame(){
	switch (m_type)
	{
	case 96://H264
		return ReadH264Frame();
	default:
		break;
	}
	return MyBuffer();
}

void MediaFile::Close(){
	if (m_file != NULL) {
		FILE* file = m_file;
		m_file = NULL;
		fclose(file);
	}
	m_type = -1;
	m_size = -1;
}

void MediaFile::Reset(){
	if (m_file) {
		fseek(m_file, 0, SEEK_SET);//��ͷ��ʼ
	}
}

long MediaFile::FindH264Head(){//�ҵ�һ��֡�Ŀ�ͷλ��
	char c;
	while (!feof(m_file)) {
		while ((c = fgetc(m_file)) != 0);
		int cnt = 0;
		while (c == 0) {
			cnt++;
			c = fgetc(m_file);
		}
		if (cnt >= 2) {
			if (c == 1) {
				return ftell(m_file) - cnt - 1;
			}
		}
	}
	return -1;
}

MyBuffer MediaFile::ReadH264Frame(){
	MyBuffer frame;
	if (m_file != NULL) {
		long begin = FindH264Head();//��ǰ֡��ͷ
		if (begin != -1) {
			fseek(m_file, begin + 3, SEEK_SET);
			long end = FindH264Head();//��һ��֡�Ŀ�ͷ���������統ǰ֡�Ľ�β,���������ʱ���޷��ҵ���һ֡����Ҫ���⴦�� 
			long size = 0;
			if (end == -1) size = m_size - begin;
			else size = end - begin;
			frame.resize(size);
			fseek(m_file, begin, SEEK_SET);
			fread((void*)frame.c_str(), 1, size, m_file);
			return frame;
		}
	}
	return frame;
}
