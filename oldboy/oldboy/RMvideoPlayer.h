#pragma once
#include "LibraryBandi/BandiVideoLibrary.h"
#include "LibraryBandi/BandiVideoTexture_DX9.h"

class CBandiVideoDevice;
class CBandiVideoTexture;

class CRMvideoPlayer
{
	SINGLETON(CRMvideoPlayer);

private:
	CRMvideoPlayer(void);
	~CRMvideoPlayer(void);

public:
	HRESULT		CreateFactory();
	void		DestoryFactory();

	void		StartVideo();
	void		RenderVideo();

private:
	CBandiVideoLibrary		m_BandiVideoLibrary;
	CBandiVideoDevice*		m_BandiVideoDevice;
	CBandiVideoTexture*		m_BandiVideoTexture;
	BVL_VIDEO_INFO			m_BandiVideoLibraryVideoInfo;
	HWND					m_Hwnd;

	int						m_Count;
	BVL_STATUS				m_Status;
	INT						m_Pitch;
	BYTE*					m_Buffer;
	BVL_FRAME				m_Frame;

};