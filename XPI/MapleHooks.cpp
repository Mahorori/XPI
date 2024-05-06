#include "stdafx.h"

#include "MapleHooks.hpp"
#include "COutPacket.hpp"

// ZArray<unsigned char>::RemoveAll
VOID(__thiscall *RemoveAll)(__in unsigned char **) = NULL;
DWORD(__cdecl *innoHash)(__in char *pSrc, __in INT nLen, __inout DWORD *pdwKey) = NULL;