// InterFace.h: interface for the InterFace class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INTERFACE_H__40D85B53_E446_4430_AD68_F07E7AC87F72__INCLUDED_)
#define AFX_INTERFACE_H__40D85B53_E446_4430_AD68_F07E7AC87F72__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define n_AreaSizeX 280 
#define n_AreaSizeY 170

#define pi 3.14159265

#define UP		1
#define DOWN	2
#define LEFT	3
#define RIGHT	4
#define HOME	5
#define ZOOMIN	6
#define ZOOMOUT 7

#include "parameters.h"

class CInterFace  
{
public:
	CInterFace();
	virtual ~CInterFace();

	// Video
	void LoadVideo(CString strServerName,CString strID,CString strPassWd,CString strFile);

	// Control
	int px,py;
	int Width,Height;

	int x;
	int y;
	double theta_p;
	double type;
	double zoom_level;
	double length;

	CInternetSession m_Session;
	CHttpConnection *m_pConnection;
	CHttpFile *m_pFile;
	CString strServerName,id,passwd;

	int m_nPanSpeed,m_nTiltSpeed,m_nZoomSpeed;

	void Initialize();
	void AdjustCamera(double c_angle_z[7]);
	void ExecuteOrder(CString Order,CString strServerName,CString id,CString passwd);
	void CtrlCamera(int ctrl,double c_angle_z[7]);
};

#endif // !defined(AFX_INTERFACE_H__40D85B53_E446_4430_AD68_F07E7AC87F72__INCLUDED_)
