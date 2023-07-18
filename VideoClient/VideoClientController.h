#pragma once
#include"Evlc.h"
#include"VideoClientDlg.h"
enum EVLC_CMD{
	EVLC_PLAY,
	EVLC_PAUSE,
	EVLC_STOP,
	EVLC_GET_VOLUME,
	EVLC_GET_POSITION,
	EVLC_GET_LENGTH
};
class CVideoClientController
{
public:
	CVideoClientController();
	~CVideoClientController();
	int Init(CWnd*& pWnd);
	int Invoke();
	//���strUrl�������ģ���Ҫת��Ϊutf-8����
	int SetMedia(const std::string& strUrl);
	float VideoCtrl(EVLC_CMD cmd);//����-1.0��ʾ����
	void SetPosition(float pos);
	int SetWnd(HWND hWnd);
	int SetVolume(int volume);
	CVlcSize GetMediaInfo();
	std::string unicode2utf8(const std::wstring& strin);
protected:
	CEvlc m_vlc;
	CVideoClientDlg m_dlg;
};

