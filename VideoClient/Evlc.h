#pragma once
#include<string>
#include"vlc.h"
class CVlcSize {
public:
	CVlcSize(int width = 0, int height = 0) :nWidth(width), nHeight(height) {}
	CVlcSize(const CVlcSize& size) {
		nWidth = size.getWidth();
		nHeight = size.getHeight();
	}
	CVlcSize& operator=(const CVlcSize& size) {
		if (&size != this) {
			nWidth = size.getWidth();
			nHeight = size.getHeight();
		}
		return *this;
	}
	int getWidth() const {
		return nWidth;
	}
	int getHeight() const {
		return nHeight;
	}
private:
	int nWidth;
	int nHeight;
};
class CEvlc
{
public:
	CEvlc();
	~CEvlc();
	//strUrl若包含中文，请传入utf-8格式编码
	int SetMedia(const std::string& strUrl);
#ifdef WIN32
	int SetHwnd(HWND hWnd);
#endif 
	int Play();
	int Pause();
	int Stop();
	float GetPosition();
	void SetPosition(float pos);
	int GetVolume();
	int SetVolume(int volume);
	float getLength();
	CVlcSize GetMediaInfo();
	//编码问题，多字节转utf-8  步骤为 多字节 -- unicode -- utf-8
	std::string unicode2utf8(const std::wstring& strin);
protected:
	libvlc_instance_t* m_instance;
	libvlc_media_t* m_media;
	libvlc_media_player_t* m_player;
	std::string m_url;
#ifdef WIN32
	HWND m_hwnd;
#endif 
};

