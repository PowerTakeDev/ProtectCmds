#ifndef NETCHAN_H
#define NETCHAN_H

#include <bitbuf.h>
#include <inetchannel.h>
#include <inetmessage.h>
#include <utlvector.h>
#include <netadr.h>
#include "qfiles.h"
#include "net.h"

#define NET_FRAMES_BACKUP	64		// must be power of 2
#define NET_FRAMES_MASK		(NET_FRAMES_BACKUP-1)
#define MAX_SUBCHANNELS		8		// we have 8 alternative send&wait bits

#define SUBCHANNEL_FREE		0	// subchannel is free to use
#define SUBCHANNEL_TOSEND	1	// subchannel has data, but not send yet
#define SUBCHANNEL_WAITING	2   // sbuchannel sent data, waiting for ACK
#define SUBCHANNEL_DIRTY	3	// subchannel is marked as dirty during changelevel

class CNetChan : public INetChannel
{
public:
	typedef struct dataFragments_s
	{
		void* m_pFile;			// open file handle
		char m_szFilename[MAX_OSPATH]; // filename
		char* m_pBuffer;			// if NULL it's a file
		unsigned int m_uBytes;			// size in bytes
		unsigned int m_uBits;			// size in bits
		unsigned int m_uTransferID;		// only for files
		bool m_bIsCompressed;	// true if data is bzip compressed
		unsigned int m_uUncompressedSize; // full size in bytes
		bool m_bAsTCP;			// send as TCP stream
		int m_nNumFragments;	// number of total fragments
		int m_nAckedFragments; // number of fragments send & acknowledged
		int m_nPendingFragments; // number of fragments send, but not acknowledged yet
	} dataFragments_t;

	typedef struct subChannel_s
	{
		int m_nStartFragment[MAX_STREAMS];
		int m_nNumFragments[MAX_STREAMS];
		int m_nSendSeqNr;
		int m_iState; // 0 = free, 1 = scheduled to send, 2 = send & waiting, 3 = dirty
		int m_iIndex; // index in m_SubChannels[]
	} subChannel_t;

	typedef struct netframe_s
	{
		float m_fTime;
		int m_iSize;
		float m_fLatency;
		float m_fAvgLatency;
		bool m_bValid;
		int m_nChoked;
		int m_nDropped;
		float m_flInterpolationAmount;
		unsigned short m_MsgGroups[8];
	} netframe_t;

	typedef struct netflow_s
	{
		float m_fNextCompute;
		float m_fAvgBytesPerSec;
		float m_fAvgPacketsPerSec;
		float m_fAvgLoss;
		float m_fAvgChoke;
		float m_fAvgLatency;
		float m_fLatency;
		int m_nTotalPackets;
		int m_nTotalBytes;
		int m_iCurrentIndex;
		netframe_t m_Frames[NET_FRAMES_BACKUP];
		netframe_t* m_pCurrentFrame;
	} netflow_t;

public:
	bool ProcessMessages(bf_read& buf);
	static bool (CNetChan::* ProcessMessages_Actual)(bf_read&);

private:
	bool ProcessControlMessage(int cmd, bf_read& buf);
	INetMessage* FindMessage(int type);
	void UpdateMessageStats(int msggroup, int bits);
	bool IsOverflowed(void) const;

private:
	bool m_bProcessingMessages;
	bool m_bShouldDelete;
	int m_nOutSequenceNr;
	int m_nInSequenceNr;
	int m_nOutSequenceNrAck;
	int m_nOutReliableState;
	int m_nInReliableState;
	int m_nChokedPackets;
	bf_write m_StreamReliable;
	CUtlMemory<byte> m_ReliableDataBuffer;
	bf_write m_StreamUnreliable;
	CUtlMemory<byte> m_UnreliableDataBuffer;
	int m_iSocket;
	int m_iStreamSocket;
	int m_nMaxReliablePayloadSize;
	netadr_t m_RemoteAddress;
	float m_fLastReceived;
	double m_fConnectTime;
	int m_nRate;
	double m_fClearTime;
	CUtlVector<dataFragments_t*> m_WaitingList[MAX_STREAMS];
	dataFragments_t m_ReceiveList[MAX_STREAMS];
	subChannel_s m_SubChannels[MAX_SUBCHANNELS];
	int m_uFileRequestCounter;
	bool m_bFileBackgroundTranmission;
	bool m_bUseCompression;
	bool m_bStreamActive;
	int m_iSteamType;
	int m_iStreamSeqNr;
	int m_iStreamLength;
	int m_iStreamReceived;
	char m_szSteamFile[MAX_OSPATH];
	CUtlMemory<byte> m_StreamData;
	netflow_t m_DataFlow[MAX_FLOWS];
	int	m_MsgStats[INetChannelInfo::TOTAL];
	int m_nPacketDrop;
	char m_szName[32];
	unsigned int m_uChallengeNr;
	float m_fTimeout;
	INetChannelHandler* m_pMessageHandler;
	CUtlVector<INetMessage*> m_NetMessages;
	IDemoRecorder* m_pDemoRecorder;
	int m_nQueuedPackets;
};

#endif // NETCHAN_H