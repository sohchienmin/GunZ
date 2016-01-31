#include "stdafx.h"
#include "ZWeaponScreenEffect.h"
#include "ZGame.h"
#include "ZMyInfo.h"
#include "ZMyItemList.h"

ZWeaponScreenEffect::ZWeaponScreenEffect()
{
	m_nWeaponType = MWT_NONE;
	m_pSniferPanel = NULL;
}

ZWeaponScreenEffect::~ZWeaponScreenEffect()
{

}
#include "ZConfiguration.h"
void ZWeaponScreenEffect::Draw(MDrawContext* pDC)
{

	switch (m_nWeaponType)
	{
	case MWT_SNIFER:
	{
		if (m_pSniferPanel)
		{
			MFont *pFont = ZGetCombatInterface()->GetGameFont();
			pFont = MFontManager::Get("FONTa10_O2Wht");
			pDC->SetFont(pFont);
			pDC->SetColor(MCOLOR(0xFFFFFFFF));

			char szZoom[123];
			sprintf(szZoom, "Zoomed Value : %d", ZGetConfiguration()->GetEtc()->n);
			TextRelative(pDC, 1.f / 800.f, 175.f / 600.f, szZoom);
			pDC->SetColor(MCOLOR(0xFFFFFFFF));
			char szMsg[123];
			if (ZGetCombatInterface()->IsPicked())
				sprintf(szMsg, "Enemy Name: %s", ZGetCombatInterface()->GetPickedInfo());
			else
				sprintf(szMsg, "Enemy was not found");
			TextRelative(pDC, 1.f / 800.f, 150.f / 600.f, szMsg);
			pDC->SetColor(MCOLOR(0xFFFFFFFF));
			if (GetAsyncKeyState(VK_UP))
			{
				static unsigned long int st_nLastTime = 0;
				unsigned long int nNowTime = timeGetTime();
#define UP		10

				if ((nNowTime - st_nLastTime) < UP)
				{
				}
				else
				{
					st_nLastTime = nNowTime;

					ZGetConfiguration()->GetEtc()->n = ZGetConfiguration()->GetEtc()->n + 5;
					if ((ZGetConfiguration()->GetEtc()->n >= 0) && (ZGetConfiguration()->GetEtc()->n <= 1500))
						g_fFOV = (70.0f / (180.0f + (float)ZGetConfiguration()->GetEtc()->n) * pi);
					else
						ZGetConfiguration()->GetEtc()->n = 1500;
				}

			}
			if (GetAsyncKeyState(VK_DOWN))
			{
				static unsigned long int st_nLastTime = 0;
				unsigned long int nNowTime = timeGetTime();
#define DOWN		10

				if ((nNowTime - st_nLastTime) < DOWN)
				{
				}
				else
				{
					st_nLastTime = nNowTime;

					ZGetConfiguration()->GetEtc()->n = ZGetConfiguration()->GetEtc()->n - 5;
					if ((ZGetConfiguration()->GetEtc()->n >= 0) && (ZGetConfiguration()->GetEtc()->n <= 1500))
						g_fFOV = (70.0f / (180.0f + (float)ZGetConfiguration()->GetEtc()->n) * pi);
					else
						ZGetConfiguration()->GetEtc()->n = 0;
				}
			}

			pDC->SetBitmap(m_pSniferPanel);
			pDC->Draw(0, 0, RGetScreenWidth(), RGetScreenHeight());
		}
	}
		break;
	}
}

void ZWeaponScreenEffect::InitWeapon(MMatchWeaponType nWeaponType)
{
	switch (nWeaponType)
	{
	case MWT_SNIFER:
	{
		ZGetConfiguration()->GetEtc()->n = 500;
		g_fFOV = (40.0f / 180.0f * pi);
	}
		break;
	}

}

void ZWeaponScreenEffect::FinalWeapon(MMatchWeaponType nWeaponType)
{
	switch (nWeaponType)
	{
	case MWT_SNIFER:
	{
		ZGetConfiguration()->GetEtc()->n = 0;
		g_fFOV = (70.0f / 180.0f * pi);
	}
		break;
	}
}

void ZWeaponScreenEffect::OnGadget(MMatchWeaponType nWeaponType)
{

	if (m_nWeaponType == nWeaponType) return;
	if (m_nWeaponType != MWT_NONE) OnGadgetOff();

	InitWeapon(nWeaponType);
	m_nWeaponType = nWeaponType;
}

void ZWeaponScreenEffect::OnGadgetOff()
{
	FinalWeapon(m_nWeaponType);
	m_nWeaponType = MWT_NONE;
}

void ZWeaponScreenEffect::Create()
{
	MMatchWeaponType nPrimaryWeaponType = MWT_NONE,
		nSecondaryWeaponType = MWT_NONE,
		nMeleeWeaponType = MWT_NONE;

	unsigned long int nItemID;
	nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_PRIMARY);
	MMatchItemDesc* pItemDesc = NULL;
	pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc)
	{
		nPrimaryWeaponType = pItemDesc->m_nWeaponType.Ref();
	}

	nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_SECONDARY);

	pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc)
	{
		nSecondaryWeaponType = pItemDesc->m_nWeaponType.Ref();
	}
	nItemID = ZGetMyInfo()->GetItemList()->GetEquipedItemID(MMCIP_MELEE);

	pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pItemDesc)
	{
		nMeleeWeaponType = pItemDesc->m_nWeaponType.Ref();
	}

	if ((nPrimaryWeaponType == MWT_SNIFER) || (nSecondaryWeaponType == MWT_SNIFER))
	{
		if (m_pSniferPanel) delete m_pSniferPanel;
		m_pSniferPanel = new MBitmapR2;
		((MBitmapR2*)m_pSniferPanel)->Create("sniperscope.png", RGetDevice(), "interface/custom/Crosshair/sniperscope.png");
	}
}

void ZWeaponScreenEffect::Destroy()
{
	return;

	if (m_pSniferPanel)
	{
		delete m_pSniferPanel;
		m_pSniferPanel = NULL;
	}
}