// HumanTracker.h : main header file for the HUMANTRACKER application
//

#if !defined(AFX_HUMANTRACKER_H__F45852C3_108F_472C_BB78_ADC1D2641D9E__INCLUDED_)
#define AFX_HUMANTRACKER_H__F45852C3_108F_472C_BB78_ADC1D2641D9E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CHumanTrackerApp:
// See HumanTracker.cpp for the implementation of this class
//

class CHumanTrackerApp : public CWinApp
{
public:
	CHumanTrackerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHumanTrackerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CHumanTrackerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUMANTRACKER_H__F45852C3_108F_472C_BB78_ADC1D2641D9E__INCLUDED_)
