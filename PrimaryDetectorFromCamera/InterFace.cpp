// InterFace.cpp: implementation of the InterFace class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InterFace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CInterFace::CInterFace()
{
	Width = ImageWidth/2;
	Height = ImageHeight/2;
}

CInterFace::~CInterFace()
{

}

void CInterFace::LoadVideo(CString strServerName,CString strID,CString strPassWd,CString strFile)
{
	CInternetSession m_Session;
	CHttpConnection *m_pConnection;
	CHttpFile *m_pFile;

	INTERNET_PORT nPort = 80;
	
	m_pConnection = m_Session.GetHttpConnection(strServerName,nPort,strID,strPassWd);
	if (!m_pConnection)
	{
		AfxMessageBox("Fail to connect", MB_OK);
		m_pConnection = NULL;
		return;
	}

    if (m_pConnection != NULL)
    {
        m_pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET,
			"/cgi-bin/video.jpg",NULL,1,NULL,NULL,INTERNET_FLAG_RELOAD); 
		
        if (m_pFile != NULL)
        {
            DWORD dwStatus;

			m_pFile->AddRequestHeaders("User-Agent: GetWebFile/1.0\r\n",
				HTTP_ADDREQ_FLAG_ADD_IF_NEW);

            m_pFile->SendRequest();
            m_pFile->QueryInfoStatusCode(dwStatus);

			if (dwStatus == HTTP_STATUS_OK)
			{
				char buf[2000];
				int numread;
				CString filepath;
				filepath.Format("%s", strFile);
				CFile myfile(filepath,CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);
				while ((numread = m_pFile->Read(buf,sizeof(buf)-1)) > 0)
				{
					buf[numread] = '\0';
					strFile += buf;
					myfile.Write(buf, numread);
				}
				myfile.Close();
			}
        }
    }
	delete m_pFile;
	delete m_pConnection;
}

void CInterFace::Initialize()
{
	Width = ImageWidth/2;
	Height = ImageHeight/2;

	// Initializa Camera Postion
	CString Order;	

//	Order = "/cgi-bin/camctrl.cgi?move=home";
//	ExecuteOrder(Order,strServerName1,id1,passwd1);
//	ExecuteOrder(Order,strServerName2,id2,passwd2);
	
	Order.Format("/cgi-bin/camctrl.cgi?speedpan=%d",m_nPanSpeed);
	ExecuteOrder(Order,strServerName,id,passwd);
	Order.Format("/cgi-bin/camctrl.cgi?speedtilt=%d",m_nTiltSpeed);
	ExecuteOrder(Order,strServerName,id,passwd);
	Order.Format("/cgi-bin/camctrl.cgi?speedzoom=%d",m_nZoomSpeed);
	ExecuteOrder(Order,strServerName,id,passwd);
} 

void CInterFace::CtrlCamera(int ctrl,double c_angle_z[7])
{
	CString Order;

	if(ctrl==UP) Order = "/cgi-bin/camctrl.cgi?move=up";
	else if(ctrl==DOWN) Order = "/cgi-bin/camctrl.cgi?move=down";
	else if(ctrl==LEFT) {
		double dep=0;
/*		if(zoom_level<=3) dep = angle_z1to3[m_nPanSpeed+5];
		else if(zoom_level>3 && zoom_level<=7) dep = angle_z4to7[m_nPanSpeed+5];
		else if(zoom_level>7) dep = angle_z8to10[m_nPanSpeed+5];	*/
		dep = c_angle_z[m_nPanSpeed+5];

		theta_p -= dep;
		Order = "/cgi-bin/camctrl.cgi?move=right";
	}
	else if(ctrl==RIGHT) {
		double dep=0;
/*		if(zoom_level<=3) dep = angle_z1to3[m_nPanSpeed+5];
		else if(zoom_level>3 && zoom_level<=7) dep = angle_z4to7[m_nPanSpeed+5];
		else if(zoom_level>7) dep = angle_z8to10[m_nPanSpeed+5];	*/
		dep = c_angle_z[m_nPanSpeed+5];

		theta_p += dep;

		Order = "/cgi-bin/camctrl.cgi?move=left";
	}
	else if(ctrl==HOME) {
		// Upate Camera Angle
		theta_p = 0;
		zoom_level = 1;

		Order = "/cgi-bin/camctrl.cgi?move=home";
	}
	else if(ctrl==ZOOMIN) {
		// Update zoom level
		zoom_level++;
		if(zoom_level>10) zoom_level = 10;

		Order = "/cgi-bin/camctrl.cgi?zoom=tele";
	}
	else if(ctrl==ZOOMOUT) {
		// Update zoom level
		zoom_level--;
		if(zoom_level<1) zoom_level = 1;

		Order = "/cgi-bin/camctrl.cgi?zoom=wide";
	}

	ExecuteOrder(Order,strServerName,id,passwd);
}

void CInterFace::AdjustCamera(double c_angle_z[7])
{
	CString Order;

	int delta_w,delta_h;
	double dep=0;

/*	if(c.zoom_level<=3) dep = angle_z1to3[m_nPanSpeed+5];
	else if(c.zoom_level>3 && c.zoom_level<=7) dep = angle_z4to7[m_nPanSpeed+5];
	else if(c.zoom_level>7) dep = angle_z8to10[m_nPanSpeed+5];	*/
	dep = c_angle_z[m_nPanSpeed+5];

	if(!(px==0 && py==0)) {
		delta_w = Width/2 - px;
		delta_h = Height/2 - py;

		if(abs(delta_w)>50) {
			if(abs(delta_w)<=150) m_nPanSpeed = -5;
			else if(abs(delta_w)<=250) m_nPanSpeed = -3;
			else if(abs(delta_w)<=350) m_nPanSpeed = -1;

			Order.Format("/cgi-bin/camctrl.cgi?speedpan=%d",m_nPanSpeed);
			ExecuteOrder(Order,strServerName,id,passwd);

			if(delta_w>0) {
				Order = "/cgi-bin/camctrl.cgi?move=left";
				ExecuteOrder(Order,strServerName,id,passwd);		

				theta_p += dep;
			}
			else if(delta_w<0) {
				Order = "/cgi-bin/camctrl.cgi?move=right";
				ExecuteOrder(Order,strServerName,id,passwd);		

				theta_p -= dep;
			}
		}

		if(abs(delta_h)>100) {
			if(delta_h<=150) m_nTiltSpeed = 2;
			else if(delta_h>250) m_nTiltSpeed = 5;

			Order.Format("/cgi-bin/camctrl.cgi?speedtilt=%d",m_nPanSpeed);
			ExecuteOrder(Order,strServerName,id,passwd);

			if(delta_h>0) {
				Order = "/cgi-bin/camctrl.cgi?move=up";
				ExecuteOrder(Order,strServerName,id,passwd);		
			}
			else if(delta_h<0) {
				Order = "/cgi-bin/camctrl.cgi?move=down";
				ExecuteOrder(Order,strServerName,id,passwd);		
			}
		}
	}
}

void CInterFace::ExecuteOrder(CString Order,CString strServerName,CString id,CString passwd)
{
	INTERNET_PORT nPort = 80;
	
	m_pConnection = m_Session.GetHttpConnection(strServerName,nPort,id,passwd);
	if (!m_pConnection)
	{
		AfxMessageBox("Fail to connect", MB_OK);
		m_pConnection = NULL;
		return;
	}

    if (m_pConnection != NULL)
    {
		m_pFile = m_pConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET,
			Order,NULL,1,NULL,NULL,INTERNET_FLAG_RELOAD); 

		m_pFile->SendRequest();
    }
	delete m_pFile;
	delete m_pConnection;
}

