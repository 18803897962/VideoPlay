#include "pch.h"
#include "Evlc.h"

CEvlc::CEvlc(){
	m_instance = libvlc_new(0, NULL);
	m_media = NULL;
	m_player = NULL;
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
		libvlc_release(m_instance);
	}
}

int CEvlc::SetMedia(const std::string& strUrl){
	if (m_instance == NULL) return -1;
	m_media = libvlc_media_new_location(m_instance,strUrl.c_str());
	if (!m_media) return -2;
	m_player = libvlc_media_player_new_from_media(m_media);
	if (!m_player) return -3;
	return 0;
}

int CEvlc::SetHwnd(HWND hWnd){
	if (!m_instance || !m_media || !m_player) return -1;
	libvlc_media_player_set_hwnd(m_player,hWnd);
	return 0;
}

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

CVlcSize CEvlc::GetMediaInfo(){
	if (!m_instance || !m_media || !m_player) return CVlcSize(-1,-1);
	return CVlcSize(libvlc_video_get_width(m_player),libvlc_video_get_height(m_player));
}
