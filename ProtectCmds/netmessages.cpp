#include "netmessages.h"

#include <strtools.h>
#include <bitbuf.h>

#include "net.h"
#include "common.h"
#include <igameevents.h>
#include <utlbuffer.h>
#include <strtools.h>
#include <server_class.h>
#include <soundflags.h>

static char g_szText[1024];

const char* CLC_VoiceData::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: %i bytes", GetName(), Bits2Bytes(m_nLength));
	return g_szText;
}

bool CLC_VoiceData::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	m_nLength = m_DataOut.GetNumBitsWritten();

	buffer.WriteWord(m_nLength);	// length in bits

	//Send this client's XUID (only needed on the 360)
#if defined ( _X360 )
	buffer.WriteLongLong(m_xuid);
#endif

	return buffer.WriteBits(m_DataOut.GetBasePointer(), m_nLength);
}

bool CLC_VoiceData::ReadFromBuffer(bf_read& buffer)
{

	m_nLength = buffer.ReadWord();	// length in bits

#if defined ( _X360 )
	m_xuid = buffer.ReadLongLong();
#endif

	m_DataIn = buffer;

	return buffer.SeekRelative(m_nLength);
}

const char* CLC_Move::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: backup %i, new %i, bytes %i", GetName(),
		m_nNewCommands, m_nBackupCommands, Bits2Bytes(m_nLength));
	return g_szText;
}

bool CLC_Move::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	m_nLength = m_DataOut.GetNumBitsWritten();

	buffer.WriteUBitLong(m_nNewCommands, NUM_NEW_COMMAND_BITS);
	buffer.WriteUBitLong(m_nBackupCommands, NUM_BACKUP_COMMAND_BITS);

	buffer.WriteWord(m_nLength);

	return buffer.WriteBits(m_DataOut.GetData(), m_nLength);
}

bool CLC_Move::ReadFromBuffer(bf_read& buffer)
{

	m_nNewCommands = buffer.ReadUBitLong(NUM_NEW_COMMAND_BITS);
	m_nBackupCommands = buffer.ReadUBitLong(NUM_BACKUP_COMMAND_BITS);
	m_nLength = buffer.ReadWord();
	m_DataIn = buffer;
	return buffer.SeekRelative(m_nLength);
}

const char* CLC_ClientInfo::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: SendTableCRC %i", GetName(),
		m_nSendTableCRC);
	return g_szText;
}

bool CLC_ClientInfo::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	buffer.WriteLong(m_nServerCount);
	buffer.WriteLong(m_nSendTableCRC);
	buffer.WriteOneBit(m_bIsHLTV ? 1 : 0);
	buffer.WriteLong(m_nFriendsID);
	buffer.WriteString(m_FriendsName);

	for (int i = 0; i < MAX_CUSTOM_FILES; i++)
	{
		if (m_nCustomFiles[i] != 0)
		{
			buffer.WriteOneBit(1);
			buffer.WriteUBitLong(m_nCustomFiles[i], 32);
		}
		else
		{
			buffer.WriteOneBit(0);
		}
	}

	return !buffer.IsOverflowed();
}

bool CLC_ClientInfo::ReadFromBuffer(bf_read& buffer)
{

	m_nServerCount = buffer.ReadLong();
	m_nSendTableCRC = buffer.ReadLong();
	m_bIsHLTV = buffer.ReadOneBit() != 0;
	m_nFriendsID = buffer.ReadLong();
	buffer.ReadString(m_FriendsName, sizeof(m_FriendsName));

	for (int i = 0; i < MAX_CUSTOM_FILES; i++)
	{
		if (buffer.ReadOneBit() != 0)
		{
			m_nCustomFiles[i] = buffer.ReadUBitLong(32);
		}
		else
		{
			m_nCustomFiles[i] = 0;
		}
	}


	return !buffer.IsOverflowed();
}

bool CLC_BaselineAck::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteLong(m_nBaselineTick);
	buffer.WriteUBitLong(m_nBaselineNr, 1);
	return !buffer.IsOverflowed();
}

bool CLC_BaselineAck::ReadFromBuffer(bf_read& buffer)
{

	m_nBaselineTick = buffer.ReadLong();
	m_nBaselineNr = buffer.ReadUBitLong(1);
	return !buffer.IsOverflowed();
}

const char* CLC_BaselineAck::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: tick %i", GetName(), m_nBaselineTick);
	return g_szText;
}

bool CLC_ListenEvents::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	int count = MAX_EVENT_NUMBER / 32;
	for (int i = 0; i < count; ++i)
	{
		buffer.WriteUBitLong(m_EventArray.GetDWord(i), 32);
	}

	return !buffer.IsOverflowed();
}

bool CLC_ListenEvents::ReadFromBuffer(bf_read& buffer)
{

	int count = MAX_EVENT_NUMBER / 32;
	for (int i = 0; i < count; ++i)
	{
		m_EventArray.SetDWord(i, buffer.ReadUBitLong(32));
	}

	return !buffer.IsOverflowed();
}

const char* CLC_ListenEvents::ToString(void) const
{
	int count = 0;

	for (int i = 0; i < MAX_EVENT_NUMBER; i++)
	{
		if (m_EventArray.Get(i))
			count++;
	}

	Q_snprintf(g_szText, sizeof(g_szText), "%s: registered events %i", GetName(), count);
	return g_szText;
}

bool CLC_RespondCvarValue::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	buffer.WriteSBitLong(m_iCookie, 32);
	buffer.WriteSBitLong(m_eStatusCode, 4);

	buffer.WriteString(m_szCvarName);
	buffer.WriteString(m_szCvarValue);

	return !buffer.IsOverflowed();
}

bool CLC_RespondCvarValue::ReadFromBuffer(bf_read& buffer)
{

	m_iCookie = buffer.ReadSBitLong(32);
	m_eStatusCode = (EQueryCvarValueStatus)buffer.ReadSBitLong(4);

	// Read the name.
	buffer.ReadString(m_szCvarNameBuffer, sizeof(m_szCvarNameBuffer));
	m_szCvarName = m_szCvarNameBuffer;

	// Read the value.
	buffer.ReadString(m_szCvarValueBuffer, sizeof(m_szCvarValueBuffer));
	m_szCvarValue = m_szCvarValueBuffer;

	return !buffer.IsOverflowed();
}

const char* CLC_RespondCvarValue::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: status: %d, value: %s, cookie: %d", GetName(), m_eStatusCode, m_szCvarValue, m_iCookie);
	return g_szText;
}

bool SVC_Print::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	return buffer.WriteString(m_szText ? m_szText : " svc_print NULL");
}

bool SVC_Print::ReadFromBuffer(bf_read& buffer)
{

	m_szText = m_szTextBuffer;

	return buffer.ReadString(m_szTextBuffer, sizeof(m_szTextBuffer));
}

const char* SVC_Print::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: \"%s\"", GetName(), m_szText);
	return g_szText;
}

bool NET_StringCmd::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	return buffer.WriteString(m_szCommand ? m_szCommand : " NET_StringCmd NULL");
}

bool NET_StringCmd::ReadFromBuffer(bf_read& buffer)
{

	m_szCommand = m_szCommandBuffer;

	return buffer.ReadString(m_szCommandBuffer, sizeof(m_szCommandBuffer));
}

const char* NET_StringCmd::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: \"%s\"", GetName(), m_szCommand);
	return g_szText;
}

bool SVC_ServerInfo::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteShort(m_nProtocol);
	buffer.WriteLong(m_nServerCount);
	buffer.WriteOneBit(m_bIsHLTV ? 1 : 0);
	buffer.WriteOneBit(m_bIsDedicated ? 1 : 0);
	buffer.WriteLong(m_nClientCRC);  // To prevent cheating with hacked client dlls
	buffer.WriteWord(m_nMaxClasses);
	buffer.WriteLong(m_nMapCRC);       // To prevent cheating with hacked maps
	buffer.WriteByte(m_nPlayerSlot);
	buffer.WriteByte(m_nMaxClients);
	buffer.WriteFloat(m_fTickInterval);
	buffer.WriteChar(m_cOS);
	buffer.WriteString(m_szGameDir);
	buffer.WriteString(m_szMapName);
	buffer.WriteString(m_szSkyName);
	buffer.WriteString(m_szHostName);

	return !buffer.IsOverflowed();
}

bool SVC_ServerInfo::ReadFromBuffer(bf_read& buffer)
{

	m_szGameDir = m_szGameDirBuffer;
	m_szMapName = m_szMapNameBuffer;
	m_szSkyName = m_szSkyNameBuffer;
	m_szHostName = m_szHostNameBuffer;

	m_nProtocol = buffer.ReadShort();
	m_nServerCount = buffer.ReadLong();
	m_bIsHLTV = buffer.ReadOneBit() != 0;
	m_bIsDedicated = buffer.ReadOneBit() != 0;
	m_nClientCRC = buffer.ReadLong();  // To prevent cheating with hacked client dlls
	m_nMaxClasses = buffer.ReadWord();
	m_nMapCRC = buffer.ReadLong(); // To prevent cheating with hacked maps
	m_nPlayerSlot = buffer.ReadByte();
	m_nMaxClients = buffer.ReadByte();
	m_fTickInterval = buffer.ReadFloat();
	m_cOS = buffer.ReadChar();
	buffer.ReadString(m_szGameDirBuffer, sizeof(m_szGameDirBuffer));
	buffer.ReadString(m_szMapNameBuffer, sizeof(m_szMapNameBuffer));
	buffer.ReadString(m_szSkyNameBuffer, sizeof(m_szSkyNameBuffer));
	buffer.ReadString(m_szHostNameBuffer, sizeof(m_szHostNameBuffer));

	return !buffer.IsOverflowed();
}

const char* SVC_ServerInfo::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: game \"%s\", map \"%s\", max %i", GetName(), m_szGameDir, m_szMapName, m_nMaxClients);
	return g_szText;
}

bool NET_SignonState::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteByte(m_nSignonState);
	buffer.WriteLong(m_nSpawnCount);

	return !buffer.IsOverflowed();
}

bool NET_SignonState::ReadFromBuffer(bf_read& buffer)
{

	m_nSignonState = buffer.ReadByte();
	m_nSpawnCount = buffer.ReadLong();

	return !buffer.IsOverflowed();
}

const char* NET_SignonState::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: state %i, count %i", GetName(), m_nSignonState, m_nSpawnCount);
	return g_szText;
}

bool SVC_BSPDecal::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteBitVec3Coord(m_Pos);
	buffer.WriteUBitLong(m_nDecalTextureIndex, MAX_DECAL_INDEX_BITS);

	if (m_nEntityIndex != 0)
	{
		buffer.WriteOneBit(1);
		buffer.WriteUBitLong(m_nEntityIndex, MAX_EDICT_BITS);
		buffer.WriteUBitLong(m_nModelIndex, SP_MODEL_INDEX_BITS);
	}
	else
	{
		buffer.WriteOneBit(0);
	}
	buffer.WriteOneBit(m_bLowPriority ? 1 : 0);

	return !buffer.IsOverflowed();
}

bool SVC_BSPDecal::ReadFromBuffer(bf_read& buffer)
{

	buffer.ReadBitVec3Coord(m_Pos);
	m_nDecalTextureIndex = buffer.ReadUBitLong(MAX_DECAL_INDEX_BITS);

	if (buffer.ReadOneBit() != 0)
	{
		m_nEntityIndex = buffer.ReadUBitLong(MAX_EDICT_BITS);
		m_nModelIndex = buffer.ReadUBitLong(SP_MODEL_INDEX_BITS);
	}
	else
	{
		m_nEntityIndex = 0;
		m_nModelIndex = 0;
	}
	m_bLowPriority = buffer.ReadOneBit() ? true : false;

	return !buffer.IsOverflowed();
}

const char* SVC_BSPDecal::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: tex %i, ent %i, mod %i lowpriority %i",
		GetName(), m_nDecalTextureIndex, m_nEntityIndex, m_nModelIndex, m_bLowPriority ? 1 : 0);
	return g_szText;
}

bool SVC_SetView::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteUBitLong(m_nEntityIndex, MAX_EDICT_BITS);
	return !buffer.IsOverflowed();
}

bool SVC_SetView::ReadFromBuffer(bf_read& buffer)
{

	m_nEntityIndex = buffer.ReadUBitLong(MAX_EDICT_BITS);
	return !buffer.IsOverflowed();
}

const char* SVC_SetView::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: view entity %i", GetName(), m_nEntityIndex);
	return g_szText;
}

bool SVC_FixAngle::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteOneBit(m_bRelative ? 1 : 0);
	buffer.WriteBitAngle(m_Angle.x, 16);
	buffer.WriteBitAngle(m_Angle.y, 16);
	buffer.WriteBitAngle(m_Angle.z, 16);
	return !buffer.IsOverflowed();
}

bool SVC_FixAngle::ReadFromBuffer(bf_read& buffer)
{

	m_bRelative = buffer.ReadOneBit() != 0;
	m_Angle.x = buffer.ReadBitAngle(16);
	m_Angle.y = buffer.ReadBitAngle(16);
	m_Angle.z = buffer.ReadBitAngle(16);
	return !buffer.IsOverflowed();
}

const char* SVC_FixAngle::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: %s %.1f %.1f %.1f ", GetName(), m_bRelative ? "relative" : "absolute",
		m_Angle[0], m_Angle[1], m_Angle[2]);
	return g_szText;
}

bool SVC_CrosshairAngle::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteBitAngle(m_Angle.x, 16);
	buffer.WriteBitAngle(m_Angle.y, 16);
	buffer.WriteBitAngle(m_Angle.z, 16);
	return !buffer.IsOverflowed();
}

bool SVC_CrosshairAngle::ReadFromBuffer(bf_read& buffer)
{

	m_Angle.x = buffer.ReadBitAngle(16);
	m_Angle.y = buffer.ReadBitAngle(16);
	m_Angle.z = buffer.ReadBitAngle(16);
	return !buffer.IsOverflowed();
}

const char* SVC_CrosshairAngle::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: (%.1f %.1f %.1f)", GetName(), m_Angle[0], m_Angle[1], m_Angle[2]);
	return g_szText;
}

bool SVC_VoiceInit::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteString(m_szVoiceCodec ? m_szVoiceCodec : "svc_voiceinit NULL");
	buffer.WriteByte(m_nQuality);
	return !buffer.IsOverflowed();
}

bool SVC_VoiceInit::ReadFromBuffer(bf_read& buffer)
{

	m_szVoiceCodec = m_szVoiceCodecBuffer;

	buffer.ReadString(m_szVoiceCodecBuffer, sizeof(m_szVoiceCodecBuffer));
	m_nQuality = buffer.ReadByte();
	return !buffer.IsOverflowed();
}

const char* SVC_VoiceInit::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: codec \"%s\", qualitty %i", GetName(), m_szVoiceCodec, m_nQuality);
	return g_szText;
}

bool SVC_VoiceData::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteByte(m_nFromClient);
	buffer.WriteWord(m_nLength);

	return buffer.WriteBits(m_DataOut, m_nLength);
}

bool SVC_VoiceData::ReadFromBuffer(bf_read& buffer)
{
	m_nFromClient = buffer.ReadByte();
	m_nLength = buffer.ReadWord();

	m_DataIn = buffer;
	return buffer.SeekRelative(m_nLength);
}

const char* SVC_VoiceData::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: client %i, bytes %i", GetName(), m_nFromClient, Bits2Bytes(m_nLength));
	return g_szText;
}

#define NET_TICK_SCALEUP	100000.0f

bool NET_Tick::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteLong(m_nTick);
#if PROTOCOL_VERSION > 10
	buffer.WriteUBitLong(clamp((int)(NET_TICK_SCALEUP * m_flHostFrameTime), 0, 65535), 16);
	buffer.WriteUBitLong(clamp((int)(NET_TICK_SCALEUP * m_flHostFrameTimeStdDeviation), 0, 65535), 16);
#endif
	return !buffer.IsOverflowed();
}

bool NET_Tick::ReadFromBuffer(bf_read& buffer)
{
	m_nTick = buffer.ReadLong();

	return !buffer.IsOverflowed();
}

const char* NET_Tick::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: tick %i", GetName(), m_nTick);
	return g_szText;
}

bool SVC_UserMessage::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	m_nLength = m_DataOut.GetNumBitsWritten();

	buffer.WriteByte(m_nMsgType);
	buffer.WriteUBitLong(m_nLength, 11);  // max 256 * 8 bits, see MAX_USER_MSG_DATA

	return buffer.WriteBits(m_DataOut.GetData(), m_nLength);
}

bool SVC_UserMessage::ReadFromBuffer(bf_read& buffer)
{
	m_nMsgType = buffer.ReadByte();
	m_nLength = buffer.ReadUBitLong(11); // max 256 * 8 bits, see MAX_USER_MSG_DATA
	m_DataIn = buffer;
	return buffer.SeekRelative(m_nLength);
}

const char* SVC_UserMessage::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: type %i, bytes %i", GetName(), m_nMsgType, Bits2Bytes(m_nLength));
	return g_szText;
}

bool SVC_SetPause::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteOneBit(m_bPaused ? 1 : 0);
	return !buffer.IsOverflowed();
}

bool SVC_SetPause::ReadFromBuffer(bf_read& buffer)
{

	m_bPaused = buffer.ReadOneBit() != 0;
	return !buffer.IsOverflowed();
}

const char* SVC_SetPause::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: %s", GetName(), m_bPaused ? "paused" : "unpaused");
	return g_szText;
}


bool NET_SetConVar::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	int numvars = m_ConVars.Count();

	// Note how many we're sending
	buffer.WriteByte(numvars);

	for (int i = 0; i < numvars; i++)
	{
		cvar_t* cvar = &m_ConVars[i];
		buffer.WriteString(cvar->name);
		buffer.WriteString(cvar->value);
	}

	return !buffer.IsOverflowed();
}

bool NET_SetConVar::ReadFromBuffer(bf_read& buffer)
{

	int numvars = buffer.ReadByte();

	m_ConVars.RemoveAll();

	for (int i = 0; i < numvars; i++)
	{
		cvar_t cvar;
		buffer.ReadString(cvar.name, sizeof(cvar.name));
		buffer.ReadString(cvar.value, sizeof(cvar.value));
		m_ConVars.AddToTail(cvar);

	}
	return !buffer.IsOverflowed();
}

const char* NET_SetConVar::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: %i cvars, \"%s\"=\"%s\"",
		GetName(), m_ConVars.Count(),
		m_ConVars[0].name, m_ConVars[0].value);
	return g_szText;
}

bool SVC_UpdateStringTable::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	m_nLength = m_DataOut.GetNumBitsWritten();

	buffer.WriteUBitLong(m_nTableID, Q_log2(MAX_TABLES));	// TODO check bounds

	if (m_nChangedEntries == 1)
	{
		buffer.WriteOneBit(0); // only one entry changed
	}
	else
	{
		buffer.WriteOneBit(1);
		buffer.WriteWord(m_nChangedEntries);	// more entries changed
	}

	buffer.WriteUBitLong(m_nLength, 20);

	return buffer.WriteBits(m_DataOut.GetData(), m_nLength);
}

bool SVC_UpdateStringTable::ReadFromBuffer(bf_read& buffer)
{

	m_nTableID = buffer.ReadUBitLong(Q_log2(MAX_TABLES));

	if (buffer.ReadOneBit() != 0)
	{
		m_nChangedEntries = buffer.ReadWord();
	}
	else
	{
		m_nChangedEntries = 1;
	}

	m_nLength = buffer.ReadUBitLong(20);
	m_DataIn = buffer;
	return buffer.SeekRelative(m_nLength);
}

const char* SVC_UpdateStringTable::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: table %i, changed %i, bytes %i", GetName(), m_nTableID, m_nChangedEntries, Bits2Bytes(m_nLength));
	return g_szText;
}

SVC_CreateStringTable::SVC_CreateStringTable()
{

}

bool SVC_CreateStringTable::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	m_nLength = m_DataOut.GetNumBitsWritten();

	/*
	JASON: this code is no longer needed; the ':' is prepended to the table name at table creation time.
	if ( m_bIsFilenames )
	{
		// identifies a table that hosts filenames
		buffer.WriteByte( ':' );
	}
	*/

	buffer.WriteString(m_szTableName);
	buffer.WriteWord(m_nMaxEntries);
	int encodeBits = Q_log2(m_nMaxEntries);
	buffer.WriteUBitLong(m_nNumEntries, encodeBits + 1);
	buffer.WriteUBitLong(m_nLength, NET_MAX_PALYLOAD_BITS + 3); // length in bits

	buffer.WriteOneBit(m_bUserDataFixedSize ? 1 : 0);
	if (m_bUserDataFixedSize)
	{
		buffer.WriteUBitLong(m_nUserDataSize, 12);
		buffer.WriteUBitLong(m_nUserDataSizeBits, 4);
	}

	return buffer.WriteBits(m_DataOut.GetData(), m_nLength);
}

bool SVC_CreateStringTable::ReadFromBuffer(bf_read& buffer)
{

	char prefix = buffer.PeekUBitLong(8);
	if (prefix == ':')
	{
		// table hosts filenames
		m_bIsFilenames = true;
		buffer.ReadByte();
	}
	else
	{
		m_bIsFilenames = false;
	}

	m_szTableName = m_szTableNameBuffer;
	buffer.ReadString(m_szTableNameBuffer, sizeof(m_szTableNameBuffer));
	m_nMaxEntries = buffer.ReadWord();
	int encodeBits = Q_log2(m_nMaxEntries);
	m_nNumEntries = buffer.ReadUBitLong(encodeBits + 1);
	m_nLength = buffer.ReadUBitLong(NET_MAX_PALYLOAD_BITS + 3); // length in bits

	m_bUserDataFixedSize = buffer.ReadOneBit() ? true : false;
	if (m_bUserDataFixedSize)
	{
		m_nUserDataSize = buffer.ReadUBitLong(12);
		m_nUserDataSizeBits = buffer.ReadUBitLong(4);
	}
	else
	{
		m_nUserDataSize = 0;
		m_nUserDataSizeBits = 0;
	}

	m_DataIn = buffer;
	return buffer.SeekRelative(m_nLength);
}

const char* SVC_CreateStringTable::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: table %s, entries %i, bytes %i userdatasize %i userdatabits %i",
		GetName(), m_szTableName, m_nNumEntries, Bits2Bytes(m_nLength), m_nUserDataSize, m_nUserDataSizeBits);
	return g_szText;
}

bool SVC_Sounds::WriteToBuffer(bf_write& buffer)
{
	m_nLength = m_DataOut.GetNumBitsWritten();

	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	Assert(m_nNumSounds > 0);

	if (m_bReliableSound)
	{
		// as single sound message is 32 bytes long maximum
		buffer.WriteOneBit(1);
		buffer.WriteUBitLong(m_nLength, 8);
	}
	else
	{
		// a bunch of unreliable messages
		buffer.WriteOneBit(0);
		buffer.WriteUBitLong(m_nNumSounds, 8);
		buffer.WriteUBitLong(m_nLength, 16);
	}

	return buffer.WriteBits(m_DataOut.GetData(), m_nLength);
}

bool SVC_Sounds::ReadFromBuffer(bf_read& buffer)
{

	m_bReliableSound = buffer.ReadOneBit() != 0;

	if (m_bReliableSound)
	{
		m_nNumSounds = 1;
		m_nLength = buffer.ReadUBitLong(8);

	}
	else
	{
		m_nNumSounds = buffer.ReadUBitLong(8);
		m_nLength = buffer.ReadUBitLong(16);
	}

	m_DataIn = buffer;
	return buffer.SeekRelative(m_nLength);
}

const char* SVC_Sounds::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: number %i,%s bytes %i",
		GetName(), m_nNumSounds, m_bReliableSound ? " reliable," : "", Bits2Bytes(m_nLength));
	return g_szText;
}

bool SVC_Prefetch::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	// Don't write type until we have more thanone
	// buffer.WriteUBitLong( m_fType, 1 );
	buffer.WriteUBitLong(m_nSoundIndex, MAX_SOUND_INDEX_BITS);
	return !buffer.IsOverflowed();
}

bool SVC_Prefetch::ReadFromBuffer(bf_read& buffer)
{

	m_fType = SOUND; // buffer.ReadUBitLong( 1 );
	m_nSoundIndex = buffer.ReadUBitLong(MAX_SOUND_INDEX_BITS);

	return !buffer.IsOverflowed();
}

const char* SVC_Prefetch::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: type %i index %i",
		GetName(),
		(int)m_fType,
		(int)m_nSoundIndex);
	return g_szText;
}

bool SVC_ClassInfo::WriteToBuffer(bf_write& buffer)
{
	if (!m_bCreateOnClient)
	{
		m_nNumServerClasses = m_Classes.Count();	// use number from list list	
	}

	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	buffer.WriteShort(m_nNumServerClasses);

	int serverClassBits = Q_log2(m_nNumServerClasses) + 1;

	buffer.WriteOneBit(m_bCreateOnClient ? 1 : 0);

	if (m_bCreateOnClient)
		return !buffer.IsOverflowed();

	for (int i = 0; i < m_nNumServerClasses; i++)
	{
		class_t* serverclass = &m_Classes[i];

		buffer.WriteUBitLong(serverclass->classID, serverClassBits);
		buffer.WriteString(serverclass->classname);
		buffer.WriteString(serverclass->datatablename);
	}

	return !buffer.IsOverflowed();
}

bool SVC_ClassInfo::ReadFromBuffer(bf_read& buffer)
{

	m_Classes.RemoveAll();

	m_nNumServerClasses = buffer.ReadShort();

	int nServerClassBits = Q_log2(m_nNumServerClasses) + 1;

	m_bCreateOnClient = buffer.ReadOneBit() != 0;

	if (m_bCreateOnClient)
	{
		return !buffer.IsOverflowed(); // stop here
	}

	for (int i = 0; i < m_nNumServerClasses; i++)
	{
		class_t serverclass;

		serverclass.classID = buffer.ReadUBitLong(nServerClassBits);
		buffer.ReadString(serverclass.classname, sizeof(serverclass.classname));
		buffer.ReadString(serverclass.datatablename, sizeof(serverclass.datatablename));

		m_Classes.AddToTail(serverclass);
	}

	return !buffer.IsOverflowed();
}

const char* SVC_ClassInfo::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: num %i, %s", GetName(),
		m_nNumServerClasses, m_bCreateOnClient ? "use client classes" : "full update");
	return g_szText;
}

/*
bool SVC_SpawnBaseline::WriteToBuffer( bf_write &buffer )
{
	m_nLength = m_DataOut.GetNumBitsWritten();

	buffer.WriteUBitLong( GetType(), NETMSG_TYPE_BITS );

	buffer.WriteUBitLong( m_nEntityIndex, MAX_EDICT_BITS );

	buffer.WriteUBitLong( m_nClassID, MAX_SERVER_CLASS_BITS );

	buffer.WriteUBitLong( m_nLength, Q_log2(MAX_PACKEDENTITY_DATA<<3) ); // TODO see below

	return buffer.WriteBits( m_DataOut.GetData(), m_nLength );
}

bool SVC_SpawnBaseline::ReadFromBuffer( bf_read &buffer )
{
	m_nEntityIndex = buffer.ReadUBitLong( MAX_EDICT_BITS );

	m_nClassID = buffer.ReadUBitLong( MAX_SERVER_CLASS_BITS );

	m_nLength = buffer.ReadUBitLong( Q_log2(MAX_PACKEDENTITY_DATA<<3) ); // TODO wrong, check bounds

	m_DataIn = buffer;

	return buffer.SeekRelative( m_nLength );
}

const char *SVC_SpawnBaseline::ToString(void) const
{
	static char text[256];
	Q_snprintf(text, sizeof(text), "%s: ent %i, class %i, bytes %i",
		GetName(), m_nEntityIndex, m_nClassID, Bits2Bytes(m_nLength) );
	return text;
} */

bool SVC_GameEvent::WriteToBuffer(bf_write& buffer)
{
	m_nLength = m_DataOut.GetNumBitsWritten();

	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteUBitLong(m_nLength, 11);  // max 8 * 256 bits
	return buffer.WriteBits(m_DataOut.GetData(), m_nLength);

}

bool SVC_GameEvent::ReadFromBuffer(bf_read& buffer)
{

	m_nLength = buffer.ReadUBitLong(11); // max 8 * 256 bits
	m_DataIn = buffer;
	return buffer.SeekRelative(m_nLength);
}

const char* SVC_GameEvent::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: bytes %i", GetName(), Bits2Bytes(m_nLength));
	return g_szText;
}

bool SVC_SendTable::WriteToBuffer(bf_write& buffer)
{
	m_nLength = m_DataOut.GetNumBitsWritten();

	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteOneBit(m_bNeedsDecoder ? 1 : 0);
	buffer.WriteShort(m_nLength);
	buffer.WriteBits(m_DataOut.GetData(), m_nLength);

	return !buffer.IsOverflowed();
}

bool SVC_SendTable::ReadFromBuffer(bf_read& buffer)
{

	m_bNeedsDecoder = buffer.ReadOneBit() != 0;
	m_nLength = buffer.ReadShort();		// TODO do we have a maximum length ? check that

	m_DataIn = buffer;

	return buffer.SeekRelative(m_nLength);
}

const char* SVC_SendTable::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: needs Decoder %s,bytes %i",
		GetName(), m_bNeedsDecoder ? "yes" : "no", Bits2Bytes(m_nLength));
	return g_szText;
}

bool SVC_EntityMessage::WriteToBuffer(bf_write& buffer)
{
	m_nLength = m_DataOut.GetNumBitsWritten();

	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteUBitLong(m_nEntityIndex, MAX_EDICT_BITS);
	buffer.WriteUBitLong(m_nClassID, MAX_SERVER_CLASS_BITS);
	buffer.WriteUBitLong(m_nLength, 11);  // max 8 * 256 bits

	return buffer.WriteBits(m_DataOut.GetData(), m_nLength);
}

bool SVC_EntityMessage::ReadFromBuffer(bf_read& buffer)
{

	m_nEntityIndex = buffer.ReadUBitLong(MAX_EDICT_BITS);
	m_nClassID = buffer.ReadUBitLong(MAX_SERVER_CLASS_BITS);
	m_nLength = buffer.ReadUBitLong(11);  // max 8 * 256 bits
	m_DataIn = buffer;
	return buffer.SeekRelative(m_nLength);
}

const char* SVC_EntityMessage::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: entity %i, class %i, bytes %i",
		GetName(), m_nEntityIndex, m_nClassID, Bits2Bytes(m_nLength));
	return g_szText;
}

bool SVC_PacketEntities::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	buffer.WriteUBitLong(m_nMaxEntries, MAX_EDICT_BITS);

	buffer.WriteOneBit(m_bIsDelta ? 1 : 0);

	if (m_bIsDelta)
	{
		buffer.WriteLong(m_nDeltaFrom);
	}

	buffer.WriteUBitLong(m_nBaseline, 1);

	buffer.WriteUBitLong(m_nUpdatedEntries, MAX_EDICT_BITS);

	buffer.WriteUBitLong(m_nLength, DELTASIZE_BITS);

	buffer.WriteOneBit(m_bUpdateBaseline ? 1 : 0);

	buffer.WriteBits(m_DataOut.GetData(), m_nLength);

	return !buffer.IsOverflowed();
}

bool SVC_PacketEntities::ReadFromBuffer(bf_read& buffer)
{

	m_nMaxEntries = buffer.ReadUBitLong(MAX_EDICT_BITS);

	m_bIsDelta = buffer.ReadOneBit() != 0;

	if (m_bIsDelta)
	{
		m_nDeltaFrom = buffer.ReadLong();
	}
	else
	{
		m_nDeltaFrom = -1;
	}

	m_nBaseline = buffer.ReadUBitLong(1);

	m_nUpdatedEntries = buffer.ReadUBitLong(MAX_EDICT_BITS);

	m_nLength = buffer.ReadUBitLong(DELTASIZE_BITS);

	m_bUpdateBaseline = buffer.ReadOneBit() != 0;

	m_DataIn = buffer;

	return buffer.SeekRelative(m_nLength);
}

const char* SVC_PacketEntities::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: delta %i, max %i, changed %i,%s bytes %i",
		GetName(), m_nDeltaFrom, m_nMaxEntries, m_nUpdatedEntries, m_bUpdateBaseline ? " BL update," : "", Bits2Bytes(m_nLength));
	return g_szText;
}

SVC_Menu::SVC_Menu(DIALOG_TYPE type, KeyValues* data)
{
	m_bReliable = true;

	m_Type = type;
	m_MenuKeyValues = data->MakeCopy();
	m_iLength = -1;
}

SVC_Menu::~SVC_Menu()
{
	if (m_MenuKeyValues)
	{
		m_MenuKeyValues->deleteThis();
	}
}

bool SVC_Menu::WriteToBuffer(bf_write& buffer)
{
	if (!m_MenuKeyValues)
	{
		return false;
	}

	CUtlBuffer buf;
	m_MenuKeyValues->WriteAsBinary(buf);

	if (buf.TellPut() > 4096)
	{
		Msg("Too much menu data (4096 bytes max)\n");
		return false;
	}

	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteShort(m_Type);
	buffer.WriteWord(buf.TellPut());
	buffer.WriteBytes(buf.Base(), buf.TellPut());

	return !buffer.IsOverflowed();
}

bool SVC_Menu::ReadFromBuffer(bf_read& buffer)
{

	m_Type = (DIALOG_TYPE)buffer.ReadShort();
	m_iLength = buffer.ReadWord();

	CUtlBuffer buf(0, m_iLength);
	buffer.ReadBytes(buf.Base(), m_iLength);
	buf.SeekPut(CUtlBuffer::SEEK_HEAD, m_iLength);

	if (m_MenuKeyValues)
	{
		m_MenuKeyValues->deleteThis();
	}
	m_MenuKeyValues = new KeyValues("menu");
	Assert(m_MenuKeyValues);

	m_MenuKeyValues->ReadAsBinary(buf);

	return !buffer.IsOverflowed();
}

const char* SVC_Menu::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: %i \"%s\" (len:%i)", GetName(),
		m_Type, m_MenuKeyValues ? m_MenuKeyValues->GetName() : "No KeyValues", m_iLength);
	return g_szText;
}

bool SVC_GameEventList::WriteToBuffer(bf_write& buffer)
{
	Assert(m_nNumEvents > 0);

	m_nLength = m_DataOut.GetNumBitsWritten();

	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);
	buffer.WriteUBitLong(m_nNumEvents, MAX_EVENT_BITS);
	buffer.WriteUBitLong(m_nLength, 20);
	return buffer.WriteBits(m_DataOut.GetData(), m_nLength);
}

bool SVC_GameEventList::ReadFromBuffer(bf_read& buffer)
{

	m_nNumEvents = buffer.ReadUBitLong(MAX_EVENT_BITS);
	m_nLength = buffer.ReadUBitLong(20);
	m_DataIn = buffer;
	return buffer.SeekRelative(m_nLength);
}

const char* SVC_GameEventList::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: number %i, bytes %i", GetName(), m_nNumEvents, Bits2Bytes(m_nLength));
	return g_szText;
}

bool SVC_GetCvarValue::WriteToBuffer(bf_write& buffer)
{
	buffer.WriteUBitLong(GetType(), NETMSG_TYPE_BITS);

	buffer.WriteSBitLong(m_iCookie, 32);
	buffer.WriteString(m_szCvarName);

	return !buffer.IsOverflowed();
}

bool SVC_GetCvarValue::ReadFromBuffer(bf_read& buffer)
{

	m_iCookie = buffer.ReadSBitLong(32);
	buffer.ReadString(m_szCvarNameBuffer, sizeof(m_szCvarNameBuffer));
	m_szCvarName = m_szCvarNameBuffer;

	return !buffer.IsOverflowed();
}

const char* SVC_GetCvarValue::ToString(void) const
{
	Q_snprintf(g_szText, sizeof(g_szText), "%s: cvar: %s, cookie: %d", GetName(), m_szCvarName, m_iCookie);
	return g_szText;
}
