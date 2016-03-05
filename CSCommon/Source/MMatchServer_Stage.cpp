#include "stdafx.h"
#include "MMatrix.h"
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
#include "MVoteDiscussImpl.h"
#include "MUtil.h"
#include "MMatchGameType.h"
#include "MMatchRuleBaseQuest.h"
#include "MMatchRuleQuest.h"
#include "MMatchRuleBerserker.h"
#include "MMatchRuleDuel.h"
#include "MCrashDump.h"

#include "MAsyncDBJob_InsertGamePlayerLog.h"

static bool StageShowInfo(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat);



MMatchStage* MMatchServer::FindStage(const MUID& uidStage)
{
	MMatchStageMap::iterator i = m_StageMap.find(uidStage);
	if (i == m_StageMap.end()) return NULL;

	MMatchStage* pStage = (*i).second;
	return pStage;
}

bool MMatchServer::StageAdd(MMatchChannel* pChannel, const char* pszStageName, bool bPrivate, const char* pszStagePassword, MUID* pAllocUID, bool bIsAllowNullChannel)
{
	// 클랜전은 pChannel이 NULL이다.

	MUID uidStage = m_StageMap.UseUID();

	MMatchStage* pStage = new MMatchStage;
	if (pChannel && !pChannel->AddStage(pStage)) {
		delete pStage;
		return false;
	}


	MMATCH_GAMETYPE GameType = MMATCH_GAMETYPE_DEFAULT;
	bool bIsCheckTicket = false;
	DWORD dwTicketID = 0;

	if ((NULL != pChannel) && MGetServerConfig()->IsUseTicket()) {
		bIsCheckTicket = (pChannel != 0) && pChannel->IsUseTicket() && pChannel->IsTicketChannel();
		dwTicketID = pChannel->GetTicketItemID();

		// 티켓 서버에서 사설 채널은 무조건 티켓 검사 - 티켓은 클랜전 티켓과 동일하다.
		if (pChannel->GetChannelType() == MCHANNEL_TYPE_USER) {
			bIsCheckTicket = true;
			dwTicketID = GetChannelMap()->GetClanChannelTicketInfo().m_dwTicketItemID;
		}
	}

	if (!pStage->Create(uidStage, pszStageName, bPrivate, pszStagePassword, bIsAllowNullChannel, GameType, bIsCheckTicket, dwTicketID)) {
		if (pChannel) {
			pChannel->RemoveStage(pStage);
		}

		delete pStage;
		return false;
	}

	m_StageMap.Insert(uidStage, pStage);

	*pAllocUID = uidStage;

	return true;
}


bool MMatchServer::StageRemove(const MUID& uidStage, MMatchStageMap::iterator* pNextItor)
{
	MMatchStageMap::iterator i = m_StageMap.find(uidStage);
	if (i == m_StageMap.end()) {
		return false;
	}

	MMatchStage* pStage = (*i).second;

	MMatchChannel* pChannel = FindChannel(pStage->GetOwnerChannel());
	if (pChannel) {
		pChannel->RemoveStage(pStage);
	}

	pStage->Destroy();
	delete pStage;

	MMatchStageMap::iterator itorTemp = m_StageMap.erase(i);
	if (pNextItor) *pNextItor = itorTemp;

	return true;
}


bool MMatchServer::StageJoin(const MUID& uidPlayer, const MUID& uidStage, bool rejoin, MMatchTeam Team)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) {
		string x = "ISENABLEDOBJECT";
		LOG(LOG_PROG, x.c_str());
		return false;
	}

	if (pObj->GetStageUID() != MUID(0,0))
		StageLeave(pObj->GetUID());//, pObj->GetStageUID());

	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) {
		string x = "PCHANNEL";
		LOG(LOG_PROG, x.c_str());
		return false;
	}
	if (pChannel->GetChannelType() == MCHANNEL_TYPE_DUELTOURNAMENT) {
		string x = "PCHANNEL == duel";
		LOG(LOG_PROG, x.c_str());
		return false;
	}

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) {
		string x = "PSTAGE";
		LOG(LOG_PROG, x.c_str());
		return false;
	}
	if(rejoin == true)
	{
		//GetDBMgr()->InsertRejoinLog(pStage->GetName(), pObj->GetCharInfo()->m_nCID, pStage->GetBlueCLID(), pStage->GetRedCLID(), (int)Team, pStage->GetUID().Low);
	}
	int ret = ValidateStageJoin(uidPlayer, uidStage);
	if (ret != MOK) {
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, ret);
		string x = "RET != MOK";
		LOG(LOG_PROG, x.c_str());
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, ret);
		return false;
	}
	pObj->OnStageJoin();

	// Cache Add
	MMatchObjectCacheBuilder CacheBuilder;
	CacheBuilder.AddObject(pObj);
	MCommand* pCmdCacheAdd = CacheBuilder.GetResultCmd(MATCHCACHEMODE_ADD, this);
	RouteToStage(pStage->GetUID(), pCmdCacheAdd);

	// Join
	pStage->AddObject(uidPlayer, pObj);
		// ÀÓ½ÃÄÚµå... Àß¸øµÈ Å¬·£ID ¿Â´Ù¸é Ã¼Å©ÇÏ¿© Àâ±âÀ§ÇÔ...20090224 by kammir
	if(pObj->GetCharInfo()->m_ClanInfo.GetClanID() >= 9000000)
		LOG(LOG_FILE, "[UpdateCharClanInfo()] %s's ClanID:%d.", pObj->GetAccountName(), pObj->GetCharInfo()->m_ClanInfo.GetClanID());

	pObj->SetStageUID(uidStage);
	pObj->SetStageState(MOSS_NONREADY);

	// Cast Join
	if(rejoin && pStage->GetStageType() != MST_LADDER) rejoin = false;
	if(!rejoin)
	{
		pObj->SetTeam(pStage->GetRecommandedTeam());
		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_JOIN), MUID(0,0), m_This);
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
		pNew->AddParameter(new MCommandParameterUInt(pStage->GetIndex()+1));
		pNew->AddParameter(new MCommandParameterString((char*)pStage->GetName()));
	
		if (pStage->GetState() == STAGE_STATE_STANDBY)  RouteToStage(pStage->GetUID(), pNew);
		else											RouteToListener(pObj, pNew);

	} 
	else
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_LADDER_PREPARE, uidPlayer);
		pCmd->AddParameter(new MCmdParamUID(uidStage));
		pCmd->AddParameter(new MCmdParamInt((int)pObj->GetTeam()));
		RouteToListener(pObj, pCmd);
	}
	// Cache Update
	CacheBuilder.Reset();
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pScanObj = (MMatchObject*)GetObject(uidObj);
		if (pScanObj) {
			CacheBuilder.AddObject(pScanObj);
		} else {
			LOG(LOG_PROG, "MMatchServer::StageJoin - Invalid ObjectMUID(%u:%u) exist in Stage(%s)\n",
				uidObj.High, uidObj.Low, pStage->GetName());
			pStage->RemoveObject(uidObj);
			return false;
		}
	}
    MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
	RouteToListener(pObj, pCmdCacheUpdate);

	if(!rejoin)
	{
		// Cast Master(¹æÀå)
		MUID uidMaster = pStage->GetMasterUID();
		MCommand* pMasterCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
		pMasterCmd->AddParameter(new MCommandParameterUID(uidStage));
		pMasterCmd->AddParameter(new MCommandParameterUID(uidMaster));
		RouteToListener(pObj, pMasterCmd);
	}

#ifdef _QUEST_ITEM
	if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
	{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if (0 == pNode)
		{
			mlog("MMatchServer::StageJoin - 스테이지 셋팅 노드 찾기 실패.\n");
			return false;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >(pStage->GetRule());
			if (0 == pRuleQuest)
			{
				mlog("MMatchServer::StageJoin - 포인터 형변환 실패.\n");
				return false;
			}

			pRuleQuest->OnChangeCondition();
			//pRuleQuest->OnResponseQL_ToStage( pObj->GetStageUID() );
			// 동환씨께서 처음 스테이지 조인시는 이전에 설정이 퀘스트로 되있어도 
			//  처음 조인한 유저는 퀘스트 타입인지 알수가 없기에,
			//	클라이언트가 스테이지 타입이 퀘스트인지를 인식하는 시점에서
			//  이 정보를 요청을 하는 방향으로 수정함. - 05/04/14 by 추교성.
			// pStage->GetRule()->OnResponseSacrificeSlotInfoToStage( uidPlayer );
		}
	}
#endif

	if( rejoin == false)
	{
	// Cast Character Setting
	StageTeam(uidPlayer, uidStage, pObj->GetTeam());
	StagePlayerState(uidPlayer, uidStage, pObj->GetStageState());
	} else {
		
		pObj->SetForcedEntry(true);
		StageTeam(uidPlayer, uidStage, Team);
		StagePlayerState(uidPlayer, uidStage, MOSS_READY);
		MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
		RouteToListener(pObj, pCmd);
		pObj->SetLaunchedGame(true);
		pCmd = CreateCommand(MC_MATCH_LADDER_LAUNCH, uidPlayer);
		pCmd->AddParameter(new MCmdParamUID(uidStage));
		pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
		pCmd->AddParameter(new MCmdParamBool((bool)(false)));
		RouteToListener(pObj, pCmd);
		StageEnterBattle(uidPlayer, uidStage);
	}
	string x = "REJOINED!!!!!!!!!!!!!!";
	LOG(LOG_PROG, x.c_str());
	return true;
}

bool MMatchServer::StageLeave(const MUID& uidPlayer)//, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return false;

	bool bLeaverMaster = false;
	if (uidPlayer == pStage->GetMasterUID()) bLeaverMaster = true;

#ifdef _QUEST_ITEM
	if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
	{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if (0 != pNode)
		{
			if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
			{
				MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >(pStage->GetRule());
				if (pRuleQuest)
				{
					pRuleQuest->PreProcessLeaveStage(uidPlayer);
				}
				else {
					LOG(LOG_PROG, "StageLeave:: MMatchRule to MMatchRuleBaseQuest FAILED \n");
				}
			}
		}
	}
#endif

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LEAVE), MUID(0, 0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	// pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	RouteToStage(pStage->GetUID(), pNew);

	pStage->RemoveObject(uidPlayer);

	//MMatchObject* pObj = GetObject(uidPlayer);
	//if (pObj)
	{
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pObj);
		MCommand* pCmdCache = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REMOVE, this);
		RouteToStage(pStage->GetUID(), pCmdCache);
	}

	// cast Master
	if (bLeaverMaster) StageMaster(pStage->GetUID());

#ifdef _QUEST_ITEM
	// 유저가 스테이지에서 나간후에 QL을 다시 계산해 줘야 함.
	if (MGetServerConfig()->GetServerMode() == MSM_CLAN)
	{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if (0 == pNode)
		{
			mlog("MMatchServer::StageLeave - 스테이지 셋팅 노드 찾기 실패.\n");
			return false;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >(pStage->GetRule());
			if (0 == pRuleQuest)
			{
				mlog("MMatchServer::StageLeave - 포인터 형변환 실패.\n");
				return false;
			}

			if (STAGE_STATE_STANDBY == pStage->GetState())
				pRuleQuest->OnChangeCondition();
			//pRuleQuest->OnResponseQL_ToStage( uidStage );
		}
	}
#endif


	return true;
}


DWORD StageEnterBattleExceptionHandler( PEXCEPTION_POINTERS ExceptionInfo )
{
	char szStageDumpFileName[ _MAX_DIR ]= {0,};
	SYSTEMTIME SystemTime;
	GetLocalTime( &SystemTime );
	sprintf( szStageDumpFileName, "Log/StageDump_%d-%d-%d_%d-%d-%d.dmp"
		, SystemTime.wYear
		, SystemTime.wMonth
		, SystemTime.wDay
		, SystemTime.wHour
		, SystemTime.wMinute
		, SystemTime.wSecond );

	return CrashExceptionDump( ExceptionInfo, szStageDumpFileName, true );
}



bool ExceptionTraceStageEnterBattle( MMatchObject* pObj, MMatchStage* pStage )
{
	if( NULL == pObj )
	{
		return false;
	}

	if( NULL == pStage )
	{
		return false;
	}

	__try
	{
		pStage->EnterBattle(pObj);
	}
	__except( StageEnterBattleExceptionHandler(GetExceptionInformation()) )
	{
	/*	mlog( "\nexception : stage enter battle =====================\n" );


		MMatchObject* pMaster = MGetMatchServer()->GetObject( pStage->GetMasterUID() );
		if( NULL != pMaster )  
		{
			if( NULL != pMaster->GetCharInfo() )
			{
				mlog( "stage master cid : %d\n", pMaster->GetCharInfo()->m_nCID );
			}
		}
		else
		{
			mlog( "stage master hass problem.\n" );				
		}
		
		
		if( NULL != pObj->GetCharInfo() )
		{
			mlog( "cmd sender cid : %d\n", pObj->GetCharInfo()->m_nCID );
		}
		else
		{
			mlog( "cmd send char info null point.\n" );
		}

		
		MMatchStageSetting*	pStageSetting = pStage->GetStageSetting();
		if( NULL != pStageSetting )
		{
			mlog( "stage state : %d\n", pStage->GetStageSetting()->GetStageState() );

			const MSTAGE_SETTING_NODE* pExStageSettingNode = pStageSetting->GetStageSetting();
			if( NULL != pExStageSettingNode )
			{
				mlog( "stage name : %s\n", pExStageSettingNode->szMapName );
				mlog( "stage game type : %d\n", pExStageSettingNode->nGameType );
				mlog( "stage max player : %d\n", pExStageSettingNode->nMaxPlayers );
				mlog( "stage current player : %d\n", pStage->GetPlayers() );
				mlog( "stage force entry enable : %d\n", pExStageSettingNode->bForcedEntryEnabled );
				mlog( "stage rule pointer : %x\n", pStage->GetRule() );
			}
		}

		MUIDRefCache::iterator itStage, endStage;
		endStage = pStage->GetObjEnd();
		itStage = pStage->GetObjBegin();
		MMatchObject* pObj = NULL;
		for( ; endStage != itStage; ++itStage )
		{
			pObj = MGetMatchServer()->GetObject( itStage->first );
			if( NULL == pObj )
			{
				mlog( "!!!!stage can't find player!!!!\n" );
				continue;
			}

			mlog( "stage player name : %s\n", pObj->GetName() );
		}

		mlog( "=====================\n\n" );
		*/
		return false;
	}

	return true;
}



bool MMatchServer::StageEnterBattle(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	// MMatchStage* pStage = FindStage(uidStage);
	
	if(pObj->GetStageUID()!=uidStage)
		mlog(" stage enter battle hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return false;

	pObj->SetPlace(MMP_BATTLE);

	MCommand* pNew = CreateCommand(MC_MATCH_STAGE_ENTERBATTLE, MUID(0,0));
	//pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	//pNew->AddParameter(new MCommandParameterUID(uidStage));

	unsigned char nParam = MCEP_NORMAL;
	if (pObj->IsForcedEntried()) nParam = MCEP_FORCED;
	pNew->AddParameter(new MCommandParameterUChar(nParam));

	void* pPeerArray = MMakeBlobArray(sizeof(MTD_PeerListNode), 1);
	MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pPeerArray, 0);
	memset(pNode, 0, sizeof(MTD_PeerListNode));
	
	pNode->uidChar	= pObj->GetUID();
	pNode->dwIP		= pObj->GetIP();
	pNode->nPort	= pObj->GetPort();

	CopyCharInfoForTrans(&pNode->CharInfo, pObj->GetCharInfo(), pObj);
	//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® 	CopyCharBuffInfoForTrans(&pNode->CharBuffInfo, pObj->GetCharInfo(), pObj);

	pNode->ExtendInfo.nPlayerFlags = pObj->GetPlayerFlags();
	if (pStage->GetStageSetting()->IsTeamPlay())	pNode->ExtendInfo.nTeam = (char)pObj->GetTeam();
	else											pNode->ExtendInfo.nTeam = 0;	

	pNew->AddParameter(new MCommandParameterBlob(pPeerArray, MGetBlobArraySize(pPeerArray)));
	MEraseBlobArray(pPeerArray);

	RouteToStage(uidStage, pNew);

	// ¹èÆ² ½ÃÀÛ½Ã°£ ¼¼ÆÃ
	pObj->GetCharInfo()->m_nBattleStartTime = MMatchServer::GetInstance()->GetGlobalClockCount();
	pObj->GetCharInfo()->m_nBattleStartXP = pObj->GetCharInfo()->m_nXP;

	// ¶ó¿ìÆÃ ÈÄ¿¡ ³Ö¾î¾ß ÇÑ´Ù.
	return ExceptionTraceStageEnterBattle( pObj, pStage );
}

bool MMatchServer::StageLeaveBattle(const MUID& uidPlayer, bool bGameFinishLeaveBattle, bool bForcedLeave)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	if (pObj->GetPlace() != MMP_BATTLE) { return false; }

	// MMatchStage* pStage = FindStage(uidStage);

	//if(pObj->GetStageUID()!=uidStage)
	//	mlog(" stage leave battle hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);

	const MUID uidStage = pObj->GetStageUID();

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)
	{	// Å¬·£Àü½Ã ÇÑÁ·ÀÌ ´Ù³ª°¡¸é ½ºÅ×ÀÌÁö°¡ ¾ø¾îÁö¹Ç·Î ¿©±â¼­ agent¸¦ ²÷¾îÁØ´Ù. 
		if (pObj->GetRelayPeer()) {
			MAgentObject* pAgent = GetAgent(pObj->GetAgentUID());
			if (pAgent) {
				MCommand* pCmd = CreateCommand(MC_AGENT_PEER_UNBIND, pAgent->GetCommListener());
				pCmd->AddParameter(new MCmdParamUID(uidPlayer));
				Post(pCmd);
			}
		}

		UpdateCharDBCachingData(pObj);		///< XP, BP, KillCount, DeathCount Ä³½³ ¾÷µ¥ÀÌÆ®
		UpdateCharItemDBCachingData(pObj);	///< Character Item¿¡¼­ ¾÷µ¥ÀÌÆ®°¡ ÇÊ¿äÇÑ °Íµé ¾÷µ¥ÀÌÆ®
		//CheckSpendableItemCounts(pObj);		///< Ç×»ó UpdateCharItemDBCachingData µÚ¿¡ ÀÖ¾î¾ß ÇÕ´Ï´Ù.
		
		ProcessCharPlayInfo(pObj);			///< Ä³¸¯ÅÍ ÇÃ·¹ÀÌÇÑ Á¤º¸ ¾÷µ¥ÀÌÆ® 
		return false;
	}
	else
	{
		// ÇÃ·¹ÀÌ Á÷ÈÄ ´ë±â½Ç¿¡¼­ °æÇèÄ¡, ½Â/ÆÐ, ½Â·ü, ¹Ù¿îÆ¼°¡ ¹Ý¿µµÇÁö ¾Ê½À´Ï´Ù. - by kammir 2008.09.19
		// LeaveBattle°¡ µÇ¸é¼­ Ä³¸¯ÅÍ µ¥ÀÌÅÍ¸¦ ¾÷µ¥ÀÌÆ® ÇØÁØ´Ù.
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.Reset();
		for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
			MMatchObject* pScanObj = (MMatchObject*)(*i).second;
			CacheBuilder.AddObject(pScanObj);
		}
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
		RouteToListener(pObj, pCmdCacheUpdate);
	}

	pStage->LeaveBattle(pObj);
	pObj->SetPlace(MMP_STAGE);


	// ·¹º§¿¡ ¾È¸Â´Â Àåºñ¾ÆÀÌÅÛ Ã¼Å©
#define LEGAL_ITEMLEVEL_DIFF		3
	bool bIsCorrect = true;
	for (int i = 0; i < MMCIP_END; i++) {
		if (CorrectEquipmentByLevel(pObj, MMatchCharItemParts(i), LEGAL_ITEMLEVEL_DIFF)) {
			bIsCorrect = false;
		}
	}

	if (!bIsCorrect) {
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_RESULT, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterInt(MERR_TAKEOFF_ITEM_BY_LEVELDOWN));
		RouteToListener(pObj, pNewCmd);
	}
	
	CheckExpiredItems(pObj);		//< ±â°£ ¸¸·á ¾ÆÀÌÅÛÀÌ ÀÖ´ÂÁö Ã¼Å©

	if (pObj->GetRelayPeer()) {
		MAgentObject* pAgent = GetAgent(pObj->GetAgentUID());
		if (pAgent) {
			MCommand* pCmd = CreateCommand(MC_AGENT_PEER_UNBIND, pAgent->GetCommListener());
			pCmd->AddParameter(new MCmdParamUID(uidPlayer));
			Post(pCmd);
		}
	}	

	// Ä³¸¯ÅÍ ÇÃ·¹ÀÌÇÑ Á¤º¸ ¾÷µ¥ÀÌÆ® 
	ProcessCharPlayInfo(pObj);

	//=======================================================================================================================================
	
	bool bIsLeaveAllBattle = true;
	
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pAllObj = (MMatchObject*)GetObject(uidObj);
		if(NULL == pAllObj) continue;
		if(MMP_STAGE != pAllObj->GetPlace()) { 
			bIsLeaveAllBattle = false; 
			break; 
		}
	}


	if(pStage->IsRelayMap())
	{
		if(bGameFinishLeaveBattle)
		{	// ¸±·¹ÀÌ¸Ê, ¹èÆ² Á¾·á·Î ½ºÅ×ÀÌÁö·Î ³ª¿ÔÀ»¶§
			if(!pStage->m_bIsLastRelayMap)
			{	// ´ÙÀ½¸ÊÀÌ ÀÖ´Ù¸é ¹Ù·Î ´ÙÀ½ ¸Ê½ÃÀÛ Ã³¸®		

				if( !bForcedLeave ) 
				{
					pObj->SetStageState(MOSS_READY);
				}

				if( bIsLeaveAllBattle ) 
				{					
					OnStageRelayStart(uidStage);
				} 

				MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MUID(0,0));
				pNew->AddParameter(new MCommandParameterUID(uidPlayer));
				pNew->AddParameter(new MCommandParameterBool(true));
				RouteToStage(uidStage, pNew);
			}
		}
		else
		{	///< ¸ÞÀÎ ¸Þ´º·Î ½ºÅ×ÀÌÁö¿¡ ³ª¿È		
			MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MUID(0,0));
			pNew->AddParameter(new MCommandParameterUID(uidPlayer));
			pNew->AddParameter(new MCommandParameterBool(false));
			RouteToStage(uidStage, pNew);			

			if(bIsLeaveAllBattle) 
			{	///< ¸ðµÎ ½ºÅ×ÀÌÁö¿¡ ÀÖ´Ù¸é ¸±·¹ÀÌ¸Ê ¼¼ÆÃÀ» ´Ù½Ã ÇØÁØ´Ù.
				pStage->m_bIsLastRelayMap = true;//¸±·¹ÀÌ¸ÊÀ» ³¡³½´Ù
				pStage->GetStageSetting()->SetMapName(MMATCH_MAPNAME_RELAYMAP);
				pStage->SetRelayMapCurrList(pStage->GetRelayMapList());
				pStage->m_RelayMapRepeatCountRemained = pStage->GetRelayMapRepeatCount();
			}
		}
	} 
	else 
	{
		MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MUID(0,0));
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterBool(false));
		RouteToStage(uidStage, pNew);
	}

	//=======================================================================================================================================

	// ¹æ¿¡¼­ ³ª°¡¸é noready»óÅÂ·Î º¯°æµÈ´Ù. 
	// º¯°æµÈ Á¤º¸¸¦ ½ºÅ×ÀÌÁöÀÇ ¸ðµç Å¬¶óÀÌ¾ðÆ®·Î º¸³»ÁÜ. - by SungE 2007-06-04
	StagePlayerState( uidPlayer, pStage->GetUID(), pObj->GetStageState() );	
	
	UpdateCharDBCachingData(pObj);		///< XP, BP, KillCount, DeathCount Ä³½³ ¾÷µ¥ÀÌÆ®
	UpdateCharItemDBCachingData(pObj);	///< Character Item¿¡¼­ ¾÷µ¥ÀÌÆ®°¡ ÇÊ¿äÇÑ °Íµé ¾÷µ¥ÀÌÆ®
	//CheckSpendableItemCounts(pObj);		///< Ç×»ó UpdateCharItemDBCachingData µÚ¿¡ ÀÖ¾î¾ß ÇÕ´Ï´Ù.

	return true;
}

bool MMatchServer::StageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat, int nR, int nG, int nB)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)	return false;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return false;

	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_CHAT_LIMITED) return false;

	if (uidStage != pObj->GetStageUID())
	{
		return false;
	}


	if ((timeGetTime() - pObj->st_nLastStageChat) < 900)
	{
		return false;
	}
	pObj->st_nLastStageChat = timeGetTime();


	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_CHAT), MUID(0, 0), m_This);
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterString(pszChat));

#ifdef _GZFGunz
	pCmd->AddParameter(new MCommandParameterInt(nR));
	pCmd->AddParameter(new MCommandParameterInt(nG));
	pCmd->AddParameter(new MCommandParameterInt(nB));
#endif
	RouteToStage(uidStage, pCmd);
	return true;
}

bool MMatchServer::StageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	pStage->PlayerTeam(uidPlayer, nTeam);

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_TEAM, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterUInt(nTeam));

	RouteToStageWaitRoom(uidStage, pCmd);
	return true;
}

bool MMatchServer::StagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	// MMatchStage* pStage = FindStage(uidStage);
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return false;

	pStage->PlayerState(uidPlayer, nStageState);
	
	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_PLAYER_STATE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterInt(nStageState));
	RouteToStage(uidStage, pCmd);
	return true;
}

bool MMatchServer::StageMaster(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	MUID uidMaster = pStage->GetMasterUID();

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToStage(uidStage, pCmd);

	return true;
}

void MMatchServer::StageLaunch(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	ReserveAgent(pStage);

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_LAUNCH, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidStage));
	pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
	RouteToStage(uidStage, pCmd);
}

void MMatchServer::StageRelayLaunch(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	ReserveAgent(pStage);

	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if( pObj->GetStageState() == MOSS_READY) {
				MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RELAY_LAUNCH, MUID(0,0));
				pCmd->AddParameter(new MCmdParamUID(uidStage));
				pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pStage->GetMapName())));
				pCmd->AddParameter(new MCmdParamBool(false));
				RouteToListener(pObj, pCmd);
			} else {
				MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RELAY_LAUNCH, MUID(0,0));
				pCmd->AddParameter(new MCmdParamUID(uidStage));
				pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pStage->GetMapName())));
				pCmd->AddParameter(new MCmdParamBool(true));
				RouteToListener(pObj, pCmd);
			}
		} else {
			LOG(LOG_PROG, "WARNING(StageRelayLaunch) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i=pStage->RemoveObject(uidObj);
			LogObjectCommandHistory(uidObj);
		}
	}
}

void MMatchServer::StageFinishGame(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	bool bIsRelayMapUnFinish = true;
	if(pStage->GetStageType() == MST_LADDER || pStage->GetStageType() == MST_NORMAL)
	{
		ClanReDef::iterator it = ClanRejoiner.begin();
		while(it != ClanRejoiner.end())
		{
			if(it->second->StageUID == uidStage)
			{
				ClanRejoiner.erase(it++);
			}
			else {
				++it;
			}
		}
	}

	if(pStage->IsRelayMap())
	{ // ¸±·¹ÀÌ ¸ÊÀÏ¶§¿¡´Â ¹èÆ²À» ´Ù½Ã ½ÃÀÛÇØÁØ´Ù. 
		if((int)pStage->m_vecRelayMapsRemained.size() <= 0)
		{	// ³²Àº ¸ÊÀÌ ¾øÀ»¶§
			int nRepeatCount = (int)pStage->m_RelayMapRepeatCountRemained - 1;
			if(nRepeatCount < 0)
			{
				bIsRelayMapUnFinish = false;

				pStage->m_bIsLastRelayMap = true;//¸±·¹ÀÌ¸ÊÀ» ³¡³½´Ù				
				nRepeatCount = 0;
				pStage->GetStageSetting()->SetMapName(MMATCH_MAPNAME_RELAYMAP);	//"RelayMap" ¼¼ÆÃ
			}
			pStage->m_RelayMapRepeatCountRemained = (RELAY_MAP_REPEAT_COUNT)nRepeatCount;
			pStage->SetRelayMapCurrList(pStage->GetRelayMapList());
		}

		if(!pStage->m_bIsLastRelayMap) {
			// Ã³À½ ½ÃÀÛ½Ã, Flag¸¦ On½ÃÄÑÁØ´Ù. 
			if( pStage->IsStartRelayMap() == false ) {
				pStage->SetIsStartRelayMap(true);
			}			

			if((int)pStage->m_vecRelayMapsRemained.size() > 0) { // ´ÙÀ½¸ÊÀÌ ÀÖ´Ù¸é
				int nRelayMapIndex = 0;

				if(pStage->GetRelayMapType() == RELAY_MAP_TURN) {	//< ³²Àº °ÍÁß¿¡¼­ Ã¹ ¹øÂ°ºÎÅÍ ½ÃÀÛ(°¡µ¶¼º)
					nRelayMapIndex = 0; 
				} else if(pStage->GetRelayMapType() == RELAY_MAP_RANDOM) {
					nRelayMapIndex = rand() % (int)pStage->m_vecRelayMapsRemained.size();
				}

				if(nRelayMapIndex >= MAX_RELAYMAP_LIST_COUNT) { //< ¸Ê ±¸¼ºÀÌ 20°³
					mlog("StageFinishGame RelayMap Fail RelayMapList MIsCorrect MaxCount[%d] \n", (int)nRelayMapIndex);
					return;
				}

				char* szMapName = (char*)MGetMapDescMgr()->GetMapName(pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
				if (!szMapName)
				{
					mlog("RelayMapBattleStart Fail MapID[%d] \n", (int)pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
					return;
				}

				pStage->GetStageSetting()->SetMapName(szMapName);

				// ½ÇÇàÇÑ ¸±·¹ÀÌ¸ÊÀº »èÁ¦ÇØÁØ´Ù.
				vector<RelayMap>::iterator itor = pStage->m_vecRelayMapsRemained.begin();
				for(int i=0 ; nRelayMapIndex > i ; ++itor, ++i);// ÇØ´ç ÀÎµ¦½º±îÁö ÀÌµ¿
				pStage->m_vecRelayMapsRemained.erase(itor);
			} 
			else {
				mlog("MMatchServer::StageFinishGame::IsRelayMap() - m_vecRelayMapsRemained.size() == 0\n");
			}
		} else {
			pStage->SetIsStartRelayMap(false);
			bIsRelayMapUnFinish = false; // ¸±·¹ÀÌ¸Ê ÁøÇàÀÌ ³¡³µÀ½
		}
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_FINISH_GAME, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterBool(bIsRelayMapUnFinish));
	RouteToStage(uidStage, pCmd);

	return;
}

MCommand* MMatchServer::CreateCmdResponseStageSetting(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return NULL;

	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_STAGESETTING, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	MMatchStageSetting* pSetting = pStage->GetStageSetting();

	// Param 1 : Stage Settings
	void* pStageSettingArray = MMakeBlobArray(sizeof(MSTAGE_SETTING_NODE), 1);
	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageSettingArray, 0);
	CopyMemory(pNode, pSetting->GetStageSetting(), sizeof(MSTAGE_SETTING_NODE));
	pCmd->AddParameter(new MCommandParameterBlob(pStageSettingArray, MGetBlobArraySize(pStageSettingArray)));
	MEraseBlobArray(pStageSettingArray);

	// Param 2 : Char Settings
	int nCharCount = (int)pStage->GetObjCount();
	void* pCharArray = MMakeBlobArray(sizeof(MSTAGE_CHAR_SETTING_NODE), nCharCount);
	int nIndex=0;
	for (MUIDRefCache::iterator itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) {
		MSTAGE_CHAR_SETTING_NODE* pCharNode = (MSTAGE_CHAR_SETTING_NODE*)MGetBlobArrayElement(pCharArray, nIndex++);
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		pCharNode->uidChar = pObj->GetUID();
		pCharNode->nTeam = pObj->GetTeam();
		pCharNode->nState = pObj->GetStageState();
	}
	pCmd->AddParameter(new MCommandParameterBlob(pCharArray, MGetBlobArraySize(pCharArray)));
	MEraseBlobArray(pCharArray);

	// Param 3 : Stage State
	pCmd->AddParameter(new MCommandParameterInt((int)pStage->GetState()));

	// Param 4 : Stage Master
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetMasterUID()));

	return pCmd;
}



void MMatchServer::OnStageCreate(const MUID& uidChar, char* pszStageName, bool bPrivate, char* pszStagePassword)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return;


	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN)
		&& (pChannel->GetChannelType() == MCHANNEL_TYPE_DUELTOURNAMENT)) {
		return;
	}
	
	MUID uidStage;

	if (strstr(pChannel->GetName(), "Event") && (!IsAdminGrade(pObj)) && (pChannel->GetChannelType() != MCHANNEL_TYPE_PRIVATE))
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_CREATE, MERR_CANNOT_CREATE_STAGE);
		return;
	}
	if (!StageAdd(pChannel, pszStageName, bPrivate, pszStagePassword, &uidStage))
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_CREATE, MERR_CANNOT_CREATE_STAGE);
		return;
	}
	StageJoin(uidChar, uidStage, false, MMT_BLUE);

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage)
		pStage->SetFirstMasterName(pObj->GetCharInfo()->m_szName);
}


//void MMatchServer::OnStageJoin(const MUID& uidChar, const MUID& uidStage)
//{
//	MMatchObject* pObj = GetObject(uidChar);
//	if (pObj == NULL) return;
//
//	MMatchStage* pStage = NULL;
//
//	if (uidStage == MUID(0,0)) {
//		return;
//	} else {
//		pStage = FindStage(uidStage);
//	}
//
//	if (pStage == NULL) {
//		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_STAGE_NOT_EXIST);
//		return;
//	}
//
//	if ((IsAdminGrade(pObj) == false) && pStage->IsPrivate())
//	{
//		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_CANNOT_JOIN_STAGE_BY_PASSWORD);
//		return;
//	}
//
//	StageJoin(uidChar, pStage->GetUID());
//}

void MMatchServer::OnPrivateStageJoin(const MUID& uidPlayer, const MUID& uidStage, char* pszPassword)
{
	if (strlen(pszPassword) > STAGEPASSWD_LENGTH) return;

	MMatchStage* pStage = NULL;

	if (uidStage == MUID(0,0)) 
	{
		return;
	} 
	else 
	{
		pStage = FindStage(uidStage);
	}

	if (pStage == NULL) 
	{
		MMatchObject* pObj = GetObject(uidPlayer);
		if (pObj != NULL)
		{
			RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_STAGE_NOT_EXIST);
		}

		return;
	}

	// ¿µÀÚ³ª °³¹ßÀÚ¸é ¹«½Ã..

	bool bSkipPassword = false;

	MMatchObject* pObj = GetObject(uidPlayer);

	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) 
		return;

	MMatchUserGradeID ugid = pObj->GetAccountInfo()->m_nUGrade;

	if (ugid == MMUG_DEVELOPER || ugid == MMUG_ADMIN) 
		bSkipPassword = true;

	// ºñ¹Ð¹æÀÌ ¾Æ´Ï°Å³ª ÆÐ½º¿öµå°¡ Æ²¸®¸é ÆÐ½º¿öµå°¡ Æ²·È´Ù°í ÀÀ´äÇÑ´Ù.
	if(bSkipPassword==false) {
		if ((!pStage->IsPrivate()) || (strcmp(pStage->GetPassword(), pszPassword)))
		{
			MMatchObject* pObj = GetObject(uidPlayer);
			if (pObj != NULL)
			{
				RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_CANNOT_JOIN_STAGE_BY_PASSWORD);
			}

			return;
		}
	}

	StageJoin(uidPlayer, pStage->GetUID(), false, MMT_BLUE);
}

void MMatchServer::OnStageFollow(const MUID& uidPlayer, const char* pszTargetName)
{
	MMatchObject* pPlayerObj = GetObject(uidPlayer);
	if (pPlayerObj == NULL) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;

	// ÀÚ±â ÀÚ½ÅÀ» µû¶ó °¡·Á°í ÇßÀ»°æ¿ì °Ë»ç.
	if (pPlayerObj->GetUID() == pTargetObj->GetUID()) return;

	// ½ºÅ×ÀÌÆ®°¡ Àß¸øµÇ¾î ÀÖ´ÂÁö °Ë»ç.
	if (!pPlayerObj->CheckEnableAction(MMatchObject::MMOA_STAGE_FOLLOW)) return;


	// ¼­·Î ´Ù¸¥ Ã¤³ÎÀÎÁö °Ë»ç.
	if (pPlayerObj->GetChannelUID() != pTargetObj->GetChannelUID()) {

#ifdef _VOTESETTING
		RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW );
#endif
		return;
	}

	if ((IsAdminGrade(pTargetObj) == true)) {
		NotifyMessage(pPlayerObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	MMatchStage* pStage = FindStage(pTargetObj->GetStageUID());
	if (pStage == NULL) return;

	// Å¬·£Àü°ÔÀÓÀº µû¶ó°¥ ¼ö ¾øÀ½
	if (pStage->GetStageType() != MST_NORMAL) return;

	if (pStage->IsPrivate()==false) {
		if ((pStage->GetStageSetting()->GetForcedEntry()==false) && pStage->GetState() != STAGE_STATE_STANDBY) {
			// Deny Join

#ifdef _VOTESETTING
			RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW );
#endif
		} else {
			StageJoin(uidPlayer, pTargetObj->GetStageUID(), false, MMT_BLUE);
		}
	}
	else {
		// µû¶ó°¡·Á´Â ¹æÀÌ ºñ¹Ð¹øÈ£¸¦ ÇÊ¿ä·Î ÇÒ°æ¿ì´Â µû¶ó°¥¼ö ¾øÀ½.
		//RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW_BY_PASSWORD );

		// ÇØ´ç¹æÀÌ ºñ¹Ð¹æÀÌ¸é ºñ¹Ð¹øÈ£¸¦ ¿ä±¸ÇÑ´Ù.
		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_REQUIRE_PASSWORD), MUID(0,0), m_This);
		pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
		pNew->AddParameter(new MCommandParameterString((char*)pStage->GetName()));
		RouteToListener(pPlayerObj, pNew);
	}
}

void MMatchServer::OnStageLeave(const MUID& uidPlayer)//, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( !IsEnabledObject(pObj) ) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if( !IsEnabledObject(GetObject(uidPlayer)) )
	{
		return;
	}

	StageLeave(uidPlayer);// , uidStage);
}

void MMatchServer::OnStageRequestPlayerList(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	// MMatchStage* pStage = FindStage(uidStage);
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// ¹æÀÎ¿ø ¸ñ·Ï
	MMatchObjectCacheBuilder CacheBuilder;
	CacheBuilder.Reset();
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MMatchObject* pScanObj = (MMatchObject*)(*i).second;
		CacheBuilder.AddObject(pScanObj);
	}
    MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
	RouteToListener(pObj, pCmdCacheUpdate);

	// Cast Master(¹æÀå)
	MUID uidMaster = pStage->GetMasterUID();
	MCommand* pMasterCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidStage));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToListener(pObj, pMasterCmd);

	// Cast Character Setting
	StageTeam(uidPlayer, uidStage, pObj->GetTeam());
	StagePlayerState(uidPlayer, uidStage, pObj->GetStageState());
}

void MMatchServer::OnStageEnterBattle(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	StageEnterBattle(uidPlayer, uidStage);
}

void MMatchServer::OnStageLeaveBattle(const MUID& uidPlayer, bool bGameFinishLeaveBattle)//, const MUID& uidStage)
{
	if( !IsEnabledObject(GetObject(uidPlayer)) )
	{
		return;
	}

	StageLeaveBattle(uidPlayer, bGameFinishLeaveBattle, false);//, uidStage);
}


#include "CMLexicalAnalyzer.h"
// °­Åð ÀÓ½ÃÄÚµå
bool StageKick(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	MMatchStage* pStage = pServer->FindStage(uidStage);
	if (pStage == NULL) return false;
	if (uidPlayer != pStage->GetMasterUID()) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (stricmp(pszCmd, "/kick") == 0) {
				if (lex.GetCount() >= 2) {
					char* pszTarget = lex.GetByStr(1);
					if (pszTarget) {
						for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); 
							itor != pStage->GetObjEnd(); ++itor)
						{
							MMatchObject* pTarget = (MMatchObject*)((*itor).second);
							if (stricmp(pszTarget, pTarget->GetName()) == 0) {
								if (pTarget->GetPlace() != MMP_BATTLE) {
									pServer->StageLeave(pTarget->GetUID());//, uidStage);
									bResult = true;
								}
								break;
							}
						}
					}
				}
			}	// Kick
		}
	}

	lex.Destroy();
	return bResult;
}

// ¹æÀåÈ®ÀÎ ÀÓ½ÃÄÚµå
bool StageShowInfo(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	MMatchStage* pStage = pServer->FindStage(uidStage);
	if (pStage == NULL) return false;
	if (uidPlayer != pStage->GetMasterUID()) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (stricmp(pszCmd, "/showinfo") == 0) {
				char szMsg[256]="";
				sprintf(szMsg, "FirstMaster : (%s)", pStage->GetFirstMasterName());
				pServer->Announce(pChar, szMsg);
				bResult = true;
			}	// ShowInfo
		}
	}

	lex.Destroy();
	return bResult;
}
void MMatchServer::OnStageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat, int nR, int nG, int nB)
{
	if (pszChat[0] == '/') {
		if (StageKick(this, uidPlayer, uidStage, pszChat))
			return;
		if (StageShowInfo(this, uidPlayer, uidStage, pszChat))
			return;
	}

	StageChat(uidPlayer, uidStage, pszChat, nR, nG, nB);
}

void MMatchServer::OnStageStart(const MUID& uidPlayer, const MUID& uidStage, int nCountdown)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetMasterUID() != uidPlayer) return;

	if (pStage->StartGame(MGetServerConfig()->IsUseResourceCRC32CacheCheck()) == true) {
		StageRelayMapBattleStart(uidPlayer, uidStage);

		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_START), MUID(0,0), m_This);
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterUID(uidStage));
		pNew->AddParameter(new MCommandParameterInt(min(nCountdown,3)));
		RouteToStage(uidStage, pNew);

		// µðºñ¿¡ ·Î±×¸¦ ³²±ä´Ù.
		SaveGameLog(uidStage);
	}
}

void MMatchServer::OnStageRelayStart(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	
	if (pStage->StartRelayGame(MGetServerConfig()->IsUseResourceCRC32CacheCheck()) == true) {
		// µðºñ¿¡ ·Î±×¸¦ ³²±ä´Ù.
		SaveGameLog(uidStage);
	}
}

void MMatchServer::OnStartStageList(const MUID& uidComm)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetStageListTransfer(true);
}

void MMatchServer::OnStopStageList(const MUID& uidComm)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetStageListTransfer(false);
}

void MMatchServer::OnStagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	StagePlayerState(uidPlayer, uidStage, nStageState);
}


void MMatchServer::OnStageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MMatchObject* pChar = GetObject(uidPlayer);
	if (pChar == NULL) return;

	StageTeam(uidPlayer, uidStage, nTeam);
}

void MMatchServer::OnStageMap(const MUID& uidStage, char* pszMapName)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ´ë±â»óÅÂ¿¡¼­¸¸ ¹Ù²Ü¼ö ÀÖ´Ù
	if (strlen(pszMapName) < 2) return;

	pStage->SetMapName( pszMapName );
	pStage->SetIsRelayMap(strcmp(MMATCH_MAPNAME_RELAYMAP, pszMapName) == 0);
	
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_MAP), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterString(pStage->GetMapName()));

	if ( MGetGameTypeMgr()->IsQuestDerived( pStage->GetStageSetting()->GetGameType()))
	{
		MMatchRuleBaseQuest* pQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
		pQuest->RefreshStageGameInfo();
	}

    RouteToStage(uidStage, pNew);
}

void MMatchServer::StageRelayMapBattleStart(const MUID& uidPlayer, const MUID& uidStage)
{// ¸±·¹ÀÌ¸Ê ¼±ÅÃÇÏ°í °ÔÀÓ ½ÃÀÛ ¹öÆ° ´©¸£¸é ´ÙÀ½À» ¼öÇàÇÑ´Ù
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetMasterUID() != uidPlayer) return;
	if(!pStage->IsRelayMap()) return;
	
	// °ÔÀÓ Ã³À½½ÃÀÛ½Ã ÃÊ±âÈ­ ÇØÁÖ±â
	pStage->InitCurrRelayMap();

	if (pStage->m_vecRelayMapsRemained.empty()) return;

	if((int)pStage->m_vecRelayMapsRemained.size() > MAX_RELAYMAP_LIST_COUNT)
	{// ¸Ê ±¸¼ºÀÌ 20°³ ÃÊ°úÇÏ¸é ¿¡·¯
		mlog("RelayMapBattleStart Fail RelayMapList MIsCorrect OverCount[%d] \n", (int)pStage->m_vecRelayMapsRemained.size());
		return;
	}

	if (pStage->m_vecRelayMapsRemained.size() != pStage->GetRelayMapListCount())
	{
		mlog("m_vecRelayMapsRemained[%d] != GetRelayMapListCount[%d]\n", (int)pStage->m_vecRelayMapsRemained.size(), pStage->GetRelayMapListCount());
		return;
	}

	// Ã³À½ ½ÇÇàÇÒ ¸ÊÀ» Á¤ÇÑ´Ù
	int nRelayMapIndex = 0;
	if(pStage->GetRelayMapType() == RELAY_MAP_TURN )
		nRelayMapIndex = 0; // ³²Àº°ÍÁß¿¡¼­ Ã³À½¹øÂ° ºÎÅÍ ½ÃÀÛ(°¡µ¶¼º)
	else if(pStage->GetRelayMapType() == RELAY_MAP_RANDOM)
		nRelayMapIndex = rand() % int(pStage->m_vecRelayMapsRemained.size());

	if(MMATCH_MAP_RELAYMAP == pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID)
	{
		mlog("RelayMapBattleStart Fail Type[%d], RoundCount[Curr:%d][%d], ListCount[Curr:%d][%d] \n",  
			pStage->GetRelayMapType(), pStage->m_RelayMapRepeatCountRemained, pStage->GetRelayMapRepeatCount(), (int)pStage->m_vecRelayMapsRemained.size(), pStage->GetRelayMapListCount());
		return;
	}

	char* szMapName = (char*)MGetMapDescMgr()->GetMapName(pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
	if (!szMapName)
	{
		mlog("RelayMapBattleStart Fail MapID[%d] \n", (int)pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
		return;
	}

	pStage->GetStageSetting()->SetMapName(szMapName);

	// ½ÇÇàÇÑ ¸±·¹ÀÌ¸ÊÀº »èÁ¦ÇØÁØ´Ù.
	vector<RelayMap>::iterator itor = pStage->m_vecRelayMapsRemained.begin();
	for(int i=0 ; nRelayMapIndex > i ; ++itor, ++i);// ÇØ´ç ÀÎµ¦½º±îÁö ÀÌµ¿
	pStage->m_vecRelayMapsRemained.erase(itor);
}

void MMatchServer::OnStageRelayMapElementUpdate(const MUID& uidStage, int nType, int nRepeatCount)
{
	MMatchStage* pStage = FindStage(uidStage);
	pStage->SetRelayMapType((RELAY_MAP_TYPE)nType);
	pStage->SetRelayMapRepeatCount((RELAY_MAP_REPEAT_COUNT)nRepeatCount);

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapType()));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapRepeatCount()));
	RouteToStage(uidStage, pNew);
}

void MMatchServer::OnStageRelayMapListUpdate(const MUID& uidStage, int nRelayMapType, int nRelayMapRepeatCount, void* pRelayMapListBlob)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if(!pStage->IsRelayMap()) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ´ë±â»óÅÂ¿¡¼­¸¸ ¹Ù²Ü¼ö ÀÖ´Ù

	// ¸±·¹ÀÌ¸Ê Á¤º¸¸¦ ¼­¹öÂÊ ½ºÅ×ÀÌÁö¸¦ °»½Å
	RelayMap relayMapList[MAX_RELAYMAP_LIST_COUNT];
	for (int i = 0; i < MAX_RELAYMAP_LIST_COUNT; i++)
		relayMapList[i].nMapID = -1;
	int nRelayMapListCount = MGetBlobArrayCount(pRelayMapListBlob);
	if(nRelayMapListCount > MAX_RELAYMAP_LIST_COUNT)
		nRelayMapListCount = MAX_RELAYMAP_LIST_COUNT;
	for (int i = 0; i < nRelayMapListCount; i++)
	{
		MTD_RelayMap* pRelayMap = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, i);
		if(!MGetMapDescMgr()->MIsCorrectMap(pRelayMap->nMapID))
		{
			mlog("OnStageRelayMapListUpdate Fail MIsCorrectMap ID[%d] \n", (int)pRelayMap->nMapID);
			break;
		}
		relayMapList[i].nMapID = pRelayMap->nMapID;
	}

	pStage->SetRelayMapType((RELAY_MAP_TYPE)nRelayMapType);
	pStage->SetRelayMapRepeatCount((RELAY_MAP_REPEAT_COUNT)nRelayMapRepeatCount);
	pStage->SetRelayMapList(relayMapList);
	pStage->InitCurrRelayMap();


	// ºí·° ¸¸µé±â, ¸Ê¸®½ºÆ® ¼¼ÆÃ
	void* pRelayMapListBlob = MMakeBlobArray(sizeof(MTD_RelayMap), pStage->GetRelayMapListCount());
	RelayMap RelayMapList[MAX_RELAYMAP_LIST_COUNT];
	memcpy(RelayMapList, pStage->GetRelayMapList(), sizeof(RelayMap)*MAX_RELAYMAP_LIST_COUNT);
	for (int i = 0; i < pStage->GetRelayMapListCount(); i++)
	{
		MTD_RelayMap* pRelayMapList = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, i);
		pRelayMapList->nMapID = RelayMapList[i].nMapID;
	}

	// ¹æÀåÀÌ º¸³½ ¸±·¹ÀÌ¸Ê Á¤º¸¸¦ ¹æ¿øµé¿¡°Ô º¸³¿
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapType()));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapRepeatCount()));
	pNew->AddParameter(new MCommandParameterBlob(pRelayMapListBlob, MGetBlobArraySize(pRelayMapListBlob)));
	RouteToStage(uidStage, pNew);
}
void MMatchServer::OnStageRelayMapListInfo(const MUID& uidStage, const MUID& uidChar)
{
	MMatchStage* pStage = FindStage(uidStage);
	if(pStage == NULL) return;
	if(!pStage->IsRelayMap()) return;
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;
	// ´ë±â»óÅÂÀÏ¶§ ¹æÀåÀº Ã³¸® ¾ÈÇØÁÜ(¸±·¹ÀÌ¸Ê ÀÛ¼ºÁßÀÏ¼öµµ ÀÖÀ½)
	if(pStage->GetState() == STAGE_STATE_STANDBY && pStage->GetMasterUID() == uidChar) return;	

	// ºí·° ¸¸µé±â, ¸Ê¸®½ºÆ® ¼¼ÆÃ
	void* pRelayMapListBlob = MMakeBlobArray(sizeof(MTD_RelayMap), pStage->GetRelayMapListCount());
	RelayMap RelayMapList[MAX_RELAYMAP_LIST_COUNT];
	memcpy(RelayMapList, pStage->GetRelayMapList(), sizeof(RelayMap)*MAX_RELAYMAP_LIST_COUNT);
	for (int i = 0; i < pStage->GetRelayMapListCount(); i++)
	{
		MTD_RelayMap* pRelayMapList = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, i);
		pRelayMapList->nMapID = RelayMapList[i].nMapID;
	}
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapType()));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapRepeatCount()));
	pNew->AddParameter(new MCommandParameterBlob(pRelayMapListBlob, MGetBlobArraySize(pRelayMapListBlob)));
	MEraseBlobArray(pRelayMapListBlob);

	RouteToListener(pObj, pNew); // ¹æÀåÀÌ ¸±·¹ÀÌ¸Ê ¼³Á¤Áß¿¡ ¾÷µ¥ÀÌÆ®µÈ ¼³Á¤À¸·Î º¯°æ µÉ¼ö°¡ ÀÖÀ½
}

void MMatchServer::OnStageSetting(const MUID& uidPlayer, const MUID& uidStage, void* pStageBlob, int nStageCount)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ´ë±â»óÅÂ¿¡¼­¸¸ ¹Ù²Ü¼ö ÀÖ´Ù
	if (nStageCount <= 0) return;

	// validate
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) {
		mlog(" stage setting invalid object (%d, %d) ignore\n", uidPlayer.High, uidPlayer.Low);
		return;
	}

	if( pObj->GetStageUID()!=uidStage ||  nStageCount!=1 ||
		MGetBlobArraySize(pStageBlob) != (sizeof(MSTAGE_SETTING_NODE)+sizeof(int)*2) )
	{
		mlog(" stage setting hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		LogObjectCommandHistory( uidPlayer );
		return;
	}

	// ¹æÀåÀÌ°Å³ª ¿î¿µÀÚ°¡ ¾Æ´Ñµ¥ ¼¼ÆÃÀ» ¹Ù²Ù¸é ±×³É ¸®ÅÏ
	if (pStage->GetMasterUID() != uidPlayer)
	{
		MMatchObject* pObjMaster = GetObject(uidPlayer);
		if (!IsAdminGrade(pObjMaster)) return;
	}


	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageBlob, 0);

	// let's refactor
	if( (pNode->nGameType < MMATCH_GAMETYPE_DEATHMATCH_SOLO) || (pNode->nGameType >= MMATCH_GAMETYPE_MAX)) {
		mlog(" stage setting game mode hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		LogObjectCommandHistory( uidPlayer );

		// µðºñ¿¡ ³²±âÀÚ.
//		pObj->SetInvalidStageSettingDisconnectWaitInfo();
		pObj->DisconnectHacker( MMHT_INVALIDSTAGESETTING );

		return;
	}

	// ¼­¹ÙÀÌ¹úÀÌ ºñÈ°¼º ¼¼ÆÃÀÎµ¥ ¼­¹ÙÀÌ¹ú ¿äÃ»½Ã
	if( MGetServerConfig()->IsEnabledSurvivalMode()==false && pNode->nGameType==MMATCH_GAMETYPE_SURVIVAL) {
		mlog(" stage setting game mode hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		LogObjectCommandHistory( uidPlayer );
		pObj->DisconnectHacker( MMHT_INVALIDSTAGESETTING );
		return;
	}

	// ±âº»ÀûÀ¸·Î ÃÖ´ë ÀÎ¿øÀÌ STAGE_BASIC_MAX_PLAYERCOUNTÀÌ ³ÑÀ¸¸é STAGE_BASIC_MAX_PLAYERCOUNT·Î ¸ÂÃçÁÜ.
	// ³²Àº ÀÛ¾÷À» ÁøÇàÇÏ¸éÀº °¢ °ÔÀÓ¿¡ ¸Â´Â ÀÎ¿øÀ¸·Î ¼ÂÆÃÀ» ÇÔ. - by SungE 2007-05-14
	if( STAGE_MAX_PLAYERCOUNT < pNode->nMaxPlayers )
		pNode->nMaxPlayers = STAGE_MAX_PLAYERCOUNT;

	// ÀÌ ÀÌ»óÀÇ ¶ó¿îµå ¼ÂÆÃÀº ºÒ°¡´É ÇÏ´Ù. ¹«Á¶°Ç º¸Á¤ÇÑ´Ù. - By SungE 2007-11-07
	if( STAGE__MAX_ROUND < pNode->nRoundMax )
		pNode->nRoundMax = STAGE__MAX_ROUND;

	MMatchStageSetting* pSetting = pStage->GetStageSetting();
	MMatchChannel* pChannel = FindChannel(pStage->GetOwnerChannel());

	bool bCheckChannelRule = true;

	if (QuestTestServer())
	{
		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			bCheckChannelRule = false;
		}
	}

	if ((pChannel) && (bCheckChannelRule))
	{
		// ¼¼ÆÃÇÒ ¼ö ÀÖ´Â ¸Ê, °ÔÀÓÅ¸ÀÔÀÎÁö Ã¼Å©
		MChannelRule* pRule = MGetChannelRuleMgr()->GetRule(pChannel->GetRuleType());
		if (pRule)
		{
			if (!pRule->CheckGameType(pNode->nGameType))
			{
				pNode->nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
			}

			bool bDuelMode = false;
			bool bCTFMode = false;
			if ( pNode->nGameType == MMATCH_GAMETYPE_DUEL)
				bDuelMode = true;

			if ( pNode->nGameType == MMATCH_GAMETYPE_CTF)
				bCTFMode = true;


			if (!pRule->CheckCTFMap(pNode->nMapIndex) && bCTFMode)
			{
				strcpy(pNode->szMapName, MGetMapDescMgr()->GetMapName(MMATCH_MAP_MANSION));
				pNode->nMapIndex = 0;
			}
			if (!pRule->CheckMap(pNode->nMapIndex, bDuelMode) && !bCTFMode)
			{
				strcpy(pNode->szMapName, MGetMapDescMgr()->GetMapName(MMATCH_MAP_MANSION));
				pNode->nMapIndex = 0;
			}
			else if(!bCTFMode)
			{
				strcpy(pNode->szMapName, pSetting->GetMapName());
				pNode->nMapIndex = pSetting->GetMapIndex();
			}
		}
	}

	MMATCH_GAMETYPE nLastGameType = pSetting->GetGameType();

	// Äù½ºÆ® ¸ðµåÀÌ¸é ¹«Á¶°Ç ³­ÀÔºÒ°¡, ÃÖ´ëÀÎ¿ø 4¸íÀ¸·Î ¼¼ÆÃÇÑ´Ù.
	if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
	{
		if (pNode->bForcedEntryEnabled == true) pNode->bForcedEntryEnabled = false;
		pNode->nMaxPlayers = STAGE_QUEST_MAX_PLAYER;
		pNode->nLimitTime = STAGESETTING_LIMITTIME_UNLIMITED;


		// Äù½ºÆ® ¼­¹ö°¡ ¾Æ´Ñµ¥ Äù½ºÆ® °ÔÀÓÀÌ¸é ¼Ö·Îµ¥½º¸ÅÄ¡·Î ¹Ù²Û´Ù.
		if (!QuestTestServer())
		{
			pNode->nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
		}
	}

	// Äù½ºÆ® ¸ðµå¿´´Ù°¡ ´Ù¸¥ ¸ðµå°¡ µÇ¸é '³­ÀÔºÒ°¡'¸¦ Çã¿ëÀ¸·Î º¯°æ
	if (MGetGameTypeMgr()->IsQuestDerived( nLastGameType ) == true &&
		MGetGameTypeMgr()->IsQuestDerived( pNode->nGameType ) == false)
		pNode->bForcedEntryEnabled = true;

	if (!MGetGameTypeMgr()->IsTeamGame(pNode->nGameType))
	{
		pNode->bAutoTeamBalancing = true;
	}

	// ¸±·¹ÀÌ¸Ê ¼¼ÆÃ
	pStage->SetIsRelayMap(strcmp(MMATCH_MAPNAME_RELAYMAP, pNode->szMapName) == 0);
	pStage->SetIsStartRelayMap(false);

	if(!pStage->IsRelayMap())
	{	// ¸±·¹ÀÌ¸ÊÀÌ ¾Æ´Ï¸é ±âº»À¸·Î ÃÊ±âÈ­ ÇØÁØ´Ù.
		pNode->bIsRelayMap = pStage->IsRelayMap();
		pNode->bIsStartRelayMap = pStage->IsStartRelayMap();
		for (int i=0; i<MAX_RELAYMAP_LIST_COUNT; ++i)
			pNode->MapList[i].nMapID = -1;
		pNode->nRelayMapListCount = 0;
		pNode->nRelayMapType = RELAY_MAP_TURN;
		pNode->nRelayMapRepeatCount = RELAY_MAP_3REPEAT;
	}

	if(pNode->bAntiLead) {
		pSetting->SetAntiLead(true);
	} else {
		pSetting->SetAntiLead(true);
	}


	pSetting->UpdateStageSetting(pNode);
	pStage->ChangeRule(pNode->nGameType);


	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	RouteToStage(uidStage, pCmd);


	// °ÔÀÓ ¸ðµå°¡ º¯°æµÇ¾úÀ»°æ¿ì
	if (nLastGameType != pSetting->GetGameType())
	{
		char szNewMap[ MAPNAME_LENGTH ] = {0};

		if (MGetGameTypeMgr()->IsQuestDerived( nLastGameType ) == false &&
			MGetGameTypeMgr()->IsQuestDerived( pSetting->GetGameType() ) == true)
		{
//			OnStageMap(uidStage, GetQuest()->GetSurvivalMapInfo(MSURVIVAL_MAP(0))->szName);
//			OnStageMap(uidStage, pSetting->GetMapName());
			OnStageMap(uidStage, MMATCH_DEFAULT_STAGESETTING_MAPNAME);

			MMatchRuleBaseQuest* pQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule());
			pQuest->RefreshStageGameInfo();
		}
		else if ( (nLastGameType != MMATCH_GAMETYPE_DUEL) && ( pSetting->GetGameType() == MMATCH_GAMETYPE_DUEL))
		{
			strcpy( szNewMap, MGetMapDescMgr()->GetMapName( MMATCH_MAP_HALL));
			OnStageMap(uidStage, szNewMap);
		}
		else if ( ((nLastGameType == MMATCH_GAMETYPE_QUEST) || (nLastGameType == MMATCH_GAMETYPE_SURVIVAL) || (nLastGameType == MMATCH_GAMETYPE_DUEL)) &&
			      ((pSetting->GetGameType() != MMATCH_GAMETYPE_QUEST) && (pSetting->GetGameType() != MMATCH_GAMETYPE_SURVIVAL) && ( pSetting->GetGameType() != MMATCH_GAMETYPE_DUEL)))
		{
			strcpy( szNewMap, MGetMapDescMgr()->GetMapName( MMATCH_MAP_MANSION));
			OnStageMap(uidStage, szNewMap);
		}
	}
}

void MMatchServer::OnRequestStageSetting(const MUID& uidComm, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	pCmd->m_Receiver = uidComm;
	Post(pCmd);

	// ¸Ê ¼±ÅÃÀÌ ¸±·¹ÀÌ¸ÊÀÌ¸é Ã³¸®ÇØÁØ´Ù.
	OnStageRelayMapListInfo(uidStage, uidComm);

	MMatchObject* pChar = GetObject(uidComm);
	if (pChar && (MMUG_EVENTMASTER == pChar->GetAccountInfo()->m_nUGrade)) 	{
		// ÀÌº¥Æ® ¸¶½ºÅÍ¿¡°Ô Ã³À½ ¹æ¸¸µé¾ú´ø »ç¶÷À» ¾Ë·ÁÁØ´Ù
		StageShowInfo(this, uidComm, uidStage, "/showinfo");
	}
}

void MMatchServer::OnRequestPeerList(const MUID& uidChar, const MUID& uidStage)
{
	ResponsePeerList(uidChar, uidStage);
}

void MMatchServer::OnRequestGameInfo(const MUID& uidChar, const MUID& uidStage)
{
	ResponseGameInfo(uidChar, uidStage);
}

void MMatchServer::ResponseGameInfo(const MUID& uidChar, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage); if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar); if (pObj == NULL) return;
	if (pStage->GetRule() == NULL) return;

	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_GAME_INFO, MUID(0,0));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	// °ÔÀÓÁ¤º¸
	void* pGameInfoArray = MMakeBlobArray(sizeof(MTD_GameInfo), 1);
	MTD_GameInfo* pGameItem = (MTD_GameInfo*)MGetBlobArrayElement(pGameInfoArray, 0);
	memset(pGameItem, 0, sizeof(MTD_GameInfo));
	
	if (pStage->GetStageSetting()->IsTeamPlay())
	{
		pGameItem->nRedTeamScore = static_cast<char>(pStage->GetTeamScore(MMT_RED));
		pGameItem->nBlueTeamScore = static_cast<char>(pStage->GetTeamScore(MMT_BLUE));

		pGameItem->nRedTeamKills = static_cast<short>(pStage->GetTeamKills(MMT_RED));
		pGameItem->nBlueTeamKills = static_cast<short>(pStage->GetTeamKills(MMT_BLUE));
	}

	pNew->AddParameter(new MCommandParameterBlob(pGameInfoArray, MGetBlobArraySize(pGameInfoArray)));
	MEraseBlobArray(pGameInfoArray);

	// ·êÁ¤º¸
	void* pRuleInfoArray = NULL;
	if (pStage->GetRule())
		pRuleInfoArray = pStage->GetRule()->CreateRuleInfoBlob();
	if (pRuleInfoArray == NULL)
		pRuleInfoArray = MMakeBlobArray(0, 0);
	pNew->AddParameter(new MCommandParameterBlob(pRuleInfoArray, MGetBlobArraySize(pRuleInfoArray)));
	MEraseBlobArray(pRuleInfoArray);

	// Battle¿¡ µé¾î°£ »ç¶÷¸¸ List¸¦ ¸¸µç´Ù.
	int nPlayerCount = pStage->GetObjInBattleCount();

	void* pPlayerItemArray = MMakeBlobArray(sizeof(MTD_GameInfoPlayerItem), nPlayerCount);
	int nIndex=0;
	for (MUIDRefCache::iterator itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		if (pObj->GetEnterBattle() == false) continue;

		MTD_GameInfoPlayerItem* pPlayerItem = (MTD_GameInfoPlayerItem*)MGetBlobArrayElement(pPlayerItemArray, nIndex++);
		pPlayerItem->uidPlayer = pObj->GetUID();
		pPlayerItem->bAlive = pObj->CheckAlive();
		pPlayerItem->nKillCount = pObj->GetAllRoundKillCount();
		pPlayerItem->nDeathCount = pObj->GetAllRoundDeathCount();
	}
	pNew->AddParameter(new MCommandParameterBlob(pPlayerItemArray, MGetBlobArraySize(pPlayerItemArray)));
	MEraseBlobArray(pPlayerItemArray);

	RouteToListener(pObj, pNew);
}

void MMatchServer::OnMatchLoadingComplete(const MUID& uidPlayer, int nPercent)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_LOADING_COMPLETE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidPlayer));
	pCmd->AddParameter(new MCmdParamInt(nPercent));
	RouteToStage(pObj->GetStageUID(), pCmd);	
}


void MMatchServer::OnGameRoundState(const MUID& uidStage, int nState, int nRound)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	pStage->RoundStateFromClient(uidStage, nState, nRound);
}


void MMatchServer::OnDuelSetObserver(const MUID& uidChar)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_SET_OBSERVER, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidChar));
	RouteToBattle(pObj->GetStageUID(), pCmd);
}

void MMatchServer::OnRequestSpawn(const MUID& uidChar, const MVector& pos, const MVector& dir)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	// Do Not Spawn when AdminHiding
	if (IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) return;


	// ¸¶Áö¸· Á×¾ú´ø ½Ã°£°ú »õ·Î ¸®½ºÆùÀ» ¿äÃ»ÇÑ ½Ã°£ »çÀÌ¿¡ 2ÃÊ ÀÌ»óÀÇ ½Ã°£ÀÌ ÀÖ¾ú´ÂÁö °Ë»çÇÑ´Ù.
	DWORD dwTime = timeGetTime() - pObj->GetLastSpawnTime();	
	if ( dwTime < RESPAWN_DELAYTIME_AFTER_DYING_MIN) return;
	pObj->SetLastSpawnTime(timeGetTime());

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;
	if ( (pStage->GetRule()->GetRoundState() != MMATCH_ROUNDSTATE_PREPARE) && (!pObj->IsEnabledRespawnDeathTime(GetTickTime())) )
		 return;

	MMatchRule* pRule = pStage->GetRule();					// ÀÌ·± ½ÄÀÇ ÄÚµå´Â ¸¶À½¿¡ ¾ÈµéÁö¸¸ -_-; °ÔÀÓÅ¸ÀÔ º¸°í ¿¹¿ÜÃ³¸®.
	MMATCH_GAMETYPE gameType = pRule->GetGameType();		// ´Ù¸¥ ¹æ¹ý ÀÖ³ª¿ä.
	if (gameType == MMATCH_GAMETYPE_DUEL)
	{
		MMatchRuleDuel* pDuel = (MMatchRuleDuel*)pRule;		// RTTI ¾È½á¼­ dynamic cast´Â ÆÐ½º.. ¿¹¿ÜÃ³¸®µµ Â¥Áõ³ª°í -,.-
		if (uidChar != pDuel->uidChampion && uidChar != pDuel->uidChallenger)
		{
			OnDuelSetObserver(uidChar);
			return;
		}
	}

	pObj->ResetCustomItemUseCount();
	pObj->SetAlive(true);

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_RESPONSE_SPAWN, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidChar));
	pCmd->AddParameter(new MCmdParamShortVector(pos.x, pos.y, pos.z));
	pCmd->AddParameter(new MCmdParamShortVector(DirElementToShort(dir.x), DirElementToShort(dir.y), DirElementToShort(dir.z)));
	RouteToBattle(pObj->GetStageUID(), pCmd);
}

void MMatchServer::OnGameRequestTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	MMatchTimeSyncInfo* pSync = pObj->GetSyncInfo();
	pSync->Update(GetGlobalClockCount());

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_RESPONSE_TIMESYNC, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUInt(nLocalTimeStamp));
	pCmd->AddParameter(new MCmdParamUInt(GetGlobalClockCount()));
	RouteToListener(pObj, pCmd);
}

void MMatchServer::OnGameReportTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp, unsigned int nDataChecksum)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->UpdateTickLastPacketRecved();	// Last Packet Timestamp

	if (pObj->GetEnterBattle() == false)
		return;

	//// SpeedHack Test ////
	MMatchTimeSyncInfo* pSync = pObj->GetSyncInfo();
	int nSyncDiff = nLocalTimeStamp - pSync->GetLastSyncClock();
	float fError = static_cast<float>(nSyncDiff) / static_cast<float>(MATCH_CYCLE_CHECK_SPEEDHACK);
	if (fError > 2.f) {	
		pSync->AddFoulCount();
		if (pSync->GetFoulCount() >= 3) {	// 3¿¬¼Ó ½ºÇÇµåÇÙ °ËÃâ - 3Áø¾Æ¿ô

			#ifndef _DEBUG		// µð¹ö±×ÇÒ¶§´Â »©³õ¾ÒÀ½
				NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GAME_SPEEDHACK);
				StageLeave(pObj->GetUID());//, pObj->GetStageUID());
				Disconnect(pObj->GetUID());
			#endif
			mlog("SPEEDHACK : User='%s', SyncRatio=%f (TimeDiff=%d) \n", 
				pObj->GetName(), fError, nSyncDiff);
			pSync->ResetFoulCount();
		}
	} else {
		pSync->ResetFoulCount();
	}
	pSync->Update(GetGlobalClockCount());

	//// MemoryHack Test ////
	if (nDataChecksum > 0) {	// ¼­¹ö°¡ Client MemoryChecksum ¸ð¸£¹Ç·Î ÀÏ´Ü Å¬¶óÀÌ¾ðÆ®°¡ ½Å°íÇÏ´ÂÀÇ¹Ì·Î »ç¿ëÇÑ´Ù.
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GAME_MEMORYHACK);
		StageLeave(pObj->GetUID());//, pObj->GetStageUID());
		Disconnect(pObj->GetUID());
		mlog("MEMORYHACK : User='%s', MemoryChecksum=%u \n", pObj->GetName(), nDataChecksum);
	}
}

void MMatchServer::OnUpdateFinishedRound(const MUID& uidStage, const MUID& uidChar, 
						   void* pPeerInfo, void* pKillInfo)
{

}

void MMatchServer::OnRequestForcedEntry(const MUID& uidStage, const MUID& uidChar)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar);	
	if (pObj == NULL) return;

	pObj->SetForcedEntry(true);

	RouteResponseToListener(pObj, MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY, MOK);
}

void MMatchServer::OnRequestSuicide(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->ReserveSuicide( uidPlayer, MGetMatchServer()->GetGlobalClockCount() );

	// OnGameKill(uidPlayer, uidPlayer);

	//MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SUICIDE, MUID(0,0));
	//int nResult = MOK;
	//pNew->AddParameter(new MCommandParameterInt(nResult));
	//pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	//RouteToBattle(pObj->GetStageUID(), pNew);
}

void MMatchServer::OnRequestObtainWorldItem(const MUID& uidPlayer, const int nItemUID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->ObtainWorldItem(pObj, nItemUID);
}

void MMatchServer::OnRequestFlagCap(const MUID& uidPlayer, const int nItemID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if(pStage->GetStageSetting())
	{
		if(pStage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_CTF)
		{
			MMatchRuleTeamCTF* pRule = (MMatchRuleTeamCTF*)pStage->GetRule();
			if(pRule)
			{
			pRule->OnObtainWorldItem( pObj, nItemID, NULL );
			}
		}
	}
}

void MMatchServer::OnRequestSpawnWorldItem(const MUID& uidPlayer, const int nItemID, const float x, const float y, const float z, float fDropDelayTime)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if( !pObj->IsHaveCustomItem() )
		return;

	/*
	Âø¿ëÇÏ°í ÀÖ´Â ÂÊÀ» ¾Ë ¼ö ¾øÀ¸¹Ç·Î µÎÂÊÀÇ ÇÕÀ¸·Î ÃÖ´ë »ç¿ëÇÒ ¼ö ÀÖ´Â ¼ö·®À» ±¸ÇÑÈÄ
	±× ÇÑµµ ¾È¿¡¼­ »ç¿ëÇÒ ¼ö ÀÖµµ·Ï ÇÑ´Ù.
	*/
	if( pObj->IncreaseCustomItemUseCount() )
	{
		pStage->RequestSpawnWorldItem(pObj, nItemID, x, y, z, fDropDelayTime);
	}
}

void MMatchServer::OnNotifyThrowTrapItem(const MUID& uidPlayer, const int nItemID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (!pObj->IsEquipCustomItem(nItemID))
		return;

	pStage->OnNotifyThrowTrapItem(uidPlayer, nItemID);
}

void MMatchServer::OnNotifyActivatedTrapItem(const MUID& uidPlayer, const int nItemID, const MVector3& pos)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->OnNotifyActivatedTrapItem(uidPlayer, nItemID, pos);
}

float MMatchServer::GetDuelVictoryMultiflier(int nVictorty)
{
	return 1.0f;
}

float MMatchServer::GetDuelPlayersMultiflier(int nPlayerCount)
{
	return 1.0f;
}

void MMatchServer::CalcExpOnGameKill(MMatchStage* pStage, MMatchObject* pAttacker, MMatchObject* pVictim, 
					   int* poutAttackerExp, int* poutVictimExp)
{
	bool bSuicide = false;		// ÀÚ»ì
	if (pAttacker == pVictim) bSuicide = true;		

	MMATCH_GAMETYPE nGameType = pStage->GetStageSetting()->GetGameType();
	float fGameExpRatio = MGetGameTypeMgr()->GetInfo(nGameType)->fGameExpRatio;
	if (nGameType == MMATCH_GAMETYPE_TRAINING)
	{
		*poutAttackerExp = 0;
		*poutVictimExp = 0;
		return;
	}
	else if (nGameType == MMATCH_GAMETYPE_BERSERKER)
	{
		MMatchRuleBerserker* pRuleBerserker = (MMatchRuleBerserker*)pStage->GetRule();

		if (pRuleBerserker->GetBerserker() == pAttacker->GetUID())
		{
			if (pAttacker != pVictim)
			{
				// ¹ö¼­Ä¿´Â °æÇèÄ¡¸¦ 80%¸¸ È¹µæÇÑ´Ù.
				fGameExpRatio = fGameExpRatio * 0.8f;
			}
			else
			{
				// ¹ö¼­Ä¿´Â ÀÚ»ì ¶Ç´Â ÇÇ°¡ ÁÙ¾î Á×´Â°æ¿ì ¼Õ½Ç °æÇèÄ¡´Â ¾øµµ·Ï ÇÑ´Ù.
				fGameExpRatio = 0.0f;
			}
		}
	}
	else if (nGameType == MMATCH_GAMETYPE_DUEL)
	{
		MMatchRuleDuel* pRuleDuel = (MMatchRuleDuel*)pStage->GetRule();
		if (pVictim->GetUID() == pRuleDuel->uidChallenger)
		{
			fGameExpRatio *= GetDuelVictoryMultiflier(pRuleDuel->GetVictory());
		}
		else
		{
			fGameExpRatio *= GetDuelVictoryMultiflier(pRuleDuel->GetVictory()) * GetDuelPlayersMultiflier(pStage->GetPlayers());

		}
//		if (pRuleDuel->GetVictory() <= 1)
//		{
//			fGameExpRatio = fGameExpRatio * GetDuelPlayersMultiflier(pStage->GetPlayers()) * GetDuelVictoryMultiflier()
//		}
	}
	else if (nGameType == MMATCH_GAMETYPE_CTF)
	{
			MMatchRuleTeamCTF* pRuleCTF = (MMatchRuleTeamCTF*)pStage->GetRule();
			if (pAttacker != pVictim)
			{
				// ¹ö¼­Ä¿´Â °æÇèÄ¡¸¦ 80%¸¸ È¹µæÇÑ´Ù.
				if(!(pAttacker->GetTeam() == pVictim->GetTeam()) && (pVictim->GetUID() == pRuleCTF->GetBlueCarrier() || pVictim->GetUID() == pRuleCTF->GetRedCarrier()))
				fGameExpRatio = fGameExpRatio * 2;
			}
			else
			{
				// ¹ö¼­Ä¿´Â ÀÚ»ì ¶Ç´Â ÇÇ°¡ ÁÙ¾î Á×´Â°æ¿ì ¼Õ½Ç °æÇèÄ¡´Â ¾øµµ·Ï ÇÑ´Ù.
				fGameExpRatio = 0.0f;
			}
	}

	// ¸Ê, °ÔÀÓÅ¸ÀÔ¿¡ ´ëÇÑ °æÇèÄ¡ ºñÀ² Àû¿ë
	int nMapIndex = pStage->GetStageSetting()->GetMapIndex();
	if ((nMapIndex >=0) && (nMapIndex < MMATCH_MAP_COUNT))
	{
		float fMapExpRatio = MGetMapDescMgr()->GetExpRatio(nMapIndex);
		fGameExpRatio = fGameExpRatio * fMapExpRatio;
	}

	int nAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nVictimLevel = pVictim->GetCharInfo()->m_nLevel;

	// °æÇèÄ¡ °è»ê
	int nAttackerExp = (int)(MMatchFormula::GetGettingExp(nAttackerLevel, nVictimLevel) * fGameExpRatio);
	int nVictimExp = (int)(MMatchFormula::CalcPanaltyEXP(nAttackerLevel, nVictimLevel) * fGameExpRatio);


	// Å¬·£ÀüÀÏ °æ¿ì´Â È¹µæ °æÇèÄ¡°¡ 1.5¹è, ¼Õ½Ç°æÇèÄ¡ ¾øÀ½
	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pStage->GetStageType() == MST_LADDER))
	{
		nAttackerExp = (int)((float)nAttackerExp * 1.5f);
		nVictimExp = 0;
	}

	// °í¼öÃ¤³Î, ÃÊ°í¼öÃ¤³ÎÀÏ °æ¿ì¿¡´Â °æÄ¡´Ù¿î ¾øÀ½(ÀÚ»ìÁ¦¿Ü)
	MMatchChannel* pOwnerChannel = FindChannel(pStage->GetOwnerChannel());
	if ((pOwnerChannel) && (!bSuicide))
	{
		if ((pOwnerChannel->GetRuleType() == MCHANNEL_RULE_MASTERY) || 
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_ELITE) ||
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_CHAMPION))
		{
			nVictimExp=0;
		}
	}

	// Á×Àº»ç¶÷ÀÌ ¿î¿µÀÚ, °³¹ßÀÚÀÏ °æ¿ì °æÇèÄ¡ µÎ¹è
	if ((pVictim->GetAccountInfo()->m_nUGrade == MMUG_ADMIN) || 
		(pVictim->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER))
	{
		nAttackerExp = nAttackerExp * 2;
	}
	// Á×ÀÎ»ç¶÷ÀÌ ¿î¿µÀÚ, °³¹ßÀÚÀÏ °æ¿ì °æÄ¡´Ù¿î ¾øÀ½
	if ((!bSuicide) &&
		((pAttacker->GetAccountInfo()->m_nUGrade == MMUG_ADMIN) || 
		(pAttacker->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER)))
	{
		nVictimExp = 0;
	}

	// ÀÚ»ìÀÏ °æ¿ì °æÇèÄ¡ ¼Õ½ÇÀÌ µÎ¹è
	if (bSuicide) 
	{
		nVictimExp = (int)(MMatchFormula::GetSuicidePanaltyEXP(nVictimLevel) * fGameExpRatio);
		nAttackerExp = 0;
	}

	// ÆÀÅ³ÀÎ°æ¿ì °æÇèÄ¡ Á¦·Î
	if ((pStage->GetStageSetting()->IsTeamPlay()) && (pAttacker->GetTeam() == pVictim->GetTeam()))
	{
		nAttackerExp = 0;
	}


	// ÆÀÀüÀÏ °æ¿ì °æÇèÄ¡ ¹èºÐ
	if (pStage->IsApplyTeamBonus())
	{
		int nTeamBonus = 0;
		if (pStage->GetRule() != NULL)
		{
			int nNewAttackerExp = nAttackerExp;
			pStage->GetRule()->CalcTeamBonus(pAttacker, pVictim, nAttackerExp, &nNewAttackerExp, &nTeamBonus);
			nAttackerExp = nNewAttackerExp;
		}

		// ÆÀ °æÇèÄ¡ Àû¸³
		pStage->AddTeamBonus(nTeamBonus, MMatchTeam(pAttacker->GetTeam()));
	}

	// xp º¸³Ê½º Àû¿ë(³Ý¸¶ºí PC¹æ, °æÇèÄ¡ ¹ÝÁö)
	int nAttackerExpBonus = 0;
	if (nAttackerExp != 0)
	{
		//const float ta = float(atoi("15")) / 100.0f;
		//mlog( "test float : %f\n", ta * 100.0f );

		//MMatchItemBonusType nBonusType			= GetStageBonusType(pStage->GetStageSetting());
		//const double		dAttackerExp		= static_cast< double >( nAttackerExp );
		//const double		dXPBonusRatio		= static_cast< double >( MMatchFormula::CalcXPBonusRatio(pAttacker, nBonusType) );
		//const double		dAttackerExpBouns	= dAttackerExp * dXPBonusRatio;
		//const double		dSumAttackerExp		= dAttackerExp + dAttackerExpBouns;
		//
		//
		//nAttackerExpBonus = static_cast< int >( dAttackerExpBouns + 0.00001); 

		MMatchItemBonusType nBonusType = GetStageBonusType(pStage->GetStageSetting());
		const float fAttackerExpBonusRatio = MMatchFormula::CalcXPBonusRatio(pAttacker, nBonusType);
		 //ºÎµ¿¼Ò¼öÁ¡ ¿ÀÂ÷¶§¹®¿¡ °è»ê¿¡ ¿µÇâÀ» ÁÖÁö ¾Ê´Â ¹üÀ§¿¡¼­ º¸Á¤À» ÇØÁØ´Ù.
		// ¸¸¾à ÀÌºÎºÐ¿¡¼­ ´Ù½Ã ¹®Á¦°¡ ¹ß»ýÇÑ´Ù¸é º¸Á¤ÀÌ ¾Æ´Ñ º£ÀÌ½ººÎÅÍ ¼öÁ¤ ÀÛ¾÷À» ÇØ Áà¾ß ÇÑ´Ù.
		 nAttackerExpBonus = (int)(nAttackerExp * (fAttackerExpBonusRatio + 0.00001f));
	}

	*poutAttackerExp = nAttackerExp + nAttackerExpBonus;

	*poutVictimExp = nVictimExp;
}


const int MMatchServer::CalcBPonGameKill( MMatchStage* pStage, MMatchObject* pAttacker, const int nAttackerLevel, const int nVictimLevel )
{
	if( (0 == pStage) || (0 == pAttacker) ) 
		return -1;

	const int	nAddedBP		= static_cast< int >( MMatchFormula::GetGettingBounty(nAttackerLevel, nVictimLevel) );
	const float fBPBonusRatio	= MMatchFormula::CalcBPBounsRatio( pAttacker, GetStageBonusType(pStage->GetStageSetting()) );
	const int	nBPBonus		= static_cast< int >( nAddedBP * fBPBonusRatio );

	return nAddedBP + nBPBonus;
}




void MMatchServer::ProcessPlayerXPBP(MMatchStage* pStage, MMatchObject* pPlayer, int nAddedXP, int nAddedBP)
{
	if (pStage == NULL) return;
	if (!IsEnabledObject(pPlayer)) return;

	/*
		°æÇèÄ¡ °è»ê
		Ä³¸¯ÅÍ¿¡ °æÇèÄ¡ Àû¿ë
		·¹º§ °è»ê
		DBÄ³½Ì ¾÷µ¥ÀÌÆ®
		·¹º§¾÷,´Ù¿î ¸Þ¼¼Áö Àü¼Û
	*/

	MUID uidStage = pPlayer->GetStageUID();
	int nPlayerLevel = pPlayer->GetCharInfo()->m_nLevel;

	// Ä³¸¯ÅÍ XP ¾÷µ¥ÀÌÆ®
	pPlayer->GetCharInfo()->IncXP(nAddedXP);

	// ·¹º§ °è»ê
	int nNewPlayerLevel = -1;
	if ((pPlayer->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pPlayer->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nPlayerLevel)))
	{
		nNewPlayerLevel = MMatchFormula::GetLevelFromExp(pPlayer->GetCharInfo()->m_nXP);
		if (nNewPlayerLevel != pPlayer->GetCharInfo()->m_nLevel) pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
	}

	// ¹Ù¿îÆ¼ Ãß°¡ÇØÁØ´Ù
	pPlayer->GetCharInfo()->IncBP(nAddedBP);


	// DB Ä³½³ ¾÷µ¥ÀÌÆ®
	if (pPlayer->GetCharInfo()->GetDBCachingData()->IsRequestUpdate()) {
		UpdateCharDBCachingData(pPlayer);		///< XP, BP, KillCount, DeathCount Ä³½³ ¾÷µ¥ÀÌÆ®
	}

	// ¸¸¾à ·¹º§ÀÌ ¹Ù²î¸é µû·Î ·¹º§¾÷ÇÑ´Ù.
	if ((nNewPlayerLevel >= 0) && (nNewPlayerLevel != nPlayerLevel))
	{
		// ·¹º§ÀÌ ¹Ù²î¸é ¹Ù·Î Ä³½³ ¾÷µ¥ÀÌÆ®ÇÑ´Ù
		UpdateCharDBCachingData(pPlayer);

		pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
		if (!m_MatchDBMgr.UpdateCharLevel(pPlayer->GetCharInfo()->m_nCID, 
										  nNewPlayerLevel, 
										  pPlayer->GetCharInfo()->m_nBP,
										  pPlayer->GetCharInfo()->m_nTotalKillCount, 
										  pPlayer->GetCharInfo()->m_nTotalDeathCount,
										  pPlayer->GetCharInfo()->m_nTotalPlayTimeSec,
										  true))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pPlayer->GetCharInfo()->m_szName);
		}
	}

	// ·¹º§¾÷, ·¹º§ ´Ù¿î ¸Þ¼¼Áö º¸³»±â
	if (nNewPlayerLevel > 0)
	{
		if (nNewPlayerLevel > nPlayerLevel)
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0,0));
			pCmd->AddParameter(new MCommandParameterUID(pPlayer->GetUID()));
			pCmd->AddParameter(new MCommandParameterInt(nNewPlayerLevel));
			RouteToBattle(uidStage, pCmd);	
		}
		else if (nNewPlayerLevel < nPlayerLevel)
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_DOWN, MUID(0,0));
			pCmd->AddParameter(new MCommandParameterUID(pPlayer->GetUID()));
			pCmd->AddParameter(new MCommandParameterInt(nNewPlayerLevel));
			RouteToBattle(uidStage, pCmd);	
		}
	}
}

// ÆÀ º¸³Ê½º Àû¿ë
void MMatchServer::ApplyObjectTeamBonus(MMatchObject* pObject, int nAddedExp)
{
	if (!IsEnabledObject(pObject)) return;
	if (nAddedExp <= 0)
	{
		//_ASSERT(0);
		return;
	}
	
	bool bIsLevelUp = false;

	// º¸³Ê½º Àû¿ë
	if (nAddedExp != 0)
	{
		int nExpBonus = (int)(nAddedExp * MMatchFormula::CalcXPBonusRatio(pObject, MIBT_TEAM));
		nAddedExp += nExpBonus;
	}




	// Ä³¸¯ÅÍ XP ¾÷µ¥ÀÌÆ®
	pObject->GetCharInfo()->IncXP(nAddedExp);

	// ·¹º§ °è»ê
	int nNewLevel = -1;
	int nCurrLevel = pObject->GetCharInfo()->m_nLevel;

	if (nNewLevel > nCurrLevel) bIsLevelUp = true;

	if ((pObject->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pObject->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nCurrLevel)))
	{
		nNewLevel = MMatchFormula::GetLevelFromExp(pObject->GetCharInfo()->m_nXP);
		if (nNewLevel != nCurrLevel) pObject->GetCharInfo()->m_nLevel = nNewLevel;
	}

	// DB Ä³½³ ¾÷µ¥ÀÌÆ®
	if (pObject->GetCharInfo()->GetDBCachingData()->IsRequestUpdate())
	{
		UpdateCharDBCachingData(pObject);
	}

	// ¸¸¾à ·¹º§ÀÌ ¹Ù²î¸é ¹Ù·Î ·¹º§¾÷ÇÑ´Ù.
	if ((nNewLevel >= 0) && (nNewLevel != nCurrLevel))
	{
		// ·¹º§ÀÌ ¹Ù²î¸é ¹Ù·Î Ä³½³ ¾÷µ¥ÀÌÆ®ÇÑ´Ù
		UpdateCharDBCachingData(pObject);

		pObject->GetCharInfo()->m_nLevel = nNewLevel;
		nCurrLevel = nNewLevel;

		if (!m_MatchDBMgr.UpdateCharLevel(pObject->GetCharInfo()->m_nCID, 
			                              nNewLevel,
										  pObject->GetCharInfo()->m_nBP,
										  pObject->GetCharInfo()->m_nTotalKillCount,
										  pObject->GetCharInfo()->m_nTotalDeathCount,
										  pObject->GetCharInfo()->m_nTotalPlayTimeSec,
										  bIsLevelUp
										  ))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pObject->GetCharInfo()->m_szName);
		}
	}


	MUID uidStage = pObject->GetStageUID();

	unsigned long int nExpArg;
	unsigned long int nChrExp;
	int nPercent;

	nChrExp = pObject->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nCurrLevel);
	// »óÀ§ 2¹ÙÀÌÆ®´Â °æÇèÄ¡, ÇÏÀ§ 2¹ÙÀÌÆ®´Â °æÇèÄ¡ÀÇ ÆÛ¼¾Æ®ÀÌ´Ù.
	nExpArg = MakeExpTransData(nAddedExp, nPercent);


	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_TEAMBONUS, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pObject->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nExpArg));
	RouteToBattle(uidStage, pCmd);	


	// ·¹º§¾÷ ¸Þ¼¼Áö º¸³»±â
	if ((nNewLevel >= 0) && (nNewLevel > nCurrLevel))
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0,0));
		pCmd->AddParameter(new MCommandParameterUID(pObject->GetUID()));
		pCmd->AddParameter(new MCommandParameterInt(nNewLevel));
		RouteToBattle(uidStage, pCmd);	
	}
}

// ÇÃ·¹ÀÌ Áß Ä³¸¯ÅÍ Á¤º¸ ¾÷µ¥ÀÌÆ®
void MMatchServer::ProcessCharPlayInfo(MMatchObject* pPlayer)
{
	if (!IsEnabledObject(pPlayer)) return;

	/*
	¿øÇÒ¶§¸¶´Ù Ä³¸¯ÅÍ Á¤º¸¸¦ ¾÷µ¥ÀÌÆ® Àû¿ë
	°æÇèÄ¡ °è»ê
	Ä³¸¯ÅÍ¿¡ °æÇèÄ¡ Àû¿ë
	·¹º§ °è»ê
	·¹º§¾÷,´Ù¿î ¸Þ¼¼Áö Àü¼Û
	¹Ù¿îÆ¼ Ãß°¡ÇØÁØ´Ù
	Á¢¼Ó½Ã°£, °ÔÀÓ ÁøÇà½Ã°£, ÇÃ·¹ÀÌ ½Ã°£
	*/

	MUID uidStage = pPlayer->GetStageUID();
	int nPlayerLevel = pPlayer->GetCharInfo()->m_nLevel;

	// ·¹º§ °è»ê
	int nNewPlayerLevel = -1;
	if ((pPlayer->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pPlayer->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nPlayerLevel)))
	{
		nNewPlayerLevel = MMatchFormula::GetLevelFromExp(pPlayer->GetCharInfo()->m_nXP);
		if (nNewPlayerLevel != pPlayer->GetCharInfo()->m_nLevel) pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
	}
	// ¸¸¾à ·¹º§ÀÌ ¹Ù²î¸é µû·Î ·¹º§¾÷ÇÑ´Ù.
	if ((nNewPlayerLevel >= 0) && (nNewPlayerLevel != nPlayerLevel))
		pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;

	// Á¢¼Ó½Ã°£, °ÔÀÓ ÁøÇà½Ã°£, ÇÃ·¹ÀÌ ½Ã°£
	unsigned long int nNowTime = MMatchServer::GetInstance()->GetGlobalClockCount();
	unsigned long int nBattlePlayingTimeSec = 0;
	if(pPlayer->GetCharInfo()->m_nBattleStartTime != 0)
	{
		nBattlePlayingTimeSec = MGetTimeDistance(pPlayer->GetCharInfo()->m_nBattleStartTime, nNowTime) / 1000;	// ¹èÆ²À» ÁøÇàÇÑ ½Ã°£
		
		/*
		// ¾Æ¹« Ã³¸®µµ ÇÏÁö ¾Ê´Âµ¥, ¿Ö ·Î±×´Â ³²±â´Â°Ç°¡¿ä? ÀÏ´Ü ÁÖ¼® Ã³¸®ÇÕ´Ï´Ù. - carrot318
		if(nBattlePlayingTimeSec > 60*60)
		{// ÀÌ»óÀûÀ¸·Î °ªÀÌ ¼¼ÆÃµÅ¸é ·Î±×¸¦ ³²±ä´Ù.
			CTime theTime = CTime::GetCurrentTime();
			CString szTime = theTime.Format( "[%c] " );

			// °ÔÀÓ ¸ðµå
			char buf[64]={0,};
			MMatchStage* pStage = FindStage(uidStage);

			if( pStage != NULL )
			{
				switch((int)pStage->GetStageSetting()->GetGameType())
				{
				case MMATCH_GAMETYPE_DEATHMATCH_SOLO:	{sprintf(buf, "DEATHMATCH_SOLO");	} break;		///< °³ÀÎ µ¥¾²¸ÅÄ¡
				case MMATCH_GAMETYPE_DEATHMATCH_TEAM:	{sprintf(buf, "DEATHMATCH_TEAM");	} break;		///< ÆÀ µ¥¾²¸ÅÄ¡
				case MMATCH_GAMETYPE_GLADIATOR_SOLO:	{sprintf(buf, "GLADIATOR_SOLO");	} break;		///< °³ÀÎ ±Û·¡µð¿¡ÀÌÅÍ
				case MMATCH_GAMETYPE_GLADIATOR_TEAM:	{sprintf(buf, "GLADIATOR_TEAM");	} break;		///< ÆÀ ±Û·¡µð¿¡ÀÌÅÍ
				case MMATCH_GAMETYPE_ASSASSINATE:		{sprintf(buf, "ASSASSINATE");		} break;		///< º¸½ºÀü
				case MMATCH_GAMETYPE_TRAINING:			{sprintf(buf, "TRAINING");			} break;		///< ¿¬½À

				case MMATCH_GAMETYPE_SURVIVAL:			{sprintf(buf, "SURVIVAL");			} break;		///< ¼­¹ÙÀÌ¹ú
				case MMATCH_GAMETYPE_QUEST:				{sprintf(buf, "QUEST");				} break;		///< Äù½ºÆ®

				case MMATCH_GAMETYPE_BERSERKER:			{sprintf(buf, "BERSERKER");			} break;		
				case MMATCH_GAMETYPE_DEATHMATCH_TEAM2:	{sprintf(buf, "DEATHMATCH_TEAM2");	} break;		
				case MMATCH_GAMETYPE_DUEL:				{sprintf(buf, "DUEL");				} break;	
				default:								{sprintf(buf, "don't know");		} break;
				}
				mlog("%s BattlePlayT Error GameMode:%s, CID:%d, Name:%s, ServerCurrT:%u, BattleStartT:%u, PlayT:%d, PlayerConnectT:%u \n"
					, szTime, buf, pPlayer->GetCharInfo()->m_nCID, pPlayer->GetCharInfo()->m_szName, nNowTime, pPlayer->GetCharInfo()->m_nBattleStartTime, nBattlePlayingTimeSec, pPlayer->GetCharInfo()->m_nConnTime);
			}
		}
		*/
		//pPlayer->GetCharInfo()->m_nBattleStartTime = 0;
	}
	unsigned long int nLoginTotalTimeSec = MGetTimeDistance(pPlayer->GetCharInfo()->m_nConnTime, nNowTime) / 1000;	// °ÔÀÓÀ» ÁøÇàÇÑ ½Ã°£

	// ÀÌ»óÀûÀ¸·Î °æÇèÄ¡°¡ È¹µæÇßÀ¸¸é ·Î±×¸¦ ³²°ÜÁØ´Ù.
	// ¾Æ¹« Ã³¸®µµ ÇÏÁö ¾Ê´Âµ¥, ¿Ö ·Î±×´Â ³²±â´Â°Ç°¡¿ä? ÀÏ´Ü ÁÖ¼® Ã³¸®ÇÕ´Ï´Ù. - carrot318
	/*
	long int nBattleEXPGained = pPlayer->GetCharInfo()->m_nXP - pPlayer->GetCharInfo()->m_nBattleStartXP;
	if(nBattleEXPGained < -150000 || 150000 < nBattleEXPGained)
	{
		CTime theTime = CTime::GetCurrentTime();
		CString szTime = theTime.Format( "[%c] " );
		mlog("%s BattleXPGained Error CID:%d, Name:%s, StartXP:%d, EXPGained:%d \n", szTime, pPlayer->GetCharInfo()->m_nCID, pPlayer->GetCharInfo()->m_szName, pPlayer->GetCharInfo()->m_nBattleStartXP, nBattleEXPGained);
	}
	*/

#ifdef LOCALE_NHNUSA
	if (!m_MatchDBMgr.UpdateCharPlayInfo(pPlayer->GetAccountInfo()->m_nAID
										, pPlayer->GetCharInfo()->m_nCID
										, pPlayer->GetCharInfo()->m_nXP
										, pPlayer->GetCharInfo()->m_nLevel
										, nBattlePlayingTimeSec										// ¹èÆ² ½Ã°£
										, nLoginTotalTimeSec										// ÃÑ°ÔÀÓÀ» ÁøÇàÇÑ ½Ã°£
										, pPlayer->GetCharInfo()->m_nTotalKillCount
										, pPlayer->GetCharInfo()->m_nTotalDeathCount
										, pPlayer->GetCharInfo()->m_nBP
										, false))
	{
		mlog("DB UpdateCharPlayInfo Error : %s\n", pPlayer->GetCharInfo()->m_szName);
	}
#endif

}

void MMatchServer::PostGameDeadOnGameKill(MUID& uidStage, MMatchObject* pAttacker, MMatchObject* pVictim,
									int nAddedAttackerExp, int nSubedVictimExp)
{
	unsigned long int nAttackerArg = 0;
	unsigned long int nVictimArg =0;

	int nRealAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nRealVictimLevel = pVictim->GetCharInfo()->m_nLevel;

	unsigned long int nChrExp;
	int nPercent;

	nChrExp = pAttacker->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nRealAttackerLevel);
	nAttackerArg = MakeExpTransData(nAddedAttackerExp, nPercent);

	nChrExp = pVictim->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nRealVictimLevel);
	nVictimArg = MakeExpTransData(nSubedVictimExp, nPercent);
	pAttacker->KillStreak++;
	pVictim->KillStreak = 0; //this is where shutdowns are
	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_DEAD, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pAttacker->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nAttackerArg));
	pCmd->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nVictimArg));
	pCmd->AddParameter(new MCommandParameterInt(pAttacker->KillStreak));
	RouteToBattle(uidStage, pCmd);	
}

void MMatchServer::StageList(const MUID& uidPlayer, int nStageStartIndex, bool bCacheUpdate)
{
	MMatchObject* pChar = GetObject(uidPlayer);
	if (pChar == NULL) return;
	MMatchChannel* pChannel = FindChannel(pChar->GetChannelUID());
	if (pChannel == NULL) return;

	// Å¬·£¼­¹öÀÎµ¥ Å¬·£Ã¤³ÎÀÏ °æ¿ì¿¡´Â ¹æ ¸®½ºÆ®´ë½Å ´ë±âÁß Å¬·£ ¸®½ºÆ®¸¦ º¸³½´Ù.
	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN))
	{
		StandbyClanList(uidPlayer, nStageStartIndex, bCacheUpdate);
		return;
	}


	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LIST), MUID(0,0), m_This);

	int nPrevStageCount = -1, nNextStageCount = -1;
	int nNextStageIndex = pChannel->GetMaxPlayers()-1;


	// 2008.09.16 
	int nRealStageStartIndex = nStageStartIndex;
	int nStageCount = 0;
	for(int i = 0; i < pChannel->GetMaxPlayers(); i++)
	{
		// ¹æÀÌ ºñ¿öÀÕÀ¸¸é Ã³¸®¾ÈÇÑ´Ù
		if (pChannel->IsEmptyStage(i)) continue;
		// ¹æÀÌ ÀÖÀ¸¸é Ã³¸®
		if(nStageCount < nStageStartIndex) // ¾Õ¿¡ ÅÇ¿¡ Ã³¸®µÈ ¹æµé < ÇöÀç ÅÇ¿¡¼­ ½ÃÀÛÇÒ ¹æ Index
			nStageCount++;
		else
		{
			nRealStageStartIndex = i;
			break;
		}
	}

	int nRealStageCount = 0;
	for (int i = /*nStageStartIndex*/nRealStageStartIndex; i < pChannel->GetMaxPlayers(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;

		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		nRealStageCount++;
		if (nRealStageCount >= TRANS_STAGELIST_NODE_COUNT) 
		{
			nNextStageIndex = i;
			break;
		}
	}

	if (!bCacheUpdate)
	{
		nPrevStageCount = pChannel->GetPrevStageCount(nStageStartIndex);
		nNextStageCount = pChannel->GetNextStageCount(nNextStageIndex);
	}

	pNew->AddParameter(new MCommandParameterChar((char)nPrevStageCount));
	pNew->AddParameter(new MCommandParameterChar((char)nNextStageCount));


	void* pStageArray = MMakeBlobArray(sizeof(MTD_StageListNode), nRealStageCount);
	int nArrayIndex=0;

	for (int i = /*nStageStartIndex*/nRealStageStartIndex; i < pChannel->GetMaxPlayers(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;
		
		if( pStage->GetState() < STAGE_STATE_STANDBY || pStage->GetState() > STAGE_STATE_COUNT )
		{
			/* Å©·¡½¬·Î ÀÎÇØ ¹æ¾îÄÚµå Ãß°¡. ³ªÁß¿¡ pChannel->m_pStages ¹è¿­ÀÌ ¾Æ´Ñ ¸ÊÀÌ³ª ´Ù¸¥°É·Î ¸®ÆåÅä¸µÇÊ¿ä*/
			LOG(LOG_FILE, "there is unavailable stages in %s channel. No:%d \n", pChannel->GetName(), i);
			continue;
		}


		if (nArrayIndex >= nRealStageCount) break;

		MTD_StageListNode* pNode = (MTD_StageListNode*)MGetBlobArrayElement(pStageArray, nArrayIndex++);
		pNode->uidStage = pStage->GetUID();
		strcpy(pNode->szStageName, pStage->GetName());
		pNode->nNo = (unsigned char)(pStage->GetIndex() + 1);	// »ç¿ëÀÚ¿¡°Ô º¸¿©ÁÖ´Â ÀÎµ¦½º´Â 1ºÎÅÍ ½ÃÀÛÇÑ´Ù
		pNode->nPlayers = (char)pStage->GetPlayers();
		pNode->nMaxPlayers = pStage->GetStageSetting()->GetMaxPlayers();
		pNode->nState = pStage->GetState();
		pNode->nGameType = pStage->GetStageSetting()->GetGameType();
		if(pStage->GetStageSetting()->GetFPSMode())
			pNode->nFPSMode |= 2; //get fps mode
		
		// ¸±·¹ÀÌ¸é ·Îºñ ¹æ¸®½ºÆ® ¹è³Ê¸¦ ¸±·¹ÀÌ¸ÊÀ¸·Î À¯ÁöÇØÁØ´Ù.
		if(pStage->IsRelayMap()) pNode->nMapIndex = MMATCH_MAP_RELAYMAP;
		else		 			 pNode->nMapIndex = pStage->GetStageSetting()->GetMapIndex();
		
		pNode->nSettingFlag = 0;
		// ³­ÀÔ
		if (pStage->GetStageSetting()->GetForcedEntry())
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_FORCEDENTRY_ENABLED;
		}
		// ºñ¹Ð¹æ
		if (pStage->IsPrivate())
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_PRIVATE;
		}
		// ·¹º§Á¦ÇÑ
		pNode->nLimitLevel = pStage->GetStageSetting()->GetLimitLevel();
		pNode->nMasterLevel = 0;

		if (pNode->nLimitLevel != 0)
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_LIMITLEVEL;

			;
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());
			if (pMaster)
			{
				if (pMaster->GetCharInfo())
				{
					pNode->nMasterLevel = pMaster->GetCharInfo()->m_nLevel;
				}
			}
		}
	}

	pNew->AddParameter(new MCommandParameterBlob(pStageArray, MGetBlobArraySize(pStageArray)));
	MEraseBlobArray(pStageArray);

	RouteToListener(pChar, pNew);	
}


void MMatchServer::OnStageRequestStageList(const MUID& uidPlayer, const MUID& uidChannel, const int nStageCursor)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;

	pObj->SetStageCursor(nStageCursor);
	StageList(pObj->GetUID(), nStageCursor, false);
}


void MMatchServer::OnRequestQuickJoin(const MUID& uidPlayer, void* pQuickJoinBlob)
{
	MTD_QuickJoinParam* pNode = (MTD_QuickJoinParam*)MGetBlobArrayElement(pQuickJoinBlob, 0);
	ResponseQuickJoin(uidPlayer, pNode);
}

void MMatchServer::ResponseQuickJoin(const MUID& uidPlayer, MTD_QuickJoinParam* pQuickJoinParam)
{
	if (pQuickJoinParam == NULL) return;

	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;
	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return;

	list<MUID>	recommended_stage_list;
	MUID uidRecommendedStage = MUID(0,0);
	int nQuickJoinResult = MOK;


	for (int i = 0; i < pChannel->GetMaxStages(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		int ret = ValidateStageJoin(pObj->GetUID(), pStage->GetUID());
		if (ret == MOK)
		{
			if (pStage->IsPrivate()) continue;

			int nMapIndex = pStage->GetStageSetting()->GetMapIndex();
			int nGameType = pStage->GetStageSetting()->GetGameType();

			if (!CheckBitSet(pQuickJoinParam->nMapEnum, nMapIndex)) continue;
			if (!CheckBitSet(pQuickJoinParam->nModeEnum, nGameType)) continue;

			//if (((1 << nMapIndex) & (pQuickJoinParam->nMapEnum)) == 0) continue;
			//if (((1 << nGameType) & (pQuickJoinParam->nModeEnum)) == 0) continue;

			recommended_stage_list.push_back(pStage->GetUID());
		}
	}

	if (!recommended_stage_list.empty())
	{
		int nSize=(int)recommended_stage_list.size();
		int nIndex = rand() % nSize;

		int nCnt = 0;
		for (list<MUID>::iterator itor = recommended_stage_list.begin(); itor != recommended_stage_list.end(); ++itor)
		{
			if (nIndex == nCnt)
			{
				uidRecommendedStage = (*itor);
				break;
			}
			nCnt++;
		}
	}
	else
	{
		nQuickJoinResult = MERR_CANNOT_NO_STAGE;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RESPONSE_QUICKJOIN, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterInt(nQuickJoinResult));
	pCmd->AddParameter(new MCommandParameterUID(uidRecommendedStage));
	RouteToListener(pObj, pCmd);	
}

static int __cdecl _int_sortfunc(const void* a, const void* b)
{
	return *((int*)a) - *((int*)b);
}


int MMatchServer::GetLadderTeamIDFromDB(const int nTeamTableIndex, const int* pnMemberCIDArray, const int nMemberCount)
{
	if ((nMemberCount <= 0) || (nTeamTableIndex != nMemberCount))
	{
		//_ASSERT(0);
		return 0;
	}

	// cid ¿À¸§Â÷¼øÀ¸·Î ¼ÒÆÃ - db»ó¿¡ ¼ÒÆÃµÇ¾î µé¾î°¡ÀÖ´Ù. 
	int* pnSortedCIDs = new int[nMemberCount];
	for (int i = 0; i < nMemberCount; i++)
	{
		pnSortedCIDs[i] = pnMemberCIDArray[i];
	}
	qsort(pnSortedCIDs, nMemberCount, sizeof(int), _int_sortfunc);

	int nTID = 0;
	if (pnSortedCIDs[0] != 0)
	{
		if (!m_MatchDBMgr.GetLadderTeamID(nTeamTableIndex, pnSortedCIDs, nMemberCount, &nTID))
		{
			nTID = 0;
		}
	}
	

	delete[] pnSortedCIDs;

	return nTID;
}

void MMatchServer::SaveLadderTeamPointToDB(const int nTeamTableIndex, const int nWinnerTeamID, const int nLoserTeamID, const bool bIsDrawGame)
{
	// Æ÷ÀÎÆ® °è»ê - ¾×¼Ç¸®±× Àü¿ë
	int nWinnerPoint = 0, nLoserPoint = 0, nDrawPoint = 0;

	nLoserPoint = -1;
	switch (nTeamTableIndex)
	{
	case 2:	// 2´ë2
		{
			nWinnerPoint = 4;
			nDrawPoint = 1;
		}
		break;
	case 3:
		{
			nWinnerPoint = 6;
			nDrawPoint = 1;
		}
		break;
	case 4:
		{
			nWinnerPoint = 10;
			nDrawPoint = 2;
		}
		break;
	}

	if (!m_MatchDBMgr.LadderTeamWinTheGame(nTeamTableIndex, nWinnerTeamID, nLoserTeamID, bIsDrawGame,
		                                   nWinnerPoint, nLoserPoint, nDrawPoint))
	{
		mlog("DB Query(SaveLadderTeamPointToDB) Failed\n");
	}
}


void MMatchServer::OnVoteCallVote(const MUID& uidPlayer, const char* pszDiscuss, const char* pszArg)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	// ¿î¿µÀÚ°¡ °­ÅðÅõÇ¥ÇÏ¸é °­Á¦·Î °­Åð
	if (IsAdminGrade(pObj)) {
		MMatchStage* pStage = FindStage(pObj->GetStageUID());
		if (pStage)
			pStage->KickBanPlayer(pszArg, false);
		return;
	}

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	char szMsg[256];
	// ¿î¿µÀÚ°¡ °°ÀÌ °ÔÀÓÁßÀÌ¸é ÅõÇ¥ ºÒ°¡´É
	for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); itor != pStage->GetObjEnd(); itor++) {
		MUID uidObj = (MUID)(*itor).first;
		MMatchObject* pPlayer = (MMatchObject*)GetObject(uidObj);
		if ((pPlayer) && (IsAdminGrade(pPlayer)))
		{
			sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
			Announce(uidPlayer, szMsg);

			return;
		}
	}


	if( pObj->WasCallVote() )
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
		Announce(uidPlayer, szMsg);

		return;
	}

	// ÅõÇ¥¸¦ Çß´Ù´Â°É Ç¥½ÃÇØ³õÀ½.
	pObj->SetVoteState( true );

	if (pStage->GetStageType() == MST_LADDER)
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE_LADERGAME);
		Announce(uidPlayer, szMsg);

		return;
	}

	if (pStage->GetRule() && pStage->GetRule()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
		Announce(uidPlayer, szMsg);

		return;
	}
#ifdef _VOTESETTING
	// ¹æ ¼³Á¤Áß ÅõÇ¥±â´ÉÀ» °Ë»çÇÔ.
	if( !pStage->GetStageSetting()->bVoteEnabled ) {
		VoteAbort( uidPlayer );
		return;
	}

	// ÀÌ¹ø °ÔÀÓ¿¡¼­ ÅõÇ¥¸¦ °ÇÀÇÇß´ÂÁö °Ë»ç.
	if( pStage->WasCallVote() ) {
		VoteAbort( uidPlayer );
		return;
	}
	else {
		pStage->SetVoteState( true );
	}
#endif

	if (pStage->GetVoteMgr()->GetDiscuss())
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_VOTE_ALREADY_START);
		Announce(uidPlayer, szMsg);

		return;
	}

	MVoteDiscuss* pDiscuss = MVoteDiscussBuilder::Build(uidPlayer, pStage->GetUID(), pszDiscuss, pszArg);
	if (pDiscuss == NULL) return;

	if (pStage->GetVoteMgr()->CallVote(pDiscuss)) {
		pDiscuss->Vote(uidPlayer, MVOTE_YES);	// ¹ßÀÇÀÚ ¹«Á¶°Ç Âù¼º

		MCommand* pCmd = CreateCommand(MC_MATCH_NOTIFY_CALLVOTE, MUID(0,0));
		pCmd->AddParameter(new MCmdParamStr(pszDiscuss));
		pCmd->AddParameter(new MCmdParamStr(pszArg));
		RouteToStage(pStage->GetUID(), pCmd);
		return;
	}
	else
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_VOTE_FAILED);
		Announce(uidPlayer, szMsg);

		return;
	}
}

void MMatchServer::OnVoteYes(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	MVoteDiscuss* pDiscuss = pStage->GetVoteMgr()->GetDiscuss();
    if (pDiscuss == NULL) return;

	pDiscuss->Vote(uidPlayer, MVOTE_YES);
}

void MMatchServer::OnVoteNo(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	MVoteDiscuss* pDiscuss = pStage->GetVoteMgr()->GetDiscuss();
    if (pDiscuss == NULL) return;

	pDiscuss->Vote(uidPlayer, MVOTE_NO);
}

void MMatchServer::VoteAbort( const MUID& uidPlayer )
{
#ifndef MERR_CANNOT_VOTE
#define MERR_CANNOT_VOTE 120000
#endif

	MMatchObject* pObj = GetObject( uidPlayer );
	if( 0 == pObj )
		return;

	MCommand* pCmd = CreateCommand( MC_MATCH_VOTE_RESPONSE, MUID(0, 0) );
	if( 0 == pCmd )
		return;

	pCmd->AddParameter( new MCommandParameterInt(MERR_CANNOT_VOTE) );
	RouteToListener( pObj, pCmd );
}



void MMatchServer::OnEventChangeMaster(const MUID& uidAdmin)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if( 0 == pObj )
		return;

	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// °ü¸®ÀÚ ±ÇÇÑÀ» °¡Áø »ç¶÷ÀÌ ¾Æ´Ï¸é ¿¬°áÀ» ²÷´Â´Ù.
	if (!IsAdminGrade(pObj))
	{
//		DisconnectObject(uidAdmin);		
		return;
	}

	if (pStage->GetMasterUID() == uidAdmin)
		return;

	pStage->SetMasterUID(uidAdmin);
	StageMaster(pStage->GetUID());
}

void MMatchServer::OnEventChangePassword(const MUID& uidAdmin, const char* pszPassword)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if( 0 == pObj ) 
		return;

	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// °ü¸®ÀÚ ±ÇÇÑÀ» °¡Áø »ç¶÷ÀÌ ¾Æ´Ï¸é ¿¬°áÀ» ²÷´Â´Ù.
	if (!IsAdminGrade(pObj))
	{
//		DisconnectObject(uidAdmin);		
		return;
	}

	pStage->SetPassword(pszPassword);
	pStage->SetPrivate(true);
}

void MMatchServer::OnEventRequestJjang(const MUID& uidAdmin, const char* pszTargetName)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if( 0 == pObj )
		return;

	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// °ü¸®ÀÚ ±ÇÇÑÀ» °¡Áø »ç¶÷ÀÌ ¾Æ´Ï¸é ¹«½Ã
	if (!IsAdminGrade(pObj))
	{
		return;
	}

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;
	if (IsAdminGrade(pTargetObj)) return;		// ¾îµå¹Î ´ë»óÀ¸·Î Â¯ºÒ°¡
	if (MMUG_STAR == pTargetObj->GetAccountInfo()->m_nUGrade) return;	// ÀÌ¹Ì Â¯

	pTargetObj->GetAccountInfo()->m_nUGrade = MMUG_STAR;

	if (m_MatchDBMgr.EventJjangUpdate(pTargetObj->GetAccountInfo()->m_nAID, true)) {
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pTargetObj);
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REPLACE, this);
		RouteToStage(pStage->GetUID(), pCmdCacheUpdate);

		MCommand* pCmdUIUpdate = CreateCommand(MC_EVENT_UPDATE_JJANG, MUID(0,0));
		pCmdUIUpdate->AddParameter(new MCommandParameterUID(pTargetObj->GetUID()));
		pCmdUIUpdate->AddParameter(new MCommandParameterBool(true));
		RouteToStage(pStage->GetUID(), pCmdUIUpdate);
	}
}

void MMatchServer::OnEventRemoveJjang(const MUID& uidAdmin, const char* pszTargetName)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if( 0 == pObj )
		return;

	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// °ü¸®ÀÚ ±ÇÇÑÀ» °¡Áø »ç¶÷ÀÌ ¾Æ´Ï¸é ¿¬°áÀ» ²÷´Â´Ù.
	if (!IsAdminGrade(pObj))
	{
		return;
	}
	
	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;			// ¾îµå¹Î ´ë»óÀ¸·Î Â¯ºÒ°¡

	pTargetObj->GetAccountInfo()->m_nUGrade = MMUG_FREE;

	if (m_MatchDBMgr.EventJjangUpdate(pTargetObj->GetAccountInfo()->m_nAID, false)) {
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pTargetObj);
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REPLACE, this);
		RouteToStage(pStage->GetUID(), pCmdCacheUpdate);

		MCommand* pCmdUIUpdate = CreateCommand(MC_EVENT_UPDATE_JJANG, MUID(0,0));
		pCmdUIUpdate->AddParameter(new MCommandParameterUID(pTargetObj->GetUID()));
		pCmdUIUpdate->AddParameter(new MCommandParameterBool(false));
		RouteToStage(pStage->GetUID(), pCmdUIUpdate);
	}
}


void MMatchServer::OnStageGo(const MUID& uidPlayer, unsigned int nRoomNo)
{
	MMatchObject* pChar = GetObject(uidPlayer);
	if( 0 == pChar ) return;
	if (!IsEnabledObject(pChar)) return;
	if (pChar->GetPlace() != MMP_LOBBY) return;
	MMatchChannel* pChannel = FindChannel(pChar->GetChannelUID());
	if (pChannel == NULL) return;

	MMatchStage* pStage = pChannel->GetStage(nRoomNo-1);
	if (pStage) {
		MCommand* pNew = CreateCommand(MC_MATCH_REQUEST_STAGE_JOIN, GetUID());
		pNew->SetSenderUID(uidPlayer);	// ÇÃ·¹ÀÌ¾î°¡ º¸³½ ¸Þ½ÃÁöÀÎ °ÍÃ³·³ À§Àå
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
		Post(pNew);
	}
}



void MMatchServer::OnDuelQueueInfo(const MUID& uidStage, const MTD_DuelQueueInfo& QueueInfo)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_DUEL_QUEUEINFO, MUID(0,0));
	pCmd->AddParameter(new MCmdParamBlob(&QueueInfo, sizeof(MTD_DuelQueueInfo)));
	RouteToBattle(uidStage, pCmd);
}


void MMatchServer::OnQuestSendPing(const MUID& uidStage, unsigned long int t)
{
	MCommand* pCmd = CreateCommand(MC_QUEST_PING, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUInt(t));
	RouteToBattle(uidStage, pCmd);
}

void MMatchServer::SaveGameLog(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	int nMapID		= pStage->GetStageSetting()->GetMapIndex();
	int nGameType	= (int)pStage->GetStageSetting()->GetGameType();
	
	

	// test ¸ÊµîÀº ·Î±× ³²±âÁö ¾Ê´Â´Ù.
	if ( (MGetMapDescMgr()->MIsCorrectMap(nMapID)) && (MGetGameTypeMgr()->IsCorrectGameType(nGameType)) )
	{
		if (pStage->GetStageType() != MST_LADDER)
		{
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());

			MAsyncDBJob_InsertGameLog* pJob = new MAsyncDBJob_InsertGameLog(uidStage);
			pJob->Input(pMaster == NULL ? 0 : pMaster->GetCharInfo()->m_nCID,
				MGetMapDescMgr()->GetMapName(nMapID), 
				MGetGameTypeMgr()->GetInfo(MMATCH_GAMETYPE(nGameType))->szGameTypeStr);
			PostAsyncJob(pJob);
		}
	}

}

void MMatchServer::SaveGamePlayerLog(MMatchObject* pObj, unsigned int nStageID)
{	
	if( pObj == NULL ) return;
	if( nStageID == 0 ) return;
	if( pObj->GetCharInfo() == NULL ) return;

	MAsyncDBJob_InsertGamePlayerLog* pJob = new MAsyncDBJob_InsertGamePlayerLog;
	pJob->Input(nStageID, pObj->GetCharInfo()->m_nCID,
		(GetGlobalClockCount() - pObj->GetCharInfo()->m_nBattleStartTime) / 1000,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nKillCount,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nDeathCount,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nXP,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nBP);
	PostAsyncJob(pJob);
}