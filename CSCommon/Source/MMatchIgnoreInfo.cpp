#include "stdafx.h"
#include "MMatchIgnoreInfo.h"
#include "MMatchServer.h"
#include "MMatchChannel.h"


MMatchIgnoreInfo::MMatchIgnoreInfo()
{


}

MMatchIgnoreInfo::~MMatchIgnoreInfo()
{
	while (m_IgnoreList.size() > 0) {
		MMatchIgnoreList::iterator i = m_IgnoreList.begin();
		MMatchIgnoreNode* pNode = (MMatchIgnoreNode*)(*i);
		delete pNode;
		m_IgnoreList.pop_front();
	}
}

bool MMatchIgnoreInfo::Add(unsigned long nFriendCID, unsigned short nFavorite, const char* pszName)
{
	if (Find(nFriendCID) != NULL)
		return false;

	MMatchIgnoreNode* pNode = new MMatchIgnoreNode;
	pNode->nFriendCID = nFriendCID;
	pNode->nFavorite = nFavorite;
	strcpy(pNode->szName, pszName);
	strcpy(pNode->szDescription, "");
	m_IgnoreList.push_back(pNode);

	return true;
}

void MMatchIgnoreInfo::Remove(const char* pszName)
{
	for (MMatchIgnoreList::iterator i=m_IgnoreList.begin(); i!= m_IgnoreList.end(); i++) 
	{
		MMatchIgnoreNode* pNode = (*i);
		if (stricmp(pNode->szName, pszName)==0) {
			m_IgnoreList.erase(i);
			delete pNode;
			return;
		}
	}
}

MMatchIgnoreNode* MMatchIgnoreInfo::Find(unsigned long nFriendCID)
{
	for (MMatchIgnoreList::iterator i=m_IgnoreList.begin(); i!= m_IgnoreList.end(); i++) 
	{
		MMatchIgnoreNode* pNode = (*i);
		if (pNode->nFriendCID == nFriendCID)
			return pNode;
	}
	return NULL;
}

MMatchIgnoreNode* MMatchIgnoreInfo::Find(const char* pszName)
{
	for (MMatchIgnoreList::iterator i=m_IgnoreList.begin(); i!= m_IgnoreList.end(); i++) 
	{
		MMatchIgnoreNode* pNode = (*i);
		if (stricmp(pNode->szName, pszName)==0)
			return pNode;
	}
	return NULL;
}


void MMatchIgnoreInfo::UpdateDesc()
{
	MMatchServer* pServer = MMatchServer::GetInstance();
	for (MMatchIgnoreList::iterator i=m_IgnoreList.begin(); i!= m_IgnoreList.end(); i++) 
	{
		MMatchIgnoreNode* pNode = (*i);
		pNode->szDescription[0] = NULL;

		MMatchObject* pObj = pServer->GetPlayerByName(pNode->szName);
		if (pObj) {
			char szDesc[CHANNELNAME_LEN*2]="";

			pNode->nState = pObj->GetPlace();
			MMatchChannel* pChannel = pServer->FindChannel(pObj->GetChannelUID());
			if (pChannel) {
				sprintf(szDesc, "Channel '%s'", pChannel->GetName());
				strncpy(pNode->szDescription, szDesc, MATCH_SIMPLE_DESC_LENGTH);
				pNode->szDescription[MATCH_SIMPLE_DESC_LENGTH-1] = NULL;
			} else {
				strcpy(pNode->szDescription, "Unknown Channel");
			}
		} else {
			pNode->nState = MMP_OUTSIDE;
			strcpy(pNode->szDescription, "Not Logged on");
		}
	}
}