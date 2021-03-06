#include "stdafx.h"
#include "RMmacro.h"
#include "RMchildGauge.h"
#include "RMmainLoop.h"
#include "RMchildShutter.h"
#include "RMobjectManager.h"
#include "RMdefine.h"
#include "RMplayer1P.h"

CRMchildGauge::CRMchildGauge(void)
{
}


CRMchildGauge::~CRMchildGauge(void)
{
}

void CRMchildGauge::Update()
{
	SetVisibleByScene();

	//////////////////////////////////////////////////////////////////////////
	//각 플레이어별 게이지 계산 방안
	//1. 셔터의 높이를 역산 해 역산 내용을 저장
	//2. 게이지의 높이를 셔터의 높이 확인(셔터 하단 값 역산)
	//3. 셔터 높이 와 게이지 높이 비교 후 게이지가 낮은 경우 게이지 상승 연산
	//4. 게이지의 높이가 셔터 높이 보다 길거나 같은 경우 -> 강제로 끌어내림
	//4-1. 셔터에서 신경 안쓰게 할 것
	//////////////////////////////////////////////////////////////////////////

	float nowShutterPosition = 0.0f;

	switch (m_playerNumber)
	{
	case PLAYER_NONE:
		break;
	case PLAYER_ONE:
		nowShutterPosition = SHUTTER_START_POSITION_Y + (PLAYER_HP_MAX - (float) CRMplayer1P::GetInstance()->GetHP() ) * -((float) SHUTTER_START_POSITION_Y / 10) + 668;
		break;
	case PLAYER_TWO:
		nowShutterPosition = SHUTTER_START_POSITION_Y + (PLAYER_HP_MAX - (float) CRMplayer1P::GetInstance()->GetHP() ) * -((float) SHUTTER_START_POSITION_Y / 10) + 668;
		break;
	case PLAYER_MAX:
		break;
	default:
		break;
	}


	if ( m_PositionY <= nowShutterPosition)
	{
		switch (m_playerNumber)
		{
		case PLAYER_ONE:
			
			break;
		case PLAYER_TWO:
			
			break;
		default:
			break;
		}
	}
}