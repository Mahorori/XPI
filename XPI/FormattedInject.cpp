#include "stdafx.h"

#include "FormattedInject.hpp"
#include "CInstanceManager.hpp"
#include "CMaplePacket.hpp"
#include "COutPacket.hpp"
#include "extvars.hpp"

namespace FormattedInject
{
	VOID DoInitPacket(COutPacket *oPacket, BOOL bHidden, WORD n)
	{
		if (!bHidden)
			COutPacket_constructor(oPacket, n);
		else
			COutPacket_constructor(oPacket, n);
	}

	VOID DoEncode1(COutPacket *oPacket, BOOL bHidden, BYTE n)
	{
		if (!bHidden)
			Encode1_Hook(oPacket, 0, n);
		else
			oPacket->Encode1(n);
	}

	VOID DoEncode2(COutPacket *oPacket, BOOL bHidden, WORD n)
	{
		if (!bHidden)
			Encode2_Hook(oPacket, 0, n);
		else
			oPacket->Encode2(n);
	}

	VOID DoEncode4(COutPacket *oPacket, BOOL bHidden, DWORD n)
	{
		if (!bHidden)
			Encode4_Hook(oPacket, 0, n);
		else
			oPacket->Encode4(n);
	}

	VOID DoEncode8(COutPacket *oPacket, BOOL bHidden, ULONGLONG n)
	{
		if (!bHidden)
			Encode8_Hook(oPacket, 0, n);
		else
			oPacket->Encode8(n);
	}

	VOID DoEncodeString(COutPacket *oPacket, BOOL bHidden, std::string& s)
	{
		if (!bHidden)
		{
			auto pckt = pInstances->Find(oPacket);
			if (pckt != NULL)
				pckt->AddString(s.c_str(), 0);
		}

		oPacket->Encode2(s.length());
		oPacket->EncodeBuffer((LPBYTE)s.c_str(), s.length());
	}

	VOID DoEncodeBuffer(COutPacket *oPacket, BOOL bHidden, std::vector<BYTE>& vb)
	{
		if (!bHidden)
			EncodeBuffer_Hook(oPacket, 0, &vb[0], vb.size());
		else
			oPacket->EncodeBuffer(&vb[0], vb.size());
	}
}