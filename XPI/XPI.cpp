#include "stdafx.h"

#include "XPI.hpp"

#include <DbgHelp.h>
#include <Psapi.h>
#include "MapleHooks.hpp"
#include "CLog.hpp"
#include "CInstanceManager.hpp"
#include "CHookManager.hpp"
#include "CMainWindow.hpp"
#include "CResourceString.hpp"
#include "XPIUtilities.hpp"
#include "XPIConfig.h"
#include "extvars.hpp"
#include "resource.h"

#pragma  comment(lib, "dbghelp")
#pragma  comment(lib, "psapi")

_Success_(return) BOOL GetModuleSize(__in HMODULE hModule, __out LPVOID *lplpBase, __out LPDWORD lpdwSize)
{
	MODULEINFO  ModuleInfo;
	PIMAGE_NT_HEADERS pImageNtHeaders;

	if (hModule == GetModuleHandle(NULL))
	{
		pImageNtHeaders = ImageNtHeader((PVOID)hModule);
		if (pImageNtHeaders == NULL)
			return FALSE;

		*lplpBase = (LPVOID)hModule;
		*lpdwSize = pImageNtHeaders->OptionalHeader.SizeOfImage;
	}
	else
	{
		if (!GetModuleInformation(GetCurrentProcess(), hModule, &ModuleInfo, sizeof(MODULEINFO)))
			return FALSE;

		*lplpBase = ModuleInfo.lpBaseOfDll;
		*lpdwSize = ModuleInfo.SizeOfImage;
	}

	return TRUE;
}

BOOL XPIAPI LocateFunctionSignatures(__in PVOID pBase, __in DWORD dwSize)
{
	PVOID pTarget;

#ifdef _DEBUG
	pLog->Write(L"Searching for function signatures...");
#endif

	for (const MAPLE_HOOK &i : MapleHooks)
	{
		pTarget = i.Function.pAddress;
		*(PVOID*)i.Function.pTarget = pTarget;
	}

	for (const MAPLE_FUNCTION &i : MapleFunctions)
	{
		pTarget = i.pAddress;
		*(PVOID*)i.pTarget = pTarget;
	}

	return TRUE;
}

DWORD XPIAPI InitializeXPI(__in HINSTANCE hInstance)
{
#ifdef _DEBUG
	pLog = new CLog(L"XPI");
	pLog->Write(L"Initializing...");
#endif

	if (!GetModuleSize(GetModuleHandle(NULL), &pMapleBase, &dwMapleSize))
	{
#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR, L"Couldn't get target module base & size, aborting!");
#endif
		FreeLibraryAndExitThread(hInstance, EXIT_FAILURE);
	}

#ifdef _DEBUG
	pLog->Write(LOG_WF_DEBUG, L"Target module base = 0x%p.", pMapleBase);
	pLog->Write(LOG_WF_DEBUG, L"Target module size = 0x%08X.", dwMapleSize);
#endif

	if (!LocateFunctionSignatures(pMapleBase, dwMapleSize))
		FreeLibraryAndExitThread(hInstance, EXIT_FAILURE);

	pInstances = new CInstanceManager;
	pHookManager = new CHookManager;

	for (const MAPLE_HOOK& i : MapleHooks)
		pHookManager->Add((PVOID*)i.Function.pTarget, i.pHook);

#ifdef _DEBUG
	pLog->Write(LOG_WF_DEBUG, L"Installing hooks.");
#endif

	if (pHookManager->Install())
	{
#ifdef _DEBUG
		pLog->Write(L"Hooks installed & initialized correctly!");
#endif

		pStrings = new CResourceString(hInstance);
		pOpcodeInfo = new OPCODE_MAP();

		if (!LoadXPIConfig(XPI_CONFIG_FILE))
		{
#ifdef _DEBUG
			pLog->Write(L"XPI configuration file not found.");
#endif
		}

		DisableThreadLibraryCalls(hInstance);

		DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_XPI), NULL, CMainWindow::DialogProc, (LPARAM)hInstance);
		if (GetLastError() == ERROR_SUCCESS)
		{
#ifdef _DEBUG
			pLog->Write(L"Saving XPI configuration...");
#endif

			if (!SaveXPIConfig(XPI_CONFIG_FILE))
			{
#ifdef _DEBUG
				pLog->Write(L"Error writing configuration file!");
#endif
			}

			FreeLibraryAndExitThread(hInstance, EXIT_SUCCESS);
		}
		else
			FreeLibraryAndExitThread(hInstance, EXIT_FAILURE);
	}
	else
	{
#ifdef _DEBUG
		pLog->Write(LOG_WF_ERROR | LOG_WF_ECHODEBUG, L"Couldn't install hooks, aborting!");
#endif

		FreeLibraryAndExitThread(hInstance, EXIT_FAILURE);
	}
}

VOID XPIAPI DestroyXPI()
{
#ifdef _DEBUG
	if (pLog != NULL)
	{
		pLog->Write(L"Destroying XPI...");
		delete pLog;
	}
#endif

	if (pHookManager != NULL)
		delete pHookManager;

	if (pInstances != NULL)
		delete pInstances;

	if (pStrings != NULL)
		delete pStrings;

	lPacketPool.clear();

	if (pOpcodeInfo != NULL)
		delete pOpcodeInfo;
}

BOOL APIENTRY DllMain(__in HINSTANCE hInstance, __in DWORD fdwReason, __reserved LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	{
		HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InitializeXPI, (LPVOID)hInstance, 0, NULL);
		if (hThread != NULL)
			break;
		else
			return FALSE;
	}

	case DLL_PROCESS_DETACH:
		DestroyXPI();
		break;
	}

	return TRUE;
}
