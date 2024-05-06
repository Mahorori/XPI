#ifndef CSPAM_PACKET_HPP_
#define CSPAM_PACKET_HPP_

#include "CMaplePacket.hpp"
#include "CMainWindow.hpp"

#define WM_DONESPAMMING     (WM_USER + 150)
#define WM_UPDATESPAMSTATUS (WM_USER + 151)

struct CInPacket;
struct COutPacket;

// initializer structure (passed as LPARAM)
typedef struct _SPAMPACKET
{
	LPCWSTR           lpcwszPacket;
	BOOL              bFormatted;
	PACKET_DIRECTION  Direction;
	PXPIGUI           pXPIGUI;
} SPAMPACKET, far *LPSPAMPACKET, near *PSPAMPACKET;

// initializer structure for thread (passed as LPARAM)
typedef struct _SPAMPACKETTHREAD
{
	HWND				hDialog;
	ULONG				ulRepeat;
	ULONG				ulInterval;
	BOOL				bShowPacket;
	PACKET_DIRECTION	Direction;
	PULONGLONG			pullCount;
	CInPacket			*iPacket;
	COutPacket			*oPacket;
} SPAMPACKETTHREAD, far *LPSPAMPACKETTHREAD, near *PSPAMPACKETTHREAD;

class CSpamPacket
{
private:
	// window handle
	HWND              m_hDialog;
	// XPI GUI
	PXPIGUI           m_pXPIGUI;
	// show packet in listviews?
	BOOL              m_bShowPacket;
	// is the input formatted?
	BOOL              m_bFormatted;
	// packet direction
	PACKET_DIRECTION  m_Direction;
	// spam thread
	HANDLE            m_hSpamThread;
	// counter for spammed packets
	ULONGLONG         m_ullCount;
	// packet structure for outgoing spamming
	COutPacket *m_oPacket;
	// packet structure for incoming spamming
	CInPacket *m_iPacket;
	// temporary holder for the transferred string
	LPCWSTR           m_lpcwszBuffer;
	// status bar indices
	enum
	{
		SB_SESS = 0,
		SB_CURR
	};
public:
	CSpamPacket(__in HWND hDialog, __in PSPAMPACKET pSpamPacket);
	~CSpamPacket();
	BOOL OnCreate();
	VOID WarnReceive();
	VOID DrawFormatted(__in LPDRAWITEMSTRUCT lpdis, __in BOOL bFormatted);
	VOID SetFormatted(BOOL bFormatted);
	VOID OnRepeatChange();
	VOID OnIntervalChange();
	VOID ChangeInjectDirection();
	BOOL CreateUnformattedPacket();
	BOOL CreateFormattedPacket();
	VOID OnInjectClick();
	static DWORD WINAPI SpamThread(__in LPVOID lpParameter);
	VOID OnDoneSpamming();
	VOID UpdateStatus(__in DWORD dwAt, __in DWORD dwTotal);
	VOID EnableInput(__in BOOL bEnable);
	static INT_PTR CALLBACK PlainMaskedEdit(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam);
	static INT_PTR CALLBACK DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam);
};

#endif // CSPAM_PACKET_HPP_