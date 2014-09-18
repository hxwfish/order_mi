// order_mi.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "order_mi.h"

#include <atlbase.h>
#include <olectl.h>

#pragma once
#include <UILib.h>

using namespace DuiLib;

#ifdef _DEBUG
#	ifdef _UNICODE
#		pragma comment(lib, "DuiLib_ud.lib")
#	else
#		pragma comment(lib, "DuiLib_d.lib")
#	endif
#else
#	ifdef _UNICODE
#		pragma comment(lib, "DuiLib_u.lib")
#	else
#		pragma comment(lib, "DuiLib.lib")
#	endif
#endif


class CMyWebBrowserEventHandler : public CWebBrowserEventHandler
{
	virtual void NewWindow3(IDispatch **pDisp, VARIANT_BOOL *&Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
	{
		*Cancel = TRUE;
		m_pWebBrowser->Navigate2(bstrUrl);
	}
public:
	CWebBrowserUI* m_pWebBrowser;
};

CDuiString GetLoginPage()
{
	TCHAR szBuf[MAX_PATH];
	CDuiString strText = CPaintManagerUI::GetInstancePath();
	strText = strText + _T("\\") + _T("Setting.ini");

	GetPrivateProfileString(_T("Setting"), _T("LoginPage"), _T("https://account.xiaomi.com/pass/serviceLogin?callback=http%3A%2F%2Forder.mi.com%2Flogin%2Fcallback%3Ffollowup%3Dhttp%253A%252F%252Fwww.mi.com%26sign%3DMjMyMGJhNjNmZmM2NTc0YWM4NzdkN2IzMjNlZjhmMzhhODAxMDZiNg%2C%2C&sid=mi_eshop"),
		szBuf, MAX_PATH, strText);

	strText = szBuf;

	if (strText == _T(""))
		strText = _T("https://account.xiaomi.com/pass/serviceLogin?callback=http%3A%2F%2Forder.mi.com%2Flogin%2Fcallback%3Ffollowup%3Dhttp%253A%252F%252Fwww.mi.com%26sign%3DMjMyMGJhNjNmZmM2NTc0YWM4NzdkN2IzMjNlZjhmMzhhODAxMDZiNg%2C%2C&sid=mi_eshop");

	return strText;
}

CDuiString GetOrderPage()
{
	TCHAR szBuf[MAX_PATH];
	CDuiString strText = CPaintManagerUI::GetInstancePath();
	strText = strText + _T("\\") + _T("Setting.ini");

	GetPrivateProfileString(_T("Setting"), _T("OrderPage"), _T("http://a.hd.mi.com/productv2/book/a/4"),
		szBuf, MAX_PATH, strText);

	strText = szBuf;

	if (strText == _T(""))
		strText = _T("http://a.hd.mi.com/productv2/book/a/4");

	return strText;
}

class COrderMIFrameWnd : public WindowImplBase
{
public:
	virtual LPCTSTR GetWindowClassName() const { return _T("OrderMIMainFrame"); }
	virtual CDuiString GetSkinFile() { return _T("main_dlg.xml"); }
	virtual CDuiString GetSkinFolder() { return CPaintManagerUI::GetInstancePath(); }

	virtual void InitWindow()
	{
		m_pWebBrowser = static_cast<CWebBrowserUI*>(m_PaintManager.FindControl(_T("IE")));

		if (m_pWebBrowser) {
			static CMyWebBrowserEventHandler handler;
			handler.m_pWebBrowser = m_pWebBrowser;
			m_pWebBrowser->SetWebBrowserEventHandler(&handler);

			m_pWebBrowser->Navigate2(m_pWebBrowser->GetHomePage());
		}

		InitUI();

		m_uBookTimes = 0;
	}

	void InitUI()
	{
		m_pIEAddr = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("addEdit")));
		m_pIEGoto = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("goto")));
		m_pBookTimes = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("textBooked")));
		m_pBookTimes->SetText(_T("0"));
		m_apRichEdit[0] = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("edit1")));
		m_apRichEdit[1] = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("edit2")));
		m_apRichEdit[2] = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("edit3")));
		m_apRichEdit[3] = static_cast<CRichEditUI*>(m_PaintManager.FindControl(_T("edit4")));
	}

	void SetWindowRange(int nRange) 
	{
		IWebBrowser2 *pWeb = m_pWebBrowser->GetWebBrowser2();
		CComVariant varZoom(nRange);
		pWeb->ExecWB(OLECMDID_OPTICAL_ZOOM, OLECMDEXECOPT_DODEFAULT, &varZoom, NULL);
	}

	void BookCompleted(ebookedStatus eStatus)
	{
		int nUserLineCount = m_apRichEdit[0]->GetLineCount();
		int nAddrLineCount = m_apRichEdit[1]->GetLineCount();
		int i = 1;

		if (m_apRichEdit[0]->GetLine(0, 100).GetLength() <=1) {
			return;
		}

		CDuiString strContext = _T("");

		if (eStatus == E_BOOK_SUCCESS || eStatus == E_BOOK_SUCCESS_OLD)
			m_apRichEdit[3]->AppendText(m_apRichEdit[0]->GetLine(0, 100));
		else if (eStatus == E_BOOK_FAIL)
			m_apRichEdit[2]->AppendText(m_apRichEdit[0]->GetLine(0, 100));

		for (i = 1; i < nUserLineCount; i++) {
			CDuiString strText = m_apRichEdit[0]->GetLine(i, 100);
			if (strText != _T("\r"))
				strContext += m_apRichEdit[0]->GetLine(i, 100);
		}
		m_apRichEdit[0]->SetText(strContext);

		strContext = _T("");
		if (nAddrLineCount > 1 && (eStatus == E_BOOK_SUCCESS))
			for (i = 1; i < nAddrLineCount; i++) {
				CDuiString strText = m_apRichEdit[1]->GetLine(i, 100);
				if (strText != _T("\r"))
					strContext += m_apRichEdit[1]->GetLine(i, 100);
			}

		if (eStatus == E_BOOK_SUCCESS)
			m_apRichEdit[1]->SetText(strContext);

		if ((eStatus == E_BOOK_SUCCESS) || (eStatus == E_BOOK_SUCCESS_OLD))
			m_uBookTimes++;

		strContext.Format(_T("%d"), m_uBookTimes);
		m_pBookTimes->SetText(strContext);
	}

	void FillBufferToCopy(int nIndex, CDuiString& strName)
	{
		HGLOBAL hClip;
		CDuiString strContext;
		
		strContext = m_apRichEdit[nIndex]->GetSelText();

		// no select any char, filling first line to buffer
		if (strContext == _T("")) {
			strContext = m_apRichEdit[nIndex]->GetLine(0, 200);
		}

		if (OpenClipboard(NULL)) {
			EmptyClipboard();

#ifdef _UNICODE
			wchar_t *buff;
			hClip = GlobalAlloc(GMEM_MOVEABLE, 2*strContext.GetLength()+sizeof(wchar_t));

			buff = (wchar_t*)GlobalLock(hClip);
			wcscpy(buff, strContext);

			GlobalUnlock(hClip);

			SetClipboardData(CF_UNICODETEXT, hClip);
#else
			char *pBuf;

			hClip = GlobalAlloc(GMEM_MOVEABLE, strContext.GetLength()+1);

			pBuf = (char*)GlobalLock(hClip);

			strcpy(pBuf, LPCSTR(strContext));
			GlobalUnlock(hClip);

			SetClipboardData(CF_TEXT, hClip);
#endif
			CloseClipboard();
		}
	}

	virtual void Notify(TNotifyUI& msg)
	{
		if (msg.sType == _T("click")) {
			if (msg.pSender->GetName() == _T("goto")) {
				CDuiString strAddr = m_pIEAddr->GetText();
				if (strAddr != _T(""))
					m_pWebBrowser->Navigate2(strAddr);
			}
			else if (msg.pSender->GetName() == _T("back")) {
				m_pWebBrowser->GoBack();
			}
			else if (msg.pSender->GetName() == _T("foward")) {
				m_pWebBrowser->GoForward();
			}
			else if (msg.pSender->GetName() == _T("refresh")) {
				m_pWebBrowser->Refresh();
			}
			else if (msg.pSender->GetName() == _T("homepage")) {
				if (m_pWebBrowser->GetHomePage() != _T(""))
					m_pWebBrowser->Navigate2(m_pWebBrowser->GetHomePage());
			}
			else if (msg.pSender->GetName() == _T("order")) {
				CDuiString strAddr = GetOrderPage();
				if (strAddr != _T(""))
					m_pWebBrowser->Navigate2(strAddr);
			}
			else if (msg.pSender->GetName() == _T("login")) {
				CDuiString strAddr = GetLoginPage();
				if (strAddr != _T(""))
					m_pWebBrowser->Navigate2(strAddr);
			}
			else if (msg.pSender->GetName() == _T("btnRange80")) {
				SetWindowRange(80);
			}			
			else if (msg.pSender->GetName() == _T("btnRange100")) {
				SetWindowRange(100);
			}
			else if (msg.pSender->GetName() == _T("btnSuccess")) {
				BookCompleted(E_BOOK_SUCCESS);
			}
			else if (msg.pSender->GetName() == _T("btnblack")) {
				BookCompleted(E_BOOK_FAIL);
			}
			else if (msg.pSender->GetName() == _T("btnSuccessOld")) {
				BookCompleted(E_BOOK_SUCCESS_OLD);
			}
			else if (msg.pSender->GetName() == _T("textBookedClear")) {
				m_pBookTimes->SetText(_T("0"));
			}

		}
		else if (msg.sType == _T("menu")) {
			if (msg.pSender->GetName() == _T("edit1")) {
				FillBufferToCopy(0, msg.pSender->GetName());
			}
			else if (msg.pSender->GetName() == _T("edit2")) {
				FillBufferToCopy(1, msg.pSender->GetName());
			}
			else if (msg.pSender->GetName() == _T("edit3")) {
				FillBufferToCopy(2, msg.pSender->GetName());
			}
			else if (msg.pSender->GetName() == _T("edit4")) {
				FillBufferToCopy(3, msg.pSender->GetName());
			}
		}
		else if (msg.sType == _T("setfocus")) {
			SIZE sz = {0, 0};
			if (msg.pSender->GetName() == _T("edit1")) {
				m_apRichEdit[0]->SetScrollPos(sz);
			}
			else if (msg.pSender->GetName() == _T("edit2")) {
				m_apRichEdit[1]->SetScrollPos(sz);
			}
		}

		__super::Notify(msg);
	}

	LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM , bool& bHandled)
	{
		if (uMsg == WM_KEYDOWN) {
			switch (wParam) {
				case VK_ESCAPE:
					bHandled = true;
					break;
				default:
					break;
			}
		}
		return FALSE;
	}

private:
	CRichEditUI* m_apRichEdit[MAX_EDIT_NUM];
	CButtonUI*   m_apBtnOpen[MAX_EDIT_NUM];
	CButtonUI*   m_apBtnSave[MAX_EDIT_NUM];
	CButtonUI*   m_pIEBack;
	CButtonUI*   m_pIEForward;
	CButtonUI*   m_pIERefresh;
	CButtonUI*   m_pIEHomePage;
	CButtonUI*   m_pIEGoto;
	CEditUI*     m_pIEAddr;
	CLabelUI*    m_pBookTimes;
	CWebBrowserUI* m_pWebBrowser;
	UINT		m_uBookTimes;
};

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());
	//CPaintManagerUI::SetResourceZip(_T("Skin.zip"));

	OleInitialize(NULL);

	COrderMIFrameWnd* orderFrame = new COrderMIFrameWnd();
	orderFrame->Create(NULL, _T("OMWnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	orderFrame->CenterWindow();
	orderFrame->ShowModal();

	return 0;
}



