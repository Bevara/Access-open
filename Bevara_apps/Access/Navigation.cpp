// Navigation.cpp : implementation file
//

#include "stdafx.h"
#include "osmo4.h"
#include "Navigation.h"
#include "MainFrm.h"
#include <gpac/options.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Navigation dialog

Navigation::Navigation(CWnd* pParent /*=NULL*/)
	: CDialog(Navigation::IDD, pParent)
{
	//{{AFX_DATA_INIT(Navigation)
	//}}AFX_DATA_INIT
}


void Navigation::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Navigation)
	DDX_Control(pDX, IDC_NAV_PREV, m_Nav_Prev);
	DDX_Control(pDX, IDC_NAV_NEXT, m_Nav_Next);
	DDX_Control(pDX, IDC_CUR_PAGE, m_cur_page);
	DDX_Control(pDX, IDC_PAGE_COUNT, m_page_count);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Navigation, CDialog)
	//{{AFX_MSG_MAP(Navigation)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_NAV_PREV, &Navigation::OnBnClickedPrev)
	ON_BN_CLICKED(IDC_NAV_NEXT, &Navigation::OnBnClickedNext)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Navigation message handlers

void Navigation::hideNav(){
	m_Nav_Prev.ShowWindow(SW_HIDE);
	m_Nav_Next.ShowWindow(SW_HIDE);
	m_cur_page.ShowWindow(SW_HIDE);
	m_page_count.ShowWindow(SW_HIDE);
	this->ShowWindow(SW_HIDE);
	this->UpdateWindow();

	isShow = false;
}

void Navigation::showNav(){
	m_Nav_Prev.ShowWindow(SW_SHOW);
	m_Nav_Next.ShowWindow(SW_SHOW);
	m_cur_page.ShowWindow(SW_SHOW);
	m_page_count.ShowWindow(SW_SHOW);
	
	updateNav();

	this->ShowWindow(SW_SHOW);
	this->UpdateWindow();

	isShow = true;
}

void Navigation::OnBnClickedNext()
{
	Osmo4 *app = GetApp();
	CMainFrame *pFrame = (CMainFrame *) app->m_pMainWnd;
	Playlist *pPlayList = pFrame->m_pPlayList;
	if (!pPlayList) return;

	/*don't play if last could trigger playlist loop*/
	if ((pPlayList->m_cur_entry<0) || (gf_list_count(pPlayList->m_entries) == 1 + (u32) pPlayList->m_cur_entry)) return;
	pPlayList->PlayNext();
	updateNav();
}

void Navigation::OnSize(UINT nType, int cx, int cy) 
{
	if (!isShow)return;
	CDialog::OnSize(nType, cx, cy);

	if (!m_Nav_Prev.m_hWnd) return;

}

void Navigation::OnBnClickedPrev(){
	Osmo4 *app = GetApp();
	CMainFrame *pFrame = (CMainFrame *) app->m_pMainWnd;
	Playlist *pPlayList = pFrame->m_pPlayList;
	if (!pPlayList) return;

	if (pPlayList->m_cur_entry<=0) return;
	pPlayList->PlayPrev();
	updateNav();
}

/*we sure don't want to close this window*/
void Navigation::OnClose() 
{
	return;
}

void  Navigation::updateNav(){
	Osmo4 *app = GetApp();
	CMainFrame *pFrame = (CMainFrame *) app->m_pMainWnd;
	Playlist *pPlayList = pFrame->m_pPlayList;
	if (!pPlayList) return;

	m_Nav_Next.ShowWindow(SW_SHOW);
	m_Nav_Prev.ShowWindow(SW_SHOW);

	
	// Update nav next 
	if (pPlayList->m_cur_entry<0) m_Nav_Next.ShowWindow(SW_HIDE);
	else if ((u32) pPlayList->m_cur_entry + 1 == gf_list_count(pPlayList->m_entries) ) m_Nav_Next.ShowWindow(SW_HIDE);
	else m_Nav_Next.ShowWindow(SW_SHOW);


	// Update nav prev 
	if (pPlayList->m_cur_entry<=0) m_Nav_Prev.ShowWindow(SW_HIDE);
	else m_Nav_Prev.ShowWindow(SW_SHOW);
	
	// Update current value 
	CString value;
	value.Format(_T("%i"), pPlayList->m_cur_entry + 1);
	m_cur_page.SetWindowText(value);

	// Update page count 
	value.Format(_T(" / %i"), gf_list_count(pPlayList->m_entries));
	m_page_count.SetWindowText(value);
}


BOOL Navigation::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN) {
		GetApp()->m_pMainWnd->SetFocus();
		GetApp()->m_pMainWnd->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL Navigation::OnInitDialog() 
{
	CDialog::OnInitDialog();
	CButton * pButton;
	pButton = (CButton *)GetDlgItem(IDC_NAV_PREV);
	if ( pButton && pButton->GetSafeHwnd() )
	{
	pButton->SetBitmap((HBITMAP)LoadImage(AfxGetApp()->m_hInstance,
      MAKEINTRESOURCE(IDB_LEFT_ARROW),
      IMAGE_BITMAP, 28, 28, LR_COLOR));
 }

	pButton = (CButton *)GetDlgItem(IDC_NAV_NEXT);
	if ( pButton && pButton->GetSafeHwnd() )
	{
	pButton->SetBitmap((HBITMAP)LoadImage(AfxGetApp()->m_hInstance,
      MAKEINTRESOURCE(IDB_RIGHT_ARROW),
      IMAGE_BITMAP, 28, 28, LR_COLOR));
	}
	return TRUE;
}
