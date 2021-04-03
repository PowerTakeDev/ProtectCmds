#include "CDetour/detours.h"

CDetour *g_GameUIHookDetour = nullptr;

DETOUR_DECL_MEMBER1(GameUI, void *, CBaseEntity*, pActivator) {
	CBaseHandle *m_player_hndl = (CBaseHandle*)( (uintptr_t)this + 1316);

	if(!m_player_hndl->IsValid())
	{
#ifdef DEBUG
		DevMsg(SMEXT_TAG " prevented server crash with CGameUI::Deactivate\n");
#endif
		return nullptr;
	}
	
	return DETOUR_MEMBER_CALL(GameUI)(pActivator);
}

namespace GameUIFix
{
	bool Load(char* error, int maxlen)
	{
		if (g_GameUIHookDetour == nullptr) {
			g_GameUIHookDetour = DETOUR_CREATE_MEMBER(GameUI, "CGameUI::Deactivate");
			if (g_GameUIHookDetour == nullptr)
			{
				V_snprintf(error, maxlen, "Failed to setup GameUI::Deactivate detour");
				return false;
			}
			g_GameUIHookDetour->EnableDetour();
		}
		return true;
	}

	void Unload()
	{
		if (g_GameUIHookDetour != nullptr)
		{
			g_GameUIHookDetour->Destroy();
			g_GameUIHookDetour = nullptr;
		}
	}
}