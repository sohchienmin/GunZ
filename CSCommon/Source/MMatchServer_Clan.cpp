#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "Msg.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MAsyncDBJob_WinTheClanGame.h"
#include "MUtil.h"

// 클랜 관련 공용 함수 ////////////////////////////////////////////////////////////////////////////
void CopyClanMemberListNodeForTrans(MTD_ClanMemberListNode* pDest, MMatchObject* pSrcObject)
{
	pDest->uidPlayer = pSrcObject->GetUID();
	strcpy(pDest->szName, pSrcObject->GetCharInfo()->m_szName);
	pDest->nLevel = (char)pSrcObject->GetCharInfo()->m_nLevel;
	pDest->nPlace = pSrcObject->GetPlace();
	pDest->nClanGrade = pSrcObject->GetCharInfo()->m_ClanInfo.m_nGrade;
}


int MMatchServer::ValidateCreateClan(const char* szClanName, MMatchObject* pMasterObject)
{
	int nResult = MOK;

	nResult = ValidateMakingName(szClanName, MIN_CLANNAME, MAX_CLANNAME);
	if (nResult != MOK)
	{
		return nResult;
	}

	//if (pMasterObject->GetCharInfo()->m_nLevel < CLAN_CREATING_NEED_LEVEL)
	//{
	//	return MERR_CLAN_CREATING_LESS_LEVEL;
	//}

	//if (pMasterObject->GetCharInfo()->m_nBP < CLAN_CREATING_NEED_BOUNTY)
	//{
	//	return MERR_CLAN_CREATING_LESS_BOUNTY;
	//}

	int nTempCLID = 0;
	if (m_MatchDBMgr.GetClanIDFromName(szClanName, &nTempCLID))
	{
		return MERR_EXIST_CLAN;
	}

	return MOK;
}

int ValidateJoinClan(MMatchObject* pAdminObject, MMatchObject* pJoinerObject, const char* szClanName)
{
	// 클랜 어드민이상 등급인지 확인
	if (! IsUpperClanGrade(pAdminObject->GetCharInfo()->m_ClanInfo.m_nGrade, MCG_ADMIN))
	{
		return MERR_CLAN_NOT_MASTER_OR_ADMIN;
	}

	// 가입자가 클랜에 이미 가입되어있는지 확인
	if (pJoinerObject->GetCharInfo()->m_ClanInfo.IsJoined() == true)
	{
		return MERR_CLAN_JOINER_JOINED_ALREADY;
	}

	// 가입자가 로비에 있는지 확인
	if (pJoinerObject->GetPlace() != MMP_LOBBY)
	{
		return MERR_CLAN_JOINER_NOT_LOBBY;
	}


	// 클랜 이름이 맞는지 확인
	if (stricmp(pAdminObject->GetCharInfo()->m_ClanInfo.m_szClanName, szClanName))
	{
		return MERR_CLAN_WRONG_CLANNAME;
	}
	
	return MOK;
}


int ValidateLeaveClan(MMatchObject* pLeaverObject)
{
	// 클랜에 속해있는지 확인
	if (!pLeaverObject->GetCharInfo()->m_ClanInfo.IsJoined())
	{
		return MERR_CLAN_NOT_JOINED;
	}

	// 마스터는 탈퇴할 수 없다
	if (pLeaverObject->GetCharInfo()->m_ClanInfo.m_nGrade == MCG_MASTER)
	{
		return MERR_CLAN_CANNOT_LEAVE;
	}

	return MOK;
}

bool IsSameClan(MMatchObject* pSrcObject, MMatchObject* pTarObject)
{
	if ((pSrcObject->GetCharInfo()->m_ClanInfo.m_nClanID == 0) || (pTarObject->GetCharInfo()->m_ClanInfo.m_nClanID == 0) ||
	   (pSrcObject->GetCharInfo()->m_ClanInfo.m_nClanID != pTarObject->GetCharInfo()->m_ClanInfo.m_nClanID))
	{
		return false;
	}

	return true;
}

void MMatchServer::UpdateCharClanInfo(MMatchObject* pObject, const int nCLID, const char* szClanName, const MMatchClanGrade nGrade)
{
	if (! IsEnabledObject(pObject)) return;

	bool bHasJoined = pObject->GetCharInfo()->m_ClanInfo.IsJoined();


	// m_ClanMap의 Join, Leave도 여기서 해준다.
	if ((bHasJoined) && (nCLID == 0))
	{
		m_ClanMap.RemoveObject(pObject->GetUID(), pObject);
	}

	// object의 정보 변경
	strcpy(pObject->GetCharInfo()->m_ClanInfo.m_szClanName, szClanName);
	pObject->GetCharInfo()->m_ClanInfo.m_nGrade = nGrade;
	pObject->GetCharInfo()->m_ClanInfo.m_nClanID = nCLID;


	if ((pObject->GetCharInfo()->m_ClanInfo.IsJoined()) && (!bHasJoined))
	{
		m_ClanMap.AddObject(pObject->GetUID(), pObject);

		// 임시코드... 잘못된 클랜ID 온다면 체크하여 잡기위함...20090224 by kammir
		if(pObject->GetCharInfo()->m_ClanInfo.GetClanID() >= 9000000)
			LOG(LOG_FILE, "[UpdateCharClanInfo()] %s's ClanID:%d.", pObject->GetAccountName(), pObject->GetCharInfo()->m_ClanInfo.GetClanID());
	}


	// route까지 여기서 해준다.
	MCommand* pNewCmd = CreateCommand(MC_MATCH_CLAN_UPDATE_CHAR_CLANINFO, MUID(0,0));
	void* pClanInfoArray = MMakeBlobArray(sizeof(MTD_CharClanInfo), 1);

	MTD_CharClanInfo* pClanInfo = (MTD_CharClanInfo*)MGetBlobArrayElement(pClanInfoArray, 0);
	
	strcpy(pClanInfo->szClanName, szClanName);
	pClanInfo->nGrade = nGrade;

	
	pNewCmd->AddParameter(new MCommandParameterBlob(pClanInfoArray, MGetBlobArraySize(pClanInfoArray)));
	MEraseBlobArray(pClanInfoArray);

	RouteToListener(pObject, pNewCmd);

}





///////////////////////////////////////////////////////////////////////////////////////////////////
MMatchClan* MMatchServer::FindClan(const int nCLID)
{
	MMatchClanMap::iterator i = m_ClanMap.find(nCLID);
	if(i==m_ClanMap.end()) return NULL;

	MMatchClan* pClan = (*i).second;
	return pClan;
}

void MMatchServer::OnClanRequestCreateClan(const MUID& uidPlayer, const char* szClanName, const char* szClanPassword)
{
	MMatchObject* pMasterObject = GetObject(uidPlayer);
	if (!IsEnabledObject(pMasterObject)) return;

	int nRet = ValidateCreateClan(szClanName, pMasterObject);
	if (nRet != MOK)
	{
		MCommand* pNewCmd = CreateCommand(MC_MATCH_CLAN_RESPONSE_CREATE_CLAN, MUID(0, 0));
		pNewCmd->AddParameter(new MCommandParameterInt(nRet));
		RouteToListener(pMasterObject, pNewCmd);
		return;
	}

	MAsyncDBJob_CreateClan* pNewJob = new MAsyncDBJob_CreateClan(uidPlayer);
	pNewJob->Input(szClanName, szClanPassword, pMasterObject->GetCharInfo()->m_nCID, pMasterObject->GetUID());
	pMasterObject->m_DBJobQ.DBJobQ.push_back(pNewJob);
}



void MMatchServer::OnClanAnswerSponsorAgreement(const int nRequestID, const MUID& uidClanMaster, char* szSponsorCharName, const bool bAnswer)
{
	MMatchObject* pClanMasterObject = GetObject(uidClanMaster);
	if (! IsEnabledObject(pClanMasterObject)) return;

	
	MCommand* pNewCmd = CreateCommand(MC_MATCH_CLAN_ANSWER_SPONSOR_AGREEMENT, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
	pNewCmd->AddParameter(new MCommandParameterUID(uidClanMaster));
	pNewCmd->AddParameter(new MCommandParameterString(szSponsorCharName));
	pNewCmd->AddParameter(new MCommandParameterBool(bAnswer));

	RouteToListener(pClanMasterObject, pNewCmd);	
}



void MMatchServer::OnClanRequestCloseClan(const MUID& uidClanMaster, const char* szClanName)
{
	ResponseCloseClan(uidClanMaster, szClanName);
}

void MMatchServer::ResponseCloseClan(const MUID& uidClanMaster, const char* szClanName)
{
	MMatchObject* pMasterObject = GetObject(uidClanMaster);
	if (! IsEnabledObject(pMasterObject)) return;

	// 클랜마스터인지 확인
	if (pMasterObject->GetCharInfo()->m_ClanInfo.m_nGrade != MCG_MASTER)
	{
		RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN, MERR_CLAN_NOT_MASTER);
		return;
	}

	// 클랜 이름이 제대로 되었는지 확인
	if (stricmp(pMasterObject->GetCharInfo()->m_ClanInfo.m_szClanName, szClanName))
	{
		RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN, MERR_CLAN_WRONG_CLANNAME);
		return;
	}

	// 실제로 디비에서 폐쇄 예약
	if (!m_MatchDBMgr.ReserveCloseClan(pMasterObject->GetCharInfo()->m_ClanInfo.m_nClanID,
										pMasterObject->GetCharInfo()->m_ClanInfo.m_szClanName,
										pMasterObject->GetCharInfo()->m_nCID,
										MGetStrLocalTime(DAY_OF_DELETE_CLAN, 0, 0, MDT_YMD)))
	{
		RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN, MERR_CLAN_CANNOT_CLOSE);
		return;
	}


	// 폐쇄예약되었다는 메세지를 보낸다.
	RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN, MOK);
}

void MMatchServer::OnClanRequestJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner)
{
	ResponseJoinClan(uidClanAdmin, szClanName, szJoiner);
}


void MMatchServer::ResponseJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner)
{
	MMatchObject* pAdminObject = GetObject(uidClanAdmin);
	if (! IsEnabledObject(pAdminObject)) return;

	MMatchObject* pJoinerObject = GetPlayerByName(szJoiner);
	if (! IsEnabledObject(pJoinerObject))
	{
		RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_RESPONSE_JOIN_CLAN, MERR_CLAN_OFFLINE_TARGET);
		return;
	}

	// 대상이 초청거부 상태이면 초청못한다
	if (pJoinerObject->CheckUserOption(MBITFLAG_USEROPTION_REJECT_INVITE) == true) {
		NotifyMessage(pAdminObject->GetUID(), MATCHNOTIFY_USER_INVITE_REJECTED);
		return;
	}

	int nRet = ValidateJoinClan(pAdminObject, pJoinerObject, szClanName);
	if (nRet != MOK)
	{
		RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_RESPONSE_JOIN_CLAN, nRet);
		return;
	}

	// 가입자에게 동의를 묻는다.
	MCommand* pNewCmd = CreateCommand(MC_MATCH_CLAN_ASK_JOIN_AGREEMENT, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterString((char*)szClanName));
	pNewCmd->AddParameter(new MCommandParameterUID(uidClanAdmin));
	pNewCmd->AddParameter(new MCommandParameterString(pAdminObject->GetCharInfo()->m_szName));
	RouteToListener(pJoinerObject, pNewCmd);


	RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_RESPONSE_JOIN_CLAN, MOK);
}

void MMatchServer::OnClanAnswerJoinAgreement(const MUID& uidClanAdmin, const char* szJoiner, const bool bAnswer)
{
	MMatchObject* pClanAdminObject = GetObject(uidClanAdmin);
	if (! IsEnabledObject(pClanAdminObject)) return;

	
	MCommand* pNewCmd = CreateCommand(MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterUID(uidClanAdmin));
	pNewCmd->AddParameter(new MCommandParameterString((char*)szJoiner));
	pNewCmd->AddParameter(new MCommandParameterBool(bAnswer));

	RouteToListener(pClanAdminObject, pNewCmd);	
}

void MMatchServer::OnClanRequestAgreedJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner)
{
	ResponseAgreedJoinClan(uidClanAdmin, szClanName, szJoiner);
}

void MMatchServer::ResponseAgreedJoinClan(const MUID& uidClanAdmin, const char* szClanName, const char* szJoiner)
{
	MMatchObject* pAdminObject = GetObject(uidClanAdmin);
	if (! IsEnabledObject(pAdminObject)) return;

	MMatchObject* pJoinerObject = GetPlayerByName(szJoiner);
	if (! IsEnabledObject(pJoinerObject)) return;


	int nRet = ValidateJoinClan(pAdminObject, pJoinerObject, szClanName);
	if (nRet != MOK)
	{
		RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, nRet);
		RouteResponseToListener(pJoinerObject, MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, nRet);
		return;
	}


	int nCLID = pAdminObject->GetCharInfo()->m_ClanInfo.m_nClanID;
	int nJoinerCID = pJoinerObject->GetCharInfo()->m_nCID;
	int nClanGrade = (int)MCG_MEMBER;

	bool bDBRet = false;

	// 실제 디비상에서 가입처리
	if (!m_MatchDBMgr.AddClanMember(nCLID, nJoinerCID, nClanGrade, &bDBRet))
	{
		RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, MERR_CLAN_DONT_JOINED);
		RouteResponseToListener(pJoinerObject, MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, MERR_CLAN_DONT_JOINED);
		return;
	}

	// 인원이 초과되면 db return 값이 false이다.
	if (!bDBRet)
	{
		RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, MERR_CLAN_MEMBER_FULL);
		RouteResponseToListener(pJoinerObject, MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, MERR_CLAN_MEMBER_FULL);
		return;
	}

	// 클랜정보 업데이트하고 Route해줌
	UpdateCharClanInfo(pJoinerObject, pAdminObject->GetCharInfo()->m_ClanInfo.m_nClanID, szClanName, MCG_MEMBER);
	// 임시코드... 잘못된 MMatchObject*가 온다면 체크하여 잡기위함...20090224 by kammir
	if(pAdminObject->GetCharInfo()->m_ClanInfo.GetClanID() >= 9000000)
		LOG(LOG_FILE, "[ResponseAgreedJoinClan()] %s's ClanID:%d.", pAdminObject->GetAccountName(), pAdminObject->GetCharInfo()->m_ClanInfo.GetClanID());




	RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN, MOK);
	RouteResponseToListener(pJoinerObject, MC_MATCH_RESPONSE_RESULT, MRESULT_CLAN_JOINED);
}



void MMatchServer::OnClanRequestLeaveClan(const MUID& uidPlayer)
{
	ResponseLeaveClan(uidPlayer);
}

void MMatchServer::ResponseLeaveClan(const MUID& uidPlayer)
{
	MMatchObject* pLeaverObject = GetObject(uidPlayer);
	if (! IsEnabledObject(pLeaverObject)) return;

	int nRet = ValidateLeaveClan(pLeaverObject);

	if (nRet != MOK)
	{
		RouteResponseToListener(pLeaverObject, MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN, nRet);
		return;
	}


	int nCLID = pLeaverObject->GetCharInfo()->m_ClanInfo.m_nClanID;
	int nLeaverCID = pLeaverObject->GetCharInfo()->m_nCID;

	// 실제로 디비상에서 탈퇴처리
	if (!m_MatchDBMgr.RemoveClanMember(nCLID, nLeaverCID))
	{
		RouteResponseToListener(pLeaverObject, MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN, MERR_CLAN_CANNOT_LEAVE);
		return;
	}

	// 클랜정보 업데이트하고 Route해줌
	UpdateCharClanInfo(pLeaverObject, 0, "", MCG_NONE);


	RouteResponseToListener(pLeaverObject, MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN, MOK);
}

void MMatchServer::OnClanRequestChangeClanGrade(const MUID& uidClanMaster, const char* szMember, int nClanGrade)
{
	ResponseChangeClanGrade(uidClanMaster, szMember, nClanGrade);
}

int ValidateChangeClanGrade(MMatchObject* pMasterObject, MMatchObject* pTargetObject, int nClanGrade)
{
	// 마스터인지 확인
	if (pMasterObject->GetCharInfo()->m_ClanInfo.m_nGrade != MCG_MASTER)
	{
		return MERR_CLAN_NOT_MASTER;
	}

	// 같은 클랜인지 확인
	if (!IsSameClan(pMasterObject, pTargetObject))
	{
		return MERR_CLAN_OTHER_CLAN;
	}


	// 마스터를 권한변경할 수 없다.
	if (pTargetObject->GetCharInfo()->m_ClanInfo.m_nGrade == MCG_MASTER)
	{
		return MERR_CLAN_NOT_MASTER;
	}

	MMatchClanGrade grade = (MMatchClanGrade)nClanGrade;
	if ((grade != MCG_ADMIN) && (grade != MCG_MEMBER))
	{
		return MERR_CLAN_CANNOT_CHANGE_GRADE;
	}

	return MOK;
}

void MMatchServer::ResponseChangeClanGrade(const MUID& uidClanMaster, const char* szMember, int nClanGrade)
{
	MMatchObject* pMasterObject = GetObject(uidClanMaster);
	if (! IsEnabledObject(pMasterObject)) return;

	MMatchObject* pTargetObject = GetPlayerByName(szMember);
	if (! IsEnabledObject(pTargetObject))
	{
		RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE, MERR_CLAN_OFFLINE_TARGET);
		return;
	}

	// 권한 변경 가능한지 체크
	int nRet = ValidateChangeClanGrade(pMasterObject, pTargetObject, nClanGrade);
	if (nRet != MOK)
	{
		RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE, nRet);
		return;
	}

	int nCLID = pMasterObject->GetCharInfo()->m_ClanInfo.m_nClanID;
	int nMemberCID = pTargetObject->GetCharInfo()->m_nCID;
	
	// 실제로 디비상에서 권한 변경
	if (!m_MatchDBMgr.UpdateClanGrade(nCLID, nMemberCID, nClanGrade))
	{
		RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE, MERR_CLAN_CANNOT_CHANGE_GRADE);
		return;
	}

	// 클랜정보 업데이트하고 Route해줌
	UpdateCharClanInfo(pTargetObject, pTargetObject->GetCharInfo()->m_ClanInfo.m_nClanID, 
						pTargetObject->GetCharInfo()->m_ClanInfo.m_szClanName, (MMatchClanGrade)nClanGrade);
	// 임시코드... 잘못된 MMatchObject*가 온다면 체크하여 잡기위함...20090224 by kammir
	if(pTargetObject->GetCharInfo()->m_ClanInfo.GetClanID() >= 9000000)
		LOG(LOG_FILE, "[ResponseChangeClanGrade()] %s's ClanID:%d.", pTargetObject->GetAccountName(), pTargetObject->GetCharInfo()->m_ClanInfo.GetClanID());


	RouteResponseToListener(pMasterObject, MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE, MOK);
}


void MMatchServer::OnClanRequestExpelMember(const MUID& uidClanAdmin, const char* szMember)
{
	ResponseExpelMember(uidClanAdmin, szMember);
}

void MMatchServer::ResponseExpelMember(const MUID& uidClanAdmin, const char* szMember)
{
	MMatchObject* pAdminObject = GetObject(uidClanAdmin);
	if (! IsEnabledObject(pAdminObject)) return;

	// 탈퇴처리할 수 있는 권한인지 검사
	if (!IsUpperClanGrade(pAdminObject->GetCharInfo()->m_ClanInfo.m_nGrade, MCG_ADMIN))
	{
		RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MERR_CLAN_NOT_MASTER_OR_ADMIN);
		return;
	}

	int nCLID = pAdminObject->GetCharInfo()->m_ClanInfo.m_nClanID;
	int nClanGrade = pAdminObject->GetCharInfo()->m_ClanInfo.m_nGrade;

	MAsyncDBJob_ExpelClanMember* pNewJob = new MAsyncDBJob_ExpelClanMember(uidClanAdmin);
	pNewJob->Input(uidClanAdmin,
					nCLID,
					nClanGrade,
					szMember);
	// PostAsyncJob(pNewJob);
	pAdminObject->m_DBJobQ.DBJobQ.push_back( pNewJob );

/*
	// 디비상에서 탈퇴처리
	int nDBRet = 0;
	int nCLID = pAdminObject->GetCharInfo()->m_ClanInfo.m_nClanID;
	int nClanGrade = pAdminObject->GetCharInfo()->m_ClanInfo.m_nGrade;
	char szTarMember[256]; 
	sprintf(szTarMember, szMember);

	// 실제로 디비상에서 권한 변경
	if (!m_MatchDBMgr.ExpelClanMember(nCLID, nClanGrade, szTarMember, &nDBRet))
	{
		RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MERR_CLAN_CANNOT_EXPEL_FOR_NO_MEMBER);
		return;
	}


	switch (nDBRet)
	{
	case MMatchDBMgr::ER_NO_MEMBER:
		{
			RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MERR_CLAN_CANNOT_EXPEL_FOR_NO_MEMBER);
			return;
		}
		break;
	case MMatchDBMgr::ER_WRONG_GRADE:
		{
			RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MERR_CLAN_CANNOT_CHANGE_GRADE);
			return;
		}
		break;
	}


	// 만약 당사자가 접속해있으면 클랜탈퇴되었다고 알려줘야한다.
	MMatchObject* pMemberObject = GetPlayerByName(szMember);
	if (IsEnabledObject(pMemberObject))
	{
		UpdateCharClanInfo(pMemberObject, 0, "", MCG_NONE);
	}


	RouteResponseToListener(pAdminObject, MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER, MOK);
*/
}


void MMatchServer::OnClanRequestMsg(const MUID& uidSender, const char* szMsg)
{
	MMatchObject* pSenderObject = GetObject(uidSender);
	if (! IsEnabledObject(pSenderObject)) return;
	
	if (!pSenderObject->GetCharInfo()->m_ClanInfo.IsJoined()) return;

	MCommand* pNewCmd = CreateCommand(MC_MATCH_CLAN_MSG, MUID(0,0));

	char szSenderName[256];
	char szTransMsg[512];
	strcpy(szSenderName, pSenderObject->GetCharInfo()->m_szName);
	strcpy(szTransMsg, szMsg);

	pNewCmd->AddParameter(new MCommandParameterString(szSenderName));
	pNewCmd->AddParameter(new MCommandParameterString(szTransMsg));


	int nCLID = pSenderObject->GetCharInfo()->m_ClanInfo.m_nClanID;

	RouteToClan(nCLID, pNewCmd);	
}

//void MMatchServer::OnClanRequestMemberList(const MUID& uidChar)
//{
//	ResponseClanMemberList(uidChar);
//}

void MMatchServer::OnClanRequestMemberList(const MUID& uidChar)
{
	MMatchObject* pObj = (MMatchObject*)GetObject(uidChar);
	if (! IsEnabledObject(pObj)) return;

	MMatchClan* pClan = FindClan(pObj->GetCharInfo()->m_ClanInfo.m_nClanID);
	if (pClan == NULL) return;

	MRefreshClientClanMemberImpl* pImpl = pObj->GetRefreshClientClanMemberImplement();
	pImpl->SetCategory(0);
	pImpl->SetChecksum(0);
	pImpl->Enable(true);
	pClan->SyncPlayerList(pObj, 0);
}

void MMatchServer::ResponseClanMemberList(const MUID& uidChar)
{
	MMatchObject* pObject = GetObject(uidChar);
	if (! IsEnabledObject(pObject)) return;

	if (!pObject->GetCharInfo()->m_ClanInfo.IsJoined()) return;

	MMatchClan* pClan = FindClan(pObject->GetCharInfo()->m_ClanInfo.m_nClanID);
	if (pClan == NULL) return;

	int nNodeCount = pClan->GetMemberCount();
	if (nNodeCount <= 0) return;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CLAN_RESPONSE_MEMBER_LIST), MUID(0,0), m_This);

	void* pMemberArray = MMakeBlobArray(sizeof(MTD_ClanMemberListNode), nNodeCount);

	int nArrayIndex=0;
	for (MUIDRefCache::iterator itor= pClan->GetMemberBegin(); itor != pClan->GetMemberEnd(); ++itor) 
	{
		MMatchObject* pScanObj = (MMatchObject*)(*itor).second;

		MTD_ClanMemberListNode* pNode = (MTD_ClanMemberListNode*)MGetBlobArrayElement(pMemberArray, nArrayIndex++);

		if (IsEnabledObject(pScanObj))
		{
			CopyClanMemberListNodeForTrans(pNode, pScanObj);		
		}

		if (nArrayIndex >= nNodeCount) break;
	}

	pNew->AddParameter(new MCommandParameterBlob(pMemberArray, MGetBlobArraySize(pMemberArray)));
	MEraseBlobArray(pMemberArray);
	RouteToListener(pObject, pNew);
}

void CopyClanInfoForTrans(MTD_ClanInfo* pDest, MMatchClan* pClan)
{
	strcpy(pDest->szClanName, pClan->GetName());
	strcpy(pDest->szMaster, pClan->GetClanInfoEx()->szMaster);
	pDest->nLevel = pClan->GetClanInfoEx()->nLevel;
	pDest->nTotalPoint = pClan->GetClanInfoEx()->nTotalPoint;
	pDest->nRanking = pClan->GetClanInfoEx()->nRanking;
	pDest->nWins = pClan->GetClanInfoEx()->nWins;
	pDest->nLosses = pClan->GetClanInfoEx()->nLosses;
	pDest->nPoint = pClan->GetClanInfoEx()->nPoint;
	pDest->nTotalMemberCount = pClan->GetClanInfoEx()->nTotalMemberCount;
	pDest->nConnedMember = pClan->GetMemberCount();
	pDest->nCLID = pClan->GetCLID();
	pDest->nEmblemChecksum = pClan->GetEmblemChecksum();
}

void MMatchServer::OnClanRequestClanInfo(const MUID& uidChar, const char* szClanName)
{
	MMatchObject* pObject = GetObject(uidChar);
	if (! IsEnabledObject(pObject)) return;

	MMatchClan* pClan = m_ClanMap.GetClan(szClanName);
	if ((pClan == NULL) || (!pClan->IsInitedClanInfoEx())) return;

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CLAN_RESPONSE_CLAN_INFO), MUID(0,0), m_This);

	void* pClanInfoArray = MMakeBlobArray(sizeof(MTD_ClanInfo), 1);
	MTD_ClanInfo* pClanInfo = (MTD_ClanInfo*)MGetBlobArrayElement(pClanInfoArray, 0);
	CopyClanInfoForTrans(pClanInfo, pClan);

	pNew->AddParameter(new MCommandParameterBlob(pClanInfoArray, MGetBlobArraySize(pClanInfoArray)));
	MEraseBlobArray(pClanInfoArray);
	RouteToListener(pObject, pNew);
}

void MMatchServer::OnClanRequestEmblemURL(const MUID& uidChar, void* pEmblemURLListBlob)
{
	MMatchObject* pObject = GetObject(uidChar);
	if (! IsEnabledObject(pObject)) return;

	int nClanURLCount = MGetBlobArrayCount(pEmblemURLListBlob);
	if (nClanURLCount < 1) return;

	for (int i = 0; i < nClanURLCount; i++)
	{
		int* pClanID = (int*)MGetBlobArrayElement(pEmblemURLListBlob, i);
		MMatchClan* pClan = m_ClanMap.GetClan(*pClanID);
		if (pClan == NULL) continue;

		MCommand* pNew = CreateCommand(MC_MATCH_CLAN_RESPONSE_EMBLEMURL, MUID(0,0));
		pNew->AddParameter(new MCmdParamInt(pClan->GetCLID()));
		pNew->AddParameter(new MCmdParamInt(pClan->GetEmblemChecksum()));
		pNew->AddParameter(new MCmdParamStr(pClan->GetEmblemURL()));
		RouteToListener(pObject, pNew);
	}
}

void MMatchServer::StandbyClanList(const MUID& uidPlayer, int nClanListStartIndex, bool bCacheUpdate)
{
	MMatchObject* pObject = GetObject(uidPlayer);
	if (! IsEnabledObject(pObject)) return;

	int nPrevClanListCount = -1, nNextClanListCount = -1;
	int nGroupCount = (int)GetLadderMgr()->GetGroupCount();
	if (nClanListStartIndex < 0) nClanListStartIndex = 0;
	if (nClanListStartIndex > nGroupCount) nClanListStartIndex = nGroupCount;


	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_CLAN_STANDBY_CLAN_LIST), MUID(0,0), m_This);


	nPrevClanListCount = nClanListStartIndex;
	if ((nPrevClanListCount < 0) || (nPrevClanListCount > nGroupCount)) nPrevClanListCount = 0;
	nNextClanListCount = nGroupCount - (nClanListStartIndex + MAX_LADDER_TEAM_MEMBER);
	if ((nNextClanListCount < 0) || (nNextClanListCount > nGroupCount)) nNextClanListCount = 0;

	pNew->AddParameter(new MCommandParameterInt(nPrevClanListCount));
	pNew->AddParameter(new MCommandParameterInt(nNextClanListCount));


	int nRealCount = max(0, min(MAX_LADDER_TEAM_MEMBER, (nGroupCount - nClanListStartIndex)));

	list<MLadderGroup*>::iterator itorGroup = GetLadderMgr()->GetGroupListBegin();
	for (int i = 0; i < nClanListStartIndex; i++)
	{
		if (itorGroup == GetLadderMgr()->GetGroupListEnd()) break;
		itorGroup++;
	}

	int nArrayIndex=0;
	void* pClanListArray = MMakeBlobArray(sizeof(MTD_StandbyClanList), nRealCount);

	for (int i = 0; i < nRealCount; i++)
	{
		if (itorGroup == GetLadderMgr()->GetGroupListEnd()) break;

		MTD_StandbyClanList* pNode = (MTD_StandbyClanList*)MGetBlobArrayElement(pClanListArray, i);
		memset(pNode, 0, sizeof(MTD_StandbyClanList));

		MLadderGroup* pLadderGroup = *itorGroup;
		
		if (pLadderGroup->GetPlayerCount() > 0)
		{
			MUID uidMember = *pLadderGroup->GetPlayerListBegin();
			MMatchObject* pMember = GetObject(uidMember);
			if ((IsEnabledObject(pMember)) && (pMember->GetCharInfo()->m_ClanInfo.IsJoined()))
			{
				strcpy(pNode->szClanName, pMember->GetCharInfo()->m_ClanInfo.m_szClanName);
			}

			pNode->nPlayers = (int)pLadderGroup->GetPlayerCount();
			pNode->nCLID = pLadderGroup->GetCLID();

			MMatchClan* pClan = FindClan(pLadderGroup->GetCLID());
			if (pClan)
				pNode->nEmblemChecksum = pClan->GetEmblemChecksum();
			else
				pNode->nEmblemChecksum = 0;

			pNode->bDodge = pLadderGroup->GetDodgeMatching();
		}
		itorGroup++;
	}

	pNew->AddParameter(new MCommandParameterBlob(pClanListArray, MGetBlobArraySize(pClanListArray)));
	MEraseBlobArray(pClanListArray);
	
	RouteToListener(pObject, pNew);	
}


void MMatchServer::SaveClanPoint(MMatchClan* pWinnerClan, MMatchClan* pLoserClan, const bool bIsDrawGame,
								 const int nRoundWins, const int nRoundLosses, const int nMapID, const int nGameType,
								 const int nOneTeamMemberCount, list<MUID>& WinnerObjUIDs,
								 const char* szWinnerMemberNames, const char* szLoserMemberNames,
								 float fPointRatio)
{
	if (bIsDrawGame) return;
	if ((!pWinnerClan) || (!pLoserClan)) return;

	int nAddedWinnerPoint = 0, nAddedLoserPoint = 0;

	int nWinnerClanPoint = pWinnerClan->GetClanInfoEx()->nPoint;
	int nLoserClanPoint = pLoserClan->GetClanInfoEx()->nPoint;

	int nWinnerCLID = pWinnerClan->GetCLID();
	int nLoserCLID = pLoserClan->GetCLID();

	int nPoint = MMatchFormula::GetClanBattlePoint(nWinnerClanPoint, nLoserClanPoint, nOneTeamMemberCount);

	nAddedWinnerPoint = nPoint * 2;

	if (nOneTeamMemberCount == MLADDERTYPE_NORMAL_4VS4 || nOneTeamMemberCount == MLADDERTYPE_NORMAL_5VS5 || nOneTeamMemberCount == MLADDERTYPE_NORMAL_6VS6 || nOneTeamMemberCount == MLADDERTYPE_NORMAL_7VS7 || nOneTeamMemberCount == MLADDERTYPE_NORMAL_8VS8) // 4v4인전 포인트가 두배
		nAddedWinnerPoint = nAddedWinnerPoint * 2;	
	else if (nOneTeamMemberCount == MLADDERTYPE_NORMAL_3VS3) 
		nAddedWinnerPoint = (int)(nAddedWinnerPoint * 1.5f);	

	nAddedWinnerPoint = int(nAddedWinnerPoint * fPointRatio);

	nAddedLoserPoint = -(nPoint / 2);


	if (pWinnerClan)
	{
		pWinnerClan->IncWins(1);
		pWinnerClan->IncPoint(nAddedWinnerPoint);
		pWinnerClan->InsertMatchedClanID(nLoserCLID);
	}

	if (pLoserClan)
	{
		pLoserClan->IncLosses(1);
		pLoserClan->IncPoint(nAddedLoserPoint);
		pLoserClan->InsertMatchedClanID(nWinnerCLID);
	}

	MAsyncDBJob_WinTheClanGame* pNewJob = new MAsyncDBJob_WinTheClanGame();
	pNewJob->Input(nWinnerCLID, 
				   nLoserCLID, 
				   bIsDrawGame,
                   nAddedWinnerPoint, 
				   nAddedLoserPoint,
                   pWinnerClan->GetName(), 
				   pLoserClan->GetName(),
                   nRoundWins, 
				   nRoundLosses, 
				   nMapID, 
				   nGameType, 
				   szWinnerMemberNames, 
				   szLoserMemberNames );
	PostAsyncJob(pNewJob);


	// 캐릭터의 클랜 기여도 업데이트
	for (list<MUID>::iterator itor=WinnerObjUIDs.begin(); itor!=WinnerObjUIDs.end(); itor++) 
	{
		MUID uidObject = (*itor);

		MMatchObject* pObject = GetObject(uidObject);
		if (IsEnabledObject(pObject))
		{
			int nCID = pObject->GetCharInfo()->m_nCID;
			pObject->GetCharInfo()->m_ClanInfo.m_nContPoint += nAddedWinnerPoint;

			MAsyncDBJob_UpdateCharClanContPoint* pJob=new MAsyncDBJob_UpdateCharClanContPoint(nCID, nWinnerCLID, nAddedWinnerPoint);
			PostAsyncJob(pJob);
		}
	}

	
}



