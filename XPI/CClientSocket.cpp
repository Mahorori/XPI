#include "stdafx.h"

#include "CClientSocket.hpp"
#include "COutPacket.hpp"
#include "extvars.hpp"
#include "MapleHooks.hpp"
#include "CInstanceManager.hpp"
#include "XPIUtilities.hpp"

ZSynchronizedHelper<ZFatalSection> *(__thiscall *ZSynchronizedHelper_constructor)(ZSynchronizedHelper<ZFatalSection> *, ZFatalSection *lock, ZFileLine *pFL) = NULL;
VOID(__thiscall *_ProcessPacket)(__in CClientSocket *, __in CInPacket* iPacket) = NULL;
FlushT _Flush = NULL;

ZSynchronizedHelper<ZFatalSection>::ZSynchronizedHelper(ZFatalSection *lock, ZFileLine *pFL)
{
	ZSynchronizedHelper_constructor(this, lock, pFL);
}

ZSynchronizedHelper<ZFatalSection>::~ZSynchronizedHelper()
{
	this->m_pLock->ZFatalSectionData._m_nRef--;
	if (this->m_pLock->ZFatalSectionData._m_nRef == 0)
		this->m_pLock->ZFatalSectionData._m_pTIB = NULL;
}

void __fastcall ProcessPacket_Hook(__in CClientSocket *pSocket, __in DWORD, __in CInPacket *iPacket)
{
	std::shared_ptr<CMaplePacket> pckt;
	pSocket->ProcessPacket(iPacket);

	if ((pckt = pInstances->Find(iPacket)) != NULL)
	{
		if (bLogging || pckt->GetState() & PACKET_INJECTED)
		{
			HWND hWnd = GetXPIWindow();
			if (hWnd != NULL)
			{
				lPacketPool.push_back(pckt);
				PostMessage(hWnd, WM_ADDPACKET, 0, (LPARAM)pckt.get());
			}
			pInstances->Remove(iPacket);
		}
	}
}

//
CClientSocket *CClientSocket::GetInstance()
{
	CClientSocket **ppInstance = reinterpret_cast<CClientSocket**>(GLOBAL_CClientSocket);
	return *ppInstance;
}

void CClientSocket::ProcessPacket(__in CInPacket *iPacket)
{
	_ProcessPacket(this, iPacket);
}

void CClientSocket::SendPacket(__in COutPacket *oPacket)
{
	ZSynchronizedHelper<ZFatalSection> _sync(&this->m_lockSend, NULL);
	if (this->m_sock._m_hSocket != NULL &&
		this->m_sock._m_hSocket != -1 &&
		this->m_ctxConnect.lAddr._m_uCount == 0)
	{
		oPacket->MakeBufferList(&this->m_lpSendBuff, uSeqBase, &this->m_uSeqSnd, TRUE, this->m_uSeqSnd);
		this->m_uSeqSnd = innoHash((char*)&this->m_uSeqSnd, sizeof(DWORD), NULL);
		this->Flush();
	}
}

void CClientSocket::Flush()
{
	_Flush(this);
}