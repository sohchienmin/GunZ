#include "stdafx.h"

#include <winsock2.h>
#include "MErrorTable.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include "MSharedCommandTable.h"
#include "MCommandLogFrame.h"
#include "ZIDLResource.h"
#include "MBlobArray.h"
#include "ZInterface.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MMatchChannel.h"
#include "MMatchStage.h"
#include "ZPost.h"
#include "MComboBox.h"
#include "MTextArea.h"
#include "MDebug.h"
#include "ZMyInfo.h"
#include "ZNetRepository.h"
#include "ZCountDown.h"

#include "ZLanguageConf.h"

#define PROPOSAL_AGREEMENT_TIMEOUT_SEC			(30)		// 30초 타임아웃

void ShowProposalWaitFrame(bool bVisible, const char* szTitle=NULL, const char* szMessage=NULL);



void OnAskReplierAgreement_OnExpire()
{
	ZChatOutput( ZMsg(MSG_CANCELED) );
	mlog("CLAN WAR CANCELLEDDD ONASKREPLIERAGREEMENT");
	ZGetGameClient()->ReplyAgreement(false);
}


void ShowProposalWaitFrame_OnExpire()
{
	mlog("CLAN WAR CANCELLEDDD SHOWPROPOSALWAITFRAME");
	ZChatOutput( ZMsg(MSG_CANCELED) );
}

void ShowProposalWaitFrame(bool bVisible, const char* szTitle, const char* szMessage)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementWait");
	if(pWidget!=NULL)
	{
		if (bVisible)
		{
			pWidget->SetText(szTitle);

			MTextArea* pTextArea = (MTextArea*)pResource->FindWidget("ProposalAgreementWait_Textarea");
			if (pTextArea) 
			{
				pTextArea->SetText(szMessage);
			}



			static ZCOUNTDOWN countDown = {PROPOSAL_AGREEMENT_TIMEOUT_SEC,
				                           "ProposalAgreementWait_Remain",
                                           "ProposalAgreementWait",
										   ShowProposalWaitFrame_OnExpire};

			countDown.nSeconds=PROPOSAL_AGREEMENT_TIMEOUT_SEC;	// static 이므로 재설정
			ZApplication::GetTimer()->SetTimerEvent(0, &OnTimer_CountDown, &countDown, true);

			pWidget->Show(true, true);
		}
		else
		{
            pWidget->Show(false);
		}
	}
}


/// 동의을 기다리고 있는지의 여부 - ui로 체크한다.
bool IsWaitingRepliersAgreement()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MWidget* pWidget = pResource->FindWidget("ProposalAgreementWait");
	if(pWidget!=NULL)
	{
		return pWidget->IsVisible();
	}
	return false;
}




///////////////////////////////////////////////////////////////////////////////////

void ZGameClient::OnResponseProposal(const int nResult, const MMatchProposalMode nProposalMode, const int nRequestID)
{
	// 로비가 아니면 안된다.
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_LOBBY) 
	{
		m_AgreementBuilder.CancelProposal();
		return;
	}


	if (nResult == MOK)
	{
		char szTitle[256];
		char szDesc[512];
		strcpy( szTitle, 
			ZMsg(MSG_LADDER_PROPOSAL_WAIT_TITLE) );

		strcpy(szDesc, 
			ZMsg(MSG_LADDER_PROPOSAL_WAIT_DESC) );
		// 동의할동안 기다리라는 메세지창을 띄운다.
		
		ShowProposalWaitFrame(true, szTitle, szDesc);
	}
	else 
	{
		m_AgreementBuilder.CancelProposal();
		ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
	}
}

void ZGameClient::OnAskAgreement(const MUID& uidProposer, void* pMemberNamesBlob,
					const MMatchProposalMode nProposalMode, const int nRequestID)
{
	// 응답할 수 없는 상황이면 바로 거절한다.
	if ((IsWaitingRepliersAgreement()) || (!ZGetGameInterface()->IsReadyToPropose()))
	{
		char szCharName[MATCHOBJECT_NAME_LENGTH];
		sprintf(szCharName, ZGetMyInfo()->GetCharName());
		ZPostReplyAgreement(uidProposer, GetPlayerUID(), szCharName, nProposalMode, nRequestID, false);

		return;
	}

	char szMemberNames[MAX_REPLIER][256];		// 0 번째는 제안자

	int nMemberCount = MGetBlobArrayCount(pMemberNamesBlob);
	if (nMemberCount < 1) return;

	for (int i = 0; i < nMemberCount; i++)
	{
		MTD_ReplierNode* pReplierNode = (MTD_ReplierNode*)MGetBlobArrayElement(pMemberNamesBlob, i);
		strcpy(szMemberNames[i], pReplierNode->szName);
	}

	FlashWindow(FindWindow(0, APPLICATION_NAME), 1);
	m_nProposalMode = nProposalMode;
	m_nRequestID = nRequestID;
	m_uidRequestPlayer = uidProposer;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MTextArea* pTextEdit = (MTextArea*)pResource->FindWidget("ProposalAgreementConfirm_Textarea");
	if (pTextEdit)
	{
		// 여기서 각각의 상황에 맞는 대사를 넣어줘야한다.
		char szTemp[256] = "";

		char szMembers[256] = " (";

		for (int i = 0; i < nMemberCount; i++)
		{
			strcat(szMembers, szMemberNames[i]);
			if (i != nMemberCount-1) strcat(szMembers, ", ");
		}
		strcat(szMembers, ")");

		switch (nProposalMode)
		{
		case MPROPOSAL_LADDER_INVITE:
			{
				ZTransMsg(szTemp, MSG_LADDER_REPLIER_AGREEMENT_LABEL, 1, szMemberNames[0]);
				strcat(szTemp, szMembers);
			}
			break;
		case MPROPOSAL_CLAN_INVITE:
			{

				ZTransMsg(szTemp, MSG_CLANBATTLE_REPLIER_AGREEMENT_LABEL, 1, szMemberNames[0]);
				strcat(szTemp, szMembers);
			}
			break;
		case MPROPOSAL_CLAN_WARMUP_INVITE:
			{
				ZTransMsg(szTemp, MSG_CLANBATTLE_REPLIER_AGREEMENT_LABEL, 1, szMemberNames[0]);
				strcat(szTemp, szMembers);
			}
		};

		
		pTextEdit->SetText(szTemp);
	}

	MWidget* pWidget = pResource->FindWidget("ProposalAgreementConfirm");
	if(pWidget!=NULL)
	{
		static ZCOUNTDOWN countDown = {PROPOSAL_AGREEMENT_TIMEOUT_SEC,
										"ProposalAgreementConfirm_Remain",
										"ProposalAgreementConfirm",
										OnAskReplierAgreement_OnExpire};
		countDown.nSeconds=PROPOSAL_AGREEMENT_TIMEOUT_SEC;	// static 이므로 재설정
		ZApplication::GetTimer()->SetTimerEvent(0, &OnTimer_CountDown, &countDown, true);

		pWidget->Show(true, true);
	}
}

void ZGameClient::OnReplyAgreement(const MUID& uidProposer, 
		                const MUID& uidChar, 
						const char* szReplierName, 
						const MMatchProposalMode nProposalMode,
					    const int nRequestID, 
						const bool bAgreement)
{
	if (!IsWaitingRepliersAgreement())
	{
		m_AgreementBuilder.CancelProposal();
		return;
	}

	ZNetAgreementBuilder::_BuildReplyResult nResult;
	nResult = m_AgreementBuilder.BuildReply(szReplierName, nProposalMode, nRequestID, bAgreement);

	
	if (nResult == ZNetAgreementBuilder::BRR_ALL_AGREED)	// 모두 동의했을경우
	{
		switch (nProposalMode)
		{
		case MPROPOSAL_LADDER_INVITE:
			{
				char szMember[MAX_LADDER_TEAM_MEMBER][MATCHOBJECT_NAME_LENGTH];
				char* ppMember[MAX_LADDER_TEAM_MEMBER];

				for(int i=0; i<MAX_LADDER_TEAM_MEMBER; i++) {
					ppMember[i] = szMember[i];
				}
				int nCount = m_AgreementBuilder.GetReplierNames(&ppMember[1], MAX_LADDER_TEAM_MEMBER-1);

				// 자신도 넣는다.
				strcpy(szMember[0], ZGetMyInfo()->GetCharName());
				nCount++;

				// 여기서 실제로 요청
				ZPostLadderRequestChallenge(ppMember, nCount, 0, 0, 0, false);
			}
			break;
		case MPROPOSAL_CLAN_INVITE:
			{
				char szMember[MAX_CLANBATTLE_TEAM_MEMBER][MATCHOBJECT_NAME_LENGTH];
				char* ppMember[MAX_CLANBATTLE_TEAM_MEMBER];

				for(int i=0; i<MAX_CLANBATTLE_TEAM_MEMBER; i++) {
					ppMember[i] = szMember[i];
				}
				int nCount = m_AgreementBuilder.GetReplierNames(&ppMember[1], MAX_CLANBATTLE_TEAM_MEMBER-1);

				// 자신도 넣는다.
				strcpy(szMember[0], ZGetMyInfo()->GetCharName());
				nCount++;

				// Balance 옵션
				//int nBalancedMatching = 0;
				int nDuel = 0;
				ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
				MButton* pButton = (MButton*)pResource->FindWidget("DuelCheckBox");
				if ((pButton) && (pButton->GetCheck()))
				{
					nDuel = 1;
				}

				int nAntiLead = 0; // anti-lead by default
				/*MButton* pAntiLead = (MButton*)pResource->FindWidget("AntiLeadCheckBox");
				if ((pAntiLead) && (pAntiLead->GetCheck()))
				{
					nAntiLead = 1;
				}*/

				int nDodge = 0;
				MButton* pDodge = (MButton*)pResource->FindWidget("DodgeCheckBox");
				if ((pDodge) && (pDodge->GetCheck()))
				{
					nDodge = 1;
				}

				MButton* pButton2 = (MButton*)pResource->FindWidget("AntiLeadCheckBox");
				
				ZPostLadderRequestChallenge(ppMember, nCount, nDuel, nAntiLead, nDodge, false);
			}
			break;
		case MPROPOSAL_CLAN_WARMUP_INVITE:
			{
				char szMember[MAX_CLANBATTLE_TEAM_MEMBER][MATCHOBJECT_NAME_LENGTH];
				char* ppMember[MAX_CLANBATTLE_TEAM_MEMBER];

				for(int i=0; i<MAX_CLANBATTLE_TEAM_MEMBER; i++) {
					ppMember[i] = szMember[i];
				}
				int nCount = m_AgreementBuilder.GetReplierNames(&ppMember[1], MAX_CLANBATTLE_TEAM_MEMBER-1);

				// 자신도 넣는다.
				strcpy(szMember[0], ZGetMyInfo()->GetCharName());
				nCount++;
				
				ZPostLadderRequestChallenge(ppMember, nCount, 1, 1, 1, true);
			}
		};


		ShowProposalWaitFrame(false);
		return;
	}
	else if (nResult == ZNetAgreementBuilder::BRR_DISAGREED)	// 거절했을 경우
	{
		ShowProposalWaitFrame(false);

		char temp[256] = "";
		char szRejecter[64] = "";

		// 거절한 사람 반환, 거절한 사람이 없을 경우는 그냥 마지막에 대답한 사람으로 정한다.
		if (!m_AgreementBuilder.GetRejecter(szRejecter)) strcpy(szRejecter, szReplierName);
		mlog("FUCKING REJECTED OMFGGGGGGGGGGGGGGGGGG");
		GunzState nGameState = ZApplication::GetGameInterface()->GetState();
		if (nGameState == GUNZ_LOBBY) 
		{
			ZTransMsg(temp, MSG_LADDER_REPLIER_AGREEMENT_REJECT, 1, szRejecter);
			ZApplication::GetGameInterface()->ShowMessage(temp, NULL, MSG_LADDER_REPLIER_AGREEMENT_REJECT);
		}
		
		
	}
}


void ZGameClient::RequestProposal(const MMatchProposalMode nProposalMode, char** ppReplierCharNames, const int nReplierCount)
{
	// 각각에 맞게 validate 해야한다.
	switch (nProposalMode)
	{
		case MPROPOSAL_LADDER_INVITE:
			{

			}
			break;
		case MPROPOSAL_CLAN_INVITE:
			{

			}
			break;
		case MPROPOSAL_CLAN_WARMUP_INVITE:
			{

			}
			break;
	};


	for (int i = 0; i < nReplierCount; i++)
	{
		int len = (int)strlen(ppReplierCharNames[i]);
		if ((len < 2) || (len >= MATCHOBJECT_NAME_LENGTH)) return;
	}

	m_nRequestID++;
	ZPostRequestProposal(GetPlayerUID(), int(nProposalMode), m_nRequestID, ppReplierCharNames, nReplierCount);
	m_AgreementBuilder.Proposal(nProposalMode, m_nRequestID, ppReplierCharNames, nReplierCount);
}

void ZGameClient::ReplyAgreement(const MUID& uidProposer, const MMatchProposalMode nProposalMode, bool bAgreement)
{
	char szCharName[MATCHOBJECT_NAME_LENGTH];
	sprintf(szCharName, ZGetMyInfo()->GetCharName());
	ZPostReplyAgreement(uidProposer, GetPlayerUID(), szCharName, nProposalMode, m_nRequestID, bAgreement);
}

void ZGameClient::ReplyAgreement(bool bAgreement)
{
	ReplyAgreement(m_uidRequestPlayer, m_nProposalMode, bAgreement);

}

void ZGameClient::OnLadderPrepare(const MUID& uidStage, const int nTeam)
{
	m_uidStage = uidStage;
}

void ZGameClient::OnPreGame(bool status)
{
	inPreGame = status;
}

void ZGameClient::OnLadderLaunch(const MUID& uidStage, const char* pszMapName)
{
	ZGetGameInterface()->OnArrangedTeamGameUI(false, false);

	m_bLadderGame = true;
 
	m_uidStage = uidStage;
	strcpy(m_szStageName, "UnNamedStage");

	SetAllowTunneling(false);

	m_MatchStageSetting.SetMapName(const_cast<char*>(pszMapName));


	// 암호화 키 설정  - 클랜전은 OnStageJoin대신 여기서 암호화키를 설정한다.
	unsigned int nStageNameChecksum = m_szStageName[0] + m_szStageName[1] + m_szStageName[2] + m_szStageName[3];
	InitPeerCrypt(uidStage, nStageNameChecksum);

	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME)
	{
		ZChangeGameState(GUNZ_GAME);		// thread safely
	}
}


void ZGameClient::OnLadderResponseChallenge(const int nResult)
{
	if (nResult != MOK)
	{
		ZApplication::GetGameInterface()->ShowErrorMessage( nResult );
	}
}