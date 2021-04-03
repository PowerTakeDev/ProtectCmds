#ifndef USERID_H
#define USERID_H

#include "SteamCommon.h"

#define IDTYPE_WON		0
#define IDTYPE_STEAM	1
#define IDTYPE_VALVE	2
#define IDTYPE_HLTV		3

struct USERID_t
{
	int idtype;
	union
	{
		TSteamGlobalUserID steamid;
	} uid;
};

#endif // USERID_H