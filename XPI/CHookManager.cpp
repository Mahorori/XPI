#include "stdafx.h"

#include "CHookManager.hpp"
#include "XPIUtilities.hpp"

#include <detours.h>
#pragma comment(lib, "detours.lib")

CHookManager::CHookManager()
{
	m_bEnabled = FALSE;
}

CHookManager::~CHookManager()
{
	Disable();
}

VOID CHookManager::Add(__in PVOID *ppv, __in PVOID pDetour)
{
	m_Hooks[ppv] = pDetour;

	if (Disable())
		Install();
}

VOID CHookManager::Remove(__in PVOID *ppv)
{
	m_Hooks.erase(ppv);

	if (Disable())
		Install();
}

BOOL CHookManager::Set(__in BOOL bInstall)
{
	LONG lError = NO_ERROR;

	if (m_bEnabled == bInstall || m_Hooks.empty())
		return FALSE;

	if (DetourTransactionBegin() != NO_ERROR)
		return FALSE;

	if (DetourUpdateThread(GetCurrentThread()) == NO_ERROR)
	{
		for(HOOK_MAP::iterator::value_type& i : m_Hooks)
		{
			if (bInstall)
				lError = DetourAttach(i.first, i.second);
			else
				lError = DetourDetach(i.first, i.second);
			if (lError != NO_ERROR)
				break;
		}
		if (lError == NO_ERROR)
		{
			if (DetourTransactionCommit() == NO_ERROR)
			{
				m_bEnabled = bInstall;
				return TRUE;
			}
		}
	}

	DetourTransactionAbort();
	return FALSE;
}

inline BOOL CHookManager::Install()
{
	return Set(TRUE);
}

inline BOOL CHookManager::Disable()
{
	return Set(FALSE);
}
