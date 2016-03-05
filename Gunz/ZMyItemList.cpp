#include "stdafx.h"

#include "ZMyItemList.h"
#include "ZGameInterface.h"
#include "MMultiColListBox.h"
#include "MLabel.h"
#include "ZApplication.h"
#include "ZShopEquipListbox.h"
#include "MMatchTransDataType.h"
#include "ZMyInfo.h"
#include "ZCharacterView.h"
#include "MMatchItemFunction.h"



enum {
	zequip_item_filter_all = 0,
	zequip_item_filter_head,
	zequip_item_filter_chest,
	zequip_item_filter_hands,
	zequip_item_filter_legs,
	zequip_item_filter_feet,
	zequip_item_filter_extra,
#ifdef _AVATAR_ENABLE
	zequip_item_filter_avatar,
#endif
	zequip_item_filter_melee,
	zequip_item_filter_range,
	zequip_item_filter_custom,
	zequip_item_filter_quest,
	zequip_item_filter_enchant,
	zequip_item_filter_dance,
};


ZMyItemList::ZMyItemList() : m_bCreated(false)
{
	m_ListFilter = zequip_item_filter_all;
}

ZMyItemList::~ZMyItemList()
{

}

bool ZMyItemList::Create()
{
	if (m_bCreated) return false;
	Clear();

	ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());

	return true;
}
void ZMyItemList::Destroy()
{
	if (!m_bCreated) return;

}
void ZMyItemList::Clear()
{
	for (int i = 0; i < MMCIP_END; i++) 
		m_uidEquipItems[i] = MUID(0,0);

	memset(m_nEquipItemID, 0, sizeof(m_nEquipItemID));

	ClearItemMap();
	m_ItemIndexVector.clear();
	m_ItemIndexVectorEquip.clear();

	ClearAccountItems();
}

unsigned long int ZMyItemList::GetEquipedItemID(MMatchCharItemParts parts)
{
	MITEMNODEMAP::iterator itor = m_ItemMap.find(m_uidEquipItems[(int)parts]);
	if (itor != m_ItemMap.end())
	{
		ZMyItemNode* pItemNode = (*itor).second;
		return pItemNode->GetItemID();
	}

	return m_nEquipItemID[parts];
}

MUID ZMyItemList::GetEquipedItemUID(MMatchCharItemParts parts)
{
	MUID uid = m_uidEquipItems[parts];

	MITEMNODEMAP::iterator itor = m_ItemMap.find(uid);
	if (itor != m_ItemMap.end())
	{
		return uid;
	}
	else
	{
		return MUID(0,0);
	}
}

unsigned long int ZMyItemList::GetItemID(int nItemIndex)
{
	ZMyItemNode* pItemNode = GetItem(nItemIndex);
	if (pItemNode == NULL) return 0;
	return pItemNode->GetItemID();

}

unsigned long int ZMyItemList::GetItemIDEquip(int nItemIndex)
{
	ZMyItemNode* pItemNode = GetItemEquip(nItemIndex);
	if (pItemNode == NULL) return 0;
	return pItemNode->GetItemID();

}

bool ZMyItemList::CheckTypeWithListFilter(int type, bool bEnchantItem)
{
	MMatchWeaponType wtype = MWT_NONE;
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	switch (m_ListFilter)
	{
		case zequip_item_filter_all:
		{
			return true;
		}
		break;
		case zequip_item_filter_head:
		{
			if (type == MMIST_HEAD)
			{
				return true;
			}
		}
		break;
		case zequip_item_filter_chest:
		{
			if (type == MMIST_CHEST)
			{
				return true;
			}
		}
		break;
		case zequip_item_filter_hands:
		{
			if (type == MMIST_HANDS)
			{
				return true;
			}
		}
		break;
		case zequip_item_filter_legs:
		{
			if (type == MMIST_LEGS)
			{
				return true;
			}
		}
		break;
		case zequip_item_filter_feet:
		{
			if (type == MMIST_FEET)
			{
				return true;
			}
		}
		break;
		case zequip_item_filter_extra:
		{
			if (type == MMIST_EXTRA || type == MMIST_FINGER)
			{
				return true;
			}
		}
		break;
#ifdef _AVATAR_ENABLE
		case zequip_item_filter_avatar:
		{
			if (type == MMIST_AVATAR)
			{
				return true;
			}
		}
		break;
#endif
		case zequip_item_filter_melee:
		{
			if (type == MMIST_MELEE)
			{
				return true;
			}
		}
		break;
		case zequip_item_filter_range:
		{
			if (type == MMIST_RANGE)
			{
				return true;
			}
		}
		break;
		case zequip_item_filter_custom:
		{
			if (type == MMIST_CUSTOM)
			{
				return true;
			}
		}
		break;
		case zequip_item_filter_enchant:
		{
			if (type == MMIST_CUSTOM)
			{
				if (bEnchantItem)
					return true;
			}
		}
		break;
#ifdef _GZFGunz
		case zequip_item_filter_dance:
		{
			if (type == MMIST_DANCE || type == MMIST_DASH)
			{
				return true;
			}
		}
#endif
		break;
		default:
		{
			return true;
		}
	}
	return false;
}

void ZMyItemList::Serialize()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if( NULL == pResource ) return;

	// ³» Àåºñ ¸ñ·Ï.
	MMultiColListBox* pListbox1 = (MMultiColListBox*)pResource->FindWidget("EquipmentList");
	if( NULL == pListbox1 ) return;

	// »óÁ¡¿¡ ÆÈ¼ö ÀÖ´Â ¸ñ·Ï.
	MMultiColListBox* pListbox2 = (MMultiColListBox*)pResource->FindWidget("MyAllEquipmentList");
	if( NULL == pListbox2 ) return;

	int nOldSelIndex1 = pListbox1->GetSelIndex();
	int nOldSelIndex2 = pListbox2->GetSelIndex();
	int nScrollPos1 = pListbox1->GetRowFirstVisible();
	int nScrollPos2 = pListbox2->GetRowFirstVisible();

	pListbox1->RemoveAll();
	pListbox2->RemoveAll();

	SerializeZItemList();

	// ½ºÅ©·Ñ À§Ä¡¿Í ¾ÆÀÌÅÛ ¼±ÅÃ »óÅÂ¸¦ RemoveAll()ÇÏ±âÀü°ú ºñ½ÁÇÑ »óÅÂ·Î º¹¿ø
	pListbox1->SetRowFirstVisible( nScrollPos1 );
	pListbox1->SetSelIndex( min(pListbox1->GetNumItem()-1, nOldSelIndex1));

	pListbox2->SetRowFirstVisible( nScrollPos2 );
	pListbox2->SetSelIndex( min(pListbox2->GetNumItem()-1, nOldSelIndex2));


	// Àåºñ Ä³¸¯ÅÍ ºä¾î
	BEGIN_WIDGETLIST("EquipmentInformation", pResource, ZCharacterView*, pCharacterView);
	ZMyInfo* pmi = ZGetMyInfo();
	unsigned long int nEquipedItemID[MMCIP_END];
	for (int i = 0; i < MMCIP_END; i++)
	{
		nEquipedItemID[i] = GetEquipedItemID(MMatchCharItemParts(i));
	}
	pCharacterView->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nEquipedItemID);
	//½ºÅ×ÀÌÁö¿¡¼­ÀÇ Ä³¸¯ÅÍ·èÀ» ¹Ù²Þ..
	ZCharacterView* pStageCharacterView = (ZCharacterView*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_Charviewer");
	if(pStageCharacterView!= NULL)
		pStageCharacterView->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nEquipedItemID);

	END_WIDGETLIST();

	// »óÁ¡ Ä³¸¯ÅÍ ºä¾î
	BEGIN_WIDGETLIST("EquipmentInformationShop", pResource, ZCharacterView*, pCharacterView);
	ZMyInfo* pmi = ZGetMyInfo();
	unsigned long int nEquipedItemID[MMCIP_END];
	for (int i = 0; i < MMCIP_END; i++)
	{
		nEquipedItemID[i] = GetEquipedItemID(MMatchCharItemParts(i));
	}
	pCharacterView->InitCharParts(pmi->GetSex(), pmi->GetHair(), pmi->GetFace(), nEquipedItemID);
	END_WIDGETLIST();
}


void ZMyItemList::SerializeZItemList()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (NULL == pResource) return;

	MMultiColListBox* pMyListBox = (MMultiColListBox*)pResource->FindWidget("EquipmentList");
	if (NULL == pMyListBox) return;

	MMultiColListBox* pSellListBox = (MMultiColListBox*)pResource->FindWidget("MyAllEquipmentList");
	if (NULL == pSellListBox) return;

	MakeMyItemUIDList();

	ZShopEquipItem_Match* pWrappedItem;
	ZShopEquipItemHandle_SellMatch* pHandleSell;
	ZShopEquipItemHandle_SendAccountMatch* pHandleSendAcc;
	MMatchItemDesc* pItemDesc;
	ZMyItemNode* pItemNode;
	MUID uidItem;

	for (int i = 0; i < (int)m_ItemIndexVector.size(); i++)
	{
		MITEMNODEMAP::iterator itor = m_ItemMap.find(m_ItemIndexVector[i]);
		if (itor != m_ItemMap.end())
		{
			pItemNode = (*itor).second;
			uidItem = (*itor).first;

			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(pItemNode->GetItemID());
			if (!pItemDesc) { continue; }

			if (false == CheckTypeWithListFilter(pItemDesc->m_nSlot, pItemDesc->IsEnchantItem())) continue;

			pWrappedItem = new ZShopEquipItem_Match(pItemDesc);
			pHandleSell = new ZShopEquipItemHandle_SellMatch(pWrappedItem);
			pHandleSell->SetItemUID(uidItem);
			pWrappedItem->SetHandleSell(pHandleSell);
			pHandleSendAcc = new ZShopEquipItemHandle_SendAccountMatch(pWrappedItem);
			pHandleSendAcc->SetItemUID(uidItem);
			pWrappedItem->SetHandleSendAccount(pHandleSendAcc);
			pMyListBox->Add(new ZShopEquipListItem(pWrappedItem));

			pWrappedItem = new ZShopEquipItem_Match(pItemDesc);
			pHandleSell = new ZShopEquipItemHandle_SellMatch(pWrappedItem);
			pHandleSell->SetItemUID(uidItem);
			pWrappedItem->SetHandleSell(pHandleSell);
			pHandleSendAcc = new ZShopEquipItemHandle_SendAccountMatch(pWrappedItem);
			pHandleSendAcc->SetItemUID(uidItem);
			pWrappedItem->SetHandleSendAccount(pHandleSendAcc);
			pSellListBox->Add(new ZShopEquipListItem(pWrappedItem));

			m_ItemIndexVectorEquip.push_back((*itor).first);
		}
	}
}

void ZMyItemList::SerializeZItemListItems(int nIndex, int nWeaponType, int nSlot)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (NULL == pResource) return;

	MMultiColListBox* pMyListBox = (MMultiColListBox*)pResource->FindWidget("EquipmentList");
	if (NULL == pMyListBox) return;
	pMyListBox->RemoveAll();

	MMultiColListBox* pSellListBox = (MMultiColListBox*)pResource->FindWidget("MyAllEquipmentList");
	if (NULL == pSellListBox) return;
	pSellListBox->RemoveAll();

	MComboBox* pWidget = (MComboBox*)pResource->FindWidget("Equip_AllEquipmenItemtFilter");
	MComboBox* pShop = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentItemFilter");


	MakeMyItemUIDList();

	ZShopEquipItem_Match* pWrappedItem;
	ZShopEquipItemHandle_SellMatch* pHandleSell;
	ZShopEquipItemHandle_SendAccountMatch* pHandleSendAcc;
	MMatchItemDesc* pItemDesc;
	ZMyItemNode* pItemNode;
	MUID uidItem;

	for (int i = 0; i < (int)m_ItemIndexVector.size(); i++)
	{
		MITEMNODEMAP::iterator itor = m_ItemMap.find(m_ItemIndexVector[i]);
		if (itor != m_ItemMap.end())
		{
			pItemNode = (*itor).second;
			uidItem = (*itor).first;

			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(pItemNode->GetItemID());
			if (!pItemDesc) { continue; }


			if (pWidget->GetSelIndex() == nIndex)
			{
				if (nIndex == 0)
				{
					if (pItemDesc->m_nSlot != nSlot)
						continue;
				}
				else
				{
					if ((pItemDesc->m_nWeaponType.Ref() != nWeaponType))
						continue;
				}
			}

			if (pShop->GetSelIndex() == nIndex)
			{
				if (nIndex == 0)
				{
					if (pItemDesc->m_nSlot != nSlot)
						continue;
				}
				else
				{
					if ((pItemDesc->m_nWeaponType.Ref() != nWeaponType))
						continue;
				}
			}


			pWrappedItem = new ZShopEquipItem_Match(pItemDesc);
			pHandleSell = new ZShopEquipItemHandle_SellMatch(pWrappedItem);
			pHandleSell->SetItemUID(uidItem);
			pWrappedItem->SetHandleSell(pHandleSell);
			pHandleSendAcc = new ZShopEquipItemHandle_SendAccountMatch(pWrappedItem);
			pHandleSendAcc->SetItemUID(uidItem);
			pWrappedItem->SetHandleSendAccount(pHandleSendAcc);
			pMyListBox->Add(new ZShopEquipListItem(pWrappedItem));

			pWrappedItem = new ZShopEquipItem_Match(pItemDesc);
			pHandleSell = new ZShopEquipItemHandle_SellMatch(pWrappedItem);
			pHandleSell->SetItemUID(uidItem);
			pWrappedItem->SetHandleSell(pHandleSell);
			pHandleSendAcc = new ZShopEquipItemHandle_SendAccountMatch(pWrappedItem);
			pHandleSendAcc->SetItemUID(uidItem);
			pWrappedItem->SetHandleSendAccount(pHandleSendAcc);
			pSellListBox->Add(new ZShopEquipListItem(pWrappedItem));

			m_ItemIndexVectorEquip.push_back((*itor).first);
		}
	}
}

void ZMyItemList::SerializeQuest()
{
	if (m_ListFilter != zequip_item_filter_all && m_ListFilter != zequip_item_filter_quest) return;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if (NULL == pResource) return;

	MMultiColListBox* pMyListBox = (MMultiColListBox*)pResource->FindWidget("EquipmentList");
	if (NULL == pMyListBox) return;

	MMultiColListBox* pSellListBox = (MMultiColListBox*)pResource->FindWidget("MyAllEquipmentList");
	if (NULL == pSellListBox) return;

	ZShopEquipItem_Quest* pWrappedItem;
	ZShopEquipItemHandle_SellQuest* pHandleSell;

	for (MQUESTITEMNODEMAP::iterator itor = m_QuestItemMap.begin(); itor != m_QuestItemMap.end(); itor++)
	{
		ZMyQuestItemNode* pItemNode = (*itor).second;
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc(pItemNode->GetItemID());
		if (!pItemDesc) continue;
		if (pItemNode->m_nCount <= 0) continue;

		MComboBox* pWidget = (MComboBox*)pResource->FindWidget("Equip_AllEquipmenItemtFilter");
		switch (pWidget->GetSelIndex())
		{
		case 0:
		{
			if (pItemDesc->m_nItemID != 200008 && pItemDesc->m_nItemID != 200018) continue;
		}
			break;
		case 1:
		{
			if (pItemDesc->m_nItemID != 200021 && pItemDesc->m_nItemID != 200008) continue;
		}
			break;
		case 2:
		{
			if (pItemDesc->m_nItemID != 200009 && pItemDesc->m_nItemID != 200018) continue;
		}
			break;
		case 3:
		{
			if (pItemDesc->m_nItemID != 200009 && pItemDesc->m_nItemID != 200021) continue;
		}
			break;
		case 4:
		{
			if (pItemDesc->m_nItemID != 200016 && pItemDesc->m_nItemID != 200017) continue;
		}
			break;
		case 5:
		{
			if (pItemDesc->m_nItemID != 200020 && pItemDesc->m_nItemID != 200037) continue;
		}
			break;
		case 6:
		{
			if (pItemDesc->m_nItemID != 200020 && pItemDesc->m_nItemID != 200037) continue;
		}
			break;
		case 7:
		{
			if (pItemDesc->m_nItemID != 200006 && pItemDesc->m_nItemID != 200019) continue;
		}
			break;
		case 8:
		{
			if (pItemDesc->m_nItemID != 200007 && pItemDesc->m_nItemID != 200019) continue;
		}
			break;
		case 9:
		{
			if (pItemDesc->m_nItemID != 200010 && pItemDesc->m_nItemID != 200028) continue;
		}
			break;
		case 10:
		{
			if (pItemDesc->m_nItemID != 200021 && pItemDesc->m_nItemID != 200022) continue;
		}
			break;
		case 11:
		{
			if (pItemDesc->m_nItemID != 200020 && pItemDesc->m_nItemID != 200026) continue;
		}
			break;
		case 12:
		{
			if (pItemDesc->m_nItemID != 200019 && pItemDesc->m_nItemID != 200027) continue;
		}
			break;
		case 13:
		{
			if (pItemDesc->m_nItemID != 200026 && pItemDesc->m_nItemID != 200027) continue;
		}
			break;
		case 14:
		{
			if (pItemDesc->m_nItemID != 200031 && pItemDesc->m_nItemID != 200032) continue;
		}
			break;
		}
		pWrappedItem = new ZShopEquipItem_Quest(pItemDesc);
		pHandleSell = new ZShopEquipItemHandle_SellQuest(pWrappedItem);
		pWrappedItem->SetHandleSell(pHandleSell);
		pMyListBox->Add(new ZShopEquipListItem(pWrappedItem));

		pWrappedItem = new ZShopEquipItem_Quest(pItemDesc);
		pHandleSell = new ZShopEquipItemHandle_SellQuest(pWrappedItem);
		pWrappedItem->SetHandleSell(pHandleSell);
		pSellListBox->Add(new ZShopEquipListItem(pWrappedItem));
	}
}

void ZMyItemList::SerializeQItemList()
{
	if ( m_ListFilter != zequip_item_filter_all && m_ListFilter != zequip_item_filter_quest ) return;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	if( NULL == pResource ) return;

	MMultiColListBox* pMyListBox = (MMultiColListBox*)pResource->FindWidget("EquipmentList");
	if( NULL == pMyListBox ) return;

	MMultiColListBox* pSellListBox = (MMultiColListBox*)pResource->FindWidget("MyAllEquipmentList");
	if( NULL == pSellListBox ) return;

	ZShopEquipItem_Quest* pWrappedItem;
	ZShopEquipItemHandle_SellQuest* pHandleSell;

	for ( MQUESTITEMNODEMAP::iterator itor = m_QuestItemMap.begin();  itor != m_QuestItemMap.end();  itor++)
	{
		ZMyQuestItemNode* pItemNode = (*itor).second;
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( pItemNode->GetItemID());
		if (!pItemDesc) continue;
		if ( pItemNode->m_nCount <= 0) continue;

		pWrappedItem = new ZShopEquipItem_Quest(pItemDesc);
		pHandleSell = new ZShopEquipItemHandle_SellQuest(pWrappedItem);
		pWrappedItem->SetHandleSell(pHandleSell);
		pMyListBox->Add(new ZShopEquipListItem(pWrappedItem));

		pWrappedItem = new ZShopEquipItem_Quest(pItemDesc);
		pHandleSell = new ZShopEquipItemHandle_SellQuest(pWrappedItem);
		pWrappedItem->SetHandleSell(pHandleSell);
		pSellListBox->Add(new ZShopEquipListItem(pWrappedItem));
	}
}


void ZMyItemList::MakeMyItemUIDList()
{
	m_ItemIndexVector.clear();
	m_ItemIndexVectorEquip.clear();

	// ÀåºñÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀº Á¦¿ÜÇÑ´Ù
	for (MITEMNODEMAP::iterator itor = m_ItemMap.begin(); itor != m_ItemMap.end(); ++itor)
	{
		bool bExist = false;

		for (int i = 0; i < MMCIP_END; i++)
		{
			if (m_uidEquipItems[i] == (*itor).first) 
				bExist = true;
		}

		if (bExist == false) {
			m_ItemIndexVector.push_back( (*itor).first );
		}
	}
}


void ZMyItemList::SerializeAccountItem()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MMultiColListBox* pListBox = (MMultiColListBox*)pResource->FindWidget("AccountItemList");
	if (!pListBox) return;

	pListBox->RemoveAll();
	m_AccountItemVector.clear();

	for (MACCOUNT_ITEMNODEMAP::iterator itor = m_AccountItemMap.begin(); itor != m_AccountItemMap.end(); ++itor)
	{
		int nAIID = (*itor).first;
		ZMyItemNode* pMyItemNode = (*itor).second;

		ZShopEquipItem* pWrappedItem = NULL;

		unsigned long int nItemID = pMyItemNode->GetItemID();
		if (MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID))
		{
			ZShopEquipItem_Match* pMItem = new ZShopEquipItem_Match(pItemDesc);
			ZShopEquipItemHandle_BringAccountMatch* pHandle = new ZShopEquipItemHandle_BringAccountMatch(pMItem);
			pHandle->SetAIID(nAIID);
			pHandle->SetMyItemNode(pMyItemNode);
			pMItem->SetHandleBringAccount(pHandle);
			pWrappedItem = pMItem;
			pListBox->Add(new ZShopEquipListItem(pWrappedItem));
		}
		else
		{
			//_ASSERT(0);
			continue;
		}

		m_AccountItemVector.push_back(nItemID);
	}
}

void ZMyItemList::SetEquipItemInfo(MUID* pEquipItemUID, unsigned long int* pEquipItemID, unsigned long int* pEquipItemCount)
{
	memcpy(m_uidEquipItems,		pEquipItemUID,		sizeof(MUID)*MMCIP_END);
	memcpy(m_nEquipItemID,		pEquipItemID,		sizeof(m_nEquipItemID));
	memcpy(m_nEquipItemCount,	pEquipItemCount,	sizeof(m_nEquipItemCount));	// ÀåºñÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÇ °¹¼öµµ Ç¥½Ã!
}

void ZMyItemList::SetEquipItemsAll(MUID* puidEquipItems)
{
	memcpy(m_uidEquipItems, puidEquipItems, sizeof(m_uidEquipItems));

	for (int i = 0; i < MMCIP_END; i++)
	{
		ZMyItemNode* pItemNode = GetItem(m_uidEquipItems[i]);
		if( pItemNode != NULL ) {
			m_nEquipItemID[i] = pItemNode->GetItemID();
			m_nEquipItemCount[i] = pItemNode->GetItemCount(); // ÀåºñÇÏ°í ÀÖ´Â ¾ÆÀÌÅÛÀÇ °¹¼öµµ Ç¥½Ã!
		} else {
			m_nEquipItemID[i] = 0;
			m_nEquipItemCount[i] = 0;
		}		
	}
}

void ZMyItemList::SetItemsAll(MTD_ItemNode* pItemNodes, const int nItemCount)
{
	ClearItemMap();

	for (int i = 0; i < nItemCount; i++) {
		ZMyItemNode* pNewItemNode = new ZMyItemNode();

		bool bIsRentItem = false;
		if ( pItemNodes[i].nRentMinutePeriodRemainder < RENT_MINUTE_PERIOD_UNLIMITED)
			bIsRentItem = true;	// ±â°£Á¦ ¾ÆÀÌÅÛ

		if (bIsRentItem  == false) {		///< ÀÏ¹Ý ¾ÆÀÌÅÛ			
			pNewItemNode->Create(pItemNodes[i].uidItem, pItemNodes[i].nItemID, pItemNodes[i].nCount);
		} else {							///< ±â°£Á¦ ¾ÆÀÌÅÛ
			pNewItemNode->Create(pItemNodes[i].uidItem, pItemNodes[i].nItemID, pItemNodes[i].nCount, bIsRentItem, pItemNodes[i].nRentMinutePeriodRemainder, pItemNodes[i].iMaxUseHour);
		}


		m_ItemMap.insert(MITEMNODEMAP::value_type(pItemNodes[i].uidItem, pNewItemNode));
	}
}

void ZMyItemList::SetGambleItemAll(MTD_GambleItemNode* pGItemNodes, int nGItemCount)
{
	m_GambleItemMgr.Release();

	for( int i = 0; i < nGItemCount; ++i ) {
		if( !m_GambleItemMgr.CreateGambleItem(pGItemNodes[ i ].uidItem, pGItemNodes[ i ].nItemID, pGItemNodes[ i ].nItemCnt) ) {
			//_ASSERT( 0 );
		}
	}
}

MUID ZMyItemList::GetForceItemUID(int nItemIndex)
{
	if ((nItemIndex < 0) || (nItemIndex >= (int)m_ItemIndexVector.size())) return MUID(0,0);

	return m_ItemIndexVector[nItemIndex];
}

MUID ZMyItemList::GetItemUID(int nItemIndex)
{
	if ((nItemIndex < 0) || (nItemIndex >= (int)m_ItemIndexVector.size())) return MUID(0,0);

	return m_ItemIndexVector[nItemIndex];
}

MUID ZMyItemList::GetItemUIDEquip(int nItemIndex)
{
	if ((nItemIndex < 0) || (nItemIndex >= (int)m_ItemIndexVectorEquip.size())) return MUID(0,0);

	return m_ItemIndexVectorEquip[nItemIndex];
}


unsigned long int ZMyItemList::GetAccountItemID(int nPos)
{
	int nSIze = (int)m_AccountItemVector.size();

	if(nPos < 0 || nPos >= nSIze)
		return 0;

	return m_AccountItemVector[nPos];
}

unsigned long int ZMyItemList::GetItemID(const MUID& uidItem)
{
	ZMyItemNode* pItemNode = GetItem(uidItem);
	if (pItemNode == NULL) return 0;
	return pItemNode->GetItemID();

}

int ZMyItemList::GetEquipedTotalWeight()
{
	MMatchItemDesc* pItemDesc = NULL;
	int nTotalWeight = 0;

	for (int i=0; i < MMCIP_END; i++)
	{
		if (m_nEquipItemID[i] != 0)
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nEquipItemID[i]);
			if (pItemDesc)
			{
				nTotalWeight += pItemDesc->m_nWeight.Ref();
			}
		}
	}
	return nTotalWeight;
}

int ZMyItemList::GetEquipedHPModifier()
{
	MMatchItemDesc* pItemDesc = NULL;
	int nTotalHPModifier = 0;

	for (int i=0; i < MMCIP_END; i++)
	{
		if (m_nEquipItemID[i] != 0)
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nEquipItemID[i]);
			if (pItemDesc)
			{

				nTotalHPModifier += pItemDesc->m_nHP.Ref();
			}
		}
	}
	return nTotalHPModifier;
}

int ZMyItemList::GetEquipedAPModifier()
{
	MMatchItemDesc* pItemDesc = NULL;
	int nTotalAPModifier = 0;

	for (int i=0; i < MMCIP_END; i++)
	{
		if (m_nEquipItemID[i] != 0)
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nEquipItemID[i]);
			if (pItemDesc)
			{
				nTotalAPModifier += pItemDesc->m_nAP.Ref();
			}
		}
	}
	return nTotalAPModifier;

}

int ZMyItemList::GetMaxWeight()
{
	MMatchItemDesc* pItemDesc = NULL;
	int nMaxWT = MAX_ITEM_COUNT;

	for (int i=0; i < MMCIP_END; i++)
	{
		if (m_nEquipItemID[i] != 0)
		{
			pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nEquipItemID[i]);
			if (pItemDesc)
			{
				nMaxWT += pItemDesc->m_nMaxWT.Ref();
			}
		}
	}
	return nMaxWT;
}


void ZMyItemList::AddAccountItem(int nAIID, unsigned long int nItemID, int nCount, int nRentMinutePeriodRemainder)
{
	bool bIsRentItem = false;
	if (nRentMinutePeriodRemainder < RENT_MINUTE_PERIOD_UNLIMITED) bIsRentItem = true;	// ±â°£Á¦ ¾ÆÀÌÅÛ

	ZMyItemNode* pItemNode = new ZMyItemNode;
	pItemNode->Create(nItemID, nCount, bIsRentItem, nRentMinutePeriodRemainder);

	m_AccountItemMap.insert(MACCOUNT_ITEMNODEMAP::value_type(nAIID, pItemNode));
}


void ZMyItemList::ClearAccountItems()
{
	ClearAccountItemMap();
}

void ZMyItemList::ClearItemMap()
{
	for (MITEMNODEMAP::iterator itor = m_ItemMap.begin(); itor != m_ItemMap.end(); ++itor)
	{
		delete (*itor).second;
	}
	m_ItemMap.clear();
}

void ZMyItemList::ClearAccountItemMap()
{
	for (MACCOUNT_ITEMNODEMAP::iterator itor = m_AccountItemMap.begin(); itor != m_AccountItemMap.end(); ++itor)
	{
		delete (*itor).second;
	}
	m_AccountItemMap.clear();
}



ZMyItemNode* ZMyItemList::GetItem(int nItemIndex)
{
	if ((nItemIndex < 0) || (nItemIndex >= (int)m_ItemIndexVector.size())) return NULL;

	MUID uidItem = m_ItemIndexVector[nItemIndex];

	MITEMNODEMAP::iterator itor = m_ItemMap.find(uidItem);
	if (itor != m_ItemMap.end())
	{
		ZMyItemNode* pItemNode = (*itor).second;
		return pItemNode;
	}

	return NULL;
}

ZMyItemNode* ZMyItemList::GetItemEquip(int nItemIndex)
{
	if ((nItemIndex < 0) || (nItemIndex >= (int)m_ItemIndexVectorEquip.size())) return NULL;

	MUID uidItem = m_ItemIndexVectorEquip[nItemIndex];

	MITEMNODEMAP::iterator itor = m_ItemMap.find(uidItem);
	if (itor != m_ItemMap.end())
	{
		ZMyItemNode* pItemNode = (*itor).second;
		return pItemNode;
	}

	return NULL;
}

ZMyItemNode* ZMyItemList::GetItem(const MUID& uidItem)
{
	MITEMNODEMAP::iterator itor = m_ItemMap.find(uidItem);
	if (itor != m_ItemMap.end())
	{
		ZMyItemNode* pItemNode = (*itor).second;

		return pItemNode;
	}

	return NULL;
}

ZMyItemNode* ZMyItemList::GetEquipedItem(MMatchCharItemParts parts)
{
	MITEMNODEMAP::iterator itor = m_ItemMap.find(m_uidEquipItems[(int)parts]);
	if (itor != m_ItemMap.end())
	{
		ZMyItemNode* pItemNode = (*itor).second;
		return pItemNode;
	}

	return NULL;
}

ZMyItemNode* ZMyItemList::GetAccountItem(int nPos)
{
	int nCnt=0;
	for (MACCOUNT_ITEMNODEMAP::iterator itor = m_AccountItemMap.begin();
		itor != m_AccountItemMap.end(); ++itor)
	{
		if (nPos == nCnt)
		{
			ZMyItemNode* pItemNode = (*itor).second;
			return pItemNode;
		}

		nCnt++;
	}

	return NULL;
}

ZMyItemNode* ZMyItemList::GetAccountItemByAIID( int nAIID )
{
	MACCOUNT_ITEMNODEMAP::iterator itor = m_AccountItemMap.find(nAIID);
	if (m_AccountItemMap.end() == itor) return NULL;
	return itor->second;
}

ZMyItemNode* ZMyItemList::GetItemByDescId(int nItemId)
{
	for (MITEMNODEMAP::iterator itor=m_ItemMap.begin(); itor!=m_ItemMap.end(); ++itor)
	{
		if (nItemId == itor->second->GetItemID())
			return itor->second;
	}
	return NULL;
}

const ZMyGambleItem* ZMyItemList::GetGambleItem(const MUID& uidItem)
{
	return m_GambleItemMgr.GetGambleItem(uidItem);
}

#ifdef _QUEST_ITEM
void ZMyItemList::SetQuestItemsAll( MTD_QuestItemNode* pQuestItemNode, const int nQuestItemCount )
{
	if( 0 == pQuestItemNode)
		return;

	// ÀüÃ¼ ¸®½ºÆ®¸¦ ¾÷µ¥ÀÌÆ® ÇÏ±â À§ÇØ¼­ ÀÌÀüÀÇ µ¥ÀÌÅÍ¸¦ ÃÊ±âÈ­ ÇÔ.
	QuestItemClear();

	for( int i = 0; i < nQuestItemCount; ++i )
	{
		if( !m_QuestItemMap.CreateQuestItem(pQuestItemNode[i].m_nItemID, 
			pQuestItemNode[i].m_nCount, GetQuestItemDescMgr().FindQItemDesc(pQuestItemNode[i].m_nItemID)) )
		{
			// error...
		}
	}
}


#endif


///////////////////////////////////////////////////////////////////////////////////

