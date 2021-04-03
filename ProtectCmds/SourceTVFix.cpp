#include "CDetour/detours.h"
#include "netmessages.h"
#include "baseclient.h"

CDetour *g_ProcessClientInfoDetour = nullptr;

DETOUR_DECL_MEMBER1(ProcessClientInfo, bool, CLC_ClientInfo *, msg)
{
	if (msg->m_bIsHLTV)
	{
#ifdef DEBUG
		CBaseClient *pClient = reinterpret_cast<CBaseClient*>(this);
		INetChannel* pNetChannel = pClient->m_pNetChannel;

		if (pNetChannel != nullptr)
		{
			DevMsg(SMEXT_TAG " %s attempted to kick SourceTV (prevented)\n", pNetChannel->GetAddress());
		}
#endif

		// TODO: ban ip
		
		msg->m_bIsHLTV = false;
	}
	
	return DETOUR_MEMBER_CALL(ProcessClientInfo)(msg);
}

namespace SourceTVFix
{
	bool Load(char* error, int maxlen)
	{
		if (g_ProcessClientInfoDetour == nullptr) {
			g_ProcessClientInfoDetour = DETOUR_CREATE_MEMBER(ProcessClientInfo, "CGameClient::ProcessClientInfo");
			if (g_ProcessClientInfoDetour == nullptr)
			{
				V_snprintf(error, maxlen, "Failed to setup CGameClient::ProcessClientInfo detour");
				return false;
			}
			g_ProcessClientInfoDetour->EnableDetour();
		}
		return true;
	}

	void Unload()
	{
		if (g_ProcessClientInfoDetour != nullptr)
		{
			g_ProcessClientInfoDetour->Destroy();
			g_ProcessClientInfoDetour = nullptr;
		}
	}
}