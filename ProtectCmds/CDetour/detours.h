#ifndef DETOURS_H_
#define DETOURS_H_

#include "extension.h"
#include <jit/jit_helpers.h>
#include <jit/x86/x86_macros.h>
#include "detourhelpers.h"

#define DETOUR_MEMBER_CALL(name) (this->*name##_Actual)
#define DETOUR_STATIC_CALL(name) (name##_Actual)

#define DETOUR_DECL_STATIC0(name, ret) \
ret (*name##_Actual)(void) = NULL; \
ret name(void)

#define DETOUR_DECL_STATIC1(name, ret, p1type, p1name) \
ret (*name##_Actual)(p1type) = NULL; \
ret name(p1type p1name)

#define DETOUR_DECL_STATIC2(name, ret, p1type, p1name, p2type, p2name) \
ret (*name##_Actual)(p1type, p2type) = NULL; \
ret name(p1type p1name, p2type p2name)

#define DETOUR_DECL_STATIC3(name, ret, p1type, p1name, p2type, p2name, p3type, p3name) \
ret (*name##_Actual)(p1type, p2type, p3type) = NULL; \
ret name(p1type p1name, p2type p2name, p3type p3name)

#define DETOUR_DECL_STATIC4(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name) \
ret (*name##_Actual)(p1type, p2type, p3type, p4type) = NULL; \
ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name)

#define DETOUR_DECL_STATIC5(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name) \
ret (*name##_Actual)(p1type, p2type, p3type, p4type, p5type) = NULL; \
ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name)

#define DETOUR_DECL_STATIC6(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name, p6type, p6name) \
ret (*name##_Actual)(p1type, p2type, p3type, p4type, p5type, p6type) = NULL; \
ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name)

#define DETOUR_DECL_STATIC7(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name, p6type, p6name, p7type, p7name) \
ret (*name##_Actual)(p1type, p2type, p3type, p4type, p5type, p6type, p7type) = NULL; \
ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name, p7type p7name)

#define DETOUR_DECL_STATIC8(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name, p6type, p6name, p7type, p7name, p8type, p8name) \
ret (*name##_Actual)(p1type, p2type, p3type, p4type, p5type, p6type, p7type, p8type) = NULL; \
ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name, p7type p7name, p8type p8name)

#define DETOUR_DECL_MEMBER0(name, ret) \
class name##Class \
{ \
public: \
	ret name(); \
	static ret (name##Class::* name##_Actual)(void); \
}; \
ret (name##Class::* name##Class::name##_Actual)(void) = NULL; \
ret name##Class::name()

#define DETOUR_DECL_MEMBER1(name, ret, p1type, p1name) \
class name##Class \
{ \
public: \
	ret name(p1type p1name); \
	static ret (name##Class::* name##_Actual)(p1type); \
}; \
ret (name##Class::* name##Class::name##_Actual)(p1type) = nullptr; \
ret name##Class::name(p1type p1name)

#define DETOUR_DECL_MEMBER2(name, ret, p1type, p1name, p2type, p2name) \
class name##Class \
{ \
public: \
	ret name(p1type p1name, p2type p2name); \
	static ret (name##Class::* name##_Actual)(p1type, p2type); \
}; \
ret (name##Class::* name##Class::name##_Actual)(p1type, p2type) = NULL; \
ret name##Class::name(p1type p1name, p2type p2name)

#define DETOUR_DECL_MEMBER3(name, ret, p1type, p1name, p2type, p2name, p3type, p3name) \
class name##Class \
{ \
public: \
	ret name(p1type p1name, p2type p2name, p3type p3name); \
	static ret (name##Class::* name##_Actual)(p1type, p2type, p3type); \
}; \
ret (name##Class::* name##Class::name##_Actual)(p1type, p2type, p3type) = NULL; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name)

#define DETOUR_DECL_MEMBER4(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name) \
class name##Class \
{ \
public: \
        ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name); \
        static ret (name##Class::* name##_Actual)(p1type, p2type, p3type, p4type); \
}; \
ret (name##Class::* name##Class::name##_Actual)(p1type, p2type, p3type, p4type) = NULL; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name, p4type p4name)

#define DETOUR_DECL_MEMBER5(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name) \
class name##Class \
{ \
public: \
        ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name); \
        static ret (name##Class::* name##_Actual)(p1type, p2type, p3type, p4type, p5type); \
}; \
ret (name##Class::* name##Class::name##_Actual)(p1type, p2type, p3type, p4type, p5type) = NULL; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name)

#define DETOUR_DECL_MEMBER6(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name, p6type, p6name) \
class name##Class \
{ \
public: \
        ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name); \
        static ret (name##Class::* name##_Actual)(p1type, p2type, p3type, p4type, p5type, p6type); \
}; \
ret (name##Class::* name##Class::name##_Actual)(p1type, p2type, p3type, p4type, p5type, p6type) = NULL; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name)

#define DETOUR_DECL_MEMBER7(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name, p6type, p6name, p7type, p7name) \
class name##Class \
{ \
public: \
        ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name, p7type p7name); \
        static ret (name##Class::* name##_Actual)(p1type, p2type, p3type, p4type, p5type, p6type, p7type); \
}; \
ret (name##Class::* name##Class::name##_Actual)(p1type, p2type, p3type, p4type, p5type, p6type, p7type) = NULL; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name, p7type p7name)

#define DETOUR_DECL_MEMBER8(name, ret, p1type, p1name, p2type, p2name, p3type, p3name, p4type, p4name, p5type, p5name, p6type, p6name, p7type, p7name, p8type, p8name) \
class name##Class \
{ \
public: \
        ret name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name, p7type p7name, p8type p8name); \
        static ret (name##Class::* name##_Actual)(p1type, p2type, p3type, p4type, p5type, p6type, p7type, p8type); \
}; \
ret (name##Class::* name##Class::name##_Actual)(p1type, p2type, p3type, p4type, p5type, p6type, p7type, p8type) = NULL; \
ret name##Class::name(p1type p1name, p2type p2name, p3type p3name, p4type p4name, p5type p5name, p6type p6name, p7type p7name, p8type p8name)


#define GET_MEMBER_CALLBACK(name) (void *)GetCodeAddress(&name##Class::name)
#define GET_MEMBER_TRAMPOLINE(name) (void **)(&name##Class::name##_Actual)

#define DETOUR_CREATE_MEMBER(name, gamedata) CDetourManager::CreateDetour(GET_MEMBER_CALLBACK(name), GET_MEMBER_TRAMPOLINE(name), gamedata);
#define DETOUR_CREATE_STATIC(name, gamedata) CDetourManager::CreateDetour(GET_STATIC_CALLBACK(name), GET_STATIC_TRAMPOLINE(name), gamedata);

#define DETOUR_DESTROY_CLASS_MEMBER(className, fnName) \
	if (g_##className##__##fnName##Detour != nullptr) \
	{ \
		g_##className##__##fnName##Detour->Destroy(); \
		g_##className##__##fnName##Detour = nullptr; \
	} \

#define DETOUR_DECL_CLASS_MEMBER(className, fnName) \
	CDetour* g_##className##__##fnName##Detour = nullptr;

#define DETOUR_INIT_CLASS_MEMBER(className, fnName, gamedataKey) \
	g_##className##__##fnName##Detour = CDetourManager::CreateDetour((void*)GetCodeAddress(&className::fnName), nullptr, gamedataKey); \
	if (g_##className##__##fnName##Detour == nullptr) \
	{ \
		V_snprintf(error, maxlen, "Failed to setup " gamedataKey " detour"); \
		return false; \
	} \
	g_##className##__##fnName##Detour->EnableDetour(); \


class GenericClass {};
typedef void (GenericClass::* VoidFunc)();

inline void* GetCodeAddr(VoidFunc mfp)
{
	return *(void**)&mfp;
}

/**
 * Converts a member function pointer to a void pointer.
 * This relies on the assumption that the code address lies at mfp+0
 * This is the case for both g++ and later MSVC versions on non virtual functions but may be different for other compilers
 * Based on research by Don Clugston : http://www.codeproject.com/cpp/FastDelegate.asp
 */
#define GetCodeAddress(mfp) GetCodeAddr(reinterpret_cast<VoidFunc>(mfp))

class CDetourManager;

class CDetour
{
public:

	bool IsEnabled();

	/**
	 * These would be somewhat self-explanatory I hope
	 */
	void EnableDetour();
	void DisableDetour();

	void Destroy();

	friend class CDetourManager;

protected:
	CDetour(void* callbackfunction, void** trampoline, const char* signame);
	CDetour(void* callbackfunction, void** trampoline, void* pAddress);

	bool Init(ISourcePawnEngine* spengine, IGameConfig* gameconf);
private:

	/* These create/delete the allocated memory */
	bool CreateDetour();
	void DeleteDetour();

	bool enabled;
	bool detoured;

	patch_t detour_restore;
	/* Address of the detoured function */
	void* detour_address;
	/* Address of the allocated trampoline function */
	void* detour_trampoline;
	/* Address of the callback handler */
	void* detour_callback;
	/* The function pointer used to call our trampoline */
	void** trampoline;

	const char* signame;
	ISourcePawnEngine* spengine;
	IGameConfig* gameconf;
};

class CDetourManager
{
public:

	static void Init(ISourcePawnEngine* spengine, IGameConfig* gameconf);

	/**
	 * Creates a new detour
	 *
	 * @param callbackfunction			Void pointer to your detour callback function.
	 * @param trampoline				Address of the trampoline pointer
	 * @param signame					Section name containing a signature to fetch from the gamedata file.
	 * @return							A new CDetour pointer to control your detour.
	 *
	 * Example:
	 *
	 * CBaseServer::ConnectClient(netadr_s &, int, int, int, char  const*, char  const*, char  const*, int)
	 *
	 * Define a new class with the required function and a member function pointer to the same type:
	 *
	 * class CBaseServerDetour
	 * {
	 * public:
	 *		 bool ConnectClient(void *netaddr_s, int, int, int, char  const*, char  const*, char  const*, int);
	 *		 static bool (CBaseServerDetour::* ConnectClient_Actual)(void *netaddr_s, int, int, int, char  const*, char  const*, char  const*, int);
	 * }
	 *
	 *	void *callbackfunc = GetCodeAddress(&CBaseServerDetour::ConnectClient);
	 *	void **trampoline = (void **)(&CBaseServerDetour::ConnectClient_Actual);
	 *
	 * Creation:
	 * CDetourManager::CreateDetour(callbackfunc,  trampoline, "ConnectClient");
	 *
	 * Usage:
	 *
	 * CBaseServerDetour::ConnectClient(void *netaddr_s, int, int, int, char  const*, char  const*, char  const*, int)
	 * {
	 *			//pre hook code
	 *			bool result = (this->*ConnectClient_Actual)(netaddr_s, rest of params);
	 *			//post hook code
	 *			return result;
	 * }
	 *
	 * Note we changed the netadr_s reference into a void* to avoid needing to define the type
	 */
	static CDetour* CreateDetour(void* callbackfunction, void** trampoline, const char* signame);
	static CDetour* CreateDetour(void* callbackfunction, void** trampoline, void* pAddress);

	friend class CBlocker;
	friend class CDetour;

private:
	static ISourcePawnEngine* spengine;
	static IGameConfig* gameconf;
};

#endif // DETOURS_H_