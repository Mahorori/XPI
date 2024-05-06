#include "stdafx.h"

#include "COutPacket.hpp"
#include "CInstanceManager.hpp"
#include "CMaplePacket.hpp"
#include "XPIUtilities.hpp"
#include "extvars.hpp"
#include "CLog.hpp"
#include "CClientSocket.hpp"
#include "MapleHooks.hpp"

#include <intrin.h>
#pragma  intrinsic(_ReturnAddress)

//
VOID(__thiscall *_Init)(__out COutPacket *, __in LONG nType) = NULL;
VOID(__thiscall *_Encode1)(__inout COutPacket *, __in BYTE n) = NULL;
VOID(__thiscall *_Encode2)(__inout COutPacket *, __in WORD n) = NULL;
VOID(__thiscall *_Encode4)(__inout COutPacket *, __in DWORD n) = NULL;
VOID(__thiscall *_Encode8)(__inout COutPacket *, __in ULONGLONG n) = NULL;
VOID(__thiscall *_EncodeStr)(__inout COutPacket * , __in LPCSTR s) = NULL;
VOID(__thiscall *_EncodeBuffer)(__inout COutPacket *, __in CONST LPVOID p, __in UINT uSize) = NULL;
VOID(__thiscall *_MakeBufferList)(__in COutPacket *, LPVOID l, WORD uSeqBase, DWORD *puSeqKey, BOOL bEnc, DWORD dwKey) = NULL;

COutPacket *(__thiscall *COutPacket_constructor)(__out COutPacket *, __in LONG nType) = NULL;

VOID __fastcall Init_Hook(__inout COutPacket *oPacket, __in DWORD, __in LONG nType)
{
	BOOL bBlock;
	CMAPLEPACKETSTRUCT cmps;

	if (nType != INT_MAX && bLogging)
	{
		if (pInstances->Find(oPacket) == NULL)
		{
			bBlock = IsOpcodeBlocked((WORD)nType);
			if (bBlock)
				nType = PACKET_BLANK_OPCODE;

			cmps.pInstance = oPacket;
			cmps.Direction = PACKET_SEND;
			cmps.ulState = bBlock ? PACKET_BLOCKED : 0;
			cmps.lpv = _ReturnAddress();
			pInstances->Add(oPacket, std::make_shared<CMaplePacket>(&cmps));
		}
	}

	oPacket->Init(nType);
}

VOID __fastcall Encode1_Hook(__inout COutPacket *oPacket, __in DWORD, __in BYTE n)
{
	auto pckt = pInstances->Find(oPacket);
	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (oPacket->m_uOffset > pckt->GetSize())
				pckt->AddBuffer(&oPacket->m_aSendBuff[pckt->GetSize()], oPacket->m_uOffset - pckt->GetSize());

			pckt->Add1(n, _ReturnAddress());
		}
	}

	oPacket->Encode1(n);
}

VOID __fastcall Encode2_Hook(__inout COutPacket *oPacket, __in DWORD, __in WORD n)
{
	auto pckt = pInstances->Find(oPacket);
	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (oPacket->m_uOffset > pckt->GetSize())
				pckt->AddBuffer(&oPacket->m_aSendBuff[pckt->GetSize()], oPacket->m_uOffset - pckt->GetSize());

			pckt->Add2(n, _ReturnAddress());
		}
	}

	oPacket->Encode2(n);
}

VOID __fastcall Encode4_Hook(__inout COutPacket *oPacket, __in DWORD, __in DWORD n)
{
	auto pckt = pInstances->Find(oPacket);
	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (oPacket->m_uOffset > pckt->GetSize())
				pckt->AddBuffer(&oPacket->m_aSendBuff[pckt->GetSize()], oPacket->m_uOffset - pckt->GetSize());

			pckt->Add4(n, _ReturnAddress());
		}
	}

	oPacket->Encode4(n);
}

VOID __fastcall Encode8_Hook(__inout COutPacket *oPacket, __in DWORD, __in ULONGLONG n)
{
	auto pckt = pInstances->Find(oPacket);
	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (oPacket->m_uOffset > pckt->GetSize())
				pckt->AddBuffer(&oPacket->m_aSendBuff[pckt->GetSize()], oPacket->m_uOffset - pckt->GetSize());

			pckt->Add8(n, _ReturnAddress());
		}
	}

	oPacket->Encode8(n);
}

VOID __fastcall EncodeStr_Hook(__inout COutPacket *oPacket, __in DWORD, __in LPCSTR s)
{
	auto pckt = pInstances->Find(oPacket);
	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (oPacket->m_uOffset > pckt->GetSize())
				pckt->AddBuffer(&oPacket->m_aSendBuff[pckt->GetSize()], oPacket->m_uOffset - pckt->GetSize());

			pckt->AddString(s, _ReturnAddress());
		}
	}

	oPacket->EncodeStr(s);
}

VOID __fastcall EncodeBuffer_Hook(__inout COutPacket *oPacket, __in DWORD, __in CONST LPVOID p, __in UINT uSize)
{
	auto pckt = pInstances->Find(oPacket);
	if (pckt != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			if (oPacket->m_uOffset > pckt->GetSize())
				pckt->AddBuffer(&oPacket->m_aSendBuff[pckt->GetSize()], oPacket->m_uOffset - pckt->GetSize());

			pckt->AddBuffer((LPBYTE)p, uSize, _ReturnAddress());
		}
	}

	oPacket->EncodeBuffer(p, uSize);
}

VOID __fastcall MakeBufferList_Hook(__in COutPacket *oPacket, __in DWORD,
	LPVOID l, WORD uSeqBase, DWORD *puSeqKey, BOOL bEnc, DWORD dwKey)
{
	// ZList<ZRef<ZSocketBuffer> > *l

	auto pckt = pInstances->Find(oPacket);
	if (pckt != NULL)
	{
		// chat packet
		CClientSocket *pClientSocket = CClientSocket::GetInstance();
		if (!pClientSocket || &pClientSocket->m_lpSendBuff != l)
		{
			// chat socket right?
			pckt->SetState(pckt->GetState() | PACKET_CHAT);
		}

		HWND hWnd = GetXPIWindow();
		if (hWnd != NULL)
		{
			PostMessage(hWnd, WM_INJECTREADY, pClientSocket != NULL, 0);

			if (oPacket->m_uOffset > pckt->GetSize())
				pckt->AddBuffer(&oPacket->m_aSendBuff[pckt->GetSize()], oPacket->m_uOffset - pckt->GetSize());

			lPacketPool.push_back(pckt);
			PostMessage(hWnd, WM_ADDPACKET, 0, (LPARAM)pckt.get());
		}

		pInstances->Remove(oPacket);
	}

	oPacket->MakeBufferList(l, uSeqBase, puSeqKey, bEnc, dwKey);
}

// class funcs
COutPacket::COutPacket(__in LONG nType)
{
	COutPacket_constructor(this, nType);
}

COutPacket::~COutPacket()
{
	RemoveAll(&this->m_aSendBuff);
}

VOID COutPacket::Init(__in LONG nType)
{
	_Init(this, nType);
}

VOID COutPacket::Encode1(__in BYTE n)
{
	_Encode1(this, n);
}

VOID COutPacket::Encode2(__in WORD n)
{
	_Encode2(this, n);
}

VOID COutPacket::Encode4(__in DWORD n)
{
	_Encode4(this, n);
}

VOID COutPacket::Encode8(__in ULONGLONG n)
{
	_Encode8(this, n);
}

VOID COutPacket::EncodeStr(__in LPCSTR s)
{
	_EncodeStr(this, s);
}

VOID COutPacket::EncodeBuffer(__in CONST LPVOID p, __in UINT uSize)
{
	_EncodeBuffer(this, p, uSize);
}

VOID COutPacket::MakeBufferList(__inout LPVOID l, __in WORD uSeqBase, __in DWORD *puSeqKey, __in BOOL bEnc, __in DWORD dwKey)
{
	_MakeBufferList(this, l, uSeqBase, puSeqKey, bEnc, dwKey);
}