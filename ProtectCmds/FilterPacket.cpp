#include "CDetour/detours.h"
#include <vcrmode.h>

#define QUERY_HEADER "\xFF\xFF\xFF\xFF"
#define NORMAL_SEQ_PACKET "\x00"

int (*g_real_recvfrom_ptr)(int , char *, int , int , struct sockaddr *, int *);
bool g_brecvfrom_hooked = false;

CDetour *g_FilterPacketDetour = nullptr;

int MyRecvFromHook(int s, char *buf, int len, int flags, struct sockaddr *from, int *fromlen) {
	int ret = g_real_recvfrom_ptr(s,buf,len,flags,from,fromlen);
	
	if (ret >= 0) {
		if ( !memcmp(buf+3, NORMAL_SEQ_PACKET, 1) ) return ret;
		else if ( !memcmp(buf, QUERY_HEADER, 4) ) return ret;
		
		buf[0] = '\x00';
		
		return 1;
	}
	
	return -1;
}

namespace FilterPacket
{
	bool Load(char* error, int maxlen)
	{
		if (!g_brecvfrom_hooked)
		{
			g_real_recvfrom_ptr = g_pVCR->Hook_recvfrom;
			g_pVCR->Hook_recvfrom = &MyRecvFromHook;
			g_brecvfrom_hooked = true;
		}
		return true;
	}

	void Unload()
	{
		if (g_brecvfrom_hooked) {
			g_pVCR->Hook_recvfrom = g_real_recvfrom_ptr;
			g_brecvfrom_hooked = false;
		}
	}
}