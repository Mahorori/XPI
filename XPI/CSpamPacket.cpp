#include "stdafx.h"

#include "CSpamPacket.hpp"

#include <windowsx.h>
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
#include <intrin.h> // _ReturnAddress

#include "CMaplePacket.hpp"
#include "CResourceString.hpp"
#include "CInstanceManager.hpp"
#include "FormattedInject.hpp"
#include "XPIUtilities.hpp"
#include "resource.h"
#include "extvars.hpp"
#include "CInPacket.hpp"
#include "COutPacket.hpp"
#include "CClientSocket.hpp"

CSpamPacket::CSpamPacket(__in HWND hDialog, __in PSPAMPACKET pSpamPacket)
{
	// initialize class data
	m_hDialog = hDialog;
	m_pXPIGUI = pSpamPacket->pXPIGUI;
	m_bFormatted = pSpamPacket->bFormatted;
	m_Direction = pSpamPacket->Direction;
	// release this after initializing edit control text
	m_lpcwszBuffer = pSpamPacket->lpcwszPacket;
	m_hSpamThread = NULL;
	m_ullCount = 0;
	m_bShowPacket = TRUE;

	// done with transferred data
	delete pSpamPacket;

	// set window data to hold class instance
	SetWindowLongPtrW(m_hDialog, GWLP_USERDATA, (LONG_PTR)this);
}

CSpamPacket::~CSpamPacket()
{
	// this should only be necessary if an error occurs in OnCreate
	if (m_lpcwszBuffer != NULL)
		delete m_lpcwszBuffer;

	// if the thread is still running, end it
	if (m_hSpamThread != NULL)
		TerminateThread(m_hSpamThread, EXIT_SUCCESS);
}

BOOL CSpamPacket::OnCreate()
{
	// main title
	if (m_bFormatted)
		SetWindowText(m_hDialog, pStrings->Get(IDS_SPAM_TITLE_FORMAT).c_str());
	else
		SetWindowText(m_hDialog, pStrings->Get(IDS_SPAM_TITLE_PLAIN).c_str());

	// main icon
	SendMessageW(m_hDialog, WM_SETICON, ICON_BIG, (LPARAM)m_pXPIGUI->hIcon);
	SendMessageW(m_hDialog, WM_SETICON, ICON_SMALL, (LPARAM)m_pXPIGUI->hIconSmall);

	// status bar
	HWND hStatus = GetDlgItem(m_hDialog, IDC_SPAMSTATUS);

	INT nStatusWidths[] = { 160, -1 };

	SendMessageW(hStatus, SB_SETPARTS, (WPARAM)_countof(nStatusWidths), (LPARAM)nStatusWidths);
	SendMessageW(hStatus, SB_SETTEXT, (WPARAM)SB_SESS, (LPARAM)(pStrings->Get(IDS_SPAM_SESSION) + L"0").c_str());
	SendMessageW(hStatus, SB_SETTEXT, (WPARAM)SB_CURR, (LPARAM)pStrings->Get(IDS_IDLE).c_str());

	// edit control
	HWND hEdit = GetDlgItem(m_hDialog, IDC_SPAMEDIT);

	SendMessageW(hEdit, WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);

	if (m_bFormatted)
		Edit_SetCueBannerText(hEdit, pStrings->Get(IDS_FORMAT_INJECT_CUE).c_str());
	else
		Edit_SetCueBannerText(hEdit, pStrings->Get(IDS_PLAIN_INJECT_CUE).c_str());

	Edit_SetText(hEdit, m_lpcwszBuffer);

	if (m_lpcwszBuffer != NULL)
	{
		// pointer can be freed now
		delete m_lpcwszBuffer;
		m_lpcwszBuffer = NULL;
	}

	Edit_LimitText(hEdit, 0);

	// edit control mask (for plain data)
	SetWindowLongPtrW(hEdit, GWLP_USERDATA, (LONG_PTR)GetWindowLongPtrW(hEdit, GWLP_WNDPROC));
	if (!m_bFormatted)
		SetWindowLongPtrW(hEdit, GWLP_WNDPROC, (LONG_PTR)CSpamPacket::PlainMaskedEdit);

	// formatted icons
	ShowWindow(GetDlgItem(m_hDialog, IDC_SPAMFORMATTED), m_bFormatted ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(m_hDialog, IDC_SPAMPLAIN), m_bFormatted ? SW_HIDE : SW_SHOW);

	// pre captions
	SetWindowText(GetDlgItem(m_hDialog, IDC_SPAMREPEATLABEL), pStrings->Get(IDS_SPAM_REPEAT).c_str());
	SetWindowText(GetDlgItem(m_hDialog, IDC_SPAMINTERVALLABEL), pStrings->Get(IDS_SPAM_INTERVAL).c_str());

	// set default values for edit boxes
	HWND hRepeatEdit = GetDlgItem(m_hDialog, IDC_SPAMREPEATEDIT);
	SendMessageW(hRepeatEdit, WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);
	Edit_SetText(hRepeatEdit, L"5");
	Edit_LimitText(hRepeatEdit, 6);

	HWND hIntervalEdit = GetDlgItem(m_hDialog, IDC_SPAMINTERVALEDIT);
	SendMessageW(hIntervalEdit, WM_SETFONT, (WPARAM)m_pXPIGUI->hFont, TRUE);
	Edit_SetText(hIntervalEdit, L"250");
	Edit_LimitText(hIntervalEdit, 6);

	// up/down controls
	HWND hRepeatUpDown = GetDlgItem(m_hDialog, IDC_SPAMREPEATUPDOWN);
	SendMessageW(hRepeatUpDown, UDM_SETBUDDY, (WPARAM)hRepeatEdit, 0);
	SendMessageW(hRepeatUpDown, UDM_SETRANGE32, (WPARAM)0, (LPARAM)999999);
	SendMessageW(hRepeatUpDown, UDM_SETPOS32, 0, (LPARAM)5);
	SendMessageW(hRepeatUpDown, UDM_SETUNICODEFORMAT, (WPARAM)TRUE, 0);
	SendMessageW(hRepeatUpDown, UDM_SETBASE, (WPARAM)10, 0);

	HWND hIntervalUpDown = GetDlgItem(m_hDialog, IDC_SPAMINTERVALUPDOWN);
	SendMessageW(hIntervalUpDown, UDM_SETBUDDY, (WPARAM)hIntervalEdit, 0);
	SendMessageW(hIntervalUpDown, UDM_SETRANGE32, (WPARAM)0, (LPARAM)999999);
	SendMessageW(hIntervalUpDown, UDM_SETPOS32, 0, (LPARAM)250);
	SendMessageW(hIntervalUpDown, UDM_SETUNICODEFORMAT, (WPARAM)TRUE, 0);
	SendMessageW(hIntervalUpDown, UDM_SETBASE, (WPARAM)10, 0);

	// post captions
	SetWindowText(GetDlgItem(m_hDialog, IDC_SPAMREPEATPOST), pStrings->Get(IDS_SPAM_REPEAT_POST).c_str());
	SetWindowText(GetDlgItem(m_hDialog, IDC_SPAMINTERVALPOST), pStrings->Get(IDS_SPAM_INTERVAL_POST).c_str());

	// check box
	Button_SetText(GetDlgItem(m_hDialog, IDC_SPAMSHOWCHECK), pStrings->Get(IDS_SPAM_CHECK_TEXT).c_str());
	if (m_Direction == PACKET_RECV)
		Button_Enable(GetDlgItem(m_hDialog, IDC_SPAMSHOWCHECK), FALSE);

	// inject button
	if (m_Direction == PACKET_SEND)
		SendMessageW(GetDlgItem(m_hDialog, IDC_SPAMINJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_pXPIGUI->hInjectOut);
	else // PACKET_RECV
		SendMessageW(GetDlgItem(m_hDialog, IDC_SPAMINJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_pXPIGUI->hInjectIn);

	// tooltips
	HWND hIconTips = CreateWindowEx(0, TOOLTIPS_CLASS, L"", TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_hDialog, 0, m_pXPIGUI->hInstance, NULL);
	if (hIconTips == NULL)
		return FALSE;

	SetWindowPos(hIconTips, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	TOOLINFOW ti = { sizeof(ti) };
	/***/
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	ti.hwnd = m_hDialog;

	std::wstring wstrTemp;

	wstrTemp = pStrings->Get(IDS_SWITCH_PLAIN);
	ti.uId = (UINT_PTR)GetDlgItem(m_hDialog, IDC_SPAMFORMATTED);
	ti.lpszText = (LPWSTR)wstrTemp.c_str();
	SendMessage(hIconTips, TTM_ADDTOOL, 0, (LPARAM)&ti);

	wstrTemp = pStrings->Get(IDS_SWITCH_FORMATTED);
	ti.uId = (UINT_PTR)GetDlgItem(m_hDialog, IDC_SPAMPLAIN);
	ti.lpszText = (LPWSTR)wstrTemp.c_str();
	SendMessage(hIconTips, TTM_ADDTOOL, 0, (LPARAM)&ti);

	return TRUE;
}

INT_PTR CALLBACK CSpamPacket::PlainMaskedEdit(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	WNDPROC wpOld = (WNDPROC)GetWindowLongPtrW(hDialog, GWLP_USERDATA);

	switch (uMessage)
	{
	case WM_CHAR:
		if (GetAsyncKeyState(VK_CONTROL))
			break;
		if (s_cbPlainMask[wParam & 0xFF] == FALSE)
			return FALSE;
		wParam = toupper(wParam);
		break;
	}

	return CallWindowProc(wpOld, hDialog, uMessage, wParam, lParam);
}

VOID CSpamPacket::WarnReceive()
{
	EDITBALLOONTIP  TipError;
	std::wstring    wstrTitle = pStrings->Get(IDS_POTENTIAL_DANGER);
	std::wstring    wstrText = pStrings->Get(IDS_WARN_RECEIVE_MSG);

	TipError.cbStruct = sizeof(EDITBALLOONTIP);
	TipError.ttiIcon = TTI_WARNING;
	TipError.pszTitle = wstrTitle.c_str();
	TipError.pszText = wstrText.c_str();
	Edit_ShowBalloonTip(GetDlgItem(m_hDialog, IDC_SPAMINTERVALEDIT), &TipError);
}

VOID CSpamPacket::DrawFormatted(__in LPDRAWITEMSTRUCT lpdis, __in BOOL bFormatted)
{
	ImageList_Draw(m_pXPIGUI->hFormattedImageList, bFormatted ? XPI_FILI_FORMATTED : XPI_FILI_PLAIN, lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, ILD_TRANSPARENT);
}

VOID CSpamPacket::SetFormatted(BOOL bFormatted)
{
	// don't change the formatting setting if the spam packet thread is running
	if (m_hSpamThread != NULL)
		return;

	m_bFormatted = bFormatted;

	HWND hEdit = GetDlgItem(m_hDialog, IDC_SPAMEDIT);

	if (bFormatted)
	{
		// get rid of mask
		SetWindowLongPtrW(hEdit, GWLP_WNDPROC, (LONG_PTR)GetWindowLongPtrW(hEdit, GWLP_USERDATA));
		SetWindowText(m_hDialog, pStrings->Get(IDS_SPAM_TITLE_FORMAT).c_str());
		Edit_SetCueBannerText(hEdit, pStrings->Get(IDS_FORMAT_INJECT_CUE).c_str());
	}
	else
	{
		// replace mask
		SetWindowLongPtrW(hEdit, GWLP_WNDPROC, (LONG_PTR)CSpamPacket::PlainMaskedEdit);
		SetWindowText(m_hDialog, pStrings->Get(IDS_SPAM_TITLE_PLAIN).c_str());
		Edit_SetCueBannerText(hEdit, pStrings->Get(IDS_PLAIN_INJECT_CUE).c_str());
	}

	// toggle icons
	ShowWindow(GetDlgItem(m_hDialog, IDC_SPAMFORMATTED), bFormatted ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(m_hDialog, IDC_SPAMPLAIN), bFormatted ? SW_HIDE : SW_SHOW);

	// clear any text
	Edit_SetText(hEdit, NULL);
}

VOID CSpamPacket::OnRepeatChange()
{
	WCHAR   wszTemp[16];
	LPWSTR  lpwszEnd;
	ULONG   ulResult;

	Edit_GetText(GetDlgItem(m_hDialog, IDC_SPAMREPEATEDIT), wszTemp, _countof(wszTemp));
	ulResult = wcstoul(wszTemp, &lpwszEnd, 10);

	if (ulResult == 0)
	{
		Edit_SetText(GetDlgItem(m_hDialog, IDC_SPAMREPEATEDIT), L"0");
		SendMessageW(GetDlgItem(m_hDialog, IDC_SPAMREPEATUPDOWN), UDM_SETPOS32, 0, (LPARAM)0);
	}
}

VOID CSpamPacket::OnIntervalChange()
{
	WCHAR   wszTemp[16];
	LPWSTR  lpwszEnd;
	ULONG   ulResult;

	Edit_GetText(GetDlgItem(m_hDialog, IDC_SPAMINTERVALEDIT), wszTemp, _countof(wszTemp));
	ulResult = wcstoul(wszTemp, &lpwszEnd, 10);

	if (ulResult == 0)
	{
		Edit_SetText(GetDlgItem(m_hDialog, IDC_SPAMINTERVALEDIT), L"0");
		SendMessageW(GetDlgItem(m_hDialog, IDC_SPAMINTERVALUPDOWN), UDM_SETPOS32, 0, (LPARAM)0);
		if (m_Direction == PACKET_SEND)
		{
			Button_Enable(GetDlgItem(m_hDialog, IDC_SPAMSHOWCHECK), FALSE);
			Button_SetCheck(GetDlgItem(m_hDialog, IDC_SPAMSHOWCHECK), BST_CHECKED);
		}
		else // m_Direction == PACKET_RECV
			WarnReceive();
	}
	else if (m_Direction == PACKET_SEND)
		Button_Enable(GetDlgItem(m_hDialog, IDC_SPAMSHOWCHECK), TRUE);
}

VOID CSpamPacket::ChangeInjectDirection()
{
	// don't allow for changes if a packet is being spammed
	if (m_hSpamThread != NULL)
		return;

	if (m_Direction == PACKET_RECV)
	{
		m_Direction = PACKET_SEND;
		SendMessageW(GetDlgItem(m_hDialog, IDC_SPAMINJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_pXPIGUI->hInjectOut);
		Button_Enable(GetDlgItem(m_hDialog, IDC_SPAMSHOWCHECK), TRUE);
	}
	else // m_Direction == PACKET_SEND
	{
		m_Direction = PACKET_RECV;
		SendMessageW(GetDlgItem(m_hDialog, IDC_SPAMINJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_pXPIGUI->hInjectIn);
		Button_Enable(GetDlgItem(m_hDialog, IDC_SPAMSHOWCHECK), FALSE);
		OnIntervalChange();
	}
}

BOOL CSpamPacket::CreateUnformattedPacket()
{
	HWND hEdit = GetDlgItem(m_hDialog, IDC_SPAMEDIT);
	if (hEdit == NULL)
		return FALSE;

	INT nLength = Edit_GetTextLength(hEdit);

	if (nLength <= 0)
	{
		std::wstring wstrTitle, wstrText;
		wstrTitle = wstrTitle = pStrings->Get(IDS_INJECT_ERROR_TITLE);
		wstrText = pStrings->Get(IDS_INJECT_ERROR_EMPTY);

		EDITBALLOONTIP  TipError;

		TipError.cbStruct = sizeof(EDITBALLOONTIP);
		TipError.ttiIcon = TTI_ERROR;
		TipError.pszTitle = wstrTitle.c_str();
		TipError.pszText = wstrText.c_str();
		Edit_ShowBalloonTip(hEdit, &TipError);

		return FALSE;
	}

	std::wstring wstr(nLength, 0);

	// NOTE: no need to check this since we already do so with Edit_GetTextLength
	Edit_GetText(hEdit, &wstr[0], nLength + 1);

	// convert string to byte array
	std::vector<BYTE> vbData;

	if (!StringToBuffer(wstr, &vbData))
	{
		std::wstring wstrTitle, wstrText;
		wstrTitle = wstrTitle = pStrings->Get(IDS_INJECT_ERROR_TITLE);
		wstrText = pStrings->Get(IDS_INJECT_ERROR_PARSE);

		EDITBALLOONTIP  TipError;

		TipError.cbStruct = sizeof(EDITBALLOONTIP);
		TipError.ttiIcon = TTI_ERROR;
		TipError.pszTitle = wstrTitle.c_str();
		TipError.pszText = wstrText.c_str();
		Edit_ShowBalloonTip(hEdit, &TipError);

		return FALSE;
	}

	// ensure there is at least an opcode
	if (vbData.size() < sizeof(WORD))
	{
		std::wstring wstrTitle, wstrText;
		wstrTitle = wstrTitle = pStrings->Get(IDS_INJECT_ERROR_TITLE);
		wstrText = pStrings->Get(IDS_INJECT_ERROR_SHORT);

		EDITBALLOONTIP  TipError;

		TipError.cbStruct = sizeof(EDITBALLOONTIP);
		TipError.ttiIcon = TTI_ERROR;
		TipError.pszTitle = wstrTitle.c_str();
		TipError.pszText = wstrText.c_str();
		Edit_ShowBalloonTip(hEdit, &TipError);

		return FALSE;
	}

	if (m_Direction == PACKET_SEND)
	{
		if (m_bShowPacket)
			this->m_oPacket = new COutPacket(*(WORD*)(&vbData[0]));
		else
		{
			// need to fix
			this->m_oPacket = new COutPacket(*(WORD*)(&vbData[0]));
		}

		if (vbData.size() > sizeof(WORD))
		{
			if (m_bShowPacket)
				EncodeBuffer_Hook(m_oPacket, 0, (&vbData[0]) + sizeof(WORD), vbData.size() - sizeof(WORD));
			else
				m_oPacket->EncodeBuffer((&vbData[0]) + sizeof(WORD), vbData.size() - sizeof(WORD));
		}
	}
	else // PACKET_RECV
	{
		COutPacket oTempPacket(*(WORD*)(&vbData[0]));
		if (vbData.size() > sizeof(WORD))
			oTempPacket.EncodeBuffer((&vbData[0]) + sizeof(WORD), vbData.size() - sizeof(WORD));

		this->m_iPacket = new CInPacket;
		ZeroMemory(this->m_iPacket, sizeof(CInPacket));

		this->m_iPacket->m_aRecvBuff = oTempPacket.m_aSendBuff;
		this->m_iPacket->m_uLength = oTempPacket.m_uOffset;
		this->m_iPacket->m_nState = RS_COMPLETED;
		this->m_iPacket->m_uDataLen = oTempPacket.m_uOffset;
	}

	return TRUE;
}

BOOL CSpamPacket::CreateFormattedPacket()
{
	HWND hEdit = GetDlgItem(m_hDialog, IDC_SPAMEDIT);
	if (hEdit == NULL)
		return FALSE;

	INT nLength = Edit_GetTextLength(hEdit);

	if (nLength <= 0)
	{
		std::wstring wstrTitle, wstrText;
		wstrTitle = wstrTitle = pStrings->Get(IDS_INJECT_ERROR_TITLE);
		wstrText = pStrings->Get(IDS_INJECT_ERROR_EMPTY);

		EDITBALLOONTIP  TipError;

		TipError.cbStruct = sizeof(EDITBALLOONTIP);
		TipError.ttiIcon = TTI_ERROR;
		TipError.pszTitle = wstrTitle.c_str();
		TipError.pszText = wstrText.c_str();
		Edit_ShowBalloonTip(hEdit, &TipError);

		return FALSE;
	}

	std::wstring wstr(nLength + 1, 0);

	// NOTE: no need to check this since we already do so with Edit_GetTextLength
	Edit_GetText(hEdit, &wstr[0], nLength + 1);

	std::string str(nLength + 1, 0);
	if (WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &str[0], nLength + 1, NULL, NULL) == 0)
		return FALSE;

	std::string strData = str;
	/***/
	std::string::const_iterator iter = strData.begin();
	std::string::const_iterator end = strData.end();

	if (m_bShowPacket && m_Direction != PACKET_RECV)
	{
		this->m_oPacket = new COutPacket;
		formatted_packet_parser parser(m_oPacket, FALSE);

		if (!boost::spirit::qi::phrase_parse(iter, end, parser, boost::spirit::ascii::space) || iter != end)
		{
			EDITBALLOONTIP  TipError;
			std::wstring    wstrTitle = pStrings->Get(IDS_INJECT_ERROR_TITLE);
			std::wstring    wstrText = pStrings->Get(IDS_INJECT_ERROR_PARSE);

			TipError.cbStruct = sizeof(EDITBALLOONTIP);
			TipError.ttiIcon = TTI_ERROR;
			TipError.pszTitle = wstrTitle.c_str();
			TipError.pszText = wstrText.c_str();
			Edit_ShowBalloonTip(hEdit, &TipError);
			return FALSE;
		}

		return TRUE;
	}

	COutPacket oTempPacket;

	// hide send spam otherwise
	formatted_packet_parser parser(&oTempPacket, TRUE);

	if (!boost::spirit::qi::phrase_parse(iter, end, parser, boost::spirit::ascii::space) || iter != end)
	{
		EDITBALLOONTIP  TipError;
		std::wstring    wstrTitle = pStrings->Get(IDS_INJECT_ERROR_TITLE);
		std::wstring    wstrText = pStrings->Get(IDS_INJECT_ERROR_PARSE);

		TipError.cbStruct = sizeof(EDITBALLOONTIP);
		TipError.ttiIcon = TTI_ERROR;
		TipError.pszTitle = wstrTitle.c_str();
		TipError.pszText = wstrText.c_str();
		Edit_ShowBalloonTip(hEdit, &TipError);
		return FALSE;
	}

	if (m_Direction == PACKET_RECV)
	{
		this->m_iPacket = new CInPacket;
		ZeroMemory(this->m_iPacket, sizeof(CInPacket));

		this->m_iPacket->m_aRecvBuff = oTempPacket.m_aSendBuff;
		this->m_iPacket->m_uLength = oTempPacket.m_uOffset;
		this->m_iPacket->m_nState = RS_COMPLETED;
		this->m_iPacket->m_uDataLen = oTempPacket.m_uOffset;
	}
	else
	{
		m_oPacket = new COutPacket;
		CopyMemory(m_oPacket, &oTempPacket, sizeof(COutPacket));
	}

	return TRUE;
}

VOID CSpamPacket::OnInjectClick()
{
	m_bShowPacket = Button_GetCheck(GetDlgItem(m_hDialog, IDC_SPAMSHOWCHECK)) == BST_CHECKED ? FALSE : TRUE;

	if (m_hSpamThread == NULL)
	{
		EnableInput(FALSE);

		WCHAR   wszTemp[16];
		LPWSTR  lpwszEnd;

		BOOL bResult;

		bResult = m_bFormatted ? CreateFormattedPacket() : CreateUnformattedPacket();

		if (!bResult)
		{
			EnableInput(TRUE);
			return;
		}

		LPSPAMPACKETTHREAD lpThreadData = new SPAMPACKETTHREAD;

		lpThreadData->hDialog = m_hDialog;
		lpThreadData->bShowPacket = m_bShowPacket;
		lpThreadData->Direction = m_Direction;
		lpThreadData->pullCount = &m_ullCount;
		lpThreadData->oPacket = m_oPacket;
		lpThreadData->iPacket = m_iPacket;

		Edit_GetText(GetDlgItem(m_hDialog, IDC_SPAMREPEATEDIT), wszTemp, _countof(wszTemp));
		lpThreadData->ulRepeat = wcstoul(wszTemp, &lpwszEnd, 10);

		Edit_GetText(GetDlgItem(m_hDialog, IDC_SPAMINTERVALEDIT), wszTemp, _countof(wszTemp));
		lpThreadData->ulInterval = wcstoul(wszTemp, &lpwszEnd, 10);

		m_hSpamThread = CreateThread(NULL, 0, CSpamPacket::SpamThread, (LPVOID)lpThreadData, 0, NULL);
	}
	else
	{
		// stop spamming
		TerminateThread(m_hSpamThread, EXIT_SUCCESS);
		OnDoneSpamming();
	}
}

DWORD WINAPI CSpamPacket::SpamThread(__in LPVOID lpParameter)
{
	LPSPAMPACKETTHREAD  lpThreadData = (LPSPAMPACKETTHREAD)lpParameter;
	// create a local copy of the data for memory's sake
	SPAMPACKETTHREAD    ThreadData = *lpThreadData;

	delete lpThreadData;

	LARGE_INTEGER liDueTime;
	HANDLE        hTimer = CreateWaitableTimer(NULL, TRUE, NULL);

	// convert milliseconds to 100-nanosecond intervals
	liDueTime.QuadPart = ThreadData.ulInterval * -10000LL;

	INT iReportInterval;

	if (ThreadData.ulInterval == 0)
		iReportInterval = 10000;
	else if (ThreadData.ulInterval >= 100)
		iReportInterval = 1;
	else
		iReportInterval = 100 / ThreadData.ulInterval;

	WCHAR wszTemp[16];

	// status bar
	std::wstring wstrPacketCurrent = pStrings->Get(IDS_SPAM_CURRENT);
	if (ThreadData.ulRepeat == 0)
		wstrPacketCurrent += L"∞";
	else
	{
		StringCchPrintfW(wszTemp, _countof(wszTemp), L"0/%d", ThreadData.ulRepeat);
		wstrPacketCurrent += wszTemp;
	}
	SendMessageW(GetDlgItem(ThreadData.hDialog, IDC_SPAMSTATUS), SB_SETTEXT, (WPARAM)SB_CURR, (LPARAM)wstrPacketCurrent.c_str());

	CInPacket iPacket;
	std::shared_ptr<CMaplePacket> pOriginalPacket = NULL;

	if (ThreadData.bShowPacket && ThreadData.Direction == PACKET_SEND && ThreadData.oPacket != NULL)
		pOriginalPacket = pInstances->Find(ThreadData.oPacket);

	for (DWORD i = 0; ThreadData.ulRepeat == 0 ? TRUE : i < ThreadData.ulRepeat; i++)
	{
		*ThreadData.pullCount = *ThreadData.pullCount + 1LL;

		if (ThreadData.Direction == PACKET_SEND && ThreadData.oPacket != NULL)
		{
			if (ThreadData.bShowPacket && pOriginalPacket != NULL)
			{
				CMAPLEPACKETSTRUCT cmps;

				cmps.pInstance = ThreadData.oPacket;
				cmps.Direction = PACKET_SEND;
				cmps.ulState = PACKET_INJECTED;
				cmps.lpv = _ReturnAddress();

				auto pTempPacket = std::make_shared<CMaplePacket>(&cmps);
				pTempPacket->CopyMembersFrom(pOriginalPacket.get());

				pInstances->Add(ThreadData.oPacket, pTempPacket);

				// diff thread (SendMessage)
				CClientSocket::GetInstance()->SendPacket(ThreadData.oPacket);

				pInstances->Remove(ThreadData.oPacket);
			}
			else
			{
				// Hide Packet
				CClientSocket::GetInstance()->SendPacket(ThreadData.oPacket);
			}
		}
		else if (ThreadData.Direction == PACKET_RECV && ThreadData.iPacket != NULL)
		{
			CopyMemory(&iPacket, ThreadData.iPacket, sizeof(CInPacket));

			CMAPLEPACKETSTRUCT cmps;

			cmps.pInstance = ThreadData.iPacket;
			cmps.Direction = PACKET_RECV;
			cmps.ulState = PACKET_INJECTED;
			cmps.lpv = _ReturnAddress();

			pInstances->Add(ThreadData.iPacket, std::make_shared<CMaplePacket>(&cmps));

			CClientSocket::GetInstance()->ProcessPacket(ThreadData.iPacket);

			pInstances->Remove(ThreadData.iPacket);

			CopyMemory(ThreadData.iPacket, &iPacket, sizeof(CInPacket));
		}

		if (i % iReportInterval == 0)
			SendMessageW(ThreadData.hDialog, WM_UPDATESPAMSTATUS, (WPARAM)i, (LPARAM)ThreadData.ulRepeat);

		SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, 0);
		WaitForSingleObject(hTimer, INFINITE);
	}

	SendMessageW(ThreadData.hDialog, WM_DONESPAMMING, 0, 0);
	return EXIT_SUCCESS;
}

VOID CSpamPacket::OnDoneSpamming()
{
	m_hSpamThread = NULL;

	if (m_Direction == PACKET_SEND)
	{
		if (m_oPacket != NULL)
		{
			delete m_oPacket;
			m_oPacket = NULL;
		}
	}
	else
	{
		if (m_iPacket != NULL)
		{
			delete m_iPacket;
			m_iPacket = NULL;
		}
	}

	EnableInput(TRUE);
}

VOID CSpamPacket::UpdateStatus(__in DWORD dwAt, __in DWORD dwTotal)
{
	std::wstring wstrTemp = pStrings->Get(IDS_SPAM_SESSION);
	WCHAR wszTemp[36];
	StringCchPrintfW(wszTemp, _countof(wszTemp), L"%d", m_ullCount);
	wstrTemp += wszTemp;
	SendMessageW(GetDlgItem(m_hDialog, IDC_SPAMSTATUS), SB_SETTEXT, (WPARAM)SB_SESS, (LPARAM)wstrTemp.c_str());

	if ((dwAt == UINT_MAX && dwTotal == UINT_MAX) || dwTotal == 0)
		return;

	wstrTemp = pStrings->Get(IDS_SPAM_CURRENT);
	StringCchPrintfW(wszTemp, _countof(wszTemp), L"%d/%d", dwAt + 1, dwTotal);
	wstrTemp += wszTemp;
	SendMessageW(GetDlgItem(m_hDialog, IDC_SPAMSTATUS), SB_SETTEXT, (WPARAM)SB_CURR, (LPARAM)wstrTemp.c_str());
}

VOID CSpamPacket::EnableInput(__in BOOL bEnable)
{
	if (!bEnable)
	{
		SendMessageW(GetDlgItem(m_hDialog, IDC_SPAMINJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)m_pXPIGUI->hStop);
		Edit_Enable(GetDlgItem(m_hDialog, IDC_SPAMEDIT), FALSE);
		Edit_Enable(GetDlgItem(m_hDialog, IDC_SPAMREPEATEDIT), FALSE);
		Edit_Enable(GetDlgItem(m_hDialog, IDC_SPAMINTERVALEDIT), FALSE);
		Button_Enable(GetDlgItem(m_hDialog, IDC_SPAMSHOWCHECK), FALSE);
	}
	else
	{
		SendMessageW(GetDlgItem(m_hDialog, IDC_SPAMSTATUS), SB_SETTEXT, (WPARAM)SB_CURR, (LPARAM)pStrings->Get(IDS_IDLE).c_str());
		UpdateStatus(UINT_MAX, UINT_MAX);

		SendMessageW(GetDlgItem(m_hDialog, IDC_SPAMINJECT), BM_SETIMAGE, IMAGE_ICON, (LPARAM)(m_Direction == PACKET_RECV ? m_pXPIGUI->hInjectIn : m_pXPIGUI->hInjectOut));
		Edit_Enable(GetDlgItem(m_hDialog, IDC_SPAMEDIT), TRUE);
		Edit_Enable(GetDlgItem(m_hDialog, IDC_SPAMREPEATEDIT), TRUE);
		Edit_Enable(GetDlgItem(m_hDialog, IDC_SPAMINTERVALEDIT), TRUE);

		if (m_Direction == PACKET_SEND)
			OnIntervalChange();
	}
}

INT_PTR CALLBACK CSpamPacket::DialogProc(__in HWND hDialog, __in UINT uMessage, __in WPARAM wParam, __in LPARAM lParam)
{
	switch (uMessage)
	{
	case WM_INITDIALOG:
		if (!(new CSpamPacket(hDialog, (PSPAMPACKET)lParam))->OnCreate())
		{
			EndDialog(hDialog, EXIT_FAILURE);
			DestroyWindow(hDialog);
			return FALSE;
		}
		break;

	case WM_NOTIFY:
		if (((LPNMHDR)lParam)->idFrom == IDC_SPAMINJECT && ((LPNMHDR)lParam)->code == BCN_DROPDOWN)
			GetClassInstance<CSpamPacket>(hDialog)->ChangeInjectDirection();
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SPAMFORMATTED && HIWORD(wParam) == STN_CLICKED)
			GetClassInstance<CSpamPacket>(hDialog)->SetFormatted(FALSE);
		else if (LOWORD(wParam) == IDC_SPAMPLAIN && HIWORD(wParam) == STN_CLICKED)
			GetClassInstance<CSpamPacket>(hDialog)->SetFormatted(TRUE);
		else if (LOWORD(wParam) == IDC_SPAMREPEATEDIT && HIWORD(wParam) == EN_CHANGE)
			GetClassInstance<CSpamPacket>(hDialog)->OnRepeatChange();
		else if (LOWORD(wParam) == IDC_SPAMINTERVALEDIT && HIWORD(wParam) == EN_CHANGE)
			GetClassInstance<CSpamPacket>(hDialog)->OnIntervalChange();
		else if (LOWORD(wParam) == IDC_SPAMINJECT && HIWORD(wParam) == BN_CLICKED)
			GetClassInstance<CSpamPacket>(hDialog)->OnInjectClick();
		break;

	case WM_DRAWITEM:
		if (wParam == IDC_SPAMFORMATTED)
			GetClassInstance<CSpamPacket>(hDialog)->DrawFormatted((LPDRAWITEMSTRUCT)lParam, TRUE);
		else if (wParam == IDC_SPAMPLAIN)
			GetClassInstance<CSpamPacket>(hDialog)->DrawFormatted((LPDRAWITEMSTRUCT)lParam, FALSE);
		break;

	case WM_DONESPAMMING:
		GetClassInstance<CSpamPacket>(hDialog)->OnDoneSpamming();
		break;

	case WM_UPDATESPAMSTATUS:
		GetClassInstance<CSpamPacket>(hDialog)->UpdateStatus((DWORD)wParam, (DWORD)lParam);
		break;

	case WM_CLOSE:
		EndDialog(hDialog, EXIT_SUCCESS);
		DestroyWindow(hDialog);
		break;

	case WM_DESTROY:
		delete GetClassInstance<CSpamPacket>(hDialog);
		break;

	default:
		return FALSE;
	}

	return TRUE;
}
