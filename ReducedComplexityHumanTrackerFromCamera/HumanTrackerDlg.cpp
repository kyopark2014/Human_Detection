// HumanTrackerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HumanTracker.h"
#include "HumanTrackerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int **motion;
int ***img_back;
int ***img_current;
int ***img_previous;

/////////////////////////////////////////////////////////////////////////////
// CHumanTrackerDlg dialog

CHumanTrackerDlg::CHumanTrackerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHumanTrackerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHumanTrackerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	cnt = 0;	
}

void CHumanTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHumanTrackerDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CHumanTrackerDlg, CDialog)
	//{{AFX_MSG_MAP(CHumanTrackerDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHumanTrackerDlg message handlers

BOOL CHumanTrackerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_pImage = new CxImage;	

	iWidth = 704;
	iHeight = 480;
	Width = iWidth;
	Height = iHeight;

	SetTimer(0,100,NULL);

	// initialize
	InitDisplayMemory();
	// Set size of windows
	SetWindowPos(NULL,0,0,Width,Height+30,SWP_NOMOVE|SWP_SHOWWINDOW); // Adjust testbed size

	// camera
	int camera_id = 0;
	if(camera_id==0) {
		c1.strServerName = "129.49.69.81";
		c1.id = "root";
		c1.passwd  = "msdl007";
	}
	else if(camera_id==1) {
		c1.strServerName = "129.49.69.89";
		c1.id = "root";
		c1.passwd  = "msdl007";
	}
	else if(camera_id==2) {
		c1.strServerName = "116.89.168.148";
		c1.id = "root";
		c1.passwd  = "0813";
	}
	else if(camera_id==3) {
		c1.strServerName = "116.89.168.149";
		c1.id = "root";
		c1.passwd  = "0813";
	}
	else if(camera_id==4) {
		c1.strServerName = "116.89.168.150";
		c1.id = "root";
		c1.passwd  = "0813";
	}

	// Initialize Camera
	c1.m_nPanSpeed=-1, c1.m_nTiltSpeed=5, c1.m_nZoomSpeed=1;
	c1.Initialize();

	// Reduced Complexity Rate
	rate = 4;
	InitProcessingMemory();

	// define MotionTracker
	mt = new CMotionTracker(rate);

	// initialize
	mt->Initialize();

	// define localtracker
	lt = new CLocalTracker();
	
	// set background image
	CString ImageFile;
	cnt = 0;
	ImageFile.Format("video_%02d.jpg",cnt%2);
	LoadCameraImage(ImageFile);
	LoadBackgroundColorImageFromFile(ImageFile);	

	// first detection
	ImageFile.Format("video_%02d.jpg",cnt%2);
	LoadCameraImage(ImageFile);		
	HumanDetection(cnt,ImageFile);	
	ShowImages(); 
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CHumanTrackerDlg::LoadBackgroundColorImageFromFile(CString ImageFile) {
	CxImage *m_pImageFile = new CxImage;	
	m_pImageFile->Load(ImageFile, CxImage::FindType(ImageFile));	
	int i,j;

	RGBQUAD color;
	int m=0,n=0;
	for(j=0;j<iHeight;j+=rate) {
		m = 0;
		for(i=0;i<iWidth;i+=rate) {		
			color = m_pImageFile->GetPixelColor(i,j);
			
			mt->img_back[0][m][n] = color.rgbRed;
			mt->img_back[1][m][n] = color.rgbGreen;
			mt->img_back[2][m][n] = color.rgbBlue;
			m++;
		}
		n++;
	}

	delete m_pImageFile;
}

void CHumanTrackerDlg::LoadCameraImage(CString ImageFile)
{
	c1.LoadVideo(c1.strServerName,c1.id,c1.passwd,ImageFile); 
}

void CHumanTrackerDlg::HumanDetection(int num,CString ImageFile) {
	int i,j,t,k,m,n;

	// initialize target information
	for(t=0;t<MTG;t++) {
		for(k=0;k<NTG;k++) {
			target[t][k].bx0 = 0;
			target[t][k].by0 = 0;
			target[t][k].bx1 = 0;
			target[t][k].by1 = 0;
			target[t][k].px = 0;
			target[t][k].py = 0;
			target[t][k].valid = 0;
		}
	}

		// load current image
	m_pImage->Load(ImageFile, CxImage::FindType(ImageFile));

	// save current image
	for(k=0;k<3;k++) {
		for(i=0;i<iWidth;i++) {
			for(j=0;j<iHeight;j++) {			
				img_previous[k][i][j] = img_current[k][i][j];
			}
		}
	}  
	// reduced complexity	
	for(k=0;k<3;k++) {
		for(j=0;j<iHeight/rate;j++) {		
			for(i=0;i<iWidth/rate;i++) {			
				mt->img_previous[k][i][j] = mt->img_current[k][i][j];
			}
		}		
	}  

	RGBQUAD color;
	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			color = m_pImage->GetPixelColor(i,j);
			
			img_current[0][i][j] = color.rgbRed;
			img_current[1][i][j] = color.rgbGreen;
			img_current[2][i][j] = color.rgbBlue;
		}
	}

	// reduced complexity	
	for(k=0;k<3;k++) {
		m=0,n=0;	
		for(j=0;j<iHeight;j+=rate) {		
			m = 0;
			for(i=0;i<iWidth;i+=rate) {			
				mt->img_current[k][m][n] = img_current[k][i][j];
				m++;
			}
			n++;	
		}		
	}  

	// execution motion tracker
	mt->DoMotonTracker();

	//recover
	for(t=0;t<MTG;t++) { 
		m_candidate[t].valid = mt->motion_candidate[t].valid;
		m_candidate[t].bx0 = mt->motion_candidate[t].bx0*rate;
		m_candidate[t].by0 = mt->motion_candidate[t].by0*rate;
		m_candidate[t].bx1 = mt->motion_candidate[t].bx1*rate;
		m_candidate[t].by1 = mt->motion_candidate[t].by1*rate;
		m_candidate[t].px = mt->motion_candidate[t].px*rate;
		m_candidate[t].py = mt->motion_candidate[t].py*rate;
	}
                
	for(j=0;j<iHeight/rate;j++) {        
		for(i=0;i<iWidth/rate;i++) {     
			for(m=0;m<rate;m++) {
				for(n=0;n<rate;n++) {     
					motion[i*rate+m][j*rate+n] = mt->motion[i][j];
				}
			}
		}
	}

	mt->ShowMotionBoxInfo(num);
//	mt->SaveMotionBoxInfo(num);

	// find candidates for a motion block
	for(t=0;t<MTG;t++) {
		if(m_candidate[t].valid!=0) {
			int bx0,by0,bx1,by1;

			bx0 = m_candidate[t].bx0;
			by0 = m_candidate[t].by0;
			bx1 = m_candidate[t].bx1;
			by1 = m_candidate[t].by1;
		
			int Wc=bx1-bx0+1;
			int Hc=by1-by0+1;

			int ***img_c;	
			img_c = (int ***)malloc(sizeof(int)*3);	
			if(img_c==NULL) exit(-1);
			for(i=0;i<3;i++) {
				img_c[i] = (int **)malloc(sizeof(int)*Wc);
				if(img_c[i]==NULL) exit(-1);

				for(j=0;j<Wc;j++) {
					img_c[i][j] = (int *)malloc(sizeof(int)*Hc);
					if(img_c[i][j]==NULL) exit(-1);
				}
			}	

			int **motion_c;
			motion_c = (int **)malloc(sizeof(int)*Wc);
			if(motion_c==NULL) exit(-1);
			for(i=0;i<Wc;i++) {
				motion_c[i] = (int *)malloc(sizeof(int)*Hc);
				if(motion_c[i]==NULL) exit(-1);
			}

			int m=0,n=0;
			for(i=bx0;i<=bx1;i++) { 			
				n = 0;
				for(j=by0;j<=by1;j++) {                	    
					img_c[0][m][n] = img_current[0][i][j]; 
					img_c[1][m][n] = img_current[1][i][j]; 
					img_c[2][m][n] = img_current[2][i][j]; 
					motion_c[m][n] = motion[i][j];
					n++; 
				}
				m++;
			}  

			// detection
			det = new CDetection(Wc,Hc);
			det->DoDetection(img_c,motion_c,Wc,Hc);

			// save face candidate
			for(int k=0;k<NTG;k++) {
				target[t][k].bx0 = det->candidate[k].bx0 + bx0;
				target[t][k].by0 = det->candidate[k].by0 + by0;
				target[t][k].bx1 = det->candidate[k].bx1 + bx0;
				target[t][k].by1 = det->candidate[k].by1 + by0;
				target[t][k].px = det->candidate[k].px + bx0;
				target[t][k].py = det->candidate[k].py + by0;
				target[t][k].valid = det->candidate[k].valid;                    
			}    

			// detele allocaed memory
			for(i=0;i<3;i++) {
				for(j=0;j<Wc;j++) {
					free(img_c[i][j]);
				}
				free(img_c[i]);
			}
			free(img_c); 

			for(i=0;i<Wc;i++) {
				free(motion_c[i]);
			}
			free(motion_c);  

			delete det; 
		} 

		// save detection result
//		SaveTargetInfo(num,t); 
	} 

	// update background
	for(t=0;t<MTG;t++) {            
		if(m_candidate[t].valid != 0) {
			boolean hasmotionorcandidate=false;
			for(int k=0;k<NTG;k++) {
				if(target[t][k].valid==1) 
				hasmotionorcandidate=true;
			} 
			if(hasmotionorcandidate==false)   mt->VerifyCandidate(t,0.05);
			else							mt->VerifyCandidate(t,0.005);	
		}            
	}

	// for panning and zooming
	for(t=0;t<MTG;t++) {            
		if(m_candidate[t].valid != 0) {
			if((m_candidate[t].bx1-m_candidate[t].bx0)*(m_candidate[t].by1-m_candidate[t].by0) > iWidth*iHeight*0.9) {
				for(j=0;j<iHeight/rate;j++) {
					for(i=0;i<iWidth/rate;i++) {					
						mt->img_back[0][i][j] = mt->img_current[0][i][j];
						mt->img_back[1][i][j] = mt->img_current[1][i][j];
						mt->img_back[2][i][j] = mt->img_current[2][i][j];
					}
				}
			}
		}
	}	

	// Show detection result
//	ShowDetectionResult(num);
//	SaveDetectionResult(num);

	// Execute LocalTracking
	lt->DoLocalTracker(m_candidate,target,motion);     
	ShowLocalTrackingResult(num); 
//	SaveLocalTrackingResult(num); 

	// show result on screen
	m_pImage->Draw(dcMem,CRect(0,0,iWidth,iHeight));
}

void CHumanTrackerDlg::ShowDetectionResult(int num) {
	int i,j,t,k;

	int **pixels = (int **)malloc(sizeof(int)*iWidth);
	if(pixels==NULL) exit(-1);
	for(i=0;i<iWidth;i++) {
		pixels[i] = (int *)malloc(sizeof(int)*iHeight);
		if(pixels[i]==NULL) exit(-1);
	}
	for(i=0;i<iWidth;i++)
		for(j=0;j<iHeight;j++)	pixels[i][j] = 0; 

	int bx0,by0,bx1,by1;
	for(t=0;t<MTG;t++) {
		if(mt->motion_candidate[t].valid!=0) {
			bx0 = mt->motion_candidate[t].bx0;
			by0 = mt->motion_candidate[t].by0;
			bx1 = mt->motion_candidate[t].bx1;
			by1 = mt->motion_candidate[t].by1;

			mt->DrawBox(pixels,1,bx0,by0,bx1,by1);
		}

		for(k=0;k<NTG;k++) {
			if(target[t][k].valid!=0) {
				bx0 = target[t][k].bx0;
				by0 = target[t][k].by0;
				bx1 = target[t][k].bx1;
				by1 = target[t][k].by1;

				if(target[t][k].valid==1) 
					mt->DrawBox(pixels,2,bx0,by0,bx1,by1);
				else if(target[t][k].valid==2) 
					mt->DrawBox(pixels,3,bx0,by0,bx1,by1);
				else if(target[t][k].valid==3) 
					mt->DrawBox(pixels,4,bx0,by0,bx1,by1);
			}
		}			
	}

	// Draw boxs
	COLORREF color;	
	
	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			if(pixels[i][j]!=0) {
				if(pixels[i][j]==1) 
					color = RGB(0,255,255);
				else if(pixels[i][j]==2) 
					color = RGB(255,0,0);
				else if(pixels[i][j]==3) 
					color = RGB(0,200,0);
				else if(pixels[i][j]==4) 
					color = RGB(155,155,155);
				m_pImage->SetPixelColor(i,j,color);
			}

		}
	}

	// release memery
	for(i=0;i<iWidth;i++) free(pixels[i]); 
	free(pixels); 
}

void CHumanTrackerDlg::SaveDetectionResult(int num) {
	// Save result
	CString ImageFile;
	ImageFile.Format("data\\detect%d.jpg",num);
	m_pImage->Save(ImageFile, CxImage::FindType(ImageFile));
}

void CHumanTrackerDlg::ShowLocalTrackingResult(int num) {
	int i,j,t;

	int **pixels = (int **)malloc(sizeof(int)*iWidth);
	if(pixels==NULL) exit(-1);
	for(i=0;i<iWidth;i++) {
		pixels[i] = (int *)malloc(sizeof(int)*iHeight);
		if(pixels[i]==NULL) exit(-1);
	}
	for(i=0;i<iWidth;i++)
		for(j=0;j<iHeight;j++)	pixels[i][j] = 0; 

	int bx0,by0,bx1,by1;

	for(t=0;t<MTG;t++) {
		HUMANID tg = lt->objectid_current[t];

		if(tg.valid==1) {
			bx0 = tg.bx0;
			by0 = tg.by0;
			bx1 = tg.bx1;
			by1 = tg.by1;

			mt->DrawBox(pixels,1,bx0,by0,bx1,by1);

			bx0 = tg.hx0;
			by0 = tg.hy0;
			bx1 = tg.hx1;
			by1 = tg.hy1;

			mt->DrawBox(pixels,2,bx0,by0,bx1,by1);
		}			
	}

	// Draw boxs
	COLORREF color;	
	
	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			if(pixels[i][j]!=0) {
				if(pixels[i][j]==1) 
					color = RGB(0,255,255);
				else if(pixels[i][j]==2) 
					color = RGB(255,0,0);
				m_pImage->SetPixelColor(i,j,color);
			}
		}
	}

	// release memery
	for(i=0;i<iWidth;i++) free(pixels[i]); 
	free(pixels); 
} 

void CHumanTrackerDlg::SaveLocalTrackingResult(int num) {
	// Save result
	CString ImageFile;
	ImageFile.Format("data\\localtrack%d.jpg",num);
	m_pImage->Save(ImageFile, CxImage::FindType(ImageFile));
}


void CHumanTrackerDlg::SaveTargetInfo(int num,int t) {
	CString fname;
	fname.Format("data\\candidate_info%d_%d.txt",num,t);
	FILE *fp = fopen(fname,"w");

	for(int k=0;k<NTG;k++) {
		fprintf(fp,"%d %d %d %d %d\n",target[t][k].valid,
			target[t][k].bx0,target[t][k].by0,target[t][k].bx1,target[t][k].by1);
	}		
	
	fclose(fp);
}


void CHumanTrackerDlg::InitDisplayMemory()
{
	CClientDC dc(this);	

	dcMem.CreateCompatibleDC(&dc);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc,Width,Height);
	CBitmap *pOldBitmap = dcMem.SelectObject(&bitmap);
}

void CHumanTrackerDlg::ShowImages()
{
	CClientDC dc(this);
	
	dc.BitBlt(0,0,Width,Height,&dcMem,0,0,SRCCOPY); 
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CHumanTrackerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CClientDC dc(this);
		dc.BitBlt(0,0,Width,Height,&dcMem,0,0,SRCCOPY); 

		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CHumanTrackerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}



void CHumanTrackerDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent==0) {
		cnt++;
		
		CString ImageFile;
		ImageFile.Format("video_%02d.jpg",cnt%2);
		LoadCameraImage(ImageFile);
		
		HumanDetection(cnt,ImageFile);	
		ShowImages(); 
	}	
	
	CDialog::OnTimer(nIDEvent);
}

void CHumanTrackerDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	KillTimer(0);
	delete m_pImage;
	FinalizeMemory();
}


void CHumanTrackerDlg::InitProcessingMemory() {
	int i,j,k;
	
	// init	
	motion = (int **)malloc(sizeof(int)*iWidth);
	if(motion==NULL) exit(-1);
	for(i=0;i<iWidth;i++) {
		motion[i] = (int *)malloc(sizeof(int)*iHeight);
		if(motion[i]==NULL) exit(-1);
	}
	for(i=0;i<iWidth;i++)
		for(j=0;j<iHeight;j++)	motion[i][j] = 0; 

	img_back = (int ***)malloc(sizeof(int)*3);	
	if(img_back==NULL) exit(-1);
	for(i=0;i<3;i++) {
		img_back[i] = (int **)malloc(sizeof(int)*iWidth);
		if(img_back[i]==NULL) exit(-1);

		for(j=0;j<iWidth;j++) {
			img_back[i][j] = (int *)malloc(sizeof(int)*iHeight);
			if(img_back[i][j]==NULL) exit(-1);
		}
	}
	for(i=0;i<3;i++) {
		for(j=0;j<iWidth;j++) {
			for(k=0;k<iHeight;k++) {
				img_back[i][j][k]=0;
			}
		}
	}

	img_current = (int ***)malloc(sizeof(int)*3);	
	if(img_current==NULL) exit(-1);
	for(i=0;i<3;i++) {
		img_current[i] = (int **)malloc(sizeof(int)*iWidth);
		if(img_current[i]==NULL) exit(-1);

		for(j=0;j<iWidth;j++) {
			img_current[i][j] = (int *)malloc(sizeof(int)*iHeight);
			if(img_current[i][j]==NULL) exit(-1);
		}
	}
	for(i=0;i<3;i++) {
		for(j=0;j<iWidth;j++) {
			for(k=0;k<iHeight;k++) {
				img_current[i][j][k]=0;
			}
		}
	}

	img_previous = (int ***)malloc(sizeof(int)*3);	
	if(img_previous==NULL) exit(-1);
	for(i=0;i<3;i++) {
		img_previous[i] = (int **)malloc(sizeof(int)*iWidth);
		if(img_previous[i]==NULL) exit(-1);

		for(j=0;j<iWidth;j++) {
			img_previous[i][j] = (int *)malloc(sizeof(int)*iHeight);
			if(img_previous[i][j]==NULL) exit(-1);
		}
	}
	for(i=0;i<3;i++) {
		for(j=0;j<iWidth;j++) {
			for(k=0;k<iHeight;k++) {
				img_previous[i][j][k]=0;
			}
		}
	}
}

void CHumanTrackerDlg::FinalizeMemory() {
	int i,j;
	for(i=0;i<3;i++) {
		for(j=0;j<iWidth;j++) {
			free(img_back[i][j]); free(img_current[i][j]); free(img_previous[i][j]); 
		}
		free(img_back[i]); free(img_current[i]); free(img_previous[i]);
	}
	free(img_back); free(img_current); free(img_previous); 

	for(i=0;i<iWidth;i++) {
		free(motion[i]); 
	}
	free(motion); 
}
