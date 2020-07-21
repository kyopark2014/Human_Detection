// HumanTrackerDlg.h : header file
//

#if !defined(AFX_HUMANTRACKERDLG_H__D15349CC_ECA9_40B5_AC3C_48100FC0EC90__INCLUDED_)
#define AFX_HUMANTRACKERDLG_H__D15349CC_ECA9_40B5_AC3C_48100FC0EC90__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CHumanTrackerDlg dialog

#include "motiontracker.h"
#include "detection.h"
#include "interface.h"
#include "localtracker.h"

class CHumanTrackerDlg : public CDialog
{
// Construction
public:
	CHumanTrackerDlg(CWnd* pParent = NULL);	// standard constructor

	CxImage *m_pImage;
	int iWidth,iHeight;
	int Width,Height;
	CDC dcMem;

	CMotionTracker *mt;
	CDetection *det;
	CLocalTracker *lt;   

	CANDIDATE target[MTG][NTG]; 

	void HumanDetection(int t,CString ImageFile);
	void InitDisplayMemory();
	void ShowImages();

	void SaveTargetInfo(int num,int t);
	void ShowDetectionResult(int num);
	void SaveDetectionResult(int num);

	// Show local tracking result
	void ShowLocalTrackingResult(int num);
	void SaveLocalTrackingResult(int num);

	// camera
	CInterFace c1;
	void LoadCameraImage(CString ImageFile);
	int cnt;

	// Reduced Complexity
	int rate;
	MOTIONCANDIDATE m_candidate[MTG];
	void InitProcessingMemory();
	void FinalizeMemory();
	void LoadBackgroundColorImageFromFile(CString ImageFile);

// Dialog Data
	//{{AFX_DATA(CHumanTrackerDlg)
	enum { IDD = IDD_HUMANTRACKER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHumanTrackerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CHumanTrackerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HUMANTRACKERDLG_H__D15349CC_ECA9_40B5_AC3C_48100FC0EC90__INCLUDED_)
