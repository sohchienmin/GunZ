#ifndef MMATCHCHANNEL_H
#define MMATCHCHANNEL_H

#include <map>
#include <list>
using namespace std;

#include "MMatchGlobal.h"
#include "MUID.h"
#include "MMatchChannel.h"
#include "MPageArray.h"
#include "MSmartRefresh.h"
#include "MMatchChannelRule.h"


#define DEFAULT_CHANNEL_MAXPLAYERS			200
#define DEFAULT_CHANNEL_MAXSTAGES			100
#define MAX_CHANNEL_MAXSTAGES				500
#define NUM_PLAYERLIST_NODE					6
#define CHANNEL_NO_LEVEL					(-1)
#define CHANNELNAME_STRINGRESID_LEN			64		// 실제로는 이보다 길수도 있지만 그런 경우가 생기지 않도록 ASSERT로 확인

class MMatchObject;
class MMatchStage;
class MCommand;


enum MCHANNEL_TYPE {
	MCHANNEL_TYPE_PRESET	= 0,		// 일반채널
	MCHANNEL_TYPE_USER		= 1,		// 사설채널
	MCHANNEL_TYPE_PRIVATE	= 2,		// 비밀채널 - 현재 안쓰임
	MCHANNEL_TYPE_CLAN		= 3,		// 클랜채널
	MCHANNEL_TYPE_QUEST		= 4,		// 퀘스트채널 2006-05-23추가 - by SungE
	MCHANNEL_TYPE_DUELTOURNAMENT = 5,	// 듀얼토너먼트채널 2009-09-21추가 - by 홍기주
	MCHANNEL_TYPE_MAX
};


// 채널 리스트 달라고 요청할때 보내는 구조체
struct MCHANNELLISTNODE {
	MUID			uidChannel;						// 채널 UID
	short			nNo;							// 채널번호
	unsigned char	nPlayers;						// 현재인원
	short			nMaxPlayers;					// 최대인원
	short			nLevelMin;						// 최소레벨
	short			nLevelMax;						// 최대레벨
	char			nChannelType;					// 채널타입
	char			szChannelName[CHANNELNAME_LEN];	// 채널이름
	char			szChannelNameStrResId[CHANNELNAME_STRINGRESID_LEN];	// 채널이름의 스트링리소스 ID (클라이언트에서 공식채널의 번역된 이름을 표기하도록 하기 위함)
	bool			bIsUseTicket;
	unsigned int	nTicketID;
	bool			bIsOwner;
	int				nOnlinePlayers;
};

// typedef map<string, MMatchObject*>			MObjectStrMap;
typedef map<int, MMatchStage*>				MChannelStageMap;
typedef MPageArray<MMatchObject*>			MChannelUserArray;


class MMatchChannel {
private:
	MUID			m_uidChannel;
	char			m_szChannelName[CHANNELNAME_LEN];
	char			m_szChannelNameStrResId[CHANNELNAME_STRINGRESID_LEN];
	MCHANNEL_TYPE	m_nChannelType;
	int				m_nMaxPlayers;
	int				m_nLevelMin;
	int				m_nLevelMax;
	int				m_nMaxStages;
	char			m_szRuleName[CHANNELRULE_LEN];
	MCHANNEL_RULE	m_nRuleType;
	//bool			m_bNewbieChannel;		// 뉴비채널은 정말 초보만 들어갈 수 있다.
	
	bool			m_bIsUseTicket;			// 입장권 사용 유무.
	bool			m_bIsTicketChannel;		// 입장권을 사용하는 채널인지.
    DWORD			m_dwTicketItemID;		// 입장권 ItemID.
	
	MUIDRefCache	m_ObjUIDCaches;			// 채널전체 플레이어들
	MUIDRefCache	m_ObjUIDLobbyCaches;	// 로비에 있는 플레이어들
//	MObjectStrMap	m_ObjStrCaches;

	MMatchStage*	m_pStages[MAX_CHANNEL_MAXSTAGES];
	list<int>		m_UnusedStageIndexList;

	MChannelUserArray			m_UserArray;
	MSmartRefresh				m_SmartRefresh;

	unsigned long	m_nChecksum;	// 목록및 정보 갱신용
	unsigned long	m_nLastChecksumTick;

	unsigned long	m_nLastTick;
	unsigned long	m_nEmptyPeriod;

	void JoinLobby(const MUID& uid, const MMatchObject* pObj);
	void LeaveLobby(const MUID& uid);
protected:
	inline bool IsChecksumUpdateTime(unsigned long nTick);
	void UpdateChecksum(unsigned long nTick);
	unsigned long GetEmptyPeriod()	{ return m_nEmptyPeriod; }

public:
	bool CheckTick(unsigned long nClock);
	void Tick(unsigned long nClock);

	unsigned long GetChecksum()		{ return m_nChecksum; }
	bool CheckLifePeriod();

public:
	bool Create(const MUID& uid, const char* pszName, const char* pszRuleName, 
				MCHANNEL_TYPE nType=MCHANNEL_TYPE_PRESET, int nMaxPlayers=DEFAULT_CHANNEL_MAXPLAYERS, 
				int nLevelMin=CHANNEL_NO_LEVEL, int nLevelMax=CHANNEL_NO_LEVEL,
				const bool bIsTicketChannel = false, const DWORD dwTicketItemID = 0, const bool bIsUseTicket = false,
				const char* pszNameStrResId = NULL);
	void Destroy();

	const char* GetName()			{ return m_szChannelName; }
	const char* GetNameStringResId(){ return m_szChannelNameStrResId; }
	const char* GetRuleName()		{ return m_szRuleName; }
	MUID GetUID()					{ return m_uidChannel; }
	MCHANNEL_TYPE GetChannelType()	{ return m_nChannelType; }
	MCHANNEL_RULE GetRuleType()		{ return m_nRuleType; }

	int GetMaxPlayers()				{ return m_nMaxPlayers; }
	int GetLevelMin()				{ return m_nLevelMin; }
	int GetLevelMax()				{ return m_nLevelMax; }
	int	GetMaxStages()				{ return m_nMaxStages; }
	size_t GetObjCount()			{ return m_ObjUIDCaches.size(); }
	int GetPlayers();
	MUIDRefCache::iterator GetObjBegin()		{ return m_ObjUIDCaches.begin(); }
	MUIDRefCache::iterator GetObjEnd()			{ return m_ObjUIDCaches.end(); }
	MUIDRefCache::iterator GetLobbyObjBegin()	{ return m_ObjUIDLobbyCaches.begin(); }
	MUIDRefCache::iterator GetLobbyObjEnd()		{ return m_ObjUIDLobbyCaches.end(); }


	void AddObject(const MUID& uid, MMatchObject* pObj);
	void RemoveObject(const MUID& uid);
public:
	bool AddStage(MMatchStage* pStage);
	void RemoveStage(MMatchStage* pStage);
	bool IsEmptyStage(int nIndex);
	MMatchStage* GetStage(int nIndex);
	int GetPrevStageCount(int nStageIndex);	// nStageIndex를 포함하지 않는 nStageIndex이전의 만들어진 방 개수 
	int GetNextStageCount(int nStageIndex);	// nStageIndex를 포함하지 않는 nStageIndex이후의 만들어진 방 개수 
	
	//bool IsNewbieChannel()			{ return m_bNewbieChannel; }

	const bool	IsUseTicket()		{ return m_bIsUseTicket; }
	const bool	IsTicketChannel()	{ return m_bIsTicketChannel; }
	const DWORD GetTicketItemID()	{ return m_dwTicketItemID; }

public:
	MChannelUserArray* GetUserArray()	{ return &m_UserArray; }

public:
	void SyncPlayerList(MMatchObject* pObj, int nPage);
};


struct MMatchClanChannelTicketInfo
{
	MMatchClanChannelTicketInfo() 
	{
		m_bIsTicketChannel	= false;
		m_dwTicketItemID	= 0;
	}

	bool	m_bIsTicketChannel;
	DWORD	m_dwTicketItemID;
};


class MMatchChannelMap : public map<MUID, MMatchChannel*> {
private:
	MUID						m_uidGenerate;
	unsigned long				m_nChecksum;
	map<MUID, MMatchChannel*>	m_TypesChannelMap[MCHANNEL_TYPE_MAX];
	MMatchClanChannelTicketInfo m_ClanChannelTicketInfo;

	void Insert(const MUID& uid, MMatchChannel* pChannel)	{	insert(value_type(uid, pChannel));	}
	MUID UseUID()				{	m_uidGenerate.Increase();	return m_uidGenerate;	}
	
//	void UpdateChecksum(unsigned long nClock);

public:
	MMatchChannelMap()			{	m_uidGenerate = MUID(0,0);	m_nChecksum=0; }
	virtual ~MMatchChannelMap()	{	}
	void Destroy();
	
	MMatchChannel* Find(const MUID& uidChannel);
	MMatchChannel* Find(const MCHANNEL_TYPE nChannelType, const char* pszChannelName);

	bool Add(const char* pszChannelName, const char* pszRuleName, MUID* pAllocUID, MCHANNEL_TYPE nType=MCHANNEL_TYPE_PRESET, int nMaxPlayers=DEFAULT_CHANNEL_MAXPLAYERS, int nLevelMin=-1, int nLevelMax=-1,
		const bool bIsTicketChannel = false, const DWORD dwTicketItemID = 0, const bool bIsUseTicket = false, const char* pszChannelNameStrResId = NULL);
	bool Remove(const MUID& uidChannel, MMatchChannelMap::iterator* pNextItor);
	void Update(unsigned long nClock);

	unsigned long GetChannelListChecksum() { return m_nChecksum; }
	int GetChannelCount(MCHANNEL_TYPE nChannelType);
	
	map<MUID, MMatchChannel*>::iterator GetTypesChannelMapBegin(MCHANNEL_TYPE nType);
	map<MUID, MMatchChannel*>::iterator GetTypesChannelMapEnd(MCHANNEL_TYPE nType);

	const MMatchClanChannelTicketInfo& GetClanChannelTicketInfo() const { return m_ClanChannelTicketInfo; }

	void SetIsTicketClanChannel( const bool bIsTicketClanChannel )	{ m_ClanChannelTicketInfo.m_bIsTicketChannel = bIsTicketClanChannel; }
	void SetClanChannelTicketItemID( const DWORD dwTicketItemID )	{ m_ClanChannelTicketInfo.m_dwTicketItemID = dwTicketItemID; }
};


#endif