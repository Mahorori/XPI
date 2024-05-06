#pragma once

struct COutPacket
{
	//COutPacket() = delete;
	COutPacket(const COutPacket&) = delete;

	BOOL m_bLoopback;
	BYTE *m_aSendBuff;
	UINT m_uOffset;
	BOOL m_bTypeHeader1Byte;
	BOOL m_bIsEncryptedByShanda;

public:
	COutPacket() = default;
	COutPacket(__in LONG nType);
	~COutPacket();

	VOID Init(__in LONG nType);
	VOID Encode1(__in BYTE n);
	VOID Encode2(__in WORD n);
	VOID Encode4(__in DWORD n);
	VOID Encode8(__in ULONGLONG n);
	VOID EncodeStr(__in LPCSTR s);
	VOID EncodeBuffer(__in const LPVOID p, __in UINT uSize);
	VOID MakeBufferList(__inout LPVOID l, __in WORD uSeqBase, __in DWORD *puSeqKey, __in BOOL bEnc, __in DWORD dwKey);
};

extern COutPacket *(__thiscall *COutPacket_constructor)(__out COutPacket *, __in LONG nType);

extern VOID(__thiscall *_Init)(__out COutPacket *, __in LONG nType);
extern VOID(__thiscall *_Encode1)(__inout COutPacket *, __in BYTE n);
extern VOID(__thiscall *_Encode2)(__inout COutPacket *, __in WORD n);
extern VOID(__thiscall *_Encode4)(__inout COutPacket *, __in DWORD n);
extern VOID(__thiscall *_Encode8)(__inout COutPacket *, __in ULONGLONG n);
extern VOID(__thiscall *_EncodeStr)(__inout COutPacket *, __in LPCSTR s);
extern VOID(__thiscall *_EncodeBuffer)(__inout COutPacket *, __in CONST LPVOID p, __in UINT uSize);
extern VOID(__thiscall *_MakeBufferList)(__in COutPacket *, LPVOID l, WORD uSeqBase, DWORD *puSeqKey, BOOL bEnc, DWORD dwKey);

extern VOID __fastcall Init_Hook(__inout COutPacket *oPacket, __in DWORD, __in LONG nType);
extern VOID __fastcall Encode1_Hook(__inout COutPacket *oPacket, __in DWORD, __in BYTE n);
extern VOID __fastcall Encode2_Hook(__inout COutPacket *oPacket, __in DWORD, __in WORD n);
extern VOID __fastcall Encode4_Hook(__inout COutPacket *oPacket, __in DWORD, __in DWORD n);
extern VOID __fastcall Encode8_Hook(__inout COutPacket *oPacket, __in DWORD, __in ULONGLONG n);
extern VOID __fastcall EncodeStr_Hook(__inout COutPacket *oPacket, __in DWORD, __in LPCSTR s);
extern VOID __fastcall EncodeBuffer_Hook(__inout COutPacket *oPacket, __in DWORD, __in const LPVOID p, __in UINT uSize);
extern VOID __fastcall MakeBufferList_Hook(__in COutPacket *oPacket, __in DWORD,
	LPVOID l, WORD uSeqBase, DWORD *puSeqKey, BOOL bEnc, DWORD dwKey);