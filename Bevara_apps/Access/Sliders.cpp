// Sliders.cpp : implementation file
//



#include "stdafx.h"
#include "osmo4.h"
#include "Sliders.h"
#include "MainFrm.h"

#include <gpac/options.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Sliders dialog

Sliders::Sliders(CWnd* pParent /*=NULL*/)
	: CDialog(Sliders::IDD, pParent)
{
	//{{AFX_DATA_INIT(Sliders)
	//}}AFX_DATA_INIT

	m_grabbed = GF_FALSE;
	type = NONE;
}


void Sliders::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Sliders)
	DDX_Control(pDX, ID_AUDIO_VOL, m_AudioVol);
	DDX_Control(pDX, ID_SLIDER, m_PosSlider);

	//Maja inculde button
	DDX_Control(pDX, IDC_AUDIO_MUTE, m_AudioMute);
	DDX_Control(pDX, IDC_PLAY, m_Play);

	DDX_Control(pDX, IDC_NAV_PREV, m_Nav_Prev);
	DDX_Control(pDX, IDC_NAV_NEXT, m_Nav_Next);
	DDX_Control(pDX, IDC_CUR_PAGE, m_cur_page);
	DDX_Control(pDX, IDC_PAGE_COUNT, m_page_count);

	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(Sliders, CDialog)
	//{{AFX_MSG_MAP(Sliders)
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_AUDIO_MUTE, &Sliders::OnBnClickedAudioMute)
	ON_BN_CLICKED(IDC_PLAY, &Sliders::OnBnClickedPlay)
	ON_BN_CLICKED(IDC_NAV_PREV, &Sliders::OnBnClickedPrev)
	ON_BN_CLICKED(IDC_NAV_NEXT, &Sliders::OnBnClickedNext)
	ON_NOTIFY(NM_CUSTOMDRAW, ID_AUDIO_VOL, &Sliders::OnNMCustomdrawAudioVol)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Sliders message handlers

void Sliders::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	
	Osmo4 *app = GetApp();
	if (pScrollBar->GetDlgCtrlID() == ID_SLIDER) {
		switch (nSBCode) {
		case TB_PAGEUP:
		case TB_PAGEDOWN:
		case TB_LINEUP:
		case TB_LINEDOWN:
		case TB_TOP:
		case TB_BOTTOM:
//			m_grabbed = GF_TRUE;
			break;
		case TB_THUMBPOSITION:
		case TB_THUMBTRACK:
			m_grabbed = GF_TRUE;
			break;
		case TB_ENDTRACK:
			if (m_grabbed) {
				if (!app->can_seek || !app->m_isopen) {
					m_PosSlider.SetPos(0);
				} else {
					u32 range = m_PosSlider.GetRangeMax() - m_PosSlider.GetRangeMin();
					u32 seek_to = m_PosSlider.GetPos();
					app->PlayFromTime(seek_to);
				}
				m_grabbed = GF_FALSE;
			}
			break;
		}
	}
	if (pScrollBar->GetDlgCtrlID() == ID_AUDIO_VOL) {
		u32 vol = m_AudioVol.GetPos();
		gf_term_set_option(app->m_term, GF_OPT_AUDIO_VOLUME, vol);
	}
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


void Sliders::setLayout(slider_type type){
	switch(type){
	case NONE:
		m_PosSlider.ShowWindow(SW_HIDE);
		m_AudioMute.ShowWindow(SW_HIDE);
		m_AudioVol.ShowWindow(SW_HIDE);
		m_Play.ShowWindow(SW_HIDE);
		m_Nav_Prev.ShowWindow(SW_HIDE);
		m_Nav_Next.ShowWindow(SW_HIDE);
		m_cur_page.ShowWindow(SW_HIDE);
		m_page_count.ShowWindow(SW_HIDE);
		break;

	case NAVIGATION:
		m_PosSlider.ShowWindow(SW_HIDE);
		m_AudioMute.ShowWindow(SW_HIDE);
		m_AudioVol.ShowWindow(SW_HIDE);
		m_Play.ShowWindow(SW_HIDE);
		m_Nav_Prev.ShowWindow(SW_SHOW);
		m_Nav_Next.ShowWindow(SW_SHOW);
		m_cur_page.ShowWindow(SW_SHOW);
		m_page_count.ShowWindow(SW_SHOW);
		updateNav();
		break;

	case PLAY:
		m_Nav_Prev.ShowWindow(SW_HIDE);
		m_Nav_Next.ShowWindow(SW_HIDE);
		m_cur_page.ShowWindow(SW_HIDE);
		m_page_count.ShowWindow(SW_HIDE);
		m_PosSlider.ShowWindow(SW_SHOW);
		m_AudioMute.ShowWindow(SW_SHOW);
		m_AudioVol.ShowWindow(SW_SHOW);
		m_Play.ShowWindow(SW_SHOW);
		updatePlay();
		break;
	}
	this->Invalidate();
	this->UpdateWindow();
}

void Sliders::OnBnClickedNext()
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

void Sliders::OnBnClickedPrev(){
	Osmo4 *app = GetApp();
	CMainFrame *pFrame = (CMainFrame *) app->m_pMainWnd;
	Playlist *pPlayList = pFrame->m_pPlayList;
	if (!pPlayList) return;

	if (pPlayList->m_cur_entry<=0) return;
	pPlayList->PlayPrev();
	updateNav();
}

void  Sliders::updatePlay(){
	Osmo4 *app = GetApp();

	if (app->isPlaying())
		setPlay();
	else
		setPause();
}

void  Sliders::updateNav(){
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
	SetDlgItemText(IDC_PAGE_COUNT, value);
}

void Sliders::onSizeSliders(UINT nType, int cx, int cy){
	
	if (!m_PosSlider.m_hWnd) return;
	RECT rc, rc2, rc3, rc4;


	//u32 tw = 40;
	//Maja changed to space for mute button
	u32 tw = 140; //80 worked OK
	// Maja hardcoded mute button width
	u32 mutewidth = 32;

	//m_PosSlider.GetClientRect(&rc);
	//rc.right = rc.left + cx;
	//m_PosSlider.SetWindowPos(this, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOMOVE);


	m_PosSlider.GetClientRect(&rc);
	rc.right = rc.left + cx - tw;
	rc.top += 10;
	rc.bottom += 10;

	const UINT nPixelsLength = 24;
	m_PosSlider.ModifyStyle(0,TBS_FIXEDLENGTH,FALSE);
	m_PosSlider.SendMessage(TBM_SETTHUMBLENGTH,nPixelsLength,0); 


	
	//Maja added mute button move
	m_AudioMute.GetClientRect(&rc3);
	rc3.top = rc3.bottom = cy/2;
	rc3.top -= cy/3;
	rc3.bottom += cy/3;
	rc3.left = rc.right +  mutewidth + 5;
	rc3.right = rc3.left+mutewidth;
	
	m_Play.GetWindowRect(&rc4);
	rc4.top = rc4.bottom = cy/2;
	rc4.top -= cy/2;
	rc4.bottom += cy/2;
	rc4.left = 10;
	rc4.right = rc4.left + mutewidth;

	m_AudioVol.GetClientRect(&rc2);
	rc2.top = rc2.bottom = cy/2;
	rc2.top -= cy/3;
	rc2.bottom += cy/3;
	rc2.left = rc.right + mutewidth+mutewidth;
	rc2.right = rc.right+tw;


	//m_Play.SetWindowPos(this, rc4.left, rc4.top, rc4.right, rc4.bottom, SWP_NOZORDER | SWP_NOMOVE);
	m_PosSlider.SetWindowPos(this, rc.left, rc.top, rc.right, rc.bottom, SWP_NOZORDER | SWP_NOMOVE);
	m_Play.MoveWindow(&rc4);
	m_AudioVol.MoveWindow(&rc2);
	m_AudioMute.MoveWindow(&rc3);
}

void Sliders::onSizeNav(UINT nType, int cx, int cy){
	if (!m_Nav_Prev.m_hWnd) return;
	RECT rc, rc2, rc3, rc4;
	
	u32 navWidth = 32;
	u32 margin = 40;
	u32 pageSz = 30;

	m_Nav_Prev.GetWindowRect(&rc4);
	rc4.top = rc4.bottom = cy/2;
	rc4.top -= cy/2;
	rc4.bottom += cy/2;
	rc4.left = margin;
	rc4.right = rc4.left + navWidth;


	m_Nav_Next.GetWindowRect(&rc3);
	rc3.top = rc3.bottom = cy/2;
	rc3.top -= cy/2;
	rc3.bottom += cy/2;
	rc3.right = cx - margin;
	rc3.left = rc3.right - navWidth;

	m_cur_page.GetClientRect(&rc2);
	rc2.top = rc2.bottom = cy/2;
	rc2.top -= cy/3;
	rc2.bottom += cy/3;
	rc2.left = cx/2 - pageSz;
	rc2.right = rc2.left + pageSz;

	
	
	m_page_count.GetClientRect(&rc);
	rc.top = rc.bottom = cy/3;
	//rc.top -= cy/3;
	rc.bottom += cy/3;
	rc.left = rc2.right+5;
	rc.right = rc.left+ pageSz;

	m_Nav_Prev.MoveWindow(&rc4);
	m_Nav_Next.MoveWindow(&rc3);
	m_cur_page.MoveWindow(&rc2);
	m_page_count.MoveWindow(&rc);
}

void Sliders::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

	onSizeSliders(nType, cx, cy);
	onSizeNav(nType, cx, cy);
}

/*we sure don't want to close this window*/
void Sliders::OnClose() 
{
	u32 i = 0;
	return;
}

BOOL Sliders::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYDOWN) {
		GetApp()->m_pMainWnd->SetFocus();
		GetApp()->m_pMainWnd->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
		return TRUE;
	}
	return CDialog::PreTranslateMessage(pMsg);
}


BOOL Sliders::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_AudioVol.SetRange(0, 100);
	AUDIO_MUTED = false;

	// -- Get CButton pointer of the volume button
	CButton * pButton;
	pButton = (CButton *)GetDlgItem(IDC_AUDIO_MUTE);
	if ( pButton && pButton->GetSafeHwnd() )
	{
	pButton->SetBitmap((HBITMAP)LoadImage(AfxGetApp()->m_hInstance,
      MAKEINTRESOURCE(IDB_VOLUME),
      IMAGE_BITMAP, 16, 16, LR_COLOR));
 }

	pButton = (CButton *)GetDlgItem(IDC_PLAY);
	if ( pButton && pButton->GetSafeHwnd() )
	{
	pButton->SetBitmap((HBITMAP)LoadImage(AfxGetApp()->m_hInstance,
      MAKEINTRESOURCE(IDB_PLAY),
      IMAGE_BITMAP, 28, 28, LR_COLOR));
	}


	pButton;
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

void Sliders::setPlay(){
		CButton * pButton;
		pButton = (CButton *)GetDlgItem(IDC_PLAY);
		if ( pButton && pButton->GetSafeHwnd() )
		{
		pButton->SetBitmap((HBITMAP)LoadImage(AfxGetApp()->m_hInstance,
		 MAKEINTRESOURCE(IDB_PAUSE),
		IMAGE_BITMAP, 28, 28, LR_COLOR));
		}
}

void Sliders::setPause(){
		CButton * pButton;
		pButton = (CButton *)GetDlgItem(IDC_PLAY);
		if ( pButton && pButton->GetSafeHwnd() )
		{
		pButton->SetBitmap((HBITMAP)LoadImage(AfxGetApp()->m_hInstance,
		 MAKEINTRESOURCE(IDB_PLAY),
		IMAGE_BITMAP, 28, 28, LR_COLOR));
		}
}

void Sliders::SetVolume()
{
	m_AudioVol.SetPos(gf_term_get_option(GetApp()->m_term, GF_OPT_AUDIO_VOLUME));
}

void Sliders::OnBnClickedPlay(){
	Osmo4 *app = GetApp();

	app->OnFilePlay();

	updatePlay();
}

void Sliders::OnBnClickedAudioMute()
{

	Osmo4 *app = GetApp();

	// TODO: Add your control notification handler code here
	// if MUTE, then unmute
	if (AUDIO_MUTED)
	{
		u32 vol = m_AudioVol.GetPos();
		gf_term_set_option(app->m_term, GF_OPT_AUDIO_VOLUME, vol);
		// -- Get CButton pointer of the volume button
		CButton * pButton;
		pButton = (CButton *)GetDlgItem(IDC_AUDIO_MUTE);
		if ( pButton && pButton->GetSafeHwnd() )
		{
		pButton->SetBitmap((HBITMAP)LoadImage(AfxGetApp()->m_hInstance,
		 MAKEINTRESOURCE(IDB_VOLUME),
		IMAGE_BITMAP, 16, 16, LR_COLOR));
		}
		AUDIO_MUTED = false;
	}
	// if not MUTE, then mute
	else
	{
		gf_term_set_option(app->m_term, GF_OPT_AUDIO_VOLUME, 0);
		// -- Get CButton pointer of the volume button
		CButton * pButton;
		pButton = (CButton *)GetDlgItem(IDC_AUDIO_MUTE);
		if ( pButton && pButton->GetSafeHwnd() )
		{
		pButton->SetBitmap((HBITMAP)LoadImage(AfxGetApp()->m_hInstance,
		 MAKEINTRESOURCE(IDB_VOLUME_MUTED),
		IMAGE_BITMAP, 16, 16, LR_COLOR));
		}
		AUDIO_MUTED = true;
	}
}


void Sliders::OnNMCustomdrawAudioVol(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
