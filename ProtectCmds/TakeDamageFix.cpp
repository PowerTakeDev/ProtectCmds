#include "CDetour/detours.h"
#include <dbg.h>

CDetour *g_TakeDamageDetour = nullptr;

static void **g_pGameRules = nullptr;

class CTakeDamageInfo {};

DETOUR_DECL_MEMBER1(TakeDamage, void *, CTakeDamageInfo&, inputInfo) {
	if ( !(*g_pGameRules) )
	{
#ifdef DEBUG
		DevMsg(SMEXT_TAG " prevented server crash with CBaseEntity::TakeDamage NULL g_pGameRules\n");
#endif
		return nullptr;
	}
	
	return DETOUR_MEMBER_CALL(TakeDamage)(inputInfo);
}

namespace TakeDamageFix
{
	bool Load(char* error, int maxlen)
	{
		char* addr;
		if (!g_pGameConf->GetMemSig("g_pGameRules", (void**)&addr) || addr == nullptr)
		{
			V_snprintf(error, maxlen, "Failed to get structure g_pGameRules");
			return false;
		}
		g_pGameRules = reinterpret_cast<void**>(addr);

		if (g_TakeDamageDetour == nullptr)
		{
			g_TakeDamageDetour = DETOUR_CREATE_MEMBER(TakeDamage, "CBaseEntity::TakeDamage");
			if (g_TakeDamageDetour == nullptr)
			{
				V_snprintf(error, maxlen, "Failed to setup CBaseEntity::TakeDamage detour");
				return false;
			}

			g_TakeDamageDetour->EnableDetour();
		}
		return true;
	}

	void Unload()
	{
		if (g_TakeDamageDetour != nullptr) {
			g_TakeDamageDetour->Destroy();
			g_TakeDamageDetour = nullptr;
		}
	}
}