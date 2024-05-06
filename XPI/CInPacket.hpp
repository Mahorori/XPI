#pragma once

struct CClientSocket;

enum
{
	RS_HEADER = 0,
	RS_DATA,
	RS_COMPLETED
};

struct CInPacket
{
	BOOL m_bLoopback;
	INT m_nState;
	BYTE *m_aRecvBuff;
	UINT m_uLength;
	UINT m_uRawSeq;
	UINT m_uDataLen;
	UINT m_uOffset;

public:
	~CInPacket();

	BYTE Decode1();
	WORD Decode2();
	DWORD Decode4();
	ULONGLONG Decode8();
	LPCSTR * DecodeStr(__out LPCSTR *result);
	VOID DecodeBuffer(__out void *p, __in UINT uSize);
};

extern BYTE(__thiscall *_Decode1)(__inout CInPacket *);
extern WORD(__thiscall *_Decode2)(__inout CInPacket *);
extern DWORD(__thiscall *_Decode4)(__inout CInPacket *);
extern ULONGLONG(__thiscall *_Decode8)(__inout CInPacket *);
extern LPCSTR *(__thiscall *_DecodeStr)(__inout CInPacket *, __out LPCSTR *result);
extern VOID(__thiscall *_DecodeBuffer)(__inout CInPacket *, __out CONST LPVOID p, __in UINT uSize);

extern BYTE __fastcall Decode1_Hook(__inout CInPacket *iPacket);
extern WORD __fastcall Decode2_Hook(__inout CInPacket *iPacket);
extern DWORD __fastcall Decode4_Hook(__inout CInPacket *iPacket);
extern ULONGLONG __fastcall Decode8_Hook(__inout CInPacket *iPacket);
extern LPCSTR * __fastcall DecodeStr_Hook(__inout CInPacket *iPacket, __in DWORD, __out LPCSTR *result);
extern VOID __fastcall DecodeBuffer_Hook(__inout CInPacket *iPacket, __in DWORD, __out_bcount(uLength) PBYTE pb, __in UINT uSize);