#include "stdafx.h"

#include "ZShop.h"
#include "ZPost.h"
#include "ZGameClient.h"
#include "MUID.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MMultiColListBox.h"
#include "MLabel.h"
#include "ZEquipmentListBox.h"
#include "ZNetRepository.h"
#include "ZShopEquipItem.h"
#include "ZShopEquipListbox.h"

enum {
	zshop_item_filter_all = 0,

	zshop_item_filter_head,
	zshop_item_filter_chest,
	zshop_item_filter_hands,
	zshop_item_filter_legs,
	zshop_item_filter_feet,
	zshop_item_filter_extra,
#ifdef _AVATAR_ENABLE
	zshop_item_filter_avatar,
#endif
	zshop_item_filter_melee,
	zshop_item_filter_range,
	zshop_item_filter_custom,
	zshop_item_filter_quest,
	zshop_item_filter_enchant,
	zshop_item_filter_dance,
};


ZShop::ZShop() : m_nPage(0), m_bCreated(false)
{
	m_ListFilter = zshop_item_filter_all;
}

ZShop::~ZShop()
{
	ClearShop();
	ClearGamble();
}

bool ZShop::Create()
{
	if (m_bCreated) return false;

	// 서버에 Shop Item 리스트 달라고 요청
	ZPostRequestShopItemList(ZGetGameClient()->GetPlayerUID(), 0, 0);	// 0개면 전체를 달라고 요청하는것이다
	ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());

	m_bCreated = true;
	return true;
}

void ZShop::Destroy()
{
	if (!m_bCreated) return;

	m_bCreated = false;
}

ZShop* ZShop::GetInstance()
{
	static ZShop m_stShop;
	return &m_stShop;
}

bool ZShop::CheckTypeWithListFilter(int type, bool bEnchantItem)
{
	MMatchWeaponType wtype = MWT_NONE;
	ZIDLResource* pResource = ZGetGameInterface()->GetIDLResource();
	switch (m_ListFilter)
	{
	case zshop_item_filter_all:
	{
		return true;
	}
		break;
	case zshop_item_filter_head:
	{
		if (type == MMIST_HEAD)
		{
			return true;
		}
	}
		break;
	case zshop_item_filter_chest:
	{
		if (type == MMIST_CHEST)
		{
			return true;
		}
	}
		break;
	case zshop_item_filter_hands:
	{
		if (type == MMIST_HANDS)
		{
			return true;
		}
	}
		break;
	case zshop_item_filter_legs:
	{
		if (type == MMIST_LEGS)
		{
			return true;
		}
	}
		break;
	case zshop_item_filter_feet:
	{
		if (type == MMIST_FEET)
		{
			return true;
		}
	}
		break;
	case zshop_item_filter_extra:
	{
		if (type == MMIST_EXTRA || type == MMIST_FINGER)
		{
			return true;
		}
	}
		break;
#ifdef _AVATAR_ENABLE
	case zshop_item_filter_avatar:
	{
		if (type == MMIST_AVATAR)
		{
			return true;
		}
	}
		break;
#endif

	case zshop_item_filter_melee:
	{
		if (type == MMIST_MELEE)
		{
			return true;
		}
	}
		break;
	case zshop_item_filter_range:
	{
		if (type == MMIST_RANGE)
		{
			return true;
		}
	}
		break;
	case zshop_item_filter_custom:
	{
		if (type == MMIST_CUSTOM)
		{
			return true;
		}
	}
		break;
	case zshop_item_filter_enchant:
	{
		if (type == MMIST_CUSTOM)
		{
			if (bEnchantItem)
				return true;
		}
	}
		break;
#ifdef _RGGunz
		case zshop_item_filter_dance:
		{
			if (type == MMIST_DANCE || type == MMIST_DASH)
			{
				return true;
			}
		}
		break;
#endif
	default:
	{
		return true;
	}
	}
	return false;
}
void ZShop::SerializeQuest()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MMultiColListBox* pListBox = (MMultiColListBox*)pResource->FindWidget("AllEquipmentList");
	if (!pListBox) { return; }

	pListBox->RemoveAll();

	for (int i = 0; i<GetItemCount(); i++)
	{
		ZShopEquipItem* pWrappedItem = NULL;

		if (MQuestItemDesc* pDesc = GetQuestItemDescMgr().FindQItemDesc(m_vShopItem[i]->nItemID))
		{
			if (m_ListFilter != zshop_item_filter_all && m_ListFilter != zshop_item_filter_quest) continue;
			MComboBox* pWidget = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentItemFilter");
			switch (pWidget->GetSelIndex())
			{
			case 1:
			{
				if (pDesc->m_nItemID != 200008 && pDesc->m_nItemID != 200018) continue;
			}
				break;
			case 2:
			{
				if (pDesc->m_nItemID != 200021 && pDesc->m_nItemID != 200008) continue;
			}
				break;
			case 3:
			{
				if (pDesc->m_nItemID != 200009 && pDesc->m_nItemID != 200018) continue;
			}
				break;
			case 4:
			{
				if (pDesc->m_nItemID != 200009 && pDesc->m_nItemID != 200021) continue;
			}
				break;
			case 5:
			{
				if (pDesc->m_nItemID != 200016 && pDesc->m_nItemID != 200017) continue;
			}
				break;
			case 6:
			{
				if (pDesc->m_nItemID != 200020 && pDesc->m_nItemID != 200037) continue;
			}
				break;
			case 7:
			{
				if (pDesc->m_nItemID != 200020 && pDesc->m_nItemID != 200037) continue;
			}
				break;
			case 8:
			{
				if (pDesc->m_nItemID != 200006 && pDesc->m_nItemID != 200019) continue;
			}
				break;
			case 9:
			{
				if (pDesc->m_nItemID != 200007 && pDesc->m_nItemID != 200019) continue;
			}
				break;
			case 10:
			{
				if (pDesc->m_nItemID != 200010 && pDesc->m_nItemID != 200028) continue;
			}
				break;
			case 11:
			{
				if (pDesc->m_nItemID != 200021 && pDesc->m_nItemID != 200022) continue;
			}
				break;
			case 12:
			{
				if (pDesc->m_nItemID != 200020 && pDesc->m_nItemID != 200026) continue;
			}
				break;
			case 13:
			{
				if (pDesc->m_nItemID != 200019 && pDesc->m_nItemID != 200027) continue;
			}
				break;
			case 14:
			{
				if (pDesc->m_nItemID != 200026 && pDesc->m_nItemID != 200027) continue;
			}
				break;
			case 15:
			{
				if (pDesc->m_nItemID != 200031 && pDesc->m_nItemID != 200032) continue;
			}
				break;
			}
			MUID uidItem = MUID(0, i + 1);
			ZShopEquipItem_Quest* pQItem = new ZShopEquipItem_Quest(pDesc);
			ZShopEquipItemHandle_PurchaseQuest* pHandle =
				new ZShopEquipItemHandle_PurchaseQuest(pQItem);
			pWrappedItem = pQItem;
			pWrappedItem->SetHandlePurchase(pHandle);
		}

		if (!pWrappedItem) { continue; }

		ZShopEquipListItem* pListItem = new ZShopEquipListItem(pWrappedItem);
		pListBox->Add(pListItem);
	}
}

void ZShop::SerializeItems(int nIndex, int nWeaponType, int nSlot)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MMultiColListBox* pListBox = (MMultiColListBox*)pResource->FindWidget("AllEquipmentList");
	MComboBox* pWidget = (MComboBox*)pResource->FindWidget("Shop_AllEquipmentItemFilter");
	MComboBox* pAll = (MComboBox*)pResource->FindWidget("Shop_AllFilter");
	if (!pListBox) { return; }
	pListBox->RemoveAll();
	for (int i = 0; i<GetItemCount(); i++)
	{
		ZShopEquipItem* pWrappedItem = NULL;

		if (MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(m_vShopItem[i]->nItemID))
		{
			MMatchWeaponType wtype = pDesc->m_nWeaponType.Ref();
			if (pDesc->m_nResSex.Ref() != -1 && pDesc->m_nResSex.Ref() != int(ZGetMyInfo()->GetSex())) continue;

			if (pAll->GetSelIndex() == 0)
			{
			}
			else if (pAll->GetSelIndex() == 1)
			{
				if (pDesc->m_bIsCashItem) continue;
				if (pDesc->m_bIsEventItem) continue;
			}

			else if (pAll->GetSelIndex() == 2)
			{
				if (!pDesc->m_bIsCashItem) continue;
			}
			else if (pAll->GetSelIndex() == 3)
			{
				if (!pDesc->m_bIsEventItem) continue;
			}

			if (pWidget->GetSelIndex() == nIndex)
			{
				if (nIndex == 0)
				{
					if (pDesc->m_nSlot != nSlot)
						continue;
				}
				else
				{
					if ((pDesc->m_nWeaponType.Ref() != nWeaponType))
						continue;
				}
			}



			ZShopEquipItem_Match* pMItem = new ZShopEquipItem_Match(pDesc);
			ZShopEquipItemHandle_PurchaseMatch* pHandle =
				new ZShopEquipItemHandle_PurchaseMatch(pMItem);
			pWrappedItem = pMItem;
			pWrappedItem->SetHandlePurchase(pHandle);
		}
		else if (MQuestItemDesc* pDesc = GetQuestItemDescMgr().FindQItemDesc(m_vShopItem[i]->nItemID))
		{
			if (m_ListFilter != zshop_item_filter_all && m_ListFilter != zshop_item_filter_quest) continue;
			if (pAll->GetSelIndex() != 0) continue;
			MUID uidItem = MUID(0, i + 1);
			ZShopEquipItem_Quest* pQItem = new ZShopEquipItem_Quest(pDesc);
			ZShopEquipItemHandle_PurchaseQuest* pHandle =
				new ZShopEquipItemHandle_PurchaseQuest(pQItem);
			pWrappedItem = pQItem;
			pWrappedItem->SetHandlePurchase(pHandle);
		}
		if (!pWrappedItem) { continue; }

		ZShopEquipListItem* pListItem = new ZShopEquipListItem(pWrappedItem);
		pListBox->Add(pListItem);
	}
}
void ZShop::Serialize(int nType)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MMultiColListBox* pListBox = (MMultiColListBox*)pResource->FindWidget("AllEquipmentList");
	MComboBox* pWidget = (MComboBox*)pResource->FindWidget("Shop_AllFilter");
	if (!pListBox) { return; }
	pListBox->RemoveAll();


	for (int i=0; i<GetItemCount(); i++)
	{
		ZShopEquipItem* pWrappedItem = NULL;

		if (MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(m_vShopItem[i]->nItemID))
		{
			if (nType == 0 || pWidget->GetSelIndex() == 0)
			{
			}
			else if (nType == 1 || pWidget->GetSelIndex() == 1)
			{
				if (pDesc->m_bIsCashItem) continue;
				if (pDesc->m_bIsEventItem) continue;
			}

			else if (nType == 2 || pWidget->GetSelIndex() == 2)
			{
				if (!pDesc->m_bIsCashItem) continue;
			}
			else if (nType == 3 || pWidget->GetSelIndex() == 3)
			{
				if (!pDesc->m_bIsEventItem) continue;
			}	
			
			if (CheckTypeWithListFilter(pDesc->m_nSlot, pDesc->IsEnchantItem()) == false) continue;
			if (pDesc->m_nResSex.Ref() != -1 && pDesc->m_nResSex.Ref() != int(ZGetMyInfo()->GetSex())) continue;

			ZShopEquipItem_Match* pMItem = new ZShopEquipItem_Match(pDesc);
			ZShopEquipItemHandle_PurchaseMatch* pHandle = 
				new ZShopEquipItemHandle_PurchaseMatch(pMItem);
			pWrappedItem = pMItem;
			pWrappedItem->SetHandlePurchase(pHandle);
		}
		else if (MQuestItemDesc* pDesc = GetQuestItemDescMgr().FindQItemDesc( m_vShopItem[i]->nItemID))
		{
			if (nType != 0 || pWidget->GetSelIndex() != 0) continue;
			if (m_ListFilter != zshop_item_filter_all && m_ListFilter != zshop_item_filter_quest) continue;

			MUID uidItem = MUID(0, i+1);
			ZShopEquipItem_Quest* pQItem = new ZShopEquipItem_Quest(pDesc);
			ZShopEquipItemHandle_PurchaseQuest* pHandle = 
				new ZShopEquipItemHandle_PurchaseQuest(pQItem);
			pWrappedItem = pQItem;
			pWrappedItem->SetHandlePurchase(pHandle);
		}

		if (!pWrappedItem) { continue; }

		ZShopEquipListItem* pListItem = new ZShopEquipListItem(pWrappedItem);
		pListBox->Add(pListItem);
	}
}

unsigned long int ZShop::GetItemID(int nIndex)
{
	if ((nIndex < 0) || (nIndex >= (int)m_vShopItem.size())) return 0;
	return m_vShopItem[nIndex]->nItemID;
}

void ZShop::SetItemsAll(const vector< MTD_ShopItemInfo*>& vShopItemList)
{
	ClearShop();

	for ( int i = 0;  i < (int)vShopItemList.size();  i++)
	{
		MTD_ShopItemInfo* pNode = new MTD_ShopItemInfo;
		memcpy( pNode, vShopItemList[i], sizeof(MTD_ShopItemInfo));
		m_vShopItem.push_back(pNode);
	}
}

void ZShop::SetItemsGamble( const vector<MTD_GambleItemNode*>& vGItemList)
{
	ClearGamble();

	if( IsAdminGrade(ZGetMyInfo()->GetUGradeID()) )
		mlog( "Shop Gamble item count : %d\n", (int)vGItemList.size() );

	for ( int i = 0;  i < (int)vGItemList.size();  i++)
	{
		MTD_GambleItemNode* pNode = new MTD_GambleItemNode;
		memcpy( pNode, vGItemList[ i], sizeof( MTD_GambleItemNode));
		m_vGItemList.push_back( pNode);

		if( IsAdminGrade(ZGetMyInfo()->GetUGradeID()) )
		{
			const ZGambleItemDefine* pGItemDef = ZGetGambleItemDefineMgr().GetGambleItemDefine( pNode->nItemID );
			if( NULL != pGItemDef )
			{
				mlog( "shop gitem : %d, %s\n"
					, pGItemDef->GetGambleItemID()
					, pGItemDef->GetName().c_str() );
			}
			else
			{
				mlog ("invaled shop gitem : %d\n"
					, pNode->nItemID );
			}
		}
	}
}

void ZShop::ClearShop()
{
	for ( int i = 0;  i < (int)m_vShopItem.size();  i++)
		delete m_vShopItem[ i];

	m_vShopItem.clear();
}

void ZShop::ClearGamble()
{
	for ( int i = 0;  i < (int)m_vGItemList.size();  i++)
		delete m_vGItemList[ i];

	m_vGItemList.clear();
}
