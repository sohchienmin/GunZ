#include "stdafx.h"

#include "ZApplication.h"
#include "ZCharacterSelectView.h"
#include "RealSpace2.h"
#include "ZInterfaceBackground.h"
#include "ZGameClient.h"
#include "ZPost.h"
#include "MListBox.h"
#include "ZCharacter.h"
#include "MListBox.h"
#include "MComboBox.h"
#include "MMatchObject.h"
#include "RShaderMgr.h"
#include "ZCharacterView.h"
#include "MDebug.h"
#include "MMatchGlobal.h"

#include "MLabel.h"
#include "MButton.h"
#include "MPicture.h"
#include "ZMyInfo.h"

#ifdef LOCALE_NHNUSA
#include "ZNHN_USA_Report.h"
#endif

extern bool Enable_Cloth;
static rvector force = rvector( 15, 0, 0 );
static float maxForce = 30;

ZSelectCharacterInfo ZCharacterSelectView::m_CharInfo[MAX_CHAR_COUNT];
int ZCharacterSelectView::m_nNumOfCharacter = 0;
int ZCharacterSelectView::m_nSelectedCharacter = -1;
char ZCharacterSelectView::m_szLastChar[MATCHOBJECT_NAME_LENGTH] = "";

#define FNAME_LASTCHAR	"model/lastchar.dat"

void ZCharacterSelectView::LoadLastChar()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	// last char 불러온다.
	FILE* fp = fopen( FNAME_LASTCHAR, "rt");
	char szName[256] = "";
	if (fp)
	{
		fscanf(fp, "%s", szName);
		fclose(fp);
		if (szName[0] != 0)
		{
			for (int i = 0; i < MAX_CHAR_COUNT; i++)
			{
				char szWidget[256];
				sprintf( szWidget, "CharSel_Name%d", i);
				MLabel* pLabel = (MLabel*)pResource->FindWidget( szWidget);

				if ( pLabel)
				{
					if ( !strcmp( pLabel->GetText(), szName))
						ZCharacterSelectView::SetSelectedCharacter(i);
				}
			}
		}
	}
}

void ZCharacterSelectView::SetLastChar(char* szName)
{ 
	strcpy(m_szLastChar, szName); 

	// 마지막 선택 캐릭 저장
	FILE* fp = fopen(FNAME_LASTCHAR, "wt");
	if (fp)
	{
		fputs(szName, fp);
		fclose(fp);
	}

	// 레지스트리도 같이 저장 - 파일을 레지스트리로 교체중..나중에 FNAME_LASTCHAR 빼자. 한달쯤 후에.. - bird
	ZGetApplication()->SetSystemValue("LastChar", szName);
}

void ZCharacterSelectView::SetSelectedCharacterFromLastChar()
{
	if (m_szLastChar[0] != 0)
	{
		for (int i = 0; i < MAX_CHAR_COUNT; i++)
		{
			if ( !strcmp( m_CharInfo[i].m_AccountCharInfo.szName, m_szLastChar))
			{
				m_nSelectedCharacter = i;
				return;
			}
		}
	}

	// 만약 없으면 제일 처음껄로 결정
	for (int i = 0; i < MAX_CHAR_COUNT; i++)
	{
		if (m_CharInfo[i].m_AccountCharInfo.szName[0] != 0)
		{
			m_nSelectedCharacter = i;
			return;
		}
	}

	m_nSelectedCharacter = -1;
}

ZCharacterSelectView::ZCharacterSelectView()
{
	m_pMesh = NULL;
	m_pVisualMesh = NULL;
	m_pBackground = NULL;
	m_bReserveSelectChar = false;
	m_nSelCharIndex = -1;
	m_nState = ZCSVS_SELECT;

#ifdef _FASTDEBUG
	return;
#endif


//	m_pMesh = new RMesh;
	m_pVisualMesh = new RVisualMesh;

	m_fCRot = 0.f;

	m_pShadow	= new ZShadow;
}

ZCharacterSelectView::~ZCharacterSelectView(void)
{
#ifdef _FASTDEBUG
	return;
#endif

	if (m_pVisualMesh) SAFE_DELETE(m_pVisualMesh);

	SAFE_DELETE(m_pShadow);
}

void ZCharacterSelectView::DrawCharacterLight(rvector& vCharPos)
{
	if ((m_pBackground == NULL) || (m_pBackground->GetChurchEnd() == NULL)) return;

	RLightList* pLightList = m_pBackground->GetChurchEnd()->GetObjectLightList();
	RLIGHT* pLight = 0;

	for( RLightList::iterator iter = pLightList->begin(); iter != pLightList->end(); ++iter )
	{
		if( iter != pLightList->end() )
		{
			pLight = (*iter);
			break;
		}
	}

	D3DLIGHT9 light;

	light.Type			= D3DLIGHT_POINT;
	light.Ambient.r		= 0.1f;
	light.Ambient.g		= 0.1f;
	light.Ambient.b		= 0.1f;
	light.Specular.r	= 1.0f;
	light.Specular.g	= 1.0f;
	light.Specular.b	= 1.0f;
	light.Attenuation0	= 0.05f; 
	light.Attenuation1	= 0.002f; 
	light.Attenuation2	= 0.0f; 

	if( pLight == 0 )
	{
		light.Range			= 600.f;
		light.Position		= vCharPos+rvector(-50,-100,250);
		light.Diffuse.r		= .8f;
		light.Diffuse.g		= .8f;
		light.Diffuse.b		= .8f;
	}
	else
	{
		light.Range			= pLight->fAttnEnd; //- pLight->fAttnStart;
		light.Position	.x	= pLight->Position.x;
		light.Position	.y	= pLight->Position.y;
		light.Position	.z	= pLight->Position.z;
		light.Diffuse.r		= pLight->Color.x * pLight->fIntensity;
		light.Diffuse.g		= pLight->Color.y * pLight->fIntensity;
		light.Diffuse.b		= pLight->Color.z * pLight->fIntensity;
	}

	m_pVisualMesh->SetLight(0,&light,false);

	memset( &light, 0, sizeof(D3DLIGHT9)	);
	light.Attenuation1 = 0.005f;

	m_pVisualMesh->SetLight(1,&light,true);

//	RGetDevice()->SetLight( 0, &light );
//	RGetDevice()->LightEnable( 0, TRUE );

//	if( RShaderMgr::mbUsingShader )
//	{
//		RGetShaderMgr()->setLight( 0, &light );
//		RGetShaderMgr()->LightEnable( 0, TRUE );
//		memset( &light, 0, sizeof(D3DLIGHT9) );
//		light.Attenuation1 = 0.005f;
//		RGetShaderMgr()->setLight( 1, &light );
//		RGetShaderMgr()->LightEnable( 1, TRUE );
//	}


	RGetShaderMgr()->setAmbient( 0x00cccccc );
	RGetDevice()->SetRenderState( D3DRS_LIGHTING, TRUE );
	RGetDevice()->SetRenderState( D3DRS_AMBIENT, 0x00cccccc );

}

void ZCharacterSelectView::Draw()
{
	if ( m_nState == ZCSVS_SELECT) 
	{
		if ( (m_nSelCharIndex < 0) || (m_nSelCharIndex > MAX_CHAR_COUNT)) return;
		if (!m_CharInfo[m_nSelCharIndex].m_bLoaded) return;
	}

	if (m_pVisualMesh == NULL) return;

	if( !Enable_Cloth && m_pVisualMesh->isChestClothMesh() )
	{
		m_pVisualMesh->DestroyCloth();
	}
	else if( m_pVisualMesh->isChestClothMesh() )
	{
		srand( timeGetTime());
 		int rint = rand() % 10;
		force.x += rint - 7;
		force.x = min(max( force.x, 5 ), maxForce * 0.3 );
		srand( timeGetTime());
		rint = rand() % (int)(maxForce*0.3);
		force.y += rint - 4;	
		force.y = min(max( force.y, 0 ), maxForce );
		force.z = -70;

		m_pVisualMesh->SetClothForce(force);
	}

	rmatrix World;
	rvector Pos, Dir, Up(0,0,1);
	Pos = m_pBackground->GetCharPos();
	Dir = m_pBackground->GetCharDir();

	static MButton* s_button_l;
	static MButton* s_button_r;

	if(s_button_l==NULL) {
		s_button_l = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("Charviewer_Rotate_L---");
	}

	if(s_button_r==NULL) {
		s_button_r = (MButton*)ZGetGameInterface()->GetIDLResource()->FindWidget("Charviewer_Rotate_R---");
	}

	if( s_button_l && s_button_l->IsButtonDown() ) {
		m_fCRot += 1.f;
	}

	if( s_button_r && s_button_r->IsButtonDown() ) {
		m_fCRot -= 1.f;
	}


	if( m_nState == ZCSVS_CREATE ) {
	
		rmatrix m = RGetRotZ(m_fCRot);
		Dir = Dir * m;
	}

	DWORD dw;
	RGetDevice()->GetRenderState(D3DRS_FOGENABLE,&dw);
	RGetDevice()->SetRenderState(D3DRS_FOGENABLE,FALSE);

	if(m_pVisualMesh!=NULL)
	{
		MakeWorldMatrix(&World, Pos, Dir, Up);

/*
		if(m_pBackground->GetChurchEnd())
		{
			m_pBackground->GetChurchEnd()->SetObjectLight(Pos);
		}
*/

		DrawCharacterLight(Pos);

		bool bGame = ZGetGame() ? true:false;

		m_pVisualMesh->SetClothValue(bGame,0.f); // cloth
		m_pVisualMesh->SetWorldMatrix(World);
		m_pVisualMesh->Frame();
		m_pVisualMesh->Render(true);

		AniFrameInfo* pInfo = m_pVisualMesh->GetFrameInfo(ani_mode_lower);

		if( pInfo->m_pAniSet )
		{
			if ((!strcmp(pInfo->m_pAniSet->GetName(), "login_intro")) &&
				(m_pVisualMesh->isOncePlayDone()))
			{
				m_pVisualMesh->SetAnimation("login_idle");
			}
		}

		if ((m_bReserveSelectChar) && (m_pVisualMesh->isOncePlayDone()))
		{
			ZPostSelectMyChar(ZGetGameClient()->GetPlayerUID(), m_nSelCharIndex);

			m_bReserveSelectChar = false;
		}

//		m_pVisualMesh->UpdateCloth();
//		m_pVisualMesh->RenderCloth();

	}
    
	RGetDevice()->SetRenderState(D3DRS_FOGENABLE,dw);
}


bool ZCharacterSelectView::SelectMyCharacter()
{
	if (m_nState != ZCSVS_SELECT) return false;
	if (m_bReserveSelectChar==true) return false;
	if (m_nSelCharIndex < 0) return false;
	if (!m_CharInfo[m_nSelectedCharacter].m_bLoaded) return false;


	char szWidgetName[256];
	sprintf( szWidgetName, "CharSel_Name%d", m_nSelCharIndex);

	MLabel* pLabel;
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidgetName);
	if ( pLabel)
	{
		char szName[ 256];
		strcpy( szName, pLabel->GetText());

		mlog( "Selected character name : %s (", szName);

		for ( int i = 0;  i < (int)strlen( szName);  i++)
		{
			mlog( "%02X ", szName[ i] & 0x00FF);
		}

		mlog( ")  (len = %d)\n", (int)strlen( szName));
	}
	
	if (m_pVisualMesh != NULL) {
		if(m_pVisualMesh->SetAnimation("login_walk")==false) {
			m_pVisualMesh->SetAnimation("login_idle");
		}
	}

	m_bReserveSelectChar = true;
	m_nSelCharIndex = m_nSelectedCharacter;
	m_nReservedSelectTime = timeGetTime();

#ifdef _FASTDEBUG
	if (m_bReserveSelectChar == true)
	{
		ZPostSelectMyChar(ZGetGameClient()->GetPlayerUID(), m_nSelCharIndex);
		m_bReserveSelectChar = false;

#ifdef LOCALE_NHNUSA
		GetNHNUSAReport().ReportChoiceCharacter();
#endif
	}
#endif

	return true;
}

void ZCharacterSelectView::SelectChar(int nSelectIndex)
{
	if (m_nState != ZCSVS_SELECT) return;

	m_nSelCharIndex = nSelectIndex;
	m_nSelectedCharacter = nSelectIndex;

	if( (nSelectIndex < 0) || (nSelectIndex > MAX_CHAR_COUNT))
	{
//		mlog("Index is not available...%d %s\n",__LINE__, __FILE__);
		return;
	}

	if (strlen(m_CharInfo[nSelectIndex].m_AccountCharInfo.szName) <= 0) return;
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	
	m_nSelCharIndex = nSelectIndex;
	UpdateInterface(nSelectIndex);

#ifdef _FASTDEBUG
	return;
#endif


	if (!m_CharInfo[nSelectIndex].m_bLoaded) return;

	if (m_CharInfo[nSelectIndex].m_CharInfo.nSex == 0)
	{
		m_pMesh = ZGetMeshMgr()->Get("heroman1");
	}
	else
	{
		m_pMesh = ZGetMeshMgr()->Get("herowoman1");
	}
	m_pVisualMesh->Create(m_pMesh);

	if (m_pVisualMesh != NULL)
	{
		if (strlen(m_CharInfo[nSelectIndex].m_AccountCharInfo.szName) > 0)
		{
			ZChangeCharParts(m_pVisualMesh, MMatchSex(m_CharInfo[nSelectIndex].m_CharInfo.nSex), 
				m_CharInfo[nSelectIndex].m_CharInfo.nHair, m_CharInfo[nSelectIndex].m_CharInfo.nFace, 
				m_CharInfo[nSelectIndex].m_CharInfo.nEquipedItemDesc);

			// 무기
			if( m_pVisualMesh ) {
				RWeaponMotionType type = eq_weapon_etc;
				unsigned long int nItemDescID = GetVisualWeaponID(
					m_CharInfo[nSelectIndex].m_CharInfo.nEquipedItemDesc[MMCIP_MELEE],
					m_CharInfo[nSelectIndex].m_CharInfo.nEquipedItemDesc[MMCIP_PRIMARY],
					m_CharInfo[nSelectIndex].m_CharInfo.nEquipedItemDesc[MMCIP_SECONDARY],
					m_CharInfo[nSelectIndex].m_CharInfo.nEquipedItemDesc[MMCIP_CUSTOM1],
					m_CharInfo[nSelectIndex].m_CharInfo.nEquipedItemDesc[MMCIP_CUSTOM2]);

				MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemDescID);
				RMesh* pMesh = NULL;

				if (pDesc) {
					pMesh = ZGetWeaponMeshMgr()->Get( pDesc->m_pMItemName->Ref().m_szMeshName );
				}

				if( pMesh ) {
					type = pMesh->GetMeshWeaponMotionType();

					CheckTeenVersionMesh(&pMesh);

					m_pVisualMesh->AddWeapon(type , pMesh);
					m_pVisualMesh->SelectWeaponMotion(type);
				}
				else {
					m_pVisualMesh->RemoveAllWeapon();
					m_pVisualMesh->SelectWeaponMotion(eq_weapon_etc);
				}
			}

			m_pVisualMesh->Stop();
			m_pVisualMesh->Play();

		}
	}


	if (m_pVisualMesh != NULL)
	{
		m_pVisualMesh->SetAnimation("login_intro");

		m_pVisualMesh->GetFrameInfo(ani_mode_lower)->m_nFrame = 0;
		m_pVisualMesh->GetFrameInfo(ani_mode_upper)->m_nFrame = 0;
	}

	if(Enable_Cloth)
		m_pVisualMesh->ChangeChestCloth(1.f,3);

}

void ZCharacterSelectView::SetState(ZSelectViewState nState)
{
	if (m_nState == nState) return;

	m_nState = nState;
}

void ZCharacterSelectView::OnChangedCharInfo(int sex,int index)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if(pResource==NULL)
		return;

	unsigned long int nItemID[MMCIP_END];
	memset(nItemID, 0, sizeof(nItemID));

	// 헤어

//	nItemID[MMCIP_HEAD] = g_InitialHair[nHairIndex][(int)(nSex)];
	nItemID[MMCIP_HEAD] = 0;

	// 코스츔
	nItemID[MMCIP_MELEE]		= g_InitialCostume[index][sex].nMeleeItemID;
	nItemID[MMCIP_PRIMARY]		= g_InitialCostume[index][sex].nPrimaryItemID;
	nItemID[MMCIP_SECONDARY]	= g_InitialCostume[index][sex].nSecondaryItemID;
	nItemID[MMCIP_CUSTOM1]		= g_InitialCostume[index][sex].nCustom1ItemID;
	nItemID[MMCIP_CUSTOM2]		= g_InitialCostume[index][sex].nCustom2ItemID;

	MLabel* pLabel = NULL;

	pLabel = (MLabel*)pResource->FindWidget("CC_MeleeLabel");
	if (pLabel != NULL)	{
		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID[MMCIP_MELEE]);
		if (pDesc != NULL)	pLabel->SetText(pDesc->m_pMItemName->Ref().m_szItemName);
		else				pLabel->SetText(" ");
	}

	pLabel = (MLabel*)pResource->FindWidget("CC_PrimaryLabel");
	if (pLabel != NULL)	{
		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID[MMCIP_PRIMARY]);
		if (pDesc != NULL)	pLabel->SetText(pDesc->m_pMItemName->Ref().m_szItemName);
		else				pLabel->SetText(" ");
	}

	pLabel = (MLabel*)pResource->FindWidget("CC_SecondaryLabel");
	if (pLabel != NULL)	{
		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID[MMCIP_SECONDARY]);
		if (pDesc != NULL)	pLabel->SetText(pDesc->m_pMItemName->Ref().m_szItemName);
		else				pLabel->SetText(" ");
	}

	pLabel = (MLabel*)pResource->FindWidget("CC_Item1Label");
	if (pLabel != NULL)	{
		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID[MMCIP_CUSTOM1]);
		if (pDesc != NULL)	pLabel->SetText(pDesc->m_pMItemName->Ref().m_szItemName);
		else				pLabel->SetText(" ");
	}

	pLabel = (MLabel*)pResource->FindWidget("CC_Item2Label");
	if (pLabel != NULL)	{
		MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID[MMCIP_CUSTOM2]);
		if (pDesc != NULL)	pLabel->SetText(pDesc->m_pMItemName->Ref().m_szItemName);
		else				pLabel->SetText(" ");
	}

}

void ZCharacterSelectView::OnChangedCharCostume()
{
	if (m_nState != ZCSVS_CREATE) return;
	if (m_pVisualMesh == NULL) return;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
//	MListBox* pListBox = (MListBox*)pResource->FindWidget("CS_CharList");
	MComboBox* pSexCB, *pHairCB, *pFaceCB, *pCostumeCB;
	pSexCB = (MComboBox*)pResource->FindWidget("CC_Sex");
	pHairCB = (MComboBox*)pResource->FindWidget("CC_Hair");
	pFaceCB = (MComboBox*)pResource->FindWidget("CC_Face");
	pCostumeCB = (MComboBox*)pResource->FindWidget("CC_Costume");

	if ((pSexCB != NULL) && (pHairCB!=NULL) && (pFaceCB!=NULL) && (pCostumeCB!=NULL) )
//		&&(pListBox != NULL) )
	{
		MMatchSex nSex = MMatchSex(pSexCB->GetSelIndex());

		if (nSex == MMS_MALE)
		{
			m_pMesh = ZGetMeshMgr()->Get("heroman1");
		}
		else
		{
			m_pMesh = ZGetMeshMgr()->Get("herowoman1");
		}
		m_pVisualMesh->Create(m_pMesh);

		int nCostumeIndex = pCostumeCB->GetSelIndex();
		int nHairIndex = pHairCB->GetSelIndex();
		int nFaceIndex = pFaceCB->GetSelIndex();

		unsigned long int nItemID[MMCIP_END];
		memset(nItemID, 0, sizeof(nItemID));

		// 헤어
//		nItemID[MMCIP_HEAD] = g_InitialHair[nHairIndex][(int)(nSex)];
		nItemID[MMCIP_HEAD] = 0;

		// 코스츔
		nItemID[MMCIP_MELEE] = g_InitialCostume[nCostumeIndex][(int)(nSex)].nMeleeItemID;
		nItemID[MMCIP_PRIMARY] = g_InitialCostume[nCostumeIndex][(int)(nSex)].nPrimaryItemID;
		nItemID[MMCIP_SECONDARY] = g_InitialCostume[nCostumeIndex][(int)(nSex)].nSecondaryItemID;
		nItemID[MMCIP_CUSTOM1] = g_InitialCostume[nCostumeIndex][(int)(nSex)].nCustom1ItemID;
		nItemID[MMCIP_CHEST] = g_InitialCostume[nCostumeIndex][(int)(nSex)].nChestItemID;
		nItemID[MMCIP_HANDS] = g_InitialCostume[nCostumeIndex][(int)(nSex)].nHandsItemID;
		nItemID[MMCIP_LEGS] = g_InitialCostume[nCostumeIndex][(int)(nSex)].nLegsItemID;
		nItemID[MMCIP_FEET] = g_InitialCostume[nCostumeIndex][(int)(nSex)].nFeetItemID;

		OnChangedCharInfo((int)nSex,nCostumeIndex);

		ZChangeCharParts(m_pVisualMesh, nSex, nHairIndex, nFaceIndex, nItemID);
		ZChangeCharWeaponMesh(m_pVisualMesh, nItemID[MMCIP_PRIMARY]);

		m_pVisualMesh->SetAnimation("login_idle");
	}

	if( Enable_Cloth )
		m_pVisualMesh->ChangeChestCloth(1.f,3);
}

void ZCharacterSelectView::UpdateInterface(int nSelIndex)
{
	if ((nSelIndex < 0) || (nSelIndex > 4) ||
	   (strlen(m_CharInfo[nSelIndex].m_AccountCharInfo.szName) <= 0)) return;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// ui 업데이트

	// 초기화
	char szName[256];
	MWidget* pWidget;
	MLabel* pLabel;

#ifdef _GZFGunz
	for ( int i = 0; i < MAX_CHAR_COUNT; i++)
	{
		sprintf( szName, "CharSel_Name%d", i);
		pLabel = (MLabel*)pResource->FindWidget( szName);
		if ( pLabel)
		{
			if (i == nSelIndex)
				pLabel->SetTextColor(MCOLOR(m_CharInfo[nSelIndex].m_AccountCharInfo.nRed, m_CharInfo[nSelIndex].m_AccountCharInfo.nGreen, m_CharInfo[nSelIndex].m_AccountCharInfo.nBlue));
			else
				pLabel->SetTextColor(MCOLOR(0xFF606060));
		}

		sprintf( szName, "CharSel_Level%d", i);
		pLabel = (MLabel*)pResource->FindWidget( szName);
		if ( pLabel)
		{
			if ( i == nSelIndex)
				pLabel->SetTextColor(MCOLOR(m_CharInfo[nSelIndex].m_AccountCharInfo.nRed, m_CharInfo[nSelIndex].m_AccountCharInfo.nGreen, m_CharInfo[nSelIndex].m_AccountCharInfo.nBlue));
			else
				pLabel->SetTextColor( MCOLOR(0xFF606060));		// 회색
		}

		sprintf( szName, "CharSel_ClanName%d", i);
		pLabel = (MLabel*)pResource->FindWidget( szName);
		if ( pLabel)
		{
			if ( i == nSelIndex)
				pLabel->SetTextColor(MCOLOR(m_CharInfo[nSelIndex].m_AccountCharInfo.nRed, m_CharInfo[nSelIndex].m_AccountCharInfo.nGreen, m_CharInfo[nSelIndex].m_AccountCharInfo.nBlue));
			else
				pLabel->SetTextColor( MCOLOR(0xFF606060));		// 회색
		}
	}
#else
	for ( int i = 0; i < MAX_CHAR_COUNT; i++)
	{
		sprintf( szName, "CharSel_Name%d", i);
		pLabel = (MLabel*)pResource->FindWidget( szName);
		if ( pLabel)
		{
			if ( i == nSelIndex)
				pLabel->SetTextColor( MCOLOR(0xFFFFFFFF));		// 흰색
			else
				pLabel->SetTextColor( MCOLOR(0xFF606060));		// 회색
		}

		sprintf( szName, "CharSel_Level%d", i);
		pLabel = (MLabel*)pResource->FindWidget( szName);
		if ( pLabel)
		{
			if ( i == nSelIndex)
				pLabel->SetTextColor( MCOLOR(0xFFFFFFFF));		// 흰색
			else
				pLabel->SetTextColor( MCOLOR(0xFF606060));		// 회색
		}

		sprintf( szName, "CharSel_ClanName%d", i);
		pLabel = (MLabel*)pResource->FindWidget( szName);
		if ( pLabel)
		{
			if ( i == nSelIndex)
				pLabel->SetTextColor( MCOLOR(0xFFFFFFFF));		// 흰색
			else
				pLabel->SetTextColor( MCOLOR(0xFF606060));		// 회색
		}
	}
#endif

	pWidget = (MWidget*)pResource->FindWidget("CS_SelectCharDefKey");
	pWidget->SetFocus();

}

bool ZCharacterSelectView::IsEmpty(int nIndex)
{
	if (strlen(m_CharInfo[nIndex].m_AccountCharInfo.szName) <= 0) return true;

	return false;
}

void ZCharacterSelectView::ClearCharInfo()
{
	memset(ZCharacterSelectView::m_CharInfo, 0, sizeof(ZSelectCharacterInfo) * MAX_CHAR_COUNT);
}

void ZCharacterSelectView::OnRestore()
{
	m_pVisualMesh->OnRestore();
}

void ZCharacterSelectView::OnInvalidate()
{
	m_pVisualMesh->OnInvalidate();
}


void ZCharacterSelectView::ClearInterfaces()
{
	// Initialize widgets
	MLabel* pLabel;
	MPicture* pPicture;
	for (int i = 0; i < MAX_CHAR_COUNT; i++)
	{
		char szWidgetName[256];

		sprintf( szWidgetName, "CharSel_Name%d", i);
		pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidgetName);
		if ( pLabel)
			pLabel->SetText( "");

		sprintf( szWidgetName, "CharSel_Level%d", i);
		pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidgetName);
		if ( pLabel)
			pLabel->SetText( "");

		sprintf( szWidgetName, "CharSel_ClanName%d", i);
		pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidgetName);
		if ( pLabel)
			pLabel->SetText( "");

		sprintf( szWidgetName, "CharSel_SelectBtn%d", i);
		pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidgetName);
		if ( pPicture)
			pPicture->Show( false);

		sprintf( szWidgetName, "CharSel_Selectbar%d", i);
		pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidgetName);
		if ( pPicture)
			pPicture->Show( false);
	}
}


void ZCharacterSelectView::OnReceivedAccountCharInfo(void* pCharListBlob)
{
	int nCharMaxLevel=0;

	m_nNumOfCharacter = 0;
	m_nSelectedCharacter = -1;

	ClearCharInfo();
	ClearInterfaces();

	int nCount = MGetBlobArrayCount(pCharListBlob);

	MButton* pButton;
	MPicture* pPicture;

	int nPosY = (int)( 390.0f * (RGetScreenHeight() / 600.0f));
	for (int i = 0; i < nCount; i++)
	{
		MTD_AccountCharInfo* pAccountCharInfo = (MTD_AccountCharInfo*)MGetBlobArrayElement(pCharListBlob, i);
		if ( (pAccountCharInfo->nCharNum >= 0) && (pAccountCharInfo->nCharNum < MAX_CHAR_COUNT))
		{
			int nIndex = pAccountCharInfo->nCharNum;
			memcpy(&ZCharacterSelectView::m_CharInfo[nIndex].m_AccountCharInfo, pAccountCharInfo, sizeof(MTD_AccountCharInfo));
			ZCharacterSelectView::m_CharInfo[nIndex].m_CharInfo.nHP = DEFAULT_CHAR_HP;
			ZCharacterSelectView::m_CharInfo[nIndex].m_CharInfo.nAP = DEFAULT_CHAR_AP;

			// 아이템 속성값 적용
			for (int k = 0; k < MMCIP_END; k++)
			{
				MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(ZCharacterSelectView::m_CharInfo[nIndex].m_CharInfo.nEquipedItemDesc[k]);
				if (pItemDesc)
				{
					// 지금은 HP와 AP만 적용중
					ZCharacterSelectView::m_CharInfo[nIndex].m_CharInfo.nHP += pItemDesc->m_nHP.Ref();
					ZCharacterSelectView::m_CharInfo[nIndex].m_CharInfo.nAP += pItemDesc->m_nAP.Ref();
				}
			}

			// Get first slot number
			if ( m_nSelectedCharacter == -1) m_nSelectedCharacter = nIndex;

			char szWidgetName[256];
			MRECT rect;
			sprintf( szWidgetName, "CharSel_Name%d", nIndex);

			MLabel* pLabel;
			pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidgetName);
			if ( pLabel)
			{
				pLabel->SetText( pAccountCharInfo->szName);
#ifdef _GZFGunz
				pLabel->SetTextColor(MCOLOR(pAccountCharInfo->nRed, pAccountCharInfo->nGreen, pAccountCharInfo->nBlue));
#endif
				rect = pLabel->GetRect();
				rect.y = nPosY;
				pLabel->SetBounds( rect);
			}

			sprintf( szWidgetName, "CharSel_Level%d", nIndex);
			pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidgetName);
			if ( pLabel)
			{
				char szString[256];
				sprintf( szString, "%d %s", pAccountCharInfo->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER));
				pLabel->SetText( szString);
#ifdef _GZFGunz
				pLabel->SetTextColor(MCOLOR(pAccountCharInfo->nRed, pAccountCharInfo->nGreen, pAccountCharInfo->nBlue));
#endif
				rect = pLabel->GetRect();
				rect.y = nPosY;
				pLabel->SetBounds( rect);
			}

			sprintf( szWidgetName, "CharSel_ClanName%d", nIndex);
			pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidgetName);
			if ( pLabel)
			{
				/* 클랜정보는 뺐습니다. - bird
				char szString[256];
				if ( strcmp( pAccountCharInfo->szClanName, "") == 0)
					strcpy( szString, "클랜 : ---");
				else
					sprintf( szString, "클랜 : %s", pAccountCharInfo->szClanName);
				pLabel->SetText( szString);
				*/
				pLabel->SetText("");

				rect = pLabel->GetRect();
				rect.y = nPosY + (int)( 22.0f * (RGetScreenHeight() / 600.0f));
				pLabel->SetBounds( rect);
			}

			sprintf( szWidgetName, "CharSel_Selectbar%d", nIndex);
			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidgetName);
			if ( pPicture)
			{
				pPicture->Show( true);
				rect = pPicture->GetRect();
				rect.y = nPosY;
				pPicture->SetBounds( rect);
			}


			sprintf( szWidgetName, "CharSel_SelectBtn%d", nIndex);
			pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidgetName);
			if ( pButton)
			{
				pButton->Show( true);
				rect = pButton->GetRect();
				rect.y = nPosY;
				pButton->SetBounds( rect);
			}

			m_nNumOfCharacter++;

			if (nCharMaxLevel < pAccountCharInfo->nLevel)
				nCharMaxLevel = pAccountCharInfo->nLevel;

			nPosY += (int)( 34.0f * (RGetScreenHeight() / 600.0f));
		}
	}

	pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CS_SelectChar");
	if ( pButton)
		pButton->Enable( (m_nNumOfCharacter) ? true : false);
	pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CS_DeleteChar");
	if ( pButton)
		pButton->Enable( (m_nNumOfCharacter) ? true : false);
	pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CS_CreateChar");
	if ( pButton)
		pButton->Enable( (m_nNumOfCharacter == MAX_CHAR_COUNT) ? false : true);


	// 내가 정말 뉴비인지 체크
	if (nCharMaxLevel <= 1) ZGetMyInfo()->SetNewbie(true);
}


void ZCharacterSelectView::OnReceivedCharInfo(int nCharNum, void* pCharInfoBlob)
{
	int nCount = MGetBlobArrayCount(pCharInfoBlob);
	if (nCount != 1) return;
	if ((nCharNum < 0) || (nCharNum > MAX_CHAR_COUNT)) return;

	MTD_CharInfo* pCharInfo = (MTD_CharInfo*)MGetBlobArrayElement(pCharInfoBlob, 0);
	memcpy(&ZCharacterSelectView::m_CharInfo[nCharNum].m_CharInfo, pCharInfo, sizeof(MTD_CharInfo));

	m_CharInfo[nCharNum].m_bLoaded = true;
}