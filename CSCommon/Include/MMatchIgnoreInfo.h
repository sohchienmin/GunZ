#ifndef _MMATCHIGNOREINFO_H
#define _MMATCHIGNOREINFO_H

//#pragma once


#include <list>
using namespace std;
#include "MMatchGlobal.h"


#define MAX_IGNORE_COUNT	40


// ģ������
struct MMatchIgnoreNode {
	unsigned long	nFriendCID;
	unsigned short	nFavorite;
	char			szName[MATCHOBJECT_NAME_LENGTH];

	unsigned char	nState;										// ��ġ����(�κ�, ������ ���)
	char			szDescription[MATCH_SIMPLE_DESC_LENGTH];	// ��ġ����(�κ��̸� ���)
};
class MMatchIgnoreList : public list<MMatchIgnoreNode*> {};


class MMatchIgnoreInfo {
private:
	MCriticalSection	m_csFriendListLock;
public:
	MMatchIgnoreList	m_IgnoreList;
public:
	MMatchIgnoreInfo();
	virtual ~MMatchIgnoreInfo();
	bool Add(unsigned long nFriendCID, unsigned short nFavorite, const char* pszName);
	void Remove(const char* pszName);
	MMatchIgnoreNode* Find(unsigned long nFriendCID);
	MMatchIgnoreNode* Find(const char* pszName);
	void UpdateDesc();
};

#pragma pack(1)
struct MIGNORELISTNODE {
	unsigned char	nState;
	char			szName[MATCHOBJECT_NAME_LENGTH];
	char			szDescription[MATCH_SIMPLE_DESC_LENGTH];
};
#pragma pack()


#endif