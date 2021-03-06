#ifndef _INCLUDE_SOURCEMOD_DETOURHELPERS_H_
#define _INCLUDE_SOURCEMOD_DETOURHELPERS_H_

#if defined PLATFORM_POSIX
#include <sys/mman.h>
#ifndef PAGE_SIZE
#define	PAGE_SIZE	4096
#endif
#define ALIGN(ar) ((long)ar & ~(PAGE_SIZE-1))
#define	PAGE_EXECUTE_READWRITE	PROT_READ|PROT_WRITE|PROT_EXEC
#endif

#include <jit/x86/x86_macros.h>

struct patch_t
{
	patch_t()
	{
		patch[0] = 0;
		bytes = 0;
	}
	unsigned char patch[20];
	size_t bytes;
};

inline void ProtectMemory(void *addr, int length, int prot)
{
#if defined PLATFORM_POSIX
	void *addr2 = (void *)ALIGN(addr);
	mprotect(addr2, sysconf(_SC_PAGESIZE), prot);
#elif defined PLATFORM_WINDOWS
	DWORD old_prot;
	VirtualProtect(addr, length, prot, &old_prot);
#endif
}

inline void SetMemPatchable(void *address, size_t size)
{
	ProtectMemory(address, (int)size, PAGE_EXECUTE_READWRITE);
}

inline void PatchRelJump32(unsigned char *target, void *callback)
{
	SetMemPatchable(target, 5);

	// jmp <32-bit displacement>
	target[0] = IA32_JMP_IMM32;
	*(int32_t *)(&target[1]) = int32_t((unsigned char *)callback - (target + 5));
}

inline void PatchAbsJump64(unsigned char *target, void *callback)
{
	int i = 0;
	SetMemPatchable(target, 14);
	
	// push <lower 32-bits>         ; allocates 64-bit stack space on x64
	// mov [rsp+4], <upper 32-bits> ; unnecessary if upper bits are 0
	// ret                          ; jump to address on stack
	target[i++] = IA32_PUSH_IMM32;
	*(int32_t *)(&target[i]) = int32_t(int64_t(callback));
	i += 4;
	if ((int64_t(callback) >> 32) != 0)
	{
		target[i++] = IA32_MOV_RM_IMM32;
		target[i++] = ia32_modrm(MOD_DISP8, 0, kREG_SIB);
		target[i++] = ia32_sib(NOSCALE, kREG_NOIDX, kREG_ESP);
		target[i++] = 0x04;
		*(int32_t *)(&target[i]) = (int64_t(callback) >> 32);
		i += 4;
	}
	target[i] = IA32_RET;
}

inline void DoGatePatch(unsigned char *target, void *callback)
{
#if defined(_WIN64) || defined(__x86_64__)
	int64_t diff = int64_t(callback) - (int64_t(target) + 5);
	int32_t upperBits = (diff >> 32);
	if (upperBits == 0 || upperBits == -1)
		PatchRelJump32(target, callback);
	else
		PatchAbsJump64(target, callback);
#else
	PatchRelJump32(target, callback);
#endif
}

inline void ApplyPatch(void *address, int offset, const patch_t *patch, patch_t *restore)
{
	ProtectMemory(address, 20, PAGE_EXECUTE_READWRITE);

	unsigned char *addr = (unsigned char *)address + offset;
	if (restore)
	{
		for (size_t i=0; i<patch->bytes; i++)
		{
			restore->patch[i] = addr[i];
		}
		restore->bytes = patch->bytes;
	}

	for (size_t i=0; i<patch->bytes; i++)
	{
		addr[i] = patch->patch[i];
	}
}

#endif //_INCLUDE_SOURCEMOD_DETOURHELPERS_H_