#ifndef _MASYNCDBJOB_IGNORELIST_H
#define _MASYNCDBJOB_IGNORELIST_H


#include "MAsyncDBJob.h"



class MAsyncDBJob_IgnoreList : public MAsyncJob {
protected:
	MUID			m_uid;
	
protected:	// Input Argument
	int				m_nCID;
protected:	// Output Result
	MMatchIgnoreInfo*	m_pIgnoreInfo;
public:
	MAsyncDBJob_IgnoreList(const MUID& uid, int nCID)
		: MAsyncJob( 
		MASYNCJOB_IGNORELIST, uid)
	{
		m_pIgnoreInfo = NULL;
		m_uid = uid;
		m_nCID = nCID;
	}
	virtual ~MAsyncDBJob_IgnoreList()	{}

	const MUID& GetUID()			{ return m_uid; }
	MMatchIgnoreInfo* GetIgnoreInfo()			{ return m_pIgnoreInfo; }
	void SetIgnoreInfo(MMatchIgnoreInfo* pIgnoreInfo)	{ m_pIgnoreInfo = pIgnoreInfo; }

	virtual void Run(void* pContext);
};





#endif