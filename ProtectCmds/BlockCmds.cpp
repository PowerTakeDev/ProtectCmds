#include "CDetour/detours.h"
#include "limits.h"
#include "baseclient.h"
#include "gameclient.h"
#include "strtools.cpp"
#include "util.h"

//DETOUR_DECL_CLASS_MEMBER(CBaseClient, ExecuteStringCommand_);

CDetour* g_ExecuteStringCommandDetour = nullptr;

int g_iMaxUserCmds = 30;
float g_fLastUserCmds[MAXPLAYERS+1];
int g_iUserCmds[MAXPLAYERS+1];

#define BLOCKCMDS_MAX_CMD_LEN 1024

DETOUR_DECL_MEMBER1(ExecuteStringCommand, bool, char *, pCommand)
{
	if (strlen(pCommand) > BLOCKCMDS_MAX_CMD_LEN)
		return false;
	
	CBaseClient* pThis = reinterpret_cast<CBaseClient*>(this);
	int iClient = pThis->m_nClientSlot + 1;
	
	if (!pThis->IsConnected())
		return false;
	
	float fCurrentUserCmdsTime = g_pGlobals->curtime;
	if (g_fLastUserCmds[iClient]+1.0 > fCurrentUserCmdsTime)
	{
		g_iUserCmds[iClient]++;
		if (g_iUserCmds[iClient] >= g_iMaxUserCmds)
		{
			pThis->Disconnect("Buffer overflow in net message");
			return false;
		}
	}
	else
	{
		g_iUserCmds[iClient] = 1;
		g_fLastUserCmds[iClient] = fCurrentUserCmdsTime;
	}

	if (!strncmp(pCommand, "sm ", 3) || !strncmp(pCommand, "meta ", 5))
	{
		return false;
	}
	
	return DETOUR_MEMBER_CALL(ExecuteStringCommand)(pCommand);
}

namespace BlockCmds
{
	bool Load(char* error, int maxlen)
	{
		if (g_ExecuteStringCommandDetour == nullptr) {
			g_ExecuteStringCommandDetour = DETOUR_CREATE_MEMBER(ExecuteStringCommand, "CGameClient::ExecuteStringCommand");
			if (g_ExecuteStringCommandDetour) g_ExecuteStringCommandDetour->EnableDetour();
			else g_pSM->LogError(myself, "Failed to setup CGameClient::ExecuteStringCommand detour");
		}
		//DETOUR_INIT_CLASS_MEMBER(CBaseClient, ExecuteStringCommand_, "CBaseClient::ExecuteStringCommand");
		return true;
	}

	void Unload()
	{
		if (g_ExecuteStringCommandDetour != nullptr) {
			g_ExecuteStringCommandDetour->Destroy();
			g_ExecuteStringCommandDetour = nullptr;
		}
		//DETOUR_DESTROY_CLASS_MEMBER(CBaseClient, ExecuteStringCommand_);
	}
}