#include "stdafx.h"

#include "CInPacket.hpp"
#include "CClientSocket.hpp"
#include "CMaplePacket.hpp"
#include "XPIUtilities.hpp"
#include "CInstanceManager.hpp"
#include "extvars.hpp"
#include "MapleHooks.hpp"

#include <intrin.h>
#pragma intrinsic(_ReturnAddress)

BYTE(__thiscall *_Decode1)(__inout CInPacket *) = NULL;
WORD(__thiscall *_Decode2)(__inout CInPacket *) = NULL;
DWORD(__thiscall *_Decode4)(__inout CInPacket *) = NULL;
ULONGLONG(__thiscall *_Decode8)(__inout CInPacket *) = NULL;
LPCSTR *(__thiscall *_DecodeStr)(__inout CInPacket *, __out LPCSTR *result) = NULL;
VOID(__thiscall *_DecodeBuffer)(__inout CInPacket *, __out CONST LPVOID p, __in UINT uSize) = NULL;

BYTE __fastcall Decode1_Hook(__inout CInPacket *iPacket)
{
	std::shared_ptr<CMaplePacket> pckt;
	BYTE n = iPacket->Decode1();

	if (iPacket->m_nState == RS_COMPLETED)
	{
		if ((pckt = pInstances->Find(iPacket)) != NULL)
		{
			if (bLogging || pckt->GetState() & PACKET_INJECTED)
				pckt->Add1(n, _ReturnAddress());
		}
	}

	return n;
}

WORD __fastcall Decode2_Hook(__inout CInPacket *iPacket)
{
	std::shared_ptr<CMaplePacket> pckt;
	WORD n = iPacket->Decode2();

	if (iPacket->m_nState == RS_COMPLETED)
	{
		if ((pckt = pInstances->Find(iPacket)) == NULL && bLogging)
		{
			BOOL bBlock = IsOpcodeBlocked(n);

			if (bBlock)
				n = PACKET_BLANK_OPCODE;

			// create a new CMaplePacket if one does not already exist
			CMAPLEPACKETSTRUCT cmps;

			cmps.pInstance = iPacket;
			cmps.Direction = PACKET_RECV;
			cmps.ulState = bBlock ? PACKET_BLOCKED : 0;
			cmps.lpv = _ReturnAddress();

			pckt = std::make_shared<CMaplePacket>(&cmps);
			pInstances->Add(iPacket, pckt);
		}

		if (pckt != NULL)
		{
			if (bLogging || pckt->GetState() & PACKET_INJECTED)
				pckt->Add2(n, _ReturnAddress());
		}
	}
	return n;
}

DWORD __fastcall Decode4_Hook(__inout CInPacket *iPacket)
{
	std::shared_ptr<CMaplePacket> pckt;
	DWORD n = iPacket->Decode4();

	if (iPacket->m_nState == RS_COMPLETED)
	{
		if ((pckt = pInstances->Find(iPacket)) != NULL)
		{
			if (bLogging || pckt->GetState() & PACKET_INJECTED)
				pckt->Add4(n, _ReturnAddress());
		}
	}

	return n;
}

ULONGLONG __fastcall Decode8_Hook(__inout CInPacket *iPacket)
{
	std::shared_ptr<CMaplePacket> pckt;
	ULONGLONG n = iPacket->Decode8();

	if (iPacket->m_nState == RS_COMPLETED)
	{
		if ((pckt = pInstances->Find(iPacket)) != NULL)
		{
			if (bLogging || pckt->GetState() & PACKET_INJECTED)
				pckt->Add8(n, _ReturnAddress());
		}
	}

	return n;
}

LPCSTR * __fastcall DecodeStr_Hook(__inout CInPacket *iPacket, __in DWORD, __out LPCSTR *result)
{
	std::shared_ptr<CMaplePacket> pckt;
	LPCSTR *lplpcsz = iPacket->DecodeStr(result);

	if (iPacket->m_nState == RS_COMPLETED)
	{
		if ((pckt = pInstances->Find(iPacket)) != NULL)
		{
			if (bLogging || pckt->GetState() & PACKET_INJECTED)
				if (lplpcsz != NULL)
					if (*lplpcsz != NULL)
						pckt->AddString(*lplpcsz, _ReturnAddress());
		}
	}

	return lplpcsz;
}

VOID __fastcall DecodeBuffer_Hook(__inout CInPacket *iPacket, __in DWORD, __out_bcount(uLength) PBYTE pb, __in UINT uSize)
{
	std::shared_ptr<CMaplePacket> pckt;
	iPacket->DecodeBuffer(pb, uSize);

	if (iPacket->m_nState != RS_COMPLETED)
		return;

	if ((pckt = pInstances->Find(iPacket)) != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
			pckt->AddBuffer(pb, uSize, _ReturnAddress());
	}
}

//
CInPacket::~CInPacket()
{
	RemoveAll(&this->m_aRecvBuff);
}

BYTE CInPacket::Decode1()
{
	return _Decode1(this);
}

WORD CInPacket::Decode2()
{
	return _Decode2(this);
}

DWORD CInPacket::Decode4()
{
	return _Decode4(this);
}

ULONGLONG CInPacket::Decode8()
{
	return _Decode8(this);
}

LPCSTR * CInPacket::DecodeStr(__out LPCSTR *result)
{
	return _DecodeStr(this, result);
}

VOID CInPacket::DecodeBuffer(__out CONST LPVOID p, __in UINT uSize)
{
	return _DecodeBuffer(this, p, uSize);
}