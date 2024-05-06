#pragma once

#include "CInPacket.hpp"

#define WM_CLIENTSOCKET (WM_USER + 1)
#define PACKET_BLANK_OPCODE 0xFFFF

#ifndef Padding
#define Padding(x) struct { unsigned char __padding##x[(x)]; };
#endif

typedef LPVOID ZSocketBuffer, ZInetAddr, ZFileLine;
struct COutPacket;

struct ZSocketBase
{
	unsigned int _m_hSocket;
};

struct ZFatalSectionData
{
	void *_m_pTIB;
	int _m_nRef;
};

struct ZFatalSection
{
	ZFatalSectionData ZFatalSectionData;
};

template<class T>
struct ZSynchronizedHelper
{
	T *m_pLock;

public:
	explicit ZSynchronizedHelper(ZFatalSection *lock, ZFileLine *pFL);
	~ZSynchronizedHelper();
};

template < typename T >
struct ZList
{
	void *vfptr;
	void *gap;
	unsigned int _m_uCount;
	T *_m_pHead;
	T *_m_pTail;
};

struct CClientSocket
{
	struct CONNECTCONTEXT
	{
		ZList<ZInetAddr> lAddr;
		void *posList;
		BOOL bLogin;
	};

	union
	{
		struct
		{
			void *vfptr;
			HWND m_hWnd;
			ZSocketBase m_sock;
			CONNECTCONTEXT m_ctxConnect;
		};
		struct
		{
			Padding(0x50);
			union
			{
				// v83 +0x50
				LPVOID m_lpSendBuff; // ZList<ZRef<ZSocketBuffer>> m_lpSendBuff;
				Padding(0x10);
			};
		};
		struct
		{
			Padding(0x0000007C);		// v83 +0x7C
			ZFatalSection m_lockSend;
			DWORD m_uSeqSnd;			// v83 +0x84
			DWORD m_uSeqRcv;
		};
	};

public:
	static CClientSocket *GetInstance();

	void ProcessPacket(__in CInPacket *iPacket);
	void SendPacket(__in COutPacket *oPacket);
	void Flush();
};

extern ZSynchronizedHelper<ZFatalSection> *(__thiscall *ZSynchronizedHelper_constructor)(ZSynchronizedHelper<ZFatalSection> *, ZFatalSection *lock, ZFileLine *pFL);
extern VOID(__thiscall *_ProcessPacket)(__in CClientSocket *, __in CInPacket* pPacket);

extern VOID __fastcall ProcessPacket_Hook(__in CClientSocket *pSocket, __in DWORD, __in CInPacket *iPacket);

typedef void(__thiscall *FlushT)(CClientSocket *);
extern FlushT _Flush;