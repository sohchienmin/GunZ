#include "stdafx.h"
#include "MAsyncDBJob_IgnoreList.h"

void MAsyncDBJob_IgnoreList::Run(void* pContext)
{
	_ASSERT(m_pIgnoreInfo);

	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	// 해당캐릭터의 친구목록 가져오기
	if (!pDBMgr->IgnoreGetList(m_nCID, m_pIgnoreInfo)) 
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}
