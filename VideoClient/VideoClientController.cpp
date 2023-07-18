#include "pch.h"
#include "VideoClientController.h"

CVideoClientController::CVideoClientController(){
	m_dlg.m_controller = this;
}

CVideoClientController::~CVideoClientController()
{
}

int CVideoClientController::Init(CWnd*& pWnd){
    pWnd = &m_dlg;
    return 0;
}

int CVideoClientController::Invoke(){
	INT_PTR nResponse = m_dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}
	return nResponse;
}

int CVideoClientController::SetMedia(const std::string& strUrl){
	return m_vlc.SetMedia(strUrl);
}

float CVideoClientController::VideoCtrl(EVLC_CMD cmd){
	switch (cmd)
	{
	case EVLC_PLAY:
		return (float)m_vlc.Play();
	case EVLC_PAUSE:
		return (float)m_vlc.Pause();
	case EVLC_STOP:
		return (float)m_vlc.Stop();
	case EVLC_GET_VOLUME:
		return (float)m_vlc.GetVolume();
	case EVLC_GET_POSITION:
		return m_vlc.GetPosition();
	case EVLC_GET_LENGTH:
		return m_vlc.getLength();
	default:
		break;
	}
	return -1.0f;
}

void CVideoClientController::SetPosition(float pos){
	m_vlc.SetPosition(pos);
}

int CVideoClientController::SetWnd(HWND hWnd){
	return m_vlc.SetHwnd(hWnd);
}

int CVideoClientController::SetVolume(int volume){
	return m_vlc.SetVolume(volume);
}

CVlcSize CVideoClientController::GetMediaInfo(){
	return m_vlc.GetMediaInfo();
}

std::string CVideoClientController::unicode2utf8(const std::wstring& strin){
	return m_vlc.unicode2utf8(strin);
}
