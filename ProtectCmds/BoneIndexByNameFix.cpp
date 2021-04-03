#include "CDetour/detours.h"
#include <studio.h>

CDetour *g_BoneIndexByNameDetour = nullptr;

DETOUR_DECL_MEMBER2(BoneIndexByName, int, CStudioHdr *, pStudioHdr, const char *, szName)
{
	register int EAX asm("eax");
	register int ECX asm("ecx");

	if (!EAX || !ECX || !pStudioHdr)
	{
#ifdef DEBUG
		DevMsg(SMEXT_TAG " prevented server crash with CBaseAnimating::Studio_BoneIndexByName\n");
#endif
		return -1;
	}
	
	return DETOUR_MEMBER_CALL(BoneIndexByName)(pStudioHdr, szName);
}

namespace BoneIndexByNameFix
{
	bool Load(char* error, int maxlen)
	{
		if (g_BoneIndexByNameDetour == nullptr) {
			g_BoneIndexByNameDetour = DETOUR_CREATE_MEMBER(BoneIndexByName, "CBaseAnimating::Studio_BoneIndexByName");
			if (g_BoneIndexByNameDetour == nullptr)
			{
				V_snprintf(error, maxlen, "Failed to setup CBaseAnimating::BoneIndexByName detour");
				return false;
			}
			g_BoneIndexByNameDetour->EnableDetour();
		}
		return true;
	}

	void Unload() {
		if (g_BoneIndexByNameDetour != nullptr) {
			g_BoneIndexByNameDetour->Destroy();
			g_BoneIndexByNameDetour = nullptr;
		}
	}
}