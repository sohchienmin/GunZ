#include "stdafx.h"
#include "ZShopEquipListbox.h"
#include "ZShopEquipInterface.h"
#include "ZItemSlotView.h"
#include "ZShop.h"


ZShopEquipListItem::ZShopEquipListItem(ZShopEquipItem* pItemData)
: m_pItemData(pItemData)
{
	//_ASSERT(m_pItemData);
	m_pItemData->GetName(m_szName);
	m_pItemData->GetLevelResText(m_szLevel);
	m_pItemData->GetPriceText(m_szPrice);
}

ZShopEquipListItem::~ZShopEquipListItem()
{
	delete m_pItemData;
}

void ZShopEquipListItem::OnDraw( MRECT& r, MDrawContext* pDC, bool bSelected, bool bMouseOver )
{
	// 배경색
	if (bSelected)
		pDC->SetColor(220, 220, 220);
	else if (bMouseOver)
		pDC->SetColor(60, 60, 60);
	else
		pDC->SetColor(30, 30, 30);

	pDC->FillRectangle(r);

	MRECT rcIcon;
	GetIconRect(rcIcon, r);
	rcIcon.x += r.x;
	rcIcon.y += r.y;

	// 아이콘 배경색
	pDC->SetColor(15, 15, 15);
	pDC->FillRectangle(rcIcon);

	pDC->SetBitmap(m_pItemData->GetIcon());
	pDC->Draw(rcIcon);

	MRECT rc;	// 비트맵을 제외한 영역
	rc.x = rcIcon.x + rcIcon.w + 2;
	rc.w = r.w - rcIcon.w - 6;
	rc.y = r.y + 2;
	rc.h = r.h - 4;

	// 텍스트 색
	if (!bSelected)
		pDC->SetColor(200, 200, 200);
	else
		pDC->SetColor(20, 20, 20);

	// 아이템명, 가격
	pDC->TextMultiLine2(rc, GetString(), CONVERT600(2), true, MAM_LEFT | MAM_TOP);
	pDC->Text(rc, m_szPrice, MAM_RIGHT | MAM_BOTTOM);

	// 레벨 (레벨이 낮은 경우 붉은 표시)
	// 썸네일 아이콘 위에 찍기 때문에 그림자를 그려서 시안성을 높인다

	pDC->SetColor(20, 20, 20);
	pDC->Text(MRECT(rcIcon.x+1, rcIcon.y, rcIcon.w, rcIcon.h), m_szLevel, MAM_LEFT | MAM_BOTTOM);	// 1픽셀씩 빗겨찍음
	pDC->Text(MRECT(rcIcon.x-1, rcIcon.y, rcIcon.w, rcIcon.h), m_szLevel, MAM_LEFT | MAM_BOTTOM);
	pDC->Text(MRECT(rcIcon.x, rcIcon.y, rcIcon.w, rcIcon.h+1), m_szLevel, MAM_LEFT | MAM_BOTTOM);
	pDC->Text(MRECT(rcIcon.x, rcIcon.y, rcIcon.w, rcIcon.h-1), m_szLevel, MAM_LEFT | MAM_BOTTOM);

	if (ZGetMyInfo()->GetLevel() < m_pItemData->GetLevelRes())
		pDC->SetColor(200, 10, 10);
	else
		pDC->SetColor(200, 200, 200);

	pDC->Text(rcIcon, m_szLevel, MAM_LEFT | MAM_BOTTOM);
}

bool ZShopEquipListItem::GetDragItem( MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString )
{
	if (!m_pItemData) return true;

	*ppDragBitmap = m_pItemData->GetIcon();
	strcpy(szDragString, m_szName);
	strcpy(szDragItemString, m_szName);
	return true;
}

int ZShopEquipListItem::GetSortHint()
{
	// 리스트박스가 아이템을 종류별로 정렬하도록 종류에 따라서 순번을 리턴한다
	// (각 아이템의 래퍼 클래스가 각자 sorthint를 리턴하도록 하지 않은 이유는 값들이 분산되어 있으면 코드 읽기 힘드니까..)
	if (ZSEIT_GAMBLE == m_pItemData->GetType()) 
		return 1000000;

	if (ZSEIT_MATCH == m_pItemData->GetType())
	{
		int hint = 2000000;
		int nResLv = m_pItemData->GetLevelRes();
		MMatchItemDesc* pItemDesc = ((ZShopEquipItem_Match*)m_pItemData)->GetDesc();
		if (!pItemDesc) { return 0; }

		if (pItemDesc->m_nType.Ref() == MMIT_MELEE)
			return hint + 100000 + pItemDesc->m_nWeaponType.Ref() * 1000 + nResLv;
		if (pItemDesc->m_nType.Ref() == MMIT_RANGE)
			return hint + 200000 + pItemDesc->m_nWeaponType.Ref() * 1000 + nResLv;
		if (pItemDesc->m_nType.Ref() == MMIT_EQUIPMENT)
			return hint + 300000 + pItemDesc->m_nSlot * 1000 + nResLv;

		hint = 4000000;
		if (pItemDesc->m_nType.Ref() == MMIT_CUSTOM)
		{
			hint = hint + 400000 + pItemDesc->m_nWeaponType.Ref() * 1000 + nResLv;
			if (pItemDesc->m_nWeaponType.Ref() == MWT_POTION)
			{
				hint = hint + pItemDesc->m_nEffectId * 100;
			}
			return hint;
		}
		if (pItemDesc->m_nType.Ref() == MMIT_COMMUNITY)
			return hint + 500000 + pItemDesc->m_nWeaponType.Ref() * 1000 + nResLv;
		if (pItemDesc->m_nType.Ref() == MMIT_LONGBUFF)
			return hint + 600000 + pItemDesc->m_nWeaponType.Ref() * 1000 + nResLv;

		return hint + 700000 + nResLv;
	}

	if (ZSEIT_SET == m_pItemData->GetType())
	{
		int nResLv = ((ZShopEquipItem_Set*)m_pItemData)->GetLevelRes();
		return 3000000 + nResLv;
	}

	if (ZSEIT_QUEST == m_pItemData->GetType())
		return 5000000;

	return 9999999;
}

void ZShopEquipListItem::GetIconRect(MRECT& out, const MRECT& rcItem)
{
	int len = rcItem.h-4;
	out.Set(2, 2, len, len);
}

bool ZShopEquipListItem::IsPtInRectToShowToolTip(MRECT& rcItem, MPOINT& pt)
{
	// pt가 썸네일 아이콘 영역에 있는지 판단
	MRECT rcIcon;
	GetIconRect(rcIcon, rcItem);
	// 리스트 좌표계로 변환
	rcIcon.x += rcItem.x;
	rcIcon.y += rcItem.y;
	// pt-in-rect 판정
	return (rcIcon.x < pt.x && pt.x < rcIcon.x+rcIcon.w &&
		rcIcon.y < pt.y && pt.y < rcIcon.y+rcIcon.h);
}



ZShopEquipListbox::ZShopEquipListbox(const char* szName, MWidget* pParent, MListener* pListener)
: MMultiColListBox(szName, pParent, pListener)
{
	m_idxItemLastTooltip = -1;
}

bool ZShopEquipListbox::OnEvent( MEvent* pEvent, MListener* pListener )
{
	MRECT rtClient = GetClientRect();

	if(pEvent->nMessage==MWM_MOUSEMOVE)
	{
		SetupItemDescTooltip();
		//return true;			  // 메시지를 먹어버리면 툴팁이 안나온다
	}

	return MMultiColListBox::OnEvent(pEvent, pListener);
}

char* ZShopEquipListbox::GetItemDescTooltipName()
{
	if (GetParent() && 0==strcmp("Shop", GetParent()->m_szIDLName))
		return "Shop_ItemDescription";

	if (GetParent() && 0==strcmp("Equipment", GetParent()->m_szIDLName))
		return "Equip_ItemDescription";

	//_ASSERT(0);
	return "_noExistWidgetName_";
}

void ZShopEquipListbox::SetupItemDescTooltip()
{
	const char* szTextAreaName = GetItemDescTooltipName();
	MTextArea* pItemDescTextArea = (MTextArea*)ZGetGameInterface()->GetIDLResource()->FindWidget(szTextAreaName);
	if (pItemDescTextArea)
	{
		MPOINT ptInList = MScreenToClient(this, MEvent::LatestPos);
		int idxItem = FindItem(ptInList);
		if (idxItem!=-1)
		{
			ZShopEquipListItem* pItem = (ZShopEquipListItem*)Get(idxItem);
			MRECT rcListBox = GetRect();
			MRECT rcItem;
			if (pItem && CalcItemRect(idxItem, rcItem))	// 항목이 표시되고 있는 영역을 알아냄 (리스트 좌표계)
			{
				if (pItem->IsPtInRectToShowToolTip(rcItem, ptInList)) // 항목에게 이 좌표가 썸네일 이미지 영역인지 물어봄
				{
					if (m_idxItemLastTooltip==idxItem) return;
					m_idxItemLastTooltip = idxItem;

					pItem->GetItemData()->FillItemDesc(pItemDescTextArea);

					// 툴팁의 위치
					MRECT rcTextArea = pItemDescTextArea->GetRect();
					MPOINT posDesc(rcItem.x, rcItem.y);
					posDesc = MClientToScreen(this, posDesc);
					posDesc.x -= pItemDescTextArea->GetClientWidth();			// 일단 아이콘의 왼쪽으로
					if (posDesc.y+rcTextArea.h > rcListBox.y + rcListBox.h)		// 리스트 박스 하단을 넘어가지 않게 하자
						posDesc.y = rcListBox.y + rcListBox.h - rcTextArea.h;
					if (posDesc.y < 0)											// 그렇다고 화면 위로 뚫고 가면 안된다
						posDesc.y = 0;
					pItemDescTextArea->SetPosition(posDesc);
					pItemDescTextArea->SetZOrder(MZ_TOP);
					ZGetGameInterface()->GetShopEquipInterface()->ShowItemDescription(true, pItemDescTextArea, this);
					return;
				}
			}
		}
	}

	m_idxItemLastTooltip = -1;
	ZGetGameInterface()->GetShopEquipInterface()->ShowItemDescription(false, pItemDescTextArea, this);
}


// ============ ZShopPurchaseItemListBoxListener ===============================================

void ShopPurchaseItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
}

void ShopSaleItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
}

void CharacterEquipmentItemListBoxOnDrop(void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if (pSender == NULL) return;
	if (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)) return;

	ZItemSlotView* pItemSlotView = (ZItemSlotView*)pSender;

	ZPostRequestTakeoffItem(ZGetGameClient()->GetPlayerUID(), pItemSlotView->GetParts());
}

// ============ ZShopPurchaseItemListBoxListener ===============================================

class ZShopPurchaseItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true)
		{
			ZShopEquipListbox* pEquipmentListBox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pEquipmentListBox->GetSelItem();
			if (pListItem == NULL) return true;

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformationShop");
			if (pCharacterView)
				pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

			pListItem->GetItemData()->UpdateCharInfoText();

			ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType( pListItem->GetItemData()->GetSlotType());

			WidgetEnableShow("BuyConfirmCaller", true, true);

			return true;
		}
		else if(MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK) == true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->OnBuyButton();
			return true;
		}

		return false;
	}
};
ZShopPurchaseItemListBoxListener g_ShopPurchaseItemListBoxListener;

MListener* ZGetShopPurchaseItemListBoxListener(void)
{
	return &g_ShopPurchaseItemListBoxListener;
}

class ZEquipMyItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if ( MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true) {

			ZShopEquipListbox* pEquipmentListBox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pEquipmentListBox->GetSelItem();
			if (!pListItem) return false;

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformation");
			if (pCharacterView)
				pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

			pListItem->GetItemData()->UpdateCharInfoText();

			ZGetGameInterface()->GetShopEquipInterface()->SetKindableItem( pListItem->GetItemData()->GetSlotType());
			ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType( pListItem->GetItemData()->GetSlotType());

			WidgetEnableShow("Equip",				pListItem->GetItemData()->CanEquip(), true);
			WidgetEnableShow("SendAccountItemBtn",	pListItem->GetItemData()->CanSendAccount(), true);

			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->Equip();		
			return true;
		}

		return false;
	}
};
ZEquipMyItemListBoxListener g_EquipmentItemListBoxListener;

MListener* ZGetEquipmentMyItemListBoxListener(void)
{
	return &g_EquipmentItemListBoxListener;
}


class ZShopSellItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if(MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true)
		{
			ZShopEquipListbox* pListbox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pListbox->GetSelItem();
			
			if (!pListItem) return false;

			ZGetGameInterface()->GetShopEquipInterface()->DrawCharInfoText();
			ZGetGameInterface()->GetShopEquipInterface()->SetKindableItem( pListItem->GetItemData()->GetSlotType());

			WidgetEnableShow("SellConfirmCaller", pListItem->GetItemData()->CanSell(), true);
			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->OnSellButton();
			return true;
		}

		return false;
	}
};

ZShopSellItemListBoxListener g_ShopSellItemListBoxListener;

MListener* ZGetShopSellItemListBoxListener(void)
{
	return &g_ShopSellItemListBoxListener;
}

class ZAccountItemListBoxListener : public MListener{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if ( MWidget::IsMsg(szMessage, MLB_ITEM_SEL)==true) {

			ZShopEquipListbox* pListbox = (ZShopEquipListbox*)pWidget;
			ZShopEquipListItem* pListItem = (ZShopEquipListItem*)pListbox->GetSelItem();
			if (!pListItem) return false;

			unsigned long int nItemID = ZGetMyInfo()->GetItemList()->GetAccountItemID( pListbox->GetSelIndex());

			ZCharacterView* pCharacterView = (ZCharacterView*)ZGetGameInterface()->GetIDLResource()->FindWidget("EquipmentInformation");
			if (pCharacterView)
				pListItem->GetItemData()->UpdateCharacterView(pCharacterView);

			pListItem->GetItemData()->UpdateCharInfoText();
			ZGetGameInterface()->GetShopEquipInterface()->SelectArmorWeaponTabWithSlotType( pListItem->GetItemData()->GetSlotType());

			// 중앙은행에서 바운티 아이템이 들어있다라도 조건없이 내장비로 옮길수있다.
			WidgetEnableShow("BringAccountItemBtn", true, true);

			return true;
		}
		else if ( MWidget::IsMsg(szMessage, MLB_ITEM_DBLCLK)==true)
		{
			ZGetGameInterface()->GetShopEquipInterface()->OnBringAccountButton();
			return true;
		}

		return false;
	}
};

ZAccountItemListBoxListener g_AccountItemListBoxListener;

MListener* ZGetAccountItemListBoxListener(void)
{
	return &g_AccountItemListBoxListener;
}



class ZShopListFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MCMBBOX_CHANGED) == true)
		{
			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentFilter");
			if (pComboBox)
			{
				int sel = pComboBox->GetSelIndex();

				ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
				MComboBox* pWidget = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentItemFilter");
				MComboBox* pWidget2 = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentFilter");
				MComboBox* pAll = (MComboBox*)pResource->FindWidget("Shop_AllFilter");
				ZGetShop()->m_ListFilter = sel;
				ZGetShop()->Serialize(pAll->GetSelIndex());

				ZMyItemList* pil = ZGetMyInfo()->GetItemList();
				pil->m_ListFilter = sel;
				pil->Serialize();
				switch (pWidget2->GetSelIndex())
				{
				case 8:
				{
					pWidget->SetSelIndex(0);
					pWidget->Enable(true);
					pWidget->RemoveAll();
					pWidget->Add("All");
					pWidget->Add("Dagger");
					pWidget->Add("Dual Dagger");
					pWidget->Add("Katana");
					pWidget->Add("Great Sword");
					pWidget->Add("Double Katana");
				}
					break;
				case 9:
				{
					pWidget->SetSelIndex(0);
					pWidget->Enable(true);
					pWidget->RemoveAll();
					pWidget->Add("All");
					pWidget->Add("Pistol");
					pWidget->Add("Double Pistol");
					pWidget->Add("Revolver");
					pWidget->Add("Double Revolver");
					pWidget->Add("SMG");
					pWidget->Add("Double SMG ");
					pWidget->Add("Shotgun");
					pWidget->Add("Sawed Shotgun");
					pWidget->Add("Rifle");
					pWidget->Add("Machine Gun");
					pWidget->Add("Rocket");
					pWidget->Add("Sniper");
				}
					break;
				case 10:
				{
					pWidget->SetSelIndex(0);
					pWidget->Enable(true);
					pWidget->RemoveAll();
					pWidget->Add("All");
					pWidget->Add("Med");
					pWidget->Add("Repair");
					pWidget->Add("Bullet");
					pWidget->Add("Flash");
					pWidget->Add("Frag");
					pWidget->Add("Smoke");
					pWidget->Add("Food");
				}
					break;
				case 11:
				{
					pWidget->SetSelIndex(0);
					ZGetShop()->SerializeQuest();
					pWidget->Enable(true);
					pWidget->RemoveAll();
					pWidget->Add("All");
					pWidget->Add("Goblin King");
					pWidget->Add("Fake Goblin King");
					pWidget->Add("Thunder Goblin King");
					pWidget->Add("Dwarf Goblin King");
					pWidget->Add("Giant Lizardman");
					pWidget->Add("Broken Golem");
					pWidget->Add("Superion The Tainted");
					pWidget->Add("Aneramon The Wicked");
					pWidget->Add("Lich");
					pWidget->Add("Palmpoa Commander");
					pWidget->Add("Palmpow");
					pWidget->Add("Cursed Palmpow");
					pWidget->Add("Mario Special");
					pWidget->Add("Ghost King");
				}
					break;
				case 12:
				{
					pWidget->SetSelIndex(0);
					pWidget->Enable(true);
					pWidget->RemoveAll();
					pWidget->Add("All");
					pWidget->Add("Fire");
					pWidget->Add("Cold");
					pWidget->Add("Lighting");
					pWidget->Add("Poison");
				}
					break;
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				{
					pWidget->SetSelIndex(0);
					pWidget->CloseComboBoxList();
					pWidget->Enable(false);
					pWidget->RemoveAll();
					pWidget->SetText("Sub Filter");
				}
					break;
				default:
				{
					pWidget->SetSelIndex(0);
					pWidget->CloseComboBoxList();
					pWidget->Enable(false);
					pWidget->RemoveAll();
					pWidget->SetText("Sub Filter");
				}
					break;
				}


			}

			ZGetGameInterface()->GetShopEquipInterface()->SelectShopTab(-1);
		}
		return true;
	}
};

ZShopListFilterListener g_ShopListFilterListener;

MListener* ZGetShopListFilterListener()
{
	return &g_ShopListFilterListener;
}

/////////////////////////////////////////////////////////////////

class ZShopItemListFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MCMBBOX_CHANGED) == true)
		{
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MComboBox* pWidget = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentItemFilter");
			MComboBox* pWidget2 = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentFilter");
			ZMyItemList* pil = ZGetMyInfo()->GetItemList();

			switch (pWidget2->GetSelIndex())
			{
			case 8:
			{
				pil->SerializeZItemListItems(pWidget->GetSelIndex(), pWidget->GetSelIndex(), 1);
				ZGetShop()->SerializeItems(pWidget->GetSelIndex(), pWidget->GetSelIndex(), 1);
			}
				break;
			case 9:
			{
				pil->SerializeZItemListItems(pWidget->GetSelIndex(), pWidget->GetSelIndex() + 5, 2);
				ZGetShop()->SerializeItems(pWidget->GetSelIndex(), pWidget->GetSelIndex() + 5, 2);
			}
				break;
			case 10:
			{
				pil->SerializeZItemListItems(pWidget->GetSelIndex(), pWidget->GetSelIndex() + 17, 3);
				ZGetShop()->SerializeItems(pWidget->GetSelIndex(), pWidget->GetSelIndex() + 17, 3);
			}
				break;
			case 11:
			{
				ZGetShop()->SerializeQuest();
			}
				break;
			case 12:
			{
				pil->SerializeZItemListItems(pWidget->GetSelIndex(), pWidget->GetSelIndex() + 25, 3);
				ZGetShop()->SerializeItems(pWidget->GetSelIndex(), pWidget->GetSelIndex() + 25, 3);
			}
				break;
			}
		}
		return true;
	}
};

BEGIN_IMPLEMENT_LISTENER(ZGetScreenShotCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ShowScreenShotDialog(true);
END_IMPLEMENT_LISTENER();
BEGIN_IMPLEMENT_LISTENER(ZGetViewScreenShotCallerButtonListener, MBTN_CLK_MSG)
ZGetGameInterface()->ViewShowScreenShotDialog(true);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetExitScreenShotPictureCallerButtonListener, MBTN_CLK_MSG)
MFrame* pFrame = (MFrame*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShotPictureFrame");
pFrame->Show(false, false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetExitScreenShotCallerButtonListener, MBTN_CLK_MSG)
MFrame* pMain = (MFrame*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShot");
pMain->Show(false, false);
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetNextScreenShotCallerButtonListener, MBTN_CLK_MSG)
MFrame* pFrame = (MFrame*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShotPictureFrame");
MListBox* pListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShot_List");
MPicture* pPicture = (MPicture*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShotPicture");
pFrame->Show(true, true);
pListBox->SetSelIndex(pListBox->GetSelIndex() + 1);
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
END_IMPLEMENT_LISTENER();

BEGIN_IMPLEMENT_LISTENER(ZGetPrevScreenShotCallerButtonListener, MBTN_CLK_MSG)
MFrame* pFrame = (MFrame*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShotPictureFrame");
MListBox* pListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShot_List");
MPicture* pPicture = (MPicture*)ZGetGameInterface()->GetIDLResource()->FindWidget("ScreenShotPicture");
pFrame->Show(true, true);
pListBox->SetSelIndex(pListBox->GetSelIndex() - 1);
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
END_IMPLEMENT_LISTENER();

ZShopItemListFilterListener g_ShopItemListFilterListener;
MListener* ZGetShopItemListFilterListener()
{
	return &g_ShopItemListFilterListener;
}

class MEquipListFilterListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MCMBBOX_CHANGED) == true) {

			ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();

			MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("Equip_AllEquipmentFilter");

			MComboBox* pWidget = (MComboBox*)pResource->FindWidget("Equip_AllEquipmenItemtFilter");
			if (pComboBox)
			{
				int sel = pComboBox->GetSelIndex();

				switch (pComboBox->GetSelIndex())
				{
				case 8:
				{
					pWidget->SetSelIndex(0);
					pWidget->Enable(true);
					pWidget->RemoveAll();
					pWidget->Add("All");
					pWidget->Add("Dagger");
					pWidget->Add("Dual Dagger");
					pWidget->Add("Katana");
					pWidget->Add("Great Sword");
					pWidget->Add("Double Katana");
				}
					break;
				case 9:
				{
					pWidget->SetSelIndex(0);
					pWidget->Enable(true);
					pWidget->RemoveAll();
					pWidget->Add("All");
					pWidget->Add("Pistol");
					pWidget->Add("Double Pistol");
					pWidget->Add("Revolver");
					pWidget->Add("Double Revolver");
					pWidget->Add("SMG");
					pWidget->Add("Double SMG ");
					pWidget->Add("Shotgun");
					pWidget->Add("Sawed Shotgun");
					pWidget->Add("Rifle");
					pWidget->Add("Machine Gun");
					pWidget->Add("Rocket");
					pWidget->Add("Sniper");
				}
					break;
				case 10:
				{
					pWidget->SetSelIndex(0);
					pWidget->Enable(true);
					pWidget->RemoveAll();
					pWidget->Add("All");
					pWidget->Add("Med");
					pWidget->Add("Repair");
					pWidget->Add("Bullet");
					pWidget->Add("Flash");
					pWidget->Add("Frag");
					pWidget->Add("Smoke");
					pWidget->Add("Food");
				}
					break;
				case 11:
				{
					pWidget->SetSelIndex(0);
					pWidget->Enable(true);
					pWidget->RemoveAll();
					pWidget->Add("All");
					pWidget->Add("Goblin King");
					pWidget->Add("Fake Goblin King");
					pWidget->Add("Thunder Goblin King");
					pWidget->Add("Dwarf Goblin King");
					pWidget->Add("Giant Lizardman");
					pWidget->Add("Broken Golem");
					pWidget->Add("Superion The Tainted");
					pWidget->Add("Aneramon The Wicked");
					pWidget->Add("Lich");
					pWidget->Add("Palmpoa Commander");
					pWidget->Add("Palmpow");
					pWidget->Add("Cursed Palmpow");
					pWidget->Add("Mario Special");
					pWidget->Add("Ghost King");//done
					pWidget->Add("Woodlander");//done				
					pWidget->Add("Cyber Quest");
					pWidget->Add("MineCraft");
				}
					break;
				case 12:
				{
					pWidget->SetSelIndex(0);
					pWidget->Enable(true);
					pWidget->RemoveAll();
					pWidget->Add("All");
					pWidget->Add("Fire");
					pWidget->Add("Cold");
					pWidget->Add("Lighting");
					pWidget->Add("Poison");
				}
					break;
				case 0:
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				{
					pWidget->CloseComboBoxList();
					pWidget->Enable(false);
					pWidget->RemoveAll();
					pWidget->SetText("Sub Filter");
				}
					break;
				default:
				{
					pWidget->CloseComboBoxList();
					pWidget->Enable(false);
					pWidget->RemoveAll();
					pWidget->SetText("Sub Filter");
				}
					break;
				}
				ZMyItemList* pil = ZGetMyInfo()->GetItemList();
				pil->m_ListFilter = sel;
				pil->Serialize();

			}

			ZGetGameInterface()->GetShopEquipInterface()->SelectEquipmentTab(-1);
		}
		return true;
	}
};

MEquipListFilterListener g_EquipListFilterListener;

MListener* ZGetEquipListFilterListener()
{
	return &g_EquipListFilterListener;
}

class MEquipListFilterItemListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MCMBBOX_CHANGED) == true) {
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MComboBox* pWidget = (MComboBox*)pResource->FindWidget("Equip_AllEquipmenItemtFilter");
			MComboBox* pWidget2 = (MComboBox*)pResource->FindWidget("Equip_AllEquipmentFilter");

			ZMyItemList* pil = ZGetMyInfo()->GetItemList();

			switch (pWidget2->GetSelIndex())
			{
			case 8:
			{
				pil->SerializeZItemListItems(pWidget->GetSelIndex(), pWidget->GetSelIndex(), 1);
			}
				break;
			case 9:
			{
				pil->SerializeZItemListItems(pWidget->GetSelIndex(), pWidget->GetSelIndex() + 5, 2);
			}
				break;
			case 10:
			{
				pil->SerializeZItemListItems(pWidget->GetSelIndex(), pWidget->GetSelIndex() + 17, 3);
			}
				break;
			case 11:
			{
				pil->SerializeQuest();
			}
				break;
			case 12:
			{
				pil->SerializeZItemListItems(pWidget->GetSelIndex(), pWidget->GetSelIndex() + 25, 3);
			}
				break;
			}
		}
		return true;
	}
};
MEquipListFilterItemListener g_EquipListItemFilterListener;

MListener* ZGetEquipListItemFilterListener()
{
	return &g_EquipListItemFilterListener;
}


class MShopAllFilterItemListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MCMBBOX_CHANGED) == true) {
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MComboBox* pWidget = (MComboBox*)pResource->FindWidget("Shop_AllFilter");
			ZGetShop()->Serialize(pWidget->GetSelIndex());
		}
		return true;
	}
};
MShopAllFilterItemListener g_ShopAllFilterListener;

MListener* ZGetShopAllFilterListener()
{
	return &g_ShopAllFilterListener;
}

class MEquipAllFilterItemListener : public MListener {
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		if (MWidget::IsMsg(szMessage, MCMBBOX_CHANGED) == true) {
			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			MComboBox* pWidget = (MComboBox*)pResource->FindWidget("Equip_AllFilter");
			
		}
		return true;
	}
};
MEquipAllFilterItemListener g_EquipAllFilterListener;

MListener* ZGetEquipAllFilterListener()
{
	return &g_EquipAllFilterListener;
}
