// MotionSplitter.h : main header file for the MOTIONSPLITTER application
//

#if !defined(AFX_MOTIONSPLITTER_H__28359035_C142_489B_AB53_DA660967B404__INCLUDED_)
#define AFX_MOTIONSPLITTER_H__28359035_C142_489B_AB53_DA660967B404__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMotionSplitterApp:
// See MotionSplitter.cpp for the implementation of this class
//

class CMotionSplitterApp : public CWinApp
{
public:
	CMotionSplitterApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMotionSplitterApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMotionSplitterApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MOTIONSPLITTER_H__28359035_C142_489B_AB53_DA660967B404__INCLUDED_)
