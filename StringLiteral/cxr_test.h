/////////////////////////////////////////////////////////////
// cxr_test.h
//
// This file was generated by CXR, the literal string encryptor.
// CXR, Copyright 2002, Smaller Animals Software, Inc., all rights reserved.
//
// Please do not edit this file. Any changes here will be overwritten on the next compile.
// If you wish to make changes to a string, please edit:
//     test.cxr
//

/////////////////////////////////////////////////////////////

#pragma once
#include "cxr_inc.h"

// cxr 적용 테스트입니당

///////////////////////////
#ifdef _USING_CXR
#define STR_CXRTEST1 	_CXR("\x86\x8b\x83\x8a\x86\x80\x86\x89\x88\x8a\x8c\x84\x82\x8b\x80\x88\x85\x84\x80\x8e\x87\x8e\x86\x8f\x89\x84\x88\x8e\x84\x8f\x8e\x8c\x80\x81\x8d\x80\x8f\x81\x87\x8d\x85\x84\x8b\x88\x8a\x81\x85\x85\x8e\x81\x8c\x8c\x8a\x87\x8e\x86\x8b\x87\x86\x82\x87\x8d\x88\x87\x83\x8c")
#else
#define STR_CXRTEST1 _CXR("Your all bases are belong to us.")
#endif

///////////////////////////
#ifdef _USING_CXR
#define STR_CXRTEST2 	_CXR("\x8e\x84\x8d\x84\x80\x8b\x82\x85\x84\x85\x8b\x82\x83\x83\x86\x83\x82\x88\x8a\x8a")// 디파인 뒤에 주석이 달린 경우를 테스트
#else
#define STR_CXRTEST2 _CXR("It's You!")// 디파인 뒤에 주석이 달린 경우를 테스트
#endif

///////////////////////////
#ifdef _USING_CXR
#define STR_CXRTEST3 	_CXR("\x88\x80\x84\x87\x88\x8e\x84\x8f\x8f\x85")		// 디파인 뒤에 공백을 두고 주석이 달린 경우를 테스트
#else
#define STR_CXRTEST3 _CXR("Aye.")		// 디파인 뒤에 공백을 두고 주석이 달린 경우를 테스트
#endif


