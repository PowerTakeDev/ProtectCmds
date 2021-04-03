#ifndef HASH_STEAMID_H
#define HASH_STEAMID_H

namespace HashSteamID
{
	bool Load(char* error, int maxlen);
	void Unload();
}

#endif // HASH_STEAMID_H