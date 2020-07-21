// MotionSplitterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MotionSplitter.h"
#include "MotionSplitterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int ***img_current;
int **motion;

/////////////////////////////////////////////////////////////////////////////
// CMotionSplitterDlg dialog

CMotionSplitterDlg::CMotionSplitterDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMotionSplitterDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMotionSplitterDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMotionSplitterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMotionSplitterDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMotionSplitterDlg, CDialog)
	//{{AFX_MSG_MAP(CMotionSplitterDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMotionSplitterDlg message handlers

BOOL CMotionSplitterDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_pImage = new CxImage;	

	CString ImageFile;

	int n_overlappedmotion;
	//ImageFile.Format("m45.jpg"); n_overlappedmotion=1;
	//ImageFile.Format("image_sample\\m46_0.jpg"); n_overlappedmotion=1;
	//ImageFile.Format("image_sample\\m4_0.jpg"); n_overlappedmotion=0;
	//ImageFile.Format("image_sample\\m6_0.jpg"); n_overlappedmotion=0;
	//ImageFile.Format("image_sample\\m24_0.jpg"); n_overlappedmotion=0;
	//ImageFile.Format("image_sample\\m45_0.jpg"); n_overlappedmotion=1;
	//ImageFile.Format("image_sample\\m48_0.jpg"); n_overlappedmotion=0;
	ImageFile.Format("image_sample\\m53_1.jpg"); n_overlappedmotion=0;

	// load current image
	m_pImage->Load(ImageFile, CxImage::FindType(ImageFile));

	iWidth = m_pImage->GetWidth();
	iHeight = m_pImage->GetHeight();

	// Set size of windows
	SetWindowPos(NULL,0,0,iWidth,iHeight+30,SWP_NOMOVE|SWP_SHOWWINDOW); // Adjust testbed size

	// image
	int i,j,k;
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
	
	// motion
	motion = (int **)malloc(sizeof(int)*iWidth);
	if(motion==NULL) exit(-1);
	for(i=0;i<iWidth;i++) {
		motion[i] = (int *)malloc(sizeof(int)*iHeight);
		if(motion[i]==NULL) exit(-1);
	}
	for(i=0;i<iWidth;i++)
		for(j=0;j<iHeight;j++)	motion[i][j] = 0; 

	// get the color
	RGBQUAD color;
	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			color = m_pImage->GetPixelColor(i,j);
			
			img_current[0][i][j] = color.rgbRed;
			img_current[1][i][j] = color.rgbGreen;
			img_current[2][i][j] = color.rgbBlue;
		}
	}

	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			if(img_current[0][i][j]<5 && img_current[1][i][j]<5 && img_current[2][i][j]<5) {
				motion[i][j]=0;
			}
			else {
				motion[i][j]=1;
			}
		}
	}

	// set the color to intensity
	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			if(motion[i][j]==1) {
				color.rgbRed = img_current[0][i][j];
				color.rgbGreen = img_current[1][i][j];
				color.rgbBlue = img_current[2][i][j];
			}
			else {
				color.rgbRed = 0;
				color.rgbGreen = 0;
				color.rgbBlue = 0;
			}

			m_pImage->SetPixelColor(i,j,color);
		}
	} 

	// initialize display memory
	InitDisplayMemory();

	MotionSplit *ms = new MotionSplit();

	int mx0=0;
	int my0=0;
	int mx1=iWidth-1;
	int my1=iHeight-1;
	int min_head=60;
	ms->DoMotionSplitter(motion,mx0,my0,mx1,my1,n_overlappedmotion+1,min_head);

	for(k=0;k<n_overlappedmotion+1;k++) {
		int bx0 = mx0+ms->bx0[k];
		int by0 = my0+ms->by0[k];
		int bx1 = mx0+ms->bx1[k];
		int by1 = my0+ms->by1[k];

		int hx0 = mx0+ms->hx0[k];
		int hy0 = my0+ms->hy0[k];
		int hx1 = mx0+ms->hx1[k];
		int hy1 = my0+ms->hy1[k];

		i=bx0;
		for(j=by0;j<=by1;j++) {
			color.rgbRed = 255;
			color.rgbGreen = 0;
			color.rgbBlue = 255;

			m_pImage->SetPixelColor(i,j,color);
		}

		i=bx1;
  		for(j=by0;j<=by1;j++) {
			color.rgbRed = 255;
			color.rgbGreen = 0;
			color.rgbBlue = 255;

			m_pImage->SetPixelColor(i,j,color);
		}

		j=by0;
		for(i=bx0;i<=bx1;i++) {
			color.rgbRed = 255;
			color.rgbGreen = 0;
			color.rgbBlue = 255;

			m_pImage->SetPixelColor(i,j,color);
		}

		j=by1;
		for(i=bx0;i<=bx1;i++) {
			color.rgbRed = 255;
			color.rgbGreen = 0;
			color.rgbBlue = 255;

			m_pImage->SetPixelColor(i,j,color);
		}


		i=hx0;
		for(j=hy0;j<=hy1;j++) {
			color.rgbRed = 255;
			color.rgbGreen = 0;
			color.rgbBlue = 0;

			m_pImage->SetPixelColor(i,j,color);
		}

		i=hx1;
  		for(j=hy0;j<=hy1;j++) {
			color.rgbRed = 255;
			color.rgbGreen = 0;
			color.rgbBlue = 0;

			m_pImage->SetPixelColor(i,j,color);
		}

		j=hy0;
		for(i=hx0;i<=hx1;i++) {
			color.rgbRed = 255;
			color.rgbGreen = 0;
			color.rgbBlue = 0;

			m_pImage->SetPixelColor(i,j,color);
		}

		j=hy1;
		for(i=hx0;i<=hx1;i++) {
			color.rgbRed = 255;
			color.rgbGreen = 0;
			color.rgbBlue = 0;

			m_pImage->SetPixelColor(i,j,color);
		}

	} 

	FILE *fp;
	fp = fopen("result.txt","w");
	for(k=0;k<n_overlappedmotion+1;k++) {
		fprintf(fp,"cx=%d M[%d %d %d %d] H[%d %d %d %d]\n",ms->cx[k],ms->bx0[k],ms->by0[k],ms->bx1[k],ms->by1[k],ms->hx0[k],ms->hy0[k],ms->hx1[k],ms->hy1[k]);
	}
	fclose(fp); 
	
	// Draw
	m_pImage->Draw(dcMem,CRect(0,0,iWidth,iHeight));

	// delete image memory
	for(i=0;i<3;i++) {
		for(j=0;j<iWidth;j++) {
			free(img_current[i][j]);
		}
		free(img_current[i]);
	}
	free(img_current);

	for(i=0;i<iWidth;i++) {
		free(motion[i]); 
	}
	free(motion);  

	delete ms;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMotionSplitterDlg::InitDisplayMemory()
{
	CClientDC dc(this);	

	dcMem.CreateCompatibleDC(&dc);
	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc,iWidth,iHeight);
	CBitmap *pOldBitmap = dcMem.SelectObject(&bitmap);
}

void CMotionSplitterDlg::ShowImages()
{
	CClientDC dc(this);
	
	dc.BitBlt(0,0,iWidth,iHeight,&dcMem,0,0,SRCCOPY); 
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMotionSplitterDlg::OnPaint() 
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
		dc.BitBlt(0,0,iWidth,iHeight,&dcMem,0,0,SRCCOPY); 

		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMotionSplitterDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}
