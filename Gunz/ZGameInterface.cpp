﻿#include "stdafx.h"

#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZPost.h"
#include "UPnP.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZConfiguration.h"
#include "FileInfo.h"
#include "ZInterfaceItem.h"
#include "MPicture.h"
#include "ZInterfaceListener.h"
#include "ZEffectSmoke.h"
#include "ZEffectLightTracer.h"
#include "MProfiler.h"
//#include "MActionKey.h"
#include "ZActionDef.h"
#include "MSlider.h"
#include "ZMsgBox.h"
#include "MDebug.h"
#include "MMatchTransDataType.h"
#include "MBlobArray.h"
#include "MListBox.h"
#include "MTextArea.h"
#include "MTabCtrl.h"
#include "MComboBox.h"
#include "ZInterfaceBackground.h"
#include "ZCharacterSelectView.h"
#include "ZCharacterViewList.h"
#include "ZCharacterView.h"
#include "MMatchStage.h"
#include "ZScreenEffectManager.h"
#include "RShaderMgr.h"
#include "ZShopEquipInterface.h"
#include "ZShopEquipListbox.h"
#include "ZMyItemList.h"
#include "ZMyInfo.h"
#include "ZStageSetting.h"
#include "RealSoundEffect.h"
#include "ZInitialLoading.h"
#include "RShaderMgr.h"
#include "zeffectflashbang.h"
#include "MToolTip.h"
#include "ZRoomListbox.h"
#include "ZPlayerListBox.h"
#include "MMatchNotify.h"
#include "ZMapListBox.h"
#include "ZDuelTournamentRankingListBox.h"
#include "ZToolTip.h"
#include "ZCanvas.h"
#include "ZCrossHair.h"
#include "ZPlayerMenu.h"
#include "ZItemMenu.h"
#include "MPanel.h"
#include "ZNetRepository.h"
#include "ZStencilLight.h"
#include "MUtil.h"
#include "ZMap.h"
#include "ZBmNumLabel.h"
#include "ZItemSlotView.h"
#include "ZMapDesc.h"
#include "MStringTable.h"

#include "ZReplay.h"
#include "MFileDialog.h"
#include "ZServerView.h"
#include "ZLocale.h"

#include "ZLocatorList.h"
#include "ZSecurity.h"
#include "ZInput.h"
#include "ZActionKey.h"
#include "ZMonsterBookInterface.h"
#include "ZGameInput.h"
#include "ZOptionInterface.h"
#include "ZNHN_USA_Report.h"

#include "ZPlayerManager.h"
#include "ZShop.h"
#include "ZGambleItemDefine.h"

#ifdef _XTRAP
#include "./XTrap/Xtrap_C_Interface.h"				// Update sgk 0702
#endif

#ifdef LOCALE_NHNUSA
#include "ZNHN_USA.h"
#endif

#ifdef _GAMEGUARD
#include "ZGameguard.h"
#endif

extern MCommandLogFrame* m_pLogFrame;

static int g_debug_tex_update_cnt;
bool ZGameInterface::m_bSkipGlobalEvent = false;

bool ZGameInterface::m_sbRemainClientConnectionForResetApp = false;
ZGameClient* ZGameInterface::m_spGameClient = NULL;

void ZChangeGameState(GunzState state)
{
	PostMessage(g_hWnd, WM_CHANGE_GAMESTATE, int(state), 0);
}


void ZEmptyBitmap()
{
	MBitmapManager::Destroy();
	MBitmapManager::DestroyAniBitmap();
}

void ZGameInterface::LoadBitmaps(const char* szDir, const char* szSubDir, ZLoadingProgress *pLoadingProgress)
{
	// szSubDir(´ëÃ¼°æ·Î)¿¡¼­ ¸ÕÀú ÆÄÀÏÀ» Ã£°í ¾øÀ¸¸é szDir¿¡¼­ ·ÎµùÇÑ´Ù
	// (´ëÃ¼°æ·Î¿¡´Â »ç¿ëÀÚ°¡ ¿É¼Ç¿¡¼­ ¼±ÅÃÇÑ ¾ð¾î¿ë ºñÆ®¸ÊÀ» ³Ö¾îµÐ´Ù)
	mlog("start log bipmap\n");

	const char *loadExts[] = { ".png", ".bmp", ".tga" };

#define EXT_LEN 4

	MZFileSystem *pfs=ZGetFileSystem();

	int nDirLen = (int)strlen(szDir);

	int nTotalCount = 0;
	// °³¼ö¸¦ ¼¼±âÀ§ÇÑ copy
	for(int i=0; i<pfs->GetFileCount(); i++){
		const char* szFileName = pfs->GetFileName(i);
		const MZFILEDESC* desc = pfs->GetFileDesc(i);
		if(!desc) continue;
		int nLen = (int)strlen(szFileName);

		for(int j=0;j<sizeof(loadExts)/sizeof(loadExts[0]);j++) {
			// È®ÀåÀÚ°¡ ¸ÂÀ¸¸é..
			if( nLen>EXT_LEN && stricmp(szFileName+nLen-EXT_LEN, loadExts[j])==0 )
			{
				bool bAddDirToAliasName = false;
				bool bMatch = false;
				// °æ·Î°¡ ¸Â¾Æ¾ßÇÑ´Ù
				if(nDirLen==0 || strnicmp(desc->m_szFileName,szDir,nDirLen)==0)
					nTotalCount++;
				// custom crosshair
				if(strnicmp(desc->m_szFileName,PATH_CUSTOM_CROSSHAIR,strlen(PATH_CUSTOM_CROSSHAIR))==0)
					nTotalCount++;
			}
		}
	} // °³¼ö¸¦ ¼¼±âÀ§ÇÑ Ä«ÇÇ

	int nCount = 0;
	for(int i=0; i<pfs->GetFileCount(); i++){

		const char* szFileName = pfs->GetFileName(i);
		const MZFILEDESC* desc = pfs->GetFileDesc(i);
		const MZFILEDESC* subDesc = NULL;
		int nLen = (int)strlen(szFileName);
		const char* szTargetFile = NULL;

		for(int j=0;j<sizeof(loadExts)/sizeof(loadExts[0]);j++) {
			// È®ÀåÀÚ°¡ ¸ÂÀ¸¸é..
			if( nLen>EXT_LEN && stricmp(szFileName+nLen-EXT_LEN, loadExts[j])==0 )
			{
				bool bAddDirToAliasName = false;
				bool bMatch = false;
				// °æ·Î°¡ ¸Â¾Æ¾ßÇÑ´Ù
				if(nDirLen==0 || strnicmp(desc->m_szFileName,szDir,nDirLen)==0)
					bMatch = true;
				// custom crosshair
				if(strnicmp(desc->m_szFileName,PATH_CUSTOM_CROSSHAIR,strlen(PATH_CUSTOM_CROSSHAIR))==0)
				{
					bMatch = true;
					bAddDirToAliasName = true;
				}

				if(bMatch) {
					nCount++;
					if(pLoadingProgress && nCount%10==0)
						pLoadingProgress->UpdateAndDraw((float)nCount/(float)nTotalCount);


					char aliasname[256];
					char drive[_MAX_DRIVE],dir[_MAX_DIR],fname[_MAX_FNAME],ext[_MAX_EXT];
					_splitpath(szFileName,drive,dir,fname,ext);

					if (!bAddDirToAliasName) sprintf(aliasname,"%s%s",fname,ext);
					else sprintf(aliasname, "%s%s%s", dir, fname,ext);

#ifdef _PUBLISH
					MZFile::SetReadMode( MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE );
#endif
					// ´ëÃ¼ °æ·Î¿¡ °°Àº ÀÌ¸§ÀÇ ÆÄÀÏÀÌ Á¸ÀçÇÏ¸é ±×°ÍÀ» ·Îµù. ¾øÀ¸¸é µðÆúÆ® °æ·Î¿¡¼­ ·Îµù
					std::string strSubFile = desc->m_szFileName;
					strSubFile.replace(0, nDirLen, szSubDir);
					subDesc = pfs->GetFileDesc(strSubFile.c_str());
					if (subDesc)
						szTargetFile = subDesc->m_szFileName;
					else
						szTargetFile = desc->m_szFileName;

					//szFileName = desc->m_szFileName;
					MBitmapR2* pBitmap = new MBitmapR2;
					if(pBitmap->Create(aliasname, RGetDevice(), szTargetFile)==true)
						MBitmapManager::Add(pBitmap);
					else
						delete pBitmap;

#ifdef _PUBLISH
					MZFile::SetReadMode( MZIPREADFLAG_MRS2 );
#endif

				}
			}
		}
	}

	mlog("end of load bitmaps2\n");
	//ZLoadBitmap(PATH_CUSTOM_CROSSHAIR, ".png", true);

}

/*
void InitHotBar(MWidget* pHotBar)
{
if(pHotBar==NULL) return;

#define HOTBAR_BTN_COUNT	10
#define HOTBAR_BTN_WIDTH	32
#define HOTBAR_SPINBTN_WIDTH	16
MRECT HotBarClientRect = pHotBar->GetClientRect();
MButton* pNew = new MButton("<", pHotBar, pHotBar);
pNew->SetBounds(HotBarClientRect.x, HotBarClientRect.y, HOTBAR_SPINBTN_WIDTH, HOTBAR_SPINBTN_WIDTH);
pNew = new MButton(">", pHotBar, pHotBar);
pNew->SetBounds(HotBarClientRect.x, HotBarClientRect.y+HOTBAR_SPINBTN_WIDTH, HOTBAR_SPINBTN_WIDTH, HOTBAR_SPINBTN_WIDTH);

for(int i=0; i<HOTBAR_BTN_COUNT; i++){
MButton* pNew = new MHotBarButton(NULL, pHotBar, &g_HotBarButtonListener);
pNew->SetBounds(HotBarClientRect.x+HOTBAR_SPINBTN_WIDTH+1+i*(HOTBAR_BTN_WIDTH+1), HotBarClientRect.y, HOTBAR_BTN_WIDTH, HOTBAR_BTN_WIDTH);
}
}
*/

void AddListItem(MListBox* pList, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	class MDragableListItem : public MDefaultListItem{
		char m_szDragItemString[256];
	public:
		MDragableListItem(MBitmap* pBitmap, const char* szText, const char* szItemString)
			: MDefaultListItem(pBitmap, szText){
				if(szItemString!=NULL) strcpy(m_szDragItemString, szItemString);
				else m_szDragItemString[0] = 0;
			}
			virtual bool GetDragItem(MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString){
				*ppDragBitmap = GetBitmap(0);
				if(GetString(1)!=NULL) strcpy(szDragString, GetString(1));
				else szDragString[0] = 0;
				strcpy(szDragItemString, m_szDragItemString);
				return true;
			};
	};
	MDefaultListItem* pNew = new MDragableListItem(pBitmap, szString, szItemString);
	pList->Add(pNew);
}

bool InitSkillList(MWidget* pWidget)
{
	if(pWidget==NULL) return false;

	if(strcmp(pWidget->GetClassName(), MINT_LISTBOX)!=0) return false;
	MListBox* pList = (MListBox*)pWidget;

	pList->SetItemHeight(32);
	pList->SetVisibleHeader(false);

	pList->AddField("Icon", 32);
	pList->AddField("Name", 600);
	AddListItem(pList, MBitmapManager::Get("skill000.png"), "Fire-Ball", "Object.Skill $player $target 0");
	AddListItem(pList, MBitmapManager::Get("skill001.png"), "Bull-Shit", "Object.Skill $player $target 1");
	return true;
}

bool InitItemList(MWidget* pWidget)
{
	if(pWidget==NULL) return false;

	if(strcmp(pWidget->GetClassName(), MINT_LISTBOX)!=0) return false;
	MListBox* pList = (MListBox*)pWidget;

	//	pList->SetViewStyle(MVS_ICON);
	pList->SetVisibleHeader(false);
	pList->SetItemHeight(40);

	pList->AddField("Icon", 42);
	pList->AddField("Name", 600);
	// Å×½ºÆ®·Î 30°³ ³Ö¾î³õÀ½
	for (int i = 0; i < 30; i++)
	{
		char szName[256], szItem[256];
		int d = i % 6;
		sprintf(szItem, "item%03d.png", d);
		sprintf(szName, "³ª¹«ºí·¹ÀÌµå%d", i);
		AddListItem(pList, MBitmapManager::Get(szItem), szName, "Command Something");
	}

	return true;
}

#define DEFAULT_SLIDER_MAX			10000

ZGameInterface::ZGameInterface(const char* szName, MWidget* pParent, MListener* pListener) : ZInterface(szName,pParent,pListener)
{
	MSetString( 1, ZMsg(MSG_MENUITEM_OK));
	MSetString( 2, ZMsg(MSG_MENUITEM_CANCEL));
	MSetString( 3, ZMsg(MSG_MENUITEM_YES));
	MSetString( 4, ZMsg(MSG_MENUITEM_NO));
	MSetString( 5, ZMsg(MSG_MENUITEM_MESSAGE));

	m_pShopEquipInterface = new ZShopEquipInterface;

	m_bShowInterface = true;
	m_bViewUI = true;
	m_bWaitingArrangedGame = false;

	m_pMyCharacter = NULL;

	SetBounds(0, 0, MGetWorkspaceWidth(), MGetWorkspaceHeight());

	m_pGame = NULL;
	m_pCombatInterface = NULL;
	m_pLoadingInterface = NULL;

	m_dwFrameMoveClock = 0;

	m_nInitialState = GUNZ_LOGIN;
	m_nPreviousState = GUNZ_LOGIN;

	m_nState = GUNZ_NA;

	m_bCursor = true;
	MCursorSystem::Show(m_bCursor);

	//m_bCursor = false;
	m_bLogin = false;
	m_bLoading = false;

	m_pRoomListFrame = NULL;
	m_pDuelTournamentLobbyFrame = NULL;
	m_pBottomFrame = NULL;
	m_pClanInfoBg = NULL;
	m_pDuelTournamentInfoBg = NULL;
	m_pDuelTournamentRankingLabel = NULL;

	m_pMsgBox = new ZMsgBox("", Mint::GetInstance()->GetMainFrame(), this, MT_OK);
	m_pConfirmMsgBox = new ZMsgBox("", Mint::GetInstance()->GetMainFrame(), this, MT_YESNO);

	m_pMonsterBookInterface = new ZMonsterBookInterface();

//	ZApplication::GetStageInterface()->SetMapName( MMATCH_DEFAULT_STAGESETTING_MAPNAME);
	m_pMapThumbnail=NULL;

	m_pCharacterSelectView = NULL;
	m_pBackground = new ZInterfaceBackground();

	m_pCursorSurface=NULL;
	
	if (!m_sbRemainClientConnectionForResetApp)
		m_spGameClient=NULL;

	m_nDrawCount = 0;

	m_bTeenVersion = true;
//	m_pAmbSound = NULL;
//	m_bisReserveChangeWeapon = false;
//	m_nReserveChangeWeapon = ZCWT_MELEE;

	m_pScreenEffectManager = NULL;
	m_pEffectManager = NULL;
	m_pGameInput = NULL;
	
	m_bReservedWeapon = false;
	m_ReservedWeapon = ZCWT_NONE;

	m_bLeaveBattleReserved = false;
	m_bLeaveStageReserved = false;

	Mint::GetInstance()->SetGlobalEvent(ZGameInterface::OnGlobalEvent);
	ZGetInput()->SetEventListener(ZGameInterface::OnGlobalEvent);

	m_pPlayerMenu = NULL;
	m_pMiniMap = NULL;

    m_bOnEndOfReplay = false;
	m_nLevelPercentCache = 0;

	m_pLoginBG = NULL;
	m_pLoginPanel = NULL;

	m_nLoginState = LOGINSTATE_STANDBY;
	m_dwLoginTimer = 0;

	m_nLocServ = 0;

	m_dwHourCount = 0;
	m_dwTimeCount = timeGetTime() + 3600000;
	st_nLastTimeAddedCash = 0;
	bEnough = true;
	// Lobby Bitmap
	if ( m_pRoomListFrame != NULL)
	{
        delete m_pRoomListFrame;
		m_pRoomListFrame = NULL;
	}
	if (m_pDuelTournamentLobbyFrame != NULL)
	{
		delete m_pDuelTournamentLobbyFrame;
		m_pDuelTournamentLobbyFrame = NULL;
	}
	if ( m_pBottomFrame != NULL)
	{
		delete m_pBottomFrame;
		m_pBottomFrame = NULL;
	}	
	if ( m_pClanInfoBg != NULL)
	{
		delete m_pClanInfoBg;
		m_pClanInfoBg = NULL;
	}
	if ( m_pDuelTournamentInfoBg != NULL)
	{
		delete m_pDuelTournamentInfoBg;
		m_pDuelTournamentInfoBg = NULL;
	}
	if ( m_pDuelTournamentRankingLabel != NULL)
	{
		delete m_pDuelTournamentRankingLabel;
		m_pDuelTournamentRankingLabel = NULL;
	}

	// Login Bitmap
	if ( m_pLoginBG != NULL)
	{
		delete m_pLoginBG;
		m_pLoginBG = NULL;
	}

	if ( m_pLoginPanel != NULL)
	{
		delete m_pLoginPanel;
		m_pLoginPanel = NULL;
	}

	m_dwRefreshTime = 0;

	m_pLocatorList = ZGetConfiguration()->GetLocatorList();
	m_pTLocatorList = ZGetConfiguration()->GetTLocatorList();

	// º¸ÀÌ½º °ü·Ã ÃÊ±âÈ­
	m_dwVoiceTime = 0;
	m_szCurrVoice[ 0] = 0;
	m_szNextVoice[ 0] = 0;
	m_dwNextVoiceTime = 0;

	m_bReservedQuit = false;
	m_bReserveResetApp = false;

	m_dErrMaxPalyerDelayTime = 0;
	m_bErrMaxPalyer = false;

	m_bGameFinishLeaveBattle = true;

	m_MyPort = 0;

	timeSpecialCase = 0;
	specialCase = false;
	paused = false;
	tenSeconds = false;
}

ZGameInterface::~ZGameInterface()
{
	ZEmptyBitmap();

	OnDestroy();

	SAFE_DELETE(m_pMiniMap);
	SAFE_RELEASE(m_pCursorSurface);
	SAFE_DELETE(m_pScreenEffectManager);
	SAFE_DELETE(m_pEffectManager);
	SAFE_DELETE(m_pCharacterSelectView);
	SAFE_DELETE(m_pBackground);
	SAFE_DELETE(m_pMsgBox);
	SAFE_DELETE(m_pConfirmMsgBox);
	SAFE_DELETE(m_pMonsterBookInterface);

	SAFE_DELETE(m_pRoomListFrame);
	SAFE_DELETE(m_pDuelTournamentLobbyFrame);
	SAFE_DELETE(m_pBottomFrame);
	SAFE_DELETE(m_pClanInfoBg);
	SAFE_DELETE(m_pDuelTournamentInfoBg);
	SAFE_DELETE(m_pDuelTournamentRankingLabel);

	SAFE_DELETE(m_pLoginPanel);

	SAFE_DELETE(m_pShopEquipInterface);
}


bool ZGameInterface::InitInterface(const char* szSkinName, ZLoadingProgress *pLoadingProgress)
{
	DWORD _begin_time,_end_time;
#define BEGIN_ { _begin_time = timeGetTime(); }
#define END_(x) { _end_time = timeGetTime(); float f_time = (_end_time - _begin_time) / 1000.f; mlog("%s : %f \n", x,f_time ); }


	SetObjectTextureLevel(ZGetConfiguration()->GetVideo()->nCharTexLevel);
	SetMapTextureLevel(ZGetConfiguration()->GetVideo()->nMapTexLevel);
	SetEffectLevel(ZGetConfiguration()->GetVideo()->nEffectLevel);
	SetTextureFormat(ZGetConfiguration()->GetVideo()->nTextureFormat);
#ifndef _DEBUG
	SetBandiCaptureConfig(ZGetConfiguration()->GetMovingPicture()->iResolution);	// µ¿¿µ»ó Ä¸ÃÄ ÇØ»óµµ ¼¼ÆÃ
	SetBandiCaptureFileSize(ZGetConfiguration()->GetMovingPicture()->iFileSize);		// µ¿¿µ»ó Ä¸ÃÄ ÆÄÀÏÅ©±â(¿ë·®Á¦ÇÑ)
#endif

	bool bRet = true;
	char szPath[256];
	char szSubPath[256] = "interface/grm/default/";	// ¾ð¾îº¯°æÀ» À§ÇÑ ´ëÃ¼ °æ·Î
	char szFileName[256];
	char a_szSkinName[256];
	strcpy(a_szSkinName, szSkinName);

	ZGetInterfaceSkinPath(szPath, a_szSkinName);
	ZGetInterfaceSkinPathSubLanguage(szSubPath, a_szSkinName);
	sprintf(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);

	/*
	BEGIN_;
	if ((stricmp(szSkinName, DEFAULT_INTERFACE_SKIN)) && (!IsExist(szFileName)))
	{
		strcpy(a_szSkinName, DEFAULT_INTERFACE_SKIN);
		ZGetInterfaceSkinPath(szPath, a_szSkinName);
		sprintf(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);
		bRet = false;
	}
	END_("interface skin");
	*/

	ZEmptyBitmap();

	ZLoadingProgress pictureProgress("pictures",pLoadingProgress,.7f);
	BEGIN_;
	//ZLoadBitmap(szPath, ".png");
	//ZLoadBitmap(szPath, ".bmp");
	//ZLoadBitmap(szPath, ".tga");
	////	ZLoadBitmap(szPath, ".dds");
	//ZLoadBitmap(PATH_CUSTOM_CROSSHAIR, ".png", true);
	LoadBitmaps(szPath, szSubPath, &pictureProgress);

	END_("loading pictures");

	BEGIN_;
	if (!m_IDLResource.LoadFromFile(szFileName, this, ZGetFileSystem()))
	{
		// ·Îµå ½ÇÆÐÇÏ¸é Default·Î ·Îµå
		strcpy(a_szSkinName, DEFAULT_INTERFACE_SKIN);
		ZGetInterfaceSkinPath(szPath, a_szSkinName);
		sprintf(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);
		//ZLoadBitmap(szPath, ".png");
		//ZLoadBitmap(szPath, ".bmp");
		//ZLoadBitmap(szPath, ".tga");
		//ZLoadBitmap(szPath, ".dds");
		LoadBitmaps(szPath, szSubPath, &pictureProgress);

		if (m_IDLResource.LoadFromFile(szFileName, this, ZGetFileSystem()))
		{
			mlog("IDLResource Loading Success!!\n");
		}
		else
		{
			mlog("IDLResource Loading Failed!!\n");
		}
		bRet = false;
	}
	else
	{
		mlog("IDLResource Loading Success!!\n");
	}
	END_("IDL resources");

// ´ÙÀÌ¾Ë·Î±× look ¼¼ÆÃ
//	MBFrameLook* pFrameLook = (MBFrameLook*)m_IDLResource.FindFrameLook("Custom1FrameLook");
	MBFrameLook* pFrameLook = (MBFrameLook*)m_IDLResource.FindFrameLook("DefaultFrameLook");
	if (pFrameLook != NULL)
	{
		m_pMsgBox->ChangeCustomLook((MFrameLook*)pFrameLook);
		m_pConfirmMsgBox->ChangeCustomLook((MFrameLook*)pFrameLook);
	}
	else
	{
		//_ASSERT(0);
	}

	ZStageSetting::InitStageSettingDialog();

// EquipmentListBox ÀÌº¥Æ® ¼¼ÆÃ
	// »óÁ¡¿¡ÀÖ´Â°Í
	ZShopEquipListbox* pShopEquipListBox = (ZShopEquipListbox*)m_IDLResource.FindWidget("AllEquipmentList");
	if (pShopEquipListBox)
	{
		pShopEquipListBox->SetOnDropCallback(ShopPurchaseItemListBoxOnDrop);
	}
	pShopEquipListBox = (ZShopEquipListbox*)m_IDLResource.FindWidget("MyAllEquipmentList");
	if (pShopEquipListBox)
	{
		pShopEquipListBox->SetOnDropCallback(ShopSaleItemListBoxOnDrop);
	}

	// Àåºñ¿¡ ÀÖ´Â°Í
	pShopEquipListBox = (ZShopEquipListbox*)m_IDLResource.FindWidget("EquipmentList");
	if (pShopEquipListBox)
	{
		pShopEquipListBox->SetOnDropCallback(CharacterEquipmentItemListBoxOnDrop);
	}

	// °èÁ¤¿¡ ÀÖ´Â°Í
	pShopEquipListBox = (ZShopEquipListbox*)m_IDLResource.FindWidget("AccountItemList");
	if (pShopEquipListBox)
	{
//		pShopEquipListBox->SetOnDropCallback(CharacterEquipmentItemListBoxOnDrop);
	}

	// ¾ÆÀÌÅÛ ¼³¸í textareaÀÇ Ä¿½ºÅÒ ·èÀ» ÁöÁ¤
	m_textAreaLookItemDesc.SetBgColor(MCOLOR(10, 10, 10, 220));

	MTextArea* pTextArea = (MTextArea*)m_IDLResource.FindWidget("Shop_ItemDescription");
	if (pTextArea) pTextArea->ChangeCustomLook(&m_textAreaLookItemDesc);
	pTextArea = (MTextArea*)m_IDLResource.FindWidget("Equip_ItemDescription");
	if (pTextArea) pTextArea->ChangeCustomLook(&m_textAreaLookItemDesc);
/*
	MPanel* pPanel = new MPanel();
	pPanel->SetPosition(10, 10);
	pPanel->SetSize(300, 300);
*/
	InitInterfaceListener();

	// CenterMessage °¡¿îµ¥ Á¤·Ä
#define CENTERMESSAGE	"CenterMessage"
	BEGIN_WIDGETLIST(CENTERMESSAGE, &m_IDLResource, MLabel*, pWidget);
	pWidget->SetAlignment(MAM_HCENTER);
	END_WIDGETLIST();

	ZGetOptionInterface()->InitInterfaceOption();

#ifdef _FASTDEBUG
	m_IDLResource.FindWidget("LoginPassword")->SetText("1111");
//	m_IDLResource.FindWidget("LoginPassword")->SetText("2708");
#endif
	InitMaps(m_IDLResource.FindWidget("MapSelection"));

	//_ASSERT(m_pPlayerMenu==NULL);
	// Player Menu
	m_pPlayerMenu = new ZPlayerMenu("PlayerMenu", this, this, MPMT_VERTICAL);
	((MPopupMenu*)m_pPlayerMenu)->Show(false);

	return true;
}

bool ZGameInterface::InitInterfaceListener()
{
	// ´ÙÀÌ¾Ë·Î±×
	m_pMsgBox->SetListener(ZGetMsgBoxListener());
	m_pConfirmMsgBox->SetListener(ZGetConfirmMsgBoxListener());


	// ¿É¼Ç
	SetListenerWidget("VideoGamma", ZGetOptionGammaSliderChangeListener());
	SetListenerWidget("ResizeCancel", ZGetCancelResizeConfirmListener());
	SetListenerWidget("ResizeRequest", ZGetRequestResizeListener());
	SetListenerWidget("ResizeReject", ZGetViewConfirmCancelListener());
	SetListenerWidget("ResizeAccept", ZGetViewConfrimAcceptListener());
	SetListenerWidget("16BitSound",ZGet16BitSoundListener());
	SetListenerWidget("8BitSound",ZGet8BitSoundListener());

	SetListenerWidget("NetworkPortChangeRestart", ZGetNetworkPortChangeRestartListener());
	SetListenerWidget("NetworkPortChangeCancel", ZGetNetworkPortChangeCancelListener());

	SetListenerWidget("MouseSensitivitySlider", ZGetMouseSensitivitySliderListener());
	SetListenerWidget("MouseSensitivityEdit", ZGetMouseSensitivityEditListener());

	// ·Î±×ÀÎÈ­¸é
	SetListenerWidget("Exit", ZGetExitListener());
	SetListenerWidget("News", ZGetNewsListener());
	SetListenerWidget("LoginOK", ZGetLoginListener());
	SetListenerWidget("OptionFrame", ZGetOptionFrameButtonListener());
	SetListenerWidget("Register", ZGetRegisterListener());
	SetListenerWidget("Stage_OptionFrame", ZGetOptionFrameButtonListener());
	SetListenerWidget("Stage_OptionFrame2", ZGetOptionFrameButtonListener());
	SetListenerWidget("LobbyOptionFrame", ZGetOptionFrameButtonListener());
	SetListenerWidget("LobbyOptionFrame2", ZGetOptionFrameButtonListener());
	SetListenerWidget("LobbyOptionFrame6", ZGetOptionFrameButtonListener());
	SetListenerWidget("WarningExit", ZGetExitListener());

	// ·Îºñ
	SetListenerWidget("Logout", ZGetLogoutListener());
	SetListenerWidget("ChannelChattingInput", ZGetChannelChatInputListener());
	SetListenerWidget("GameStart", ZGetGameStartListener());
	SetListenerWidget("ChatInput", ZGetChatInputListener());
	SetListenerWidget("ParentClose", ZGetParentCloseListener());
	SetListenerWidget("ChannelListFrameCaller", ZGetChannelListFrameCallerListener());
	SetListenerWidget("MapList", ZGetMapListListener());
	SetListenerWidget("Lobby_StageExit", ZGetLobbyListener());
	SetListenerWidget("LoginState", ZGetLoginStateButtonListener());
	SetListenerWidget("GreeterState", ZGetGreeterStateButtonListener());
	SetListenerWidget("CombatMenuClose", ZGetCombatMenuCloseButtonListener());
	SetListenerWidget("PreviousState", ZGetPreviousStateButtonListener());
	SetListenerWidget("QuickJoin", ZGetQuickJoinButtonListener());
	SetListenerWidget("QuickJoin2", ZGetQuickJoinButtonListener());
	SetListenerWidget("Lobby_Charviewer_info", ZGetLobbyCharInfoCallerButtonListener());
	SetListenerWidget("StageBeforeBtn", ZGetLobbyPrevRoomListButtonListener());
	SetListenerWidget("StageAfterBtn", ZGetLobbyNextRoomListPrevButtonListener());
//	SetListenerWidget("Lobby_StageList", )

	SetListenerWidget("Lobby_RoomNo1", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo2", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo3", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo4", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo5", ZGetLobbyNextRoomNoButtonListener());
	SetListenerWidget("Lobby_RoomNo6", ZGetLobbyNextRoomNoButtonListener());

	/*
	SetListenerWidget("ChannelPlayerList_all", ZGetChannelPlayerOptionGroupAll());
	SetListenerWidget("ChannelPlayerList_friend", ZGetChannelPlayerOptionGroupFriend());
	SetListenerWidget("ChannelPlayerList_clan", ZGetChannelPlayerOptionGroupClan());

	SetListenerWidget("LobbyPlayerListTabChannel", ZGetListenerLobbyPlayerListTabChannel());
	SetListenerWidget("LobbyPlayerListTabFriend", ZGetListenerLobbyPlayerListTabFriend());
	SetListenerWidget("LobbyPlayerListTabClan", ZGetListenerLobbyPlayerListTabClan());
	*/
	
	SetListenerWidget("LobbyChannelPlayerListPrev", ZGetPlayerListPrevListener());
	SetListenerWidget("LobbyChannelPlayerListNext", ZGetPlayerListNextListener());

//	SetListenerWidget("StagePlayerListTabGame", ZGetListenerGamePlayerListTabGame());
//	SetListenerWidget("StagePlayerListTabFriend", ZGetListenerGamePlayerListTabFriend());

	SetListenerWidget("StagePlayerListPrev", ZGetStagePlayerListPrevListener());
	SetListenerWidget("StagePlayerListNext", ZGetStagePlayerListNextListener());

	SetListenerWidget("ArrangedPlayerWars", ZGetArrangedPlayerWarsListener());

	SetListenerWidget("PlayerWarsVoteButton0", ZGetPlayerWarsVote0());
	SetListenerWidget("PlayerWarsVoteButton1", ZGetPlayerWarsVote1());
	SetListenerWidget("PlayerWarsVoteButton2", ZGetPlayerWarsVote2());

	SetListenerWidget("ArrangedTeamGame", ZGetArrangedTeamGameListener());
	SetListenerWidget("ArrangedTeamDialogOk", ZGetArrangedTeamDialogOkListener());
	SetListenerWidget("ArrangedTeamDialogClose", ZGetArrangedTeamDialogCloseListener());
	SetListenerWidget("ArrangedTeamGame_Cancel", ZGetArrangedTeamGame_CancelListener());

	SetListenerWidget("ArrangedTeamGameWarmUp", ZGetArrangedTeamGameWarmUpListener());
	SetListenerWidget("ArrangedTeamWarmUpDialogOk", ZGetArrangedTeamWarmUpDialogOkListener());
	SetListenerWidget("ArrangedTeamWarmUpDialogClose", ZGetArrangedTeamWarmUpDialogCloseListener());

	SetListenerWidget("LeaveClanOK",		ZGetLeaveClanOKListener());
	SetListenerWidget("LeaveClanCancel",	ZGetLeaveClanCancelListener());

	SetListenerWidget("DuelTournamentGame",				ZGetDuelTournamentGameButtonListener());
	SetListenerWidget("DuelTournamentGame_2Test",		ZGetDuelTournamentGame2TestButtonListener());
	SetListenerWidget("DuelTournamentGame_4Test",		ZGetDuelTournamentGame4TestButtonListener());
	SetListenerWidget("DuelTournamentWaitMatchCancel",	ZGetDuelTournamentWaitCancelButtonListener());

	// channel
	SetListenerWidget("ChannelJoin", ZGetChannelListJoinButtonListener());
	SetListenerWidget("ChannelListClose", ZGetChannelListCloseButtonListener());
	SetListenerWidget("ChannelList", ZGetChannelListListener());
	SetListenerWidget("PrivateChannelEnter", ZGetPrivateChannelEnterListener());
	
	SetListenerWidget("ChannelList_Normal", ZGetChannelList());
	SetListenerWidget("ChannelList_Private", ZGetChannelList());
	SetListenerWidget("ChannelList_Clan", ZGetChannelList());
	SetListenerWidget("ChannelList_DuelTournament", ZGetChannelList());
	SetListenerWidget("MyClanChannel", ZGetMyClanChannel());
	
	// stage 
	SetListenerWidget("StageCreateFrameCaller", ZGetStageCreateFrameCallerListener());

	SetListenerWidget("StageListFrameCaller", ZGetStageListFrameCallerListener());
	SetListenerWidget("StageCreateBtn", ZGetStageCreateBtnListener());
	SetListenerWidget("PrivateStageJoinBtn", ZGetPrivateStageJoinBtnListener());
	SetListenerWidget("StageJoin", ZGetStageJoinListener());
	SetListenerWidget("StageChattingInput", ZGetStageChatInputListener());
	SetListenerWidget("StageSettingCaller", ZGetStageSettingCallerListener());
	SetListenerWidget("StageType", ZGetStageSettingStageTypeListener());
	SetListenerWidget("StageMaxPlayer", ZGetStageSettingChangedComboboxListener());			// µ¿È¯ÀÌ°¡ Ãß°¡ : ÃÖ´ëÀÎ¿ø
	SetListenerWidget("StageRoundCount", ZGetStageSettingChangedComboboxListener());		// µ¿È¯ÀÌ°¡ Ãß°¡ : °æ±âÈ½¼ö
	SetListenerWidget("StageLimitTime", ZGetStageSettingChangedComboboxListener());			// µ¿È¯ÀÌ°¡ Ãß°¡ : Á¦ÇÑ½Ã°£
	SetListenerWidget("StageIntrude", ZGetStageSettingChangedComboboxListener());			// µ¿È¯ÀÌ°¡ Ãß°¡ : ³­ÀÔÇã¿ë
	SetListenerWidget("StageTeamRed", ZGetStageTeamRedListener());
	SetListenerWidget("StageTeamRed2", ZGetStageTeamRedListener());
	SetListenerWidget("StageTeamBlue", ZGetStageTeamBlueListener());
	SetListenerWidget("StageTeamBlue2", ZGetStageTeamBlueListener());
	SetListenerWidget("StageObserverBtn", ZGetStageObserverBtnListener());					// µ¿È¯ÀÌ°¡ Ãß°¡ : °üÀü Ã¼Å©
	SetListenerWidget("StageReady", ZGetStageReadyListener());
	SetListenerWidget("StageSettingApplyBtn", ZGetStageSettingApplyBtnListener());
	SetListenerWidget("BattleExit", ZGetBattleExitButtonListener());
	SetListenerWidget("StageExit", ZGetStageExitButtonListener());

	SetListenerWidget("MapChange", ZGetMapChangeListener());
	SetListenerWidget("MapSelect", ZGetMapSelectListener());

	SetListenerWidget("MapList", ZGetStageMapListSelectionListener());
//	SetListenerWidget("MapList_Caller", ZGetStageMapListCallerListener());

	SetListenerWidget("Stage_SacrificeItemButton0", ZStageSacrificeItem0());
	SetListenerWidget("Stage_SacrificeItemButton1", ZStageSacrificeItem1());
	SetListenerWidget("Stage_PutSacrificeItem", ZStagePutSacrificeItem());
	SetListenerWidget("Stage_SacrificeItemBoxOpen", ZStageSacrificeItemBoxOpen());
	SetListenerWidget("Stage_SacrificeItemBoxClose", ZStageSacrificeItemBoxClose());
	SetListenerWidget("Stage_SacrificeItemListbox", ZGetSacrificeItemListBoxListener());
	SetListenerWidget("MonsterBookCaller", ZGetMonsterBookCaller());

	// ¸Ê ¼±ÅÃ
	SetListenerWidget("MapSelection",  ZGetMapComboListener());

	// ¸±·¹ÀÌ¸Ê
	SetListenerWidget("Stage_RelayMapType",			ZGetRelayMapTypeListener());
	SetListenerWidget("Stage_RelayMapRepeatCount",	ZGetRelayMapTurnCountListener());
	SetListenerWidget("Stage_RelayMapListbox",		ZGetRelayMapListBoxListener());
	SetListenerWidget("Stage_MapListbox",			ZGetMapListBoxListener());
	SetListenerWidget("Stage_RelayMap_OK_Button",	ZGetRelayMapOKButtonListener());
	SetListenerWidget("Stage_RelayMapBoxOpen",		ZStageRelayMapBoxOpen());
	SetListenerWidget("Stage_RelayMapBoxClose",		ZStageRelayMapBoxClose());

	// ¿É¼Ç
	SetListenerWidget("SaveOptionButton", ZGetSaveOptionButtonListener());
	SetListenerWidget("CancelOptionButton", ZGetCancelOptionButtonListener());

	/*
	SetListenerWidget("ShowVideoOptionGroup", ZGetShowVideoOptionGroupButtonListener());
	SetListenerWidget("ShowAudioOptionGroup", ZGetShowAudioOptionGroupButtonListener());
	SetListenerWidget("ShowMouseOptionGroup", ZGetShowMouseOptionGroupButtonListener());
	SetListenerWidget("ShowKeyboardOptionGroup", ZGetShowKeyboardOptionGroupButtonListener());
	SetListenerWidget("ShowEtcOptionGroup", ZGetShowEtcOptionGroupButtonListener());
	*/
	SetListenerWidget("DefaultSettingLoad", ZGetLoadDefaultKeySettingListener() );
	SetListenerWidget("Optimization",ZSetOptimizationListener());

	//======================================================================================================
	// »óÁ¡
	SetListenerWidget("ShopCaller", ZGetShopCallerButtonListener());
	SetListenerWidget("ShopCaller1", ZGetShopCallerButtonListener());
	SetListenerWidget("ShopClose", ZGetShopCloseButtonListener());

	SetListenerWidget("EquipmentCaller", ZGetEquipmentCallerButtonListener());
	SetListenerWidget("EquipmentCaller2", ZGetEquipmentCallerButtonListener());
	SetListenerWidget("EquipmentCaller4", ZGetEquipmentCallerButtonListener());

	SetListenerWidget("EquipmentClose", ZGetEquipmentCloseButtonListener());

	SetListenerWidget("CharSelectionCaller", ZGetCharSelectionCallerButtonListener());

	SetListenerWidget("BuyConfirmCaller", ZGetBuyButtonListener());

	SetListenerWidget("SellConfirmCaller", ZGetSellButtonListener());
	SetListenerWidget("TradeCountableItem_Cancel", ZGetCountableItemTradeDlgCloseListener());
	SetListenerWidget("TradeCountableItem_Ok", ZGetCountableItemTradeDlgOkButtonListener());
	SetListenerWidget("TradeCountableItem_CountUp", ZGetCountableItemTradeDlgCountUpButtonListener());
	SetListenerWidget("TradeCountableItem_CountDn", ZGetCountableItemTradeDlgCountDnButtonListener());	
	SetListenerWidget("TradeCountableItem_CountNum", ZGetCountableItemTradeDlgCountChangeListener());
	SetListenerWidget("SellCashItemConfirmFrame_Sell", ZGetSellCashItemConfirmDlgOkButtonListener());
	SetListenerWidget("SellCashItemConfirmFrame_Cancel", ZGetSellCashItemConfirmDlgCancelListener());
	SetListenerWidget("UnlimitedRent", ZGetUnlimitedRentButtonListener());
	//======================================================================================================

	SetListenerWidget("Equip", ZGetEquipButtonListener());
	SetListenerWidget("EquipmentSearch", ZGetEquipmentSearchButtonListener());
	SetListenerWidget("ForcedEntryToGame", ZGetStageForcedEntryToGameListener());
	SetListenerWidget("ForcedEntryToGame2", ZGetStageForcedEntryToGameListener());
	SetListenerWidget("AllEquipmentListCaller", ZGetAllEquipmentListCallerButtonListener());
	SetListenerWidget("MyAllEquipmentListCaller", ZGetMyAllEquipmentListCallerButtonListener());
	SetListenerWidget("CashEquipmentListCaller", ZGetCashEquipmentListCallerButtonListener());
	//	SetListenerWidget("Shop_Ask", ZGetShopAskButtonListener());
	//	SetListenerWidget("Shop_Gift", ZGetShopGiftButtonListener());
	SetListenerWidget("CashRecharge", ZGetShopCachRechargeButtonListener());
	SetListenerWidget("ShopSearchFrameCaller", ZGetShopSearchCallerButtonListener());

	SetListenerWidget("AllEquipmentList", ZGetShopPurchaseItemListBoxListener());
	SetListenerWidget("MyAllEquipmentList", ZGetShopSellItemListBoxListener());
	//SetListenerWidget("CashEquipmentList", ZGetCashShopItemListBoxListener());

	SetListenerWidget("Shop_AllEquipmentFilter", ZGetShopListFilterListener());
	SetListenerWidget("Equip_AllEquipmentFilter", ZGetEquipListFilterListener());
	SetListenerWidget("Shop_to_Equipment", ZGetShopEquipmentCallerButtonListener() );
	SetListenerWidget("Shop_to_Equipment2", ZGetShopEquipmentCallerButtonListener());
	SetListenerWidget("Shop_ArmorEquipmentTab",  ZShopItemEquipmentTabOpen());
	SetListenerWidget("Shop_WeaponEquipmentTab",  ZShopWeaponEquipmentTabOpen());	
	SetListenerWidget("Shop_EquipListFrameCloseButton", ZShopListFrameClose());
	SetListenerWidget("Shop_EquipListFrameOpenButton",  ZShopListFrameOpen());

	// Àåºñ
	SetListenerWidget("Equipment_CharacterTab", ZGetEquipmentCharacterTabButtonListener());
	SetListenerWidget("Equipment_AccountTab", ZGetEquipmentAccountTabButtonListener());
	SetListenerWidget("EquipmentList", ZGetEquipmentMyItemListBoxListener());
	SetListenerWidget("AccountItemList", ZGetAccountItemListBoxListener());
	SetListenerWidget("Equipment_to_Shop", ZGetEquipmentShopCallerButtonListener());		
	SetListenerWidget("Equip_ArmorEquipmentTab",  ZEquipItemEquipmentTabOpen());
	SetListenerWidget("Equip_WeaponEquipmentTab",  ZEquipWeaponEquipmentTabOpen());	
	SetListenerWidget("Equip_EquipListFrameOpenButton",  ZEquipListFrameOpen());
	SetListenerWidget("Equip_EquipListFrameCloseButton", ZEquipListFrameClose());
	SetListenerWidget("Equipment_CharacterRotate", ZEquipmetRotateBtn());

	SetListenerWidget("BringAccountItemBtn", ZGetBringAccountItemButtonListener());
	SetListenerWidget("SendAccountItemBtn", ZGetSendAccountItemButtonListener());

	// Ä³¸¯ÅÍ ¼±ÅÃ ºÎºÐ
	SetListenerWidget("CS_SelectChar", ZGetSelectCharacterButtonListener());
	SetListenerWidget("CS_SelectCharDefKey", ZGetSelectCharacterButtonListener());
	SetListenerWidget("CS_CreateChar", ZGetShowCreateCharacterButtonListener());
	SetListenerWidget("CS_DeleteChar", ZGetDeleteCharacterButtonListener());
	SetListenerWidget("CS_Prev", ZGetLogoutListener());
	SetListenerWidget("CharSel_SelectBtn0", ZGetSelectCharacterButtonListener0());
	SetListenerWidget("CharSel_SelectBtn1", ZGetSelectCharacterButtonListener1());
	SetListenerWidget("CharSel_SelectBtn2", ZGetSelectCharacterButtonListener2());
	SetListenerWidget("CharSel_SelectBtn3", ZGetSelectCharacterButtonListener3());
	SetListenerWidget("ShowChar_infoGroup", ZGetShowCharInfoGroupListener());
	SetListenerWidget("ShowEquip_InfoGroup", ZGetShowEquipInfoGroupListener());

	//	SetListenerWidget("CS_CharList", ZGetSelectCharacterListBoxDBClickListener);
	// Ä³¸¯ÅÍ »ý¼º
	SetListenerWidget("CC_CreateChar", ZGetCreateCharacterButtonListener());
	SetListenerWidget("CC_Cancel", ZGetCancelCreateCharacterButtonListener());
	SetListenerWidget("CC_Sex", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Hair", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Face", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Face", ZChangeCreateCharInfoListener());
	SetListenerWidget("CC_Costume", ZChangeCreateCharInfoListener());
	SetListenerWidget("Charviewer_Rotate_L", ZGetCreateCharacterLeftButtonListener());
	SetListenerWidget("Charviewer_Rotate_R", ZGetCreateCharacterRightButtonListener());
	SetListenerWidget("RedColor", ZGetCreateCharacterRedColorListener());
	SetListenerWidget("GreenColor", ZGetCreateCharacterGreenColorListener());
	SetListenerWidget("BlueColor", ZGetCreateCharacterBlueColorListener());

	SetListenerWidget("Lobby_Charviewer_Rotate_L", ZGetSelectCameraLeftButtonListener());
	SetListenerWidget("Lobby_Charviewer_Rotate_R", ZGetSelectCameraRightButtonListener());

	SetListenerWidget("CS_DeleteCharButton", ZGetConfirmDeleteCharacterButtonListener());
	SetListenerWidget("CS_CloseConfirmDeleteCharButton", ZGetCloseConfirmDeleteCharButtonListener());

	// ¸ó½ºÅÍ µµ°¨
	SetListenerWidget("MonsterBook_PrevPageButton", ZGetMonsterInterfacePrevPage());
	SetListenerWidget("MonsterBook_NextPageButton", ZGetMonsterInterfaceNextPage());
	SetListenerWidget("MonsterBook_Close", ZGetMonsterInterfaceQuit());
	SetListenerWidget("MonsterBook_Close2", ZGetMonsterInterfaceQuit());


	// °ÔÀÓ °á°ú È­¸é
	SetListenerWidget("GameResult_ButtonQuit", ZGetGameResultQuit());


	//112
	SetListenerWidget("112_ConfirmEdit", ZGet112ConfirmEditListener());
	SetListenerWidget("112_ConfirmOKButton", ZGet112ConfirmOKButtonListener());
	SetListenerWidget("112_ConfirmCancelButton", ZGet112ConfirmCancelButtonListener());


	// Å¬·£
	SetListenerWidget("ClanSponsorAgreementConfirm_OK", ZGetClanSponsorAgreementConfirm_OKButtonListener());
	SetListenerWidget("ClanSponsorAgreementConfirm_Cancel", ZGetClanSponsorAgreementConfirm_CancelButtonListener());
	SetListenerWidget("ClanSponsorAgreementWait_Cancel", ZGetClanSponsorAgreementWait_CancelButtonListener());
	SetListenerWidget("ClanJoinerAgreementConfirm_OK", ZGetClanJoinerAgreementConfirm_OKButtonListener());
	SetListenerWidget("ClanJoinerAgreementConfirm_Cancel", ZGetClanJoinerAgreementConfirm_CancelButtonListener());
	SetListenerWidget("ClanJoinerAgreementWait_Cancel", ZGetClanJoinerAgreementWait_CancelButtonListener());
	
	// Å¬·£ »ý¼º ´ÙÀÌ¾ó·Î±×
	SetListenerWidget("LobbyPlayerListTabClanCreateButton", ZGetLobbyPlayerListTabClanCreateButtonListener());
	SetListenerWidget("ClanCreateDialogOk", ZGetClanCreateDialogOk());
	SetListenerWidget("ClanCreateDialogClose", ZGetClanCreateDialogClose());


	// µ¿ÀÇ °ü·Ã
	SetListenerWidget("ProposalAgreementWait_Cancel", ZGetProposalAgreementWait_CancelButtonListener());
	SetListenerWidget("ProposalAgreementConfirm_OK", ZGetProposalAgreementConfirm_OKButtonListener());
	SetListenerWidget("ProposalAgreementConfirm_Cancel", ZGetProposalAgreementConfirm_CancelButtonListener());

	// ¸®ÇÃ·¹ÀÌ ºê¶ó¿ìÁ® °ü·Ã
	SetListenerWidget("ReplayOkButton",				ZReplayOk());
	SetListenerWidget("ReplayCaller",				ZGetReplayCallerButtonListener());
	SetListenerWidget("Replay_View",				ZGetReplayViewButtonListener());
	SetListenerWidget("ReplayClose",				ZGetReplayExitButtonListener());
	SetListenerWidget("Replay_FileList",			ZGetReplayFileListBoxListener());

#ifdef _GZFGunz
	SetListenerWidget("Shop_AllFilter", ZGetShopAllFilterListener());
	SetListenerWidget("Equip_AllFilter", ZGetEquipAllFilterListener());

	SetListenerWidget("Shop_AllEquipmentItemFilter", ZGetShopItemListFilterListener());
	SetListenerWidget("Equip_AllEquipmenItemtFilter", ZGetEquipListItemFilterListener());
	SetListenerWidget("UpdatePassword", ZGetUpdateClanPassword());
	SetListenerWidget("LockMyClanChannel", ZGetLockMyChannel());
	SetListenerWidget("UnlockMyClanChannel", ZGetUnlockMyChannel());
	SetListenerWidget("SumbitPassword", ZGetSumbitPassword());
	SetListenerWidget("BeforeButton", ZGetBeforeButtonListener());
	SetListenerWidget("PlayButton", ZGetPlayButtonListener());
	SetListenerWidget("AfterButton", ZGetAfterButtonListener());
	SetListenerWidget("BGMVolumeSlider", ZGetBGMVolumeSizeSliderListener());
	SetListenerWidget("EffectVolumeSlider", ZGetEffectVolumeSizeSliderListener());
	SetListenerWidget("SwordTrailSize", ZGetSwordTrailSizeSliderListener());
	SetListenerWidget("CharacterZoomSlider", ZGetCharacterZoomSliderListener());
	SetListenerWidget("DefaultCharacterZoom", ZGetDefaultCharacterZoomListener());
	SetListenerWidget("ScreenShotCaller", ZGetScreenShotCallerButtonListener());

	SetListenerWidget("ViewScreenShot", ZGetViewScreenShotCallerButtonListener());
	SetListenerWidget("ExitScreenShotPicture", ZGetExitScreenShotPictureCallerButtonListener());
	SetListenerWidget("ExitScreenShot", ZGetExitScreenShotCallerButtonListener());
	SetListenerWidget("NextScreenShotPicture", ZGetNextScreenShotCallerButtonListener());
	SetListenerWidget("PrevScreenShotPicture", ZGetPrevScreenShotCallerButtonListener());	
	
	// DAILY CHALLENGE CODE
	SetListenerWidget("ChallengeCaller", ZGetChallengeCallerButtonListener());
	SetListenerWidget("ChallengeCloseButton", ZGetChallengeButtonCloseListener());
	SetListenerWidget("ChallengeAcceptButton", ZGetChallengeButtonAcceptListener());
#endif

	MTabCtrl *pTab = (MTabCtrl*)m_IDLResource.FindWidget("PlayerListControl");
	if( pTab ) pTab->UpdateListeners();

	return true;
}

void ZGameInterface::FinalInterface()
{
	// Player Menu
	SAFE_DELETE(m_pPlayerMenu);

	m_IDLResource.Clear();

	mlog("clear IDL resource end.\n");

	SetCursor(NULL);

	mlog("Final interface end.\n");
}

bool ZGameInterface::ChangeInterfaceSkin(const char* szNewSkinName)
{
	char szPath[256];
	char szFileName[256];
	ZGetInterfaceSkinPath(szPath, szNewSkinName);
	sprintf(szFileName, "%s%s", szPath, FILENAME_INTERFACE_MAIN);

	FinalInterface();
	bool bSuccess=InitInterface(szNewSkinName);

	if(bSuccess)
	{
		switch(m_nState)
		{
		case GUNZ_LOGIN:	ShowWidget("Login", true); break;
		case GUNZ_LOBBY:	ShowWidget("Lobby", true); 	break;
		case GUNZ_STAGE: 	ShowWidget("Stage", true); 	break;
		case GUNZ_CHARSELECTION:
			if (m_bShowInterface)
			{
				ShowWidget("CharSelection", true);
			}break;
		case GUNZ_CHARCREATION : ShowWidget("CharCreation", true); break;
		}
		ZGetOptionInterface()->Resize(MGetWorkspaceWidth(),MGetWorkspaceHeight());
	}

	return bSuccess;
}


// ÆÄÃ÷±³È¯°ú ¹«±âÀåºñÀÇ ÀÓ½Ã µ¥ÀÌÅÍ µé....

static bool g_parts[10];	// Ä³¸¯ÅÍ Àåºñ ºÎÂø Å×½ºÆ®¿ë ÀÓ½Ã º¯¼ö
static bool g_parts_change;

//static int	g_select_weapon=0;
//static bool g_weapon[10];
//static bool g_weapon_change;



/// OnCommand¸¦ ZGameInterface_OnCommand.cpp·Î ¿È±è. 

bool ZGameInterface::ShowWidget(const char* szName, bool bVisible, bool bModal)
{
	MWidget* pWidget = m_IDLResource.FindWidget(szName);

	if ( pWidget == NULL)
		return false;

	if ( strcmp( szName, "Lobby") == 0)
	{
		pWidget->Show(bVisible, bModal);

		pWidget = m_IDLResource.FindWidget( "Shop");
		pWidget->Show( false);
		pWidget = m_IDLResource.FindWidget( "Equipment");
		pWidget->Show( false);
	}
	else
		pWidget->Show(bVisible, bModal);

	return true;
}

void ZGameInterface::SetListenerWidget(const char* szName, MListener* pListener)
{
	BEGIN_WIDGETLIST(szName, &m_IDLResource, MWidget*, pWidget);
	pWidget->SetListener(pListener);
	END_WIDGETLIST();
}

void ZGameInterface::EnableWidget(const char* szName, bool bEnable)
{
	MWidget* pWidget = m_IDLResource.FindWidget(szName);
	if (pWidget) pWidget->Enable(bEnable);
}

void ZGameInterface::SetTextWidget(const char* szName, const char* szText)
{
	BEGIN_WIDGETLIST(szName, &m_IDLResource, MWidget*, pWidget);
	pWidget->SetText(szText);
	END_WIDGETLIST();
}

bool ZGameInterface::OnGameCreate(void)
{
	// ¹èÆ² ÁøÀÔ½Ã ´Ù¸¥ À§Á¬Àº ¸ðµÎ false Ã³¸®
	HideAllWidgets();

	// ½æ³×ÀÏ ÀÌ¹ÌÁö ¾ð·ÎµåÇØ¼­ ¸Þ¸ð¸® È®º¸
	GetItemThumbnailMgr()->UnloadTextureTemporarily();

	// WPE hacking protect
	HMODULE hMod = GetModuleHandle( "ws2_32.dll"); 
	FARPROC RetVal = GetProcAddress( hMod, "recv"); 
	if ( (BYTE)RetVal == 0xE9)
	{
		mlog( "Hacking detected");

//		MessageBox(NULL, ZMsg(MSG_HACKING_DETECTED), ZMsg( MSG_WARNING), MB_OK);
		ZApplication::GetGameInterface()->ShowWidget("HackWarnings", true, true);

		ZPostDisconnect();
	}


	m_Camera.Init();
	ClearMapThumbnail();

	g_parts[6] = true;//Ä®À» Áã°í ½ÃÀÛ ÀÓ½Ã..

	//DrawLoadingScreen("Now Loading...", 0.0f);
	ZApplication::GetSoundEngine()->CloseMusic();

	m_bLoading = true;
	//m_pLoadingInterface = new ZLoading("loading", this, this);

	ZLoadingProgress gameLoading("Game");

	ZGetInitialLoading()->Initialize( 1, 0,0, RGetScreenWidth(), RGetScreenHeight(), 0,0, 1024, 768, true );

	// ·Îµù ÀÌ¹ÌÁö ·Îµå
	char szFileName[256];
	int nBitmap = rand() % 9;
	switch ( nBitmap)
	{
		case ( 0) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_dash.jpg");
			break;
		case ( 1) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_gaurd.jpg");
			break;
		case ( 2) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_ksa.jpg");
			break;
		case ( 3) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_safefall.jpg");
			break;
		case ( 4) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_tumbling.jpg");
			break;
		case ( 5) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_wallhang.jpg");
			break;
		case ( 6) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_walljump.jpg");
			break;
		case ( 7) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_wallrun01.jpg");
			break;
		case ( 8) :
			strcpy( szFileName, "Interface/Default/LOADING/loading_wallrun02.jpg");
			break;
		default :
			strcpy( szFileName, "");
			break;
	}
	switch ( rand() % 3)
	{
	case ( 0) :
		strcpy( szFileName, "Interface/Default/LOADING/loading_1.jpg");
		break;
	case ( 1) :
		strcpy( szFileName, "Interface/Default/LOADING/loading_2.jpg");
		break;
	case ( 2) :
		strcpy( szFileName, "Interface/Default/LOADING/loading_3.jpg");
		break;
	}

	ZGetCamera()->m_fDist = ZGetConfiguration()->GetCharacterZoomInInt() + 290;
	if ( !ZGetInitialLoading()->AddBitmap( 0, szFileName))
		ZGetInitialLoading()->AddBitmap( 0, "Interface/Default/LOADING/loading_teen.jpg");
	ZGetInitialLoading()->AddBitmapBar( "Interface/Default/LOADING/loading.bmp" );
	ZGetInitialLoading()->SetTipNum( nBitmap);

#ifndef _FASTDEBUG
	ZGetInitialLoading()->SetPercentage( 0.0f );
	ZGetInitialLoading()->Draw( MODE_FADEIN, 0 , true  );
#else
	ZGetInitialLoading()->SetPercentage( 10.f );
	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0, true );
#endif

	//m_pLoadingInterface->OnCreate();

	//m_pLoadingInterface->Progress( LOADING_BEGIN );
	//Redraw();	// Loading ScreenÀ¸·Î ´Ù½Ã ±×¸°´Ù.

	m_pGame=new ZGame;
	if(!m_pGame->Create(ZApplication::GetFileSystem(), &gameLoading ))
	{
		mlog("ZGame »ý¼º ½ÇÆÐ\n");
		SAFE_DELETE(m_pGame);
		m_bLoading = false;
		//m_pLoadingInterface->OnDestroy();
		//delete m_pLoadingInterface; m_pLoadingInterface = NULL;

		// ¸®¼Ò½º·Îµù ½ÇÆÐµîÀÇ ¹èÆ² ½ÃÀÛ ½ÇÆÐÇÒ½Ã ¼­¹ö¿¡ ½ºÅ×ÀÌÁö ¶°³­´Ù´Â ÆÐÅ¶À» º¸³»°í ·Îºñ·Î µ¹¾Æ°£´Ù.
		ZPostStageLeave(ZGetGameClient()->GetPlayerUID());
		ZApplication::GetGameInterface()->SetState(GUNZ_LOBBY);

		ZGetInitialLoading()->Release();
		
		return false;
	}

	/*
	if(m_spGameClient->IsForcedEntry())
		g_pGame->SetForcedEntry();
	*/

	m_pMyCharacter=(ZMyCharacter*)ZGetGame()->m_pMyCharacter;

	

	SetFocus();

	//ZGetInitialLoading()->SetPercentage( 100.f );

	m_pGameInput = new ZGameInput();

	m_pCombatInterface = new ZCombatInterface("combatinterface", this, this);
	m_pCombatInterface->SetBounds(GetRect());
	m_pCombatInterface->OnCreate();

	
	MWidget *pWidget = m_IDLResource.FindWidget("SkillFrame");
	if(pWidget!=NULL) pWidget->Show(true);

	// Skill List
	InitSkillList(m_IDLResource.FindWidget("SkillList"));

	pWidget = m_IDLResource.FindWidget("InventoryFrame");
	if(pWidget!=NULL) pWidget->Show(true);

	// Item List
	InitItemList(m_IDLResource.FindWidget("ItemList"));

	// Ä¿¼­ »ç¿ëÇÏÁö ¾ÊÀ½
	SetCursorEnable(false);

	m_bLoading = false;

#ifndef _FASTDEBUG
	ZGetInitialLoading()->SetPercentage( 100.0f) ;
	ZGetInitialLoading()->Draw( MODE_FADEOUT, 0 , true );
#endif
	ZGetInitialLoading()->Release();

	if (ZGetGameClient()->inPreGame) 
	{
		m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_STAGE_EXIT, false);
	}
	else
	{
		m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_STAGE_EXIT, true);
	}


	if( (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_REPLAY) ||
		(ZGetGameClient()->IsLadderGame()) || 
		ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT || ZGetGameClient()->inPreGame)
	{
		m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, false);
	}
	else
	{
		m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, true);
	}


#ifdef LOCALE_NHNUSA
	GetNHNUSAReport().ReportCreateGameScreen();
#endif

	//// µ¿¿µ»ó Ä¸ÃÄ...2008.10.02
#ifndef _DEBUG
	m_Capture = new ZBandiCapturer;
	m_Capture->Init(g_hWnd, (LPDIRECT3DDEVICE9)RGetDevice());
#endif


	return true;
}

void ZGameInterface::OnGameDestroy(void)
{
	mlog( "game interface destory begin.\n" );
	MPicture* pPicture;
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_ClanBitmap1");
	if(pPicture) pPicture->SetBitmap(NULL);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_ClanBitmap2");
	if(pPicture) pPicture->SetBitmap(NULL);

	MWidget *pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "GameResult");
	if ( pWidget) {
		MFrame *pFrame = (MFrame*)pWidget;
		pFrame->MFrame::Show( false);
	}

	ZGetGameClient()->RequestOnGameDestroyed();

	SAFE_DELETE(m_pMiniMap);

	if (m_pGameInput)
	{
		delete m_pGameInput; m_pGameInput = NULL;
//		Mint::GetInstance()->SetGlobalEvent(NULL);
	}

	if (m_pCombatInterface)
	{
		m_pCombatInterface->OnDestroy();
		delete m_pCombatInterface;
		m_pCombatInterface = NULL;
	}

	ShowWidget(CENTERMESSAGE, false);

	if(ZGetGame()!=NULL){
		ZGetGame()->Destroy();
		SAFE_DELETE(m_pGame);
	}

	SetCursorEnable(true);
	m_bLeaveBattleReserved = false;
	m_bLeaveStageReserved = false;

	// µ¿¿µ»ó Ä¸ÃÄ...2008.10.02
#ifndef _DEBUG
	SAFE_DELETE(m_Capture);
#endif

	mlog("game interface destroy finished\n");
}

void ZGameInterface::OnGreeterCreate(void)
{
	ShowWidget("Greeter", true);

	if ( m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDSKY);

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->OpenMusic( BGMID_INTRO, ZApplication::GetFileSystem());
}

void ZGameInterface::OnGreeterDestroy(void)
{
	ShowWidget("Greeter", false);

	if ( m_pBackground)
		m_pBackground->SetScene(LOGIN_SCENE_FALLDOWN);
}

void ZGameInterface::OnLoginCreate(void)
{
	// WPE hacking protect
	HMODULE hMod = GetModuleHandle( "ws2_32.dll"); 
	FARPROC RetVal = GetProcAddress( hMod, "recv"); 
	if ( (BYTE)RetVal == 0xE9)
	{
		mlog( "Hacking detected");

//		MessageBox(NULL, ZMsg(MSG_HACKING_DETECTED), ZMsg( MSG_WARNING), MB_OK);
		ZApplication::GetGameInterface()->ShowWidget("HackWarnings", true, true);

		ZPostDisconnect();
	}

	MComboBox* pComboBox = (MComboBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("MapTexLevel");
	if (pComboBox)
	{
		if (pComboBox->GetSelIndex() == 4)
		{
			RBspObject::SetWhiteMap(true);
			ZGetConfiguration()->GetVideo()->bLightMap = true;
		}
	}


	m_bLoginTimeout = false;
	m_nLoginState = LOGINSTATE_FADEIN;
	m_dwLoginTimer = timeGetTime();

	// ¹è°æ ÀÌ¹ÌÁö ·Îµù
	if ( m_pLoginBG != NULL)
	{
		delete m_pLoginBG;
		m_pLoginBG = NULL;
	}
	m_pLoginBG = new MBitmapR2;
	bool bRead = false;
	
	// ¿ÜºÎ ÆÄÀÏÀ» ÀÐ´Â´Ù.
#ifdef _DEBUG
	bRead = m_pLoginBG->Create( "loginbg.png", RGetDevice(), "wallpaper.jpg", false);
#else
	MZFile::SetReadMode( MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE );
	bRead = m_pLoginBG->Create( "loginbg.png", RGetDevice(), "wallpaper.jpg", false);
	MZFile::SetReadMode( MZIPREADFLAG_MRS2 );
#endif



    // ÆÐ³Î ÀÌ¹ÌÁö ·Îµù
	if ( m_pLoginPanel != NULL)
	{
		delete m_pLoginPanel;
		m_pLoginPanel = NULL;
	}

	m_pLoginPanel = new MBitmapR2;
	((MBitmapR2*)m_pLoginPanel)->Create( "loginpanel.png", RGetDevice(), "Interface/loadable/loginpanel.tga");
	if ( m_pLoginPanel)
	{
		// ÀÐ¾î¿Â ºñÆ®¸Ê ÀÌ¹ÌÁö Æ÷ÀÎÅÍ¸¦ ÇØ´ç À§Á¬¿¡ ³Ñ°ÜÁà¼­ Ç¥½ÃÇÑ´Ù
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget( "Login_Panel");
		if ( pPicture)
			pPicture->SetBitmap( m_pLoginPanel->GetSourceBitmap());
	}

	MButton* pLoginOk = (MButton*)m_IDLResource.FindWidget( "LoginOK");
	if (pLoginOk)
		pLoginOk->Enable(true);

	MWidget* pLoginFrame	= m_IDLResource.FindWidget( "LoginFrame");
	MWidget* pLoginBG		= m_IDLResource.FindWidget( "Login_BackgrdImg");
	if (pLoginFrame)
	{
		if ( pLoginBG)
			pLoginFrame->Show(false);
		else
			pLoginFrame->Show(true);
	}

	pLoginFrame = m_IDLResource.FindWidget( "Login_ConnectingMsg");
	if ( pLoginFrame)
		pLoginFrame->Show( false);

	MLabel* pErrorLabel = (MLabel*)m_IDLResource.FindWidget( "LoginError");
	if ( pErrorLabel)
		pErrorLabel->SetText( "");

	MLabel* pPasswd = (MLabel*)m_IDLResource.FindWidget( "LoginPassword");
	if ( pPasswd)
		pPasswd->SetText( "");
	// ³Ý¸¶ºíµµ ÀÌÁ¦ ¼­¹ö¸®½ºÆ®¸¦ º¸¿©ÁÖ±â ¶§¹®¿¡ ÁÖ¼®Ã³¸®
	// Netmarble ¿¡¼­ ·Î±äÇÑ °æ¿ì¿¡ Standalone LoginÀ» ¿ä±¸ÇÏ¸é °ÔÀÓÁ¾·á
	//if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_NETMARBLE) {
	//	mlog("Netmarble Logout \n");
	//	ZApplication::Exit();
	//	return;
	//}
	
	HideAllWidgets();

	ShowWidget("Login", true);
	//ShowWidget("LoginFrame", true);
	//BEGIN_WIDGETLIST("MaxHP", &m_IDLResource, MWidget*, pWidget);

	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget( "SelectedServer");
	if ( pServerList)
	{
		pServerList->ClearServerList();
		if ( strcmp( Z_LOCALE_DEFAULT_FONT, "Arial") == 0)
			pServerList->SetTextOffset( -2);

		// Å×½ºÆ®
//		pServerList->AddServer( "Billcap", "192.168.0.132", 6000, 2, 0, 1000, true);

	//		ShowWidget("LabelServerIP", true);
	//		ShowWidget("ServerAddress", true);
	//		ShowWidget("ServerPort", true);

	//		pServerList->AddServer( "Debug Server", "127.0.0.1", 6000, 4, 0, 1000, true);			// Debug server


#ifdef _LOCATOR
	
		if ( ZApplication::GetInstance()->IsLaunchTest())
		{
			if( m_pTLocatorList && (m_pTLocatorList->GetSize() > 0))
				m_nLocServ = rand() % m_pTLocatorList->GetSize();			// Ã³À½ ½ÃÀÛÇÏ´Â Å×½ºÆ® ¼­¹ö¸¦ ·£´ýÀ¸·Î ¼±ÅÃÇÑ´Ù.
		}
		else
		{
			if( m_pLocatorList && (m_pLocatorList->GetSize() > 0))
				m_nLocServ = rand() % m_pLocatorList->GetSize();			// Ã³À½ ½ÃÀÛÇÏ´Â ¼­¹ö¸¦ ·£´ýÀ¸·Î ¼±ÅÃÇÑ´Ù.
		}

		RequestServerStatusListInfo();

#else

		for ( int i = 0;  i < ZGetConfiguration()->GetServerCount();  i++)
		{
			ZSERVERNODE ServerNode = ZGetConfiguration()->GetServerNode( i);

			if ( ServerNode.nType != 1)
				pServerList->AddServer( ServerNode.szName, ServerNode.szAddress, ServerNode.nPort, ServerNode.nType, 0, 1000, true);
		}
#endif
	}


	MWidget* pWidget = m_IDLResource.FindWidget("LoginID");
	if(pWidget)
	{
		char buffer[256];
		if (ZGetApplication()->GetSystemValue("LoginID", buffer))
			pWidget->SetText(buffer);
			
	}
	MButton* bpWidget = (MButton*)GetIDLResource()->FindWidget("RememberMe");
	if (bpWidget) {
		char buffer[256];
		if (ZGetApplication()->GetSystemValue("RememberMe", buffer)) {
			if(!strcmp(buffer,"true")) {
				bpWidget->SetCheck(true);
					MWidget* pWidget = m_IDLResource.FindWidget("LoginPassword");
					if(pWidget) {
							char bufferr[256];
							if (ZGetApplication()->GetSystemValue("Password", bufferr))
								pWidget->SetText(bufferr);
					}
			} else {
				bpWidget->SetCheck(false);
			}
		}
	}
	if (pWidget != NULL && pWidget->GetText() != "" && pWidget->GetText() != NULL)
		pPasswd->SetFocus(); //set it to focused if username is set ~ Monckey100
	// ¼­¹ö IP
	pWidget = m_IDLResource.FindWidget("ServerAddress");
	if(pWidget)
	{
		pWidget->SetText(ZGetConfiguration()->GetServerIP());
#ifdef _DEBUG
		// µð¹ö±× ½Ã¿¡´Â ·ÎÄÃIP·Î ¼¼ÆÃÇØÁØ´Ù.....20090317 by kammir
		pWidget->SetText(m_spGameClient->GetUDPInfo()->GetAddress());
#endif
	}
	pWidget = m_IDLResource.FindWidget("ServerPort");
	if(pWidget)
	{
		char szText[ 25];
		sprintf( szText, "%d", ZGetConfiguration()->GetServerPort());
		pWidget->SetText( szText);
	}

	if ( m_pBackground)
	{
		m_pBackground->LoadMesh();
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDSKY);
	}

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->OpenMusic( BGMID_INTRO, ZApplication::GetFileSystem());

	if (m_spGameClient->IsConnected())
	{
		ZPostDisconnect();
	}

// ¾ÕÀ¸·Î ¸ðµç ÆÛºí¸®¼Å´Â ¼­¹ö¸®½ºÆ® È­¸éÀ» º¸¿©ÁØ´Ù´Â °¡Á¤À¸·Î ÁÖ¼®Ã³¸®
//#ifdef LOCALE_NHNUSA
//	m_IDLResource.FindWidget( "LoginID")->SetText( "GUNZ");
//	m_IDLResource.FindWidget( "LoginPassword")->SetText( "1234567890");
//#elif LOCALE_JAPAN

//#if defined(_DEBUG)	|| defined(_RELEASE)
		ShowWidget("Login_Logo", false);
		ShowWidget("Label_ID", true);
		ShowWidget("LoginID", true);
		ShowWidget("Label_Password", true);
		ShowWidget("LoginPassword", true);
//#else

		
		// ·Î±×ÀÎ ¹öÆ°À» È°¼ºÈ­ ÇÏ±âÀ§ÇÑ ¶«»§¿ë ID, PassWord
		//m_IDLResource.FindWidget( "LoginID")->SetText( "GUNZ");
		// m_IDLResource.FindWidget( "LoginPassword")->SetText( "1234567890");
//#endif
//#endif

#ifndef _PUBLISH
/*	// ÀÚµ¿Á¢¼ÓÇÏ°ÔÇÏ´ÂÅ×½ºÆ®
	m_IDLResource.FindWidget("LoginID")->SetText("n4");
	m_IDLResource.FindWidget("LoginPassword")->SetText("4");
	//m_IDLResource.FindWidget("ServerAddress")->SetText("192.168.0.79");
	ZPostConnect("192.168.0.60", 6000);	//*/
#endif
}
void ZGameInterface::OnLoginDestroy(void)
{
	ShowWidget("Login", false);

	MWidget* pWidget = m_IDLResource.FindWidget("LoginID");
	if(pWidget)
	{
		// ·Î±äÀÌ ¼º°øÇÏ¸é write ÇØ¾ß ÇÏ³ª.. Áö±Ý check out °ü°è·Î ¿©±â´Ù -_-;
		ZGetApplication()->SetSystemValue("LoginID", pWidget->GetText());

		if ( m_pBackground)
			m_pBackground->SetScene(LOGIN_SCENE_FALLDOWN);
	}
	MButton* bpWidget = (MButton*)GetIDLResource()->FindWidget("RememberMe");
	if(bpWidget) {
		if (bpWidget->GetCheck()) {
			ZGetApplication()->SetSystemValue("RememberMe", "true");
			MWidget* pWidget = m_IDLResource.FindWidget("LoginPassword");
			if(pWidget)
			 {
				ZGetApplication()->SetSystemValue("Password", pWidget->GetText());
			}
		} else 
			ZGetApplication()->SetSystemValue("RememberMe", "false");
	}
	// ¹è°æ ÀÌ¹ÌÁö¸¦ ¸Þ¸ð¸®·ÎºÎÅÍ »èÁ¦ÇÑ´Ù
	if ( m_pLoginBG != NULL)
	{
		// ¹è°æ ÀÌ¹ÌÁö¸¦ º¸¿©ÁÖ´Â À§Á¬ÀÇ ºñÆ®¸Ê ÀÌ¹ÌÁö Æ÷ÀÎÅÍ¸¦ ¸®¼ÂÇÑ´Ù
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget( "Login_BackgrdImg");
		if ( pPicture)
			pPicture->SetBitmap( NULL);
	
		delete m_pLoginBG;
		m_pLoginBG = NULL;
	}

	// ÆÐ³Î ÀÌ¹ÌÁö¸¦ ¸Þ¸ð¸®·ÎºÎÅÍ »èÁ¦ÇÑ´Ù
	if ( m_pLoginPanel != NULL)
	{
		// ÆÐ³Î ÀÌ¹ÌÁö¸¦ º¸¿©ÁÖ´Â À§Á¬ÀÇ ºñÆ®¸Ê ÀÌ¹ÌÁö Æ÷ÀÎÅÍ¸¦ ¸®¼ÂÇÑ´Ù
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget( "Login_Panel");
		if ( pPicture)
			pPicture->SetBitmap( NULL);
	
		delete m_pLoginPanel;
		m_pLoginPanel = NULL;
	}

	ZGetShop()->Destroy();
}

void ZGameInterface::OnDirectLoginCreate(void)
{
#ifdef LOCALE_KOREA

	OnNetmarbleLoginCreate();

#endif
}

void ZGameInterface::OnDirectLoginDestroy(void)
{
#ifdef LOCALE_KOREA

	OnNetmarbleLoginDestroy();

#endif
}

#include "ZNetmarble.h"
void ZGameInterface::OnNetmarbleLoginCreate(void)
{
	if ( m_pBackground)
	{
		m_pBackground->LoadMesh();
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDSKY);
	}

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->OpenMusic( BGMID_INTRO, ZApplication::GetFileSystem());

	if (m_spGameClient->IsConnected())
	{
		ZPostDisconnect();
	}

	HideAllWidgets();
	ShowWidget("NetmarbleLogin", true);

	ZBaseAuthInfo* pAuthInfo = ZGetLocale()->GetAuthInfo();
	if (pAuthInfo)
	{
#ifdef _DEBUG
		mlog("Connect to Netmarble GunzServer(IP:%s , Port:%d) \n", pAuthInfo->GetServerIP(), pAuthInfo->GetServerPort());
#endif
		ZPostConnect(pAuthInfo->GetServerIP(), pAuthInfo->GetServerPort());
	}
}

void ZGameInterface::OnNetmarbleLoginDestroy(void)
{
		if ( m_pBackground)
			m_pBackground->SetScene(LOGIN_SCENE_FALLDOWN);
}


void ZGameInterface::OnGameOnLoginCreate(void)
{
	if ( m_pBackground)
	{
		m_pBackground->LoadMesh();
		m_pBackground->SetScene(LOGIN_SCENE_FIXEDSKY);
	}

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->OpenMusic( BGMID_INTRO, ZApplication::GetFileSystem());

	if (m_spGameClient->IsConnected())
	{
		ZPostDisconnect();
	}

	HideAllWidgets();
	ShowWidget("NetmarbleLogin", true);

	ZBaseAuthInfo* pAuthInfo = ZGetLocale()->GetAuthInfo();
	if (pAuthInfo)
		ZPostConnect( pAuthInfo->GetServerIP(), pAuthInfo->GetServerPort());
}

void ZGameInterface::OnGameOnLoginDestroy(void)
{
		if ( m_pBackground)
			m_pBackground->SetScene(LOGIN_SCENE_FALLDOWN);
}


void ZGameInterface::OnLobbyCreate(void)
{
	/*if( m_pAmbSound != NULL )
	{
		m_pAmbSound->Stop();
		m_pAmbSound		= NULL;
	}
	//*/
	
	// ¸®ÇÃ·¹ÀÌ ÈÄ¿¡ ¹Ù²ï LevelPercent°ªÀ» ¿ø·¡´ë·Î º¹¿øÇÑ´Ù
	if ( m_bOnEndOfReplay)
	{
		m_bOnEndOfReplay = false;
		ZGetMyInfo()->SetLevelPercent( m_nLevelPercentCache);
	}

	// ¸®ÇÃ·¹ÀÌ ÈÄ¿¡ ¹Ù²ï LevelPercent°ªÀ» ¿ø·¡´ë·Î º¹¿øÇÑ´Ù
	if ( m_bOnEndOfReplay)
	{
		m_bOnEndOfReplay = false;
		ZGetMyInfo()->SetLevelPercent( m_nLevelPercentCache);
	}

	if( m_pBackground != 0 )
		m_pBackground->Free();	// Free Memory...

	if (m_spGameClient)
	{
		m_spGameClient->ClearPeers();
		m_spGameClient->ClearStageSetting();
	}

	SetRoomNoLight(1);
	ZGetGameClient()->RequestOnLobbyCreated();
	

	ShowWidget("CombatMenuFrame", false);
	ShowWidget("Lobby", true);
	EnableLobbyInterface(true);

	MWidget* pWidget = m_IDLResource.FindWidget("StageName");
	if(pWidget){
		char buffer[256];
		if (ZGetApplication()->GetSystemValue("StageName", buffer))
			pWidget->SetText(buffer);
	}

 	ZRoomListBox* pRoomList = (ZRoomListBox*)m_IDLResource.FindWidget("Lobby_StageList");
	if (pRoomList) pRoomList->Clear();

	ShowWidget("Lobby_StageList", true);
	/*
	ShowWidget("ChannelFrame", true);
	ShowWidget("StageListFrame", true);
	ShowWidget("StageFrame", true);
	*/

	MPicture* pPicture = 0;
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StripBottom");
 	if(pPicture != NULL)	pPicture->SetAnimation( 0, 1000.0f);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Lobby_StripTop");
	if(pPicture != NULL)	pPicture->SetAnimation( 1, 1000.0f);

    pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_RoomListBG");
	if ( pPicture)
	{
		m_pRoomListFrame = new MBitmapR2;
		((MBitmapR2*)m_pRoomListFrame)->Create( "gamelist_panel.png", RGetDevice(), "interface/loadable/gamelist_panel.png");

		if ( m_pRoomListFrame != NULL)
			pPicture->SetBitmap( m_pRoomListFrame->GetSourceBitmap());

		m_pDuelTournamentLobbyFrame = new MBitmapR2;
		((MBitmapR2*)m_pDuelTournamentLobbyFrame)->Create( "dueltournament_lobby_panel.png", RGetDevice(), "interface/loadable/dueltournament_lobby_panel.png");
	}
    pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_BottomBG");
	if ( pPicture)
	{
		m_pBottomFrame = new MBitmapR2;
		((MBitmapR2*)m_pBottomFrame)->Create( "bottom_panel.png", RGetDevice(), "interface/loadable/bottom_panel.png");

		if ( m_pBottomFrame != NULL)
			pPicture->SetBitmap( m_pBottomFrame->GetSourceBitmap());
	}

	m_pClanInfoBg = new MBitmapR2;
	((MBitmapR2*)m_pClanInfoBg)->Create( "claninfo_panel.tga", RGetDevice(), "interface/loadable/claninfo_panel.tga");
	if ( m_pClanInfoBg != NULL)
	{
		pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_ClanInfoBG");
		if ( pPicture)	pPicture->SetBitmap( m_pClanInfoBg->GetSourceBitmap());
	}

	m_pDuelTournamentInfoBg = new MBitmapR2;
	((MBitmapR2*)m_pDuelTournamentInfoBg)->Create( "dueltournamentinfo_panel.tga", RGetDevice(), "interface/loadable/dueltournamentinfo_panel.tga");
	if ( m_pDuelTournamentInfoBg != NULL)
	{
		pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_DuelTournamentInfoBG");
		if ( pPicture)	pPicture->SetBitmap( m_pDuelTournamentInfoBg->GetSourceBitmap());
	}

	m_pDuelTournamentRankingLabel = new MBitmapR2;
	((MBitmapR2*)m_pDuelTournamentRankingLabel)->Create( "DuelTournamentRankingLabel.png", RGetDevice(), "interface/loadable/DuelTournamentRankingLabel.png");
	if ( m_pDuelTournamentRankingLabel != NULL)
	{
		pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_DuelTournamentRankingListLabel");
		if ( pPicture)	pPicture->SetBitmap( m_pDuelTournamentRankingLabel->GetSourceBitmap());
	}

	ZDuelTournamentRankingListBox* pDTRankingListBox = (ZDuelTournamentRankingListBox*)m_IDLResource.FindWidget("Lobby_DuelTournamentRankingList");
	if (pDTRankingListBox) pDTRankingListBox->LoadInterfaceImgs();

	// music
#ifdef _BIRDSOUND
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY);
	ZApplication::GetSoundEngine()->PlayMusic( true);
#else
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY, ZApplication::GetFileSystem());
	ZApplication::GetSoundEngine()->PlayMusic( true);
#endif

//	m_pBackground->SetScene(SCENE_TOWN);

	//ZCharacterViewList* pCharacterViewList = NULL;
	//pCharacterViewList = ZGetCharacterViewList(GUNZ_STAGE);
	//if (pCharacterViewList != NULL)
	//{
	//	pCharacterViewList->RemoveAll();
	//}
	//pCharacterViewList = ZGetCharacterViewList(GUNZ_LOBBY);
	//if (pCharacterViewList != NULL)
	//{
	//	pCharacterViewList->Assign(ZGetGameClient()->GetObjCacheMap());
	//}

//	SetupPlayerListTab();

	ZPlayerListBox *pPlayerListBox = (ZPlayerListBox*)m_IDLResource.FindWidget( "LobbyChannelPlayerList" );
	if(pPlayerListBox)
		pPlayerListBox->SetMode(ZPlayerListBox::PLAYERLISTMODE_CHANNEL);

//	InitLadderUI();

	// Ã¤ÆÃÀÔ·Â¿¡ Æ÷Ä¿½º¸¦ ¸ÂÃçÁØ´Ù.
	pWidget= m_IDLResource.FindWidget( "ChannelChattingInput" );
	if(pWidget) pWidget->SetFocus();

	if ( m_pBackground)
		m_pBackground->SetScene( LOGIN_SCENE_FIXEDCHAR);


	// UI ¾÷µ¥ÀÌÆ®
	bool bEnable = ZGetGameClient()->GetEnableInterface();

	pWidget = m_IDLResource.FindWidget( "StageJoin");
	if ( pWidget)		pWidget->Enable( bEnable);

	pWidget = m_IDLResource.FindWidget( "StageCreateFrameCaller");
	if ( pWidget)		pWidget->Enable( bEnable);
	
	pWidget = m_IDLResource.FindWidget( "QuickJoin");
	if ( pWidget)		pWidget->Enable( bEnable);
	
	pWidget = m_IDLResource.FindWidget( "QuickJoin2");
	if ( pWidget)		pWidget->Enable( bEnable);
	
	pWidget = m_IDLResource.FindWidget( "ArrangedTeamGame");
	if ( pWidget)		pWidget->Enable( bEnable);

	pWidget = m_IDLResource.FindWidget( "ArrangedTeamGameWarmUp");
	if ( pWidget)		pWidget->Enable( bEnable);

	pWidget = m_IDLResource.FindWidget( "ChannelChattingInput");
	if ( pWidget)		pWidget->Enable( bEnable);

	pWidget = m_IDLResource.FindWidget( "Lobby_StageList");
	if ( pWidget)		pWidget->Enable( bEnable);

	InitLobbyUIByChannelType();
}

void ZGameInterface::InitLobbyUIByChannelType()
{
	bool bClanBattleUI =  ((ZGetGameClient()->GetServerMode() == MSM_CLAN) && (ZGetGameClient()->GetChannelType()==MCHANNEL_TYPE_CLAN));
	bool bDuelTournamentUI = (ZGetGameClient()->GetChannelType() == MCHANNEL_TYPE_DUELTOURNAMENT);

	// ¼ø¼­°¡ Áß¿äÇÏ´Ù.. trueÀÎÂÊÀ» ³ªÁß¿¡ È£ÃâÇØÁà¾ß ÇÑ´Ù (ÃßÇÏÁö¸¸ ÀÌÁ¦¿Í¼­ ÇÔ¼ö ÇÑ±ºµ¥·Î ÇÕÃÄÁÖ±âµµ Á» µÎ·Á¿î...)
	if (bClanBattleUI)
	{
		ZGetGameInterface()->InitDuelTournamentLobbyUI(false);
		ZGetGameInterface()->InitClanLobbyUI(true);
	}
	else if (bDuelTournamentUI)
	{
		ZGetGameInterface()->InitClanLobbyUI(false);
		ZGetGameInterface()->InitDuelTournamentLobbyUI(true);

		ZPostDuelTournamentRequestSideRankingInfo( ZGetMyUID());
	}
	else
	{
		ZGetGameInterface()->InitClanLobbyUI(false);
		ZGetGameInterface()->InitDuelTournamentLobbyUI(false);
	}

	if (bDuelTournamentUI)
		ZPostDuelTournamentRequestSideRankingInfo( ZGetMyUID());
}


void ZGameInterface::OnLobbyDestroy(void)
{
	ShowWidget("Lobby", false);

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_RoomListBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_BottomBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);
    
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_ClanInfoBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_DuelTournamentInfoBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Lobby_DuelTournamentRankingListLabel");
	if ( pPicture)
		pPicture->SetBitmap( NULL);


	SAFE_DELETE(m_pRoomListFrame);
	SAFE_DELETE(m_pDuelTournamentLobbyFrame);
	SAFE_DELETE(m_pBottomFrame);
	SAFE_DELETE(m_pClanInfoBg);
	SAFE_DELETE(m_pDuelTournamentInfoBg);
	SAFE_DELETE(m_pDuelTournamentRankingLabel);

	ZDuelTournamentRankingListBox* pDTRankingListBox = (ZDuelTournamentRankingListBox*)m_IDLResource.FindWidget("Lobby_DuelTournamentRankingList");
	if (pDTRankingListBox) {
		pDTRankingListBox->UnloadInterfaceImgs();
		pDTRankingListBox->SetVisible(false);
	}

	MWidget* pWidget = m_IDLResource.FindWidget("StageName");
	if(pWidget) ZGetApplication()->SetSystemValue("StageName", pWidget->GetText());
}

void ZGameInterface::OnStageCreate(void)
{
	// WPE hacking protect
	HMODULE hMod = GetModuleHandle( "ws2_32.dll"); 
	FARPROC RetVal = GetProcAddress( hMod, "recv"); 
	if ( ZCheckHackProcess() || (BYTE)RetVal == 0xE9)
	{
		mlog( "Hacking detected");

//		MessageBox(NULL, ZMsg(MSG_HACKING_DETECTED), ZMsg( MSG_WARNING), MB_OK);
		ZApplication::GetGameInterface()->ShowWidget("HackWarnings", true, true);

		ZPostDisconnect();
	}


	mlog("StageCreated\n");

	if (m_spGameClient)
	{
		m_spGameClient->ClearPeers();
	}

	ShowWidget("Shop", false);
	ShowWidget("Equipment", false);
	ShowWidget("Stage", true);
	EnableStageInterface(true);
	MButton* pObserverBtn = (MButton*)m_IDLResource.FindWidget("StageObserverBtn");
	if ( pObserverBtn)
		pObserverBtn->SetCheck( false);

	/*
	MListBox* pListBox  = (MListBox*)m_IDLResource.FindWidget("StageChattingOutput");
	if (pListBox != NULL)
	{
		pListBox->RemoveAll();		
	}
	*/

	//ZCharacterViewList* pCharacterViewList = NULL;
	//pCharacterViewList = ZGetCharacterViewList(GUNZ_LOBBY);
	//if (pCharacterViewList != NULL)
	//{
	//	pCharacterViewList->RemoveAll();
	//}
	//pCharacterViewList = ZGetCharacterViewList(GUNZ_STAGE);
	//if (pCharacterViewList != NULL)
	//{
	//	pCharacterViewList->Assign(ZGetGameClient()->GetObjCacheMap());
	//}

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	
	if( pCharView != NULL )
	{
		//MMatchObjCacheMap* pObjCacheMap = ZGetGameClient()->GetObjCacheMap();
		//for(MMatchObjCacheMap::iterator itor = pObjCacheMap->begin(); itor != pObjCacheMap->end(); ++itor)
		//{
		//	MMatchObjCache* pObj = (*itor).second;
		//	if( pObj->GetUID() == ZGetMyUID() )
		//	{
		//		pCharView->SetCharacter( pObj->GetUID() );
		//	}
		//}
		pCharView->SetCharacter( ZGetMyUID());
	}

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripBottom");
	if(pPicture != NULL)	pPicture->SetBitmapColor(0xFFFFFFFF);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripTop");
	if(pPicture != NULL)	pPicture->SetBitmapColor(0xFFFFFFFF);

	ZPostRequestStageSetting(ZGetGameClient()->GetStageUID());
	SerializeStageInterface();

#ifdef _BIRDSOUND
		ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY);
		ZApplication::GetSoundEngine()->PlayMusic(true);
#else
		ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY, ZApplication::GetFileSystem());
		ZApplication::GetSoundEngine()->PlayMusic(true);
#endif
/*
#ifdef _BIRDSOUND
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY);
	ZApplication::GetSoundEngine()->PlayMusic();
#else
#ifndef _FASTDEBUG
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_LOBBY, ZApplication::GetFileSystem());
	ZApplication::GetSoundEngine()->PlayMusic();
#endif
#endif
*/
/*
	// ¹æÀåÀÌ¸é ÇöÀç °ÔÀÓ ¼³Á¤À» ´Ù½Ã ¼³Á¤ÇÑ´Ù.
	if ( ZGetGameClient() && ZGetGameClient()->AmIStageMaster())
	{
		MChannelRule* pRule = ZGetChannelRuleMgr()->GetRule( ZGetGameClient()->GetChannelRuleName());
		if ( pRule)
		{
			MComboBox* pCB = (MComboBox*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("StageType");				
			if ( pCB)
			{
				pCB->SetSelIndex( pRule->GetDefault());

				ZStageSetting::InitStageSettingGameFromGameType();
				ZStageSetting::PostDataToServer();
			}
		}
	}
*/
	ZApplication::GetStageInterface()->OnCreate();
}

void ZGameInterface::OnStageDestroy(void)
{
	ZApplication::GetStageInterface()->OnDestroy();
}

char* GetItemSlotName( const char* szName, int nItem)
{
	static char szTemp[256];
	strcpy( szTemp, szName);

	switch (nItem)
	{
		case MMCIP_HEAD :		strcat( szTemp, "_EquipmentSlot_Head");			break;
		case MMCIP_CHEST :		strcat( szTemp, "_EquipmentSlot_Chest");		break;
		case MMCIP_HANDS :		strcat( szTemp, "_EquipmentSlot_Hands");		break;
		case MMCIP_LEGS :		strcat( szTemp, "_EquipmentSlot_Legs");			break;
		case MMCIP_FEET :		strcat( szTemp, "_EquipmentSlot_Feet");			break;
		case MMCIP_FINGERL :	strcat( szTemp, "_EquipmentSlot_FingerL");		break;
		case MMCIP_FINGERR :	strcat( szTemp, "_EquipmentSlot_FingerR");		break;
		case MMCIP_AVATAR :		strcat( szTemp, "_EquipmentSlot_Avatar");		break;
#ifdef _GZFGunz
		case MMCIP_DANCE:		strcat( szTemp, "_EquipmentSlot_Dance");		break;
		case MMCIP_DASH:		strcat(szTemp,  "_EquipmentSlot_Dash");			break;
#endif
		case MMCIP_MELEE :		strcat( szTemp, "_EquipmentSlot_Melee");		break;
		case MMCIP_PRIMARY :	strcat( szTemp, "_EquipmentSlot_Primary");		break;
		case MMCIP_SECONDARY :	strcat( szTemp, "_EquipmentSlot_Secondary");	break;
		case MMCIP_CUSTOM1 :	strcat( szTemp, "_EquipmentSlot_Custom1");		break;
		case MMCIP_CUSTOM2 :	strcat( szTemp, "_EquipmentSlot_Custom2");		break;
		case MMCIP_COMMUNITY1:	strcat( szTemp, "_EquipmentSlot_Community1");	break;
		case MMCIP_COMMUNITY2:	strcat( szTemp, "_EquipmentSlot_Community2");	break;
		case MMCIP_LONGBUFF1 :	strcat( szTemp, "_EquipmentSlot_LongBuff1");	break;
		case MMCIP_LONGBUFF2 :	strcat( szTemp, "_EquipmentSlot_LongBuff2");	break;

		default: break;
	}

	return szTemp;
}

bool ZGameInterface::OnCreate(ZLoadingProgress *pLoadingProgress)
{
	if (!m_sbRemainClientConnectionForResetApp)
		m_spGameClient = new ZGameClient();

	if(!m_Tips.Initialize(ZApplication::GetFileSystem(), ZGetLocale()->GetLanguage())) {
		mlog("Check tips.xml\n");
		return false;
	}
	
	ZLoadingProgress interfaceProgress("interfaceSkin",pLoadingProgress,.7f);
	if(!InitInterface(ZGetConfiguration()->GetInterfaceSkinName(),&interfaceProgress))
	{
		mlog("ZGameInterface::OnCreate: Failed InitInterface\n");
		return false;
	}

	interfaceProgress.UpdateAndDraw(1.f);

	/*
	if( !InitLocatorList(ZApplication::GetFileSystem(), "system/ZLocatorList.xml") )
	{
		mlog( "ZGameInterface::OnCreate: Fail InitLocatorList\n" );
		// ·ÎÄÉÀÌÅÍ¸¦ ¸ø ÀÐ¾îµµ °ÔÀÓÀº ÁøÇàÀ» ÇÔ. 
		// return false;
	}
	*/
	

	// mlog("ZGameInterface::OnCreate : InitInterface \n");

	m_pScreenEffectManager=new ZScreenEffectManager;
	if(!m_pScreenEffectManager->Create()) 
		return false;

	mlog("Screen effect manager create success.\n" );

	m_pEffectManager = new ZEffectManager;
	if(!m_pEffectManager->Create())
		return false;

	mlog("Effect manager create success.\n");
	
	//m_pCursorSurface=CreateImageSurface("Interface/Default/cursor.png");
	//if( m_pCursorSurface != NULL )
		//RGetDevice()->SetCursorProperties(0,0,m_pCursorSurface);

//	ResetCursor();

	SetTeenVersion(ZGetLocale()->IsTeenMode());

	if (!m_sbRemainClientConnectionForResetApp)
	{
		int nNetworkPort = RandomNumber( ZGetConfiguration()->GetEtc()->nNetworkPort1, ZGetConfiguration()->GetEtc()->nNetworkPort2);
		if (m_spGameClient->Create( nNetworkPort) == false) {
			string strMsg = "Unknown Network Error";
			if (GetLastError() == WSAEADDRINUSE)
				NotifyMessage(MATCHNOTIFY_NETWORK_PORTINUSE, &strMsg);
			else
				NotifyMessage(MATCHNOTIFY_NETWORK_CREATE_FAILED, &strMsg);
			ShowMessage(strMsg.c_str());
		}
		m_spGameClient->SetOnCommandCallback(OnCommand);
		m_spGameClient->CreateUPnP(nNetworkPort);
		m_MyPort = nNetworkPort;
			
		mlog("Client create success.\n");
	}

//	SetState(m_nInitialState);

	// »óÁ¡ ¹× Àåºñ ÃÊ±âÈ­ ¼³Á¤
	ZItemSlotView* itemSlot;
	for(int i = 0;  i < MMCIP_END;  i++)
	{
		itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget( GetItemSlotName( "Shop", i));
		if ( itemSlot)
		{
			strcpy( itemSlot->m_szItemSlotPlace, "Shop");

			// ¾ÆÁÖ Àú·¾ ÇÏµåÄÚµù... -_-;
			switch ( itemSlot->GetParts())
			{
			case MMCIP_HEAD:		itemSlot->SetText( ZMsg( MSG_WORD_HEAD));				break;
			case MMCIP_CHEST:		itemSlot->SetText( ZMsg( MSG_WORD_CHEST));				break;
			case MMCIP_HANDS:		itemSlot->SetText( ZMsg( MSG_WORD_HANDS));				break;
			case MMCIP_LEGS:		itemSlot->SetText( ZMsg( MSG_WORD_LEGS));				break;
			case MMCIP_FEET:		itemSlot->SetText( ZMsg( MSG_WORD_FEET));				break;
			case MMCIP_FINGERL:		itemSlot->SetText( ZMsg( MSG_WORD_LFINGER));			break;
			case MMCIP_FINGERR:		itemSlot->SetText( ZMsg( MSG_WORD_RFINGER));			break;
			case MMCIP_AVATAR:		itemSlot->SetText( ZMsg( MSG_WORD_AVATAR));				break;
			case MMCIP_DANCE:		itemSlot->SetText( ZMsg( MSG_WORD_DANCE));				break;
			case MMCIP_DASH:		itemSlot->SetText( ZMsg( MSG_WORD_DASH));				break;
			case MMCIP_MELEE:		itemSlot->SetText( ZMsg( MSG_WORD_MELEE));				break;
			case MMCIP_PRIMARY:		itemSlot->SetText( ZMsg( MSG_WORD_WEAPON1));			break;
			case MMCIP_SECONDARY:	itemSlot->SetText( ZMsg( MSG_WORD_WEAPON2));			break;
			case MMCIP_CUSTOM1:		itemSlot->SetText( ZMsg( MSG_WORD_ITEM1));				break;
			case MMCIP_CUSTOM2:		itemSlot->SetText( ZMsg( MSG_WORD_ITEM2));				break;
			case MMCIP_COMMUNITY1:	itemSlot->SetText( ZMsg( MSG_WORD_COMMUNITY_ITEM1));	break;
			case MMCIP_COMMUNITY2:	itemSlot->SetText( ZMsg( MSG_WORD_COMMUNITY_ITEM2));	break;
			case MMCIP_LONGBUFF1:	itemSlot->SetText( ZMsg( MSG_WORD_LONGBUFF_ITEM1));		break;
			case MMCIP_LONGBUFF2:	itemSlot->SetText( ZMsg( MSG_WORD_LONGBUFF_ITEM2));		break;
			default :				itemSlot->SetText("");									break;

			}
		}
	}
	for(int i = 0;  i < MMCIP_END;  i++)
	{
		itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget( GetItemSlotName( "Equip", i));
		if ( itemSlot)
		{
			strcpy( itemSlot->m_szItemSlotPlace, "Equip");

			// ¾ÆÁÖ Àú·¾ ÇÏµåÄÚµù... -_-;
			switch ( itemSlot->GetParts() )
			{
			case MMCIP_HEAD:		itemSlot->SetText( ZMsg( MSG_WORD_HEAD));				break;
			case MMCIP_CHEST:		itemSlot->SetText( ZMsg( MSG_WORD_CHEST));				break;
			case MMCIP_HANDS:		itemSlot->SetText( ZMsg( MSG_WORD_HANDS));				break;
			case MMCIP_LEGS:		itemSlot->SetText( ZMsg( MSG_WORD_LEGS));				break;
			case MMCIP_FEET:		itemSlot->SetText( ZMsg( MSG_WORD_FEET));				break;
			case MMCIP_FINGERL:		itemSlot->SetText( ZMsg( MSG_WORD_LFINGER));			break;
			case MMCIP_FINGERR:		itemSlot->SetText( ZMsg( MSG_WORD_RFINGER));			break;
			case MMCIP_AVATAR:		itemSlot->SetText( ZMsg( MSG_WORD_AVATAR));				break;
			case MMCIP_DANCE:		itemSlot->SetText( ZMsg( MSG_WORD_DANCE));				break;
			case MMCIP_DASH:		itemSlot->SetText( ZMsg(MSG_WORD_DASH));				break;
			case MMCIP_MELEE:		itemSlot->SetText( ZMsg( MSG_WORD_MELEE));				break;
			case MMCIP_PRIMARY:		itemSlot->SetText( ZMsg( MSG_WORD_WEAPON1));			break;
			case MMCIP_SECONDARY:	itemSlot->SetText( ZMsg( MSG_WORD_WEAPON2));			break;
			case MMCIP_CUSTOM1:		itemSlot->SetText( ZMsg( MSG_WORD_ITEM1));				break;
			case MMCIP_CUSTOM2:		itemSlot->SetText( ZMsg( MSG_WORD_ITEM2));				break;
			case MMCIP_COMMUNITY1:	itemSlot->SetText( ZMsg( MSG_WORD_COMMUNITY_ITEM1));	break;
			case MMCIP_COMMUNITY2:	itemSlot->SetText( ZMsg( MSG_WORD_COMMUNITY_ITEM2));	break;
			case MMCIP_LONGBUFF1:	itemSlot->SetText( ZMsg( MSG_WORD_LONGBUFF_ITEM1));		break;
			case MMCIP_LONGBUFF2:	itemSlot->SetText( ZMsg( MSG_WORD_LONGBUFF_ITEM2));		break;
			default :				itemSlot->SetText("");									break;
			}
		}
	}
	ZShopEquipListbox* pItemListBox = (ZShopEquipListbox*)m_IDLResource.FindWidget("EquipmentList");
	if ( pItemListBox)
	{
		//¾ÆÀÌÅÛ ¿ìÅ¬¸¯ ¸Þ´º Áö±ÝÀº ¾È¾²´Ï±î ÁÖ¼®
		//pItemListBox->AttachMenu(new ZItemMenu("ItemMenu", pItemListBox, pItemListBox, MPMT_VERTICAL));	
		pItemListBox->EnableDragAndDrop( true);
	}

	// ¸®ÇÃ·¹ÀÌ ¹Ú½º ÃÊ±âÈ­
	MListBox* pReplayBox = (MListBox*)m_IDLResource.FindWidget( "Replay_FileList");
	if ( pReplayBox)
	{
		pReplayBox->m_FontAlign = MAM_VCENTER;
		pReplayBox->SetVisibleHeader( false);
		pReplayBox->AddField( "NAME", 92);
		pReplayBox->AddField( "VERSION", 8);
		pReplayBox->m_bAbsoulteTabSpacing = false;
	}


	// Setting Configuration about ZGameClient
	if (Z_ETC_BOOST && RIsActive())
		m_spGameClient->PriorityBoost(true);

	m_spGameClient->SetRejectNormalChat(Z_ETC_REJECT_NORMALCHAT);
	m_spGameClient->SetRejectTeamChat(Z_ETC_REJECT_TEAMCHAT);
	m_spGameClient->SetRejectClanChat(Z_ETC_REJECT_CLANCHAT);
	m_spGameClient->SetRejectWhisper(Z_ETC_REJECT_WHISPER);
	m_spGameClient->SetRejectInvite(Z_ETC_REJECT_INVITE);


	// °á°úÈ­¸é ¸®½ºÆ®¹Ú½º ÃÊ±âÈ­
	int nLineHeightTextArea = int(18/600.f * MGetWorkspaceHeight());	// 800*600 ÇØ»óµµ ±âÁØÀ¸·Î 18ÇÈ¼¿
	MTextArea* pTextArea = (MTextArea*)m_IDLResource.FindWidget( "CombatResult_PlayerNameList");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "CombatResult_PlayerKillList");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "CombatResult_PlayerDeathList");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "ClanResult_PlayerNameList1");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "ClanResult_PlayerKillList1");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "ClanResult_PlayerDeathList1");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "ClanResult_PlayerNameList2");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "ClanResult_PlayerKillList2");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "ClanResult_PlayerDeathList2");
	if ( pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget( "CombatResult_Header");
	if ( pPicture)
		pPicture->SetOpacity( 80);
	pPicture = (MPicture*)m_IDLResource.FindWidget( "ClanResult_Header1");
	if ( pPicture)
		pPicture->SetOpacity( 80);
	pPicture = (MPicture*)m_IDLResource.FindWidget( "ClanResult_Header2");
	if ( pPicture)
		pPicture->SetOpacity( 80);

	// ¸±·¹ÀÌ ¸Ê
	{
		MComboBox* pCombo = (MComboBox*)m_IDLResource.FindWidget( "Stage_RelayMapType" );
		if ( pCombo)
		{
			pCombo->RemoveAll();
			pCombo->Add(ZMsg( MSG_GAME_RELAYMAP_TURN));
			pCombo->Add(ZMsg( MSG_GAME_RELAYMAP_RANDOM));
			pCombo->SetSelIndex(0);
		}
		pCombo = (MComboBox*)m_IDLResource.FindWidget( "Stage_RelayMapRepeatCount" );
		if ( pCombo)
		{
			pCombo->RemoveAll();
			pCombo->Add(ZMsg( MSG_GAME_RELAYMAP_1ROUND));
			pCombo->Add(ZMsg( MSG_GAME_RELAYMAP_2ROUND));
			pCombo->Add(ZMsg( MSG_GAME_RELAYMAP_3ROUND));
			pCombo->Add(ZMsg( MSG_GAME_RELAYMAP_4ROUND));
			pCombo->Add(ZMsg( MSG_GAME_RELAYMAP_5ROUND));
			pCombo->SetSelIndex(2);
		}

		// ¸Ê ¸®½ºÆ® ¹Ú½º ÃÊ±âÈ­
		MListBox* pMapListBox = (MListBox*)m_IDLResource.FindWidget( "Stage_MapListbox" );
		if ( pMapListBox)
		{
			pMapListBox->m_FontAlign = MAM_LEFT;
			pMapListBox->AddField( "ICON", 23);
			pMapListBox->AddField( "NAME", 170);
			pMapListBox->SetItemHeight( 23);
			pMapListBox->SetVisibleHeader( false);
			pMapListBox->m_bNullFrame = true;
		}
		// ¸±·¹ÀÌ¸Ê ¸®½ºÆ® ¹Ú½º ÃÊ±âÈ­
		MListBox* pRelayMapListBox = (MListBox*)m_IDLResource.FindWidget( "Stage_RelayMapListbox" );
		if ( pRelayMapListBox)
		{
			pRelayMapListBox->m_FontAlign = MAM_LEFT;
			pRelayMapListBox->AddField( "ICON", 23);
			pRelayMapListBox->AddField( "NAME", 170);
			pRelayMapListBox->SetItemHeight( 23);
			pRelayMapListBox->SetVisibleHeader( false);
			pRelayMapListBox->m_bNullFrame = true;
		}
	}

	// Èñ»ý ¾ÆÀÌÅÛ ¸®½ºÆ® ¹Ú½º ÃÊ±âÈ­
	MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget( "Stage_SacrificeItemListbox");
	if ( pListBox)
	{
		pListBox->m_FontAlign = MAM_VCENTER;
		pListBox->AddField( "ICON", 32);
		pListBox->AddField( "NAME", 170);
		pListBox->SetItemHeight( 32);
		pListBox->SetVisibleHeader( false);
		pListBox->m_bNullFrame = true;
		pListBox->EnableDragAndDrop( true);
		pListBox->SetOnDropCallback( OnDropCallbackRemoveSacrificeItem);
	}

	// Èñ»ý ¾ÆÀÌÅÛ ½½·Ô ÃÊ±âÈ­
	itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget( "Stage_SacrificeItemButton0");
	if ( itemSlot)
	{
		itemSlot->EnableDragAndDrop( true);
		strcpy( itemSlot->m_szItemSlotPlace, "SACRIFICE0");
	}
	itemSlot = (ZItemSlotView*)m_IDLResource.FindWidget( "Stage_SacrificeItemButton1");
	if ( itemSlot)
	{
		itemSlot->EnableDragAndDrop( true);
		strcpy( itemSlot->m_szItemSlotPlace, "SACRIFICE1");
	}

	// È¹µæ ¾ÆÀÌÅÛ ¸®½ºÆ® ¹Ú½º ÃÊ±âÈ­
	pListBox = (MListBox*)m_IDLResource.FindWidget( "QuestResult_ItemListbox");
	if ( pListBox)
	{
		pListBox->m_FontAlign = MAM_VCENTER;
		pListBox->AddField( "ICON", 35);
		pListBox->AddField( "NAME", 300);
		pListBox->SetItemHeight( 32);
		pListBox->SetVisibleHeader( false);
		pListBox->SetFont( MFontManager::Get( "FONTa10b"));
		pListBox->m_FontColor = MCOLOR( 0xFFFFF794);
		pListBox->m_bNullFrame = true;
	}

	// ¼­¹ÙÀÌ¹ú ·©Å· ¸®½ºÆ® ¹Ú½º ÃÊ±âÈ­
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "SurvivalResult_MyRank");
	if (pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "SurvivalResult_MyName");
	if (pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "SurvivalResult_MyRankPoint");
	if (pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "SurvivalResult_PlayerRankList");
	if (pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "SurvivalResult_PlayerNameList");
	if (pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}
	pTextArea = (MTextArea*)m_IDLResource.FindWidget( "SurvivalResult_PlayerRankPointList");
	if (pTextArea)
	{
		pTextArea->SetFont( MFontManager::Get( "FONTa10b"));
		pTextArea->SetCustomLineHeight( nLineHeightTextArea);
	}

	// BmNumLabel ÃÊ±âÈ­
	int nMargin[ BMNUM_NUMOFCHARSET] = { 15,15,15,15,15,15,15,15,15,15,8,10,8 };
	int nMarginNarrow[ BMNUM_NUMOFCHARSET] = { 12,12,12,12,12,12,12,12,12,12,8,10,8 };
	ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_ClanInfoWinLose");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_ClanInfoPoints");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_ClanInfoTotalPoints");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_ClanInfoRanking");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_DuelTournamentInfoWinLose");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_DuelTournamentInfoPoints");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_DuelTournamentInfoWinners");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_DuelTournamentInfoRanking");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_DuelTournamentInfoWinLosePrev");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_DuelTournamentInfoPointsPrev");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_DuelTournamentInfoWinnersPrev");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "Lobby_DuelTournamentInfoRankingPrev");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMarginNarrow);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "QuestResult_GetPlusXP");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "QuestResult_GetMinusXP");
	if ( pBmNumLabel)
	{
		pBmNumLabel->SetCharMargin( nMargin);
		pBmNumLabel->SetIndexOffset( 16);
	}
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "QuestResult_GetTotalXP");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "QuestResult_GetBounty");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "SurvivalResult_GetReachedRound");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "SurvivalResult_GetPoint");
	if ( pBmNumLabel)
		pBmNumLabel->SetCharMargin( nMargin);
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "SurvivalResult_GetXP");
	if ( pBmNumLabel)
	{
		pBmNumLabel->SetCharMargin( nMargin);
		pBmNumLabel->SetIndexOffset( 16);
	}
	pBmNumLabel = (ZBmNumLabel*)m_IDLResource.FindWidget( "SurvivalResult_GetBounty");
	if ( pBmNumLabel)
	{
		pBmNumLabel->SetCharMargin( nMargin);
		pBmNumLabel->SetIndexOffset( 16);
	}


	// ¸ÅÄ¡ Å¸ÀÔÀ» ´Ù½Ã ÀÐ¾îµéÀÎ´Ù.
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_DEATHMATCH_SOLO, ZMsg( MSG_MT_DEATHMATCH_SOLO));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_DEATHMATCH_TEAM, ZMsg( MSG_MT_DEATHMATCH_TEAM));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_GLADIATOR_SOLO, ZMsg( MSG_MT_GLADIATOR_SOLO));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_GLADIATOR_TEAM, ZMsg( MSG_MT_GLADIATOR_TEAM));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_ASSASSINATE, ZMsg( MSG_MT_ASSASSINATE));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_TRAINING, ZMsg( MSG_MT_TRAINING));
//	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_CLASSIC_SOLO, ZMsg( MSG_MT_CLASSIC_SOLO));
//	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_CLASSIC_TEAM, ZMsg( MSG_MT_CLASSIC_TEAM));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_SURVIVAL, ZMsg( MSG_MT_SURVIVAL));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_QUEST, ZMsg( MSG_MT_QUEST));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_BERSERKER, ZMsg( MSG_MT_BERSERKER));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_DEATHMATCH_TEAM2, ZMsg( MSG_MT_DEATHMATCH_TEAM2));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_DUEL, ZMsg( MSG_MT_DUEL));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_DUELTOURNAMENT, ZMsg( MSG_MT_DUELTOURNAMENT));
	ZGetGameTypeManager()->SetGameTypeStr( MMATCH_GAMETYPE_CTF, ZMsg( MSG_MT_CTF));

#ifndef _DEBUG
	MWidget* pWidget = m_IDLResource.FindWidget( "MonsterBookCaller");
	if ( pWidget)
		pWidget->Show( false);
#endif

#ifndef _QUEST_ITEM// Äù½ºÆ® ¾ÆÀÌÅÛ Ä«Å×°í¸®¸¦ Á¦°ÅÇÑ´Ù!
	MComboBox* pComboBox = (MComboBox*)m_IDLResource.FindWidget( "Shop_AllEquipmentFilter");
	if ( pComboBox)
		pComboBox->Remove( 10);
	pComboBox = (MComboBox*)m_IDLResource.FindWidget( "Equip_AllEquipmentFilter");
	if ( pComboBox)
		pComboBox->Remove( 10);
#endif

	MComboBox* pCombo = (MComboBox*)m_IDLResource.FindWidget( "Shop_AllEquipmentFilter");
	if ( pCombo)
	{
		pCombo->SetAlignment( MAM_LEFT);
		pCombo->SetListboxAlignment( MAM_LEFT);
	}
	pCombo = (MComboBox*)m_IDLResource.FindWidget( "Equip_AllEquipmentFilter");
	if ( pCombo)
	{
		pCombo->SetAlignment( MAM_LEFT);
		pCombo->SetListboxAlignment( MAM_LEFT);
	}


	pCombo = (MComboBox*)m_IDLResource.FindWidget( "StageType");
	if ( pCombo)
		pCombo->SetListboxAlignment( MAM_LEFT);


	// Ä³¸¯ÅÍ ºä¾î
	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget( "EquipmentInformation");
	if ( pCharView)
		pCharView->EnableAutoRotate( true);
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget( "EquipmentInformationShop");
	if ( pCharView)
		pCharView->EnableAutoRotate( true);

	// µà¾óÅä³Ê¸ÕÆ® ·Îºñ ÀüÀûUI¿¡ ¼³¸í ÅøÆÁ Ãß°¡
	char szTooltip[256];
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_WINLOSE_PREV);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoWinLosePrev", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_TP_PREV);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoPointsPrev", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_WINNER_PREV);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoWinnersPrev", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_RANK_PREV);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoRankingPrev", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_WINLOSE);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoWinLose", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_TP);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoPoints", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_WINNER);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoWinners", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_RANK);
	SetWidgetToolTipText("Lobby_DuelTournamentInfoRanking", szTooltip, MAM_RIGHT | MAM_TOP);
	ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_SIDERANKLIST);
	SetWidgetToolTipText("Lobby_DuelTournamentRankingList", szTooltip, MAM_RIGHT | MAM_TOP);

	SetCursorEnable(true);

	mlog( "game interface create success.\n" );

	return true;
}

void ZGameInterface::OnDestroy()
{
	mlog("Destroy interface begin \n");

	MTextArea* pTextArea = (MTextArea*)m_IDLResource.FindWidget("Shop_ItemDescription");
	if (pTextArea) pTextArea->ChangeCustomLook(NULL);

	//ZCharacterViewList* pCharViewList = (ZCharacterViewList*)m_IDLResource.FindWidget("StagePlayerList");
	//for(list<ZMeshView*>::iterator iter = ZMeshView::msMeshViewList.begin(); iter != ZMeshView::msMeshViewList.end(); ++iter )
	//{
	//	ZMeshView* pmv = *iter;
	//	if( pmv )
	//		pmv->OnInvalidate();
	//}

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	if(pCharView!= 0) pCharView->OnInvalidate();

	SetCursorEnable(false);

	SetState(GUNZ_NA);	// ÇöÀç GunzState Á¾·á

	SAFE_DELETE(m_pMapThumbnail);

	if (!m_sbRemainClientConnectionForResetApp)
	{
		m_spGameClient->Destroy();
		mlog("destroy client.\n");
		
		SAFE_DELETE(m_spGameClient);
		// mlog("SAFE_DELETE(m_spGameClient) : \n");
	}

	SAFE_DELETE(m_pLoginBG);			// ÇØÁ¦ ¾ÈµÇ´Â °æ¿ì°¡ ÀÖ±æ·¡ -_-;

	m_Tips.Finalize();
	FinalInterface();

	mlog("Destroy game interface done.\n");
}

void ZGameInterface::OnShutdownState()
{
	mlog("ZGameInterface::OnShutdown() : begin \n");
	
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MLabel* pLabel = (MLabel*)pResource->FindWidget("NetmarbleLoginMessage");
//			pLabel->SetText(MGetErrorString(MERR_CLIENT_DISCONNECTED));
			pLabel->SetText( ZErrStr(MERR_CLIENT_DISCONNECTED) );
			ZApplication::GetGameInterface()->ShowWidget("NetmarbleLogin", true);

	mlog("ZGameInterface::OnShutdown() : done() \n");
}

/*
void ZGameInterface::DrawLoadingScreen(const char* szMessage, float t)
{
	MDrawContext* pDC = Mint::GetInstance()->GetDrawContext();
	pDC->SetColor(0, 0, 0);
	pDC->FillRectangle(0, 0, MGetWorkspaceWidth(), MGetWorkspaceHeight());
	pDC->SetColor(255, 255, 255);
	pDC->Text(10, MGetWorkspaceHeight()-20, szMessage);
	t = min(max(0, t), 1);
	pDC->FillRectangle(0, 0, int(MGetWorkspaceWidth()*t), MGetWorkspaceHeight());

	// Update Scene
	Mint::GetInstance()->Update();
}
*/

bool ZGameInterface::SetState(GunzState nState)
{
#ifdef _BIRDTEST
	if ((nState != GUNZ_LOGIN) && (m_nState==GUNZ_BIRDTEST)) return false;
#endif

	// ÀÌÀü À§Ä¡¿Í ÇöÀç À§Ä¡°¡ °°À¸¸é ¹«½Ã
	if ( m_nState == nState)
		return true;

	if ( nState == GUNZ_PREVIOUS)
		nState = m_nPreviousState;


	// °ÔÀÓÀ» ½ÃÀÛ(GUNZ_GAME)ÇÏ±â Àü¿¡ Äù½ºÆ® ¸ðµå ½ÃÀÛ ¹«ºñ¸¦ º¸¿©ÁÙ ÇÊ¿ä°¡ ÀÖ´ÂÁö °Ë»ç
	if ( nState == GUNZ_GAME)
	{
		if ( ZApplication::GetStageInterface()->IsShowStartMovieOfQuest())
		{
			ZApplication::GetStageInterface()->ChangeStageEnableReady( true);
			MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageReady");
			if ( pWidget)
				pWidget->Enable( false);
			ZApplication::GetStageInterface()->StartMovieOfQuest();
			return true;
		}
	}

	m_nPreviousState = m_nState;

	if(m_nState==GUNZ_GAME) OnGameDestroy();
	else if(m_nState==GUNZ_LOGIN)
	{
		MWidget* pWidget = m_IDLResource.FindWidget( "Login_BackgrdImg");
		if ( !pWidget)
			OnLoginDestroy();
	}
	else if(m_nState==GUNZ_DIRECTLOGIN) OnDirectLoginDestroy();
	else if(m_nState==GUNZ_LOBBY) OnLobbyDestroy();
	else if(m_nState==GUNZ_STAGE) OnStageDestroy();
	else if(m_nState==GUNZ_GREETER) OnGreeterDestroy();
	else if(m_nState==GUNZ_CHARSELECTION)
	{
		OnCharSelectionDestroy();

		// °®°íÀÖ´Â Äù½ºÆ® ¾ÆÀÌÅÛ ¸ñ·Ï ¿äÃ»
		if ( nState == GUNZ_LOBBY)
		{
			ZGetMyInfo()->GetItemList()->QuestItemClear();
			ZPostRequestGetCharQuestItemInfo( ZGetGameClient()->GetPlayerUID());
		}
	}
	else if(m_nState==GUNZ_CHARCREATION) OnCharCreationDestroy();
#ifdef _BIRDTEST
	else if(m_nState==GUNZ_BIRDTEST) OnBirdTestDestroy();
#endif

	bool bStateChanged = true;
	if(nState==GUNZ_GAME) bStateChanged = OnGameCreate();
	else if(nState==GUNZ_LOGIN) OnLoginCreate();
	else if(nState==GUNZ_DIRECTLOGIN) OnDirectLoginCreate();
	else if(nState==GUNZ_LOBBY)	OnLobbyCreate();
	else if(nState==GUNZ_STAGE) OnStageCreate();
	else if(nState==GUNZ_GREETER) OnGreeterCreate();
	else if(nState==GUNZ_CHARSELECTION)
	{
		if ( m_nPreviousState == GUNZ_LOGIN)
		{
			MWidget* pWidget = m_IDLResource.FindWidget( "Login_BackgrdImg");
			if ( !pWidget)
				OnCharSelectionCreate();
			else
			{
				m_nLoginState = LOGINSTATE_LOGINCOMPLETE;
				m_dwLoginTimer = timeGetTime() + 1000;
				return true;
			}
		}
		else
			OnCharSelectionCreate();
	}
	else if(nState==GUNZ_CHARCREATION) OnCharCreationCreate();
	else if(nState==GUNZ_SHUTDOWN) OnShutdownState();
#ifdef _BIRDTEST
	else if(nState==GUNZ_BIRDTEST) OnBirdTestCreate();
#endif


	if(bStateChanged==false){
		m_pMsgBox->SetText("Error: Can't Create a Game!");
		m_pMsgBox->Show(true, true);
		SetState(GUNZ_PREVIOUS);
	}
	else{
		m_nState = nState;
	}

	m_nDrawCount = 0;
	return true;
}

_NAMESPACE_REALSPACE2_BEGIN
//extern int g_nCheckWallPolygons,g_nRealCheckWallPolygons;
//extern int g_nCheckFloorPolygons,g_nRealCheckFloorPolygons;
extern int g_nPoly,g_nCall;
extern int g_nPickCheckPolygon,g_nRealPickCheckPolygon;
_NAMESPACE_REALSPACE2_END

#ifndef _PUBLISH
#include "fmod.h"
#endif


/*
void ZGameInterface::OnUpdateGameMessage(void)
{
switch (ZGetGame()->GetMatch()->GetMatchState())
{
case ZMS_ROUND_READY:
{
int nRoundReadyCount = ZGetGame()->GetMatch()->GetRoundReadyCount();
if(nRoundReadyCount<-1){
ShowWidget(CENTERMESSAGE, false);
return;
}

char szReadyMessage[256] = "";
if(nRoundReadyCount>0){
sprintf(szReadyMessage, "Round %d : Start in %d", 
ZGetGame()->GetMatch()->GetNowRound()+1, nRoundReadyCount);
}
else{
strcpy(szReadyMessage, "Fight");
}

ShowWidget(CENTERMESSAGE, true);
SetTextWidget(CENTERMESSAGE, szReadyMessage);
}
break;
case ZMS_ROUND_PLAYING:
{
ShowWidget(CENTERMESSAGE, false);
}
break;
case ZMS_ROUND_FINISH:
{
char szReadyMessage[256] = "";
sprintf(szReadyMessage, "Finish!");

ShowWidget(CENTERMESSAGE, true);
SetTextWidget(CENTERMESSAGE, szReadyMessage);
}
break;
case ZMS_GAME_FINISH:
{
ShowWidget(CENTERMESSAGE, false);
}
break;
}

}
*/

void ZGameInterface::OnDrawStateGame(MDrawContext* pDC)
{
	if(m_pGame!=NULL) 
	{
		if(!IsMiniMapEnable())
			m_pGame->Draw();

		if (m_bViewUI) {

			if(m_bLeaveBattleReserved)
			{
				int nSeconds = (m_dwLeaveBattleTime - timeGetTime() + 999 ) / 1000;
				m_pCombatInterface->SetDrawLeaveBattle(m_bLeaveBattleReserved,nSeconds);
			}else
				m_pCombatInterface->SetDrawLeaveBattle(false,0);

			if(GetCamera()->GetLookMode()==ZCAMERA_MINIMAP) {
				//_ASSERT(m_pMiniMap);
				m_pMiniMap->OnDraw(pDC);
			}

			// ±×¸®´Â ¼ø¼­¶§¹®¿¡ ¸¸µç Æã¼Ç
			m_pCombatInterface->OnDrawCustom(pDC);
		}

	}
	m_ScreenDebugger.DrawDebugInfo(pDC);
}

void ZGameInterface::OnDrawStateLogin(MDrawContext* pDC)
{
	if( m_pBackground!=0)
	{
		m_pBackground->SetScene(LOGIN_SCENE_MAIN);
		m_pBackground->LoadMesh();
		m_pBackground->Draw();
	}

#ifndef _FASTDEBUG
	if (m_nDrawCount == 1)
	{
		/*static RealSoundEffectSource* pSES = ZApplication::GetSoundEngine()->GetSES("fx_amb_wind");
		
		if( pSES != NULL )
		{
		
			m_pAmbSound	= ZGetSoundEngine()->PlaySE( pSES, true );
		}
		//*/
	}
#endif

	MLabel* pConnectingLabel = (MLabel*)m_IDLResource.FindWidget( "Login_ConnectingMsg");
	if ( pConnectingLabel)
	{
		char szMsg[ 128];
		memset( szMsg, 0, sizeof( szMsg));
        int nCount = ( timeGetTime() / 800) % 4;
		for ( int i = 0;  i < nCount;  i++)
			szMsg[ i] = '<';
		sprintf( szMsg, "%s %s ", szMsg, "Connecting");
		for ( int i = 0;  i < nCount;  i++)
			strcat( szMsg, ">");

		pConnectingLabel->SetText( szMsg);
		pConnectingLabel->SetAlignment( MAM_HCENTER | MAM_VCENTER);
	}


	MWidget* pWidget = m_IDLResource.FindWidget( "LoginFrame");
	MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget( "Login_BackgrdImg");
	if ( !pWidget || !pPicture)
		return;


	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget( "SelectedServer");
	MEdit* pPassword = (MEdit*)m_IDLResource.FindWidget( "LoginPassword");
	MWidget* pLogin = m_IDLResource.FindWidget( "LoginOK");
	if ( pServerList && pPassword && pLogin)
	{
		if ( pServerList->IsSelected() && (int)strlen( pPassword->GetText()))
			pLogin->Enable( true);
		else
			pLogin->Enable( false);
	}


	DWORD dwCurrTime = timeGetTime();

	// Check timeout
	if ( m_bLoginTimeout && (m_dwLoginTimeout <= dwCurrTime))
	{
		m_bLoginTimeout = false;

		MLabel* pLabel = (MLabel*)m_IDLResource.FindWidget( "LoginError");
		if (pLabel)
			pLabel->SetText( ZErrStr( MERR_CLIENT_CONNECT_FAILED));

		MButton* pLoginOK = (MButton*)m_IDLResource.FindWidget( "LoginOK");
		if (pLoginOK)
			pLoginOK->Enable(true);

		pWidget->Show(true);

		if ( pConnectingLabel)
			pConnectingLabel->Show( false);
	}

	// Fade in
	if ( m_nLoginState == LOGINSTATE_FADEIN)
	{
		m_bLoginTimeout = false;

		if ( dwCurrTime >= m_dwLoginTimer)
		{
			int nOpacity = pPicture->GetOpacity() + 3;
			if ( nOpacity > 255)
				nOpacity = 255;

			pPicture->SetOpacity( nOpacity);

			m_dwLoginTimer = dwCurrTime + 9;
		}

		if ( pPicture->GetOpacity() == 255)
		{
			m_dwLoginTimer = dwCurrTime + 1000;
			m_nLoginState = LOGINSTATE_SHOWLOGINFRAME;
		}
		else
			pWidget->Show( false);
	}
	// Show login frame
	else if ( m_nLoginState == LOGINSTATE_SHOWLOGINFRAME)
	{
		m_bLoginTimeout = false;

		if ( timeGetTime() > m_dwLoginTimer)
		{
			m_nLoginState = LOGINSTATE_STANDBY;
			pWidget->Show( true);
		}
	}
	// Standby
	else if ( m_nLoginState == LOGINSTATE_STANDBY)
	{
#ifdef _LOCATOR
		// Refresh server status info
		if ( timeGetTime() > m_dwRefreshTime)
			RequestServerStatusListInfo();
#endif
	}
	// Login Complete
	else if ( m_nLoginState == LOGINSTATE_LOGINCOMPLETE)
	{
		m_bLoginTimeout = false;

		if ( timeGetTime() > m_dwLoginTimer)
			m_nLoginState = LOGINSTATE_FADEOUT;

		if ( pConnectingLabel)
			pConnectingLabel->Show( false);
	}
	// Fade out
	else if ( m_nLoginState == LOGINSTATE_FADEOUT)
	{
		m_bLoginTimeout = false;
		pWidget->Show(false);

		if ( dwCurrTime >= m_dwLoginTimer)
		{
			int nOpacity = pPicture->GetOpacity() - 3;
			if ( nOpacity < 0)
				nOpacity = 0;

			pPicture->SetOpacity( nOpacity);

			m_dwLoginTimer = dwCurrTime + 9;
		}

		if ( pPicture->GetOpacity() == 0)
		{
			OnLoginDestroy();
			
			m_nLoginState = LOGINSTATE_STANDBY;
			m_nState = GUNZ_CHARSELECTION;
			OnCharSelectionCreate();
		}
	}

	if(IsErrMaxPlayer())
	{
		if(m_dErrMaxPalyerDelayTime <= dwCurrTime)
		{
			ZPostDisconnect();
			SetErrMaxPlayer(false);
			ZApplication::GetGameInterface()->ShowErrorMessage( 10003 );
		}
	}
	
//#ifdef LOCALE_NHNUSA
//#if defined(LOCALE_NHNUSA) || defined(_DEBUG)
#define CHECK_PING_TIME	(1 * 3 * 1000)	// 3 sec
	static unsigned long tmAgentPingTestCheckTimer = dwCurrTime;
	if (dwCurrTime - tmAgentPingTestCheckTimer > CHECK_PING_TIME)
	{
		tmAgentPingTestCheckTimer = dwCurrTime;

		if ( pServerList)
		{
			SERVERLIST cServerList = pServerList->GetServerList();
		
			for ( SERVERLIST::iterator itr = cServerList.begin(); itr != cServerList.end();  itr++)
			{
				ServerInfo *serverInfo = (*itr);

				MCommand* pCmd = GetGameClient()->CreateCommand(MC_UDP_PING, MUID(0,0));
				unsigned int nIP = (unsigned int)inet_addr(m_spGameClient->GetUDPInfo()->GetAddress());
				pCmd->AddParameter(new MCmdParamUInt(dwCurrTime));
				GetGameClient()->SendCommandByUDP(pCmd, serverInfo->szAgentIP, 7778);
				delete pCmd;
			}
		}
	}
//#endif
}

void ZGameInterface::OnDrawStateLobbyNStage(MDrawContext* pDC)
{
	ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();

	DWORD dwClock = timeGetTime();
	if ( (dwClock - m_dwFrameMoveClock) < 30)
		return;
	m_dwFrameMoveClock = dwClock;


	if( GetState() == GUNZ_LOBBY )
	{
		// TODO : ÀÌµéÀ» draw Å¸ÀÓ¿¡ °è¼ÓÇØ¼­ ÇÒ ÇÊ¿ä´Â ¾ø¾îº¸ÀÎ´Ù. Àû´çÇÑ °÷À¸·Î ¿Å±âÀÚ

		// Lobby
		char buf[512];
		// ÀÌ¸§
		MLabel* pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerName");
		if (pLabel)
		{
			sprintf( buf, "%s", ZGetMyInfo()->GetCharName() );
			pLabel->SetText(buf);
#ifdef _GZFGunz
			pLabel->SetTextColor(MCOLOR(ZGetMyInfo()->GetRed(), ZGetMyInfo()->GetGreen(), ZGetMyInfo()->GetBlue()));
#endif
		}
		// »ó¼¼Á¤º¸ (µ¿È¯ÀÌ°¡ Ãß°¡)
		// Clan
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecClan");
		sprintf( buf, "%s : %s", ZMsg( MSG_CHARINFO_CLAN), ZGetMyInfo()->GetClanName());
		if (pLabel) pLabel->SetText(buf);
		// LV
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecLevel");
		sprintf( buf, "%s : %d %s", ZMsg( MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
		if (pLabel) pLabel->SetText(buf);
		// XP
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecXP");
		sprintf( buf, "%s : %d%%", ZMsg( MSG_CHARINFO_XP), ZGetMyInfo()->GetLevelPercent());
		if (pLabel) pLabel->SetText(buf);
		// BP
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecBP");
		sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
		if (pLabel) pLabel->SetText(buf);
		// BPPlus
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecBPPlus");
		sprintf(buf, "Cash : %d", ZGetMyInfo()->GetBPPlus());
		if (pLabel) pLabel->SetText(buf);
		// EventCoins
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecEventCoins");
		sprintf(buf, "Event Coins : %d", ZGetMyInfo()->GetEventCoins());
		if (pLabel) pLabel->SetText(buf);
		// HP
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecHP");
		sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
		if (pLabel) pLabel->SetText(buf);
		// AP
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecAP");
		sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
		if (pLabel) pLabel->SetText(buf);
		// WT
		pLabel = (MLabel*)pRes->FindWidget("Lobby_PlayerSpecWT");
		ZMyItemList* pItems= ZGetMyInfo()->GetItemList();
		sprintf( buf, "%s : %d/%d", ZMsg( MSG_CHARINFO_WEIGHT), pItems->GetEquipedTotalWeight(), pItems->GetMaxWeight());
		if (pLabel) pLabel->SetText(buf);

		// Ã¤³Î Á¤º¸
		pLabel = (MLabel*)pRes->FindWidget("Lobby_ChannelName");
		sprintf( buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg( MSG_WORD_LOBBY), ZGetGameClient()->GetChannelName());
		if (pLabel)
		{
			pLabel->SetAlignment(MAM_HCENTER);
			pLabel->SetText(buf);
		}
	}

	// Stage
	else if( GetState() == GUNZ_STAGE)
	{
		// ÇÃ·¹ÀÌ¾î Á¤º¸ Ç¥½Ã
		char buf[512];
		MListBox* pListBox = (MListBox*)pRes->FindWidget( "StagePlayerList_");
		bool bShowMe = true;
		if ( pListBox)
		{
			ZStagePlayerListItem* pItem = NULL;
			if ( pListBox->GetSelIndex() < pListBox->GetCount())
			{
				if ( pListBox->GetSelIndex() >= 0)
					pItem = (ZStagePlayerListItem*)pListBox->Get( pListBox->GetSelIndex());

				if ( (pListBox->GetSelIndex() != -1) && (strcmp(ZGetMyInfo()->GetCharName(), pItem->GetString( 3)) != 0))
					bShowMe = false;
			}

			ZPlayerInfo* pInfo = NULL;
			if ( bShowMe)
				pInfo = ZGetPlayerManager()->Find( ZGetMyUID());
			else if ( pItem != NULL)
				pInfo = ZGetPlayerManager()->Find( pItem->GetUID());


			MLabel* pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerName");
			if (pLabel)
			{
				if (bShowMe)
				{
					sprintf(buf, "%s", ZGetMyInfo()->GetCharName());
					pLabel->SetTextColor(MCOLOR(255, 255, 255));
					pLabel->SetText(buf);
				}
				else
				{
					sprintf(buf, "%s", pItem->GetString(3));
					pLabel->SetTextColor(MCOLOR(255, 255, 255));
					pLabel->SetText(buf);
				}
			}

			// Å¬·£
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecClan");
			if ( pLabel)
			{
				if ( bShowMe)
					sprintf( buf, "%s : %s", ZMsg( MSG_CHARINFO_CLAN), ZGetMyInfo()->GetClanName());
				else
				{
					if ( strcmp( pItem->GetString( 5), "") == 0)
						sprintf( buf, "%s :", ZMsg( MSG_CHARINFO_CLAN));
					else
						sprintf( buf, "%s : %s", ZMsg( MSG_CHARINFO_CLAN), pItem->GetString( 5));
				}
				pLabel->SetText(buf);
			}

			// ·¹º§
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecLevel");
			if ( pLabel)
			{
				if ( bShowMe)
					sprintf( buf, "%s : %d %s", ZMsg( MSG_CHARINFO_LEVEL), ZGetMyInfo()->GetLevel(), ZMsg(MSG_CHARINFO_LEVELMARKER));
				else
					sprintf( buf, "%s : %s %s", ZMsg( MSG_CHARINFO_LEVEL), pItem->GetString( 1), ZMsg(MSG_CHARINFO_LEVELMARKER));
				pLabel->SetText(buf);
			}

			// XP
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecXP");
			if ( pLabel)
			{
				if ( bShowMe)
					sprintf( buf, "%s : %d%%", ZMsg( MSG_CHARINFO_XP), ZGetMyInfo()->GetLevelPercent());
				else
					sprintf( buf, "%s : -", ZMsg( MSG_CHARINFO_XP));
				pLabel->SetText(buf);
				pLabel->Enable( bShowMe);
			}

			// BP
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecBP");
			if ( pLabel)
			{
				if ( bShowMe)
					sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_BOUNTY), ZGetMyInfo()->GetBP());
				else
					sprintf( buf, "%s : -", ZMsg( MSG_CHARINFO_BOUNTY));
				pLabel->SetText(buf);
				pLabel->Enable( bShowMe);
			}

			// HP
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecHP");
			if ( pLabel)
			{
				if ( bShowMe)
					sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_HP), ZGetMyInfo()->GetHP());
				else
					sprintf( buf, "%s : -", ZMsg( MSG_CHARINFO_HP));
				pLabel->SetText(buf);
				pLabel->Enable( bShowMe);
			}

			// AP
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecAP");
			if ( pLabel)
			{
				if ( bShowMe)
					sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_AP), ZGetMyInfo()->GetAP());
				else
					sprintf( buf, "%s : -", ZMsg( MSG_CHARINFO_AP));
				pLabel->SetText(buf);
				pLabel->Enable( bShowMe);
			}

			// WT
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecWT");
			if ( pLabel)
			{
				ZMyItemList* pItems= ZGetMyInfo()->GetItemList();
				if ( bShowMe)
					sprintf( buf, "%s : %d/%d", ZMsg( MSG_CHARINFO_WEIGHT), pItems->GetEquipedTotalWeight(), pItems->GetMaxWeight());
				else
					sprintf( buf, "%s : -", ZMsg( MSG_CHARINFO_WEIGHT));
				pLabel->SetText(buf);
				pLabel->Enable( bShowMe);
			}


			

			// Ranking
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecRanking");
			if ( pLabel && pInfo)
			{
				if ( pInfo->GetRank() == 0)
					sprintf( buf, "%s : -", ZMsg( MSG_CHARINFO_RANKING));
				else
					sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_RANKING), pInfo->GetRank());
				pLabel->SetText(buf);
			}

			// Kill
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecKill");
			if ( pLabel && pInfo)
			{
				sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_KILL), pInfo->GetKill());
				pLabel->SetText(buf);
			}

			// Death
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecDeath");
			if ( pLabel && pInfo)
			{
				sprintf( buf, "%s : %d", ZMsg( MSG_CHARINFO_DEATH), pInfo->GetDeath());
				pLabel->SetText(buf);
			}

			// Winning percent
			pLabel = (MLabel*)pRes->FindWidget("Stage_PlayerSpecWinning");
			if ( pLabel && pInfo)
			{
				sprintf( buf, "%s : %.1f%%", ZMsg( MSG_CHARINFO_WINNING), pInfo->GetWinningRatio());
				pLabel->SetText(buf);
			}


			// Character View
			if ( bShowMe)
			{
				ZCharacterView* pCharView = (ZCharacterView*)pRes->FindWidget( "Stage_Charviewer");
				if ( pCharView)
					pCharView->SetCharacter( ZGetMyUID());
			}
		}


		// Äù½ºÆ® ¸ðµå·Î ½ÃÀÛÇÒ¶§ ¹«ºñ¸¦ º¸¿©ÁÜ
		ZApplication::GetStageInterface()->OnDrawStartMovieOfQuest();


		// ±¤¼± ÀÌ¹ÌÁö Opacity Á¶Àý
		int nOpacity = 90.0f * ( sin( timeGetTime() * 0.003f) + 1) + 75;

		MLabel* pLabel = (MLabel*)pRes->FindWidget( "Stage_SenarioName");
		MPicture* pPicture = (MPicture*)pRes->FindWidget( "Stage_Lights0");
		if ( pPicture)
		{
			pPicture->SetOpacity( nOpacity);
		}
		pPicture = (MPicture*)pRes->FindWidget( "Stage_Lights1");
		if ( pPicture)
		{
			pPicture->SetOpacity( nOpacity);
		}



		// Èñ»ý ¾ÆÀÌÅÛ ¸®½ºÆ® ÇÁ·¹ÀÓ ¿òÁ÷ÀÓ
		MWidget* pWidget = pRes->FindWidget( "Stage_ItemListView");
		if ( !pWidget)
			return;

		int nEndPos = ZApplication::GetStageInterface()->m_nListFramePos;
		MRECT rect = pWidget->GetRect();
		if ( rect.x != nEndPos)
		{
			int nNewPos = rect.x + ( nEndPos - rect.x) * 0.25;
			if ( nNewPos == rect.x)		// not changed
				rect.x = nEndPos;
			else						// changed
				rect.x = nNewPos;

			pWidget->SetBounds( rect);

			if ( rect.x == 0)
			{
				pWidget = pRes->FindWidget( "Stage_CharacterInfo");
				if ( pWidget)
					pWidget->Enable( false);
			}
		}

		// ¸±·¹ÀÌ¸Ê ¸®½ºÆ® ÇÁ·¹ÀÓ ¿òÁ÷ÀÓ
		pWidget = pRes->FindWidget( "Stage_RelayMapListView");
		if ( !pWidget)
			return;

		nEndPos = ZApplication::GetStageInterface()->m_nRelayMapListFramePos;
		rect = pWidget->GetRect();
		if ( rect.x != nEndPos)
		{
			int nNewPos = rect.x + ( nEndPos - rect.x) * 0.25;
			if ( nNewPos == rect.x)		// not changed
				rect.x = nEndPos;
			else						// changed
				rect.x = nNewPos;

			pWidget->SetBounds( rect);

			if ( rect.x == 0)
			{
				pWidget = pRes->FindWidget( "Stage_CharacterInfo");
				if ( pWidget)
					pWidget->Enable( false);
			}
		}
	}
}

void ZGameInterface::OnDrawStateCharSelection(MDrawContext* pDC)
{
	if ( m_pBackground && m_pCharacterSelectView)
	{
		m_pBackground->LoadMesh();
		m_pBackground->Draw();
		m_pCharacterSelectView->Draw();

		// Draw effects(smoke, cloud)
		ZGetEffectManager()->Draw( timeGetTime());

		// Draw maiet logo effect
//		ZGetScreenEffectManager()->DrawEffects();
	}
}

void ZGameInterface::OnDraw(MDrawContext *pDC)
{
	m_nDrawCount++;

	__BP(11,"ZGameInterface::OnDraw");

	if(m_bLoading) 
	{
		__EP(11);
		return;
	}

#ifdef _BIRDTEST
	if (GetState() == GUNZ_BIRDTEST)
	{
		OnBirdTestDraw();
		__EP(11);
		return;
	}
#endif		

	switch (GetState())
	{
	case GUNZ_GAME:
		{
			OnDrawStateGame(pDC);
		}
		break;
	case GUNZ_LOGIN:
	case GUNZ_DIRECTLOGIN:
		{
			OnDrawStateLogin(pDC);
		}
		break;
	case GUNZ_LOBBY:
	case GUNZ_STAGE:
		{
			OnDrawStateLobbyNStage(pDC);
		}
		break;
	case GUNZ_CHARSELECTION:
	case GUNZ_CHARCREATION:
		{
			OnDrawStateCharSelection(pDC);
		}
		break;
	}

	__EP(11);
}



// ÀÓ½Ã

// ³²ÀÚ´Â 2¹øÂ° ÄÚÆ® ºÎÅÍ..

void ZGameInterface::TestChangePartsAll() 
{
}

void ZGameInterface::TestChangeParts(int mode) {

#ifndef _PUBLISH
	// µðÀÚÀÌ³Ê¿ë... È¥ÀÚ ¿Ê°¥¾Æ ÀÔ´Â Å×½ºÆ® ÇÒ¶§ »ç¿ëÇÑ´Ù...

	RMeshPartsType ptype = eq_parts_etc;

 		 if(mode==0)	{ ptype = eq_parts_chest;  }
	else if(mode==1)	{ ptype = eq_parts_head	;  }
	else if(mode==2)	{ ptype = eq_parts_hands;  }
	else if(mode==3)	{ ptype = eq_parts_legs	;  }
	else if(mode==4)	{ ptype = eq_parts_feet	;  }
	else if(mode==5)	{ ptype = eq_parts_face	;  }

	ZPostChangeParts(ptype,1);

#endif

}
// Ä³¸¯ÅÍ ¼±ÅÃ À©µµ¿ì°¡ »ý±â¸é »èÁ¦..
void ZGameInterface::TestToggleCharacter()
{
	ZPostChangeCharacter();
}

void ZGameInterface::TestChangeWeapon(RVisualMesh* pVMesh)
{
	static int nWeaponIndex = 0;

	int nItemID = 0;
	switch(nWeaponIndex)
	{
	case 0:
		nItemID = 1;		// katana
		break;
	case 1:
		nItemID = 5;		// dagger
		break;
	case 2:
		nItemID = 2;		// double pistol
		break;
	case 3:
		nItemID = 3;		// SMG
		break;
	case 4:
		nItemID = 6;		// shotgun
		break;
	case 5:
		nItemID = 7;		// Rocket
		break;
	case 6:
		nItemID = 4;		// grenade
		break;
	}


	if (GetState() == GUNZ_GAME)
	{
		if (m_pMyCharacter == NULL) return;


		switch(nWeaponIndex)
		{
		case 0:
		case 1:
			m_pMyCharacter->GetItems()->EquipItem(MMCIP_MELEE, nItemID);		// dagger
			m_pMyCharacter->ChangeWeapon(MMCIP_MELEE);
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			m_pMyCharacter->GetItems()->EquipItem(MMCIP_PRIMARY, nItemID);		// Rocket
			m_pMyCharacter->ChangeWeapon(MMCIP_PRIMARY);
			break;
		case 6:
			m_pMyCharacter->GetItems()->EquipItem(MMCIP_CUSTOM1, nItemID);		// grenade
			m_pMyCharacter->ChangeWeapon(MMCIP_CUSTOM1);
			break;
		}

	}
	else if (GetState() == GUNZ_CHARSELECTION)
	{
		if (pVMesh != NULL)
		{
			ZChangeCharWeaponMesh(pVMesh, nItemID);
			pVMesh->SetAnimation("login_intro");
			pVMesh->GetFrameInfo(ani_mode_lower)->m_nFrame = 0;
//			pVMesh->m_nFrame[ani_mode_lower] = 0;
		}
	}
	else if (GetState() == GUNZ_LOBBY)
	{
		if (pVMesh != NULL)
		{
			ZChangeCharWeaponMesh(pVMesh, nItemID);
		}
	}


	nWeaponIndex++;
	if (nWeaponIndex >= 7) nWeaponIndex = 0;
}

/*
bool ZGameInterface::ProcessLowLevelCommand(const char* szCommand)
{
if(stricmp(szCommand, "FORWARD")==0){
}
else if(stricmp(szCommand, "BACK")==0){
}
else if(stricmp(szCommand, "LEFT")==0){
}
else if(stricmp(szCommand, "RIGHT")==0){
}
return false;
}
*/

void ZGameInterface::RespawnMyCharacter()	// È¥ÀÚÅ×½ºÆ®ÇÒ¶§ Å¬¸¯ÇÏ¸é µÇ»ì¾Æ³­´Ù.
{
	if (ZGetGame() == NULL) return;
	m_pMyCharacter->Revival();
	rvector pos=rvector(0,0,0), dir=rvector(0,1,0);

	ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
	if (pSpawnData != NULL)
	{
		pos = pSpawnData->m_Pos;
		dir = pSpawnData->m_Dir;
	}

	m_pMyCharacter->SetPosition(pos);
	m_pMyCharacter->SetDirection(dir);
}

bool ZGameInterface::OnGlobalEvent(MEvent* pEvent)
{
	if (ZGameInterface::CheckSkipGlobalEvent() == true)
		return true;

	if ((ZGetGameInterface()->GetState() == GUNZ_GAME)) 
		return ZGameInput::OnEvent(pEvent);

#ifndef _PUBLISH
	switch(pEvent->nMessage){
		case MWM_CHAR:
		{
			switch (pEvent->nKey) {
			case '`' :
				if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_DEVELOP)
				{
					ZGetConsole()->Show(!ZGetConsole()->IsVisible());
					ZGetConsole()->SetZOrder(MZ_TOP);
					return true;
					//				m_pLogFrame->Show(ZGetConsole()->IsVisible());
				}
				break;
			}
		}break;

		case MWM_KEYDOWN:
			{
				// Ctrl+R UI ¸®¼Ò½º ¸®·Îµù - ¹®Á¦°¡ ¸¹´Ù. ±Ùµ¥ ±×³É UI µðÀÚÀÎ¿ë ±ÞÁ¶ ±â´ÉÀÌ´Ï±î ±×³É ¾¹½Ã´ç..
				// (800*600ÀÌ¿Ü¿¡¼­´Â È­¸éÀÌ ÀÌ»óÇØÁü, Á¾·á½Ã ¸Þ¸ð¸®¸¯ ¾öÃ» ³², °ÔÀÓÁß¿£ ¸®·Îµå ÀÛµ¿¾ÈµÊ)
				if (pEvent->GetCtrlState() && pEvent->nKey == 'R') {
					GunzState state = ZGetGameInterface()->GetState();
					m_sbRemainClientConnectionForResetApp = true;
					ZLoadingProgress progress("");
					ZGetGameInterface()->OnDestroy();
					ZGetGameInterface()->OnCreate(&progress);
					//OnRestore();
					ZGetGameInterface()->SetState(state);
					m_sbRemainClientConnectionForResetApp = false;
				}
			}
	}
#endif
	return false;
}

bool ZGameInterface::OnDebugEvent(MEvent* pEvent, MListener* pListener)
{
	switch(pEvent->nMessage){
	case MWM_KEYDOWN:
		{
			switch (pEvent->nKey)
			{
			case VK_F10:
				m_pLogFrame->Show(!m_pLogFrame->IsVisible());
				return true;
			case VK_NUMPAD8:
				{
					if (GetState() == GUNZ_CHARSELECTION)
					{
						if (m_pCharacterSelectView != NULL)
						{
//							TestChangeWeapon(m_pCharacterSelectView->GetVisualMesh());
						}
					}
					else if (GetState() == GUNZ_LOBBY)
					{
						if (ZGetCharacterViewList(GUNZ_LOBBY) != NULL)
						{
							RVisualMesh* pVMesh = 
								ZGetCharacterViewList(GUNZ_LOBBY)->Get(ZGetGameClient()->GetPlayerUID())->GetVisualMesh();

							TestChangeWeapon(pVMesh);
						}
					}

				}
				break;
			}
		}
		break;
	}
	return false;
}

bool ZGameInterface::OnEvent(MEvent* pEvent, MListener* pListener)
{
#ifndef _PUBLISH
	if (OnDebugEvent(pEvent, pListener)) return true;
#endif

	return false;
}

bool ZGameInterface::OnCommand(MWidget* pWidget, const char* szMessage)
{
	if (pWidget==m_pPlayerMenu) {
		MMenuItem* pItem = (MMenuItem*)pWidget;


		OutputDebugString("PLAYERMENU");
	}
	return false;
}

void ZGameInterface::ChangeParts(int key)
{
	//1¹ø~6¹øÅ°µî·Ï
	/*
	int mode = 0;
	int type = 0;		// ³ªÁß¿¡ ¿©·¯ ¹úÀÌ¶ó¸é ÀåºñÇÒ id ÁöÁ¤...

	if(key=='1')	{ mode=0;type = 2;}
	else if(key=='2')	{ mode=1;type = 7;}
	else if(key=='3')	{ mode=2;type =12;}
	else if(key=='4')	{ mode=3;type =17;}
	else if(key=='5')	{ mode=4;type =22;}
	else if(key=='6')	{ mode=5;type =27;}
	else return;

	g_parts[mode] = !g_parts[mode];	// Ä³¸¯ÅÍ Å¬·¡½º·Î ¿Å±â±â
	//	g_parts_change = true;

	if(!g_parts[mode])	// ±âº»¿Ê
	type = 0;	

	g_pGame->m_pMyCharacter->OnChangeParts(mode,type);

	ZPostChangeParts(mode,type);
	*/
}
/*
void ZGameInterface::UpdateReserveChangeWeapon() {

	if(!m_pMyCharacter) return;

	if(m_bisReserveChangeWeapon) {
		if( m_pMyCharacter->m_fNextShotTime < g_pGame->GetTime()) {
			ChangeWeapon(m_nReserveChangeWeapon);
			m_bisReserveChangeWeapon = false;
		}
	}
}
*/

void ZGameInterface::ChangeWeapon(ZChangeWeaponType nType)
{
	ZMyCharacter* pChar = ZGetGame()->m_pMyCharacter;

	if (pChar->m_pVMesh == NULL) return;	
	if (pChar->IsDie() ) return;
	if (m_pGame->GetMatch()->IsRuleGladiator()) return;


	int nParts = -1;

	bool bWheel = false;
	if ((nType == ZCWT_PREV) || (nType == ZCWT_NEXT))
	{
		bWheel = true;

		int nHasItemCount = 0;
		int nPos = -1;
		int ItemQueue[MMCIP_END];

		for (int i = MMCIP_MELEE; i < MMCIP_CUSTOM2 + 1; i++)
		{
			//Steven: Remove Customs From Mouse Scroll
			if (!pChar->GetItems()->GetItem((MMatchCharItemParts)i)->IsEmpty()) {
				if (ZGetConfiguration()->GetExtra()->bSpecialItemsScroll == true
					|| pChar->GetItems()->GetSelectedWeaponParts() == MMCIP_CUSTOM1 && ZCWT_NEXT
					|| pChar->GetItems()->GetSelectedWeaponParts() == MMCIP_CUSTOM2 && ZCWT_PREV
					|| i < MMCIP_SECONDARY + 1) {
					if (pChar->GetItems()->GetSelectedWeaponParts() == i)
						nPos = nHasItemCount;

					ItemQueue[nHasItemCount++] = i;
				}
			}
		}


		if (nPos < 0) return;

		if (nType == ZCWT_PREV)
		{
			if (nHasItemCount <= 1) return;

			int nNewPos = nPos - 1;
			int nCount = 0;
			if (nNewPos < 0) nNewPos = nHasItemCount-1;
			while ( nNewPos != nPos)
			{
				MMatchCharItemParts nPart = (MMatchCharItemParts)ItemQueue[ nNewPos];
				if ( (nPart == MMCIP_CUSTOM1) || (nPart == MMCIP_CUSTOM2) ) {
					ZItem* pItem = ZGetGame()->m_pMyCharacter->GetItems()->GetItem(nPart);
					if (pItem && pItem->GetBulletCurrMagazine() > 0) { break; }
				} else { break; }

				nNewPos = nNewPos - 1;
				if (nNewPos < 0) nNewPos = nHasItemCount-1;

				nCount++;
				if ( nCount > (nHasItemCount * 2))
					return;
			}
			nPos = nNewPos;

		}
		else if (nType == ZCWT_NEXT)
		{
			if (nHasItemCount <= 1) return;

			int nNewPos = nPos + 1;
			if (nNewPos >= nHasItemCount) nNewPos = 0;
			while(nNewPos != nPos) 
			{
				MMatchCharItemParts nPart = (MMatchCharItemParts)ItemQueue[nNewPos];
				if ( (nPart == MMCIP_CUSTOM1) || (nPart == MMCIP_CUSTOM2) ) {
					ZItem* pItem = ZGetGame()->m_pMyCharacter->GetItems()->GetItem(nPart);
					if (pItem && pItem->GetBulletCurrMagazine() > 0) {	break; }
				} else { break; }

				nNewPos = nNewPos + 1;
				if (nNewPos >= nHasItemCount) nNewPos = 0;
			}
			nPos = nNewPos;
		}

		nParts = ItemQueue[nPos];
	}
	else
	{
		switch (nType)
		{
		case ZCWT_MELEE:	nParts = (int)(MMCIP_MELEE);	break;
		case ZCWT_PRIMARY:	nParts = (int)(MMCIP_PRIMARY);	break;
		case ZCWT_SECONDARY:nParts = (int)(MMCIP_SECONDARY);break;
		case ZCWT_CUSTOM1:	nParts = (int)(MMCIP_CUSTOM1);	break;
		case ZCWT_CUSTOM2:	nParts = (int)(MMCIP_CUSTOM2);	break;
		default: break;
		}

		if ( (nParts == MMCIP_CUSTOM1) || (nParts == MMCIP_CUSTOM2) ) 
		{
			ZItem* pItem = pChar->GetItems()->GetItem((MMatchCharItemParts)nParts);
			if (pItem->GetBulletCurrMagazine() <= 0) return;
		}
	}

	if (nParts < 0) return;
	if (pChar->GetItems()->GetSelectedWeaponParts() == nParts) return;

	if(bWheel && (pChar->GetStateUpper() == ZC_STATE_UPPER_LOAD && pChar->IsUpperPlayDone() == false ))
		return;
	
	const ZCharaterStatusBitPacking &uStatus = pChar->m_dwStatusBitPackingValue.Ref();
	ZMyCharaterStatusBitPacking & zStatus = pChar->m_statusFlags.Ref();

	if (zStatus.m_bWallHang || zStatus.m_bShot || zStatus.m_bShotReturn || uStatus.m_bTumble 
		|| zStatus.m_bSkill || zStatus.m_bGuard || uStatus.m_bBlast || 	uStatus.m_bBlastFall 
		|| uStatus.m_bBlastDrop || 	uStatus.m_bBlastStand || uStatus.m_bBlastAirmove
		|| zStatus.m_bSlash || zStatus.m_bJumpSlash || zStatus.m_bJumpSlashLanding
		 
		|| (pChar->GetStateUpper() == ZC_STATE_UPPER_SHOT && pChar->IsUpperPlayDone() == false )
		
		)
		{
			m_bReservedWeapon = true;
			m_ReservedWeapon  = nType;
			return;
		}

	m_bReservedWeapon = false;
	ZPostChangeWeapon(nParts);

	m_pMyCharacter->m_dwStatusBitPackingValue.Ref().m_bSpMotion = false;
	m_pMyCharacter->ChangeWeapon((MMatchCharItemParts)nParts);
}


void ZGameInterface::OnGameUpdate(float fElapsed)
{
	__BP(12,"ZGameInterface::OnGameUpdate");
	if(m_pGame==NULL) return;
	if (m_pGameInput) m_pGameInput->Update(fElapsed);


	m_pGame->Update(fElapsed);

	if (m_pCombatInterface) m_pCombatInterface->Update(fElapsed);

	if(m_bReservedWeapon)
		ChangeWeapon(m_ReservedWeapon);

	__EP(12);
}


extern bool g_bTestFromReplay;

void ZGameInterface::OnReplay()
{
	ShowWidget( "ReplayConfirm", false);

	CreateReplayGame(NULL);
}

bool ZGameInterface::Update(float fElapsed)
{
	// ¿¹¾àµÈ Á¾·á¸¦ È®ÀÎÇÑ´Ù.
	if ( m_bReservedQuit) {
		if ( timeGetTime() > m_dwReservedQuitTimer) {
			ZGetApplication()->Exit();
		}
	}

	if ( m_pBackground && ( ( GetState() == GUNZ_CHARSELECTION) || ( GetState() == GUNZ_CHARCREATION)) )
		m_pBackground->OnUpdate( fElapsed);


	// º¸ÀÌ½º »ç¿îµå ¾÷µ¥ÀÌÆ®
	OnVoiceSound();


	// Å×½ºÆ®·Î ¸®ÇÃ·¹ÀÌ´Â ´Ù½Ã Ã³À½ºÎÅÍ ½ÃÀÛÇÏ±â À§ÇØ ÀÌ·¸°Ô ¸¸µé¾ú´Ù.
	if (GetState() == GUNZ_LOBBY)
	{
		if (g_bTestFromReplay == true) 
		{
			ShowWidget( "Lobby", false);
			ShowWidget( "ReplayConfirm", true);
			return false;
		}
	}

	__BP(13,"ZGameInterface::Update");

	ZGetOptionInterface()->Update();

	GetShopEquipInterface()->Update();

	__BP(14,"ZGameInterface::GameClient Run & update");
	if (m_spGameClient != NULL) m_spGameClient->Run();
	m_spGameClient->Tick();
	__EP(14);

 	if(!m_bLoading) {
		if(GetState()==GUNZ_GAME){
			OnGameUpdate(fElapsed);
		}
		else{
#ifdef _BIRDTEST 
			if (GetState()==GUNZ_BIRDTEST) OnBirdTestUpdate();
#endif
		}
	}

	if(GetState()==GUNZ_LOBBY && m_bWaitingArrangedGame) {
        // ÀÓ½Ã·Î
		MLabel *pLabel = (MLabel*)m_IDLResource.FindWidget("LobbyWaitingArrangedGameLabel");
		if(pLabel) {
			int nCount = (timeGetTime()/500)%5;
			char dots[10];
			for(int i=0;i<nCount;i++) {
				dots[i]='.';
			}
			dots[nCount]=0;

			char szBuffer[256];
			sprintf(szBuffer,"%s%s", ZMsg( MSG_WORD_FINDTEAM), dots);
			pLabel->SetText(szBuffer);
		}
	}

	// ·Îºñ°¡ ¾Æ´Ñµ¥ Åä³Ê¸ÕÆ® Âü°¡Ã¢ÀÌ ¶°ÀÖÀ¸¸é Á¢¼Ó ²÷°Ü¼­ ·Î±×ÀÎ È­¸éÀ¸·Î µ¹¾Æ°£ °æ¿ì µîÀÌ¹Ç·Î Âü°¡Ã¢À» ´Ý¾ÆÁØ´Ù
	MWidget* pDTWaitMatchDlg = m_IDLResource.FindWidget("DuelTournamentWaitMatchDialog");
	if(pDTWaitMatchDlg && pDTWaitMatchDlg->IsVisible() && GetState()!=GUNZ_LOBBY )
		OnDuelTournamentGameUI(false);

	UpdateDuelTournamentWaitMsgDots();

	UpdateCursorEnable();

	// ½ÇÁ¦ °ÔÀÓ¿¡¼­ ³ª°¡´Â Æã¼ÇÀ» ºÎ¸¥´Ù
	if(ZGetGame()!=NULL && m_bLeaveBattleReserved && (m_dwLeaveBattleTime < timeGetTime()))
		LeaveBattle();

	if(specialCase && timeSpecialCase < timeGetTime()) {
		specialCase = false;
		timeSpecialCase = 0;
		OnArrangedTeamGameUI(true, true);
	}

	if(paused) {
		DWORD timeLeft = timePause - timeGetTime();
		if (timeLeft <= 10000 && tenSeconds) {
			tenSeconds = false;
			char szMsg[128];
			sprintf(szMsg, "^2The game will be unpaused in 10 seconds.");
			ZChatOutput(szMsg);
		}
	}

	if (paused && timePause < timeGetTime()) {
		paused = false;
		timePause = 0;
		char szMsg[128];
		sprintf(szMsg, "^2The game has been unpaused.");
		ZChatOutput(szMsg);
		ZGetGame()->m_pMyCharacter->GetStatus().CheckCrc();
		ZGetGame()->m_pMyCharacter->GetStatus().Ref().Freeze = 0;
		ZGetGame()->m_pMyCharacter->GetStatus().MakeCrc();
	}
	__EP(13);

	return true;
}

void ZGameInterface::OnResetCursor()
{
	SetCursorEnable(m_bCursor);
}

void ZGameInterface::SetCursorEnable(bool bEnable)
{
	//	_RPT1(_CRT_WARN,"cursor %d\n",bEnable);
	
	if(m_bCursor==bEnable) return;

	m_bCursor = bEnable;
	MCursorSystem::Show(bEnable);

	/*
	if(!bEnable)
	{
	ShowCursor(FALSE);
	}else
	ShowCursor(TRUE);
	*/

/*	if(m_pCursorSurface)	// RAONHAJE Mouse Cursor HardwareDraw
	{
		RGetDevice()->SetCursorProperties(0,0,m_pCursorSurface);
		RGetDevice()->ShowCursor(bEnable);
	}	*/
}

void ZGameInterface::UpdateCursorEnable()
{
	
	if( GetState()!=GUNZ_GAME ||
		(GetCombatInterface() && GetCombatInterface()->IsShowResult()) ||
		IsMenuVisible() || 
		m_pMsgBox->IsVisible() ||
		m_pConfirmMsgBox->IsVisible() )
		SetCursorEnable(true);
	else
	{
		MWidget* pWidget = m_IDLResource.FindWidget( "112Confirm");
		if ( pWidget && pWidget->IsVisible())
			SetCursorEnable( true);

		else
			SetCursorEnable( false);
	}
}

void ZGameInterface::UpdateDuelTournamentWaitMsgDots()
{
	// 'µà¾óÅä³Ê¸ÕÆ® Âü°¡Áß' ´ÙÀÌ¾ó·Î±× ¸Þ½ÃÁöÀÇ ... ±æÀÌ¸¦ ½Ã°£¿¡ µû¶ó Á¶Á¤ÇÑ´Ù
	if(GetState()!=GUNZ_LOBBY) return;
    
	MLabel *pLabel = (MLabel*)m_IDLResource.FindWidget("DuelTournamentWaitMatchLabel");
	if(pLabel == NULL || !pLabel->IsVisible()) return;

	const char* szText = pLabel->GetText();
	size_t len = strlen(szText);
	if (len == 0) return;

	int nCount = (timeGetTime()/500)%5 + 1;
	char dots[10];
	for(int i=0;i<nCount;i++) {
		dots[i]='.';
	}
	dots[nCount]=0;

	// ±âÁ¸¿¡ ºÙ¾îÀÖ´ø ...À» Á¦¿ÜÇÑ ¿øº» ¹®ÀÚ¿­ÀÇ ±æÀÌ¸¦ ¾Ë¾Æ³½´Ù
	int msgEndPosExceptDots = 0;
	for (size_t pos=len-1; pos>=0; --pos) {
		if (szText[pos] != '.') {
			msgEndPosExceptDots = (int)pos;
			break;
		}
	}

	// ¿øº» ¹®ÀÚ¿­¿¡ »õ ...À» ºÙÀÎ´Ù
	char szBuffer[256];
	strncpy(szBuffer, szText, msgEndPosExceptDots+1);
	szBuffer[msgEndPosExceptDots+1] = 0;
	strcat(szBuffer, dots);
	pLabel->SetText(szBuffer);
}

void ZGameInterface::SetMapThumbnail(const char* szMapName)
{
	SAFE_DELETE(m_pMapThumbnail);

	char szThumbnail[256];
	sprintf(szThumbnail, "maps/%s/%s.rs.bmp", szMapName,szMapName);

	m_pMapThumbnail=Mint::GetInstance()->OpenBitmap(szThumbnail);
	if(!m_pMapThumbnail)
	{
		sprintf(szThumbnail, "maps/%s/%s.bmp", szMapName,szMapName);
		m_pMapThumbnail=Mint::GetInstance()->OpenBitmap(szThumbnail);
	}
}

void ZGameInterface::ClearMapThumbnail()
{
	SAFE_DELETE(m_pMapThumbnail);
}


void ZGameInterface::Reload()
{
	if(!ZGetGame()->m_pMyCharacter->GetItems()->GetSelectedWeapon()) return;
	MMatchItemDesc* pSelectedItemDesc = ZGetGame()->m_pMyCharacter->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pSelectedItemDesc == NULL) return;

	if (pSelectedItemDesc->m_nType.Ref() != MMIT_RANGE)  return;

	if(ZGetGame()->m_pMyCharacter->GetItems()->GetSelectedWeapon()->isReloadable()==false) return;

	ZMyCharacter* pChar = ZGetGame()->m_pMyCharacter;

	//jintriple3 ¸Þ¸ð¸® ÇÁ·Ï½Ã...ºñÆ® ÆÐÅ·..
	ZCharaterStatusBitPacking &uStatus = pChar->m_dwStatusBitPackingValue.Ref();

	// ¶ç¾îÁø »óÅÂ¿¡¼­´Â ÀåÀü¸øÇÔ..
	if(uStatus.m_bBlast || 
		uStatus.m_bBlastFall || 
		uStatus.m_bBlastDrop || 
		uStatus.m_bBlastStand || 
		uStatus.m_bBlastAirmove )
		return;

	if( pChar->GetStateUpper()==ZC_STATE_UPPER_RELOAD) return ;

	uStatus.m_bSpMotion = false;

	ZPostReload();
}

void ZGameInterface::SaveScreenShot()
{
	static unsigned long int st_nLastTime = 0;
	unsigned long int nNowTime = timeGetTime();
#define SCREENSHOT_DELAY		2000

	// 2ÃÊ µô·¹ÀÌ
	if ((nNowTime-st_nLastTime) < SCREENSHOT_DELAY)	return;
	st_nLastTime = nNowTime;


	char szPath[_MAX_PATH];
	char szFilename[_MAX_PATH];
	char szFilenameSafe[_MAX_PATH];

	TCHAR szMyDocPath[MAX_PATH];
	if(GetMyDocumentsPath(szMyDocPath)) {
		strcpy(szPath,szMyDocPath);
		strcat(szPath,GUNZ_FOLDER);
		CreatePath( szPath );
		strcat(szPath,SCREENSHOT_FOLDER);
		CreatePath( szPath );
		strcat(szPath, "/");
	}

	// ÇöÀç °ÔÀÓ Á¤º¸·Î ÆÄÀÏ¸íÀ» ±¸¼º
	SYSTEMTIME t;
	GetLocalTime( &t );
	char szCharName[MATCHOBJECT_NAME_LENGTH];
	ValidateFilename(szCharName, ZGetMyInfo()->GetCharName(), '_');

	sprintf(szFilename,"%s_%4d%02d%02d_%02d%02d%02d",  
		szCharName, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);

	sprintf(szFilenameSafe,"nocharname_%4d%02d%02d_%02d%02d%02d",  
		t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);	// Ä³¸¯¸íÀ» »ý·«ÇÑ ¹öÀü

	/*//±¸ ÀÏ·Ã¹øÈ£ ÆÄÀÏ¸í
	do {
		sprintf(szPath,"Gunz%03d",nsscount++);
		sprintf(screenshotBMP, "%s.bmp", szPath);
		sprintf(screenshotJPG, "%s.jpg", szPath);
	}
	while( (IsExist(screenshotBMP)||(IsExist(screenshotJPG))) && nsscount<1000);
	*/

	LPBYTE data = NULL;
	LPDIRECT3DSURFACE9 frontbuffer=NULL;

	//if(nsscount<1000)
	{

		HRESULT hr;
		LPDIRECT3DDEVICE9 pDevice=RGetDevice();

		D3DDISPLAYMODE d3ddm;
		pDevice->GetDisplayMode( 0,&d3ddm );

		hr=pDevice->CreateOffscreenPlainSurface( d3ddm.Width,d3ddm.Height,D3DFMT_A8R8G8B8,D3DPOOL_SCRATCH,&frontbuffer ,NULL);
		hr=pDevice->GetFrontBufferData(0,frontbuffer); 
		
		//_ASSERT(hr==D3D_OK);

		RECT rt;
		GetWindowRect(g_hWnd,&rt);

		D3DLOCKED_RECT LRECT;
		if(frontbuffer->LockRect(&LRECT,&rt,NULL)==D3D_OK)
		{
			int nWidth=rt.right-rt.left;
			int nHeight=rt.bottom-rt.top;

			int pitch=LRECT.Pitch;
			LPBYTE source=(LPBYTE)LRECT.pBits;

			data=new BYTE[nWidth*nHeight*4];
			for(int i=0;i<nHeight;i++)
			{
				memcpy(data+i*nWidth*4,source+pitch*i,nWidth*4);
			}

			char szFullPath[_MAX_PATH];	// È®ÀåÀÚ´Â ¾Èµé¾îÀÖÀ½
			char szFullPathToNotify[_MAX_PATH];
			
			sprintf(szFullPath, "%s%s", szPath, szFilename);
			sprintf(szFullPathToNotify,GUNZ_FOLDER SCREENSHOT_FOLDER"/""%s.jpg", szFilename);
			bool bSuccess = RScreenShot(nWidth,nHeight,data, szFullPath);

			if(!bSuccess) {	// Ä³¸¯¸í¶§¹®¿¡ ½ÇÆÐÇßÀ» ¼ö ÀÖÀ¸´Ï Ä³¸¯¸íÀ» »ý·«ÇÑ ÆÄÀÏ¸íÀ¸·Î Àç½Ãµµ
				
				sprintf(szFullPath, "%s%s", szPath, szFilenameSafe);
				sprintf(szFullPathToNotify,GUNZ_FOLDER SCREENSHOT_FOLDER"/""%s.jpg", szFilenameSafe);
				bSuccess = RScreenShot(nWidth,nHeight,data, szFullPath);

				if(!bSuccess) goto SCREENSHOTERROR;
			}

			char szOutput[_MAX_PATH*2];
			ZTransMsg( szOutput,MSG_SCREENSHOT_SAVED,1,szFullPathToNotify );
			ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);

			//SAFE_DELETE(data); ¾îÀÌÄí
			delete[] data;
		}
		SAFE_RELEASE(frontbuffer);
	}
	return;

SCREENSHOTERROR:
	//SAFE_DELETE(data);
	delete[] data;
	SAFE_RELEASE(frontbuffer);

	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), 
		ZMsg(MSG_SCREENSHOT_CANT_SAVE) );

	return;
}

void ZGameInterface::ShowMessage(const char* szText, MListener* pCustomListenter, int nMessageID)
{
	if (pCustomListenter)
		m_pMsgBox->SetCustomListener(pCustomListenter);

	char text[1024] ="";

	// nMessageID°¡ 0ÀÌ ¾Æ´Ï¸é ¸Þ¼¼Áö µÚ¿¡ ¸Þ¼¼Áö ¹øÈ£µµ ÇÔ²² Ãâ·ÂÇØÁØ´Ù.(´Ù¸¥ ³ª¶ó ¸»ÀÏ¶§ È®ÀÎÇÏ±â À§ÇÔ)
	if (nMessageID != 0)
	{
		sprintf(text, "%s (M%d)", szText, nMessageID);
	}
	else
	{
		strcpy(text, szText);
	}

	m_pMsgBox->SetText(text);
	m_pMsgBox->Show(true, true);
}

void ZGameInterface::ShowConfirmMessage(const char* szText, MListener* pCustomListenter)
{
	if (pCustomListenter)
		m_pConfirmMsgBox->SetCustomListener(pCustomListenter);

	m_pConfirmMsgBox->SetText(szText);
	m_pConfirmMsgBox->Show(true, true);
}

void ZGameInterface::ShowMessage(int nMessageID)
{
	const char *str = ZMsg( nMessageID );
	if(str)
	{
		char text[1024];
		sprintf(text, "%s (M%d)", str, nMessageID);
		ShowMessage(text);
	}
}

void ZGameInterface::ShowErrorMessage(int nErrorID)
{
	const char *str = ZErrStr( nErrorID );
	if(str)
	{
		char text[1024];
		sprintf(text, "%s (E%d)", str, nErrorID);
		ShowMessage(text);
	}
}

void ZGameInterface::ShowErrorMessage(const char* szErrorMsg, int nErrorID)
{
	char szErrorID[64];
	sprintf(szErrorID, " (E%d)", nErrorID);

	char text[1024];
	strcpy(text, szErrorMsg);
	strcat(text, szErrorID);
	
	ShowMessage(text);
}

void ZGameInterface::ChangeSelectedChar(int nNum)
{
	bool bRequested = false;
	// ¸¸¾à Ä³¸¯ÅÍ Á¤º¸¸¦ ¾È¹Þ¾Ò¾úÀ¸¸é ¼­¹ö¿¡ ´Þ¶ó°í ¿äÃ»ÇÑ´Ù.
	if ((!ZCharacterSelectView::m_CharInfo[nNum].m_bLoaded) && (!ZCharacterSelectView::m_CharInfo[nNum].m_bRequested))
	{
		ZPostAccountCharInfo(nNum);
		GetCharacterSelectView()->UpdateInterface(nNum);
		//ZCharacterSelectView::SetSelectedCharacter(nNum);
		ZCharacterSelectView::m_CharInfo[nNum].m_bRequested = true;
		bRequested = true;
	}


	// Ä³¸¯ÅÍ º¸ÀÌ±â
	if ((!bRequested) && (GetCharacterSelectView() != NULL))
	{
		GetCharacterSelectView()->SelectChar(nNum);
	}



}

void ZGameInterface::OnCharSelectionCreate(void)
{
	ZApplication::GetSoundEngine()->OpenMusic( BGMID_INTRO, ZApplication::GetFileSystem());

	EnableCharSelectionInterface(true);

	if (m_pCharacterSelectView!=NULL) SAFE_DELETE(m_pCharacterSelectView);
	m_pCharacterSelectView = new ZCharacterSelectView();
	m_pCharacterSelectView->SetBackground(m_pBackground);
	m_pCharacterSelectView->SelectChar(ZCharacterSelectView::GetSelectedCharacter());

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MWidget* pWidget = pResource->FindWidget( "CS_SelectCharDefKey");
	if ( pWidget)
		pWidget->Enable( true);

	pWidget = pResource->FindWidget( "CharSel_GameRoomUser");
	if ( pWidget)
	{
		if ( ZGetMyInfo()->GetPGradeID() == MMPG_PREMIUM_IP)
			pWidget->Show( true);
		else
			pWidget->Show( false);
	}

#ifdef LOCALE_NHNUSA
	GetNHNUSAReport().ReportDisplaySelCharacter();
#endif
}

void ZGameInterface::OnCharSelect(void)
{
	if( m_pCharacterSelectView->SelectMyCharacter() == false )
		return;

	EnableCharSelectionInterface( false);

	if ( m_pBackground)
		m_pBackground->SetScene( LOGIN_SCENE_SELECTCHAR);

#ifdef LOCALE_NHNUSA
	GetNHNUSAReport().ReportSelectCharacter();
#endif
}

void ZGameInterface::OnCharSelectionDestroy(void)
{
	ShowWidget("CharSelection", false);
	if (m_pCharacterSelectView!=NULL) SAFE_DELETE(m_pCharacterSelectView);
}

void ZGameInterface::OnCharCreationCreate(void)
{
	ShowWidget("CharSelection", false);
	ShowWidget("CharCreation", true);

	if (m_pCharacterSelectView!=NULL) SAFE_DELETE(m_pCharacterSelectView);
	m_pCharacterSelectView = new ZCharacterSelectView();
	m_pCharacterSelectView->SetBackground(m_pBackground);
	m_pCharacterSelectView->SetState(ZCharacterSelectView::ZCSVS_CREATE);
	m_pCharacterSelectView->OnChangedCharCostume();

#ifdef LOCALE_NHNUSA
	GetNHNUSAReport().ReportDisplayCreateCharacter();
#endif
}

void ZGameInterface::OnCharCreationDestroy(void)
{
	ShowWidget("CharCreation", false);
	ShowWidget("CharSelection", true);

	if (m_pCharacterSelectView!=NULL) SAFE_DELETE(m_pCharacterSelectView);
}

void ZGameInterface::ChangeToCharSelection(void)
{
	ZCharacterSelectView::ClearCharInfo();
	ZPostAccountCharList(ZGetMyInfo()->GetSystemInfo()->pbyGuidAckMsg);		// update sgk 0409 // Ä³¸¯ÅÍ ¸®½ºÆ® ¿äÃ»
}

void ZGameInterface::OnInvalidate()
{
	ZGetWorldManager()->OnInvalidate();
	if(m_pGame)
		m_pGame->OnInvalidate();
	if(m_pBackground)
		m_pBackground->OnInvalidate();
	if( m_pCharacterSelectView)
		m_pCharacterSelectView->OnInvalidate();
	//ZCharacterViewList* pCharViewList = (ZCharacterViewList*)m_IDLResource.FindWidget("StagePlayerList");
	//for(list<ZMeshView*>::iterator iter = ZMeshView::msMeshViewList.begin(); iter != ZMeshView::msMeshViewList.end(); ++iter )
	//{
	//	ZMeshView* pmv = *iter;
	//	if( pmv )
	//		pmv->OnInvalidate();
	//}

	if(ZGetEffectManager())
		ZGetEffectManager()->OnInvalidate();	

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	if(pCharView!= 0) pCharView->OnInvalidate();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformation");
	if(pCharView!= 0) pCharView->OnInvalidate();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformationShop");
	if(pCharView!= 0) pCharView->OnInvalidate();

	if (m_pCombatInterface)
		m_pCombatInterface->OnInvalidate();
}

void ZGameInterface::OnRestore()
{
	ZGetWorldManager()->OnRestore();
	if(m_pGame)
		m_pGame->OnRestore();
	if(m_pBackground)
		m_pBackground->OnRestore();
//	ResetCursor();	
	if( m_pCharacterSelectView)
		m_pCharacterSelectView->OnRestore();
	//ZCharacterViewList* pCharViewList = (ZCharacterViewList*)m_IDLResource.FindWidget("StagePlayerList");
	//for(list<ZMeshView*>::iterator iter = ZMeshView::msMeshViewList.begin(); iter != ZMeshView::msMeshViewList.end(); ++iter )
	//{
	//	ZMeshView* pmv = *iter;
	//	if( pmv )
	//		pmv->OnRestore();
	//}

	if(ZGetEffectManager())
		ZGetEffectManager()->OnRestore();	

	ZCharacterView* pCharView = (ZCharacterView*)m_IDLResource.FindWidget("Stage_Charviewer");
	if(pCharView!= 0) pCharView->OnRestore();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformation");
	if(pCharView!= 0) pCharView->OnRestore();
	pCharView = (ZCharacterView*)m_IDLResource.FindWidget("EquipmentInformationShop");
	if(pCharView!= 0) pCharView->OnRestore();

	if (m_pCombatInterface)
		m_pCombatInterface->OnRestore();
}


void ZGameInterface::UpdateBlueRedTeam( void)
{
	MButton* pBlueTeamBtn  = (MButton*)m_IDLResource.FindWidget("StageTeamBlue");
	MButton* pBlueTeamBtn2 = (MButton*)m_IDLResource.FindWidget("StageTeamBlue2");
	MButton* pRedTeamBtn  = (MButton*)m_IDLResource.FindWidget("StageTeamRed");
	MButton* pRedTeamBtn2 = (MButton*)m_IDLResource.FindWidget("StageTeamRed2");
	if ((pRedTeamBtn == NULL) || (pBlueTeamBtn == NULL) || (pRedTeamBtn2 == NULL) || (pBlueTeamBtn2 == NULL))
		return;

	// ÆÀÇÃ(ÆÀÀü)
	if ( m_bTeamPlay)
	{
		pRedTeamBtn->Show(true);
		pRedTeamBtn2->Show(true);
		pBlueTeamBtn->Show(true);
		pBlueTeamBtn2->Show(true);

		ZPlayerListBox* pListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("StagePlayerList_");

		if(pListBox)
		{
			int nR=0,nB=0;

			// À§Á¬¿¡¼­ ¼ýÀÚ¸¦ ¼¼¿Â´Ù. ÁÁÁö¾Ê´Ù.
			for( int i = 0;  i < pListBox->GetCount(); i++)
			{
				ZStagePlayerListItem *pSItem = (ZStagePlayerListItem*)pListBox->Get(i);
				if(pSItem->m_nTeam == 1)		nR++;
				else if(pSItem->m_nTeam == 2)	nB++;
			}

			char buffer[64];
			ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
			ZBmNumLabel *pNumLabel;

//			sprintf(buffer,"%d",nB);
			sprintf(buffer,"%s:%d", ZMsg( MSG_WORD_BLUETEAM), nB);
			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfBlueTeam");
			if ( pNumLabel)
			{
				pNumLabel->SetText(buffer);
				pNumLabel->Show( true);
			}
			MButton* pButton = (MButton*)pRes->FindWidget( "StageTeamBlue");
			if ( pButton)
				pButton->SetText( buffer);

//			sprintf(buffer,"%d",nR);
			sprintf(buffer,"%s:%d", ZMsg( MSG_WORD_REDTEAM), nR);
			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfRedTeam");
			if ( pNumLabel)
			{
				pNumLabel->SetIndexOffset(16);
				pNumLabel->SetText(buffer);
				pNumLabel->Show( true);
			}
			pButton = (MButton*)pRes->FindWidget( "StageTeamRed");
			if ( pButton)
				pButton->SetText( buffer);

			sprintf( buffer, "%s : %d", ZMsg( MSG_WORD_PLAYERS), nB+nR);
			MLabel* pLabel = (MLabel*)pRes->FindWidget( "Stage_NumOfPerson");
			if ( pLabel)
				pLabel->SetText( buffer);
		}
	}
	else
	{
		pRedTeamBtn->Show(false);
		pRedTeamBtn2->Show(false);
		pBlueTeamBtn->Show(false);
		pBlueTeamBtn2->Show(false);

		ZPlayerListBox* pListBox = (ZPlayerListBox*)m_IDLResource.FindWidget("StagePlayerList_");

		if(pListBox)
		{
			int nPlayerNum=0;

			// À§Á¬¿¡¼­ ¼ýÀÚ¸¦ ¼¼¿Â´Ù. ÁÁÁö¾Ê´Ù.
			for( int i = 0;  i < pListBox->GetCount(); i++)
			{
				ZStagePlayerListItem *pSItem = (ZStagePlayerListItem*)pListBox->Get(i);
				if( pSItem->m_nTeam == 0)
					nPlayerNum++;
			}

			ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
			ZBmNumLabel *pNumLabel;

			char buffer[64];

			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfBlueTeam");
			if ( pNumLabel)
				pNumLabel->Show( false);

			pNumLabel = (ZBmNumLabel*)pRes->FindWidget("StageNumOfRedTeam");
			if ( pNumLabel)
				pNumLabel->Show( false);

			sprintf( buffer, "%s : %d", ZMsg( MSG_WORD_PLAYERS), nPlayerNum);
			MLabel* pLabel = (MLabel*)pRes->FindWidget( "Stage_NumOfPerson");
			if ( pLabel)
				pLabel->SetText( buffer);
		}
	}
}

void ZGameInterface::ShowInterface(bool bShowInterface)
{
	m_bShowInterface = bShowInterface;

	if (m_nState != GUNZ_GAME)
	{
		SetCursorEnable(bShowInterface);
	}

	// Login
	if (m_nState == GUNZ_LOGIN)
	{
		ShowWidget("Login", m_bShowInterface);
	}
	else if (m_nState == GUNZ_CHARSELECTION)
	{
		ShowWidget("CharSelection", m_bShowInterface);
	}
	else if (m_nState == GUNZ_GAME)
	{
		bool bConsole=ZGetConsole()->IsVisible();
		bool bLogFrame=m_pLogFrame->IsVisible();

		//		m_pCombatInterface->Show(m_bShowInterface);
		m_pLogFrame->Show(m_bShowInterface);
		ZGetConsole()->Show(m_bShowInterface);
		ShowWidget("CombatInfo1",m_bShowInterface);
		ShowWidget("CombatInfo2",m_bShowInterface);
		ShowWidget("Time", m_bShowInterface);
		ZGetConsole()->Show(bConsole);
		m_pLogFrame->Show(bLogFrame);
	}
}

/*
void ZGameInterface::OnMoveMouse(int iDeltaX,int iDeltaY)
{
	bool bRotateEnable=false;
	if(!m_pMyCharacter->m_bWallJump && !m_pMyCharacter->m_bWallJump2 && !m_pMyCharacter->m_bWallHang && 
		!m_pMyCharacter->m_bTumble && !m_pMyCharacter->m_bBlast && !m_pMyCharacter->m_bBlastStand && !m_pMyCharacter->m_bBlastDrop )
		bRotateEnable=true;


	if (RIsActive())
	{
		static float lastanglex,lastanglez;
		if(bRotateEnable)		{
			float fRotateStep = 0.0005f * Z_MOUSE_SENSITIVITY*10.0f;
			m_Camera.m_fAngleX += (iDeltaY * fRotateStep);
			m_Camera.m_fAngleZ += (iDeltaX * fRotateStep);

			m_Camera.m_fAngleX=min(CAMERA_ANGLEX_MAX,
								max(CAMERA_ANGLEX_MIN,m_Camera.m_fAngleX));

			lastanglex=m_Camera.m_fAngleX;
			lastanglez=m_Camera.m_fAngleZ;
		}else
		{
			// °¢µµÁ¦ÇÑÀÌ ÇÊ¿äÇÏ´Ù

			float fRotateStep = 0.0005f * Z_MOUSE_SENSITIVITY*10.0f;
			m_Camera.m_fAngleX += (iDeltaY * fRotateStep);
			m_Camera.m_fAngleZ += (iDeltaX * fRotateStep);

			m_Camera.m_fAngleX=min(CAMERA_ANGLEX_MAX,max(CAMERA_ANGLEX_MIN,
								min(lastanglex+pi/4.f,max(lastanglex-pi/4.f,m_Camera.m_fAngleX))));

			//_ASSERT(m_Camera.m_fAngleX>=CAMERA_ANGLEX_MIN && m_Camera.m_fAngleX<=CAMERA_ANGLEX_MAX );

			m_Camera.m_fAngleZ=min(lastanglez+pi/4.f,
								max(lastanglez-pi/4.f,m_Camera.m_fAngleZ));



		}
	}
}
*/

void ZGameInterface::OnResponseShopItemList( const vector< MTD_ShopItemInfo*> &vShopItemList , const vector<MTD_GambleItemNode*>& vGItemList )
{
	ZGetShop()->SetItemsAll( vShopItemList );
	ZGetShop()->SetItemsGamble( vGItemList );
	//ZGetShop()->Serialize();
}

void ZGameInterface::OnResponseCharacterItemList(MUID* puidEquipItem
												 , MTD_ItemNode* pItemNodes
												 , int nItemCount
												 , MTD_GambleItemNode* pGItemNodes
												 , int nGItemCount )
{
	ZGetMyInfo()->GetItemList()->SetItemsAll(pItemNodes, nItemCount);
	ZGetMyInfo()->GetItemList()->SetGambleItemAll( pGItemNodes, nGItemCount );
	ZGetMyInfo()->GetItemList()->SetEquipItemsAll(puidEquipItem);
	ZGetMyInfo()->GetItemList()->Serialize();

	// »óÁ¡¿¡¼­ Ä³¸¯ÅÍ UI Text Á¤º¸¸¦ ¾÷µ¥ÀÌÆ® ÇØÁØ´Ù (·¹º§, ¹Ù¿îÆ¼, Ã¼·Â, ¹æ¾î, ¹«°Ô)
	GetShopEquipInterface()->DrawCharInfoText();
}



void ZGameInterface::FinishGame()
{
	ZGetGame()->StopRecording();
	ZGetGameInterface()->GetCombatInterface()->Finish();
	m_bGameFinishLeaveBattle = true;
}

void ZGameInterface::SerializeStageInterface()
{
	// Äù½ºÆ® ¸ðµå...  ¸ÓÇÏ´Â ÄÚµåÀÎÁö ¸ð¸£°ÚÀ½...  -_-;
	// ³ªÁß¿¡ ÇÊ¿ä¾ø°Ô µÇ¸é ÀÌ ÇÔ¼ö ÀÚÃ¼¸¦ »èÁ¦ÇÏ°í ¹Ù·Î OnStageInterfaceSettupÀ¸·Î ³Ñ¾î°¡°Ô...
/*
#ifdef _QUEST
	ZStageSetting::InitStageSettingGameType();

	if ( ZGetGameClient()->GetServerMode() == MSM_CLAN)
	{
		// ³ªÁß¿¡ ·£´ý¸Ê ±¸ÇöÇÒ¶§±îÁö ±×³É ¸Ê ÇÏ³ª¸¸ »ç¿ë
		MComboBox* pCBMapSelection = (MComboBox*)m_IDLResource.FindWidget( "MapSelection");
		if ( pCBMapSelection)
		{
			int nSelected = pCBMapSelection->GetSelIndex();
			pCBMapSelection->RemoveAll();

			InitMaps( pCBMapSelection);

			if ( nSelected >= pCBMapSelection->GetCount())
				nSelected = pCBMapSelection->GetCount() - 1;

			pCBMapSelection->SetSelIndex( nSelected);
			pCBMapSelection->SetText( "asdfasdfa");
		}
	}
#endif
*/
	MComboBox* pCombo = (MComboBox*)m_IDLResource.FindWidget( "MapSelection");
	if ( pCombo)
		InitMaps( pCombo);

	ZApplication::GetStageInterface()->OnStageInterfaceSettup();
}


void ZGameInterface::HideAllWidgets()
{
	ShowWidget("Login", false);
	ShowWidget("Lobby", false);
	ShowWidget("Stage", false);
	ShowWidget("Game", false);
	ShowWidget("Option", false);
	ShowWidget("CharSelection", false);
	ShowWidget("CharCreation", false);
	ShowWidget("Shop", false);
//	ShowWidget("LobbyChannelPlayerList", false);

	// dialog
	ShowWidget("StageSettingFrame", false);
	ShowWidget("BuyConfirm", false);
	ShowWidget("Equipment", false);
	ShowWidget("StageCreateFrame", false);
	ShowWidget("PrivateStageJoinFrame", false);
}

bool SetWidgetToolTipText(char* szWidget,const char* szToolTipText, MAlignmentMode mam) {

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if(pResource==NULL)		return false;
	if(!szToolTipText)		return false;


	MWidget* pWidget = pResource->FindWidget(szWidget);

	if(pWidget) {

		
		if(!szToolTipText[0]) {
			pWidget->DetachToolTip();
		}
		else {
//			pWidget->AttachToolTip(szToolTipText);
			pWidget->AttachToolTip(new ZToolTip(szToolTipText, pWidget, mam));
		}
/*
		MToolTip* pTT =	pWidget->GetToolTip();
		if(pTT) {
			if(!szToolTipText[0]) {
				pTT->AttachToolTip()
			}
			else {

			}
//			pTT->SetText(szToolTipText);
			return true;
		}
*/
	}
	return false;
}

// #define CheckLine(str) 
// ÃÖ´ë 200 À¸·ÎÀâ°í -14 ±îÁö Ã¼Å© ±ÛÀÚ°¡ Àß¸®Áö ¾Êµµ·Ï..

 // zmaplistbox.cpp ¿¡µµ ¼±¾ðµÇ¾îÀÖ´Ù..

bool GetItemDescName(string& str,DWORD nItemID)
{
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

	if ( pItemDesc == NULL)
	{
		str.clear();
		return false;
	}

	str = (string)pItemDesc->m_pMItemName->Ref().m_szItemName;
	return false;
}

bool GetItemDescStr(string& str,DWORD nItemID) {

	static char temp[1024];

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);

	if(pItemDesc==NULL) {
		str.clear();
//		str += " ";
		return false;
	}

	bool bAdd = false;
	bool bLine = false;
	int	 nLine = 0;
	int  nLen = 0;

	if(pItemDesc->m_pMItemName->Ref().m_szItemName) {
		str += pItemDesc->m_pMItemName->Ref().m_szItemName;
		bAdd = true;
	}

	if(pItemDesc->m_nResLevel.Ref()) {
		if(bAdd) str += " / ";
		sprintf(temp,"Á¦ÇÑ·¹º§:%d",pItemDesc->m_nResLevel.Ref());

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nWeight.Ref()){
		if(bAdd) str += " / ";
		sprintf(temp,"¹«°Ô:%d",pItemDesc->m_nWeight);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nMaxBullet.Ref()) {
		if(bAdd) str += " / ";
		sprintf(temp,"ÃÖ´ëÅº¼ö : %d",pItemDesc->m_nMaxBullet);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	
	if(pItemDesc->m_nMagazine.Ref()) {
		if(bAdd) str += " / ";
		sprintf(temp,"ÅºÃ¢ : %d",pItemDesc->m_nMagazine);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nDamage.Ref()) {
		if(bAdd) str += " / ";
		sprintf(temp,"°ø°Ý·Â : %d",pItemDesc->m_nDamage);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nDelay.Ref()) {
		if(bAdd) str += " / ";
		sprintf(temp,"µô·¹ÀÌ : %d",pItemDesc->m_nDelay);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nReloadTime.Ref()) {
		if(bAdd) str += " / ";
		sprintf(temp,"ÀåÀü½Ã°£ : %d",pItemDesc->m_nReloadTime);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nHP.Ref()) {
		if(bAdd) str += " / ";
		sprintf(temp,"+HP : %d",pItemDesc->m_nHP);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nAP.Ref()) {
		if(bAdd) str += " / ";
		sprintf(temp,"+AP : %d",pItemDesc->m_nAP);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	if(pItemDesc->m_nMaxWT.Ref()) {
		if(bAdd) str += " / ";
		sprintf(temp,"+ÃÖ´ë¹«°Ô : %d",pItemDesc->m_nMaxWT);

		nLen = (int)strlen(temp);
		if((int)str.size() + nLen > (nLine+1) * MAX_TOOLTIP_LINE_STRING+3) { str += "\n"; nLine++; }

		str += temp;
		bAdd = true;
	}

	return true;
}

void ZGameInterface::ShowEquipmentDialog(bool bShow)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if (bShow)
	{	// ½ºÅ×ÀÌÁö ¾È¿¡¼­ Àåºñ ¹öÆ° ¼±ÅÃ
		ShowWidget( "Lobby", false);
		ShowWidget( "Stage", false);
		ShowWidget( "Shop",  false);



		MButton* pButton = (MButton*)pResource->FindWidget("Equipment_to_Shop");
		MLabel* pLabel = (MLabel*)pResource->FindWidget("Equip_Message");
		if ( pButton && pLabel)
		{
			char buf[ 256];
			if ( ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE)
			{
				pButton->Show(true);
				sprintf( buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg( MSG_WORD_STAGE), ZMsg( MSG_WORD_EQUIPMENT));
				pLabel->SetAlignment(MAM_HCENTER);
				pLabel->SetText(buf);
			}
			else
			{
				pButton->Show( true);
				sprintf( buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg( MSG_WORD_LOBBY), ZMsg( MSG_WORD_EQUIPMENT));
				pLabel->SetAlignment(MAM_HCENTER);
				pLabel->SetText(buf);
			}
		}

		MWidget* pWidget = pResource->FindWidget("Equipment");
		if(pWidget!=NULL) pWidget->Show(true, true);

		BEGIN_WIDGETLIST( "EquipmentInformation", pResource, ZCharacterView*, pCharacterView);
		ZMyInfo* pmi = ZGetMyInfo();
		ZMyItemList* pil = ZGetMyInfo()->GetItemList();

		unsigned long int nEquipedItemID[MMCIP_END];

		for (int i = 0; i < MMCIP_END; i++)
		{
			nEquipedItemID[i] = pil->GetEquipedItemID(MMatchCharItemParts(i));
		}
		pCharacterView->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nEquipedItemID);
		END_WIDGETLIST();

		GetShopEquipInterface()->SelectEquipmentTab(0);

//		ZPostRequestGetCharQuestItemInfo( ZGetGameClient()->GetPlayerUID());
		ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());
		ZPostStageState( ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_EQUIPMENT);

		// Animation sprite
		MPicture* pPicture = 0;
		pPicture = (MPicture*)pResource->FindWidget("Equip_StripBottom");
 		if( pPicture != NULL)
			pPicture->SetAnimation( 0, 1000.0f);
		pPicture = (MPicture*)pResource->FindWidget("Equip_StripTop");
		if( pPicture != NULL)
			pPicture->SetAnimation( 1, 1000.0f);

		pWidget = pResource->FindWidget("Equip");
		if ( pWidget != NULL)
			pWidget->Enable( false);
		pWidget = pResource->FindWidget("SendAccountItemBtn");
		if ( pWidget != NULL)
			pWidget->Enable( false);
		pWidget = pResource->FindWidget("BringAccountItemBtn");
		if ( pWidget != NULL)
			pWidget->Enable( false);
		pWidget = pResource->FindWidget("BringAccountSpendableItemConfirmOpen");
		if ( pWidget != NULL)
			pWidget->Enable( false);

		MTextArea* pTextArea = (MTextArea*)pResource->FindWidget("Equip_ItemDescription");
		if(pTextArea)	pTextArea->SetText("");

		/*pTextArea = (MTextArea*)pResource->FindWidget("Equip_ItemDescription3");
		if ( pTextArea)
		{
			pTextArea->SetTextColor( MCOLOR( 180, 180, 180));
			pTextArea->SetText( ZMsg( MSG_SHOPMSG));
		}*/

		/*pPicture = (MPicture*)pResource->FindWidget("Equip_ItemIcon");
		pPicture->SetBitmap( NULL);*/
		GetShopEquipInterface()->DrawCharInfoText();
	}
	else
	{	// ½ºÅ×ÀÌÁö ¾È Àåºñºä¾î¿¡¼­ ³ª°¡±â ¹öÆ° ¼±ÅÃ
		ZPostStageState( ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_NONREADY);

		MWidget* pWidget = pResource->FindWidget("Equipment");
		if ( pWidget!=NULL)
			pWidget->Show(false);

		if ( ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE)
			ShowWidget( "Stage", true);
		else
			ShowWidget( "Lobby", true);
		GetShopEquipInterface()->DrawCharInfoText();

		// ½æ³×ÀÏ ÀÌ¹ÌÁö ¾ð·ÎµåÇØ¼­ ¸Þ¸ð¸® È®º¸
		GetItemThumbnailMgr()->UnloadTextureTemporarily();
	}
}

void ZGameInterface::ShowShopDialog(bool bShow)
{
	if (bShow)
	{	// »óÁ¡ ¼±ÅÃ
		ShowWidget("Lobby", false);
		ShowWidget("Stage", false);
		ShowWidget("Equipment", false);

		ZGetShop()->Create();
		ZGetShop()->Serialize();

		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();



		MPicture* pPicture = 0;
		pPicture = (MPicture*)pResource->FindWidget("Shop_StripBottom");
		if (pPicture != NULL)
			pPicture->SetAnimation(0, 1000.0f);
		pPicture = (MPicture*)pResource->FindWidget("Shop_StripTop");
		if (pPicture != NULL)
			pPicture->SetAnimation(1, 1000.0f);

		// ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_SHOP);

		MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Shop_ItemDescription");
		if (pTextArea)	pTextArea->SetText("");

		//pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Shop_ItemIcon");
		//pPicture->SetBitmap( NULL);

		char buf[256];
		MButton* pButton = (MButton*)pResource->FindWidget("Shop_to_Equipment2");
		//MButton* pButton2 = (MButton*)pResource->FindWidget("Shop_to_Equipment2");
		MLabel* pLabel = (MLabel*)pResource->FindWidget("Shop_Message");
		if (pLabel)
		{
			char buf[256];
			if (ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE)
			{
				//pButton->Show(true);
				sprintf(buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg(MSG_WORD_STAGE), ZMsg(MSG_WORD_SHOP));
				pLabel->SetAlignment(MAM_HCENTER);
				pLabel->SetText(buf);
				ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_EQUIPMENT);
			}
			else
			{
				//pButton->Show(true);
				sprintf(buf, "%s > %s > %s", ZGetGameClient()->GetServerName(), ZMsg(MSG_WORD_LOBBY), ZMsg(MSG_WORD_SHOP));
				pLabel->SetAlignment(MAM_HCENTER);
				pLabel->SetText(buf);
			}
		}

		MWidget* pWidget = pResource->FindWidget("Shop");
		if(pWidget!=NULL)
			pWidget->Show(true, true);
		pWidget = pResource->FindWidget("BuyConfirmCaller");
		if (pWidget != NULL)
			pWidget->Enable(false);
		pWidget = pResource->FindWidget("SellConfirmCaller");
		if (pWidget != NULL)
			pWidget->Enable(false);

		GetShopEquipInterface()->SelectShopTab(0);

		ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());

		GetShopEquipInterface()->DrawCharInfoText();
	}
	else
	{	// Àåºñ ¼±ÅÃ
		ZPostStageState(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID(), MOSS_NONREADY);

		ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		MWidget* pWidget = pResource->FindWidget("Shop");
		if(pWidget!=NULL) pWidget->Show(false);

		if (ZApplication::GetGameInterface()->GetState() == GUNZ_STAGE)
			ShowWidget("Stage", true);
		else
			ShowWidget("Lobby", true);

		GetShopEquipInterface()->DrawCharInfoText();

		// ½æ³×ÀÏ ÀÌ¹ÌÁö ¾ð·ÎµåÇØ¼­ ¸Þ¸ð¸® È®º¸
		GetItemThumbnailMgr()->UnloadTextureTemporarily();
	}
}








void ZGameInterface::EnableCharSelectionInterface(bool bEnable)
{
	MWidget* pWidget;

	pWidget = m_IDLResource.FindWidget("CS_SelectChar");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_DeleteChar");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_CreateChar");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Prev");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Name_Pre");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Name_Next");
	if (pWidget) pWidget->Enable(bEnable);
	pWidget = m_IDLResource.FindWidget("CS_Name");
	if (pWidget) pWidget->Enable(bEnable);

	if ( !bEnable)
	{
		for ( int i = 0;  i < MAX_CHAR_COUNT;  i++)
		{
			char szName[ 256];
			sprintf( szName, "CharSel_SelectBtn%d", i);
			pWidget = m_IDLResource.FindWidget( szName);
			if ( pWidget)
				pWidget->Show( false);
		}
	}

	if ( bEnable && (ZCharacterSelectView::GetNumOfCharacter() == 0))
	{
		pWidget = m_IDLResource.FindWidget( "CS_SelectChar");
		if (pWidget) pWidget->Enable( false);
		pWidget = m_IDLResource.FindWidget( "CS_DeleteChar");
		if (pWidget) pWidget->Enable( false);
	}

	if ( bEnable && (ZCharacterSelectView::GetNumOfCharacter() >= MAX_CHAR_COUNT))
	{
		pWidget = m_IDLResource.FindWidget( "CS_CreateChar");
		if (pWidget) pWidget->Enable( false);
	}
}

void ZGameInterface::EnableLobbyInterface(bool bEnable)
{

	EnableWidget("LobbyOptionFrame", bEnable);			// ¿É¼Ç ¹öÆ°
	EnableWidget("LobbyOptionFrame2", bEnable);
	EnableWidget("Lobby_Charviewer_info", bEnable);		// Á¤º¸º¸±â ¹öÆ°
	EnableWidget("StageJoin", bEnable);					// °ÔÀÓÂü°¡ ¹öÆ°
	EnableWidget("StageCreateFrameCaller", bEnable);	// °ÔÀÓ»ý¼º ¹öÆ°
	EnableWidget("ShopCaller", bEnable);				// »óÁ¡ ¹öÆ°
	EnableWidget("ShopCaller1", bEnable);
	EnableWidget("EquipmentCaller", bEnable);			// Àåºñ ¹öÆ°
	EnableWidget("EquipmentCaller2", bEnable);
	EnableWidget("EquipmentCaller4", bEnable);
	EnableWidget("ReplayCaller", bEnable);				// ¸®ÇÃ·¹ÀÌ ¹öÆ°
	EnableWidget("CharSelectionCaller", bEnable);		// Ä³¸¯ÅÍ º¯°æ ¹öÆ°
	EnableWidget("Logout", bEnable);					// ·Î±×¾Æ¿ô ¹öÆ°
	EnableWidget("QuickJoin", bEnable);					// ÄüÁ¶ÀÎ ¹öÆ°
	EnableWidget("QuickJoin2", bEnable);				// ÄüÁ¶ÀÎ ¹öÆ°
	EnableWidget("ChannelListFrameCaller", bEnable);	// Ã¤³Îº¯°æ ¹öÆ°
	EnableWidget("StageList", bEnable);					// ¹æ¸®½ºÆ®
	EnableWidget("Lobby_StageList",bEnable);
	EnableWidget("LobbyChannelPlayerList", bEnable);
	EnableWidget("ChannelChattingOutput", bEnable);
	EnableWidget("ChannelChattingInput", bEnable);

	if (bEnable)
	{
		MMatchServerMode nCurrentServerMode = ZGetGameClient()->GetServerMode();
		MCHANNEL_TYPE nCurrentChannelType = ZGetGameClient()->GetChannelType();
		bool bClanBattleUI = (nCurrentServerMode== MSM_CLAN) && (nCurrentChannelType==MCHANNEL_TYPE_CLAN);
		ZGetGameInterface()->InitClanLobbyUI(bClanBattleUI);
	}

	// ½ºÅ×ÀÌÁö°¡ ¿­¸®¸é °ÔÀÓ ¼¼ÆÃ°ü·Ã À§Á¬ ¼¼ÆÃ(°ÔÀÓ½ÃÀÛ¹öÆ° Æ÷ÇÔ)
	ZApplication::GetStageInterface()->SetEnableWidgetByRelayMap(!bEnable);
}

void ZGameInterface::EnableStageInterface(bool bEnable)
{
	EnableWidget("Stage_Charviewer_info", bEnable);		// Á¤º¸º¸±â ¹öÆ°
	EnableWidget("StagePlayerNameInput_combo", bEnable);
	EnableWidget("GameStart", bEnable);					// °ÔÀÓ½ÃÀÛ ¹öÆ°
	MButton* pButton = (MButton*)m_IDLResource.FindWidget("StageReady");				// ·¹µð ¹öÆ°
	if ( pButton)
	{
		pButton->Enable( bEnable);
		pButton->SetCheck( false);
	}
	EnableWidget("ForcedEntryToGame", bEnable);			// ³­ÀÔ ¹öÆ°
	EnableWidget("ForcedEntryToGame2", bEnable);		// ³­ÀÔ ¹öÆ°
	EnableWidget("StageTeamBlue",  bEnable);			// blueÆÀ ¼³Á¤ ¹öÆ°
	EnableWidget("StageTeamBlue2", bEnable);			// blueÆÀ ¼³Á¤ ¹öÆ°
	EnableWidget("StageTeamRed",  bEnable);				// redÆÀ ¼³Á¤ ¹öÆ°
	EnableWidget("StageTeamRed2", bEnable);				// redÆÀ ¼³Á¤ ¹öÆ°
	EnableWidget("ShopCaller", bEnable);				// »óÁ¡ ¹öÆ°
	EnableWidget("ShopCaller1", bEnable);
	EnableWidget("EquipmentCaller", bEnable);			// Àåºñ ¹öÆ°
	EnableWidget("EquipmentCaller2", bEnable);
	EnableWidget("EquipmentCaller4", bEnable);
	EnableWidget("StageSettingCaller", bEnable);		// ¹æ¼³Á¤ ¹öÆ°
	EnableWidget("StageObserverBtn", bEnable);			// °üÀü Ã¼Å© ¹öÆ°
	EnableWidget("Lobby_StageExit", bEnable);

	
	// ³ª°¡±â ¹öÆ°

	EnableWidget("MapSelection", bEnable);				// ¸Ê¼±ÅÃ ÄÞº¸¹Ú½º

	if( ZGetGameClient()->AmIStageMaster() == true )
		EnableWidget("StageType", true);					// °ÔÀÓ¹æ½Ä ÄÞº¸¹Ú½º
	else
		EnableWidget("StageType", false);					// °ÔÀÓ¹æ½Ä ÄÞº¸¹Ú½º

	EnableWidget("StageMaxPlayer", bEnable);			// ÃÖ´ëÀÎ¿ø ÄÞº¸¹Ú½º
	EnableWidget("StageRoundCount", bEnable);			// °æ±âÈ½¼ö ÄÞº¸¹Ú½º

	EnableWidget("StagePlayerList_", bEnable);
}

void ZGameInterface::SetRoomNoLight( int d )
{
	/*
	MTabCtrl *pTab = (MTabCtrl*)m_IDLResource.FindWidget("Lobby_RoomNoControl");
	if( pTab ) 
		pTab->SetSelIndex(d-1);
	*/
    
	/*
	for(int i=1;i<=6;i++)
	{
		char szBuffer[64];
		sprintf(szBuffer, "Lobby_RoomNo%d", i);
		MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szBuffer);
		if(pButton)
		{
			bool bCheck = (i==d);
			pButton->SetCheck(bCheck);
		}
	}
	*/

	char szBuffer[64];
	sprintf(szBuffer, "Lobby_RoomNo%d", d);
	MButton* pButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szBuffer);
	if(pButton) 
		pButton->SetCheck(true);

}

void ZGameInterface::ShowPrivateStageJoinFrame(const char* szStageName)
{
	MEdit* pStageNameEdit = (MEdit*)m_IDLResource.FindWidget("PrivateStageName");
	if (pStageNameEdit)
	{
		pStageNameEdit->SetText(szStageName);
		pStageNameEdit->Enable(false);
	}
	MEdit* pPassEdit = (MEdit*)m_IDLResource.FindWidget("PrivateStagePassword");
	if (pPassEdit!=NULL)
	{
		pPassEdit->SetMaxLength(STAGEPASSWD_LENGTH);
		pPassEdit->SetText("");
		pPassEdit->SetFocus();
	}

	MWidget* pPrivateStageJoinFrame = m_IDLResource.FindWidget("PrivateStageJoinFrame");
	if (pPrivateStageJoinFrame)
	{
		pPrivateStageJoinFrame->Show(true, true);
	}
}

void ZGameInterface::LeaveBattle()
{
	ShowMenu(false);
	m_bGameFinishLeaveBattle = false;

	ZPostStageLeaveBattle(ZGetGameClient()->GetPlayerUID(), m_bGameFinishLeaveBattle);//, ZGetGameClient()->GetStageUID());
	if (m_bLeaveStageReserved) {
		ZPostStageLeave(ZGetGameClient()->GetPlayerUID());//, ZGetGameClient()->GetStageUID());
		ZApplication::GetGameInterface()->SetState(GUNZ_LOBBY);
	} else {
		ZApplication::GetGameInterface()->SetState(GUNZ_STAGE);
	}

	ZGetGameInterface()->SetCursorEnable(true);
	m_bLeaveBattleReserved = false;
	m_bLeaveStageReserved = false;
}

void ZGameInterface::ReserveLeaveStage()
{
	m_bLeaveStageReserved = true;
	ReserveLeaveBattle();
}

void ZGameInterface::ReserveLeaveStagePreGame()
{
	//ShowMenu(false);
	//m_bGameFinishLeaveBattle = false;
	ZPostStageLeave(ZGetGameClient()->GetPlayerUID());
	ZApplication::GetGameInterface()->SetState(GUNZ_LOBBY);
	//ZGetGameInterface()->SetCursorEnable(true);
	//m_bLeaveBattleReserved = false;
	//m_bLeaveStageReserved = false;
	specialCase = true;
	timeSpecialCase = timeGetTime() + 1000;
}

void ZGameInterface::PauseGame()
{
	paused = true;
	timePause = timeGetTime() + 60000*2;
}

void ZGameInterface::ResumeGame()
{
	paused = false;
	timePause = 0;
}

void ZGameInterface::ReserveLeaveBattle()
{
	bool bLeaveImmediately = false;
#ifndef _PUBLISH
	bLeaveImmediately = true;
#endif

	if(!m_pGame) return;

	// ¿ÉÀú¹öÀÏ¶§³ª..
	if (m_pCombatInterface->GetObserver()->IsVisible()) 
		bLeaveImmediately = true;

	// ¹ö±×¹ß»ýÀ¸·Î ÁÖ¼®Ã³¸®..... 20090616 by kammir
	// Á×ÀÚ¸¶ÀÚ ½ºÅ×ÀÌÁö·Î ³ª°£ÈÄ ¹Ù·Î ¹èÆ²·Î µé¾î¿Ã°æ¿ì(6.5ÃÊ¾È¿¡ ÀçÇöµÉ°æ¿ì)
	// ¼­¹ö¿¡¼­ Ä³¸¯ÅÍ¸¦ ½ºÆùÇØÁÖÁö ¾Ê´Â´Ù....±×°ÍÀ¸·Î ÀÎÇØ ¹ö±× ¹ß»ý
	// Á×¾î¼­ ½ºÆùµÇ±â¸¦ ±â´Ù¸±¶§´Â
	//ZMatch* pMatch = ZGetGame()->GetMatch();
	//int nRemainTime = pMatch->GetRemainedSpawnTime();
	//if ((nRemainTime > 0) && (nRemainTime <= 5))
	//	bLeaveImmediately = true;

	// ¹Ù·Î °ÔÀÓ¿¡¼­ ³ª°£´Ù
	if(bLeaveImmediately) 
	{
		LeaveBattle();
		return;
	}

	m_bLeaveBattleReserved = true;
	m_dwLeaveBattleTime = timeGetTime() + 5000;
}

void ZGameInterface::ShowMenu(bool bEnable)
{
	if(!GetCombatInterface()->IsShowResult())
		m_CombatMenu.ShowModal(bEnable);
	ZGetGameInterface()->SetCursorEnable(bEnable);
}

void ZGameInterface::HideMenu()
{
	m_CombatMenu.ShowModal(false);
}


bool ZGameInterface::IsMenuVisible()
{
	return m_CombatMenu.IsVisible();
}

void ZGameInterface::Show112Dialog(bool bShow)
{
/*
	MWidget* pWidget = m_IDLResource.FindWidget("112Confirm");
	if(pWidget==NULL) return;

	if (pWidget->IsVisible() == bShow) return;
	pWidget->Show(bShow);
	
	if (bShow)
	{
		SetCursorEnable(true);

		MEdit* pReasonEdit = (MEdit*)m_IDLResource.FindWidget("112_ConfirmEdit");
		if (pReasonEdit)
		{
			pReasonEdit->SetText("");
			pReasonEdit->SetFocus();
		}
	}
	else
	{
		if (GetState() == GUNZ_GAME)
		{
			if (!IsMenuVisible()) SetCursorEnable(false);
		}
	}
*/
	MWidget* pWidget = m_IDLResource.FindWidget( "112Confirm");
	if ( !pWidget)
		return;

	if ( pWidget->IsVisible() == bShow)
		return;

	pWidget->Show( bShow, true);
	pWidget->SetFocus();

	if ( !bShow)
		return;


	MComboBox* pCombo1 = (MComboBox*)m_IDLResource.FindWidget( "112_ConfirmID");
	MComboBox* pCombo2 = (MComboBox*)m_IDLResource.FindWidget( "112_ConfirmReason");

	if ( !pCombo1 || !pCombo2)
		return;

	pCombo1->RemoveAll();
	pCombo2->SetSelIndex( 0);


	switch ( m_nState)
	{
		case GUNZ_LOBBY:
		{
			ZPlayerListBox *pPlayerListBox = (ZPlayerListBox*)m_IDLResource.FindWidget( "LobbyChannelPlayerList");
			if ( pPlayerListBox)
			{
				for ( int i = 0;  i < pPlayerListBox->GetCount();  i++)
					pCombo1->Add( pPlayerListBox->GetPlayerName( i));
			}
		}
		break;

		case GUNZ_STAGE:
		{
			ZPlayerListBox *pPlayerListBox = (ZPlayerListBox*)m_IDLResource.FindWidget( "StagePlayerList_");
			if ( pPlayerListBox)
			{
				for ( int i = 0;  i < pPlayerListBox->GetCount();  i++)
					pCombo1->Add( pPlayerListBox->GetPlayerName( i));
			}
		}
		break;

		case GUNZ_GAME:
		{
			for ( ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); itor++)
				pCombo1->Add( (*itor).second->GetUserName());
		}
		break;
	}

	pCombo1->SetSelIndex( 0);
}


void ZGameInterface::RequestQuickJoin()
{
	MTD_QuickJoinParam	quick_join_param;

	quick_join_param.nMapEnum = 0xFFFFFFFF;

	// Æ®·¹ÀÌ´×, Ä®ÀüÀº »«´Ù.
	quick_join_param.nModeEnum = 0;
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_DEATHMATCH_SOLO);
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_DEATHMATCH_TEAM);
	SetBitSet(quick_join_param.nModeEnum, MMATCH_GAMETYPE_ASSASSINATE);




	ZPostRequestQuickJoin(ZGetGameClient()->GetPlayerUID(), &quick_join_param);
}

/*
// 0 = Ã¤³Î , 1 = Ä£±¸ , 2 = Å¬·£
void ZGameInterface::SetupPlayerListButton(int index)
{
	if(index<0 || index>=3) return;

	ZIDLResource *pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MWidget *pWidget;

	pWidget = pResource->FindWidget("LobbyPlayerListTabChannel");
	if(pWidget) pWidget->Show(index==0);

	pWidget = pResource->FindWidget("LobbyPlayerListTabFriend");
	if(pWidget) pWidget->Show(index==1);

	pWidget = pResource->FindWidget("LobbyPlayerListTabClan");
	if(pWidget) pWidget->Show(index==2);

	// Å¬·£ÀÎµ¥ Å¬·£¿¡ °¡ÀÔÀÌ ¾ÈµÇ¾îÀÖÀ¸¸é »ý¼º Ã¢À» º¸ÀÎ´Ù
	bool bShowCreateFrame = (index==2 && !ZGetMyInfo()->IsClanJoined());

	pWidget = m_IDLResource.FindWidget("LobbyPlayerListClanCreateFrame");
	if(pWidget) pWidget->Show(bShowCreateFrame);
	pWidget = m_IDLResource.FindWidget("LobbyChannelPlayerList");
	if(pWidget) pWidget->Show(!bShowCreateFrame);
}

void ZGameInterface::SetupPlayerListTab()
{
	ZIDLResource *pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget *pWidget;
	pWidget = pResource->FindWidget("LobbyPlayerListTabClan");
	if(!pWidget || !pWidget->IsVisible()) return;

	// Å¬·£ÀÎµ¥ Å¬·£¿¡ °¡ÀÔÀÌ ¾ÈµÇ¾îÀÖÀ¸¸é »ý¼º Ã¢À» º¸ÀÎ´Ù
	bool bShowCreateFrame = !ZGetMyInfo()->IsClanJoined();

	pWidget = m_IDLResource.FindWidget("LobbyPlayerListClanCreateFrame");
	if(pWidget) pWidget->Show(bShowCreateFrame);
	pWidget = m_IDLResource.FindWidget("LobbyChannelPlayerList");
	if(pWidget) pWidget->Show(!bShowCreateFrame);
}
*/

void ZGameInterface::InitClanLobbyUI(bool bClanBattleEnable)
{
	OnArrangedTeamGameUI(false, false);

	MWidget *pWidget;

	pWidget= m_IDLResource.FindWidget( "StageJoin" );
	if(pWidget) pWidget->Show(!bClanBattleEnable);
	pWidget= m_IDLResource.FindWidget( "StageCreateFrameCaller" );
	if(pWidget) pWidget->Show(!bClanBattleEnable);

	pWidget= m_IDLResource.FindWidget( "ArrangedTeamGame" );
	if(pWidget) pWidget->Show(bClanBattleEnable);

	pWidget= m_IDLResource.FindWidget( "ArrangedTeamGameWarmUp" );
	if(pWidget) pWidget->Show(bClanBattleEnable);

	m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, !bClanBattleEnable);
//	pWidget= m_IDLResource.FindWidget( "BattleExit" );
//	if(pWidget) pWidget->Enable(!bClanBattleEnable);

	pWidget= m_IDLResource.FindWidget( "QuickJoin" );
	if(pWidget) pWidget->Show(!bClanBattleEnable);

	pWidget= m_IDLResource.FindWidget( "QuickJoin2" );
	if(pWidget) pWidget->Show(!bClanBattleEnable);

	bool bClanServer = ZGetGameClient()->GetServerMode()==MSM_CLAN;

	pWidget= m_IDLResource.FindWidget( "PrivateChannelInput" );
	if(pWidget) pWidget->Show(bClanServer);
	
	pWidget= m_IDLResource.FindWidget( "PrivateChannelEnter" );
	if(pWidget) pWidget->Show(bClanServer);

	pWidget= m_IDLResource.FindWidget( "Lobby_ClanInfoBG" );
	if(pWidget) pWidget->Show(bClanBattleEnable);

	pWidget= m_IDLResource.FindWidget( "Lobby_ClanList" );
	if(pWidget) pWidget->Show(bClanBattleEnable);
}

void ZGameInterface::InitDuelTournamentLobbyUI(bool bEnableDuelTournamentUI)
{
	MWidget *pWidget;

	pWidget= m_IDLResource.FindWidget( "Lobby_RoomListBG" );
	if(pWidget) {
		if (!bEnableDuelTournamentUI) ((MPicture*)pWidget)->SetBitmap( m_pRoomListFrame);
		else ((MPicture*)pWidget)->SetBitmap( m_pDuelTournamentLobbyFrame);
	}

	pWidget= m_IDLResource.FindWidget( "StageBeforeBtn");
	if(pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget= m_IDLResource.FindWidget( "StageAfterBtn");
	if(pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget= m_IDLResource.FindWidget( "Lobby_RoomNo1");
	if(pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget= m_IDLResource.FindWidget( "Lobby_RoomNo2");
	if(pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget= m_IDLResource.FindWidget( "Lobby_RoomNo3");
	if(pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget= m_IDLResource.FindWidget( "Lobby_RoomNo4");
	if(pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget= m_IDLResource.FindWidget( "Lobby_RoomNo5");
	if(pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget= m_IDLResource.FindWidget( "Lobby_RoomNo6");
	if(pWidget) pWidget->Show(!bEnableDuelTournamentUI);

	pWidget= m_IDLResource.FindWidget( "StageJoin" );
	if(pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget= m_IDLResource.FindWidget( "StageCreateFrameCaller" );
	if(pWidget) pWidget->Show(!bEnableDuelTournamentUI);
	pWidget= m_IDLResource.FindWidget( "QuickJoin" );
	if(pWidget) pWidget->Show(!bEnableDuelTournamentUI);

	pWidget= m_IDLResource.FindWidget( "DuelTournamentGame" );
	if(pWidget) pWidget->Show(bEnableDuelTournamentUI);

#ifdef _PUBLISH
	pWidget= m_IDLResource.FindWidget( "DuelTournamentGame_2Test" );
	if(pWidget) pWidget->Show(false);
	pWidget= m_IDLResource.FindWidget( "DuelTournamentGame_4Test" );
	if(pWidget) pWidget->Show(false);
#else	// Å×½ºÆ®¿ë 2,4ÀÎ µà¾óÅä³Ê¸ÕÆ® Âü°¡ ¹öÆ°
	pWidget= m_IDLResource.FindWidget( "DuelTournamentGame_2Test" );
	if(pWidget) pWidget->Show(bEnableDuelTournamentUI);
	pWidget= m_IDLResource.FindWidget( "DuelTournamentGame_4Test" );
	if(pWidget) pWidget->Show(bEnableDuelTournamentUI);
#endif

	pWidget= m_IDLResource.FindWidget( "Lobby_DuelTournamentInfoBG" );
	if(pWidget) pWidget->Show(bEnableDuelTournamentUI);
	pWidget= m_IDLResource.FindWidget( "Lobby_DuelTournamentRankingList" );
	if(pWidget) pWidget->Show(bEnableDuelTournamentUI);

	if (bEnableDuelTournamentUI) {
		pWidget = m_IDLResource.FindWidget("Lobby_DuelTournamentInfoName");
		if(pWidget) pWidget->SetText(ZGetMyInfo()->GetCharName());
	}
	
	pWidget= m_IDLResource.FindWidget( "Lobby_DuelTournamentRankingListLabel" );
	if(pWidget) pWidget->Show(bEnableDuelTournamentUI);

	pWidget = m_IDLResource.FindWidget("Lobby_DuelTournamentNeedPointNextRank");
	if(pWidget) pWidget->Show(bEnableDuelTournamentUI);

	//Ãâ·ÂÅ×½ºÆ®¿ë
/*	if (bEnableDuelTournamentUI)
	{
		pWidget= m_IDLResource.FindWidget( "Lobby_DuelTournamentRankingList" );
		ZDuelTournamentRankingListBox* pRankingList = (ZDuelTournamentRankingListBox*)pWidget;

		ZDUELTOURNAMENTRANKINGITEM tempItem;
		tempItem.bEmptyItem = false;
		tempItem.nFluctuation = 1;
		tempItem.nLosses = 30;
		tempItem.nWins = 193;
		tempItem.nWinners = 42;
		tempItem.nPoint = 1832;
		tempItem.nRank = 35;
		strcpy(tempItem.szCharName, "´¿´¿ÀÌ");
		pRankingList->SetRankInfo(0, tempItem);

		tempItem.nFluctuation = -2;
		tempItem.nLosses = 25;
		tempItem.nWins = 43;
		tempItem.nWinners = 22;
		tempItem.nPoint = 1532;
		tempItem.nRank = 36;
		strcpy(tempItem.szCharName, "±èµ¿Çõ¸Ç");
		pRankingList->SetRankInfo(1, tempItem);

		tempItem.nFluctuation = 534528;
		tempItem.nLosses = 823458;
		tempItem.nWins = 823458;
		tempItem.nWinners = 820098;
		tempItem.nPoint = 8518;
		tempItem.nRank = 823528;
		strcpy(tempItem.szCharName, "Á¦ÀÏ±äÀÌ¸§ÀÓ");
		pRankingList->SetRankInfo(2, tempItem);

		tempItem.nFluctuation = -1;
		tempItem.nLosses = 34;
		tempItem.nWins = 120;
		tempItem.nWinners = 30;
		tempItem.nPoint = 1300;
		tempItem.nRank = 38;
		strcpy(tempItem.szCharName, "^1·²^2·²^3·¯");
		pRankingList->SetRankInfo(3, tempItem);

		tempItem.nFluctuation = 0;
		tempItem.nLosses = 22;
		tempItem.nWins = 110;
		tempItem.nWinners = 30;
		tempItem.nPoint = 980;
		tempItem.nRank = 39;
		strcpy(tempItem.szCharName, "iiiiiiiiiiii");
		pRankingList->SetRankInfo(4, tempItem);

		ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();

		pWidget = pRes->FindWidget("Lobby_DuelTournamentNeedPointNextRank");
		if(pWidget) { 
			if (bEnableDuelTournamentUI) {
				char szBuffer[256] = "";
				ZTransMsg(szBuffer, MSG_LOBBY_DUELTOURNAMENT_NEEDPOINT_TONEXTRANK, 1, "32");
				pWidget->SetText(szBuffer);
				((MLabel*)pWidget)->SetAlignment(MAM_RIGHT);
			}
		}		
		//
		//char szOutput[256];
		//sprintf(szOutput,"%d/%d",823458,823458);
		//ZBmNumLabel *pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoWinLose");
		//pNumLabel->SetText(szOutput);
		//sprintf(szOutput,"%d",8518);
		//pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoPoints");
		//pNumLabel->SetText(szOutput);
		//sprintf(szOutput,"%d",820098);
		//pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoWinners");
		//pNumLabel->SetText(szOutput);
		//sprintf(szOutput,"%d",823528);
		//pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoRanking");
		//pNumLabel->SetText(szOutput);
	}
*/
}

void ZGameInterface::InitChannelFrame(MCHANNEL_TYPE nChannelType)
{
	MWidget* pWidget;

	pWidget = m_IDLResource.FindWidget("PrivateChannelInput");
	if (pWidget) pWidget->Show(nChannelType == MCHANNEL_TYPE_USER);
	pWidget = m_IDLResource.FindWidget("PrivateChannelEnter");
	if (pWidget) pWidget->Show(nChannelType == MCHANNEL_TYPE_USER);
	pWidget = m_IDLResource.FindWidget("MyClanChannel");
	if (pWidget) pWidget->Show(nChannelType == MCHANNEL_TYPE_CLAN);
	pWidget = m_IDLResource.FindWidget("LockMyClanChannel");
	if (pWidget) pWidget->Show(nChannelType == MCHANNEL_TYPE_CLAN);
	pWidget = m_IDLResource.FindWidget("UnlockMyClanChannel");
	if (pWidget) pWidget->Show(nChannelType == MCHANNEL_TYPE_CLAN);

	MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget("ChannelList");
	if (pListBox) pListBox->RemoveAll();
}

void ZGameInterface::InitLadderUI(bool bLadderEnable)
{
	OnArrangedTeamGameUI(false, false);

	MWidget *pWidget;

	pWidget= m_IDLResource.FindWidget( "StageJoin" );
	if(pWidget) pWidget->Show(!bLadderEnable);
	pWidget= m_IDLResource.FindWidget( "StageCreateFrameCaller" );
	if(pWidget) pWidget->Show(!bLadderEnable);

	pWidget= m_IDLResource.FindWidget( "ArrangedTeamGame" );
	if(pWidget) pWidget->Show(bLadderEnable);

	pWidget= m_IDLResource.FindWidget( "ArrangedTeamGameWarmUp" );
	if(pWidget) pWidget->Show(bLadderEnable);

	m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, !bLadderEnable);
	//pWidget= m_IDLResource.FindWidget( "BattleExit" );
	//if(pWidget) pWidget->Enable(!bLadderEnable);

	bool bLadderServer = 
		ZGetGameClient()->GetServerMode()==MSM_CLAN ||
		ZGetGameClient()->GetServerMode()==MSM_LADDER ||
		ZGetGameClient()->GetServerMode()==MSM_EVENT;

	pWidget= m_IDLResource.FindWidget( "PrivateChannelInput" );
	if(pWidget) pWidget->Show(bLadderServer);
	
	pWidget= m_IDLResource.FindWidget( "PrivateChannelEnter" );
	if(pWidget) pWidget->Show(bLadderServer);

}

void ZGameInterface::OnArrangedTeamGameUI(bool bFinding, bool isvote)
{
	
	MWidget *pWidget;
	if (isvote == true)
	{
		FlashWindow(FindWindow(0, APPLICATION_NAME), 1);
		pWidget = m_IDLResource.FindWidget("ArrangedTeamGame");
		if (pWidget) {
			pWidget->Show(false);
		}

		pWidget= m_IDLResource.FindWidget( "ArrangedTeamGameWarmUp" );
		if(pWidget) { 
			pWidget->Show(false);
		}

		pWidget = m_IDLResource.FindWidget("LobbyFindClanTeam");
		if (pWidget != NULL){
			pWidget->Show(false);
		}
		pWidget = m_IDLResource.FindWidget("PlayerWarsMapVote");
		if (pWidget)
		{
			pWidget->Show(bFinding);
			pWidget->SetText("Clan War Map Voter");
		}
	}
	else
	{
		pWidget = m_IDLResource.FindWidget("PlayerWarsMapVote");
		if (pWidget) pWidget->Show(false);
		pWidget = m_IDLResource.FindWidget("ArrangedTeamGame");
		if (pWidget) pWidget->Show(!bFinding);

		pWidget= m_IDLResource.FindWidget( "ArrangedTeamGameWarmUp" );
		if(pWidget) pWidget->Show(!bFinding);

		pWidget = m_IDLResource.FindWidget("LobbyFindClanTeam");
		if (pWidget != NULL) pWidget->Show(bFinding);
	}


	// ´ÙÀ½ À§Á¬µéÀ» enable/disable ÇØÁØ´Ù
#define SAFE_ENABLE(x,b) { pWidget= m_IDLResource.FindWidget( x ); if(pWidget) pWidget->Enable(!b); }

	SAFE_ENABLE("LobbyChannelPlayerList", bFinding);
//	SAFE_ENABLE("LobbyPlayerListTabClanCreateButton");
	SAFE_ENABLE("ShopCaller", bFinding);
	SAFE_ENABLE("ShopCaller1", bFinding);
	SAFE_ENABLE("EquipmentCaller", bFinding);
	SAFE_ENABLE("EquipmentCaller2", bFinding);
	SAFE_ENABLE("EquipmentCaller4", bFinding);
	SAFE_ENABLE("ChannelListFrameCaller", bFinding);
	SAFE_ENABLE("LobbyOptionFrame", bFinding);
	SAFE_ENABLE("LobbyOptionFrame2", bFinding);
	SAFE_ENABLE("Logout", bFinding);
	SAFE_ENABLE("ReplayCaller", bFinding);
	SAFE_ENABLE("CharSelectionCaller", bFinding);
	SAFE_ENABLE("QuickJoin", bFinding);
	SAFE_ENABLE("QuickJoin2", bFinding);

	m_bWaitingArrangedGame = bFinding;
}

// µà¾óÅä³Ê¸ÕÆ® ½ÅÃ»ÇßÀ» ¶§ È­¸é»óÀÇ ¹öÆ°µéÀ» ºñÈ°¼ºÈ­ÇÑ´Ù( È¤Àº ±×¹Ý´ë)
void ZGameInterface::OnDuelTournamentGameUI(bool bWaiting)
{
	MWidget *pWidget;

	pWidget = m_IDLResource.FindWidget("DuelTournamentWaitMatchDialog");
	if(pWidget) pWidget->Show(bWaiting);

	// ´ÙÀ½ À§Á¬µéÀ» enable/disable ÇØÁØ´Ù
	SAFE_ENABLE("LobbyChannelPlayerList", bWaiting);
	SAFE_ENABLE("ShopCaller", bWaiting);
	SAFE_ENABLE("ShopCaller1", bWaiting);
	SAFE_ENABLE("EquipmentCaller", bWaiting);
	SAFE_ENABLE("EquipmentCaller2", bWaiting);
	SAFE_ENABLE("EquipmentCaller4", bWaiting);
	SAFE_ENABLE("ChannelListFrameCaller", bWaiting);
	SAFE_ENABLE("LobbyOptionFrame", bWaiting);
	SAFE_ENABLE("LobbyOptionFrame2", bWaiting);
	SAFE_ENABLE("Logout", bWaiting);
	SAFE_ENABLE("ReplayCaller", bWaiting);
	SAFE_ENABLE("CharSelectionCaller", bWaiting);
	SAFE_ENABLE("DuelTournamentGame", bWaiting);
	SAFE_ENABLE("DuelTournamentGame_2Test", bWaiting);
	SAFE_ENABLE("DuelTournamentGame_4Test", bWaiting);
}

bool ZGameInterface::IsReadyToPropose()
{
	if(GetState() != GUNZ_LOBBY)
		return false;

	if(m_bWaitingArrangedGame)
		return false;

	if(GetLatestExclusive()!=NULL)
		return false;

	if(m_pMsgBox->IsVisible())
		return false;

	// TODO : ¾Õ¿¡ modal Ã¢ÀÌ ¶°ÀÖÀ¸¸é return false ÇÏÀÚ..
	return true;
}

bool ZGameInterface::IsMiniMapEnable()
{
	return GetCamera()->GetLookMode()==ZCAMERA_MINIMAP;
}

bool ZGameInterface::OpenMiniMap()
{
	if(!m_pMiniMap) {
		m_pMiniMap = new ZMiniMap;
		if(!m_pMiniMap->Create(ZGetGameClient()->GetMatchStageSetting()->GetMapName()))
		{
			SAFE_DELETE(m_pMiniMap);
			return false;
		}
	}

	return true;
}









// ¸®ÇÃ·¹ÀÌ °ü·Ã ÇÔ¼öµé(µ¿È¯ÀÌ°¡ Ãß°¡)

/***********************************************************************
  ShowReplayDialog : public
  
  desc : ¸®ÇÃ·¹ÀÌ È­¸é º¸ÀÌ±â
  arg  : true(=show) or false(=hide)
  ret  : none
************************************************************************/
class ReplayListBoxItem : public MListItem
{
protected:
	char		m_szName[ _MAX_PATH];
	char		m_szVersion[ 10];

public:
	ReplayListBoxItem( const char* szName, const char* szVersion)
	{
		strcpy(m_szName, szName);
		strcpy(m_szVersion, szVersion);
	}

	virtual const char* GetString( void)
	{
		return m_szName;
	}
	virtual const char* GetString( int i)
	{
		if ( i == 0)
			return m_szName;
		else if ( i == 1)
			return m_szVersion;

		return NULL;
	}
	virtual MBitmap* GetBitmap( int i)
	{
		return NULL;
	}
};

void ZGameInterface::LoadCustomImages()
{
	char szFold[MAX_PATH];
	TCHAR szTempFolder[MAX_PATH];
	WIN32_FIND_DATA w32Data;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	int nFileCounter = 0;
	char nTempSkipToTwo = 0;

	memset(m_szScreenShots, 0, sizeof(m_szScreenShots));
	if (GetMyDocumentsPath(szFold))
	{
		strcat(szFold, GUNZ_FOLDER);
		if (GetFileAttributes(szFold) == INVALID_FILE_ATTRIBUTES)
			CreatePath(szFold);
		strcat(szFold, SCREENSHOT_FOLDER);
		if (GetFileAttributes(szFold) == INVALID_FILE_ATTRIBUTES)
			CreatePath(szFold);
		strcpy(m_szScreenShotPath, szFold);
	}
	strcpy(szTempFolder, m_szScreenShotPath);
	strcat(szTempFolder, "\\*");
	hFile = FindFirstFile(szTempFolder, &w32Data);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		mlog("[OpenCustomMusic] Failed due to finding first file - %d\n", GetLastError());
	}
	do
	{
		if (nTempSkipToTwo >= 2 && hFile != INVALID_HANDLE_VALUE)
		{
			strcpy(m_szScreenShots[nFileCounter], w32Data.cFileName);
			++nFileCounter;
		}
		++nTempSkipToTwo;
	} while (FindNextFile(hFile, &w32Data) != false);
	m_szScreenShotsSize = nFileCounter;
}
void ZGameInterface::ShowScreenShotDialog(bool bShow)
{
	if (bShow)
	{
		MWidget* pWidget;
		pWidget = (MWidget*)m_IDLResource.FindWidget("ScreenShot");
		if (pWidget)
			pWidget->Show(true, true);
		pWidget->SetVisible(true);
		MButton* pButton = (MButton*)m_IDLResource.FindWidget("PrevScreenShotPicture");
		pButton->SetText("<");
		LoadCustomImages();
		MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget("ScreenShot_List");
		pListBox->RemoveAll();
		if (pListBox && m_szScreenShotsSize > 0)
		{
			pListBox->RemoveAll();
			for (int j = 0; j< m_szScreenShotsSize; j++)
				pListBox->Add((char*)m_szScreenShots[j]);
		}
	}
	else
		ShowWidget("ScreenShot", false);
}
void ZGameInterface::ViewShowScreenShotDialog(bool bShow)
{
	if (bShow && m_szScreenShotsSize > 0)
	{
		MFrame* pFrame = (MFrame*)m_IDLResource.FindWidget("ScreenShotPictureFrame");
		MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget("ScreenShot_List");
		MPicture* pPicture = (MPicture*)m_IDLResource.FindWidget("ScreenShotPicture");
		pFrame->Show(true, true);
		MBitmapR2* m_pScreenShotView = new MBitmapR2;
		TCHAR szPath[MAX_PATH];
		bool bRead = true;
		if (GetMyDocumentsPath(szPath))
		{
			strcat(szPath, GUNZ_FOLDER);
			strcat(szPath, "/Screenshots/");
			CreatePath(szPath);
		}
		strcat(szPath, pListBox->GetSelItemString());
		pFrame->SetText(pListBox->GetSelItemString());

		MZFile::SetReadMode(MZIPREADFLAG_ZIP | MZIPREADFLAG_MRS | MZIPREADFLAG_MRS2 | MZIPREADFLAG_FILE);
		bRead = m_pScreenShotView->Create(pListBox->GetSelItemString(), RGetDevice(), szPath, false);
		MZFile::SetReadMode(MZIPREADFLAG_MRS2);

		pPicture->SetBitmap(m_pScreenShotView->GetSourceBitmap());
	}
	else
		ShowWidget("ScreenShot", false);
}

void ZGameInterface::ShowReplayDialog( bool bShow)
{
	if ( bShow)			// º¸ÀÌ±âÀÌ¸é...
	{
		// ¸®ÇÃ·¹ÀÌ È­¸é º¸ÀÌ±â
		MWidget* pWidget;
		pWidget = (MWidget*)m_IDLResource.FindWidget( "Replay");
		if ( pWidget)
			pWidget->Show( true, true);

		// 'º¸±â'¹öÆ° ºñÈ°¼ºÈ­
		pWidget = (MWidget*)m_IDLResource.FindWidget( "Replay_View");
		if ( pWidget)
			pWidget->Enable( false);


		// ÆÄÀÏ ¸®½ºÆ® ÃÊ±âÈ­
		MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget( "Replay_FileList");
		if ( pListBox)
		{
			pListBox->RemoveAll();

			// Get path name
			TCHAR szPath[ MAX_PATH];
			if ( GetMyDocumentsPath( szPath))		// ³»¹®¼­ Æú´õ °æ·Î
			{
				strcat( szPath, GUNZ_FOLDER);		// Gunz Æú´õ °æ·Î
				strcat( szPath, REPLAY_FOLDER);		// Replay Æú´õ °æ·Î
				CreatePath( szPath );
			}
			TCHAR szFullPath[ MAX_PATH];
			strcpy( szFullPath, szPath);

			strcat( szPath, "/*.gzr");			// È®Àå¸í ÇÊÅÍ

			// Get file list
			struct _finddata_t c_file;
			long hFile;
			char szName[ _MAX_PATH];
			char szFullPathName[ _MAX_PATH];
			if ( (hFile = _findfirst( szPath, &c_file)) != -1L)
			{
				do
				{
					strcpy( szName, c_file.name);

					strcpy( szFullPathName, szFullPath);
					strcat( szFullPathName, "/");
					strcat( szFullPathName, szName);

					DWORD dwFileVersion = 0;
					char szVersion[10];
					int nRead;
					DWORD header;
					ZFile *file = zfopen( szFullPathName);
					if ( file)
					{
						nRead = zfread( &header, sizeof( header), 1, file);
						if( (nRead != 0) && (header == GUNZ_REC_FILE_ID))
						{
							zfread( &dwFileVersion, sizeof( dwFileVersion), 1, file);
							sprintf( szVersion, "v%d.0", dwFileVersion);
						}
						else
							strcpy( szVersion, "--");

						zfclose( file);
					}
					else
						strcpy( szVersion, "--");
				
					pListBox->Add( new ReplayListBoxItem( szName, szVersion));			// Add to listbox
				} while ( _findnext( hFile, &c_file) == 0);

				_findclose( hFile);
			}

			pListBox->Sort();		// Sorting
		}
	}
	else				// °¨Ãß±âÀÌ¸é...
	{
		// ¸®ÇÃ·¹ÀÌ È­¸é °¨Ãß±â
		ShowWidget( "Replay", false);
	}
}

/***********************************************************************
  ViewReplay : public
  
  desc : ¸®ÇÃ·¹ÀÌ ½ÇÇà
  arg  : none
  ret  : none
************************************************************************/
void ZGameInterface::ViewReplay( void)
{
	// ¸®ÇÃ·¹ÀÌ ´ÙÀÌ¾ó·Î±× ´Ý±â
	ShowReplayDialog( false);


	// Æú´õ °æ·Î-È­ÀÏ¸í ±¸ÇÔ
	MListBox* pListBox = (MListBox*)m_IDLResource.FindWidget( "Replay_FileList");
	if ( !pListBox)
		return ;

	if ( pListBox->GetSelItemString() == NULL)
		return ;

	TCHAR szName[ MAX_PATH];
	if ( GetMyDocumentsPath( szName))				// ³»¹®¼­ Æú´õ °æ·Î
	{
		strcat( szName, GUNZ_FOLDER);				// Gunz Æú´õ °æ·Î
		strcat( szName, REPLAY_FOLDER);				// Replay Æú´õ °æ·Î
		strcat( szName, "/");
		strcat( szName, pListBox->GetSelItemString());
	}


	m_bOnEndOfReplay = true;
	m_nLevelPercentCache = ZGetMyInfo()->GetLevelPercent();

	// ¸®ÇÃ·¹ÀÌ ½ÇÇà
	if ( !CreateReplayGame( szName))
	{
		ZApplication::GetGameInterface()->ShowMessage( "Can't Open Replay File" );
	}

	// ¿É¼Ç ¸Þ´ºÀÇ ÀÏºÎ ¹öÆ° ºñÈ°¼ºÈ­
	m_CombatMenu.EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, false);
}



////////////////////////////////////////////////////////////////
#ifdef _QUEST_ITEM
void ZGameInterface::OnResponseCharacterItemList_QuestItem( MTD_QuestItemNode* pQuestItemNode, int nQuestItemCount )
{
	if( 0 == pQuestItemNode)
		return;

	ZGetMyInfo()->GetItemList()->SetQuestItemsAll( pQuestItemNode, nQuestItemCount );

	ZApplication::GetStageInterface()->SerializeSacrificeItemListBox();	
	ZGetMyInfo()->GetItemList()->Serialize();

	// »óÁ¡¿¡¼­ Ä³¸¯ÅÍ UI Text Á¤º¸¸¦ ¾÷µ¥ÀÌÆ® ÇØÁØ´Ù (·¹º§, ¹Ù¿îÆ¼, Ã¼·Â, ¹æ¾î, ¹«°Ô)
	GetShopEquipInterface()->DrawCharInfoText();
}

void ZGameInterface::OnResponseBuyQuestItem( const int nResult, const int nBP )
{
	if( MOK == nResult )
	{
		// ÀÓ½Ã
		ZApplication::GetGameInterface()->ShowMessage( MSG_GAME_BUYITEM );

		ZGetMyInfo()->SetBP( nBP );
	}
	else if( MERR_TOO_MANY_ITEM == nResult )
	{
		// ÀÓ½Ã.
		ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
	}
	else if( MERR_TOO_EXPENSIVE_BOUNTY == nResult )
	{
		// ÀÓ½Ã.
		ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
	}
	else
	{
		// Á¤ÀÇµÇ¾îÀÖÁö ¾ÊÀ½.
		mlog( "ZGameInterface::OnCommand::MC_MATCH_RESPONSE_BUY_QUEST_ITEM - Á¤ÀÇµÇÁö ¾ÊÀº °á°úÃ³¸®.\n" );
		ASSERT( 0 );				
	}

	// »óÁ¡¿¡¼­ Ä³¸¯ÅÍ UI Text Á¤º¸¸¦ ¾÷µ¥ÀÌÆ® ÇØÁØ´Ù (·¹º§, ¹Ù¿îÆ¼, Ã¼·Â, ¹æ¾î, ¹«°Ô)
	GetShopEquipInterface()->DrawCharInfoText();
}

void ZGameInterface::OnResponseSellQuestItem( const int nResult, const int nBP )
{
	if( MOK == nResult )
	{
		// ÀÓ½Ã.
		ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_SELLITEM);

		ZGetMyInfo()->SetBP( nBP );
	}
	else
	{
	}

	// »óÁ¡¿¡¼­ Ä³¸¯ÅÍ UI Text Á¤º¸¸¦ ¾÷µ¥ÀÌÆ® ÇØÁØ´Ù (·¹º§, ¹Ù¿îÆ¼, Ã¼·Â, ¹æ¾î, ¹«°Ô)
	GetShopEquipInterface()->DrawCharInfoText();
}
#endif


// Äù½ºÆ®¿ë ¾ÆÀÌÅÛ ¾ÆÀÌÄÜ È­ÀÏ¸í(ÇÏµå ÄÚµù µ¢¾î¸®~  -_-;)
// ³õÀ»µ¥°¡ ¾ø¾î¼­ ÀÏ´Ü ¿©±â´Ù...  -_-;
MBitmap* ZGameInterface::GetQuestItemIcon( int nItemID, bool bSmallIcon)
{
	char szFileName[ 64] = "";
	switch ( nItemID)
	{
		// Page
		case 200001 :		// Âõ°ÜÁø 13ÆäÀÌÁö
		case 200002 :		// Âõ°ÜÁø 25ÆäÀÌÁö
		case 200003 :		// Âõ°ÜÁø 41ÆäÀÌÁö
		case 200004 :		// Âõ°ÜÁø 65ÆäÀÌÁö
			strcpy( szFileName, "slot_icon_page");
			break;

		// Skull
		case 200005 :		// ÀÛÀºµÎ°ñ
		case 200006 :		// Å«µÎ°ñ
		case 200007 :		// ¼ö¼ö²²±âÀÇ µÎ°ñ
		case 200008 :		// °íºí¸°ÀÇ µÎ°ñ
		case 200009 :		// °íºí¸° Å·ÀÇ µÎ°ñ
		case 200010 :		// °Å´ëÇÑ À¯°ñ
			strcpy( szFileName, "slot_icon_skull");
			break;

		// Fresh
		case 200011 :		// ³¯°í±â
		case 200012 :		// ºÒ°í±â
		case 200013 :		// ½ºÅ×ÀÌÅ©
			strcpy( szFileName, "slot_icon_fresh");
			break;

		// Ring
		case 200014 :		// Ã¶±Í°ÉÀÌ
		case 200015 :		// Àº±Í°ÉÀÌ
		case 200016 :		// ±Ý±Í°ÉÀÌ
		case 200017 :		// ÇÃ·¡Æ¼³Ñ ±Í°ÉÀÌ
			strcpy( szFileName, "slot_icon_ring");
			break;

		// Necklace
		case 200018 :		// Å©¸²½¼ÀÇ ¸ñ°ÉÀÌ
			strcpy( szFileName, "slot_icon_neck");
			break;

		// Doll
		case 200019 :		// ½ºÄÌ·¹Åæ ÀÎÇü
		case 200020 :		// ÄÚº¼µå ÀÎÇü
		case 200021 :		// °íºí¸° ÀÎÇü
		case 200022 :		// ÆÊÆ÷¿ì ÀÎÇü
		case 200023 :		// Åä³¢ÀÎÇü
		case 200024 :		// °õÀÎÇü
		case 200025 :		// ÀúÁÖ ¹ÞÀº °õÀÎÇü
		case 200026 :		// ÆÊÆ÷¿ì ¾ÆÀÌµ¹
		case 200027 :		// ÀúÁÖ ¹ÞÀº ÆÊÆ÷¿ì ¾ÆÀÌµ¹
			strcpy( szFileName, "slot_icon_doll");
			break;

		// Book
		case 200028 :		// ¾Ç¸¶ÀÇ »çÀü
		case 200029 :		// ½ºÅ©¶óÀÌ´õÀÇ ¸íºÎ »óÆí
		case 200030 :		// ½ºÅ©¶óÀÌ´õÀÇ ¸íºÎ ÇÏÆí
			strcpy( szFileName, "slot_icon_book");
			break;

		// Object
		case 200031 :		// Ãàº¹¹ÞÀº ½ÊÀÚ°¡
		case 200032 :		// ÀúÁÖ¹ÞÀº ½ÊÀÚ°¡
		case 200033 :		// °î±ªÀÌ
		case 200034 :		// ¸»ÇÏ´Â µ¹¸ÍÀÌ
		case 200035 :		// ¾óÀ½ÀÇ °áÁ¤
		case 200036 :		// ºÎ¼­Áø ±â¾î
		case 200037 :		// °í±Þ ±â¾î
			strcpy( szFileName, "slot_icon_object");
			break;

		// Sword
		case 200038 :		// °íºí¸° Å·ÀÇ µµ³¢
		case 200039 :		// ÄÚº¼µå ¿ÕÀÇ µµ³¢
		case 200040 :		// °Å´ë ½ºÄÌ·¹ÅæÀÇ Ä®
		case 200041 :		// ÀúÁÖ¹ÞÀº ½Ã½ÅÀÇ Ä®
		case 200042 :		// ¸®Ä¡ÀÇ ²¿¸®
		case 200043 :		// ÆÊÆ÷¿ìÀÇ ¾óÀ½ Ä®
		case 200044 :		// ½ºÅ©¶óÀÌ´õÀÇ Ä®
			strcpy( szFileName, "slot_icon_qsword");
			break;
	}

//	if ( bSmallIcon)
//		strcat( szFileName, "_s");

	strcat(szFileName, ".tga");


	return MBitmapManager::Get( szFileName);
}


void ZGameInterface::OnResponseServerStatusInfoList( const int nListCount, void* pBlob )
{
	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget( "SelectedServer");
	if ( !pServerList)
		return;

	int nCurrSel = pServerList->GetCurrSel2();
    pServerList->ClearServerList();


#ifdef	_DEBUG
//		pServerList->AddServer( (char*)ZMsg(MSG_SERVER_DEBUG), "", 0, 7, 0, 1000, true);			// Debug server
//		pServerList->AddServer( "", "", 0, 0, 0, 1000, false);						// Null
#else
		if ( ZIsLaunchDevelop())
		{
//			pServerList->AddServer( (char*)ZMsg(MSG_SERVER_DEBUG), "", 0, 7, 0, 1000, true);			// Debug server
//			pServerList->AddServer( "", "", 0, 0, 0, 1000, false);						// Null
		}
#endif



	if( (0 < nListCount) && (0 != pBlob) )
	{
		/*
		 * Àü¼ÛµÇ´Â ServerList´Â ÇöÁ¦ È°¼ºÈ­µÇ¾îÀÖ´Â DBÀÇ Á¤º¸¸¸ º¸³»ÁÜ.
		 *  Àü¼ÛµÇÁö ¾ÊÀº ServerID´Â Server°¡ ÇöÁ¦ µ¿ÀÛÁßÀÌÁö ¾Ê´Â°ÍÀÓ.
		 */

		for( int i = 0; i < nListCount; ++i )
		{
			MTD_ServerStatusInfo* pss = (MTD_ServerStatusInfo*)MGetBlobArrayElement( pBlob, i );
			if( 0 == pss )
			{
				mlog( "ZGameInterface::OnResponseServerStatusInfoList - %d¹øÂ°¿¡¼­ NULLÆ÷ÀÎÅÍ ¹ß»ý.", i );
				continue;
			}

			in_addr inaddr;
			inaddr.S_un.S_addr = pss->m_dwIP;
			
			char* pszAddr = inet_ntoa( inaddr );
			char strAddrBuf[32];
			strcpy(strAddrBuf, pszAddr);

			// pss->m_nCurPlayer;	// ÇöÁ¦ Á¢¼ÓÀ¯Àú ¼ö.
			// pss->m_nMaxPlayer;	// ¼­¹ö ÃÖ´ë Á¢¼Ó °¡´É ¼ö.
			// pss->m_nPort;		// ¼­¹ö Port.
			// pss->m_nServerID;	// ¼­¹ö ID.
			// pss->m_nType;		// ¼­¹ö Å¸ÀÔ.
			// pss->m_bIsLive;		// ÇöÁ¦ ¼­¹ö°¡ µ¿ÀÛÇÏ°í ÀÖ´Â°¡.

			char szServName[ 128 ] = {0,};
//				if( pss->m_bIsLive )
//					_snprintf( szServName, 127, "server%d", pss->m_nServerID );
//				else
//					_snprintf( szServName, 127, "server%d(dead)", pss->m_nServerID );

			if ( pss->m_nType == 1)					// Normal server (¿ø·¡ "type==1"ÀÌ Debug server¿´À¸³ª ³Ý¸¶ºí¿¡¼­ ÀÏ¹Ý¼­¹ö·Î »ç¿ëÇÏ°í ÀÖ¾î¼­ º¯°æ)
				sprintf( szServName, "%s %d", ZMsg(MSG_SERVER_NORMAL), pss->m_nServerID );
			else if ( pss->m_nType == 2)			// Match server
				sprintf( szServName, "%s %d", ZMsg(MSG_SERVER_MATCH), pss->m_nServerID );
			else if ( pss->m_nType == 3)			// Clan server
				sprintf( szServName, "%s %d", ZMsg(MSG_SERVER_CLAN), pss->m_nServerID );
			else if ( pss->m_nType == 4)			// Quest server
				sprintf( szServName, "%s %d", ZMsg(MSG_SERVER_QUEST), pss->m_nServerID );
			else if ( pss->m_nType == 5)			// Event server
				sprintf( szServName, "%s %d", ZMsg(MSG_SERVER_EVENT), pss->m_nServerID );
			else if ( pss->m_nType == 6)			// Test server
				sprintf( szServName, "Test Server %d", pss->m_nServerID );
			else if ( pss->m_nType == 7)			// Debug server
				sprintf( szServName, "%s %d", ZMsg(MSG_SERVER_DEBUG), pss->m_nServerID );
			else 
				continue;


			int nRow = 0;
			int nCol = 0;
			char* pszAgentIP = "158.69.126.40";

			// ¼­¹ö ÀÌ¸§À» ¹Þ¾Æ¼­ Á÷Á¢ Ç¥½ÃÇØÁÜ
			sprintf( szServName, "%s", pss->m_szServerName);

			if ( (pss->m_nServerID % 50) >= 25)
				nRow = 1;

#ifdef LOCALE_NHNUSA
			//	¹Ì±¹ ¼­¹ö ¸®½ºÆ® ¹è¿­
			//	Å¬·£¼­¹ö  (Type 2) ¼­¹öID: ¼­ºÎÅ¬·£50~74,		µ¿ºÎÅ¬·£75~99		(Å¬·£¼­¹ö´Â 50~99 »çÀÌ)
			//	¸ÅÄ¡¼­¹ö  (Type 3) ¼­¹öID: ¼­ºÎ¸ÅÄ¡100~124,		µ¿ºÎ¸ÅÄ¡125~149		(¸ÅÄ¡¼­¹ö´Â 100~149 »çÀÌ)
			//	Äù½ºÆ®¼­¹ö(Type 4) ¼­¹öID: ¼­ºÎÄù½ºÆ®150~174,	µ¿ºÎÄù½ºÆ®175~199	(Äù½ºÆ®¼­¹ö´Â 150~200 »çÀÌ)
			//	Å¬·£¼­¹ö  (Type 2) ¼­¹öID: XXXXÅ¬·£250~74,		À¯·´ºÎÅ¬·£275~299		(Å¬·£¼­¹ö´Â 250~299 »çÀÌ)
			//	¸ÅÄ¡¼­¹ö  (Type 3) ¼­¹öID: XXXX¸ÅÄ¡300~324,		À¯·´¸ÅÄ¡325~349		(¸ÅÄ¡¼­¹ö´Â 300~349 »çÀÌ)
			//	Äù½ºÆ®¼­¹ö(Type 4) ¼­¹öID: XXXXºÎÄù½ºÆ®350~374,	À¯·´Äù½ºÆ®375~399	(Äù½ºÆ®¼­¹ö´Â 350~400 »çÀÌ)
			//	ÀÌ¼­¹öµéÀ» ÁÂ¿ì·Î ³ª´²ÁÖ°í½ÍÀ¸¸é  25±âÁØÀ¸·Î  25º¸´Ù ÀÛÀ¸¸é ¿ÞÂÊ, Å©¸é ¿À¸¥ÂÊ
			//	ÀÌ¼­¹öµéÀ» »óÇÏ·Î ³ª´²ÁÖ°í½ÍÀ¸¸é 200±âÁØÀ¸·Î 200º¸´Ù ÀÛÀ¸¸é À§ÂÊ, Å©¸é ¾Æ·¡ÂÊ

			// À¯·´¼­¹ö¸¦ ÇÑÄ­ ¹ØÀ¸·Î ÇØÁØ´Ù. 
			// ³ªÁß¿¡ 4°³·Î ³ª´©°Ô µÈ´Ù¸é ¿©±â¼­ Ã³¸®ÇØÁÖ¸éµÈ´Ù.
			//		-------------------------
			//		|	¼­ºÎ	|	µ¿ºÎ	|
			//		|	nRow=0	|	nRow=1	|
			//		|	nCol=0 	|	nCol=0	|
			//		-------------------------
			//		|	XXXX	|	À¯·´	|
			//		|	nRow=0	|	nRow=1	|
			//		|	nCol=1 	|	nCol=1	|
			//		-------------------------
 			if ( pss->m_nServerID > 200)
			{ // 200 ÀÌ»óºÎÅÍ À¯·´ ¼­¹ö¿Í, XXXX¼­¹öÀÌ´Ù.
				nCol = 1;
			}

			// ¼­¹ö Ãß°¡
			sockaddr_in Addr;
			Addr.sin_addr.S_un.S_addr = pss->m_dwAgentIP;
			pszAgentIP = inet_ntoa(Addr.sin_addr);

#else if defined(LOCALE_KOREA) || defined(_DEBUG)
			//		-------------------------
			//		|			|	Å¬·£Àü	|
			//		|	ÀÏ¹Ý¼­¹ö|	nRow=1	|
			//		|	nRow=0 	|	nCol=0	|
			//		|	nCol=0	|------------
			//		|			|	Äù½ºÆ®	|
			//		------------|	nRow=1	|
			//		|			|	nCol=1	|
			//		|	·¹º§¼­¹ö|------------
			//		|	nRow=0	|	ÀÚÀ¯¼­¹ö|
			//		|	nCol=1 	|	nRow=1	|
			//		|			|	nCol=2	|
			//		-------------------------
			// º¸±âÆíÇÏ°Ô ÇÏµåÄÚµù~~
			// ÀÏ¹Ý¼­¹ö 1~50, ·¹º§¼­¹ö 51~100, Å¬·£Àü 101~150, Äù½ºÆ®¼­¹ö 151~200, ÀÚÀ¯¼­¹ö 201~250
				 if (							pss->m_nServerID <  51)	{	nCol = 0; nRow = 0;	}
			else if (  50 < pss->m_nServerID && pss->m_nServerID < 101)	{	nCol = 1; nRow = 0;	}
			else if ( 100 < pss->m_nServerID && pss->m_nServerID < 151)	{	nCol = 0; nRow = 1;	}
			else if ( 150 < pss->m_nServerID && pss->m_nServerID < 201)	{	nCol = 1; nRow = 1;	}
			else if ( 200 < pss->m_nServerID && pss->m_nServerID < 251)	{	nCol = 2; nRow = 1;	}
#endif
			pServerList->AddServer( szServName, strAddrBuf, pss->m_nPort, pss->m_nType, pss->m_nCurPlayer, pss->m_nMaxPlayer, pss->m_bIsLive, pszAgentIP, nRow, nCol);


#ifdef _DEBUG
			// ¼­¹ö ·Î±× ³²±è
			mlog( "ServerList - ID:%d, Name:%s, IP:%s, Port:%d, Type:%d, (%d/%d)\n",
				pss->m_nServerID, szServName, strAddrBuf, pss->m_nPort, pss->m_nType, pss->m_nCurPlayer, pss->m_nMaxPlayer, "" );
#endif
		}
	}

#ifdef LOCALE_NHNUSA
	char szAuthString[ NHN_AUTH_LENGTH ]	= {0,};
	const int nStrRet = GetAuthString( szAuthString, NHN_AUTH_LENGTH );
	if( 0 != nStrRet )
	{
		WriteNHNAuthLog( nStrRet, "GetAuthString" );
	}
	else
	{
		((ZNHN_USAAuthInfo*)ZGetLocale()->GetAuthInfo())->SetAuthStr( szAuthString, strlen(szAuthString) );
	}
#endif


	pServerList->SetCurrSel( 0);

	m_dwRefreshTime = timeGetTime() + 10000;


#ifdef LOCALE_NHNUSA
	GetNHNUSAReport().ReportDisplayServerList();
#endif
}


void ZGameInterface::OnResponseBlockCountryCodeIP( const char* pszBlockCountryCode, const char* pszRoutingURL )
{
	if( 0 != pszBlockCountryCode )
	{
		// ºí·°µÈ IPÀÇ ±¹°¡¸¦ Å¬¶óÀÌ¾ðÆ®¿¡ Åëº¸ÇØÁà¾ß ÇÔ.
		// Message¿¡ Ãâ·Â ¹®ÀåÀ» Ãß°¡ÇØ¾ß ÇÔ.

		ShowMessage( pszRoutingURL );
	}
}


void ZGameInterface::RequestServerStatusListInfo()
{
/*
#ifdef _DEBUG
	// Locator¸®½ºÆ® ±¸¼º. ÀÌ°Ç xml·ÎºÎÅÍ °¡Á®¿È.
	// ¼­¹ö Á¤º¸ ¿äÃ».

	if( 0 == m_pLocatorList ) 
		return;

	const int nSize = m_pLocatorList->GetSize();
	for( int i = 0; i < nSize; ++i )
	{
		const string strIP = m_pLocatorList->GetIPByPos( i );

		MCommand* pCmd = ZNewCmd( MC_REQUEST_SERVER_LIST_INFO );
		if( 0 != pCmd )
		{
 			for( int i = 0; i < 3; ++i )
			{
				GetGameClient()->SendCommandByUDP( pCmd, const_cast<char*>(strIP.c_str()), LOCATOR_PORT );
				Sleep( 1000 );
			}
		}
		delete pCmd;
	}
#endif
*/
	ZLocatorList*	pLocatorList;

	if ( ZApplication::GetInstance()->IsLaunchTest())				// Å×½ºÆ® ·±ÃÄÀÏ °æ¿ì
		pLocatorList = m_pTLocatorList;
	else
		pLocatorList = m_pLocatorList;


	if( 0 == pLocatorList ) 
		return;

	if ( pLocatorList->GetSize() < 1)
		return;

	MCommand* pCmd = ZNewCmd( MC_REQUEST_SERVER_LIST_INFO );
	if( 0 != pCmd )
	{
		const string strIP = pLocatorList->GetIPByPos( m_nLocServ++);
		m_nLocServ %= pLocatorList->GetSize();

		GetGameClient()->SendCommandByUDP( pCmd, const_cast<char*>(strIP.c_str()), LOCATOR_PORT);
		delete pCmd;
	}

	m_dwRefreshTime = timeGetTime() + 1500;
}


/*
bool ZGameInterface::InitLocatorList( MZFileSystem* pFileSystem, const char* pszLocatorList )
{
	if( (0 == pszLocatorList) || (0 == strlen(pszLocatorList)) ) 
		return false;

	if( 0 == m_pLocatorList )
	{
		m_pLocatorList = new ZLocatorList;
		if( 0 == m_pLocatorList )
			return false;
	}
	else
		m_pLocatorList->Clear();
	
	return m_pLocatorList->Init( pFileSystem, pszLocatorList );

	return false;
}
*/

void ZGameInterface::OnRequestXTrapSeedKey(unsigned char *pComBuf)		// Update sgk 0702
{
#ifdef _XTRAP
	unsigned char szSendBuf[128] = {0,};
	int nRetValue = XTrap_CS_Step2(pComBuf, szSendBuf, XTRAP_PROTECT_PE | XTRAP_PROTECT_TEXT | XTRAP_PROTECT_EXCEPT_VIRUS);
    ZPostResponseXTrapSeedKey(szSendBuf);
	if (nRetValue != 0)
	{
		char szMsgBuf[500] = {
#ifdef LOCALE_KOREA
		"ºñÁ¤»óÀûÀÎ ÇàÀ§°¡ °¨ÁöµÇ¾ú½À´Ï´Ù. °ÔÀÓÀ» Á¾·áÇÕ´Ï´Ù.\n"
#else
		"An abnormal behavior is detected. Terminating game.\n"
#endif
		};
		if (ZGetGameClient())
		{
			ZGetGameClient()->Disconnect();
			AfxMessageBox(szMsgBuf);
			mlog("Disconnect() called!\n");
		}
		else
		{
			OnDisconnectMsg(MERR_FIND_HACKER);
			mlog("OnDisconnectMsg() called!\n");
		}
		mlog(szMsgBuf);
        PostQuitMessage(0);
	}
#endif
}

void ZGameInterface::OnDisconnectMsg( const DWORD dwMsgID )
{
	MListBox* pListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "QuestResult_ItemListbox");
	if( 0 != pListBox )
	{
		// ÇØÅ· °ü·Ã ¸Þ½ÃÁöÀÏ °æ¿ì¿£ Á¾·á ¿¹¾àÀ» ÇÑ´Ù.
		if ( (dwMsgID == MERR_FIND_HACKER) || (dwMsgID == MERR_BLOCK_HACKER) 
			|| (dwMsgID == MERR_BLOCK_BADUSER) || (dwMsgID == MERR_FIND_INVALIDFILE) 
			|| (dwMsgID == MERR_FIND_FLOODING) || (dwMsgID == MERR_BLOCK_BY_ADMIN)
			|| (dwMsgID == MERR_BLOCK_SLEEP_ACCOUNT) )
		{
			m_bReservedQuit = true;
			m_dwReservedQuitTimer = timeGetTime() + 5000;
		}

		ShowMessage( ZErrStr(dwMsgID) );
	}
}

void ZGameInterface::ShowDisconnectMsg( DWORD errStrID, DWORD delayTime )
{
	m_bReservedQuit = true;
	m_dwReservedQuitTimer = timeGetTime() + delayTime;

	ShowMessage( ZErrStr(errStrID) );
}


void ZGameInterface::OnAnnounceDeleteClan( const string& strAnnounce )
{
	char szMsg[ 128 ];
	
	sprintf( szMsg, MGetStringResManager()->GetErrorStr(MERR_CLAN_ANNOUNCE_DELETE), strAnnounce.c_str() );
	ShowMessage( szMsg );
}



// º¸ÀÌ½º »ç¿îµå °ü·Ã
void ZGameInterface::OnVoiceSound()
{
	DWORD dwCurrTime = timeGetTime();

	// ÁöÁ¤µÈ ½Ã°£ÀÌ µÆ´ÂÁö °Ë»ç
	if ( dwCurrTime < m_dwVoiceTime)
		return;

	m_szCurrVoice[ 0] = 0;

	// ¿¹¾àµÈ »ç¿îµå Ãâ·Â
	if ( m_szNextVoice[ 0] == 0)
		return;

	ZApplication::GetSoundEngine()->PlaySound( m_szNextVoice);
	m_dwVoiceTime = dwCurrTime + m_dwNextVoiceTime;

	strcpy( m_szCurrVoice, m_szNextVoice);
	m_szNextVoice[ 0] = 0;
	m_dwNextVoiceTime = 0;
}

void ZGameInterface::PlayVoiceSound( char* pszSoundName, DWORD time)
{
	if ( !Z_AUDIO_NARRATIONSOUND)
		return;


	if ( strcmp( pszSoundName, m_szCurrVoice) != 0)
	{
		sprintf( m_szNextVoice, pszSoundName);
		m_dwNextVoiceTime = time;
	}


	// ÇÃ·¹ÀÌ ÁßÀÎ »ç¿îµå°¡ ¾øÀ¸¸é ¹Ù·Î ÇÃ·¹ÀÌÇÑ´Ù.
	if ( timeGetTime() > m_dwVoiceTime)
		OnVoiceSound();
}


void ZGameInterface::OnRequestGameguardAuth( const DWORD dwIndex, const DWORD dwValue1, const DWORD dwValue2, const DWORD dwValue3 )
{
#ifdef _GAMEGUARD
	GetZGameguard().SendToCallback( dwIndex, dwValue1, dwValue2, dwValue3 );
#endif
}


void ZGameInterface::OnSendGambleItemList( void* pGItemArray, const DWORD dwCount )
{
	ZGetGambleItemDefineMgr().Release();

	MTD_DBGambleItmeNode*	pGItem;
	ZGambleItemDefine*		pZGItem;


	for( DWORD i = 0; i < dwCount; ++i )
	{
		pGItem = (MTD_DBGambleItmeNode*)MGetBlobArrayElement( pGItemArray, i );
		if( NULL ==  pGItem )
			return;

		pZGItem = new ZGambleItemDefine( pGItem->nItemID
			, pGItem->szName
			, pGItem->szDesc
			, pGItem->nBuyPrice
			, pGItem->bIsCash);
		if( NULL == pZGItem )
			return;

		if( !ZGetGambleItemDefineMgr().AddGambleItemDefine(pZGItem) )
		{
			//_ASSERT( 0 );
		}
	}
}

void ZGameInterface::SetAgentPing(DWORD nIP, DWORD nTimeStamp)
{
	ZServerView* pServerList = (ZServerView*)m_IDLResource.FindWidget( "SelectedServer");
	pServerList->SetAgentPing(nIP, nTimeStamp);
}

void ZGameInterface::MultiplySize( float byIDLWidth, float byIDLHeight, float byCurrWidth, float byCurrHeight )
{
	MWidget::MultiplySize(byIDLWidth, byIDLHeight, byCurrWidth, byCurrHeight);

	if (m_pMsgBox)
		m_pMsgBox->MultiplySize(byIDLWidth, byIDLHeight, byCurrWidth, byCurrHeight);
	if (m_pConfirmMsgBox)
		m_pConfirmMsgBox->MultiplySize(byIDLWidth, byIDLHeight, byCurrWidth, byCurrHeight);
}

void ZGameInterface::SetDuelTournamentCharacterList(MDUELTOURNAMENTTYPE nType, const vector<DTPlayerInfo>& vecDTPlayerInfo)
{
	m_eDuelTournamentType = nType;

	m_vecDTPlayerInfo.clear();
	m_vecDTPlayerInfo = vecDTPlayerInfo;

}

void ZGameInterface::UpdateDuelTournamantMyCharInfoUI()
{
	ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
	if (!pRes) return;

	const ZGameClient::DTCHARINFO* pCharInfo = GetGameClient()->GetMyDuelTournamentCharInfo();
	if (!pCharInfo) return;

	char szOutput[256];
	sprintf(szOutput,"%d/%d",pCharInfo->wins,pCharInfo->losses);
	ZBmNumLabel *pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoWinLose");
	if (pNumLabel)
		pNumLabel->SetText(szOutput);
	sprintf(szOutput,"%d",pCharInfo->tournamentPoint);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoPoints");
	if (pNumLabel)
		pNumLabel->SetText(szOutput);
	sprintf(szOutput,"%d",pCharInfo->winners);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoWinners");
	if (pNumLabel)
		pNumLabel->SetText(szOutput);

	int ranking = (pCharInfo->ranking == -1) ? 0 : pCharInfo->ranking;
	sprintf(szOutput,"%d",ranking);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoRanking");
	if (pNumLabel)
		pNumLabel->SetText(szOutput);

	GetDuelTournamentGradeIconFileName(szOutput, pCharInfo->lastWeekGrade);
	MPicture* pPicture= (MPicture*)pRes->FindWidget("Lobby_DuelTournamentInfoEmblem");
	if (pPicture) {
		pPicture->SetBitmap(MBitmapManager::Get(szOutput));

		char sz[32];
		char szTooltip[256];
		sprintf(sz, "%d", pCharInfo->lastWeekGrade);
		ZTransMsg(szTooltip, MSG_LOBBY_DUELTOURNAMENT_RECORD_TOOPTIP_GRADE, 1, sz);
		SetWidgetToolTipText("Lobby_DuelTournamentInfoEmblem", szTooltip, MAM_VCENTER);
	}
}

void ZGameInterface::UpdateDuelTournamantMyCharInfoPreviousUI()
{
	ZIDLResource* pRes = ZApplication::GetGameInterface()->GetIDLResource();
	if (!pRes) return;

	const ZGameClient::DTCHARINFO* pCharInfo = GetGameClient()->GetMyDuelTournamentCharInfoPrev();
	if (!pCharInfo) return;

	char szOutput[256];
	sprintf(szOutput,"%d/%d",pCharInfo->wins,pCharInfo->losses);
	ZBmNumLabel *pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoWinLosePrev");
	if (pNumLabel)
	{
		pNumLabel->SetIndexOffset(16);		// »ö´Ù¸¥ ±Û¾¾·Î Âï´Â´Ù
		pNumLabel->SetText(szOutput);
	}
	sprintf(szOutput,"%d",pCharInfo->tournamentPoint);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoPointsPrev");
	if (pNumLabel)
	{
		pNumLabel->SetIndexOffset(16);
		pNumLabel->SetText(szOutput);
	}
	sprintf(szOutput,"%d",pCharInfo->winners);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoWinnersPrev");
	if (pNumLabel)
	{
		pNumLabel->SetIndexOffset(16);
		pNumLabel->SetText(szOutput);
	}
	
	int ranking = (pCharInfo->ranking == -1) ? 0 : pCharInfo->ranking;
	sprintf(szOutput,"%d",ranking);
	pNumLabel = (ZBmNumLabel*)pRes->FindWidget("Lobby_DuelTournamentInfoRankingPrev");
	if (pNumLabel)
	{
		pNumLabel->SetIndexOffset(16);
		pNumLabel->SetText(szOutput);
	}
}


#ifdef _GZFGunz
unsigned long int MMColorSett[] = {
	0xFF808080,
	0xFFFF0000,
	0xFF00FF00,
	0xFF0000FF,
	0xFFFFFF00,
	0xFF800000,
	0xFF008000,
	0xFF000080,
	0xFF808000,
	0xFFFFFFFF,
	0xFF00FFFF,
	0xFF388E8E,
	0xFF7D9EC0,
	0xFFC67171,
	0xFFF5F5DC,
	0xFFA9A9A9,
	0xFFEECFA1,
	0xFFEE7600,
	0xFFCDC0B0,
	0xFF000000,
	0xFF8B2323,
	0xFFCDAA7D,
	0xFF458B00,
	0xFFD2691E,
	0xFF6495ED,
	0xFFFFB90F,
	0xFF8B008B,
	0xFF2F4F4F,
	0xFF7CFC00,
	0xFFEEE685,
	0xFF8470FF,
	0xFFFFE4E1,
	0xFFEE4000,
	0xFF8E8E38,
	0xFFFFA54F,
	0xFFFF6347,
	0xFF8B8B00,
	0xFFF5F5F5,
	0xFF00E5EE,
	0xFF708090,
	0xFFF4A460,
	0xFF03FE4A,
	0xFFFFE4E0,
	0xFFEE4001,
	0xFF8E8E3E,
};


int MDrawContext::TextMultiLine(MRECT& r, const char* szText, int nLineGap, bool bAutoNextLine, int nIndentation, int nSkipLine, MPOINT* pPositions)
{
	bool bColorSupport = true;

	MBeginProfile(99, "MDrawContext::TextMultiLine");

	int nLine = 0;
	MFont* pFont = GetFont();

	int nLength = (int)strlen(szText);
	int xRight = 0;
	int y = r.y;
	char* szCurrent = (char*)szText;
	MPOINT* pCurrentPos = pPositions;
	do {
		int nX = nLine == 0 ? 0 : nIndentation;

		int nOriginalCharCount = MMGetNextLinePos(pFont, szCurrent, r.w - nX, bAutoNextLine, true);

		if (nSkipLine <= nLine)
		{
			int nCharCount = min(nOriginalCharCount, 255);
			char buffer[256];
			if (bColorSupport) {

#define FLUSHPOS(_Pos)		if(pCurrentPos!=NULL){	\
								for(int i=0; buffer[i]!=NULL; i++){	\
									pCurrentPos[i+szCurrent-szText].x = _Pos+pFont->GetWidth(buffer, i);	\
									pCurrentPos[i+szCurrent-szText].y = y;	\
																}	\
										}

#define FLUSH				if(buffer[0]) { Text(r.x+nLastX+xRight, y, buffer); FLUSHPOS(r.x+nLastX); nLastX=nX; buffer[0]=0;pcurbuf=buffer; }

				int nLastX = nX;

				buffer[0] = 0;
				char *pcurbuf = buffer;
				for (int i = 0; i<nCharCount; i++){

					unsigned char cminus = szCurrent[i - 1], c = szCurrent[i], cc = szCurrent[i + 1], ccc = szCurrent[i + 2], cccc = szCurrent[i + 3];

					if (c == '^' && ('0' <= cc) && (cc <= 'Z'))
					{
						xRight = 0;
						FLUSH;
						SetColor(MCOLOR(MMColorSett[cc - '0']));
						i++;
						continue;
					}

					else if (c == ':' && (cc == 'D' || cc == 'd'))
					{
						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("1.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;
						}
						else if ((cminus == ' ') && (c == ':') && (cc == 'D' || cc == 'd') && (ccc == ' ' || !ccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("1.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;
						}
					}


					else if (c == ':' && (')' == cc))
					{

						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("3.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;

						}
						else if ((cminus == ' ') && (c == ':') && (cc == ')') && (ccc == ' ' || !ccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("3.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;
						}
					}
					else if (c == ':' && ('(' == cc))
					{

						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("4.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;

						}
						else if ((cminus == ' ') && (c == ':') && (cc == '(') && (ccc == ' ' || !ccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("4.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;


						}
					}
					else if (c == ':' && ('P' == cc || 'p' == cc))
					{

						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("5.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;

						}
						else if ((cminus == ' ') && (c == ':') && ('P' == cc || 'p' == cc) && (ccc == ' ' || !ccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("5.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;


						}
					}
					else if (c == ':' && ('/' == cc))
					{

						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("6.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;

						}
						else if ((cminus == ' ') && (c == ':') && (cc == '/') && (ccc == ' ' || !ccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("6.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;


						}
					}
					else if (c == '<' && ('3' == cc))
					{

						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("7.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;

						}
						else if ((cminus == ' ') && (c == '<') && (cc == '3') && (ccc == ' ' || !ccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("7.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;


						}
					}
					else if (c == '^' && ('_' == cc) && ('^' == ccc))
					{
						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("8.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							ccc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							szCurrent[i + 2] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;
						}
						else if ((cminus == ' ') && (c == '^') && (cc == '_') && (ccc == '^') && (cccc == ' ' || !cccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("8.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							ccc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							szCurrent[i + 2] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;
						}
					}
					else if ((c == 'O' || c == 'o') && (':' == cc) && (')' == ccc))
					{
						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("9.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							ccc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							szCurrent[i + 2] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;
						}
						else if ((cminus == ' ') && (c == 'O' || c == 'o') && (cc == ':') && (ccc == ')') && (cccc == ' ' || !cccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("9.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							ccc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							szCurrent[i + 2] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;
						}
					}
					else if (c == ':' && ('@' == cc))
					{

						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("10.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;

						}
						else if ((cminus == ' ') && (c == ':') && (cc == '@') && (ccc == ' ' || !ccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("10.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;


						}
					}

					else if (c == ';' && ('(' == cc))
					{

						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("2.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;

						}
						else if ((cminus == ' ') && (c == ';') && (cc == '(') && (ccc == ' ' || !ccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("2.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;


						}
					}

					else if (c == ';' && (')' == cc))
					{

						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("12.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;

						}
						else if ((cminus == ' ') && (c == ';') && (cc == ')') && (ccc == ' ' || !ccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("12.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;


						}
					}
					else if (c == ':' && ('O' == cc || 'o' == cc))
					{

						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("13.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;

						}
						else if ((cminus == ' ') && (c == ':') && ('O' == cc || 'o' == cc) && (ccc == ' ' || !ccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("13.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;


						}
					}
					else if (c == '-' && ('_' == cc) && ('-' == ccc))
					{
						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("14.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							ccc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							szCurrent[i + 2] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;
						}
						else if ((cminus == ' ') && (c == '-') && (cc == '_') && (ccc == '-') && (cccc == ' ' || !cccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("14.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							ccc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							szCurrent[i + 2] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;
						}
					}
					else if (c == ':' && ('*' == cc))
					{

						if (strlen(szText) == strlen(ZGetMyInfo()->GetCharName()) + 5){
							FLUSH;
							SetBitmap(MBitmapManager::Get("15.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;

						}
						else if ((cminus == ' ') && (c == ':') && (cc == '*') && (ccc == ' ' || !ccc)){
							FLUSH;
							SetBitmap(MBitmapManager::Get("15.png"));
							Draw((r.x + nX), y, (float)RGetScreenWidth() / (float)800 * 11, (float)RGetScreenWidth() / (float)800 * 11);
							c = ' ';
							cc = ' ';
							szCurrent[i] = ' ';
							szCurrent[i + 1] = ' ';
							xRight = (float)RGetScreenWidth() / (float)1920 * 6;

						}
					}

					int w;

					*(pcurbuf++) = c;
					if (c>127 && i<nCharCount){
						*(pcurbuf++) = cc;
						w = pFont->GetWidth(szCurrent + i, 2);
						i++;
					}
					else w = pFont->GetWidth(szCurrent + i, 1);

					*pcurbuf = 0;

					nX += w;
				}

				FLUSH;
			}
			else
			{
				strncpy(buffer, szCurrent, nCharCount);
				buffer[nCharCount] = 0;
				Text(r.x + nX, y, buffer);
				FLUSHPOS(r.x + nX);
			}
			y += pFont->GetHeight() + nLineGap;
		}

		szCurrent += nOriginalCharCount;
		nLine++;
		if (y >= r.y + r.h) break;
	} while (szCurrent<szText + nLength);

	MEndProfile(99);
	return nLine - nSkipLine;
}

#endif