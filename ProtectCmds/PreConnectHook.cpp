#include "CDetour/detours.h"
#include "baseserver.h"
#include <netadr.h>
#include "protocol.h"

CDetour *g_ConnectClientDetour = nullptr;
IForward *g_pConnectForward = nullptr;

#define RejectConnection(adr, reason) (*(void (__cdecl**)(void*, netadr_t&, const char *))(*(uint*)this + 188))(this, adr, reason);

DETOUR_DECL_MEMBER8(ConnectClient, void *, netadr_t &, addr, int, protocol, int, challenge, int, nAuthProtocol, char const *, name, char const *, password, char *, hashedCDkey, int, cdKeyLen)
{
	if(nAuthProtocol == PROTOCOL_STEAM)
	{
		char szRejectReason[255] = "Disconnect by user";
		g_pConnectForward->PushString(addr.ToString(true));
		g_pConnectForward->PushString(name);
		g_pConnectForward->PushStringEx(szRejectReason, sizeof(szRejectReason), SM_PARAM_STRING_UTF8 | SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);

		cell_t result = Pl_Continue;
		g_pConnectForward->Execute(&result);

		if(result >= Pl_Handled)
		{
			RejectConnection(addr, szRejectReason);
			return nullptr;
		}
	}

	return DETOUR_MEMBER_CALL(ConnectClient)(addr, protocol, challenge, nAuthProtocol, name, password, hashedCDkey, cdKeyLen);
}

namespace PreConnectHook
{
	bool Load(char* error, int maxlen)
	{
		if (g_ConnectClientDetour == nullptr)
		{
			g_ConnectClientDetour = DETOUR_CREATE_MEMBER(ConnectClient, "CBaseServer::ConnectClient");
			if (g_ConnectClientDetour == nullptr)
			{
				V_snprintf(error, maxlen, "Failed to setup CBaseServer::ConnectClient detour");
				return false;
			}
			g_ConnectClientDetour->EnableDetour();
		}

		g_pConnectForward = forwards->CreateForward("OnClientPreConnect", ET_Event, 4, nullptr, Param_String, Param_String, Param_String, Param_String); // fix it later :)
		
		return true;
	}

	void Unload()
	{
		forwards->ReleaseForward(g_pConnectForward);

		if (g_ConnectClientDetour != nullptr) {
			g_ConnectClientDetour->Destroy();
			g_ConnectClientDetour = nullptr;
		}
	}
}
