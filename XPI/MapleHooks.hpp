#pragma once

#include "CInPacket.hpp"
#include "COutPacket.hpp"
#include "CClientSocket.hpp"

#define GLOBAL_CClientSocket		0x00BE7914 // v83

enum OFFTYPE
{
	OFF_NONE = 0,
	OFF_ADD,
	OFF_SUB,
	OFF_PTR,
	OFF_JMP,
	OFF_CALL
};

typedef struct _MAPLE_FUNCTION
{
	LPCWSTR lpcwszName;
	PVOID   pTarget;
	PVOID	pAddress;
} MAPLE_FUNCTION, far *LPMAPLE_FUNCTION, near *PMAPLE_FUNCTION;

typedef struct _MAPLE_HOOK
{
	PVOID           pHook;
	MAPLE_FUNCTION  Function;
} MAPLE_HOOK, far *LPMAPLE_HOOK, near *PMAPLE_HOOK;

extern VOID(__thiscall *RemoveAll)(__in unsigned char **);
extern DWORD(__cdecl *innoHash)(__in char *pSrc, __in INT nLen, __inout DWORD *pdwKey);

const MAPLE_HOOK MapleHooks[] =
{
	// send functions
	{ Init_Hook, { L"COutPacket::Init", &_Init, (PVOID)0x006ECAA9 } }, // ok
	{ Encode1_Hook, { L"COutPacket::Encode1", &_Encode1, (PVOID)0x00406549 } }, // ok
	{ Encode2_Hook, { L"COutPacket::Encode2", &_Encode2, (PVOID)0x00427F74 } }, // ok
	{ Encode4_Hook, { L"COutPacket::Encode4", &_Encode4, (PVOID)0x004065A6 } }, // ok
	//{ Encode8_Hook, { L"COutPacket::Encode8", &_Encode8, (PVOID)0x496CA9 } },
	{ EncodeBuffer_Hook, { L"COutPacket::EncodeBuffer", &_EncodeBuffer, (PVOID)0x0046C00C } }, // ok
	{ EncodeStr_Hook, { L"COutPacket::EncodeStr", &_EncodeStr, (PVOID)0x0046F3CF } }, // ok
	{ MakeBufferList_Hook, { L"COutPacket::MakeBufferList", &_MakeBufferList, (PVOID)0x006ECB27 } }, // ok

	// recv functions
	{ Decode1_Hook, { L"CInPacket::Decode1", &_Decode1, (PVOID)0x004065F3 } }, // ok
	{ Decode2_Hook, { L"CInPacket::Decode2", &_Decode2, (PVOID)0x0042470C } }, // ok
	{ Decode4_Hook, { L"CInPacket::Decode4", &_Decode4, (PVOID)0x00406629 } }, // ok
	//{ Decode8_Hook, { L"CInPacket::Decode8", &_Decode8, (PVOID)0x496CA9 } },
	{ DecodeBuffer_Hook, { L"CInPacket::DecodeBuffer", &_DecodeBuffer, (PVOID)0x00432257 } }, // ok
	{ DecodeStr_Hook, { L"CInPacket::DecodeStr", &_DecodeStr, (PVOID)0x0046F30C } }, // ok
	{ ProcessPacket_Hook, { L"CClientSocket::ProcessPacket", &_ProcessPacket, (PVOID)0x004965F1 } }, // ok
	// { CChatSocket_ProcessPacket_Hook, { L"CChatSocket::ProcessPacket", &_CChatSocket_ProcessPacket, OFF_NONE, 0, L"56 57 8B 7C 24 0C 8B F1 8B CF E8 ?? ?? ?? ?? 0F B7 C0 48 83 F8 1B 77" } }
};

const MAPLE_FUNCTION MapleFunctions[] =
{
	{ L"COutPacket::COutPacket",	&COutPacket_constructor,	(PVOID)0x006EC9CE },
	{ L"ZArray::RemoveAll",			&RemoveAll,					(PVOID)0x00428CF1 },
	{ L"CIGCipher::innoHash",		&innoHash,					(PVOID)0x00A4A838 },
	{ L"ZSynchronizedHelper<ZFatalSection>::ZSynchronizedHelper<ZFatalSection>", &ZSynchronizedHelper_constructor, (PVOID)0x00403166 },
	{ L"CClientSocket::Flush",						&_Flush,					(PVOID)0x00496403 },
	//{ L"CSecurityClient::EncodeMemoryCheckResult",	&_EncodeMemoryCheckResult,	(PVOID)0x00A4A838 },
};