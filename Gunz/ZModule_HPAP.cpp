#include "stdafx.h"
#include "ZModule_HPAP.h"
#include "ZGame.h"
#include "ZApplication.h"
#include "CheckReturnCallStack.h"

ZModule_HPAP::ZModule_HPAP() : m_LastAttacker(MUID(0,0)), m_bRealDamage_DebugRegister(false), m_fAccumulationDamage(0.f), m_bAccumulationDamage(false)
{
	float f = float(rand() % 1000); 
	f += float(rand() % 100) * 0.01f;
	m_fMask.Set_MakeCrc(f);

	m_bRealDamage.Set_MakeCrc(false);
	m_fHP.Set_MakeCrc(1000.f);
	m_fAP.Set_MakeCrc(1000.f);
	m_fMaxHP.Set_MakeCrc(100);
	m_fMaxAP.Set_MakeCrc(100);
}



ZModule_HPAP::~ZModule_HPAP()
{
}

float ZModule_HPAP::GetHP() 
{
	return  m_fHP.Ref() - GetMask();
}

float	ZModule_HPAP::GetAP() 
{
	return m_fAP.Ref() - GetMask();
}

void ZModule_HPAP::OnDamage(MUID uidAttacker,float fDamage, float fRatio)
{
	if (fDamage < 0) fDamage *= -1.f;

	m_LastAttacker = uidAttacker;

	if (uidAttacker.Low == ZGetGame()->m_pMyCharacter->GetUID().Low)
	{
		MCommand* pCmd = ZNewCmd(MC_PEER_DAMAGE_TAB);
		pCmd->AddParameter(new MCmdParamInt((int)fDamage));
		pCmd->AddParameter(new MCmdParamUID(uidAttacker));
		pCmd->AddParameter(new MCmdParamBool(false));
		ZPostCommand(pCmd);
	}

#ifndef _PUBLISH
	if (CheckQuestCheet() == true) return;
#endif
	if(!m_bRealDamage.Ref())	
		PROTECT_DEBUG_REGISTER(!m_bRealDamage_DebugRegister)
			return;

	// NPC의 난이도 조절계수때문에 넣음
	ZObject* pAttacker = ZGetObjectManager()->GetObject(uidAttacker);
	if ((pAttacker) && (!IsPlayerObject(pAttacker)))
	{
		ZActor* pActor = (ZActor*)pAttacker;
		fDamage = fDamage * (pActor->GetQL() * 0.2f + 1);
	}

	if(IsAccumulationDamage())
	{
		AccumulateDamage(fDamage);
#ifndef _PUBLISH	// 내부빌드에서 누적 대미지 정보 출력
		char szAccumulationDamagePrint[256];
		sprintf(szAccumulationDamagePrint, "피해 당한 누적대미지[%2.1f]", GetAccumulationDamage());
		ZChatOutput(MCOLOR(255, 200, 100), szAccumulationDamagePrint);
#endif
	}


	float fHPDamage = (float)((float)fDamage * fRatio);
	float fAPDamage = fDamage - fHPDamage;

	if ((GetAP() - fAPDamage) < 0)
	{
		fHPDamage += (fAPDamage - GetAP());
		fAPDamage -= (fAPDamage - GetAP());
	}

	int HPprev = GetHP();
	int APprev = GetAP();
	SetHP(GetHP() - fHPDamage);
	SetAP(GetAP() - fAPDamage);

	if (HPprev > GetHP() || APprev > GetHP()) {
		ZCharacter* pCharacter = ZGetGame()->m_pMyCharacter;
		if (uidAttacker.Low == ZGetGame()->m_pMyCharacter->GetLastAttacker().Low)
		{
			pCharacter->GetStatus().CheckCrc();
			pCharacter->GetStatus().Ref().nDamageTaken += fDamage;
			pCharacter->GetStatus().MakeCrc();
		}
	}
}


void ZModule_HPAP::InitStatus()
{
	m_LastAttacker = MUID(0,0);
}

bool ZModule_HPAP::CheckQuestCheet()
{
#ifdef _PUBLISH
	return false;
#endif

	// 퀘스트 테스트용 치트 체크
	if (IsMyCharacter((ZObject*)m_pContainer))
	{
		if ((ZIsLaunchDevelop()) && (ZGetGameClient()->GetServerMode() == MSM_CLAN))
		{
			if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
			{
				if (ZGetQuest()->GetCheet(ZQUEST_CHEET_GOD) == true) return true;
			}
		}
	}

	return false;
}

void ZModule_HPAP::ShiftFugitiveValues()
{
	m_fMask.ShiftHeapPos_CheckCrc();

	m_fMaxHP.ShiftHeapPos_CheckCrc();
	m_fMaxAP.ShiftHeapPos_CheckCrc();

	m_fHP.ShiftHeapPos_CheckCrc();
	m_fAP.ShiftHeapPos_CheckCrc();

	m_bRealDamage.ShiftHeapPos_CheckCrc();
}