#include "afxwin.h"
#if !defined(AFX_NAV_H__)
#define AFX_NAV_H__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Sliders.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Sliders dialog

class Navigation : public CDialog
{
// Construction
public:
	Navigation(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(Sliders)
	enum { IDD = IDD_NAV };
	CButton m_Nav_Prev;
	CButton m_Nav_Next;
	CEdit m_cur_page;
	CStatic m_page_count;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Sliders)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(Sliders)
	afx_msg void OnSize(UINT nType, int cx, int cy);

	afx_msg void OnClose();
	afx_msg void OnBnClickedNext();
	afx_msg  void OnBnClickedPrev();

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	void hideNav();
	void showNav();

private:
	void updateNav();

private:
	bool isShow;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SLIDERS_H__3542255E_1376_4FB7_91E7_B4841BB4F173__INCLUDED_)
