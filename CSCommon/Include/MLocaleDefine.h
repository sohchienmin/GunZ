#ifndef _MLOCALEDEFINE_H
#define _MLOCALEDEFINE_H

// 사용하는 지역 디파인 /////////////////////////////////////////////////////////////////
//
// LOCALE_US		: 인터네셔날
// LOCALE_JAPAN		: 일본
// LOCALE_KOREA		: 한국
// LOCALE_BRAZIL	: 브라질
// LOCALE_INDIA		: 인도
//
/////////////////////////////////////////////////////////////////////////////////////////
// 지역에 따른 define ///////////////////////////////////////////////////////////////////
#if (!defined(LOCALE_US)) && (!defined(LOCALE_JAPAN)) && (!defined(LOCALE_KOREA)) && (!defined(LOCALE_BRAZIL)) && (!defined(LOCALE_INDIA)) && (!defined(LOCALE_NHNUSA))

#define LOCALE_KOREA		// 아무 디파인도 없으면 기본값은 한국
//MONCKEY100 was here to set locale
#endif // (!defined(LOCALE_US)) && (!defined(LOCALE_JAPAN)) && (!defined(LOCALE_KOREA))
/////////////////////////////////////////////////////////////////////////////////////////



#endif