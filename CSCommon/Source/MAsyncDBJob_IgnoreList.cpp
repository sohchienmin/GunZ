#include "stdafx.h"
#include "MAsyncDBJob_IgnoreList.h"

void MAsyncDBJob_IgnoreList::Run(void* pContext)
{
	_ASSERT(m_pIgnoreInfo);

	MMatchDBMgr* pDBMgr = (MMatchDBMgr*)pContext;

	// �ش�ĳ������ ģ����� ��������
	if (!pDBMgr->IgnoreGetList(m_nCID, m_pIgnoreInfo)) 
	{
		SetResult(MASYNC_RESULT_FAILED);
		return;
	}

	SetResult(MASYNC_RESULT_SUCCEED);
}
