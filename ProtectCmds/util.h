#ifndef _INCLUDE_L4DOWNTOWN_TOOLS_UTIL_H_
#define _INCLUDE_L4DOWNTOWN_TOOLS_UTIL_H_

#define REGISTER_NATIVE_ADDR(name, code) \
	void *addr; \
	if (!g_pGameConf->GetMemSig(name, &addr) || !addr) \
	{ \
		return pContext->ThrowNativeError("Failed to locate function " #name); \
	} \
	code;


size_t UTIL_Format(char *buffer, size_t maxlength, const char *fmt, ...);

#endif //_INCLUDE_L4DOWNTOWN_TOOLS_UTIL_H_
