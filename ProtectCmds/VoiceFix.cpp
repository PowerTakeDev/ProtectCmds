#include "baseclient.h"
#include "netmessages.h"
#include "CDetour/detours.h"
#include "limits.h"
#include "gameclient.h"
#include "common.h"

CDetour *g_ProcessVoiceDataDetour = nullptr;

float g_fLastVoice[MAXPLAYERS + 1];
int g_iTickVoice[MAXPLAYERS + 1];
int g_iDetectVoiceExploit[MAXPLAYERS + 1];

#define VOICEFIX_MAX_VOICE_PRE_SECOND 9
#define VOICEFIX_MAX_DETECTIONS 3
#define VOICEFIX_MAX_VOICE_LENGTH 1600

DETOUR_DECL_MEMBER1(ProcessVoiceData, bool, CLC_VoiceData *, msg)
{
	CGameClient* pThis = reinterpret_cast<CGameClient*>(this);
	IClient* pClient = pThis;

	int nPlayerSlot = pClient->GetPlayerSlot();
	int nLength = msg->m_nLength;
	
	if (!pClient->IsConnected())
	{
#if DEBUG
		DevMsg("Client %i sent voice message, but him is not connected.\n", nPlayerSlot);
#endif
		return false;
	}

	if (nLength > VOICEFIX_MAX_VOICE_LENGTH)
	{
#if DEBUG
		DevMsg("Client %i sent voice message over " STR(VOICEFIX_MAX_VOICE_LENGTH) " bits (%i)\n", nPlayerSlot, nLength);
#endif
		pClient->Disconnect("Buffer overflow in net message");
		return false;
	}

#ifdef DEBUG
	DevMsg("Client %i sent voice message of length %i\n", nPlayerSlot, nLength);
#endif
	
	float fCurrentVoiceTime = g_pGlobals->curtime;

	if (g_fLastVoice[nPlayerSlot] + 0.1 > fCurrentVoiceTime)
	{
		g_iTickVoice[nPlayerSlot]++;
		if (g_iTickVoice[nPlayerSlot] >= VOICEFIX_MAX_VOICE_PRE_SECOND)
		{
			g_iDetectVoiceExploit[nPlayerSlot]++;

#if DEBUG
			DevMsg(
				"Client %i has exceeded limit of voice messages per second (%i/" STR(VOICEFIX_MAX_DETECTIONS) ")\n",
				nPlayerSlot,
				g_iDetectVoiceExploit[nPlayerSlot]
			);
#endif

			if (g_iDetectVoiceExploit[nPlayerSlot] > VOICEFIX_MAX_DETECTIONS)
			{
#if DEBUG
				DevMsg(
					"Client %i has exceeded " STR(VOICEFIX_MAX_DETECTIONS) " flood detections by voice messages\n",
					nPlayerSlot
				);
#endif
				pClient->Disconnect("Buffer overflow in net message");
			}

			g_iTickVoice[nPlayerSlot] = 1;

			return false;
		}
	}
	else
	{
		g_iTickVoice[nPlayerSlot] = 1;
		g_iDetectVoiceExploit[nPlayerSlot] = 0;
		g_fLastVoice[nPlayerSlot] = fCurrentVoiceTime;
	}

	return DETOUR_MEMBER_CALL(ProcessVoiceData)(msg);
}

namespace VoiceFix
{
	bool Load(char* error, int maxlen)
	{
		if (g_ProcessVoiceDataDetour == nullptr)
		{
			g_ProcessVoiceDataDetour = DETOUR_CREATE_MEMBER(ProcessVoiceData, "CGameClient::ProcessVoiceData");
			if (g_ProcessVoiceDataDetour == nullptr)
				V_snprintf(error, maxlen, "Failed to setup CGameClient::ProcessVoiceData detour");
			else
				g_ProcessVoiceDataDetour->EnableDetour();
		}
		return true;
	}

	void Unload()
	{
		if (g_ProcessVoiceDataDetour != nullptr)
		{
			g_ProcessVoiceDataDetour->Destroy();
			g_ProcessVoiceDataDetour = nullptr;
		}
	}
}