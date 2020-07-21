// MotionSplitterDlg.h : header file
//

#if !defined(AFX_MOTIONSPLITTERDLG_H__D1C6E57D_1A8F_481D_AE10_96D2267CB339__INCLUDED_)
#define AFX_MOTIONSPLITTERDLG_H__D1C6E57D_1A8F_481D_AE10_96D2267CB339__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CMotionSplitterDlg dialog

#include "motionsplit.h"

class CMotionSplitterDlg : public CDialog
{
// Construction
public:
	CMotionSplitterDlg(CWnd* pParent = NULL);	// standard constructor

	CxImage *m_pImage;
	CDC dcMem;
	int iWidth,iHeight;

	void ShowImages();
	void InitDisplayMemory();	
	

// Dialog Data
	//{{AFX_DATA(CMotionSplitterDlg)
	enum { IDD = IDD_MOTIONSPLITTER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMotionSplitterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CMotionSplitterDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOTIONSPLITTERDLG_H__D1C6E57D_1A8F_481D_AE10_96D2267CB339__INCLUDED_)
