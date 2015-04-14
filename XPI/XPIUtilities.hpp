#ifndef XPI_UTILITIES_HPP_
#define XPI_UTILITIES_HPP_

#include <Windows.h>
#include <intrin.h>
#pragma  intrinsic(_ReturnAddress)

#include <vector>
#include <string>

#include <boost/foreach.hpp>

#ifndef foreach
#define foreach BOOST_FOREACH
#endif

// window messages
#define WM_INJECTREADY  (WM_USER + 10)
#define WM_ADDPACKET    (WM_USER + 11)
#define WM_CLEARPACKETS (WM_USER + 12)
#define WM_UPDATEOPCODE (WM_USER + 13)
#define WM_FORMATALIAS  (WM_USER + 14)

// member colors
#define MCLR_BYTE		RGB(131, 62, 0)
#define MCLR_WORD		RGB(98, 0, 0)
#define MCLR_DWORD		RGB(54, 0, 76)
#define MCLR_ULONGLONG  RGB(54, 0, 76) // same color ._.
#define MCLR_STRING		RGB(52, 73, 0)
#define MCLR_BUFFER		RGB(0, 37, 82)

#define NB_ERR  0xFF

BOOL StringToBuffer(__in std::wstring wstr, __inout std::vector<BYTE>* pvbBuffer);
BOOL StringToBuffer(__in std::string str, __inout std::vector<BYTE>* pvbBuffer);
VOID StringToWString(__in std::string str, __inout std::wstring &wstr);

BOOL IsOpcodeBlocked(__in WORD wOpcode);

HANDLE AddFontFromResource(__in HMODULE hModule, __in LPCWSTR lpcwszName);

VOID __MessageBox(_In_opt_ LPCWSTR lpText, ...);

// edit box filter look-up table
static const BOOL s_cbPlainMask[0x100] =
{
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x08
	TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x10
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x18
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x20
	TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x28
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x30
	TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, // 0x38
	TRUE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x40
	FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, // 0x48
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x50
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x58
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x60
	FALSE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, FALSE, // 0x68
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x70
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x78
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x80
	TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x88
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x90
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0x98
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0xA0
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0xA8
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0xB0
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0xB8
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0xC0
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0xC8
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0xD0
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0xD8
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0xE0
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0xE8
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0xF0
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, // 0xF8
	FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE  // 0xFF
};

INT MessageBoxIcon(__in HWND hwndOwner, __in LPCWSTR lpcwszText, __in LPCWSTR lpcwszCaption, __in UINT uType, __in HINSTANCE hInstance, __in LPCWSTR lpcwszIcon);

PVOID DumpMapleStory(__in PVOID pModuleBase, __in DWORD dwModuleSize);
VOID FreeMapleStoryDump(__in PVOID pBuffer);

template <class T>
inline T* GetClassInstance(__in HWND hWnd)
{
	return reinterpret_cast<T*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
}

#endif // XPI_UTILITIES_HPP_
