#include "stdafx.h"
#include "resource.h"
#include "RMmacro.h"
#include "RMdefine.h"
#include "RMmainLoop.h"
#include "RMObjectManager.h"
#include "RMresourceManager.h"
#include "RMrender.h"
#include "RMinput.h"
#include "RMchildNote.h"
#include "RMchildBGImage.h"
#include "RMchildShutter.h"
#include "RMsound.h"
#include "RMJudgeManager.h"
#include "RMchildEffectImage.h"
#include "RMlabel.h"
#include "RMvideoPlayer.h"
#include "RMxmlLoader.h"
#include "RMnoteManager.h"
#include "RMmusicSelectManager.h"
#include "RMresultManager.h"
#include "RMplayer1P.h"
#include "RMplayer2P.h"
#include "RMchildGauge.h"
#include "RMchildJudgeRing.h"

CRMmainLoop::CRMmainLoop(void):
	m_NowTime(0),
	m_PrevTime(0),
	m_ElapsedTime(0),
	m_FpsCheckTime(0),
	m_SceneType(SCENE_OPENING),
	m_Hwnd(NULL),
	m_MusicSelectIndex(0)
{
	m_Fps = ( 1000 / 60 ) + 1;
}

CRMmainLoop::~CRMmainLoop(void)
{
}

void CRMmainLoop::RunMessageLoop()
{
	MSG msg;
	UINT fps = 0;
	HRESULT hr = S_FALSE;

	ZeroMemory( &msg, sizeof(msg) ); //msg 초기화 함수
	//===================================================================
	// 음악 데이터를 불러온다.
	// 음악 데이터를 vector 형식으로 리스팅
	FindMusicData();

	//===================================================================
	// fmod 사용하기 fmodex.dll파일이 필요하다.
	hr = CRMsound::GetInstance()->CreateSound();
	if ( hr != S_OK )
	{
		MessageBox( NULL, ERROR_SOUND_INIT, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
		return;
	}

	//sound를 불러와서 묶어 놓고 있는 상태
	//동영상 플레이 이후 별로의 로딩화면이 없는 상태이기 때문에 미리 로딩하는 개념
	hr = CRMsound::GetInstance()->LoadSound( BGM_TITLE, SOUND_BG_TITLE );
	if ( hr != S_OK )
	{
		MessageBox( NULL, ERROR_SOUND_LOADING, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
		return;
	}

	//===================================================================
	// 동영상 출력 부분
	hr = CRMvideoPlayer::GetInstance()->CreateFactory();
	
	if ( hr == S_OK )
	{
		CRMvideoPlayer::GetInstance()->StartVideo();
	}
	else
	{
		hr = GoNextScene();
		// 동영상 재생을 위한 초기화에 실패 했을 경우 동영상 재생 패스
		if ( hr != S_OK )
		{
			MessageBox( NULL, ERROR_CHANGE_SCENE, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
			return;
		}
	}

	hr = CreateObject();
	if ( hr != S_OK )
	{
		MessageBox( NULL, ERROR_CREATE_RESOURCE, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
		return;
	}


	// 오브젝트 생성 부분을 리팩토링
	
	while ( true )
	{
		if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) ) // PeekMessage는 대기 없이 무한 루프 상태로 진행(non blocked function)
		{
			if ( msg.message == WM_QUIT )
			{
				CRMvideoPlayer::GetInstance()->DestoryFactory();
				return;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);	// Wndproc과 연결되어 있음
		}
		else
		{
			if ( m_SceneType == SCENE_OPENING )
			{
				CRMvideoPlayer::GetInstance()->RenderVideo();

				CRMinput::GetInstance()->UpdateKeyState();
				if ( CRMinput::GetInstance()->GetKeyStatusByKey( KEY_TABLE_P1_TARGET1 ) == KEY_STATUS_UP )
				{
					CRMvideoPlayer::GetInstance()->DestoryFactory();
					hr = GoNextScene();

					if ( hr != S_OK )
					{
						MessageBox( NULL, ERROR_CHANGE_SCENE, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
						return;
					}
				}
				continue;
			}
#ifdef _DEBUG
			m_NowTime = timeGetTime();

			if ( m_PrevTime == 0 )
			{
				m_PrevTime = m_NowTime;
			}

			// FPS 출력용 계산
			if( ( m_NowTime - m_FpsCheckTime ) > 1000 )
			{
			//	printConsole("FPS : %d \n", fps);

				CRMlabel*		testLabel = new CRMlabel();
				std::wstring	testString;
				testString.append(L"FPS : ");
				testString.append( std::to_wstring(fps) );

				testLabel->CreateLabel(LABEL_FPS, testString, LABEL_FONT_NORMAL, 15.0F );
				testLabel->SetRGBA( 1.0f, 1.0f, 1.0f, 1.f );
				testLabel->SetSceneType( SCENE_PLAY );
				testLabel->SetPosition( 20, 20 );

				m_FpsCheckTime = m_NowTime;
				fps = 0;
			}

			m_ElapsedTime = m_NowTime - m_PrevTime;

			if( m_ElapsedTime == m_Fps )
			{
#endif
				// 테스트 
				
				// 처리 해야 할 내부 로직들을 처리함
				// Update
				CRMobjectManager::GetInstance()->Update();

				CRMrender::GetInstance()->RenderInit();

				// 화면에 대한 처리를 진행
				// Render
				CRMobjectManager::GetInstance()->Render();
				CRMrender::GetInstance()->RenderEnd();
				m_PrevTime = m_NowTime;
#ifdef _DEBUG
				++fps;
			}
#endif // _DEBUG

			//////////////////////////////////////////////////////////////////////////
			// 소리 밀림 방지를 위해 FPS = 60에 맞추던 부분에서 빼옴
			//////////////////////////////////////////////////////////////////////////
			CRMinput::GetInstance()->UpdateKeyState();

			// test sound
			TestSound();
			
			// test Key
			hr = TestKeyboard();
			if ( hr != S_OK )
			{
				MessageBox( NULL, ERROR_CHANGE_SCENE, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
				return;
			}


			//////////////////////////////////////////////////////////////////////////
			// 씬 관리 부분 
			//////////////////////////////////////////////////////////////////////////
			
			//음악 리스트 보여주기 함수
			//현재 수정 중(모든 Label 통합 작업 중)
			if ( m_SceneType == SCENE_SELECT_MUSIC )
			{
				CRMmusicSelectManager::GetInstance()->ShowMusicList( m_MusicVector, m_MusicSelectIndex );
			}
			else if ( m_SceneType == SCENE_PLAY )
			{
				CRMnoteManager::GetInstance()->StartNote();
				CRMjudgeManager::GetInstance()->JudgeNote();

				// 이렇게 자주 해줄 필요는 없는데...
				if ( ( CRMplayer1P::GetInstance()->IsDead() && CRMplayer2P::GetInstance()->IsDead() ) || !CRMsound::GetInstance()->GetIsPlaying() )
				{
					CRMobjectManager::GetInstance()->RemoveAllNote();
					m_SceneType = SCENE_RESULT;
				}
			}
			else if ( m_SceneType == SCENE_RESULT )
			{
				CRMresultManager::GetInstance()->ShowResult();
			}

			

			//////////////////////////////////////////////////////////////////////////
			// 여기까지
			//////////////////////////////////////////////////////////////////////////

			if ( m_ElapsedTime > m_Fps )
			{
				m_PrevTime = m_NowTime;
			}
		}
	}
}


///////////////////////////////////////////
//windows handle을 활용한 파일 리스팅
//1. findFileData에 탐색을 원하는 항목을 핸들로 입력
//2. 해당 항목에 대해 파일 이름이 있는지 확인
//3. 해당 항목에 있으면 vector 자료형에 후미에 리스팅
//4. 더 이상 파일 이름이 없으면 종료
///////////////////////////////////////////
void CRMmainLoop::FindMusicData()
{
	WIN32_FIND_DATAA findFileData;
	HANDLE hFind = FindFirstFileA( MUSIC_FOLDER_SEARCH, &findFileData );

	if ( hFind == INVALID_HANDLE_VALUE )
	{
		CloseHandle(hFind);
		return;
	}

	do 
	{
		if ( (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
		{
			std::string folderName = findFileData.cFileName;
			if ( folderName.compare(".") != 0 && folderName.compare("..") != 0 )
			{
				HRESULT hr = S_FALSE;
				hr = CRMxmlLoader::GetInstance()->LoadMusicData( folderName );
				if ( hr != S_OK )
				{
					MessageBox( NULL, ERROR_LOAD_MUSIC_XML, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
					return;
				}
				m_MusicVector.push_back( folderName ); 
				//벡터에 자동으로 이름이 붙으면서 후미에 저장 되도록 함
			}
		}
	} while ( FindNextFileA(hFind, &findFileData) != 0);
	FindClose(hFind);

}


HRESULT CRMmainLoop::CreateMainLoopWindow()
{
	// HRESULT hr = S_FALSE;

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= CRMmainLoop::WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= HINST_THISCOMPONENT;
	wcex.hIcon			= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_OLDBOY));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= NULL; // 배경 색상 부분 NULL로 설정
	wcex.lpszMenuName	= NULL;	// 메뉴 생성 부분 NULL로 설정
	wcex.lpszClassName	= CLASS_NAME;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	RegisterClassEx(&wcex);

	int START_POSITION_X = 0;
	int START_POSITION_Y = 0;

	START_POSITION_X = GetSystemMetrics(SM_CXSCREEN);
	START_POSITION_Y = GetSystemMetrics(SM_CYSCREEN);

	START_POSITION_X = (START_POSITION_X - SCREEN_SIZE_X)/2;
	START_POSITION_Y = (START_POSITION_Y - SCREEN_SIZE_Y)/2;

	m_Hwnd = CreateWindow(wcex.lpszClassName, 
		GAME_NAME, 
		WS_POPUPWINDOW,
		START_POSITION_X,		// 하기 4줄이 화면 시작 좌표 의미 
		START_POSITION_Y,		//
		SCREEN_SIZE_X,	// 1024 + 16
		SCREEN_SIZE_Y,	// 668 + 32
		NULL, 
		NULL, 
		wcex.hInstance, 
		NULL);

	if ( !m_Hwnd )
	{
		return S_FALSE;
	}

	ShowWindow(m_Hwnd, SW_SHOWNORMAL);
	UpdateWindow(m_Hwnd);

	return S_OK;
}

//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  목적: 주 창의 메시지를 처리합니다.
//
//  WM_COMMAND	- 응용 프로그램 메뉴를 처리합니다.
//  WM_PAINT	- 주 창을 그립니다.
//  WM_DESTROY	- 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK CRMmainLoop::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch ( message )
	{
	case WM_CREATE:
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
//화면을 띄우기 위한 모든 자원을 생성하는 단계
//Group 1. Factory
//Group 2. renderTarget
//Group 3. Texture
//Group 4. 실 이미지 리소스 들
//Group 5. label
//////////////////////////////////////////////////////////////////////////

HRESULT CRMmainLoop::CreateObject()
{
	HRESULT hr = S_FALSE;

	// 이미지 리소스를 불러오려면 렌더가 필요함
	hr = CRMrender::GetInstance()->CreateFactory();
	if ( hr != S_OK )
	{
		MessageBox( NULL, ERROR_CREATE_RENDER, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
		return hr;
	}
	hr = CRMrender::GetInstance()->CreateRenderTarget();
	if ( hr != S_OK )
	{
		MessageBox( NULL, ERROR_CREATE_RENDER_TARGET, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
		return hr;
	}
	// 렌더를 메인루프의 생성자에 못 넣는 이유는?
	// 렌더 쪽에서 메인루프 싱글톤을 호출하므로 메모리 접근 오류 발생!

	// 이미지 리소스 파일 불러오기
	hr = CRMresourceManager::GetInstance()->CreateFactory();
	if ( hr != S_OK )
	{
		MessageBox( NULL, ERROR_CREATE_WIC_FACTORY, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
		return hr;
	}
	hr = CRMresourceManager::GetInstance()->CreateTexture();
	if ( hr != S_OK )
	{
		MessageBox( NULL, ERROR_CREATE_BG_IMAGE, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
		return hr;
	}

	/**********************************************************************************/
	// 화면 출력을 시험 하기 위해 임시로 추가 해 둠
	/**********************************************************************************/
	CRMobject*	testObject = new CRMchildBGImage();
	testObject->SetObjectType(OBJECT_BG_IMAGE_TITLE);
	testObject->SetPosition(0, 0);
	testObject->SetSceneType(SCENE_TITLE);
	CRMobjectManager::GetInstance()->AddObject(testObject, LAYER_BACKGROUND);

	testObject = new CRMchildBGImage();
	testObject->SetObjectType(OBJECT_BG_IMAGE_PLAY);
	testObject->SetPosition(0, 0);
	testObject->SetSceneType(SCENE_PLAY);
	CRMobjectManager::GetInstance()->AddObject(testObject, LAYER_BACKGROUND);
	
	testObject = new CRMchildBGImage();
	testObject->SetObjectType(OBJECT_BG_IMAGE_SELECT);
	testObject->SetPosition(0, 0);
	testObject->SetSceneType(SCENE_SELECT_MUSIC);
	CRMobjectManager::GetInstance()->AddObject(testObject, LAYER_BACKGROUND);
	
	testObject = new CRMchildBGImage();
	testObject->SetObjectType(OBJECT_ALBUM_IMAGE);
	testObject->SetPosition(500, 120);
	testObject->SetSceneType(SCENE_SELECT_MUSIC);
	CRMobjectManager::GetInstance()->AddObject(testObject, LAYER_SHUTTER);

	testObject = new CRMchildBGImage();
	testObject->SetObjectType(OBJECT_BG_IMAGE_RESULT);
	testObject->SetPosition(0, 0);
	testObject->SetSceneType(SCENE_RESULT);
	CRMobjectManager::GetInstance()->AddObject(testObject, LAYER_BACKGROUND);

	testObject = new CRMchildJudgeRing();
	testObject->SetObjectType(OBJECT_JUDGERING);
	testObject->SetPosition(382, 530);
	testObject->SetSceneType(SCENE_PLAY);
	CRMobjectManager::GetInstance()->AddObject(testObject, LAYER_JUDGERING);

	testObject = new CRMchildJudgeRing();
	testObject->SetObjectType(OBJECT_JUDGERING);
	testObject->SetPosition(895, 530);
	testObject->SetSceneType(SCENE_PLAY);
	CRMobjectManager::GetInstance()->AddObject(testObject, LAYER_JUDGERING);

	CRMchildGauge* gaugeObject = new CRMchildGauge();
	gaugeObject->SetObjectType(OBJECT_GAUGE_1P);
	gaugeObject->SetPosition(378, 650);
	gaugeObject->SetSceneType(SCENE_PLAY);
	gaugeObject->SetPlayer(PLAYER_ONE);
	CRMobjectManager::GetInstance()->AddObject(gaugeObject, LAYER_GAUGE_PLAYER1);

	gaugeObject = new CRMchildGauge();
	gaugeObject->SetObjectType(OBJECT_GAUGE_2P);
	gaugeObject->SetPosition(893, 650);
	gaugeObject->SetSceneType(SCENE_PLAY);
	gaugeObject->SetPlayer(PLAYER_TWO);
	CRMobjectManager::GetInstance()->AddObject(gaugeObject, LAYER_GAUGE_PLAYER2);

	for ( int i = 0 ; i < MAX_NOTE_IN_POOL ; ++i )
	{
		testObject = new CRMchildNote();
		testObject->SetObjectType(OBJECT_NOTE_NORMAL_1);
		testObject->SetPosition( DEFAULT_POSITION_X, DEFAULT_POSITION_Y );
		testObject->SetSceneType(SCENE_PLAY);
		CRMobjectManager::GetInstance()->AddObject(testObject, LAYER_MEMORY_POOL);
	}

	CRMchildShutter* shutterObject = new CRMchildShutter();
	shutterObject->SetObjectType(OBJECT_SHUTTER);
	shutterObject->SetPosition(0, -890);
	shutterObject->SetSceneType(SCENE_PLAY);
	shutterObject->SetPlayer(PLAYER_ONE);
	CRMobjectManager::GetInstance()->AddObject(shutterObject, LAYER_SHUTTER);

	shutterObject = new CRMchildShutter();
	shutterObject->SetObjectType(OBJECT_SHUTTER);
	shutterObject->SetPosition(515, -890);
	shutterObject->SetSceneType(SCENE_PLAY);
	shutterObject->SetPlayer(PLAYER_TWO);
	CRMobjectManager::GetInstance()->AddObject(shutterObject, LAYER_SHUTTER);

	for ( int i = 0 ; i < MAX_EFFECT ; ++ i )
	{
		testObject = new CRMchildEffectImage();
		testObject->SetObjectType(OBJECT_NOTE_HIT);
		testObject->SetPosition(DEFAULT_POSITION_X, DEFAULT_POSITION_Y);
		testObject->SetSceneType(SCENE_PLAY);
		CRMobjectManager::GetInstance()->AddObject(testObject, LAYER_NOTE_HIT);
	}

	//<<<< 여기까지 이미지 자원
	//>>>> 여기부터 Label 자원
	
	//1. 포지션 위치 값 확인하고
	//2. 고정 값 / 유동 값 적용
	//3. 유동 값은 각 해당 위치에서 업데이트 되도록 set함수 제작
	//4. visible 정리 꼭 할 것

	//////////////////////////////////////////////////////////////////////////
	//music select 관련 label
	//


	//////////////////////////////////////////////////////////////////////////
	//P1 관련 label
	//


	//////////////////////////////////////////////////////////////////////////
	//p2 관련 label
	//



	return hr;
}


//////////////////////////////////////////////////////////////////////////
//TestSound 함수는 배경음을 제외한 나머지 음에 대한 컨트롤을 담당하고 있는 함수
//각 플레이어의 키가 눌러 진 것을 확인해 이펙트 음 재생 
//
//중간 이하에는 테스트 노트 생성용 코드
//음원과 연결된 노트가 나올 경우 삭제 가능
//////////////////////////////////////////////////////////////////////////

void CRMmainLoop::TestSound()
{
	
	if ( m_SceneType != SCENE_PLAY )
	{
		return;
	}

	if ( CRMinput::GetInstance()->GetKeyStatusByKey( KEY_TABLE_P1_TARGET1 ) == KEY_STATUS_DOWN )
	{
		CRMsound::GetInstance()->PlayEffect( SOUND_NOTE_1 );
		return;
	}
	if ( CRMinput::GetInstance()->GetKeyStatusByKey( KEY_TABLE_P1_TARGET2 ) == KEY_STATUS_DOWN )
	{
		CRMsound::GetInstance()->PlayEffect( SOUND_NOTE_2 );
		return;
	}
	if ( CRMinput::GetInstance()->GetKeyStatusByKey( KEY_TABLE_P2_TARGET1 ) == KEY_STATUS_DOWN )
	{
		CRMsound::GetInstance()->PlayEffect( SOUND_NOTE_1 );
		return;
	}
	if ( CRMinput::GetInstance()->GetKeyStatusByKey( KEY_TABLE_P2_TARGET2 ) == KEY_STATUS_DOWN )
	{
		CRMsound::GetInstance()->PlayEffect( SOUND_NOTE_2 );
		return;
	}

	//이하는 테스트 노트 생성용 코드
	if ( CRMinput::GetInstance()->GetKeyStatusByKey( KEY_TABLE_P1_ATTACK ) == KEY_STATUS_DOWN )
	{
		int testRand1 = rand();
		if( testRand1%2 == 0 )
		{
			CRMjudgeManager::GetInstance()->StartNote( PLAYER_ONE , OBJECT_NOTE_NORMAL_1 );
		}
		else
		{
			CRMjudgeManager::GetInstance()->StartNote( PLAYER_ONE , OBJECT_NOTE_NORMAL_2 );
		}
		return;
	}
	if ( CRMinput::GetInstance()->GetKeyStatusByKey( KEY_TABLE_P2_ATTACK ) == KEY_STATUS_DOWN )
	{
		int testRand2 = rand();
		if( testRand2%2 == 0 )
		{
			CRMjudgeManager::GetInstance()->StartNote( PLAYER_TWO , OBJECT_NOTE_NORMAL_1 );
		}
		else
		{
			CRMjudgeManager::GetInstance()->StartNote( PLAYER_TWO , OBJECT_NOTE_NORMAL_2 );
		}
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//키보드를 통한 각 씬 이동 컨트롤 함수
//&& 이후 각 상황을 둬 현재 상황 부여
//음악 선택 화면에서 
//////////////////////////////////////////////////////////////////////////

HRESULT CRMmainLoop::TestKeyboard()
{
	HRESULT hr = S_OK;

	if ( ( CRMinput::GetInstance()->GetKeyStatusByKey( KEY_TABLE_P1_TARGET1 ) == KEY_STATUS_UP ) && m_SceneType == SCENE_TITLE )
	{
		hr = GoNextScene();
	}

	if ( ( CRMinput::GetInstance()->GetKeyStatusByKey( KEY_TABLE_P1_TARGET1 ) == KEY_STATUS_DOWN ) && m_SceneType == SCENE_SELECT_MUSIC )
	{
		m_PlayMusicName = m_MusicVector.at( m_MusicSelectIndex % m_MusicVector.size() );

		hr = GoNextScene();
	}

	if ( ( CRMinput::GetInstance()->GetKeyStatusByKey( KEY_TABLE_P2_ATTACK ) == KEY_STATUS_DOWN ) && m_SceneType == SCENE_SELECT_MUSIC )
	{
		++m_MusicSelectIndex %= m_MusicVector.size();

		m_PlayMusicName = m_MusicVector.at(m_MusicSelectIndex);

		hr = CRMresourceManager::GetInstance()->CreateTextureAlbum( m_PlayMusicName );

		if ( hr != S_OK )
		{
			MessageBox( NULL, ERROR_LOAD_IMAGE, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
			return hr;
		}

		hr = CRMsound::GetInstance()->LoadPlaySound( m_PlayMusicName );

		if ( hr != S_OK )
		{
			MessageBox( NULL, ERROR_LOAD_SOUND, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
			return hr;
		}

		CRMsound::GetInstance()->PlaySound( SOUND_BG_PLAY, true );
	}

	if ( ( CRMinput::GetInstance()->GetKeyStatusByKey( KEY_TABLE_LIST_UP ) == KEY_STATUS_DOWN ) && m_SceneType == SCENE_SELECT_MUSIC )
	{
		m_MusicSelectIndex += 7;
		--m_MusicSelectIndex %= m_MusicVector.size();

		m_PlayMusicName = m_MusicVector.at(m_MusicSelectIndex);

		hr = CRMresourceManager::GetInstance()->CreateTextureAlbum( m_PlayMusicName );

		if ( hr != S_OK )
		{
			MessageBox( NULL, ERROR_LOAD_IMAGE, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
			return hr;
		}

		hr = CRMsound::GetInstance()->LoadPlaySound( m_PlayMusicName );

		if ( hr != S_OK )
		{
			MessageBox( NULL, ERROR_LOAD_SOUND, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
			return hr;
		}

		CRMsound::GetInstance()->PlaySound( SOUND_BG_PLAY, true );
	}

	if ( ( CRMinput::GetInstance()->GetKeyStatusByKey( KEY_TABLE_P1_TARGET1 ) == KEY_STATUS_UP ) && m_SceneType == SCENE_RESULT )
	{
		hr = GoNextScene();
	}
	return hr;
}

//////////////////////////////////////////////////////////////////////////
//각 씬을 넘기기 위한 절대 함수
//특징: 씬을 되돌릴 수는 없음
//각 씬의 상태를 확인 후 다음 씬으로(기획 되었는 상태) 이동하도록 되어 있음
//////////////////////////////////////////////////////////////////////////

HRESULT CRMmainLoop::GoNextScene()
{
	HRESULT hr = S_FALSE;

	if ( m_SceneType == SCENE_OPENING )
	{
		m_SceneType = SCENE_TITLE;
		CRMsound::GetInstance()->PlaySound( SOUND_BG_TITLE );
		return S_OK;
	}

	if ( m_SceneType == SCENE_TITLE )
	{
		m_SceneType = SCENE_SELECT_MUSIC;
		
		m_PlayMusicName = m_MusicVector.at(m_MusicSelectIndex);

		hr = CRMresourceManager::GetInstance()->CreateTextureAlbum( m_PlayMusicName );

		if ( hr != S_OK )
		{
			MessageBox( NULL, ERROR_LOAD_IMAGE, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
			return hr;
		}

		hr = CRMsound::GetInstance()->LoadPlaySound( m_PlayMusicName );

		if ( hr != S_OK )
		{
			MessageBox( NULL, ERROR_LOAD_SOUND, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
			return hr;
		}

		CRMsound::GetInstance()->PlaySound( SOUND_BG_PLAY, true );

		return S_OK;
	}

	if ( m_SceneType == SCENE_SELECT_MUSIC )
	{
		hr = CRMresourceManager::GetInstance()->CreateTexture( m_PlayMusicName );
		if ( hr != S_OK )
		{
			MessageBox( NULL, ERROR_LOAD_IMAGE, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
			return hr;
		}

		hr = CRMsound::GetInstance()->LoadPlaySound( m_PlayMusicName );

		if ( hr != S_OK )
		{
			MessageBox( NULL, ERROR_LOAD_SOUND, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
			return hr;
		}

		hr = CRMxmlLoader::GetInstance()->LoadNoteData( m_PlayMusicName );

		if ( hr != S_OK )
		{
			MessageBox( NULL, ERROR_LOAD_MUSIC_XML, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
			return hr;
		}

		m_SceneType = SCENE_PLAY;
		CRMsound::GetInstance()->PlaySound( SOUND_BG_PLAY, false );

		CRMnoteManager::GetInstance()->Initialize();

		return S_OK;
	}


	if ( m_SceneType == SCENE_RESULT )
	{
		m_SceneType = SCENE_SELECT_MUSIC;
		CRMplayer1P::GetInstance()->Init();
		CRMplayer2P::GetInstance()->Init();

		printConsole("플레이어 초기화 1P : %d, 2P : %d \n", CRMplayer1P::GetInstance()->GetHP(), CRMplayer2P::GetInstance()->GetHP());

		CRMsound::GetInstance()->PlaySound( SOUND_BG_PLAY, true );

		hr = CRMresourceManager::GetInstance()->CreateTextureAlbum( m_PlayMusicName );

		if ( hr != S_OK )
		{
			MessageBox( NULL, ERROR_LOAD_IMAGE, ERROR_TITLE_NORMAL, MB_OK | MB_ICONSTOP );
			return hr;
		}

		return S_OK;
	}

	return S_FALSE;
}
