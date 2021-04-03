SMSDK = /home/sourcemod
MMSOURCE = /home/metamod
HL2SDK = /home/hl2sdk
#SERVER_ROOT=/game

#####################################
### EDIT BELOW FOR OTHER PROJECTS ###
#####################################

PROJECT = protectcmds
OBJECTS = PreConnectHook.cpp TakeDamageFix.cpp SourceTVFix.cpp GameUIFix.cpp FilterPacket.cpp BoneIndexByNameFix.cpp BlockCmds.cpp HashSteamID.cpp NetChanFix.cpp VoiceFix.cpp smsdk_ext.cpp extension.cpp util.cpp CDetour/detours.cpp asm/asm.c

##############################################
### CONFIGURE ANY OTHER FLAGS/OPTIONS HERE ###
##############################################

C_OPT_FLAGS = -DNDEBUG -O3 -funroll-loops -pipe -fno-strict-aliasing
C_DEBUG_FLAGS = -D_DEBUG -DDEBUG -g -ggdb3
C_GCC4_FLAGS = -fvisibility=hidden
CPP_GCC4_FLAGS = -fvisibility-inlines-hidden -std=c++14
CPP = gcc
CPP_OSX = clang

ENGINE_EXT = ext.1.ep1
CFLAGS += -DSOURCE_ENGINE=1

HL2PUB = $(HL2SDK)/public

INCLUDE += -I$(HL2SDK)/public/dlls
METAMOD = $(MMSOURCE)/core-legacy

OS := $(shell uname -s)

LIB_EXT = so
HL2LIB = $(HL2SDK)/linux_sdk

LIB_SUFFIX = _i486.$(LIB_EXT)

INCLUDE += -I. -I.. -Isdk -I$(SMSDK)/public -I$(SMSDK)/public/amtl -I$(SMSDK)/public/sourcepawn

ifeq "$(USEMETA)" "true"
	LINK_HL2 = $(HL2LIB)/tier1_i486.a $(LIB_PREFIX)vstdlib$(LIB_SUFFIX) $(LIB_PREFIX)tier0$(LIB_SUFFIX)

	LINK += $(LINK_HL2)

	INCLUDE += -I. -I.. -Isdk -I$(HL2PUB) -I$(HL2PUB)/engine -I$(HL2PUB)/steam -I$(HL2PUB)/mathlib -I$(HL2PUB)/tier0 \
	-I$(HL2PUB)/tier1 -I$(METAMOD) -I$(METAMOD)/sourcehook -I$(SMSDK)/public -I$(SMSDK)/public/extensions \
	-I$(SMSDK)/public/sourcepawn
	CFLAGS += -DSE_EPISODEONE=1 -DSE_DARKMESSIAH=2 -DSE_ORANGEBOX=3 -DSE_BLOODYGOODTIME=4 -DSE_EYE=5 \
		-DSE_ORANGEBOXVALVE=6 -DSE_LEFT4DEAD=7 -DSE_LEFT4DEAD2=8 -DSE_ALIENSWARM=9 \
		-DSE_PORTAL2=10 -DSE_CSGO=11 -DSE_CSS=12
endif

LINK += -m32 -lm -ldl -lstdc++ 

CFLAGS += -DPOSIX -Dstricmp=strcasecmp -D_stricmp=strcasecmp -D_strnicmp=strncasecmp -Dstrnicmp=strncasecmp \
	-D_snprintf=snprintf -D_vsnprintf=vsnprintf -D_alloca=alloca -Dstrcmpi=strcasecmp -DCOMPILER_GCC -Wall -Werror \
	-Wno-overloaded-virtual -Wno-switch -Wno-unused -msse -DSOURCEMOD_BUILD -DHAVE_STDINT_H -m32 -Wno-invalid-offsetof -Wno-write-strings
CPPFLAGS += -Wno-non-virtual-dtor -fno-exceptions -fno-rtti

################################################
### DO NOT EDIT BELOW HERE FOR MOST PROJECTS ###
################################################

BINARY = $(PROJECT).$(ENGINE_EXT).$(LIB_EXT)

ifeq "$(DEBUG)" "true"
	BIN_DIR = Debug
	CFLAGS += $(C_DEBUG_FLAGS)
else
	BIN_DIR = Release
	CFLAGS += $(C_OPT_FLAGS)
endif

ifeq "$(USEMETA)" "true"
	BIN_DIR := $(BIN_DIR).$(ENGINE)
endif

ifeq "$(OS)" "Darwin"
	CPP = $(CPP_OSX)
	LIB_EXT = dylib
	CFLAGS += -DOSX -D_OSX
	LINK += -dynamiclib -lstdc++ -mmacosx-version-min=10.5
else
	LIB_EXT = so
	CFLAGS += -D_LINUX
	LINK += -shared
endif

IS_CLANG := $(shell $(CPP) --version | head -1 | grep clang > /dev/null && echo "1" || echo "0")

ifeq "$(IS_CLANG)" "1"
	CPP_MAJOR := $(shell $(CPP) --version | grep clang | sed "s/.*version \([0-9]\)*\.[0-9]*.*/\1/")
	CPP_MINOR := $(shell $(CPP) --version | grep clang | sed "s/.*version [0-9]*\.\([0-9]\)*.*/\1/")
else
	CPP_MAJOR := $(shell $(CPP) -dumpversion >&1 | cut -b1)
	CPP_MINOR := $(shell $(CPP) -dumpversion >&1 | cut -b3)
endif

# If not clang
ifeq "$(IS_CLANG)" "0"
	CFLAGS += -mfpmath=sse
endif

# Clang || GCC >= 4
ifeq "$(shell expr $(IS_CLANG) \| $(CPP_MAJOR) \>= 4)" "1"
	CFLAGS += $(C_GCC4_FLAGS)
	CPPFLAGS += $(CPP_GCC4_FLAGS)
endif

# Clang >= 3 || GCC >= 4.7
ifeq "$(shell expr $(IS_CLANG) \& $(CPP_MAJOR) \>= 3 \| $(CPP_MAJOR) \>= 4 \& $(CPP_MINOR) \>= 7)" "1"
	CFLAGS += -Wno-delete-non-virtual-dtor
endif

# OS is Linux and not using clang
ifeq "$(shell expr $(OS) \= Linux \& $(IS_CLANG) \= 0)" "1"
	LINK += -static-libgcc
endif

OBJ_BIN := $(OBJECTS:%.cpp=$(BIN_DIR)/%.o)

# This will break if we include other Makefiles, but is fine for now. It allows
#  us to make a copy of this file that uses altered paths (ie. Makefile.mine)
#  or other changes without mucking up the original.
MAKEFILE_NAME := $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST))

$(BIN_DIR)/%.o: %.cpp
	$(CPP) $(INCLUDE) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

all:
	mkdir -p $(BIN_DIR)/CDetour
	if [ "$(USEMETA)" = "true" ]; then \
		ln -sf $(HL2LIB)/$(LIB_PREFIX)vstdlib$(LIB_SUFFIX); \
		ln -sf $(HL2LIB)/$(LIB_PREFIX)tier0$(LIB_SUFFIX); \
	fi
	$(MAKE) -f $(MAKEFILE_NAME) extension
	#mv $(BIN_DIR)/$(BINARY) $(SERVER_ROOT)/cstrike/addons/sourcemod/extensions/$(BINARY)

extension: $(OBJ_BIN)
	$(CPP) $(INCLUDE) $(OBJ_BIN) $(LINK) -o $(BIN_DIR)/$(BINARY)

debug:
	$(MAKE) -f $(MAKEFILE_NAME) all DEBUG=true

default: all

clean:
	rm -rf $(BIN_DIR)/*.o
	rm -rf $(BIN_DIR)/CDetour/*.o
	rm -rf $(BIN_DIR)/$(BINARY)

