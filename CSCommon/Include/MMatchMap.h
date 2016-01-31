#ifndef _MMATCHMAP_H
#define _MMATCHMAP_H

#include "MMatchGlobal.h"


enum MMATCH_MAP
{
	MMATCH_MAP_MANSION = 0,
	MMATCH_MAP_PRISON = 1,
	MMATCH_MAP_STATION = 2,
	MMATCH_MAP_PRISON_II = 3,
	MMATCH_MAP_BATTLE_ARENA = 4,
	MMATCH_MAP_TOWN = 5,
	MMATCH_MAP_DUNGEON = 6,
	MMATCH_MAP_RUIN = 7,
	MMATCH_MAP_ISLAND = 8,
	MMATCH_MAP_GARDEN = 9,
	MMATCH_MAP_CASTLE = 10,
	MMATCH_MAP_FACTORY = 11,
	MMATCH_MAP_PORT = 12,
	MMATCH_MAP_LOST_SHRINE = 13,
	MMATCH_MAP_STAIRWAY = 14,
	MMATCH_MAP_SNOWTOWN = 15,
	MMATCH_MAP_HALL = 16,
	MMATCH_MAP_CATACOMB = 17,
	MMATCH_MAP_JAIL = 18,
	MMATCH_MAP_SHOWERROOM = 19,
	MMATCH_MAP_HIGH_HAVEN = 20,
	MMATCH_MAP_CITADEL = 21,
	MMATCH_MAP_RELAYMAP = 22,
	MMATCH_MAP_HALLOWEEN_TOWN = 23,
	MMATCH_MAP_WEAPON_SHOP = 24,
	MMATCH_MAP_Arena = 25,
#ifdef _RGGunz
	MMATCH_MAP_CUSTOM26 = 26,
	MMATCH_MAP_CUSTOM27 = 27,
	MMATCH_MAP_CUSTOM28 = 28,
	MMATCH_MAP_CUSTOM29 = 29,
	MMATCH_MAP_CUSTOM30 = 30,
	MMATCH_MAP_CUSTOM31 = 31,
	MMATCH_MAP_CUSTOM32 = 32,
	MMATCH_MAP_CUSTOM33 = 33,
	MMATCH_MAP_CUSTOM34 = 34,
	MMATCH_MAP_CUSTOM35 = 35,
	MMATCH_MAP_CUSTOM36 = 36,
	MMATCH_MAP_CUSTOM37 = 37,
	MMATCH_MAP_CUSTOM38 = 38,
	MMATCH_MAP_CUSTOM39 = 39,
	MMATCH_MAP_CUSTOM40 = 40,
	MMATCH_MAP_CUSTOM41 = 41,
	MMATCH_MAP_CUSTOM42 = 42,
	MMATCH_MAP_CUSTOM43 = 43,
	MMATCH_MAP_CUSTOM44 = 44,
	MMATCH_MAP_CUSTOM45 = 45,
	MMATCH_MAP_CUSTOM46 = 46,
	MMATCH_MAP_CUSTOM47 = 47,
	MMATCH_MAP_CUSTOM48 = 48,
	MMATCH_MAP_CUSTOM49 = 49,
	MMATCH_MAP_CUSTOM50 = 50,
	MMATCH_MAP_CUSTOM51 = 51,
	MMATCH_MAP_CUSTOM52 = 52,
	MMATCH_MAP_CUSTOM53 = 53,
	MMATCH_MAP_CUSTOM54 = 54,
	MMATCH_MAP_CUSTOM55 = 55,
	MMATCH_MAP_CUSTOM56 = 56,
	MMATCH_MAP_CUSTOM57 = 57,
	MMATCH_MAP_CUSTOM58 = 58,
	MMATCH_MAP_CUSTOM59 = 59,
	MMATCH_MAP_CUSTOM60 = 60,
	MMATCH_MAP_CUSTOM61 = 61,
	MMATCH_MAP_CUSTOM62 = 62,
#endif
	MMATCH_MAP_MAX
};
#define MMATCH_MAP_COUNT	MMATCH_MAP_MAX			// ÀüÃ¼ ¸Ê °¹¼ö

#define MMATCH_MAPNAME_RELAYMAP				"RelayMap"


class MMapDesc
{
private:
	const struct MapInfo
	{
		int			nMapID;							// map id
		char		szMapName[MAPNAME_LENGTH];		// ¸Ê ÀÌ¸§
		char		szMapImageName[MAPNAME_LENGTH];	// ¸Ê ÀÌ¹ÌÁö ÀÌ¸§
		char		szBannerName[MAPNAME_LENGTH];	// º£³Ê ÀÌ¸§
		float		fExpRatio;						// °æÇèÄ¡ ½Àµæ·ü
		int			nMaxPlayers;					// ÃÖ´ë ÀÎ¿ø
		bool		bOnlyDuelMap;					// µà¾ó¸Ê Àü¿ë
		bool		bIsCTFMap;
	};


	// data
	MapInfo	m_MapVectors[MMATCH_MAP_COUNT];
	MMapDesc();
public:
	~MMapDesc() { }
	
	static MMapDesc* GetInstance();

	bool Initialize(const char* szFileName);
	bool Initialize(MZFileSystem* pfs, const char* szFileName);
	bool MIsCorrectMap(const int nMapID);
	

 
	bool IsMapOnlyDuel( const int nMapID);
	bool IsCTFMap( const int nMapID);
	int GetMapID( const int nMapID);
	const char* GetMapName(const int nMapID);
	const char* GetMapImageName(const char* szMapName);
	const char* GetBannerName(const char* szMapName);
	float GetExpRatio( const int nMapID); 
	int GetMaxPlayers( const int nMapID);

	int GetMapCount(){ return MMATCH_MAP_COUNT; }
};


inline MMapDesc* MGetMapDescMgr() 
{ 
	return MMapDesc::GetInstance();
}



#endif