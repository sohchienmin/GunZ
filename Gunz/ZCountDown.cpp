#include "stdafx.h"
#include "ZCountDown.h"
#include "ZGameInterface.h"
#include "ZApplication.h"

void OnTimer_CountDown(void* pParam)
{
	ZCOUNTDOWN *pCountDown = (ZCOUNTDOWN*)pParam;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pTargetWidget = pResource->FindWidget(pCountDown->szTargetWidget);
	if(!pTargetWidget || !pTargetWidget->IsVisible()) return;	// 없거나 숨겨졌으면 카운트를 중지한다.

	if(pCountDown->nSeconds>0) {

		// 라벨이 있으면 갱신한다
		if(pCountDown->szLabelWidget!=NULL)
		{
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MWidget* pWidget = pResource->FindWidget(pCountDown->szLabelWidget);
			if(pWidget)
			{
				char buffer[256];
				sprintf(buffer,"%d",pCountDown->nSeconds);
				pWidget->SetText(buffer);		
			}
		}

		pCountDown->nSeconds--;
		ZApplication::GetTimer()->SetTimerEvent(1000, &OnTimer_CountDown, pCountDown, true);
		return;
	}

	// 0이 되었으므로 대상 위젯을 숨기고, 
	pTargetWidget->Show(false);	

	// 콜백이 있으면 실행한다
	if(pCountDown->pCallBack)
		pCountDown->pCallBack();
}

void OnTimer_CountDown_Challenge(void* pParam)
{
	ZCOUNTDOWN *pCountDown = (ZCOUNTDOWN*)pParam;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pTargetWidget = pResource->FindWidget(pCountDown->szTargetWidget);
	if(!pTargetWidget || !pTargetWidget->IsVisible()) {
		MWidget* pWidget = pResource->FindWidget(pCountDown->szLabelWidget);
		if(pWidget) {
			pWidget->SetText("");
		}
		delete pCountDown;
		return;
	}

	if(pCountDown->nSeconds>0) {
		if(pCountDown->szLabelWidget!=NULL)
		{
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MWidget* pWidget = pResource->FindWidget(pCountDown->szLabelWidget);
			if(pWidget)
			{
				int total, seconds, hours, minutes;
				total = pCountDown->nSeconds;
				if (total <= 0) {
					pWidget->SetText("");
					return;
				}
				minutes = total / 60;
				seconds = total % 60;
				hours = minutes / 60;
				minutes = minutes % 60;
				char buffer[256];
				sprintf(buffer,"Time left to complete challenge: %d : %d : %d",hours,minutes,seconds);
				pWidget->SetText(buffer);
			}
		}

		pCountDown->nSeconds--;
		ZApplication::GetTimer()->SetTimerEvent(1000, &OnTimer_CountDown_Challenge, pCountDown, true);
		return;
	}
	if(pCountDown->pCallBack)
		pCountDown->pCallBack();
}