#pragma once
#include "RMenumSet.h"

class CRMimage;

class CRMresourceManager
{
	SINGLETON(CRMresourceManager);

private:
	CRMresourceManager(void);
	~CRMresourceManager(void);
	
public:
	// ���丮 �ʱ�ȭ
	HRESULT						CreateFactory();
	IWICImagingFactory*			GetImageFactory() const { return m_pWICFactory; }

	HRESULT						CreateTexture();
	HRESULT						CreateTexture( const std::string& folderName );
	HRESULT						CreateTextureAlbum( const std::string& folderName );

	CRMimage*					GetTexture( const ObjectType& key ) { return m_TextureArray[key]; }
	
private:
	void								InitializeArray();
	void								InitializeAlbum();

	std::wstring						GetFilePath( const std::string& folderName, const std::string& resourceName ) const;

	IWICImagingFactory*					m_pWICFactory;
	std::array<CRMimage*, OBJECT_MAX>	m_TextureArray;
};

