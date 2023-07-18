#include "pch.h"
#include "Evlc.h"

CEvlc::CEvlc(){
	m_instance = libvlc_new(0, NULL);
	m_media = NULL;
	m_player = NULL;
	m_hwnd = NULL;
}

CEvlc::~CEvlc(){
	if (m_player != NULL) {
		libvlc_media_player_t* tmp = m_player;
		m_player = NULL;
		libvlc_media_player_release(tmp);
	}
	if (m_media != NULL) {
		libvlc_media_t* tmp = m_media;
		m_media = NULL;
		libvlc_media_release(tmp);
	}
	if (m_instance != NULL) {
		libvlc_instance_t* tmp = m_instance;
		m_instance = NULL;
		libvlc_release(tmp);
	}
}

int CEvlc::SetMedia(const std::string& strUrl){//对重复设置media进行处理
	if (m_instance == NULL || m_hwnd == NULL) return -1;
	if (m_url == strUrl) return 0;
	m_url = strUrl;
	if (m_media != NULL) {
		libvlc_media_release(m_media);
		m_media = NULL;
	}
	m_media = libvlc_media_new_location(m_instance,strUrl.c_str());
	if (!m_media) return -2;
	if (m_player != NULL) {
		libvlc_media_player_release(m_player);
		m_player = NULL;
	}
	m_player = libvlc_media_player_new_from_media(m_media);
	if (!m_player) return -3;
	if (!m_instance || !m_media || !m_player) return -1;
	
	//设置视频的播放尺寸
	CRect rect;
	GetWindowRect(m_hwnd,&rect);
	std::string strRatio = "";
	strRatio.resize(32);
	sprintf((char*)strRatio.c_str(), "%d:%d", rect.Width(), rect.Height());
	libvlc_video_set_aspect_ratio(m_player, strRatio.c_str());
	libvlc_media_player_set_hwnd(m_player, m_hwnd);

	return 0;
}

#ifdef DEBUG
int CEvlc::SetHwnd(HWND hWnd) {
	m_hwnd = hWnd;
	return 0;
}
#endif // DEBUG

int CEvlc::Play(){
	if (!m_instance || !m_media || !m_player) return -1;
	return libvlc_media_player_play(m_player);
}

int CEvlc::Pause(){
	if (!m_instance || !m_media || !m_player) return -1;
	libvlc_media_player_pause(m_player);
	return 0;
}

int CEvlc::Stop(){
	if (!m_instance || !m_media || !m_player) return -1;
	libvlc_media_player_stop(m_player);
	return 0;
}

float CEvlc::GetPosition(){
	if (!m_instance || !m_media || !m_player) return -1;
	return libvlc_media_player_get_position(m_player);
}

void CEvlc::SetPosition(float pos){
	if (!m_instance || !m_media || !m_player) return;
	libvlc_media_player_set_position(m_player, pos);
}

int CEvlc::GetVolume(){
	if (!m_instance || !m_media || !m_player) return -1;
	return libvlc_audio_get_volume(m_player);
}

int CEvlc::SetVolume(int volume){
	if (!m_instance || !m_media || !m_player) return -1;
	return libvlc_audio_set_volume(m_player,volume);
}

float CEvlc::getLength(){
	if (!m_instance || !m_media || !m_player) return -1.0f;
	libvlc_time_t tm = libvlc_media_player_get_length(m_player);
	float ret = tm / 1000.0f;
	return ret;
}

CVlcSize CEvlc::GetMediaInfo(){
	if (!m_instance || !m_media || !m_player) return CVlcSize(-1,-1);
	return CVlcSize(libvlc_video_get_width(m_player),libvlc_video_get_height(m_player));
}

std::string CEvlc::unicode2utf8(const std::wstring& strin){
	std::string str;
	//当不确定转换后的编码大小时，可先不填写目标串，返回值即为转换后的编码大小
	int length = ::WideCharToMultiByte(CP_UTF8, 0, strin.c_str(), strin.size(), NULL, 0, NULL, NULL);
	str.resize(length);
	::WideCharToMultiByte(CP_UTF8, 0, strin.c_str(), strin.size(), (LPSTR)str.c_str(), str.size(), NULL, NULL);
	return str;
}