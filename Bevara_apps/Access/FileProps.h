#if !defined(AFX_FILEPROPS_H__CA4484B4_0301_4BE1_8736_551250121C3F__INCLUDED_)
#define AFX_FILEPROPS_H__CA4484B4_0301_4BE1_8736_551250121C3F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileProps.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFileProps dialog

class CFileProps : public CDialog
{
// Construction
public:
	CFileProps(CWnd* pParent = NULL);   // standard constructor
	BOOL Create(CWnd * pParent)
	{
		return CDialog::Create( CFileProps::IDD, pParent);
	}

// Dialog Data
	//{{AFX_DATA(CFileProps)
	enum { IDD = IDD_PROPERTIES };
	CEdit	m_ODInfo;
	//}}AFX_DATA
	void SetGeneralInfo();

private:
	GF_ObjectManager *current_odm;
	CString m_comments;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileProps)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFileProps)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedSave();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILEPROPS_H__CA4484B4_0301_4BE1_8736_551250121C3F__INCLUDED_)
