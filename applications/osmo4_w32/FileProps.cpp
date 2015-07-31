// FileProps.cpp : implementation file
//

#include "stdafx.h"
#include "osmo4.h"
#include "FileProps.h"
#include "MainFrm.h"
#include "BevaraInfo.h"
#include "Media.h"
#include "BevaraContainer.h"

/*ISO 639 languages*/
#include <gpac/iso639.h>
#include <gpac/isomedia.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileProps dialog


CFileProps::CFileProps(CWnd* pParent /*=NULL*/)
	: CDialog(CFileProps::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFileProps)
	//}}AFX_DATA_INIT
}


void CFileProps::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFileProps)
	DDX_Control(pDX, IDC_ODINFO, m_ODInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFileProps, CDialog)
	//{{AFX_MSG_MAP(CFileProps)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDSAVE, &CFileProps::OnBnClickedSave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileProps message handlers


#define FP_TIMER_ID	20

BOOL CFileProps::OnInitDialog() 
{
	CDialog::OnInitDialog();
	current_odm = NULL;

	SetGeneralInfo();
	SetTimer(FP_TIMER_ID, 500, NULL);

	return TRUE;
}

void CFileProps::OnClose() 
{
	KillTimer(FP_TIMER_ID);
	DestroyWindow();
}

void CFileProps::OnDestroy() 
{
	CDialog::OnDestroy();
	delete this;
	((CMainFrame *)GetApp()->m_pMainWnd)->m_pProps = NULL;
}



void CFileProps::SetGeneralInfo()
{
	/* TODO : unify access in mainframe */
	CString in;
	CString out;
	char* version = NULL;
	char* comments = NULL;
	char* forg = NULL;
	char* fext = NULL;
	u32 o4CC = 0;
	Osmo4* app;
	BevaraContainer* container;

	char sText[5000];
    sprintf(sText, "%s Metadata", ((CMainFrame*)GetApp()->m_pMainWnd)->m_pPlayList->GetDisplayName());
    SetWindowText(CString(sText));
	
	app = GetApp();
	if (!app->media) return;
	if (app->media->getType() != Media::BEVARA_CONTAINER) return;
	container = (BevaraContainer*)app->media;
	m_comments = CString(container->getMetadata());
	m_ODInfo.SetWindowText(m_comments);
}


void CFileProps::OnBnClickedSave()
{
	TCHAR szFilters[]= _T("TXT Files (*.txt)|*.txt|All Files (*.*)|*.*||");

	CFileDialog fileDlg(FALSE, _T("txt"), NULL,  OFN_OVERWRITEPROMPT, szFilters);

	if(fileDlg.DoModal() == IDOK)
	{
		CFile	outFile;
		if ( !outFile.Open(fileDlg.GetPathName(), CFile::modeCreate |   CFile::modeReadWrite ) ){
			AfxMessageBox(_T("Can't create output file!"));
			return;
		}
		outFile.Write( m_comments, m_comments.GetLength() ); 
		outFile.Close();
	}
}
