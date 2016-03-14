/////////////////////////////////////////////////////////////
// cxr_MMatchBuff.h
//
// This file was generated by CXR, the literal string encryptor.
// CXR, Copyright 2002, Smaller Animals Software, Inc., all rights reserved.
//
// Please do not edit this file. Any changes here will be overwritten on the next compile.
// If you wish to make changes to a string, please edit:
//     MMatchBuff.cxr
//

/////////////////////////////////////////////////////////////

#pragma once
#include "cxr_inc.h"

/*
	== XML 기술 설명 ==
	
	+ zBuff.xml 설명	
	+ attribute 설명
		BuffID			: 버프 아이디						(int)
		BuffName		: 버프 이름							(string[128])
		BuffIconName	: 버프 아이콘 이름					(string[128])
		
		BuffType		: 버프 타입			(MMatchBuffDesc.h내의 MMatchBuffType 참고)
		BuffPeriodType	: 버프 기간 타입	(MMatchBuffDesc.h내의 MMatchBuffPeriodType 참고)
		BuffPeirod		: 버프 기간			(초 단위)
		
		HP	:
		AP	:
		XP	:		
*/

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_BUFF	_CXR("\x86\x8b\x82\x81\x87\x80\x86\x89\x8c\x8d")
#else
#define MICTOK_BUFF														_CXR("BUFF")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ATTRNAME_BUFF_ID	_CXR("\x8e\x84\x8d\x84\x83\x8b")
#else
#define MICTOK_ATTRNAME_BUFF_ID											_CXR("ID")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ATTRNAME_BUFF_ICON_NAME	_CXR("\x88\x80\x84\x8f\x82\x8b\x81\x86\x86\x8d\x88\x8e\x8c\x85\x8b\x80\x85\x82")
#else
#define MICTOK_ATTRNAME_BUFF_ICON_NAME									_CXR("IconName")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ATTRNAME_BUFF_PERIOD_TYPE	_CXR("\x8a\x87\x88\x8d\x8e\x8d\x8c\x80\x85\x8b\x85\x8e\x8f\x82\x86\x8b\x82\x88\x8a\x86\x80\x8e")
#else
#define MICTOK_ATTRNAME_BUFF_PERIOD_TYPE								_CXR("PeriodType")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ATTRNAME_BUFF_PERIOD	_CXR("\x80\x8d\x8e\x85\x81\x85\x84\x80\x81\x81\x81\x8f\x89\x8c")
#else
#define MICTOK_ATTRNAME_BUFF_PERIOD										_CXR("Period")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ATTRNAME_BUFF_EFFECT_TYPE	_CXR("\x8d\x8b\x84\x8e\x8d\x88\x8d\x87\x84\x8a\x83\x81\x82\x86\x86\x83\x87\x8b\x88\x88\x87\x81")
#else
#define MICTOK_ATTRNAME_BUFF_EFFECT_TYPE								_CXR("EffectType")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ATTRNAME_BUFF_HP_VALUE	_CXR("\x8c\x82\x8b\x80\x8e\x82")
#else
#define MICTOK_ATTRNAME_BUFF_HP_VALUE									_CXR("HP")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ATTRNAME_BUFF_AP_VALUE	_CXR("\x82\x8c\x8e\x82\x8d\x80")
#else
#define MICTOK_ATTRNAME_BUFF_AP_VALUE									_CXR("AP")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ATTRNAME_BUFF_RESPAWN_DECREASE_TIME	_CXR("\x85\x8c\x8c\x83\x86\x84\x8d\x8e\x8e\x8f\x86\x84\x8d\x85\x8c\x82")
#else
#define MICTOK_ATTRNAME_BUFF_RESPAWN_DECREASE_TIME						_CXR("Respawn")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ATTRVAL_PERIOD_TYPE_SHORT	_CXR("\x81\x83\x89\x80\x80\x81\x8a\x8c\x8a\x8d\x84\x8b")
#else
#define MICTOK_ATTRVAL_PERIOD_TYPE_SHORT								_CXR("short")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ATTRVAL_PERIOD_TYPE_LONG	_CXR("\x86\x8a\x80\x8c\x85\x87\x87\x86\x8d\x8c")
#else
#define MICTOK_ATTRVAL_PERIOD_TYPE_LONG									_CXR("long")
#endif


///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ATTRVAL_BUFF_EFFECT_TYPE_NORMAL	_CXR("\x80\x80\x8f\x82\x8f\x8a\x89\x85\x8a\x8e\x86\x87\x84\x88")
#else
#define MICTOK_ATTRVAL_BUFF_EFFECT_TYPE_NORMAL							_CXR("normal")
#endif

///////////////////////////
#ifdef _USING_CXR
#define MICTOK_ATTRVAL_BUFF_EFFECT_TYPE_DOTE	_CXR("\x84\x83\x81\x8c\x83\x8e\x8a\x80\x8b\x89")
#else
#define MICTOK_ATTRVAL_BUFF_EFFECT_TYPE_DOTE							_CXR("dote")
#endif

