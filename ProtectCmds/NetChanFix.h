#ifndef NETCHAN_FIX_H
#define NETCHAN_FIX_H

#include "NetChan.h"

namespace NetChanFix
{
	bool Load(char* error, int maxlen);
	void Unload();
}

#endif // NETCHAN_FIX_H