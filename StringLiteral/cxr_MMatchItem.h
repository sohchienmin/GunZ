/////////////////////////////////////////////////////////////
// cxr_MMatchItem.h
//
// This file was generated by CXR, the literal string encryptor.
// CXR, Copyright 2002, Smaller Animals Software, Inc., all rights reserved.
//
// Please do not edit this file. Any changes here will be overwritten on the next compile.
// If you wish to make changes to a string, please edit:
//     MMatchItem.cxr
//

/////////////////////////////////////////////////////////////

#pragma once
#include "cxr_inc.h"

// type
///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TYPE_MELEE	_CXR("\x86\x8b\x80\x8e\x8c\x8e\x82\x85\x8b\x82\x88\x80")
#else
#define MICTOK_TYPE_MELEE		_CXR("melee")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TYPE_RANGE	_CXR("\x8e\x84\x8e\x8f\x87\x88\x87\x8c\x84\x83\x8d\x8d")
#else
#define MICTOK_TYPE_RANGE		_CXR("range")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TYPE_EQUIP	_CXR("\x88\x80\x86\x83\x8f\x88\x8b\x86\x84\x81\x83\x82")
#else
#define MICTOK_TYPE_EQUIP		_CXR("equip")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TYPE_CUSTOM	_CXR("\x8a\x87\x8b\x8e\x8c\x80\x80\x80\x86\x85\x89\x89\x85\x8c")
#else
#define MICTOK_TYPE_CUSTOM		_CXR("custom")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TYPE_TICKET	_CXR("\x80\x8d\x8c\x81\x85\x8f\x85\x8f\x85\x8e\x88\x83\x80\x8c")
#else
#define MICTOK_TYPE_TICKET		_CXR("ticket")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TYPE_AVATAR	_CXR("\x8d\x8b\x86\x8a\x88\x83\x8f\x84\x8a\x88\x86\x8d\x86\x81")
#else
#define MICTOK_TYPE_AVATAR		_CXR("avatar")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TYPE_COMMUNITY	_CXR("\x8c\x82\x89\x8b\x88\x84\x87\x86\x8e\x8c\x8a\x8e\x80\x8b\x83\x81\x85\x87\x88\x80")
#else
#define MICTOK_TYPE_COMMUNITY	_CXR("community")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TYPE_LONGBUFF	_CXR("\x82\x8c\x8c\x8f\x8e\x80\x8a\x89\x87\x8b\x88\x8b\x88\x86\x8a\x8a\x81\x8d")
#else
#define MICTOK_TYPE_LONGBUFF	_CXR("longbuff")
#endif



// restrict sex
///////////////////////////
#ifdef _USING_CXR
#define MICTOK_RES_SEX_M	_CXR("\x85\x8c\x8f\x8c")
#else
#define MICTOK_RES_SEX_M	_CXR("m")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_RES_SEX_F	_CXR("\x81\x83\x88\x85")
#else
#define MICTOK_RES_SEX_F	_CXR("f")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_RES_SEX_A	_CXR("\x86\x8a\x80\x81")
#else
#define MICTOK_RES_SEX_A	_CXR("a")
#endif


// slot type
///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_NONE	_CXR("\x80\x80\x8f\x82\x8f\x8a\x88\x89\x86\x8d")
#else
#define MICTOK_SLOT_NONE		_CXR("none")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_MELEE	_CXR("\x84\x83\x81\x85\x8e\x88\x85\x8f\x89\x84\x8b\x84")
#else
#define MICTOK_SLOT_MELEE		_CXR("melee")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_RANGE	_CXR("\x8f\x8b\x89\x8d\x88\x81\x83\x8f\x83\x85\x88\x8e")
#else
#define MICTOK_SLOT_RANGE		_CXR("range")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_CUSTOM	_CXR("\x8a\x81\x81\x8d\x8a\x86\x83\x8c\x84\x8b\x87\x83\x87\x8e")
#else
#define MICTOK_SLOT_CUSTOM		_CXR("custom")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_HEAD	_CXR("\x85\x83\x8c\x8f\x8f\x85\x84\x89\x8f\x8c")
#else
#define MICTOK_SLOT_HEAD		_CXR("head")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_CHEST	_CXR("\x81\x8d\x81\x86\x82\x8e\x81\x84\x8b\x8b\x8f\x8d")
#else
#define MICTOK_SLOT_CHEST		_CXR("chest")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_HANDS	_CXR("\x8a\x8e\x8b\x8f\x82\x82\x89\x82\x8b\x84\x8b\x8d")
#else
#define MICTOK_SLOT_HANDS		_CXR("hands")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_LEGS	_CXR("\x88\x80\x86\x8a\x83\x87\x84\x8a\x84\x8b")
#else
#define MICTOK_SLOT_LEGS		_CXR("legs")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_FEET	_CXR("\x88\x8e\x8b\x8b\x87\x83\x8d\x85\x86\x81")
#else
#define MICTOK_SLOT_FEET		_CXR("feet")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_FINGER	_CXR("\x8a\x83\x84\x8e\x8f\x83\x80\x8a\x8d\x8f\x88\x82\x82\x8e")
#else
#define MICTOK_SLOT_FINGER		_CXR("finger")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_EXTRA	_CXR("\x83\x86\x8c\x8c\x87\x88\x80\x87\x85\x80\x8f\x8a")
#else
#define MICTOK_SLOT_EXTRA		_CXR("extra")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_AVATAR	_CXR("\x85\x88\x8e\x80\x8a\x83\x87\x82\x8d\x85\x88\x8e\x8f\x8e")
#else
#define MICTOK_SLOT_AVATAR		_CXR("avatar")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_COMMUNITY	_CXR("\x86\x8d\x83\x81\x8f\x83\x82\x8a\x87\x81\x85\x81\x82\x81\x8f\x88\x8e\x89\x81\x86")
#else
#define MICTOK_SLOT_COMMUNITY	_CXR("community")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT_LONGBUFF	_CXR("\x89\x83\x8b\x83\x8c\x88\x82\x8d\x8e\x8b\x8f\x88\x88\x8f\x80\x8d\x86\x82")
#else
#define MICTOK_SLOT_LONGBUFF	_CXR("longbuff")
#endif


// Spend Item Type
///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SPEND_TYPE_NORMAL	_CXR("\x88\x88\x80\x85\x8b\x8b\x86\x80\x8f\x86\x8f\x88\x85\x81")
#else
#define MICTOK_SPEND_TYPE_NORMAL	_CXR("normal")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SPEND_TYPE_LONGBUFF	_CXR("\x8c\x81\x86\x85\x88\x82\x8d\x8b\x89\x87\x82\x82\x80\x82\x80\x84\x81\x82")
#else
#define MICTOK_SPEND_TYPE_LONGBUFF	_CXR("longbuff")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SPEND_TYPE_SHORTBUFF	_CXR("\x8d\x88\x83\x84\x87\x8f\x8d\x8f\x80\x8b\x86\x8a\x8d\x88\x86\x81\x84\x87\x89\x83")
#else
#define MICTOK_SPEND_TYPE_SHORTBUFF	_CXR("shortbuff")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SPEND_TYPE_COMMUNITY 	_CXR("\x81\x8f\x8b\x8b\x8b\x8a\x87\x8d\x85\x83\x87\x85\x89\x8a\x89\x86\x8a\x8d\x8f\x85")
#else
#define MICTOK_SPEND_TYPE_COMMUNITY _CXR("community")
#endif


// Avatar Item Mesh
///////////////////////////
#ifdef _USING_CXR
#define MICTOK_AVATAR_HEAD	_CXR("\x83\x8a\x87\x8c\x8b\x8d\x84\x88\x8b\x87\x8a\x84\x80\x89\x80\x81\x80\x82\x81\x81\x86\x83\x82\x81")
#else
#define MICTOK_AVATAR_HEAD			_CXR("avatar_head")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_AVATAR_CHEST	_CXR("\x86\x84\x8c\x88\x86\x80\x81\x8a\x87\x84\x87\x87\x81\x88\x80\x8e\x82\x8d\x80\x87\x8f\x8c\x82\x8e\x85\x8c")
#else
#define MICTOK_AVATAR_CHEST			_CXR("avatar_chest")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_AVATAR_HAND	_CXR("\x88\x84\x8a\x82\x88\x83\x8a\x81\x8c\x81\x8b\x8d\x84\x8e\x8e\x88\x8c\x8d\x88\x83\x8c\x83\x82\x80")
#else
#define MICTOK_AVATAR_HAND			_CXR("avatar_hand")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_AVATAR_LEGS	_CXR("\x81\x8b\x88\x80\x82\x8d\x8a\x85\x89\x8e\x89\x86\x89\x8e\x80\x8a\x8e\x86\x81\x82\x80\x8f\x85\x88")
#else
#define MICTOK_AVATAR_LEGS			_CXR("avatar_legs")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_AVATAR_FEET	_CXR("\x80\x85\x80\x86\x86\x8c\x8c\x8e\x8d\x86\x82\x8f\x8b\x8c\x8c\x82\x86\x81\x82\x87\x8f\x8b\x8c\x8d")
#else
#define MICTOK_AVATAR_FEET			_CXR("avatar_feet")
#endif


// weapon type
///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_DAGGER	_CXR("\x88\x88\x80\x8f\x84\x86\x83\x89\x87\x80\x8f\x8e\x8c\x85")
#else
#define MICTOK_WEAPON_DAGGER		_CXR("dagger")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_DUALDAGGER	_CXR("\x84\x8c\x8c\x84\x80\x89\x8a\x8f\x8f\x84\x8f\x8d\x84\x8b\x8b\x8d\x8d\x8b\x87\x8d\x8c\x8f")
#else
#define MICTOK_WEAPON_DUALDAGGER	_CXR("dualdagger")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_KATANA	_CXR("\x8e\x85\x8e\x88\x80\x89\x81\x8f\x88\x86\x82\x80\x8b\x80")
#else
#define MICTOK_WEAPON_KATANA		_CXR("katana")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_GREATSWORD	_CXR("\x8f\x8d\x81\x8b\x8e\x88\x85\x82\x8b\x80\x86\x8b\x85\x8d\x80\x89\x85\x8b\x89\x8b\x8f\x8b")
#else
#define MICTOK_WEAPON_GREATSWORD	_CXR("greatsword")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_DOUBLEKATANA	_CXR("\x85\x8e\x82\x81\x82\x80\x8d\x8c\x8a\x8b\x88\x84\x81\x8a\x8e\x8f\x86\x84\x8d\x8c\x82\x87\x8c\x89\x8c\x86")
#else
#define MICTOK_WEAPON_DOUBLEKATANA	_CXR("doublekatana")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_PISTOL	_CXR("\x84\x8f\x86\x8d\x86\x89\x8a\x8d\x8f\x88\x83\x8e\x85\x8b")
#else
#define MICTOK_WEAPON_PISTOL		_CXR("pistol")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_PISTOLX2	_CXR("\x81\x82\x84\x8a\x8a\x89\x85\x87\x8a\x87\x8e\x8f\x87\x81\x8a\x88\x82\x84")
#else
#define MICTOK_WEAPON_PISTOLX2		_CXR("pistolx2")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_REVOLVER	_CXR("\x89\x81\x81\x8f\x89\x88\x86\x8b\x88\x8b\x80\x83\x8e\x89\x89\x88\x87\x83")
#else
#define MICTOK_WEAPON_REVOLVER		_CXR("revolver")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_REVOLVERX2	_CXR("\x80\x8b\x84\x8b\x88\x81\x81\x88\x8f\x86\x83\x88\x8e\x87\x8b\x8f\x84\x82\x8e\x87\x89\x88")
#else
#define MICTOK_WEAPON_REVOLVERX2	_CXR("revolverx2")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_SMG	_CXR("\x8c\x89\x83\x80\x80\x83\x86\x8f")
#else
#define MICTOK_WEAPON_SMG			_CXR("smg")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_SMGX2	_CXR("\x8d\x8d\x82\x81\x80\x80\x89\x80\x8e\x8a\x81\x8d")
#else
#define MICTOK_WEAPON_SMGX2			_CXR("smgx2")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_SHOTGUN	_CXR("\x84\x84\x8e\x81\x85\x80\x8b\x86\x86\x83\x83\x8d\x8d\x80\x8f\x84")
#else
#define MICTOK_WEAPON_SHOTGUN		_CXR("shotgun")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_SAWEDSHOTGUN	_CXR("\x87\x88\x8c\x8a\x8e\x8a\x8d\x84\x8d\x89\x81\x89\x80\x8b\x87\x8a\x83\x85\x88\x80\x85\x87\x8d\x82\x81\x8c")
#else
#define MICTOK_WEAPON_SAWEDSHOTGUN	_CXR("sawedshotgun")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_RIFLE	_CXR("\x8b\x82\x86\x86\x8a\x8f\x81\x89\x82\x84\x85\x88")
#else
#define MICTOK_WEAPON_RIFLE			_CXR("rifle")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_MACHINEGUN	_CXR("\x85\x80\x87\x82\x8b\x84\x8b\x8f\x82\x8c\x8e\x87\x81\x81\x89\x81\x8f\x87\x8f\x87\x88\x8b")
#else
#define MICTOK_WEAPON_MACHINEGUN	_CXR("machinegun")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_ROCKET	_CXR("\x82\x88\x86\x86\x83\x82\x8b\x8d\x8c\x84\x89\x80\x82\x8d")
#else
#define MICTOK_WEAPON_ROCKET		_CXR("rocket")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_GROCKET	_CXR("\x8d\x8c\x85\x84\x82\x8b\x8e\x8d\x82\x85\x84\x8a\x81\x8a\x8e\x8b")
#else
#define MICTOK_WEAPON_GROCKET		_CXR("grocket")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_FLAMET	_CXR("\x81\x88\x89\x83\x83\x82\x89\x87\x8c\x81\x89\x80\x87\x8e")
#else
#define MICTOK_WEAPON_FLAMET		_CXR("flamet")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_SNIFER	_CXR("\x82\x8e\x82\x8e\x8d\x8d\x8b\x84\x85\x8a\x85\x8a\x8c\x8c")
#else
#define MICTOK_WEAPON_SNIFER		_CXR("snifer")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_MEDKIT	_CXR("\x81\x8e\x88\x86\x81\x87\x83\x80\x89\x82\x8a\x85\x85\x81")
#else
#define MICTOK_WEAPON_MEDKIT		_CXR("medkit")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_REPAIRKIT	_CXR("\x80\x85\x81\x85\x8c\x8c\x84\x86\x86\x86\x8f\x87\x89\x84\x86\x8d\x88\x87\x8d\x8b")
#else
#define MICTOK_WEAPON_REPAIRKIT		_CXR("repairkit")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_BULLETKIT	_CXR("\x83\x8b\x87\x86\x8b\x8e\x8a\x86\x82\x84\x88\x80\x8c\x8f\x80\x83\x8e\x8b\x8d\x8f")
#else
#define MICTOK_WEAPON_BULLETKIT		_CXR("bulletkit")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_FLASHBANG	_CXR("\x8a\x86\x8a\x8f\x8e\x8c\x8d\x81\x89\x89\x8c\x87\x87\x8e\x86\x89\x82\x89\x82\x82")
#else
#define MICTOK_WEAPON_FLASHBANG		_CXR("flashbang")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_FRAG	_CXR("\x8c\x8f\x86\x8c\x8b\x8e\x87\x84\x86\x87")
#else
#define MICTOK_WEAPON_FRAG			_CXR("frag")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_SMOKE	_CXR("\x86\x8a\x81\x83\x87\x82\x89\x81\x86\x85\x84\x8d")
#else
#define MICTOK_WEAPON_SMOKE			_CXR("smoke")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_SKILL	_CXR("\x86\x8c\x8c\x84\x84\x87\x8c\x86\x8c\x88\x83\x8f")
#else
#define MICTOK_WEAPON_SKILL			_CXR("skill")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_FOOD	_CXR("\x8b\x89\x83\x8a\x85\x89\x8b\x8c\x86\x89")
#else
#define MICTOK_WEAPON_FOOD			_CXR("food")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_POTION	_CXR("\x86\x87\x8f\x81\x87\x87\x88\x8d\x86\x8b\x8e\x82\x84\x81")
#else
#define MICTOK_WEAPON_POTION		_CXR("potion")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_TRAP	_CXR("\x8f\x80\x84\x87\x8a\x8d\x87\x8c\x8e\x86")
#else
#define MICTOK_WEAPON_TRAP			_CXR("trap")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_DYNAMITE	_CXR("\x8a\x85\x86\x87\x84\x8d\x8d\x88\x8f\x88\x80\x8a\x84\x8b\x86\x87\x8b\x87")
#else
#define MICTOK_WEAPON_DYNAMITE		_CXR("dynamite")
#endif




// enchant
///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_ENCHANT_FIRE	_CXR("\x86\x86\x89\x8b\x83\x8d\x85\x8d\x8a\x84\x8e\x87\x88\x83\x80\x89\x8a\x8c\x87\x8f\x8b\x86\x82\x89\x80\x85")
#else
#define MICTOK_WEAPON_ENCHANT_FIRE		_CXR("enchant_fire")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_ENCHANT_COLD	_CXR("\x81\x87\x84\x84\x84\x8c\x83\x89\x8a\x85\x84\x81\x85\x8a\x8a\x8c\x87\x81\x85\x83\x85\x85\x85\x81\x8f\x8a")
#else
#define MICTOK_WEAPON_ENCHANT_COLD		_CXR("enchant_cold")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_ENCHANT_LIGHTNING	_CXR("\x87\x8e\x8c\x8a\x81\x8a\x87\x89\x86\x8b\x88\x8f\x86\x8a\x85\x87\x87\x85\x8c\x87\x8a\x80\x84\x85\x82\x8b\x8b\x82\x89\x88\x87\x8a\x89\x88\x82\x8e")
#else
#define MICTOK_WEAPON_ENCHANT_LIGHTNING	_CXR("enchant_lightning")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON_ENCHANT_POISION	_CXR("\x8e\x88\x80\x80\x8a\x85\x8a\x85\x84\x8a\x85\x8c\x80\x82\x8a\x8f\x8f\x81\x82\x88\x87\x89\x88\x8f\x8f\x89\x8f\x8b\x8d\x83")
#else
#define MICTOK_WEAPON_ENCHANT_POISION	_CXR("enchant_poison")
#endif



// true false
///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TRUE	_CXR("\x89\x8a\x8b\x8b\x80\x84\x83\x81\x80\x8a")
#else
#define MICTOK_TRUE			_CXR("true")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_FALSE	_CXR("\x84\x88\x85\x8b\x8d\x87\x80\x8f\x85\x83\x81\x82")
#else
#define MICTOK_FALSE		_CXR("false")
#endif


// ticket type
///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TICKET_TYPE_ADMISSION	_CXR("\x84\x82\x82\x88\x82\x8f\x8a\x82\x81\x8e\x87\x8c\x8c\x85\x80\x82\x81\x8d\x89\x87")
#else
#define MICTOK_TICKET_TYPE_ADMISSION	_CXR("admission")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TICKET_TYPE_CHANGEHEAD	_CXR("\x85\x87\x88\x80\x8d\x85\x8f\x88\x86\x8b\x8c\x8d\x88\x8d\x83\x89\x81\x83\x87\x84\x8f\x8a\x80\x8b")
#else
#define MICTOK_TICKET_TYPE_CHANGEHEAD	_CXR("change_head")
#endif



// Damage Type
///////////////////////////
#ifdef _USING_CXR
#define MICTOK_DAMAGE_TYPE_FIRE	_CXR("\x8d\x89\x88\x8c\x82\x89\x83\x83\x8e\x84")
#else
#define MICTOK_DAMAGE_TYPE_FIRE			_CXR("fire")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_DAMAGE_TYPE_COLD	_CXR("\x89\x8e\x8d\x88\x82\x82\x87\x8c\x84\x8a")
#else
#define MICTOK_DAMAGE_TYPE_COLD			_CXR("cold")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_DAMAGE_TYPE_POISION	_CXR("\x8c\x80\x88\x85\x87\x84\x83\x85\x8f\x85\x89\x83\x80\x8a\x89\x84")
#else
#define MICTOK_DAMAGE_TYPE_POISION		_CXR("poision")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_DAMAGE_TYPE_LIGHTNING	_CXR("\x81\x87\x84\x8d\x8d\x8c\x89\x87\x87\x85\x80\x81\x8a\x8b\x8e\x80\x80\x82\x84\x85")
#else
#define MICTOK_DAMAGE_TYPE_LIGHTNING	_CXR("lightning")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_DAMAGE_TYPE_HEAL	_CXR("\x8a\x8c\x8e\x88\x84\x8c\x80\x8b\x81\x84")
#else
#define MICTOK_DAMAGE_TYPE_HEAL			_CXR("heal")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_DAMAGE_TYPE_REPAIR	_CXR("\x8b\x83\x88\x8a\x80\x8b\x8a\x85\x8a\x89\x8f\x8b\x8f\x82")
#else
#define MICTOK_DAMAGE_TYPE_REPAIR		_CXR("repair")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_DAMAGE_TYPE_HASTE	_CXR("\x89\x8f\x87\x8c\x86\x80\x87\x80\x82\x8f\x80\x88")
#else
#define MICTOK_DAMAGE_TYPE_HASTE		_CXR("haste")
#endif



// effect
///////////////////////////
#ifdef _USING_CXR
#define MECTOK_EFFECT	_CXR("\x8e\x84\x8d\x88\x82\x8f\x88\x84\x8d\x80\x89\x86\x8b\x8a")
#else
#define MECTOK_EFFECT		_CXR("EFFECT")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_ID	_CXR("\x8c\x88\x8f\x81\x84\x8d")
#else
#define MECTOK_ID			_CXR("id")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_NAME	_CXR("\x88\x8f\x89\x88\x82\x8c\x84\x84\x81\x8c")
#else
#define MECTOK_NAME			_CXR("name")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_AREA	_CXR("\x83\x82\x80\x80\x80\x86\x8d\x8b\x8b\x88")
#else
#define MECTOK_AREA			_CXR("area")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_TIME	_CXR("\x87\x83\x83\x8f\x8e\x87\x88\x88\x8d\x88")
#else
#define MECTOK_TIME			_CXR("time")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_MOD_HP	_CXR("\x80\x83\x84\x8e\x85\x88\x8f\x8b\x83\x83\x8d\x80\x89\x81")
#else
#define MECTOK_MOD_HP		_CXR("mod_hp")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_MOD_AP	_CXR("\x8e\x89\x8f\x8f\x85\x86\x88\x82\x87\x8b\x8c\x82\x89\x82")
#else
#define MECTOK_MOD_AP		_CXR("mod_ap")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_MOD_MAXWT	_CXR("\x8c\x87\x8c\x8a\x8e\x80\x86\x8a\x8d\x85\x8b\x87\x85\x83\x8f\x88\x8d\x83\x88\x80")
#else
#define MECTOK_MOD_MAXWT	_CXR("mod_maxwt")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_MOD_SF	_CXR("\x88\x8b\x80\x8a\x8f\x81\x8c\x8e\x86\x86\x86\x81\x86\x8c")
#else
#define MECTOK_MOD_SF		_CXR("mod_sf")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_MOD_FR	_CXR("\x8f\x86\x8f\x8f\x82\x82\x8e\x88\x89\x83\x88\x82\x81\x80")
#else
#define MECTOK_MOD_FR		_CXR("mod_fr")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_MOD_CR	_CXR("\x82\x80\x85\x8e\x83\x84\x83\x87\x89\x84\x8d\x84\x86\x8b")
#else
#define MECTOK_MOD_CR		_CXR("mod_cr")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_MOD_PR	_CXR("\x8b\x82\x87\x89\x84\x83\x8f\x88\x83\x83\x82\x81\x82\x88")
#else
#define MECTOK_MOD_PR		_CXR("mod_pr")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_MOD_LR	_CXR("\x8e\x81\x8d\x8c\x84\x84\x88\x86\x89\x88\x85\x82\x8d\x8a")
#else
#define MECTOK_MOD_LR		_CXR("mod_lr")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_RES_AP	_CXR("\x87\x81\x8c\x8a\x87\x86\x89\x84\x85\x8f\x8f\x86\x8d\x8a")
#else
#define MECTOK_RES_AP		_CXR("res_ap")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_RES_FR	_CXR("\x8f\x86\x8e\x80\x8d\x8c\x80\x87\x8f\x8c\x8c\x80\x81\x83")
#else
#define MECTOK_RES_FR		_CXR("res_fr")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_RES_CR	_CXR("\x81\x87\x85\x83\x8d\x81\x89\x87\x86\x83\x8b\x81\x8e\x87")
#else
#define MECTOK_RES_CR		_CXR("res_cr")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_RES_PR	_CXR("\x8b\x86\x85\x84\x83\x89\x83\x89\x81\x85\x83\x8b\x81\x8f")
#else
#define MECTOK_RES_PR		_CXR("res_pr")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_RES_LR	_CXR("\x8a\x83\x85\x8a\x88\x8e\x82\x85\x82\x80\x89\x89\x86\x82")
#else
#define MECTOK_RES_LR		_CXR("res_lr")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_STUN	_CXR("\x81\x81\x89\x86\x8a\x8f\x89\x87\x83\x8b")
#else
#define MECTOK_STUN			_CXR("stun")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_KNOCKBACK	_CXR("\x87\x8a\x8a\x88\x8c\x85\x82\x8a\x86\x8b\x8a\x8e\x80\x8f\x88\x8f\x85\x81\x83\x8a")
#else
#define MECTOK_KNOCKBACK	_CXR("knockback")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_SMOKE	_CXR("\x83\x81\x82\x8b\x83\x8d\x8a\x8a\x88\x85\x8e\x8a")
#else
#define MECTOK_SMOKE		_CXR("smoke")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_FLASH	_CXR("\x80\x84\x84\x8d\x8b\x8c\x8a\x8c\x81\x85\x8d\x8f")
#else
#define MECTOK_FLASH		_CXR("flash")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_TEAR	_CXR("\x83\x82\x81\x85\x8b\x87\x81\x89\x86\x8d")
#else
#define MECTOK_TEAR			_CXR("tear")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MECTOK_FLAME	_CXR("\x80\x87\x81\x89\x8d\x8b\x82\x85\x80\x88\x8a\x8a")
#else
#define MECTOK_FLAME		_CXR("flame")
#endif



// item
///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ITEM	_CXR("\x87\x8d\x8b\x82\x80\x86\x8f\x81\x8e\x80")
#else
#define MICTOK_ITEM				_CXR("ITEM")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ID	_CXR("\x80\x89\x8c\x86\x86\x82")
#else
#define MICTOK_ID				_CXR("id")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_NAME	_CXR("\x83\x82\x80\x8f\x84\x8f\x8b\x85\x88\x84")
#else
#define MICTOK_NAME				_CXR("name")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TOTALPOINT	_CXR("\x8a\x81\x80\x8a\x8d\x8a\x8c\x8e\x8c\x86\x81\x84\x85\x8b\x8f\x8d\x8f\x85\x8c\x80\x8d\x81")
#else
#define MICTOK_TOTALPOINT		_CXR("totalpoint")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TYPE	_CXR("\x8a\x81\x80\x8a\x8c\x8c\x88\x89\x80\x86")
#else
#define MICTOK_TYPE				_CXR("type")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_RES_SEX	_CXR("\x88\x85\x81\x8c\x8b\x8a\x84\x8c\x86\x81\x8e\x84\x8a\x8c\x8e\x8b")
#else
#define MICTOK_RES_SEX			_CXR("res_sex")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_RES_LEVEL	_CXR("\x8c\x80\x88\x87\x8a\x85\x85\x80\x8b\x81\x82\x86\x86\x8c\x86\x80\x83\x8a\x8c\x8c")
#else
#define MICTOK_RES_LEVEL		_CXR("res_level")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLOT	_CXR("\x8a\x8c\x8f\x83\x88\x81\x8f\x8d\x8a\x8e")
#else
#define MICTOK_SLOT				_CXR("slot")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEAPON	_CXR("\x82\x82\x89\x84\x86\x80\x80\x83\x8f\x82\x84\x85\x8e\x89")
#else
#define MICTOK_WEAPON			_CXR("weapon")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SPEND_TYPE	_CXR("\x8e\x80\x8e\x85\x88\x88\x84\x89\x8b\x89\x8f\x87\x85\x83\x80\x8a\x88\x8a\x8b\x84")
#else
#define MICTOK_SPEND_TYPE		_CXR("spendtype")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_EFFECT_LEVEL	_CXR("\x8d\x8b\x86\x8e\x81\x81\x86\x81\x83\x89\x8c\x89\x87\x83\x8c\x85\x8c\x80\x8c\x8f\x8a\x86\x8a\x84\x88\x81")
#else
#define MICTOK_EFFECT_LEVEL		_CXR("effect_level")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_WEIGHT	_CXR("\x89\x8a\x8b\x88\x8f\x89\x8c\x8b\x8b\x89\x8c\x8d\x84\x88")
#else
#define MICTOK_WEIGHT			_CXR("weight")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_BOUNTY_PRICE	_CXR("\x85\x89\x80\x84\x88\x8e\x81\x8a\x81\x86\x8f\x86\x80\x84\x8a\x83\x81\x8f")
#else
#define MICTOK_BOUNTY_PRICE		_CXR("bt_price")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ISCASHITEM	_CXR("\x8f\x89\x82\x83\x81\x85\x82\x89\x83\x85\x83\x8b\x8c\x81\x82\x85\x8f\x8c\x83\x86\x8f\x86")
#else
#define MICTOK_ISCASHITEM		_CXR("iscashitem")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_DELAY	_CXR("\x81\x80\x82\x86\x88\x81\x85\x8f\x8a\x87\x89\x81")
#else
#define MICTOK_DELAY			_CXR("delay")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_EFFECT_ID	_CXR("\x83\x86\x8c\x8c\x86\x86\x85\x81\x82\x8a\x89\x83\x83\x82\x87\x8a\x82\x8d\x8f\x8f")
#else
#define MICTOK_EFFECT_ID		_CXR("effect_id")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_CONTROLLABILITY	_CXR("\x89\x84\x85\x84\x8b\x8c\x8c\x85\x86\x81\x89\x80\x8b\x82\x89\x87\x84\x81\x87\x8f\x83\x87\x8c\x8d\x85\x84")
#else
#define MICTOK_CONTROLLABILITY	_CXR("ctrl_ability")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_MAGAZINE	_CXR("\x8d\x8b\x86\x86\x8d\x8c\x8f\x83\x88\x8d\x85\x8e\x85\x84\x8d\x85\x88\x81")
#else
#define MICTOK_MAGAZINE			_CXR("magazine")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_MAXBULLET	_CXR("\x87\x83\x82\x86\x88\x8f\x82\x8c\x8e\x8a\x8f\x8b\x8b\x89\x87\x8c\x83\x85\x83\x87")
#else
#define MICTOK_MAXBULLET		_CXR("maxbullet")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_RELOADTIME	_CXR("\x8b\x82\x86\x86\x8a\x83\x84\x84\x8c\x87\x8f\x8c\x85\x8f\x8a\x86\x84\x80\x8c\x82\x8d\x89")
#else
#define MICTOK_RELOADTIME		_CXR("reloadtime")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SLUGOUTPUT	_CXR("\x8e\x8e\x8a\x8e\x85\x8e\x87\x87\x8d\x89\x8b\x8a\x82\x83\x8d\x8c\x83\x84\x87\x86\x87\x8d\x81\x8f")
#else
#define MICTOK_SLUGOUTPUT		_CXR("slug_output")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_GADGET_ID	_CXR("\x82\x82\x88\x84\x8b\x82\x80\x80\x85\x8a\x82\x84\x88\x86\x8c\x83\x8c\x8f\x8e\x84")
#else
#define MICTOK_GADGET_ID		_CXR("gadget_id")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_HP	_CXR("\x85\x8f\x8c\x80\x88\x85")
#else
#define MICTOK_HP				_CXR("hp")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_AP	_CXR("\x87\x81\x8d\x89\x83\x8f")
#else
#define MICTOK_AP				_CXR("ap")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_MAXWT	_CXR("\x8e\x8e\x8b\x80\x83\x8e\x8f\x8c\x8d\x8f\x81\x81")
#else
#define MICTOK_MAXWT			_CXR("maxwt")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SF	_CXR("\x82\x8a\x83\x8b\x8a\x8e")
#else
#define MICTOK_SF				_CXR("sf")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_FR	_CXR("\x8e\x8c\x8d\x88\x86\x8d")
#else
#define MICTOK_FR				_CXR("fr")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_CR	_CXR("\x89\x84\x85\x84\x8b\x8a")
#else
#define MICTOK_CR				_CXR("cr")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_PR	_CXR("\x8e\x81\x8c\x81\x8b\x80")
#else
#define MICTOK_PR				_CXR("pr")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_LR	_CXR("\x8f\x8c\x8f\x8a\x87\x86")
#else
#define MICTOK_LR				_CXR("lr")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_LIMITSPEED	_CXR("\x83\x83\x83\x8b\x86\x8f\x87\x84\x86\x81\x8d\x86\x82\x80\x8d\x82\x80\x88\x82\x88\x87\x85")
#else
#define MICTOK_LIMITSPEED		_CXR("limitspeed")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_LIMITJUMP	_CXR("\x84\x86\x81\x86\x89\x8d\x8c\x81\x83\x8e\x85\x8d\x84\x80\x88\x87\x8d\x8e\x87\x86")
#else
#define MICTOK_LIMITJUMP		_CXR("limitjump")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_LIMITTUMBLE	_CXR("\x8f\x87\x8f\x8e\x87\x88\x86\x86\x8e\x8e\x83\x8d\x83\x8d\x8d\x84\x80\x8c\x8f\x84\x87\x8d\x81\x8d")
#else
#define MICTOK_LIMITTUMBLE		_CXR("limittumble")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_LIMITWALL	_CXR("\x8f\x88\x8b\x8a\x80\x82\x81\x84\x8b\x82\x8b\x8f\x8c\x8e\x81\x85\x8f\x86\x82\x81")
#else
#define MICTOK_LIMITWALL		_CXR("limitwall")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_RANGE	_CXR("\x84\x84\x8e\x80\x8c\x8a\x8c\x8b\x85\x80\x80\x8e")
#else
#define MICTOK_RANGE			_CXR("range")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ANGLE	_CXR("\x8c\x87\x8c\x86\x80\x88\x87\x80\x8e\x86\x85\x86")
#else
#define MICTOK_ANGLE			_CXR("angle")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ADDED_HP	_CXR("\x81\x8f\x8b\x89\x88\x8a\x83\x83\x87\x86\x8d\x85\x8d\x82\x8a\x87")
#else
#define MICTOK_ADDED_HP			_CXR("addedHP")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ADDED_AP	_CXR("\x84\x82\x82\x88\x82\x8f\x8a\x8b\x89\x85\x81\x80\x8d\x81\x8d\x8b")
#else
#define MICTOK_ADDED_AP			_CXR("addedAP")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_DAMAGE	_CXR("\x8e\x82\x85\x8e\x81\x80\x82\x8f\x80\x87\x8c\x81\x88\x83")
#else
#define MICTOK_DAMAGE			_CXR("damage")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ITEMPOWER	_CXR("\x83\x89\x8b\x8e\x80\x86\x8e\x83\x80\x8c\x8e\x8c\x8d\x81\x80\x8f\x84\x86\x8d\x8d")
#else
#define MICTOK_ITEMPOWER		_CXR("itempower")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_DAMAGE_TYPE	_CXR("\x8a\x82\x8e\x8f\x83\x8b\x80\x8f\x84\x8e\x86\x89\x8b\x8d\x85\x80\x8a\x86\x89\x81\x81\x82")
#else
#define MICTOK_DAMAGE_TYPE		_CXR("damagetype")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_DAMAGE_TIME	_CXR("\x8e\x8d\x88\x80\x86\x8b\x81\x8a\x88\x8d\x8c\x80\x85\x8f\x89\x83\x8e\x82\x87\x86\x8a\x8b")
#else
#define MICTOK_DAMAGE_TIME		_CXR("damagetime")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_LIFETIME	_CXR("\x80\x84\x84\x87\x82\x8f\x89\x8d\x83\x82\x80\x81\x8e\x8c\x88\x87\x84\x87")
#else
#define MICTOK_LIFETIME			_CXR("lifetime")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_COLOR	_CXR("\x82\x8c\x8c\x80\x87\x82\x8a\x80\x8c\x82\x8f\x86")
#else
#define MICTOK_COLOR			_CXR("color")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_DESC	_CXR("\x88\x81\x89\x8e\x81\x8b\x80\x80\x81\x84")
#else
#define MICTOK_DESC				_CXR("desc")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_MESH_NAME	_CXR("\x8e\x89\x8f\x8f\x85\x8c\x8c\x8e\x88\x8c\x82\x8a\x82\x83\x89\x8b\x8b\x85\x8d\x89")
#else
#define MICTOK_MESH_NAME				_CXR("mesh_name")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_IMAGE_ID	_CXR("\x80\x8b\x85\x80\x84\x88\x8f\x8a\x82\x83\x84\x8a\x8d\x82\x89\x8d\x87\x82")
#else
#define MICTOK_IMAGE_ID					_CXR("image_id")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_BULLET_IMAGE_ID	_CXR("\x8e\x8a\x83\x84\x82\x8a\x88\x86\x84\x8e\x80\x8b\x89\x80\x81\x82\x8a\x8f\x83\x83\x85\x87\x83\x86\x8d\x85\x80\x8e\x83\x8b\x8c\x87")
#else
#define MICTOK_BULLET_IMAGE_ID			_CXR("bullet_image_id")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_MAGAZINE_IMAGE_ID	_CXR("\x88\x8e\x8b\x80\x84\x84\x83\x8f\x85\x80\x8e\x8c\x88\x82\x81\x87\x8d\x81\x8b\x8c\x8d\x88\x86\x8b\x8f\x87\x89\x89\x85\x83\x8e\x87\x82\x8b\x88\x8d")
#else
#define MICTOK_MAGAZINE_IMAGE_ID		_CXR("magazine_image_id")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SOUND_RELOAD	_CXR("\x8a\x8c\x8f\x83\x88\x83\x8b\x8d\x85\x86\x8f\x80\x83\x81\x82\x8d\x82\x84\x80\x8d\x88\x83")
#else
#define MICTOK_SOUND_RELOAD				_CXR("snd_reload")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SOUND_FIRE	_CXR("\x8c\x82\x88\x8b\x8b\x85\x84\x83\x8d\x8f\x84\x8f\x8c\x89\x83\x86\x87\x8e")
#else
#define MICTOK_SOUND_FIRE				_CXR("snd_fire")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SOUND_DRYFIRE	_CXR("\x88\x82\x89\x8e\x84\x82\x8b\x8d\x8e\x8f\x83\x89\x83\x85\x88\x8e\x82\x82\x86\x87\x88\x8b\x84\x82")
#else
#define MICTOK_SOUND_DRYFIRE			_CXR("snd_dryfire")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_SOUND_WEAPON_FIBER	_CXR("\x84\x8b\x87\x84\x8b\x80\x87\x80\x8d\x88\x8a\x83\x8f\x81\x82\x89\x82\x86\x85\x86\x80\x85\x87\x8c\x8b\x8e\x8e\x8c\x89\x8a\x8c\x80\x8b\x88")
#else
#define MICTOK_SOUND_WEAPON_FIBER		_CXR("snd_weapon_fiber")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_BONUS_XP_SOLO	_CXR("\x8a\x81\x80\x86\x80\x8c\x8d\x8c\x8e\x8f\x88\x83\x8b\x87\x8e\x89\x81\x80\x83\x8a\x82\x80\x85\x85\x85\x8a\x80\x89")
#else
#define MICTOK_BONUS_XP_SOLO			_CXR("xp_solo_bonus")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_BONUS_XP_TEAM	_CXR("\x81\x89\x81\x89\x83\x84\x8a\x8c\x81\x88\x8d\x80\x85\x84\x81\x81\x89\x8d\x84\x86\x83\x8a\x8b\x81\x88\x8b\x86\x8a")
#else
#define MICTOK_BONUS_XP_TEAM			_CXR("xp_team_bonus")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_BONUS_XP_QUEST	_CXR("\x89\x86\x85\x84\x8c\x87\x81\x89\x87\x85\x80\x8e\x89\x82\x8b\x8d\x8c\x8f\x84\x89\x83\x80\x88\x8f\x8c\x86\x84\x84\x8c\x85")
#else
#define MICTOK_BONUS_XP_QUEST			_CXR("xp_quest_bonus")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_BONUS_BP_SOLO	_CXR("\x88\x85\x80\x8c\x88\x8d\x88\x88\x82\x8b\x83\x89\x8c\x82\x80\x80\x8d\x82\x84\x89\x87\x8a\x88\x82\x89\x80\x8a\x8e")
#else
#define MICTOK_BONUS_BP_SOLO			_CXR("bp_solo_bonus")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_BONUS_BP_TEAM	_CXR("\x85\x84\x8b\x8c\x8a\x82\x85\x84\x87\x84\x86\x86\x89\x80\x8b\x87\x81\x81\x82\x83\x8f\x8d\x8d\x85\x8c\x83\x83\x82")
#else
#define MICTOK_BONUS_BP_TEAM			_CXR("bp_team_bonus")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_BONUS_BP_QUEST	_CXR("\x8a\x80\x83\x8d\x84\x80\x88\x8b\x8a\x8c\x86\x8f\x8c\x81\x8a\x81\x84\x8e\x82\x83\x8f\x85\x8a\x85\x87\x8b\x83\x8b\x8f\x86")
#else
#define MICTOK_BONUS_BP_QUEST			_CXR("bp_quest_bonus")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_BONUS_SPENDABLE	_CXR("\x8e\x84\x8e\x8e\x88\x8f\x83\x8a\x8f\x80\x84\x8f\x86\x86\x8f\x84\x87\x89\x8b\x84")
#else
#define MICTOK_BONUS_SPENDABLE			_CXR("spendable")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_TICKET_TYPE	_CXR("\x8a\x8f\x8e\x80\x87\x8b\x8f\x8f\x85\x89\x88\x8b\x89\x8c\x88\x8f\x89\x84\x88\x89\x85\x87\x8b\x82")
#else
#define MICTOK_TICKET_TYPE				_CXR("ticket_type")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_MAX_RENT_PERIOD	_CXR("\x85\x8a\x8f\x88\x81\x84\x8f\x8e\x8a\x87\x86\x81\x8d\x87\x85\x88\x8d\x8b\x87\x86\x8a\x8c\x8a\x8e")
#else
#define MICTOK_MAX_RENT_PERIOD			_CXR("rent_period")
#endif


/*
== XML 기술 설명 ==
+ zitem.xml 설명

+ 태그이름 : ITEM
+ attribute 설명
id : 아이템 아이디(int)
name : 아이템 이름(string[128])
mesh_name : 메쉬 이름(str[256]) , weapon.xml의 name항목의 이름
totalpoint : 종합 점수(int)
type : 종류(melee, range, equip, custom)
res_sex : 성별 제한사항(m, f, a)
res_level : 레벨 제한사항(int) , 제한이 없을경우 0으로 세팅하도록 한다.
slot : 슬롯(none, melee, range, head, chest, hands, legs, feet, finger, custom)
weapon : 무기타입(none, dagger, katana, pistol, smg, shotgun, rifle, machinegun, 
                  rocket, grocket, flamet, snifer, medkit, flashbang, frag, smoke, 
				  enchant_fire, enchant_cold, enchant_lightning, enchant_poison)
weight : 무게(int)
bt_price : 판매가(int)
iscashitem : 캐쉬아이템인지 여부(true, false)
damage : 데미지(int)
delay : 딜레이(int)
effect_id : 이펙트 아이디(int) , effect.xml의 항목과 연동되는 ID
ctrl_ability : 제어성(int)
magazine : 장탄수(int)
reloadtime : 장전시간(int)
slug_output : 탄피적출(true, false)
gadget_id : 옵션 아이디(int)
hp : hp 캐릭터 수정치(int)
ap : ap 캐릭터 수정치(int)
maxwt : max wt 캐릭터 수정치(int)
sf : sf 캐릭터 수정치(int)
fr : fr 캐릭터 수정치(int)
cr : cr 캐릭터 수정치(int)
pr : pr 캐릭터 수정치(int)
lr : lr 캐릭터 수정치(int)
desc : 설명(string[65535])
color : 색깔(color)
image_id : ui에서 보이는 image 아이디(int) , 지금은 쓰이지 않음
bullet_image_id : ui에서 보이는 bullet 이미지 아이디(int) , 지금은 쓰이지 않음
magazine_image_id : ui에서 보이는 magazine 이미지 아이디(int) , 지금은 쓰이지 않음
snd_reload : reload때 나오는 사운드 이름(str[128])
snd_fire : fire때 나오는 사운드 이름(str[128])
snd_dryfire : dryfire 때 나오는 사운드 이름(str[128])
snd_weapon_fiber : 무기의 재질(쇠, 고무 등)의 사운드(str[128])
range : melee 공격의 범위(int)
rent_period : 기간제 바운티 아이템의 최대 기간 정보 (날짜단위)(int)

+ effect.xml 설명

- 태그이름 : EFFECT
+ attribute 설명
id : 이펙트 아이디(int)
name : 이펙트 이름(string[128])
area : 이펙트 영역(int)
time : 이펙트 시간(msec)
mod_hp : hp 수정치 
mod_ap : ap 수정치
mod_maxwt : max wt 수정치
mod_sf : sf 수정치
mod_fr : fr 수정치
mod_cr : cr 수정치
mod_pr : pr 수정치
mod_lr : lr 수정치
res_ap : ap 저항판정
res_fr : fr 저항판정
res_cr : cr 저항판정
res_pr : pr 저항판정
res_lr : lr 저항판정
stun : stun 수정치
knockback : knockback 수정치
smoke : smoke 수정치
flash : flash 수정치
tear : tear 수정치
flame : frame 수정치

duplicate : 중복 착용 여부.(true, false)
 */
