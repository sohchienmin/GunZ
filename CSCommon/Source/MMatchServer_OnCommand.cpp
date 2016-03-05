#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "Msg.h"
#include "MMatchStage.h"
#include "MCommandCommunicator.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "RTypes.h"
#include "MMatchStatus.h"
#include "MMatchSchedule.h"
#include "MTypes.h"
#include "MMatchConfig.h"




#define _STATUS_CMD_START	unsigned long int nStatusStartTime = timeGetTime();
#define _STATUS_CMD_END		MGetServerStatusSingleton()->AddCmd(pCommand->GetID(), pCommand->GetSenderUID(), 0, timeGetTime()-nStatusStartTime);

bool MMatchServer::OnCommand(MCommand* pCommand)
{
	// 서버 또는 Agent 가 보내는것을 제외하고 플러딩 검사
	if(GetAgent(pCommand->GetSenderUID())==NULL && pCommand->GetSenderUID()!=m_This)
	{
		bool bFloodingSuspect = false;
		m_objectCommandHistory.PushCommand( pCommand->GetSenderUID(), pCommand->GetID(), GetGlobalClockCount(), &bFloodingSuspect);
		if(bFloodingSuspect)
		{
			if (MGetServerConfig()->IsUseBlockFlooding())
			{
				MMatchObject* pObj = GetObject( pCommand->GetSenderUID() );
				if( pObj && pObj->GetDisconnStatusInfo().GetStatus() == MMDS_CONNECTED)
				{
					LOG(LOG_FILE,"Command Flood detected");
					LogObjectCommandHistory( pCommand->GetSenderUID() );
					pObj->DisconnectHacker( MMHT_COMMAND_FLOODING );
				}
				else
				{
					LOG(LOG_FILE,"Command(%d) Flood detected, but i can't find matchobject", pCommand->GetID());
				}

				// 커맨드는 이미 처리한것으로 처리
				return true;
			}
		}
	}

	MGetServerStatusSingleton()->AddCmd(pCommand->GetID(), pCommand->GetSenderUID());

	if (MServer::OnCommand(pCommand) == true)
		return true;

#ifdef _GAMEGUARD
	if( MGetServerConfig()->IsUseGamegaurd() )
	{
		// 최고 인증후 커맨드 요청 허용 방법은 클라이언트의
		//  - by SungE 2007-09-19
		if( IsSkeepByGameguardFirstAuth(pCommand) )
		{
			return true;
		}
	}
#endif

 _STATUS_CMD_START;

	switch(pCommand->GetID()){
		case MC_MATCH_LOGIN:
			{
				char szUserID[MAX_USERID_STRING_LEN];
				char szPassword[MAX_USER_PASSWORD_STRING_LEN];
				int nCommandVersion = 0;
				unsigned long nChecksumPack = 0;
				if (pCommand->GetParameter(szUserID, 0, MPT_STR, MAX_USERID_STRING_LEN) == false) break;
				if (pCommand->GetParameter(szPassword, 1, MPT_STR, MAX_USER_PASSWORD_STRING_LEN) == false) break;
				if (pCommand->GetParameter(&nCommandVersion, 2, MPT_INT) == false) break;
				if (pCommand->GetParameter(&nChecksumPack, 3, MPT_UINT) == false) break;
				MCommandParameter* pLoginParam = pCommand->GetParameter(4);
				if (pLoginParam->GetType() != MPT_BLOB) break;
				void *pLoginBlob = pLoginParam->GetPointer();
				if (NULL == pLoginBlob)
				{
					break;
				}

				if (MGetBlobArraySize(pLoginBlob) != (8 + MAX_MD5LENGH)) {
					break;
				}

				char *szEncryptMD5Value = (char *)MGetBlobArrayElement(pLoginBlob, 0);

				OnMatchLogin(pCommand->GetSenderUID(), szUserID, szPassword, nCommandVersion, nChecksumPack, szEncryptMD5Value);
			}
			break;
		case MC_MATCH_PLAYERWARS_VOTE:
			{
				MMatchObject* pObj = (MMatchObject*)GetObject(pCommand->GetSenderUID());
				string x = "RECEIVED MC_MATCH_PLAYERS_VOTE";
				Log(LOG_DEBUG, x.c_str());
				Log(LOG_PROG, x.c_str());
				Log(LOG_FILE, x.c_str());
				if (pObj)
				{
					int Map;
					x = "INSIDE POBJ";
					Log(LOG_DEBUG, x.c_str());
					Log(LOG_PROG, x.c_str());
					Log(LOG_FILE, x.c_str());
					if (pCommand->GetParameter(&Map, 0, MPT_INT)==false) break;
					if(Map < 0 || Map > 2) return true;
					MMatchChannel* chan = MGetMatchServer()->FindChannel(pObj->GetChannelUID());
					if(chan) {
						x = "CHANNEL EXIST AND UPDATING PLAYER VOTE";
						Log(LOG_DEBUG, x.c_str());
						Log(LOG_PROG, x.c_str());
						Log(LOG_FILE, x.c_str());
						GetLadderMgr()->UpdatePlayerVote(Map, pObj);
					}
				}
				
			}
			break;
		case MC_MATCH_STAGE_REQUEST_REJOIN:
			{
				MMatchObject* pObj = (MMatchObject*)GetObject(pCommand->GetSenderUID());
				if (pObj)
				{
					ClanReDef::iterator it = ClanRejoiner.find(pObj->GetCharInfo()->m_nCID);
					if(it != ClanRejoiner.end())
					{
						if(!StageJoin(pObj->GetUID(), it->second->StageUID, true, it->second->Team))
						{
							NotifyMessage(pObj->GetUID(), MATCHNOTIFY_STAGE_NOT_EXIST);
						}
					}
				} 
			}
			break;
		case MC_REQUEST_NEWS:
			{
				GetDBMgr()->GetLatestNews(pCommand->GetSenderUID());
			}
			break;

		case MC_MATCH_LOGIN_NETMARBLE_JP:
			{
				char szLoginID[ MAX_USERID_STRING_LEN ];
				char szLoginPW[ MAX_USER_PASSWORD_STRING_LEN ];
				int nCommandVersion = 0;
				unsigned long nChecksumPack = 0;
				if (pCommand->GetParameter(szLoginID,		0, MPT_STR, MAX_USERID_STRING_LEN )==false) break;
				if (pCommand->GetParameter(szLoginPW,		1, MPT_STR, MAX_USER_PASSWORD_STRING_LEN )==false) break;
				if (pCommand->GetParameter(&nCommandVersion,2, MPT_INT)==false) break;
				if (pCommand->GetParameter(&nChecksumPack,	3, MPT_UINT)==false) break;

				OnMatchLoginFromNetmarbleJP(pCommand->GetSenderUID(), 
										szLoginID, szLoginPW, nCommandVersion, nChecksumPack);
			}
			break;
		case MC_MATCH_LOGIN_FROM_DBAGENT:
			{
				MUID CommUID;
				char szLoginID[256];
				char szName[256];
				int nSex;
				bool bFreeLoginIP;
				unsigned long nChecksumPack;

				if (pCommand->GetParameter(&CommUID,		0, MPT_UID)==false) break;
				if (pCommand->GetParameter(szLoginID,		1, MPT_STR, sizeof(szLoginID) )==false) break;
				if (pCommand->GetParameter(szName,			2, MPT_STR, sizeof(szName) )==false) break;
				if (pCommand->GetParameter(&nSex,			3, MPT_INT)==false) break;
				if (pCommand->GetParameter(&bFreeLoginIP,	4, MPT_BOOL)==false) break;
				if (pCommand->GetParameter(&nChecksumPack,	5, MPT_UINT)==false) break;

				OnMatchLoginFromDBAgent(CommUID, szLoginID, szName, nSex, bFreeLoginIP, nChecksumPack);
			}
			break;
		case MC_MATCH_LOGIN_FROM_DBAGENT_FAILED:
		{
			/*MUID CommUID;
			int nResult;

			if (pCommand->GetParameter(&CommUID,    0, MPT_UID)==false) break;
			if (pCommand->GetParameter(&nResult,    1, MPT_INT)==false) break;

			OnMatchLoginFailedFromDBAgent(CommUID, nResult);*/
		}
			break;

		case MC_MATCH_BRIDGEPEER:
			{
				MUID uidChar;
				DWORD dwIP, nPort;

				pCommand->GetParameter(&uidChar,	0, MPT_UID);
				pCommand->GetParameter(&dwIP,		1, MPT_UINT);
				pCommand->GetParameter(&nPort,		2, MPT_UINT);
				OnBridgePeer(uidChar, dwIP, nPort);
			}
			break;
		case MC_DEBUG_TEST:
			{
				DebugTest();
			}
			break;
		case MC_MATCH_REQUEST_RECOMMANDED_CHANNEL:
			{
				OnRequestRecommendedChannel(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_CHANNEL_REQUEST_JOIN:
			{
				MUID uidPlayer, uidChannel;
				char szClanPassword[24];
				uidPlayer = pCommand->GetSenderUID();
				pCommand->GetParameter(&uidChannel, 1, MPT_UID);
				pCommand->GetParameter(szClanPassword, 2, MPT_STR, sizeof(szClanPassword));
				OnRequestChannelJoin(uidPlayer, uidChannel, szClanPassword);
			}
			break;
		case MC_MATCH_CHANNEL_REQUEST_JOIN_FROM_NAME:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nChannelType;
				char szChannelName[CHANNELNAME_LEN];

				//pCommand->GetParameter(&uidPlayer,		0, MPT_UID);
				pCommand->GetParameter(&nChannelType,	1, MPT_INT);
				pCommand->GetParameter(szChannelName,	2, MPT_STR, CHANNELNAME_LEN );

				OnRequestChannelJoin(uidPlayer, MCHANNEL_TYPE(nChannelType), szChannelName);
			}
			break;
		case MC_MATCH_CHANNEL_LIST_START:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nChannelType;

				//pCommand->GetParameter(&uidPlayer,		0, MPT_UID);
				pCommand->GetParameter(&nChannelType,	1, MPT_INT);

				OnStartChannelList(uidPlayer, nChannelType);
			}
			break;
		case MC_MATCH_CHANNEL_LIST_STOP:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				//pCommand->GetParameter(&uidPlayer,		0, MPT_UID);

				OnStopChannelList(uidPlayer);
			}
			break;
		case MC_MATCH_CHANNEL_REQUEST_CHAT:
			{
				MUID uidSender, uidPlayer, uidChannel;
				static char szChat[CHAT_STRING_LEN];
				int nR, nG, nB;
				uidSender = pCommand->GetSenderUID();
				pCommand->GetParameter(&uidPlayer, 0, MPT_UID);

				if (uidSender != uidPlayer)
				{
					break;
				}


				pCommand->GetParameter(&uidChannel, 1, MPT_UID);
				pCommand->GetParameter(szChat, 2, MPT_STR, CHAT_STRING_LEN);
#ifdef _GZFGunz
				pCommand->GetParameter(&nR, 3, MPT_INT);
				pCommand->GetParameter(&nG, 4, MPT_INT);
				pCommand->GetParameter(&nB, 5, MPT_INT);
#endif
				OnChannelChat(uidSender, uidChannel, szChat, nR, nG, nB);
			}
			break;
		case MC_MATCH_STAGE_TEAM:
			{
				MUID uidPlayer, uidStage;
				MMatchTeam nTeam;
				uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nTeam, 2, MPT_UINT);

				OnStageTeam(uidPlayer, uidStage, nTeam);				
			}
			break;
		case MC_MATCH_STAGE_PLAYER_STATE:
			{
				MUID uidPlayer, uidStage;
				int nStageState;
				uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nStageState, 2, MPT_INT);


				OnStagePlayerState(uidPlayer, uidStage, MMatchObjectStageState(nStageState));				

			}
			break;
		case MC_MATCH_STAGE_CREATE:
			{
				MUID uidChar = pCommand->GetSenderUID();
				// char szStageName[128], szStagePassword[128];
				char szStageName[ STAGENAME_LENGTH ] = ""; 
				char szStagePassword[ STAGEPASSWD_LENGTH ] = "";
				bool bPrivate;

				// pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(szStageName, 1, MPT_STR, STAGENAME_LENGTH );
				pCommand->GetParameter(&bPrivate, 2, MPT_BOOL);
				pCommand->GetParameter(szStagePassword, 3, MPT_STR, STAGEPASSWD_LENGTH );

				OnStageCreate(uidChar, szStageName, bPrivate, szStagePassword);
			}
			break;

		case MC_MATCH_INFORMATION:
			{
				char szTargetName[MAX_CHARNAME_LENGTH];
				pCommand->GetParameter(szTargetName, 0, MPT_STR, MAX_CHARNAME_LENGTH);

				MMatchObject* pPlayerObj = GetObject(pCommand->GetSenderUID());
				if (pPlayerObj == NULL) break;

				MMatchObject* pTargetObj = GetPlayerByName(szTargetName);
				if (pTargetObj == NULL) break;



				MCommand* pNew = CreateCommand(MC_MATCH_INFORMATION_RECEIVE, MUID(0, 0));
				void* pInformationFrame = MMakeBlobArray(sizeof(MTD_Information), 1);
				MTD_Information* pInformation = (MTD_Information*)MGetBlobArrayElement(pInformationFrame, 0);

				strcpy(pInformation->szName, pTargetObj->GetCharInfo()->m_szName);
				pInformation->nLevel = pTargetObj->GetCharInfo()->m_nLevel;
				pInformation->nBounty = pTargetObj->GetCharInfo()->m_nBP;
				pInformation->nCash = pTargetObj->GetAccountInfo()->m_nDonationCoins;
				pInformation->nBP = pTargetObj->GetCharInfo()->m_nBP;
				pInformation->nEXP = pTargetObj->GetCharInfo()->m_nXP;
				pInformation->nFace = pTargetObj->GetCharInfo()->m_nFace;
				pInformation->nHair = pTargetObj->GetCharInfo()->m_nHair;
				pInformation->nSex = pTargetObj->GetCharInfo()->m_nSex;

				pInformation->uidTarget = pTargetObj->GetUID();

				strcpy(pInformation->szClan, pTargetObj->GetCharInfo()->m_ClanInfo.m_szClanName);
				pInformation->nClanEmblem = pTargetObj->GetCharInfo()->m_ClanInfo.m_nClanID;

				pInformation->nR = pTargetObj->GetCharInfo()->m_nRedColor;
				pInformation->nG = pTargetObj->GetCharInfo()->m_nGreenColor;
				pInformation->nB = pTargetObj->GetCharInfo()->m_nBlueColor;

				pInformation->nKill = pTargetObj->GetCharInfo()->m_nTotalKillCount;
				pInformation->nDeath = pTargetObj->GetCharInfo()->m_nTotalDeathCount;

				pNew->AddParameter(new MCommandParameterBlob(pInformationFrame, MGetBlobArraySize(pInformationFrame)));
				MEraseBlobArray(pInformationFrame);
				RouteToListener(pPlayerObj, pNew);


				MCommand* pTest = CreateCommand(MC_MATCH_INFORMATION_HPAP, MUID(0, 0));
				pTest->AddParameter(new MCommandParameterString(pPlayerObj->GetCharInfo()->m_szName));
				RouteToListener(pTargetObj, pTest);

				pTest = CreateCommand(MC_MATCH_INFORMATION_CLOTHES, MUID(0, 0));
				pTest->AddParameter(new MCommandParameterString(pPlayerObj->GetCharInfo()->m_szName));
				RouteToListener(pTargetObj, pTest);

			}
			break;

		case MC_MATCH_INFORMATION_HPAP_GO:
			{
				static char szTarget[CHAT_STRING_LEN];
				int nHP, nAP;
				pCommand->GetParameter(szTarget, 0, MPT_STR, CHAT_STRING_LEN);
				pCommand->GetParameter(&nHP, 1, MPT_INT);
				pCommand->GetParameter(&nAP, 2, MPT_INT);

				MMatchObject* pTargetObj = GetPlayerByName(szTarget);
				if (pTargetObj == NULL) break;

				MCommand* pNew = CreateCommand(MC_MATCH_INFORMATION_HPAP_GO, MUID(0, 0));
				pNew->AddParameter(new MCommandParameterString(szTarget));
				pNew->AddParameter(new MCommandParameterInt(nHP));
				pNew->AddParameter(new MCommandParameterInt(nAP));
				RouteToListener(pTargetObj, pNew);

			}
			break;

		case MC_MATCH_INFORMATION_CLOTHES_GO:
			{
				static char szTarget[CHAT_STRING_LEN];
				pCommand->GetParameter(szTarget, 0, MPT_STR, CHAT_STRING_LEN);


				MMatchObject* pTargetObj = GetPlayerByName(szTarget);
				if (pTargetObj == NULL) break;


				MCommandParameter* pParam = pCommand->GetParameter(1);
				if (pParam->GetType() != MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();
				int nCount = MGetBlobArrayCount(pBlob);
				MTD_InformationClothes* pNode = (MTD_InformationClothes*)MGetBlobArrayElement(pBlob, 0);


				MCommand* pNew = CreateCommand(MC_MATCH_INFORMATION_CLOTHES_GO, MUID(0, 0));
				void* pInformationFrame = MMakeBlobArray(sizeof(MTD_InformationClothes), 1);
				MTD_InformationClothes* pInformation = (MTD_InformationClothes*)MGetBlobArrayElement(pInformationFrame, 0);

				char szTest[123];
				for (int i = 0; i < MMCIP_END; i++)
				{
					pInformation->nEquipedItemID[i] = pNode->nEquipedItemID[i];
				}
				pInformation->nFace = pNode->nFace;
				pInformation->nHair = pNode->nHair;
				pInformation->nSex = pNode->nSex;

				pNew->AddParameter(new MCommandParameterString(szTarget));
				pNew->AddParameter(new MCommandParameterBlob(pInformationFrame, MGetBlobArraySize(pInformationFrame)));
				MEraseBlobArray(pInformationFrame);
				RouteToListener(pTargetObj, pNew);

			}
			break;
		
		case MC_MATCH_REQUEST_PRIVATE_STAGE_JOIN:
			{
				MUID uidPlayer, uidStage;
				// char szPassword[256];
				char szPassword[ STAGEPASSWD_LENGTH ] = "";

				uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(szPassword, 2, MPT_STR, STAGEPASSWD_LENGTH );

				OnPrivateStageJoin(uidPlayer, uidStage, szPassword);
			}
			break;
		
		case MC_MATCH_STAGE_FOLLOW:
			{
				// char szTargetName[MATCHOBJECT_NAME_LENGTH+1];
				// pCommand->GetParameter(szTargetName, 0, MPT_STR, MATCHOBJECT_NAME_LENGTH+1 );
				char szTargetName[ MAX_CHARNAME_LENGTH ];
				pCommand->GetParameter(szTargetName, 0, MPT_STR, MAX_CHARNAME_LENGTH );

				OnStageFollow(pCommand->GetSenderUID(), szTargetName);
			}
			break;
		case MC_MATCH_STAGE_LEAVE:
			{
				// MUID uidPlayer, uidStage;
				// uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				// pCommand->GetParameter(&uidStage, 1, MPT_UID);

				OnStageLeave(pCommand->GetSenderUID());//, uidStage);
			}
			break;
		case MC_MATCH_STAGE_REQUEST_PLAYERLIST:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				OnStageRequestPlayerList(pCommand->GetSenderUID(), uidStage);
			}
			break;
		case MC_MATCH_STAGE_REQUEST_ENTERBATTLE:
			{
				MUID uidPlayer, uidStage;
				uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				OnStageEnterBattle(uidPlayer, uidStage);
			}
			break;
		case MC_MATCH_STAGE_LEAVEBATTLE_TO_SERVER:
			{
				// MUID uidPlayer, uidStage;
				// uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				// pCommand->GetParameter(&uidStage, 1, MPT_UID);
				bool bGameFinishLeaveBattle;
				pCommand->GetParameter(&bGameFinishLeaveBattle, 1, MPT_BOOL);
				OnStageLeaveBattle(pCommand->GetSenderUID(), bGameFinishLeaveBattle);//, uidStage);
			}
			break;
		case MC_MATCH_STAGE_START:
			{
				MUID uidPlayer, uidStage;
				int nCountdown;
				uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(&nCountdown, 2, MPT_INT);

				OnStageStart(uidPlayer, uidStage, nCountdown);
			}
			break;
		case MC_MATCH_GAME_ROUNDSTATE:
			{
				MUID uidStage;
				int nState, nRound;

				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(&nState, 1, MPT_INT);
				pCommand->GetParameter(&nRound, 2, MPT_INT);

				OnGameRoundState(uidStage, nState, nRound);
			}
			break;

		case MC_MATCH_GAME_REQUEST_SPAWN:
			{
				//MUID uidChar;
				MVector pos, dir;

				//pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&pos, 1, MPT_POS);
				pCommand->GetParameter(&dir, 2, MPT_DIR);

				OnRequestSpawn(pCommand->GetSenderUID(), pos, dir);
			}
			break;
		case MC_MATCH_GAME_REQUEST_TIMESYNC:
			{
				unsigned int nLocalTimeStamp;
				pCommand->GetParameter(&nLocalTimeStamp, 0, MPT_UINT);

				OnGameRequestTimeSync(pCommand->GetSenderUID(), nLocalTimeStamp);
			}
			break;
		case MC_MATCH_GAME_REPORT_TIMESYNC:
			{
				unsigned int nLocalTimeStamp, nDataChecksum;
				pCommand->GetParameter(&nLocalTimeStamp, 0, MPT_UINT);
				pCommand->GetParameter(&nDataChecksum, 1, MPT_UINT);

				OnGameReportTimeSync(pCommand->GetSenderUID(), nLocalTimeStamp, nDataChecksum);
			}
			break;
		case MC_MATCH_STAGE_CHAT:
			{
				MUID uidPlayer, uidSender, uidStage;
				static char szChat[CHAT_STRING_LEN];
				int nR, nG, nB;
				uidSender = pCommand->GetSenderUID();
				pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				if (uidSender != uidPlayer)
				{
					break;
				}

				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				pCommand->GetParameter(szChat, 2, MPT_STR, CHAT_STRING_LEN);
#ifdef _GZFGunz
				pCommand->GetParameter(&nR, 3, MPT_INT);
				pCommand->GetParameter(&nG, 4, MPT_INT);
				pCommand->GetParameter(&nB, 5, MPT_INT);
#endif
				OnStageChat(uidPlayer, uidStage, szChat, nR, nG, nB);
			}
			break;
		case MC_MATCH_STAGE_REQUEST_QUICKJOIN:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				MCommandParameter* pQuickJoinParam = pCommand->GetParameter(1);
				if(pQuickJoinParam->GetType()!=MPT_BLOB) break;

				void* pQuickJoinBlob = pQuickJoinParam->GetPointer();
				if( NULL == pQuickJoinBlob )
					break;

				OnRequestQuickJoin(uidPlayer, pQuickJoinBlob);

			}
			break;
		case MC_MATCH_STAGE_GO:
			{
				unsigned int nRoomNo;

				pCommand->GetParameter(&nRoomNo, 0, MPT_UINT);
				OnStageGo(pCommand->GetSenderUID(), nRoomNo);
			}
			break;
		case MC_MATCH_STAGE_LIST_START:
			{
				OnStartStageList(pCommand->GetSenderUID());

			}
			break;
		case MC_MATCH_STAGE_LIST_STOP:
			{
				OnStopStageList(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_REQUEST_STAGE_LIST:
			{
				MUID uidPlayer, uidChannel;
				int nStageCursor;
				uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&uidChannel, 1, MPT_UID);
				pCommand->GetParameter(&nStageCursor, 2, MPT_INT);

				OnStageRequestStageList(uidPlayer, uidChannel, nStageCursor);
			}
			break;
		case MC_MATCH_CHANNEL_REQUEST_PLAYER_LIST:
		{
			MUID /*uidPlayer, */uidChannel;
			int nPage;

			//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
			pCommand->GetParameter(&uidChannel, 1, MPT_UID);
			pCommand->GetParameter(&nPage, 2, MPT_INT);

			OnChannelRequestPlayerList(pCommand->GetSenderUID(), uidChannel, nPage);
		}
			break;
		case MC_MATCH_STAGE_MAP:
			{
				MUID uidStage;
				// 클라이언트에서 맵 이름만 전송이 되기때문에 _MAX_DIR은 필요 없음. - by SungE 2007-04-02
				char szMapName[ MAPNAME_LENGTH ] = {0,};

				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(szMapName, 1, MPT_STR, MAPNAME_LENGTH );

				OnStageMap(uidStage, szMapName);
			}
			break;
		case MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE:
			{
				MUID uidStage;
				int nType = 0;
				int nRepeatCount = 0;

				pCommand->GetParameter(&uidStage, 0, MPT_UID );
				pCommand->GetParameter(&nType, 1, MPT_INT );
				pCommand->GetParameter(&nRepeatCount, 2, MPT_INT );

				OnStageRelayMapElementUpdate(uidStage, nType, nRepeatCount);
			}
			break;
		case MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE:
			{
				MUID uidStage;
				int nType = 0;
				int nRepeatCount = 0;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(&nType, 1, MPT_INT );
				pCommand->GetParameter(&nRepeatCount, 2, MPT_INT );
				MCommandParameter* pParam = pCommand->GetParameter(3);
				if (pParam->GetType() != MPT_BLOB)
					break;
				void* pRelayMapListBlob = pParam->GetPointer();
				if( NULL == pRelayMapListBlob )
					break;

				OnStageRelayMapListUpdate(uidStage, nType, nRepeatCount, pRelayMapListBlob);
			}
			break;
		case MC_MATCH_STAGESETTING:
			{
				MUID uidPlayer, uidStage;
				uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);

				MCommandParameter* pStageParam = pCommand->GetParameter(2);
				if(pStageParam->GetType()!=MPT_BLOB) break;
				void* pStageBlob = pStageParam->GetPointer();
				if( NULL == pStageBlob )
					break;

				int nStageCount = MGetBlobArrayCount(pStageBlob);

				OnStageSetting(uidPlayer, uidStage, pStageBlob, nStageCount);
			}
			break;
		case MC_MATCH_REQUEST_STAGESETTING:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				OnRequestStageSetting(pCommand->GetSenderUID(), uidStage);
			}
			break;
		case MC_MATCH_REQUEST_PEERLIST:
			{
				MUID uidStage;
				// pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				OnRequestPeerList(pCommand->GetSenderUID(), uidStage);
			}
			break;
		case MC_MATCH_REQUEST_GAME_INFO:
			{
				MUID uidStage;
				// pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				OnRequestGameInfo(pCommand->GetSenderUID(), uidStage);
			}
			break;
		case MC_MATCH_LOADING_COMPLETE:
			{
				
				int nPercent;
				// pCommand->GetParameter(&uidChar, 0, MPT_UID);
				pCommand->GetParameter(&nPercent, 1, MPT_INT);
				OnMatchLoadingComplete(pCommand->GetSenderUID(), nPercent);
			}
			break;
		case MC_MATCH_REQUEST_PEER_RELAY:
			{
				MUID  uidPeer;

				// if (pCommand->GetParameter(&uidChar, 0, MPT_UID) == false) break;
				if (pCommand->GetParameter(&uidPeer, 1, MPT_UID) == false) break;

				OnRequestRelayPeer(pCommand->GetSenderUID(), uidPeer);
			}
			break;
		case MC_AGENT_PEER_READY:
			{
				MUID uidChar;
				MUID uidPeer;

				if (pCommand->GetParameter(&uidChar, 0, MPT_UID) == false) break;
				if (pCommand->GetParameter(&uidPeer, 1, MPT_UID) == false) break;

				OnPeerReady(uidChar, uidPeer);
			}
			break;
		case MC_MATCH_REGISTERAGENT:
		{
			char szIP[128];
			int nTCPPort, nUDPPort;

			if (pCommand->GetParameter(&szIP, 0, MPT_STR, sizeof(szIP)) == false) break;
			if (pCommand->GetParameter(&nTCPPort, 1, MPT_INT) == false) break;
			if (pCommand->GetParameter(&nUDPPort, 2, MPT_INT) == false) break;

			// Not the best way to patch, but working for now
			if (strstr(szIP, "%")) {
				break;
			}

			OnRegisterAgent(pCommand->GetSenderUID(), szIP, nTCPPort, nUDPPort);
		}
			break;
		case MC_MATCH_UNREGISTERAGENT:
			{
				OnUnRegisterAgent(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_AGENT_REQUEST_LIVECHECK:
			{
				unsigned long nTimeStamp;
				unsigned long nStageCount;
				unsigned long nUserCount;

				if (pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT) == false) break;
				if (pCommand->GetParameter(&nStageCount, 1, MPT_UINT) == false) break;
				if (pCommand->GetParameter(&nUserCount, 2, MPT_UINT) == false) break;

				OnRequestLiveCheck(pCommand->GetSenderUID(), nTimeStamp, nStageCount, nUserCount);
			}
			break;
		case MC_AGENT_STAGE_READY:
			{
				MUID uidStage;
				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				OnAgentStageReady(pCommand->GetSenderUID(), uidStage);
			}
			break;

		case MC_MATCH_REQUEST_ACCOUNT_CHARINFO:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				char nCharNum;
				pCommand->GetParameter(&nCharNum, 0, MPT_CHAR);


				OnRequestAccountCharInfo(uidPlayer, nCharNum);
			}
			break;
		case MC_MATCH_REQUEST_SELECT_CHAR:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				unsigned long int nCharIndex;

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&nCharIndex, 1, MPT_UINT);

				OnRequestSelectChar(uidPlayer, nCharIndex);
			}
			break;
		case MC_MATCH_REQUEST_DELETE_CHAR:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				unsigned long int nCharIndex;
				// char szCharName[MAX_CHARNAME];
				char szCharName[ MAX_CHARNAME_LENGTH ];

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&nCharIndex, 1, MPT_UINT);
				pCommand->GetParameter(szCharName, 2, MPT_STR, MAX_CHARNAME_LENGTH );

				OnRequestDeleteChar(uidPlayer, nCharIndex, szCharName);
			}
			break;
		case MC_MATCH_REQUEST_CREATE_CHAR:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				unsigned long int nCharIndex;
				unsigned long int nSex, nHair, nFace, nCostume;
				int nRed, nGreen, nBlue;

				char szCharName[MAX_CHARNAME_LENGTH];

				pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&nCharIndex, 1, MPT_UINT);
				if (pCommand->GetParameter(szCharName, 2, MPT_STR, MAX_CHARNAME_LENGTH) == false) break;
				pCommand->GetParameter(&nSex, 3, MPT_UINT);
				pCommand->GetParameter(&nHair, 4, MPT_UINT);
				pCommand->GetParameter(&nFace, 5, MPT_UINT);
				pCommand->GetParameter(&nCostume, 6, MPT_UINT);
				pCommand->GetParameter(&nRed, 7, MPT_INT);
				pCommand->GetParameter(&nGreen, 8, MPT_INT);
				pCommand->GetParameter(&nBlue, 9, MPT_INT);

				OnRequestCreateChar(uidPlayer, nCharIndex, szCharName, nSex, nHair, nFace, nCostume, nRed, nGreen, nBlue);
			}
			break;
		case MC_MATCH_ROUND_FINISHINFO:
			{
				MUID uidStage, uidChar;

				pCommand->GetParameter(&uidStage, 0, MPT_UID);
				pCommand->GetParameter(&uidChar, 1, MPT_UID);
				OnUpdateFinishedRound(uidStage, uidChar, NULL, NULL);
			}
			break;
		case MC_MATCH_UPDATE_PASSWORD:
			{
				MMatchObject* pObj = GetObject(pCommand->GetSenderUID());

				int nClanGrade = pObj->GetCharInfo()->m_ClanInfo.m_nGrade;
				MMatchClanGrade grade = (MMatchClanGrade)nClanGrade;
				MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());

				if (pObj == NULL) break;
				if (!pObj->GetCharInfo()->m_ClanInfo.IsJoined()) break;
				if (pObj->GetCharInfo()->m_ClanInfo.m_nClanID == NULL) break;
				if (grade != MCG_MASTER) break;

				MMatchClan* pClan = FindClan(pObj->GetCharInfo()->m_ClanInfo.m_nClanID);
				if (pClan == NULL) break;

				char szBuffer[128];
				pCommand->GetParameter(szBuffer, 0, MPT_STR, sizeof(szBuffer));

				if (GetDBMgr()->UpdatePasswordByName(szBuffer, pClan->GetCLID()))
				{
					MCommand* pCmd = CreateCommand(MC_MATCH_CLAN_PASSWORDRESPONSE, MUID(0, 0));
					char buff[128];
					sprintf(buff, "Successfully Updated (%s).", szBuffer);
					pCmd->AddParameter(new MCmdParamStr(buff));
					RouteToListener(pObj, pCmd);
				}
			}
			break;
		case MC_MATCH_REMOVE_PASSWORD:
			{
				MMatchObject* pObj = GetObject(pCommand->GetSenderUID());

				int nClanGrade = pObj->GetCharInfo()->m_ClanInfo.m_nGrade;
				MMatchClanGrade grade = (MMatchClanGrade)nClanGrade;
				MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());

				if (pObj == NULL) break;
				if (!pObj->GetCharInfo()->m_ClanInfo.IsJoined()) break;
				if (pObj->GetCharInfo()->m_ClanInfo.m_nClanID == NULL) break;
				if (grade != MCG_MASTER) break;

				MMatchClan* pClan = FindClan(pObj->GetCharInfo()->m_ClanInfo.m_nClanID);
				if (pClan == NULL) break;


				if (GetDBMgr()->RemovePasswordByName(pClan->GetCLID()))
				{
					MCommand* pCmd = CreateCommand(MC_MATCH_CLAN_PASSWORDRESPONSE, MUID(0, 0));
					pCmd->AddParameter(new MCmdParamStr("Successfully unlocked."));
					RouteToListener(pObj, pCmd);
				}
			}
			break;
		case MC_MATCH_IGNORE_ADD:
			{
				// char szArg[MAX_CHARNAME];
				char szArg[MAX_CHARNAME_LENGTH];
				pCommand->GetParameter(szArg, 0, MPT_STR, MAX_CHARNAME_LENGTH);

				OnIgnoreAdd(pCommand->GetSenderUID(), szArg);
			}
			break;
		case MC_MATCH_IGNORE_REMOVE:
			{
				char szArg[MAX_CHARNAME_LENGTH];
				pCommand->GetParameter(szArg, 0, MPT_STR, MAX_CHARNAME_LENGTH);

				OnIgnoreRemove(pCommand->GetSenderUID(), szArg);
			}
			break;
			case MC_MATCH_IGNORE_LIST:
			{
				OnIgnoreList(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_STAGE_REQUEST_FORCED_ENTRY:
			{
				MUID uidPlayer, uidStage;
				uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&uidStage, 1, MPT_UID);
				
				OnRequestForcedEntry(uidStage, uidPlayer);
			}
			break;
		case MC_MATCH_FRIEND_ADD:
			{
				// char szArg[MAX_CHARNAME];
				char szArg[ MAX_CHARNAME_LENGTH ];
				pCommand->GetParameter(szArg, 0, MPT_STR, MAX_CHARNAME_LENGTH );

				OnFriendAdd(pCommand->GetSenderUID(), szArg);
			}
			break;
		case MC_MATCH_FRIEND_REMOVE:
			{
				// char szArg[MAX_CHARNAME];
				char szArg[ MAX_CHARNAME_LENGTH ];
				pCommand->GetParameter(szArg, 0, MPT_STR, MAX_CHARNAME_LENGTH );

				OnFriendRemove(pCommand->GetSenderUID(), szArg);
			}
			break;
		case MC_MATCH_FRIEND_LIST:
			{
				OnFriendList(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_FRIEND_MSG:
			{
				char szArg[CHAT_STRING_LEN];
				pCommand->GetParameter(szArg, 0, MPT_STR, CHAT_STRING_LEN );

				OnFriendMsg(pCommand->GetSenderUID(), szArg);
			}
			break;
		case MC_ADMIN_CHAT:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				MMatchObject* pObj = GetObject(uidPlayer);
				if (pObj == NULL) break;
				if (!IsEnabledObject(pObj)) break;
				if (!IsAdminGrade(pObj)) break;

				char szMessage[CHAT_STRING_LEN];
				if (pCommand->GetParameter(szMessage, 0, MPT_STR, CHAT_STRING_LEN) == false) break;
				if (strlen(szMessage) < 0) break;

				char buff[128];
				sprintf(buff, "^2[S]%s : %s", pObj->GetName(), szMessage);
				MCommand* pCmd = CreateCommand(MC_ADMIN_CHAT, MUID(0, 0));
				pCmd->AddParameter(new MCmdParamStr(buff));
				RouteToStaff(pCmd);
			}
			break;
		case MC_ADMIN_ALLCOMMANDS_NOTARGET:
			{
				char szMessage[CHAT_STRING_LEN] = "";

				if (pCommand->GetParameter(szMessage, 0, MPT_STR, CHAT_STRING_LEN) == false) break;

				MMatchObject* pObject = GetObject(pCommand->GetSenderUID());
				if (pObject == NULL) break;
				if (!IsAdminGrade(pObject))
				{
					break;
				}
				MMatchStage* pStage = FindStage(pObject->GetStageUID());
				if (pStage == NULL) break;
				//if (!strstr(pStage->GetName(), "Event")) break;

				string command = (string)szMessage;
				char buff[128];
				if ((command.find("kickall") != string::npos)) {
					sprintf(buff, "%s kicked you all", pObject->GetName());
					MMatchServer* pServer = MMatchServer::GetInstance();
					for (MUIDRefCache::iterator i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {
						MUID uidObj = (MUID)(*i).first;
						MMatchObject* pObj = (MMatchObject*)pServer->GetObject(uidObj);
						if (pObj->GetUID() == pObject->GetUID()) continue;
						if (pObj) {
							pServer->StageLeave(pObj->GetUID());
							MCommand* pCmd = CreateCommand(MC_ADMIN_ALLCOMMANDS_NOTARGET, MUID(0, 0));
							pCmd->AddParameter(new MCmdParamStr(buff));
							RouteToListener(pObj, pCmd);
						}
						else {
							LOG(LOG_PROG, "WARNING(RouteToStage) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
							i = pStage->RemoveObject(uidObj);
							LogObjectCommandHistory(uidObj);
						}
					}
					break;
				}

				for (MUIDRefCache::iterator i = pStage->GetObjBegin(); i != pStage->GetObjEnd(); i++) {
					MUID uidObj = (MUID)(*i).first;
					MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
					if (pObj->GetUID() == pObject->GetUID()) continue;
					if (pObj) {
						MCommand* pCmd = CreateCommand(MC_ADMIN_ALLCOMMANDS_NOTARGET, MUID(0, 0));
						pCmd->AddParameter(new MCmdParamStr(szMessage));
						RouteToListener(pObj, pCmd);
					}
					else {
						LOG(LOG_PROG, "WARNING(RouteToStage) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
						i = pStage->RemoveObject(uidObj);
						LogObjectCommandHistory(uidObj);
					}
				}
				break;

			}
			break;
		case MC_ADMIN_ALLCOMMANDS:
			{
				char szTargetName[MAX_CHARNAME_LENGTH] = "";
				char szMessage[CHAT_STRING_LEN] = "";

				if (pCommand->GetParameter(szTargetName, 0, MPT_STR, MAX_CHARNAME_LENGTH) == false) break;
				if (pCommand->GetParameter(szMessage, 1, MPT_STR, CHAT_STRING_LEN) == false) break;

				MMatchObject* pObj = GetObject(pCommand->GetSenderUID());
				if (pObj == NULL) break;
				if (!IsAdminGrade(pObj))
				{
					
					break;
				}

				MMatchObject* pTargetObj = GetPlayerByName(szTargetName);
				if (pTargetObj == NULL)
				{
					NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
					break;
				}

				MCommand* pCmd = CreateCommand(MC_ADMIN_ALLCOMMANDS, MUID(0, 0));
				pCmd->AddParameter(new MCmdParamStr(pObj->GetName()));
				pCmd->AddParameter(new MCmdParamStr(szMessage));
				RouteToListener(pTargetObj, pCmd);

			}
			break;
		case MC_ADMIN_ANNOUNCE:
			{
				MUID uidAdmin;
				static char szChat[ ANNOUNCE_STRING_LEN ];
				unsigned long int nMsgType = 0;

				pCommand->GetParameter(&uidAdmin, 0, MPT_UID);
				pCommand->GetParameter(szChat, 1, MPT_STR, ANNOUNCE_STRING_LEN );
				pCommand->GetParameter(&nMsgType, 2, MPT_UINT);

				// OnAdminAnnounce(uidAdmin, szChat, nMsgType);
				OnAdminAnnounce(pCommand->GetSenderUID(), szChat, nMsgType);
			}
			break;
		case MC_ADMIN_TERMINAL:
			{
				// MUID uidAdmin;
				char szText[1024];
				
				// pCommand->GetParameter(&uidAdmin, 0, MPT_UID);
				pCommand->GetParameter(szText, 1, MPT_STR, sizeof(szText) );

				//OnAdminTerminal(uidAdmin, szText);
				OnAdminTerminal(pCommand->GetSenderUID(), szText);

			}
			break;
		case MC_ADMIN_REQUEST_SERVER_INFO:
			{
				// MUID uidAdmin;
				// pCommand->GetParameter(&uidAdmin, 0, MPT_UID);

				//OnAdminRequestServerInfo(uidAdmin);
				OnAdminRequestServerInfo(pCommand->GetSenderUID());
			}
			break;
		case MC_ADMIN_SERVER_HALT:
			{
				MUID uidAdmin = pCommand->GetSenderUID();
				LOG(LOG_PROG, "Command(MC_ADMIN_SERVER_HALT) Received - Shutdown Start(%d%d)", uidAdmin.High, uidAdmin.Low);
				OnAdminServerHalt(uidAdmin);
			}
			break;
		case MC_ADMIN_REQUEST_KICK_PLAYER:
			{
				char szPlayer[512];
				pCommand->GetParameter(szPlayer, 0, MPT_STR, sizeof(szPlayer) );

				OnAdminRequestKickPlayer(pCommand->GetSenderUID(), szPlayer);
			}
			break;

		case MC_ADMIN_REQUEST_MUTE_PLAYER:
			{
				int nPunishHour;
				char szPlayer[512];

				pCommand->GetParameter(szPlayer, 0, MPT_STR, sizeof(szPlayer) );
				pCommand->GetParameter(&nPunishHour, 1, MPT_INT);

				OnAdminRequestMutePlayer(pCommand->GetSenderUID(), szPlayer, nPunishHour);
			}
			break;

		case MC_ADMIN_REQUEST_BLOCK_PLAYER:
			{
				int nPunishHour;
				char szPlayer[512];

				pCommand->GetParameter(szPlayer, 0, MPT_STR, sizeof(szPlayer) );
				pCommand->GetParameter(&nPunishHour, 1, MPT_INT);

				OnAdminRequestBlockPlayer(pCommand->GetSenderUID(), szPlayer, nPunishHour);
			}
			break;

		case MC_ADMIN_REQUEST_UPDATE_ACCOUNT_UGRADE:
			{
				// MUID uidAdmin;
				char szPlayer[512];

				// pCommand->GetParameter(&uidAdmin, 0, MPT_UID);
				pCommand->GetParameter(szPlayer, 1, MPT_STR, sizeof(szPlayer) );
			
				//OnAdminRequestUpdateAccountUGrade(uidAdmin, szPlayer);
				OnAdminRequestUpdateAccountUGrade(pCommand->GetSenderUID(), szPlayer);
			}
			break;
		case MC_ADMIN_REQUEST_SWITCH_LADDER_GAME:
			{
				// MUID uidAdmin;
				bool bEnabled;

				// pCommand->GetParameter(&uidAdmin, 0, MPT_UID);
				pCommand->GetParameter(&bEnabled, 1, MPT_BOOL);
			
				//OnAdminRequestSwitchLadderGame(uidAdmin, bEnabled);
				OnAdminRequestSwitchLadderGame(pCommand->GetSenderUID(), bEnabled);
			}
			break;
		case MC_ADMIN_HIDE:
			{
				OnAdminHide(pCommand->GetSenderUID());
			}
			break;
		case MC_ADMIN_PING_TO_ALL:
			{
				OnAdminPingToAll(pCommand->GetSenderUID());
			}
			break;
		case MC_ADMIN_RESET_ALL_HACKING_BLOCK :
			{
				OnAdminResetAllHackingBlock( pCommand->GetSenderUID() );
			}
			break;
		case MC_EVENT_CHANGE_MASTER:
			{
				OnEventChangeMaster(pCommand->GetSenderUID());
			}
			break;
		case MC_EVENT_CHANGE_PASSWORD:
			{
				char szPassword[ STAGEPASSWD_LENGTH ]="";
				pCommand->GetParameter(szPassword, 0, MPT_STR, STAGEPASSWD_LENGTH );

				OnEventChangePassword(pCommand->GetSenderUID() ,szPassword);
			}
			break;
		case MC_EVENT_REQUEST_JJANG:
			{
				char szTargetName[128]="";
				pCommand->GetParameter(szTargetName, 0, MPT_STR, sizeof(szTargetName) );
				OnEventRequestJjang(pCommand->GetSenderUID(), szTargetName);
			}
			break;
		case MC_EVENT_REMOVE_JJANG:
			{
				char szTargetName[128]="";
				pCommand->GetParameter(szTargetName, 0, MPT_STR, sizeof(szTargetName) );
				OnEventRemoveJjang(pCommand->GetSenderUID(), szTargetName);
			}
			break;
		
		case MC_MATCH_REQUEST_ACCOUNT_ITEMLIST:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				OnRequestAccountItemList(uidPlayer);
			}
			break;

		///< AccountItem 관련하여 추가된 커맨드
		// 홍기주(2010-03-09)
		/*
		case MC_MATCH_REQUEST_MOVE_ACCOUNTITEM_IN_BANK : 
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				int nAIIDFrom = 0;
				int nAIIDTo = 0;
				int nCount = 0;

				pCommand->GetParameter(&nAIIDFrom, 0, MPT_INT);
				pCommand->GetParameter(&nAIIDTo, 1, MPT_INT);
				pCommand->GetParameter(&nCount, 2, MPT_INT);

				OnRequestAccountItemMoveInBank(uidPlayer, nAIIDFrom, nAIIDTo, nCount);
			}
			break;
		*/
		///< 여기까지
		
		case MC_MATCH_REQUEST_SUICIDE:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);

				OnRequestSuicide(uidPlayer);
			}
			break;
		case MC_MATCH_REQUEST_OBTAIN_WORLDITEM:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nItemUID = 0;

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&nItemUID, 1, MPT_INT);

				OnRequestObtainWorldItem(uidPlayer, nItemUID);
			}
			break;
		case MC_MATCH_REQUEST_FLAG_CAP:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nItemID = 0;

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&nItemID, 0, MPT_INT);

				OnRequestFlagCap(uidPlayer, nItemID);
			}
			break;
		case MC_MATCH_REQUEST_SPAWN_WORLDITEM:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nItemID = 0;
				rvector pos;
				float nDropDelayTime = 0.f;

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(&nItemID, 1, MPT_INT);
				pCommand->GetParameter(&pos, 2, MPT_POS);
				pCommand->GetParameter(&nDropDelayTime, 3, MPT_FLOAT);

				OnRequestSpawnWorldItem(uidPlayer, nItemID, pos.x, pos.y, pos.z, nDropDelayTime);
			}
			break;
		case MC_MATCH_NOTIFY_THROW_TRAPITEM:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nItemID = 0;
				pCommand->GetParameter(&nItemID, 0, MPT_SHORT);

				OnNotifyThrowTrapItem(uidPlayer, nItemID);
			}
			break;
		case MC_MATCH_NOTIFY_ACTIVATED_TRAPITEM:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int nItemID = 0;
				MShortVector s_pos;
				pCommand->GetParameter(&nItemID, 0, MPT_SHORT);
				pCommand->GetParameter(&s_pos, 1, MPT_SVECTOR);

				OnNotifyActivatedTrapItem(uidPlayer, nItemID, MVector3(s_pos.x, s_pos.y, s_pos.z));
			}
			break;
		case MC_MATCH_SPMOTION:
			{
				MMatchObject* pObj = GetObject(pCommand->GetSenderUID());
				if (pObj == NULL) break;

				if ((timeGetTime() - pObj->st_nLastDance) < 1000)
				{
					break;
				}
				pObj->st_nLastDance = timeGetTime();

				MMatchItem* pItem = pObj->GetCharInfo()->m_EquipedItem.GetItem((MMatchCharItemParts)MMCIP_DANCE);
				
				if (pItem == NULL)
				{
					MCommand* pCmd = CreateCommand(MC_MATCH_SPMOTION_REPLY, MUID(0, 0));
					pCmd->AddParameter(new MCommandParameterInt(1010));
					pCmd->AddParameter(new MCommandParameterUID(pObj->GetUID()));
					RouteToStage(pObj->GetStageUID(), pCmd);
				}
				else
				{
					MCommand* pCmd = CreateCommand(MC_MATCH_SPMOTION_REPLY, MUID(0, 0));
					pCmd->AddParameter(new MCommandParameterInt(pItem->GetDescID()));
					pCmd->AddParameter(new MCommandParameterUID(pObj->GetUID()));
					RouteToStage(pObj->GetStageUID(), pCmd);
				}
			}
			break;
		case MC_MATCH_USER_WHISPER:
			{
				char szSenderName[ MAX_CHARNAME_LENGTH ]="";
				char szTargetName[ MAX_CHARNAME_LENGTH ]="";
				char szMessage[CHAT_STRING_LEN]="";
				
				if (pCommand->GetParameter(szSenderName, 0, MPT_STR, MAX_CHARNAME_LENGTH ) == false) break;
				if (pCommand->GetParameter(szTargetName, 1, MPT_STR, MAX_CHARNAME_LENGTH ) == false) break;
				if (pCommand->GetParameter(szMessage, 2, MPT_STR, CHAT_STRING_LEN ) == false) break;

				OnUserWhisper(pCommand->GetSenderUID(), szSenderName, szTargetName, szMessage);
			}
			break;
		case MC_MATCH_USER_WHERE:
			{
				// char szTargetName[MAX_CHARNAME]="";
				char szTargetName[ MAX_CHARNAME_LENGTH ]="";
				pCommand->GetParameter(szTargetName, 0, MPT_STR, MAX_CHARNAME_LENGTH );

				OnUserWhere(pCommand->GetSenderUID(), szTargetName);
			}
			break;
		case MC_MATCH_USER_OPTION:
			{
				unsigned long nOptionFlags=0;
				pCommand->GetParameter(&nOptionFlags, 0, MPT_UINT);

				OnUserOption(pCommand->GetSenderUID(), nOptionFlags);
			}
			break;
		case MC_MATCH_CHATROOM_CREATE:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				char szChatRoomName[MAX_CHATROOMNAME_STRING_LEN];

				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				pCommand->GetParameter(szChatRoomName, 1, MPT_STR, MAX_CHATROOMNAME_STRING_LEN );

				OnChatRoomCreate(uidPlayer, szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_JOIN:
			{
				// char szPlayerName[MAX_CHARNAME];
				char szPlayerName[ MAX_CHARNAME_LENGTH ];
				char szChatRoomName[MAX_CHATROOMNAME_STRING_LEN];
				
				pCommand->GetParameter(szPlayerName, 0, MPT_STR, MAX_CHARNAME_LENGTH );
				pCommand->GetParameter(szChatRoomName, 1, MPT_STR, MAX_CHATROOMNAME_STRING_LEN );

				OnChatRoomJoin(pCommand->GetSenderUID(), szPlayerName, szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_LEAVE:
			{
				// char szPlayerName[MAX_CHARNAME];
				char szPlayerName[ MAX_CHARNAME_LENGTH ];
				char szChatRoomName[MAX_CHATROOMNAME_STRING_LEN];
				
				pCommand->GetParameter(szPlayerName, 0, MPT_STR, MAX_CHARNAME_LENGTH );
				pCommand->GetParameter(szChatRoomName, 1, MPT_STR, MAX_CHATROOMNAME_STRING_LEN );

				OnChatRoomLeave(pCommand->GetSenderUID(), szPlayerName, szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_SELECT_WRITE:
			{
				char szChatRoomName[MAX_CHATROOMNAME_STRING_LEN];

				pCommand->GetParameter(szChatRoomName, 0, MPT_STR, MAX_CHATROOMNAME_STRING_LEN );

				OnChatRoomSelectWrite(pCommand->GetSenderUID(), szChatRoomName);
			}
			break;
		case MC_MATCH_CHATROOM_INVITE:
			{
				char szSenderName[ MAX_CHARNAME_LENGTH ]="";
				char szTargetName[MAX_CHARNAME_LENGTH ]="";
				char szRoomName[MAX_CHATROOMNAME_STRING_LEN]="";

				pCommand->GetParameter(szSenderName, 0, MPT_STR, MAX_CHARNAME_LENGTH );
				pCommand->GetParameter(szTargetName, 1, MPT_STR, MAX_CHARNAME_LENGTH );
				pCommand->GetParameter(szRoomName, 2, MPT_STR, MAX_CHATROOMNAME_STRING_LEN );

				OnChatRoomInvite(pCommand->GetSenderUID(), szTargetName);
			}
			break;
		case MC_MATCH_CHATROOM_CHAT:
			{
				char szRoomName[MAX_CHATROOMNAME_STRING_LEN];
				// char szPlayerName[MAX_CHARNAME];
				char szPlayerName[ MAX_CHARNAME_LENGTH ];
				// char szMessage[256];
				char szMessage[CHAT_STRING_LEN];

				pCommand->GetParameter(szRoomName, 0, MPT_STR, MAX_CHATROOMNAME_STRING_LEN );
				pCommand->GetParameter(szPlayerName, 1, MPT_STR, MAX_CHARNAME_LENGTH );
				pCommand->GetParameter(szMessage, 2, MPT_STR, CHAT_STRING_LEN);

				OnChatRoomChat(pCommand->GetSenderUID(), szMessage);
			}
			break;
		case MC_MATCH_REQUEST_MY_SIMPLE_CHARINFO:
		{
			//MUID uidPlayer;
			//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
			OnRequestMySimpleCharInfo(pCommand->GetSenderUID());
		}
			break;
		case MC_MATCH_REQUEST_COPY_TO_TESTSERVER:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				OnRequestCopyToTestServer(uidPlayer);
			}
			break;
			case MC_MATCH_CLAN_REQUEST_CREATE_CLAN:
			{
				char szClanName[CLAN_NAME_LENGTH];
				char szClanPassword[CLAN_NAME_LENGTH];
				pCommand->GetParameter(szClanName, 0, MPT_STR, CLAN_NAME_LENGTH);
				pCommand->GetParameter(szClanPassword, 1, MPT_STR, CLAN_NAME_LENGTH);

				OnClanRequestCreateClan(pCommand->GetSenderUID(), szClanName, szClanPassword);
			}
			break;
		case MC_MATCH_CLAN_ANSWER_SPONSOR_AGREEMENT:
			{
				MUID uidClanMaster;
				int nRequestID;
				bool bAnswer;
				// char szCharName[MAX_CHARNAME];
				char szCharName[ MAX_CHARNAME_LENGTH ];

				pCommand->GetParameter(&nRequestID,		0, MPT_INT);
				pCommand->GetParameter(&uidClanMaster,	1, MPT_UID);
				pCommand->GetParameter(szCharName,		2, MPT_STR, MAX_CHARNAME_LENGTH );
				pCommand->GetParameter(&bAnswer,		3, MPT_BOOL);

				OnClanAnswerSponsorAgreement(nRequestID, uidClanMaster, szCharName, bAnswer);
			}
			break;

		case MC_MATCH_CLAN_REQUEST_CLOSE_CLAN:
			{
				MUID uidClanMaster;
				char szClanName[CLAN_NAME_LENGTH];

				uidClanMaster = pCommand->GetSenderUID();

				// pCommand->GetParameter(&uidClanMaster,		0, MPT_UID);
				pCommand->GetParameter(szClanName, 1, MPT_STR, CLAN_NAME_LENGTH );

				OnClanRequestCloseClan(uidClanMaster, szClanName);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_JOIN_CLAN:
			{
				// MUID uidClanAdmin;
				char szClanName[CLAN_NAME_LENGTH];
				// char szJoiner[MAX_CHARNAME];
				char szJoiner[ MAX_CHARNAME_LENGTH ];

				// pCommand->GetParameter(&uidClanAdmin,	0, MPT_UID);
				pCommand->GetParameter(szClanName,		1, MPT_STR, CLAN_NAME_LENGTH );
				pCommand->GetParameter(szJoiner,		2, MPT_STR, MAX_CHARNAME_LENGTH );

				OnClanRequestJoinClan(pCommand->GetSenderUID(), szClanName, szJoiner);
			}
			break;
		case MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT:
			{
				MUID uidClanAdmin;
				bool bAnswer;
				// char szJoiner[MAX_CHARNAME];
				char szJoiner[ MAX_CHARNAME_LENGTH ];

				// uidClanAdmin = pCommand->GetSenderUID();

				pCommand->GetParameter(&uidClanAdmin,	0, MPT_UID);
				pCommand->GetParameter(szJoiner,		1, MPT_STR, MAX_CHARNAME_LENGTH );
				pCommand->GetParameter(&bAnswer,		2, MPT_BOOL);

				OnClanAnswerJoinAgreement(uidClanAdmin, szJoiner, bAnswer);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_AGREED_JOIN_CLAN:
			{
				///< 이 커맨드는 건들지 말자...		

				// MUID uidClanAdmin;
				char szClanName[CLAN_NAME_LENGTH];
				// char szJoiner[MAX_CHARNAME];
				char szJoiner[ MAX_CHARNAME_LENGTH ];

				// pCommand->GetParameter(&uidClanAdmin,	0, MPT_UID);
				pCommand->GetParameter(szClanName,		1, MPT_STR, CLAN_NAME_LENGTH );
				// 가입자 구분을 왜 이름으로 하는지 모르겠음...-_-;;
				pCommand->GetParameter(szJoiner,		2, MPT_STR, MAX_CHARNAME_LENGTH );

				OnClanRequestAgreedJoinClan(pCommand->GetSenderUID(), szClanName, szJoiner);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_LEAVE_CLAN:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer,	0, MPT_UID);

				OnClanRequestLeaveClan(uidPlayer);
			}
			break;
		case MC_MATCH_CLAN_MASTER_REQUEST_CHANGE_GRADE:
			{
				MUID uidClanMaster;
				// char szMember[MAX_CHARNAME];
				char szMember[ MAX_CHARNAME_LENGTH ];
				int nClanGrade;

				uidClanMaster = pCommand->GetSenderUID();

				// pCommand->GetParameter(&uidClanMaster,	0, MPT_UID);
				pCommand->GetParameter(szMember,		1, MPT_STR, MAX_CHARNAME_LENGTH );
				pCommand->GetParameter(&nClanGrade,		2, MPT_INT);

				OnClanRequestChangeClanGrade(uidClanMaster, szMember, nClanGrade);
			}
			break;
		case MC_MATCH_CLAN_ADMIN_REQUEST_EXPEL_MEMBER:
			{
				MUID uidClanAdmin;
				// char szMember[MAX_CHARNAME];
				char szMember[ MAX_CHARNAME_LENGTH ];

				uidClanAdmin = pCommand->GetSenderUID();
				// pCommand->GetParameter(&uidClanAdmin,	0, MPT_UID);
				pCommand->GetParameter(szMember,		1, MPT_STR, MAX_CHARNAME_LENGTH );

				OnClanRequestExpelMember(uidClanAdmin, szMember);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_MSG:
			{
				MUID uidSender;
				char szMsg[MSG_STRING_LEN];

				uidSender = pCommand->GetSenderUID();
				// pCommand->GetParameter(&uidSender,	0, MPT_UID);
				pCommand->GetParameter(szMsg,		1, MPT_STR, MSG_STRING_LEN );

				OnClanRequestMsg(uidSender, szMsg);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_MEMBER_LIST:
			{
				MUID uidChar;

				uidChar = pCommand->GetSenderUID();
				// pCommand->GetParameter(&uidChar,	0, MPT_UID);

				OnClanRequestMemberList(uidChar);
			}
			break;
		case MC_MATCH_CLAN_REQUEST_CLAN_INFO:
			{
				MUID uidChar;
				char szClanName[CLAN_NAME_LENGTH] = "";

				uidChar = pCommand->GetSenderUID();
				// pCommand->GetParameter(&uidChar,	0, MPT_UID);
				
				pCommand->GetParameter(szClanName,	1, MPT_STR, CLAN_NAME_LENGTH );

				OnClanRequestClanInfo(uidChar, szClanName);

			}
			break;
		case MC_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST:
			{
				MUID uidPlayer, uidChannel;
				unsigned long int nPlaceFilter;
				unsigned long int nOptions;
				uidPlayer = pCommand->GetSenderUID();

				//pCommand->GetParameter(&uidPlayer,		0, MPT_UID);
				pCommand->GetParameter(&uidChannel,		1, MPT_UID);
				pCommand->GetParameter(&nPlaceFilter,	2, MPT_UINT);
				pCommand->GetParameter(&nOptions,		3, MPT_UINT);

				OnChannelRequestAllPlayerList(uidPlayer, uidChannel, nPlaceFilter, nOptions);
			}
			break;
		case MC_MATCH_REQUEST_CHARINFO_DETAIL:
			{
				// MUID uidChar;
				char szCharName[ MAX_CHARNAME_LENGTH ];

				// pCommand->GetParameter(&uidChar,	0, MPT_UID);
				pCommand->GetParameter(szCharName,	1, MPT_STR, MAX_CHARNAME_LENGTH );

				OnRequestCharInfoDetail(pCommand->GetSenderUID(), szCharName);
			}
			break;
		case MC_MATCH_LADDER_REQUEST_CHALLENGE:
			{
				int nMemberCount;
				unsigned long int nOptions;
				unsigned long int nAntiLead;
				unsigned long int nDodge;
				bool warmUp;
				pCommand->GetParameter(&nMemberCount,		0, MPT_INT);
				pCommand->GetParameter(&nOptions,			1, MPT_UINT);
				pCommand->GetParameter(&nAntiLead,			3, MPT_UINT);
				pCommand->GetParameter(&nDodge, 4, MPT_UINT);
				pCommand->GetParameter(&warmUp, 5, MPT_BOOL); //from 4 to 5 ~ Monckey100
				MCommandParameter* pMemberNamesBlobParam = pCommand->GetParameter(2);
				if(pMemberNamesBlobParam->GetType()!=MPT_BLOB) break;
				void* pMemberNamesBlob = pMemberNamesBlobParam->GetPointer();
				if( NULL == pMemberNamesBlob )
					break;
				
				OnLadderRequestChallenge(pCommand->GetSenderUID(), pMemberNamesBlob, nOptions, nAntiLead, nDodge, warmUp);
			}
			break;
		case MC_MATCH_LADDER_REQUEST_CANCEL_CHALLENGE:
			{
				OnLadderRequestCancelChallenge(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_LADDER_REQUEST_CANCEL_CHALLENGE_NEW:
			{
				string x = "Clan war just got cancelled";
				LOG(LOG_DEBUG,x.c_str());
				OnLadderRequestCancelChallengeNew(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_REQUEST_PROPOSAL:
			{
				MUID uidChar;
				int nProposalMode, nRequestID, nReplierCount;

				uidChar = pCommand->GetSenderUID();
				// pCommand->GetParameter(&uidChar,			0, MPT_UID);

				pCommand->GetParameter(&nProposalMode,		1, MPT_INT);
				pCommand->GetParameter(&nRequestID,			2, MPT_INT);
				pCommand->GetParameter(&nReplierCount,		3, MPT_INT);

				MCommandParameter* pReplierNamesParam = pCommand->GetParameter(4);
				if(pReplierNamesParam->GetType()!=MPT_BLOB) break;

				void* pReplierNamesBlob = pReplierNamesParam->GetPointer();
				if( NULL == pReplierNamesBlob )
					break;

				OnRequestProposal(uidChar, nProposalMode, nRequestID, nReplierCount, pReplierNamesBlob);
			}
			break;
		case MC_MATCH_REPLY_AGREEMENT:
			{
				MUID uidProposer, uidReplier;
				char szReplierName[256];
				int nProposalMode, nRequestID;
				bool bAgreement;

				pCommand->GetParameter(&uidProposer,	0, MPT_UID);

				// pCommand->GetParameter(&uidReplier,		1, MPT_UID);
				uidReplier = pCommand->GetSenderUID();

				pCommand->GetParameter(szReplierName,	2, MPT_STR, sizeof(szReplierName) );
				pCommand->GetParameter(&nProposalMode,	3, MPT_INT);
				pCommand->GetParameter(&nRequestID,		4, MPT_INT);
				pCommand->GetParameter(&bAgreement,		5, MPT_BOOL);

				OnReplyAgreement(uidProposer, uidReplier, szReplierName, nProposalMode, nRequestID, bAgreement);
			}
			break;
		case MC_MATCH_CALLVOTE:
			{
				char szDiscuss[ VOTE_DISCUSS_STRING_LEN ]="";
				char szArg[ VOTE_ARG_STRING_LEN ]="";
				
				pCommand->GetParameter(szDiscuss, 0, MPT_STR, VOTE_DISCUSS_STRING_LEN );
				pCommand->GetParameter(szArg, 1, MPT_STR, VOTE_ARG_STRING_LEN );
				OnVoteCallVote(pCommand->GetSenderUID(), szDiscuss, szArg);
			}
			break;
		case MC_MATCH_VOTE_YES:
			{
				OnVoteYes(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_VOTE_NO:
			{
				OnVoteNo(pCommand->GetSenderUID());
			}
			break;
		case MC_MATCH_CLAN_REQUEST_EMBLEMURL:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if (pParam->GetType() != MPT_BLOB)
				{
					break;
				}
				void* pEmblemURLListBlob = pParam->GetPointer();
				if( NULL == pEmblemURLListBlob )
					break;

				OnClanRequestEmblemURL(uidPlayer, pEmblemURLListBlob);
			}
			break;
		
		case MC_QUEST_REQUEST_NPC_DEAD:
			{
				MUID uidKiller, uidNPC;
				MShortVector s_pos;
				pCommand->GetParameter(&uidKiller,	0, MPT_UID);
				pCommand->GetParameter(&uidNPC,		1, MPT_UID);
				pCommand->GetParameter(&s_pos,		2, MPT_SVECTOR);

				MUID uidSender = pCommand->GetSenderUID();
				MVector pos = MVector((float)s_pos.x, (float)s_pos.y, (float)s_pos.z);

				OnRequestNPCDead(uidSender, uidKiller, uidNPC, pos);

			}
			break;
		case MC_MATCH_QUEST_REQUEST_DEAD:
			{
				MUID uidVictim = pCommand->GetSenderUID();
				OnQuestRequestDead(uidVictim);
			}
			break;
#ifdef _DEBUG
		case MC_QUEST_TEST_REQUEST_NPC_SPAWN:
			{
				int nNPCType, nNPCCount;

				pCommand->GetParameter(&nNPCType,	0, MPT_INT);
				pCommand->GetParameter(&nNPCCount,	1, MPT_INT);
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestTestRequestNPCSpawn(uidPlayer, nNPCType, nNPCCount);
			}
			break;
		case MC_QUEST_TEST_REQUEST_CLEAR_NPC:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestTestRequestClearNPC(uidPlayer);
			}
			break;
		case MC_QUEST_TEST_REQUEST_SECTOR_CLEAR:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestTestRequestSectorClear(uidPlayer);
			}
			break;
		case MC_QUEST_TEST_REQUEST_FINISH:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestTestRequestQuestFinish(uidPlayer);
			}
			break;
#endif
		case MC_QUEST_REQUEST_MOVETO_PORTAL:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestRequestMovetoPortal(uidPlayer);
			}
			break;
		case MC_QUEST_READYTO_NEWSECTOR:
			{
				MUID uidPlayer = pCommand->GetSenderUID();

				OnQuestReadyToNewSector(uidPlayer);
			}
			break;

		case MC_QUEST_PONG:
			{
				MUID uidPlayer = pCommand->GetSenderUID();
//				unsigned long int nTimeStamp;
//				
//				pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);

				OnQuestPong(uidPlayer);
			}
			break;

		case MC_MATCH_REQUEST_CHAR_QUEST_ITEM_LIST :
			{
				MUID uidChar;

				pCommand->GetParameter( &uidChar, 0, MPT_UID );

				OnRequestCharQuestItemList( uidChar );
			}
			break;

		case MC_MATCH_REQUEST_DROP_SACRIFICE_ITEM :
			{
				// MUID	uidQuestItemOwner;
				int		nSlotIndex;
				int		nItemID;

				// pCommand->GetParameter( &uidQuestItemOwner, 0, MPT_UID );
				pCommand->GetParameter( &nSlotIndex, 1, MPT_INT );
				pCommand->GetParameter( &nItemID, 2, MPT_INT );

				OnRequestDropSacrificeItemOnSlot( pCommand->GetSenderUID(), nSlotIndex, nItemID );
			}
			break;

		case MC_MATCH_REQUEST_CALLBACK_SACRIFICE_ITEM :
			{
				MUID uidPlayer = pCommand->GetSenderUID();
				int		nSlotIndex;
				int		nItemID;

				//pCommand->GetParameter( &uidPlayer, 0, MPT_UID );
				pCommand->GetParameter( &nSlotIndex, 1, MPT_INT );
				pCommand->GetParameter( &nItemID, 2, MPT_INT );

				OnRequestCallbackSacrificeItem( uidPlayer, nSlotIndex, nItemID );
			}
			break;

		case MC_MATCH_REQUEST_BUY_QUEST_ITEM :
			{
				int	nItemID;
				int	nItemCount;

				pCommand->GetParameter( &nItemID, 1, MPT_INT );
				pCommand->GetParameter( &nItemCount, 2, MPT_INT );

				OnRequestBuyQuestItem( pCommand->GetSenderUID(), nItemID, nItemCount );
			}
			break;

		case MC_MATCH_REQUEST_SELL_QUEST_ITEM :
			{
				int		nItemID;
				int		nCount;

				pCommand->GetParameter( &nItemID, 1, MPT_INT );
				pCommand->GetParameter( &nCount, 2, MPT_INT );

				OnRequestSellQuestItem( pCommand->GetSenderUID(), nItemID, nCount );
			}
			break;

		case MC_QUEST_REQUEST_QL :
			{
				// MUID uidSender;

				// pCommand->GetParameter( &uidSender, 0, MPT_UID );

				OnRequestQL( pCommand->GetSenderUID() );
			}
			break;

		case MC_MATCH_REQUEST_SLOT_INFO :
			{
				//MUID uidSender;

				//pCommand->GetParameter( &uidSender, 0, MPT_UID );

				OnRequestSacrificeSlotInfo( pCommand->GetSenderUID() );
			}
			break;

		case MC_QUEST_STAGE_MAPSET :
			{
				MUID uidStage;
				char nMapsetID;

				pCommand->GetParameter( &uidStage,	0, MPT_UID );
				pCommand->GetParameter( &nMapsetID, 1, MPT_CHAR );

				OnQuestStageMapset(uidStage, (int)nMapsetID );
			}
			break;
		case MC_MATCH_REQUEST_MONSTER_BIBLE_INFO :
			{
				MUID uidSender;

				pCommand->GetParameter( &uidSender, 0, MPT_UID );

				OnRequestMonsterBibleInfo( uidSender );
			}
			break;

		case MC_RESPONSE_XTRAP_HASHVALUE :
			{
			}
			break;

		case MC_RESPONSE_RESOURCE_CRC32 :
			{
				DWORD dwResourceXORCache = 0;
				DWORD dwResourceCRC32Cache = 0;

				pCommand->GetParameter( &dwResourceCRC32Cache, 0, MPT_UINT );
				pCommand->GetParameter( &dwResourceXORCache, 1, MPT_UINT );

				//_ASSERT( 0 != dwResourceXORCache );
				//_ASSERT( 0 != dwResourceCRC32Cache );

				MMatchObject* pObj = GetObject( pCommand->GetSenderUID() );
				if( NULL == pObj )
				{
					return false;
				}

				MMatchStage* pStage = FindStage( pObj->GetStageUID() );
				if( NULL == pStage )
				{
					// 유저를 로비나 방으로 보내주어야 한다.
					//_ASSERT( 0 );
					return false;
				}

				if( !pStage->IsValidResourceCRC32Cache(pCommand->GetSenderUID(), dwResourceCRC32Cache, dwResourceXORCache) )
				{
					// 유저를 로비나 방으로 보내주어야 한다.
					StageLeave( pObj->GetUID());//, pObj->GetStageUID() );
					return false;
				}
			}
			break;

		case MC_NET_BANPLAYER_FLOODING:
			{
				if (pCommand->GetSenderUID() != GetUID()) return true; //why remove a packet that disconnects flooders when u can uid check it as it originates from mserver lol
				MUID uidPlayer;

				pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
				if (MGetServerConfig()->IsUseBlockFlooding())
				{
					MMatchObject* pObj = GetObject(uidPlayer);
					if (pObj && pObj->GetDisconnStatusInfo().GetStatus() == MMDS_CONNECTED)
					{
						if (pObj->GetAccountName()) {
							LOG(LOG_PROG, "Ban Player On Flooding - (MUID:%d%d, ID:%s)"
								, uidPlayer.High, uidPlayer.Low, pObj->GetAccountName());
						}
						else {
							LOG(LOG_PROG, "Ban Player On Flooding - (MUID:%d%d, ID:%s)"
								, uidPlayer.High, uidPlayer.Low);
						}

						pObj->DisconnectHacker(MMHT_COMMAND_FLOODING);
					}
					else
					{
						LOG(LOG_PROG, "Ban Player On Flooding - Can't Find Object");
					}
				}
			}
			break;
		///< 여기까지

		case MC_MATCH_DUELTOURNAMENT_REQUEST_JOINGAME:
		{
			//MUID uidPlayer;
			MDUELTOURNAMENTTYPE nType;

			//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
			pCommand->GetParameter(&nType, 1, MPT_INT);

			if (MGetServerConfig()->IsEnabledDuelTournament())    {
				ResponseDuelTournamentJoinChallenge(pCommand->GetSenderUID(), nType);
			}

		}
			break;

		case MC_MATCH_DUELTOURNAMENT_REQUEST_CANCELGAME:
		{
			//MUID uidPlayer;
			MDUELTOURNAMENTTYPE nType;

			//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
			pCommand->GetParameter(&nType, 1, MPT_INT);

			if (MGetServerConfig()->IsEnabledDuelTournament())    {
				ResponseDuelTournamentCancelChallenge(pCommand->GetSenderUID(), nType);
			}
		}
			break;
		case MC_MATCH_DUELTOURNAMENT_REQUEST_SIDERANKING_INFO:
		{
			//MUID uidPlayer;

			//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);

			if (MGetServerConfig()->IsEnabledDuelTournament()){
				ResponseDuelTournamentCharSideRanking(pCommand->GetSenderUID());
			}
		}
			break;
 
		case MC_MATCH_DUELTOURNAMENT_GAME_PLAYER_STATUS:
		{
			//MUID uidPlayer;
			//pCommand->GetParameter(&uidPlayer, 0, MPT_UID);

			if (MGetServerConfig()->IsEnabledDuelTournament()){
				ResponseDuelTournamentCharStatusInfo(pCommand->GetSenderUID(), pCommand);
			}
		}
			break;

		case MC_MATCH_REQUEST_USE_SPENDABLE_NORMAL_ITEM :
			{
				MUID uidItem, uidPlayer;

				uidPlayer = pCommand->GetSenderUID();
				pCommand->GetParameter(&uidItem, 0, MPT_UID);

				OnRequestUseSpendableNormalItem(uidPlayer, uidItem);			
			}
			break;

		case MC_MATCH_REQUEST_USE_SPENDABLE_BUFF_ITEM :
			{
				MUID uidItem;
				pCommand->GetParameter(&uidItem, 0, MPT_UID);

				//버프정보임시주석 OnRequestUseSpendableBuffItem(pCommand->GetSenderUID(), uidItem);
				//_ASSERT(0);//주석처리했으니 여긴 들어오지 않는 것으로 가정함
			}
			break;

		default:
//			//_ASSERT(0);	// 아직 핸들러가 없다.
			return false;
	}

_STATUS_CMD_END;


	return true;
}