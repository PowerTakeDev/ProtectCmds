#undef DLL_EXPORT
#include "extension.h"
#include "CDetour/detours.h"

#define GAMECONFIG_FILE "protectcmds"

#include "VoiceFix.h"
#include "GameUIFix.h"
#include "PreConnectHook.h"
#include "SourceTVFix.h"
#include "BoneIndexByNameFix.h"
#include "TakeDamageFix.h"
#include "BlockCmds.h"
#include "NetChanFix.h"
#include "FilterPacket.h"
#include "HashSteamID.h"

CProtectCmds g_ProtectCMDS;
SMEXT_LINK(&g_ProtectCMDS);

IGameConfig* g_pGameConf = nullptr;

bool CProtectCmds::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	if (!gameconfs->LoadGameConfigFile(GAMECONFIG_FILE, &g_pGameConf, error, maxlength))
	{
		V_snprintf(error, maxlength, "Could not read " GAMECONFIG_FILE ".txt: %s", error);
		return false;
	}
	
	CDetourManager::Init(g_pSM->GetScriptingEngine(), g_pGameConf);

	LoadFix(FilterPacket);
	LoadFix(NetChanFix);
	LoadFix(VoiceFix);
	LoadFix(HashSteamID);
	LoadFix(BlockCmds);
	LoadFix(GameUIFix);
	LoadFix(BoneIndexByNameFix);
	LoadFix(SourceTVFix);
	LoadFix(TakeDamageFix);
	LoadFix(PreConnectHook);

	return true;
}

void CProtectCmds::SDK_OnUnload()
{	
	UnloadFix(FilterPacket);
	UnloadFix(NetChanFix);
	UnloadFix(VoiceFix);
	UnloadFix(HashSteamID);
	UnloadFix(BlockCmds);
	UnloadFix(GameUIFix);
	UnloadFix(BoneIndexByNameFix);
	UnloadFix(SourceTVFix);
	UnloadFix(TakeDamageFix);
	UnloadFix(PreConnectHook);

	gameconfs->CloseGameConfigFile(g_pGameConf);	
}
