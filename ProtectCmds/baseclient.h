#ifndef BASECLIENT_H
#define BASECLIENT_H

#include <igameevents.h>
#include <iclient.h>
#include <checksum_crc.h>
#include <steam/steamclientpublic.h>
#include "userid.h"
#include "protocol.h"
#include <const.h>
#include <bitvec.h>
#include <basetypes.h>
#include <igameevents.h>
#include <inetmsghandler.h>
#include "netmessages.h"

class CFrameSnapshot;
class CBaseServer;
class KeyValues;

class CBaseClient : public IGameEventListener2, public IClient, public IClientMessageHandler
{
public:
	typedef struct CustomFile_s
	{
		CRC32_t m_uCRC;
		unsigned int m_uReqID;
	} CustomFile_t;

public:
	void SetSteamID(const CSteamID& inputSteamID);
	bool ExecuteStringCommand_(const char* pCommand);

public:
	int m_nClientSlot;
	int m_nEntityIndex;
	int m_nUserID;
	char m_szName[MAX_PLAYER_NAME_LENGTH];
	char m_szGUID[SIGNED_GUID_LEN + 1];
	USERID_t m_NetworkID;
	CSteamID* m_pSteamID;
	uint32 m_nFriendsID;
	char m_szFriendsName[MAX_PLAYER_NAME_LENGTH];
	KeyValues* m_pConVars;
	bool m_bConVarsChanged;
	bool m_bSendServerInfo;
	CBaseServer* m_pServer;
	bool m_bIsHLTV;
	CRC32_t m_uSendtableCRC;
	CustomFile_t m_nCustomFiles[MAX_CUSTOM_FILES];
	int m_nFilesDownloaded;
	INetChannel* m_pNetChannel;
	int m_nSignonState;
	int m_nDeltaTick;
	int m_nSignonTick;
	CFrameSnapshot* m_pLastSnapshot;
	CFrameSnapshot* m_pBaseline;
	int m_nBaselineUpdateTick;
	CBitVec<MAX_EDICTS>	m_BaselinesSent;
	int m_nBaselineUsed;
	int m_nForceWaitForTick;
	bool m_bFakePlayer;
	bool m_bReceivedPacket;
	bool m_bFullyAuthenticated;
	bool a124_4;
	double m_fNextMessageTime;
	float m_fSnapshotInterval;
};

#endif // BASECLIENT_H