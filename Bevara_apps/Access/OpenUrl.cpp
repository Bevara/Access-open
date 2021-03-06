// OpenUrl.cpp : implementation file
//

#include "stdafx.h"
#include "Osmo4.h"
#include "OpenUrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpenUrl dialog


COpenUrl::COpenUrl(CWnd* pParent /*=NULL*/)
	: CDialog(COpenUrl::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpenUrl)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void COpenUrl::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpenUrl)
	DDX_Control(pDX, IDC_COMBOURL, m_URLs);
	DDX_Control(pDX, IDC_FORCE_ACC, m_UseAcc);
	DDX_Control(pDX, IDC_ACC_LOC, m_AccLoc);
	DDX_Control(pDX, IDC_SELECT_ACC, m_SelectAcc);
	DDX_Control(pDX, IDC_STATIC_ACC, m_AccTxt);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpenUrl, CDialog)
	//{{AFX_MSG_MAP(COpenUrl)
	ON_BN_CLICKED(IDC_BUTGO, OnButgo)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_FORCE_ACC, &COpenUrl::OnBnClickedForceAcc)
	ON_BN_CLICKED(IDC_LOCATION_FILE, &COpenUrl::OnBnClickedLocationFile)
	ON_BN_CLICKED(IDC_SELECT_ACC, &COpenUrl::OnBnClickedSelectAcc)
	ON_BN_CLICKED(IDC_CANCEL, &COpenUrl::OnBnClickedCancel)
END_MESSAGE_MAP()




#define MAX_LAST_FILES		20
void UpdateLastFiles(GF_Config *cfg, const char *URL)
{
	u32 nb_entries;
	gf_cfg_set_key(cfg, "RecentFiles", URL, NULL);
	gf_cfg_insert_key(cfg, "RecentFiles", URL, "", 0);
	/*remove last entry if needed*/
	nb_entries = gf_cfg_get_key_count(cfg, "RecentFiles");
	if (nb_entries>MAX_LAST_FILES) {
		gf_cfg_set_key(cfg, "RecentFiles", gf_cfg_get_key_name(cfg, "RecentFiles", nb_entries-1), NULL);
	}
}


/////////////////////////////////////////////////////////////////////////////
// COpenUrl message handlers

void COpenUrl::OnButgo() 
{
	CString URL;
	int sel = m_URLs.GetCurSel();
	if (sel == CB_ERR) {
		m_URLs.GetWindowText(URL);
	} else {
		m_URLs.GetLBText(sel, URL);
	}
	if (!URL.GetLength()) {
		EndDialog(IDCANCEL);
		return;
	}

	Osmo4 *gpac = GetApp();

	gf_cfg_set_key(gpac->m_user.config, "Accessor", "File", "");

	m_url = URL;
	CT2A URL_ch(URL);
	UpdateLastFiles(gpac->m_user.config, (const char *)URL_ch);

	if (!m_UseAcc.GetCheck()) {
		EndDialog(IDOK);
		return;
	}

	sel = m_AccLoc.GetCurSel();
	if (sel == CB_ERR) {
		m_AccLoc.GetWindowText(URL);
	}
	else {
		m_AccLoc.GetLBText(sel, URL);
	}
	if (!URL.GetLength()) {
		EndDialog(IDOK);
		return;
	}
	CT2A ACC_ch(URL);
	gf_cfg_set_key(gpac->m_user.config, "Accessor", "File", ACC_ch);
	EndDialog(IDOK);
}

void COpenUrl::OnBnClickedCancel()
{
	EndDialog(IDCANCEL);
	return;
}


BOOL COpenUrl::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	Osmo4 *gpac = GetApp();
	u32 i=0;

	while (m_URLs.GetCount()) m_URLs.DeleteString(0);
	while (1) {
		const char *sOpt = gf_cfg_get_key_name(gpac->m_user.config, "RecentFiles", i);
		if (!sOpt) break;
		m_URLs.AddString(CString(sOpt));
		i++;
	}
	return TRUE;
}


void COpenUrl::OnBnClickedForceAcc()
{
	int show = m_UseAcc.GetCheck() ? SW_SHOW : SW_HIDE;
	
	m_AccTxt.ShowWindow(show);
	m_AccLoc.ShowWindow(show);
	m_SelectAcc.ShowWindow(show);
	m_AccTxt.ShowWindow(show);
}


void COpenUrl::OnBnClickedLocationFile()
{
	Osmo4 *gpac = GetApp();

	CString sFiles = gpac->GetFileFilter();

	/*looks like there's a bug here, main filter isn't used correctly while the others are*/
	CFileDialog fd(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, sFiles);
	fd.m_ofn.nMaxFile = 25000;
	fd.m_ofn.lpstrFile = (wchar_t *)gf_malloc(sizeof(wchar_t) * fd.m_ofn.nMaxFile);
	fd.m_ofn.lpstrFile[0] = 0;

	if (fd.DoModal() != IDOK) {
		gf_free(fd.m_ofn.lpstrFile);
		return;
	}
	CString file = fd.GetPathName();
	m_URLs.SetWindowTextW(file);
}


void COpenUrl::OnBnClickedSelectAcc()
{
	Osmo4 *gpac = GetApp();

	/*looks like there's a bug here, main filter isn't used correctly while the others are*/
	CFileDialog fd(TRUE, _T("bc"), _T("*.bc"), OFN_HIDEREADONLY | OFN_FILEMUSTEXIST, _T("Bitcode Files (*.bc)|*.bc|Text Files (*.ll)|*.ll|All Files (*.*)|*.*||"));
	fd.m_ofn.nMaxFile = 25000;
	fd.m_ofn.lpstrFile = (wchar_t *)gf_malloc(sizeof(wchar_t) * fd.m_ofn.nMaxFile);
	fd.m_ofn.lpstrFile[0] = 0;

	if (fd.DoModal() != IDOK) {
		gf_free(fd.m_ofn.lpstrFile);
		return;
	}
	CString file = fd.GetPathName();
	m_AccLoc.SetWindowTextW(file);
}
