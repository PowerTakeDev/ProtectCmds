#include "NetChanFix.h"
#include <inetmessage.h>
#include <CDetour/detours.h>
#include <strtools.cpp>
#include <inetchannel.h>
#include <inetmsghandler.h>
#include <bitbuf.h>
#include <netadr.h>
#include "protocol.h"
#include "net.h"

DETOUR_DECL_CLASS_MEMBER(CNetChan, ProcessMessages);

bool IsSafeFileToDownload( const char *pszFilename )
{
	if (V_strlen(pszFilename) != 22) // "downloads/ffffffff.dat"
	{
		return false;
	}

	char* pszExt = V_strrchr(pszFilename, '.');
	if (pszExt == nullptr ||
		V_strlen(pszExt) != 4 ||
		V_strcmp(pszExt, ".dat"))
	{
		return false;
	}

	if (strncmp(pszFilename, "downloads/", 10))
	{
		return false;
	}

	return true;
}

bool CNetChan::IsOverflowed() const
{
	return m_StreamReliable.IsOverflowed();
}

void CNetChan::UpdateMessageStats(int msggroup, int bits)
{
	netflow_t* pFlow = &m_DataFlow[FLOW_INCOMING];
	netframe_t* pFrame = pFlow->m_pCurrentFrame;

	m_MsgStats[msggroup] += bits;

	if (pFrame != nullptr)
	{
		pFrame->m_MsgGroups[msggroup] += bits;
	}
}

INetMessage* CNetChan::FindMessage(int type)
{
	int numtypes = m_NetMessages.Count();

	for (int i = 0; i < numtypes; i++)
	{
		if (m_NetMessages[i]->GetType() == type)
			return m_NetMessages[i];
	}

	return NULL;
}

bool CNetChan::ProcessControlMessage(int cmd, bf_read &buf)
{
	if (cmd == net_NOP)
	{
		return true;
	}

	if (cmd == net_Disconnect)
	{
		char szReason[256];
		buf.ReadString(szReason, sizeof(szReason));

		m_pMessageHandler->ConnectionClosing("Disconnect by user.");
		return false;
	}

	if (cmd == net_File)
	{
		char szFilename[256];

		unsigned int uTransferID = buf.ReadUBitLong(32);

		buf.ReadString(szFilename, sizeof(szFilename));

#ifdef DEBUG
		DevMsg("NET_File: \"%s\" (%u)\n", szFilename, uTransferID);
#endif

		if (!IsSafeFileToDownload(szFilename))
		{
#ifdef DEBUG
			DevMsg(SMEXT_TAG " Attempt to download file from unsafe path \"%s\". (%s)\n", szFilename, m_RemoteAddress.ToString());
#endif
			m_pMessageHandler->ConnectionClosing("Buffer overflow in net message");
			// TODO: ban ip
			return false;
		}

		if (buf.ReadOneBit() != 0)
		{
			m_pMessageHandler->FileRequested(szFilename, uTransferID);
		}
		else
		{
			m_pMessageHandler->FileDenied(szFilename, uTransferID);
		}
		return true;
	}

#ifdef DEBUG
	DevMsg(SMEXT_TAG " Received bad control cmd %i from %s.\n", cmd, m_RemoteAddress.ToString());
#endif
	return false;
}

bool CNetChan::ProcessMessages(bf_read& buf)
{
	while (true)
	{
		if (buf.IsOverflowed())
		{
#ifdef DEBUG
			DevMsg("Buffer overflow in net message from %s.\n", m_RemoteAddress.ToString());
#endif
			m_pMessageHandler->ConnectionCrashed("Buffer overflow in net message");
			return false;
		}

		if (buf.GetNumBitsLeft() < NETMSG_TYPE_BITS)
		{
			break;
		}

		unsigned char cmd = buf.ReadUBitLong(NETMSG_TYPE_BITS);
		if (cmd <= net_File)
		{
			if (!ProcessControlMessage(cmd, buf))
			{
				return false;
			}

			continue;
		}

		INetMessage* pNetMessage = FindMessage(cmd);
		if (pNetMessage == nullptr)
		{
#ifdef DEBUG
			DevMsg(SMEXT_TAG " Unknown net message (%i) from %s.\n", cmd, m_RemoteAddress.ToString());
#endif
			m_pMessageHandler->ConnectionCrashed("Buffer overflow in net message");
			return false;
		}

		const char* pchName = pNetMessage->GetName();

		int startbit = buf.GetNumBitsRead();

		if (!pNetMessage->ReadFromBuffer(buf))
		{
			m_pMessageHandler->ConnectionCrashed("Buffer overflow in net message");
			return false;
		}

		UpdateMessageStats(pNetMessage->GetGroup(), buf.GetNumBitsRead() - startbit);

		m_bProcessingMessages = true;
		bool bRet = pNetMessage->Process();
		m_bProcessingMessages = false;

		if (m_bShouldDelete)
		{
			delete this;
			return false;
		}

		if (!bRet)
		{
#ifdef DEBUG
			DevMsg(SMEXT_TAG " Failed to processing message %s from %s.\n", pchName, m_RemoteAddress.ToString());
#endif
			return false;
		}

		if (IsOverflowed())
		{
#ifdef DEBUG
			DevMsg(SMEXT_TAG " Reliable buffer is overflowed on net message %s from %s.\n", pchName, m_RemoteAddress.ToString());
#endif
			return false;
		}
	}

	return true;
}

namespace NetChanFix
{
	bool Load(char* error, int maxlen)
	{
		DETOUR_INIT_CLASS_MEMBER(CNetChan, ProcessMessages, "CNetChan::ProcessMessages");
		return true;
	}

	void Unload()
	{
		DETOUR_DESTROY_CLASS_MEMBER(CNetChan, ProcessMessages);
	}
}