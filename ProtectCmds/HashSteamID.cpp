#include "HashSteamID.h"
#include "CDetour/detours.h"
#include "baseclient.h"
#include <steam/steamclientpublic.h>
#include <checksum_crc.h>
#include <vstdlib/random.h>
#include "common.h"

DETOUR_DECL_CLASS_MEMBER(CBaseClient, SetSteamID);

namespace CRC32
{
#define CRC32_INIT_VALUE 0xFFFFFFFFUL
#define CRC32_XOR_VALUE  0xFFFFFFFFUL

#define CRC_TABLE_LEN 256

    CRC32_t* pCRCTable;

    bool SaveCRCTable(char* error, int maxlen, char* pszPath)
    {
        FILE* pFile = fopen(pszPath, "wb");
        if (pFile == nullptr)
        {
            V_snprintf(error, maxlen, "CRC32::SaveCRCTable - Failed to open file for write (%s)", pszPath);
            return false;
        }

        fwrite(pCRCTable, 1, sizeof(CRC32_t) * CRC_TABLE_LEN, pFile);

        fclose(pFile);

        return true;
    }

    bool RandomizeTable(char* error, int maxlen, char* pszPath)
    {
        RandomSeed(time(0));
        
        byte* pTable = (byte*)pCRCTable;

        for (size_t i = 0; i < sizeof(CRC32_t) * CRC_TABLE_LEN; i++)
        {
            pTable[i] = RandomInt(0x00, 0xFF);
        }

        if (!SaveCRCTable(error, maxlen, pszPath))
            return false;

        return true;
    }

    bool LoadCRCTable(char* error, int maxlen, char* pszPath)
    {
        FILE* pFile = fopen(pszPath, "rb");
        if (pFile == nullptr)
        {
            V_snprintf(error, maxlen, "CRC32::LoadCRCTable - Failed to open file for read (%s)", pszPath);
            return false;
        }

        fseek(pFile, 0L, SEEK_END);
        int nFileSize = ftell(pFile);
        fseek(pFile, 0L, SEEK_SET);

        if (nFileSize != sizeof(CRC32_t) * CRC_TABLE_LEN)
        {
            V_snprintf(
                error, maxlen,
                "CRC32::LoadCRCTable - File size %i, but expected " STR(sizeof(CRC32_t) * CRC_TABLE_LEN),
                nFileSize
            );
            fclose(pFile);
            return false;
        }

        fread(pCRCTable, 1, sizeof(CRC32_t) * CRC_TABLE_LEN, pFile);

        fclose(pFile);

        return true;
    }

    void ShutdownCRCTable()
    {
        if (pCRCTable != nullptr)
        {
            delete pCRCTable;
            pCRCTable = nullptr;
        }
    }

    bool InitCRCTable(char* error, int maxlen)
    {
        pCRCTable = new CRC32_t[CRC_TABLE_LEN];
        if (pCRCTable == nullptr)
        {
            V_snprintf(error, maxlen, "Failed to allocate %i bytes", (sizeof(CRC32_t) * CRC_TABLE_LEN));
            return false;
        }

        char szPath[MAX_OSPATH];
        g_pSM->BuildPath(Path_SM, szPath, sizeof szPath, "data/protectcmds/steamid_key.dat");

        if (!libsys->PathExists(szPath))
        {
            if (!RandomizeTable(error, maxlen, szPath))
                return false;

            return true;
        }

        if (!LoadCRCTable(error, maxlen, szPath))
            return false;

        return true;
    }

	void Init(CRC32_t* pulCRC)
	{
		*pulCRC = CRC32_INIT_VALUE;
	}

	void Final(CRC32_t* pulCRC)
	{
		*pulCRC ^= CRC32_XOR_VALUE;
	}

    void ProcessBuffer(CRC32_t* pulCRC, const void* pBuffer, int nBuffer)
    {
        // FIXME: pCRCTable possible equale null pointer

        CRC32_t ulCrc = *pulCRC;
        unsigned char* pb = (unsigned char*)pBuffer;
        unsigned int nFront;
        int nMain;

    JustAfew:

        switch (nBuffer)
        {
        case 7:
            ulCrc = pCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

        case 6:
            ulCrc = pCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

        case 5:
            ulCrc = pCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

        case 4:
            ulCrc ^= LittleLong(*(CRC32_t*)pb);
            ulCrc = pCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
            ulCrc = pCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
            ulCrc = pCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
            ulCrc = pCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
            *pulCRC = ulCrc;
            return;

        case 3:
            ulCrc = pCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

        case 2:
            ulCrc = pCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

        case 1:
            ulCrc = pCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);

        case 0:
            *pulCRC = ulCrc;
            return;
        }

        nFront = ((unsigned int)pb) & 3;
        nBuffer -= nFront;
        switch (nFront)
        {
        case 3:
            ulCrc = pCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);
        case 2:
            ulCrc = pCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);
        case 1:
            ulCrc = pCRCTable[*pb++ ^ (unsigned char)ulCrc] ^ (ulCrc >> 8);
        }

        nMain = nBuffer >> 3;
        while (nMain--)
        {
            ulCrc ^= LittleLong(*(CRC32_t*)pb);
            ulCrc = pCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
            ulCrc = pCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
            ulCrc = pCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
            ulCrc = pCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
            ulCrc ^= LittleLong(*(CRC32_t*)(pb + 4));
            ulCrc = pCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
            ulCrc = pCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
            ulCrc = pCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
            ulCrc = pCRCTable[(unsigned char)ulCrc] ^ (ulCrc >> 8);
            pb += 8;
        }

        nBuffer &= 7;
        goto JustAfew;
    }

}

void HashSteam(const CSteamID& steamid, TSteamGlobalUserID* pSteamGlobalUserID)
{
	// STEAM_X:Y:Z

	uint32 unAccountID = steamid.GetAccountID();
	
	char szSteam[12];
	int nLen = V_snprintf(szSteam, sizeof szSteam, "%u", unAccountID);

	CRC32_t crc;

	CRC32::Init(&crc);
	CRC32::ProcessBuffer(&crc, szSteam, nLen);
	CRC32::Final(&crc);

	pSteamGlobalUserID->m_SteamInstanceID = 0; // X
	pSteamGlobalUserID->m_SteamLocalUserID.Split.High32bits = 0; // Y

	if (crc < 0)
	{
		crc = -crc;
		pSteamGlobalUserID->m_SteamLocalUserID.Split.High32bits = 1; // Y
	}

	pSteamGlobalUserID->m_SteamLocalUserID.Split.Low32bits = crc; // Z
}

void CBaseClient::SetSteamID(const CSteamID& inputSteamID)
{
    //m_NetworkID.idtype = IDTYPE_STEAM;

	HashSteam(inputSteamID, &m_NetworkID.uid.steamid);

	delete m_pSteamID;
	m_pSteamID = new CSteamID;
	*m_pSteamID = inputSteamID;
}

namespace HashSteamID
{
	bool Load(char* error, int maxlen)
	{
        if (!CRC32::InitCRCTable(error, maxlen))
            return false;

		DETOUR_INIT_CLASS_MEMBER(CBaseClient, SetSteamID, "CBaseClient::SetSteamID");
		return true;
	}

	void Unload()
	{
        CRC32::ShutdownCRCTable();
		DETOUR_DESTROY_CLASS_MEMBER(CBaseClient, SetSteamID);
	}
}