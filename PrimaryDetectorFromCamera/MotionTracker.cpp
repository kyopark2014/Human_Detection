// MotionTracker.cpp: implementation of the CMotionTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HumanTracker.h"
#include "MotionTracker.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

double **gauss=NULL;
double **motion_weight=NULL;
int **idmap=NULL;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMotionTracker::CMotionTracker()
{
}

CMotionTracker::~CMotionTracker()
{
	FinalizeMemory();
}

void CMotionTracker::Initialize() {
	iWidth = ImageWidth/RATE;
	iHeight = ImageHeight/RATE;

	InitProcessingMemory();
}

void CMotionTracker::DoMotonTracker() {
	for(int k=0;k<MTG;k++) {
		motion_candidate[k].bx0 = 0;
		motion_candidate[k].by0 = 0;
		motion_candidate[k].bx1 = 0;
		motion_candidate[k].by1 = 0;
		motion_candidate[k].px = 0;
		motion_candidate[k].py = 0;
		motion_candidate[k].valid = 0;
	}		

	// Initialize gauss distribution
	GaussDistribution();

	// get motion information
	GetMotionInfo();

	// get motion object
	GetMotionObject();

	// get object information
	GetObjectInfo();

	// verify motion_candidate
 //   VerifyCandidate();
}

void CMotionTracker::InitProcessingMemory() {
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

	motion_object = (int **)malloc(sizeof(int)*iWidth);
	if(motion_object==NULL) exit(-1);
	for(i=0;i<iWidth;i++) {
		motion_object[i] = (int *)malloc(sizeof(int)*iHeight);
		if(motion_object[i]==NULL) exit(-1);
	}
	for(i=0;i<iWidth;i++)
		for(j=0;j<iHeight;j++)	motion_object[i][j] = 0; 

	motion_weight = (double **)malloc(sizeof(double)*iWidth);
	if(motion_weight==NULL) exit(-1);
	for(i=0;i<iWidth;i++) {
		motion_weight[i] = (double *)malloc(sizeof(double)*iHeight);
		if(motion_weight[i]==NULL) exit(-1);
	}
	for(i=0;i<iWidth;i++)
		for(j=0;j<iHeight;j++)	motion_weight[i][j] = 0; 

	idmap = (int **)malloc(sizeof(int)*iWidth);
	if(idmap==NULL) exit(-1);
	for(i=0;i<iWidth;i++) {
		idmap[i] = (int *)malloc(sizeof(int)*iHeight);
		if(idmap[i]==NULL) exit(-1);
	}
	for(i=0;i<iWidth;i++)
		for(j=0;j<iHeight;j++)	idmap[i][j] = 0; 


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

void CMotionTracker::FinalizeMemory() {
	int i,j;
	for(i=0;i<3;i++) {
		for(j=0;j<iWidth;j++) {
			free(img_back[i][j]); free(img_current[i][j]); free(img_previous[i][j]); 
		}
		free(img_back[i]); free(img_current[i]); free(img_previous[i]);
	}
	free(img_back); free(img_current); free(img_previous); 

	for(i=0;i<PH_M;i++) free(gauss[i]);
	free(gauss); 

	for(i=0;i<iWidth;i++) {
		free(motion[i]); 
		free(motion_object[i]); 		
		free(motion_weight[i]); 		
		free(idmap[i]); 
	}
	free(motion); 
	free(motion_object); 
	free(motion_weight); 
	free(idmap); 
}



void CMotionTracker::GaussDistribution()
{
	int Nx,Ny,i,j;
	double sigma=8;
	double dep=0.5;
	double x,y;

	Nx = PW_M;
	Ny = PH_M;

	gauss = (double **)malloc(sizeof(double)*Ny);
	if(gauss==NULL) exit(-1);
	for(j=0;j<Ny;j++) {
		gauss[j] = (double *)malloc(sizeof(double)*Nx);
		if(gauss[j]==NULL) exit(-1);
	}
	for(i=0;i<Ny;i++)
		for(j=0;j<Nx;j++)	gauss[i][j] = 0; 

	for(i=0;i<Ny;i++) {
		for(j=0;j<Nx;j++) {	
			x = -(Nx*dep)/2.+i*dep;
			y = -(Ny*dep)/2.+j*dep;
			gauss[i][j] = exp(-(pow((x/sigma),2)+pow((y/sigma),2)));
		}
	}
}

void CMotionTracker::GetMotionInfo() {
	int diff1,diff2,diff3;
	int i,j;
        
	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			diff1 = img_current[0][i][j] - img_back[0][i][j];
			diff2 = img_current[1][i][j] - img_back[1][i][j];
			diff3 = img_current[2][i][j] - img_back[2][i][j];

			if(diff1<-motionDep || diff2<-motionDep || diff3<-motionDep) 
				motion[i][j] = 1;
			else if(diff1>motionDep || diff2>motionDep || diff3>motionDep) 
				motion[i][j] = 1;
			else
				motion[i][j] = 0;			
		}
	}        
}

void CMotionTracker::GetMotionObject() {
	int i,j;

	maxMotionWeight = 0;

	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			motion_weight[i][j] = CheckMotionWeight(i,j);

			if(motion_weight[i][j]>maxMotionWeight) maxMotionWeight = motion_weight[i][j];
		}
	}
	
	// get motion object for debugging
	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			if(motion_weight[i][j]>maxMotionWeight*minMotionThresholdRate && motion_weight[i][j]>minMotionWeight)
				motion_object[i][j]=1;
			else
				motion_object[i][j]=0;
		}
	}        
}

double CMotionTracker::CheckMotionWeight(int px,int py) {
	int x0=0,y0=0,x1=0,y1=0;
	double weight=0;
	int i,j,m,n;

	for(i=0;i<PW_M;i++) {
		for(j=0;j<PH_M;j++) {
			m = px+i-PW_M/2;
			n = py+j-PH_M/2;
			if((m>=0 && m<iWidth) && (n>=0 && n<iHeight)) {
				if(motion[m][n]==1) weight += gauss[i][j];
			}
		}
	}

	return weight; 
}

void CMotionTracker::GetObjectInfo() {
	int num_obj=0;
       
	// initialize idmap
	for(int i=0;i<iWidth;i++)
		for(int j=0;j<iHeight;j++) idmap[i][j] = 0;

	// Get max weight
   	double max;
	int pxr,pyr;
	for(int k=0;k<MTG;k++) {
		motion_candidate[k].valid = 0;
		max = 0;
		pxr = 0;
		pyr = 0;
		for(int i=1;i<iWidth-1;i++) {
			for(int j=1;j<iHeight-1;j++) {
				if(motion_weight[i][j]>max) {
					max = motion_weight[i][j];
					pxr = i;
					pyr = j;
				}
			}
		} 

		//Rock            
		if(max>maxMotionWeight*minMotionThresholdRate && max>minMotionWeight) {
			if(IsolateObject(num_obj,pxr,pyr))    num_obj++;
		} 
		else break;
	}
}

void CMotionTracker::VerifyCandidate(int k,double minMotionRate) {
	int diff1,diff2,diff3;
	double motion_rate;
	double cnt;

	if(motion_candidate[k].valid!=0) {                
		cnt = 0;
		for(int i=motion_candidate[k].bx0;i<=motion_candidate[k].bx1;i++) {
			for(int j=motion_candidate[k].by0;j<=motion_candidate[k].by1;j++) {
				diff1 = img_current[0][i][j] - img_previous[0][i][j];
				diff2 = img_current[1][i][j] - img_previous[1][i][j];
				diff3 = img_current[2][i][j] - img_previous[2][i][j];

				if(diff1<-motionDep || diff2<-motionDep || diff3<-motionDep) cnt++;
				else if(diff1>motionDep || diff2>motionDep || diff3>motionDep) cnt++;
			}     
		}
		motion_rate = cnt/((motion_candidate[k].bx1-motion_candidate[k].bx0)*(motion_candidate[k].by1-motion_candidate[k].by0));

		if(motion_rate<minMotionRate) {
			motion_candidate[k].valid = 0;

			for(int i=motion_candidate[k].bx0;i<=motion_candidate[k].bx1;i++) {
				for(int j=motion_candidate[k].by0;j<=motion_candidate[k].by1;j++) {
					img_back[0][i][j] = img_current[0][i][j];
					img_back[1][i][j] = img_current[1][i][j];
					img_back[2][i][j] = img_current[2][i][j];                            
				}
			}
		}
	}
}

bool CMotionTracker::IsolateObject(int num_obj,int px,int py)
{
	int bx0=0,by0=0,bx1=0,by1=0;
	int i,j;

	// init	
	int **shape = (int **)malloc(sizeof(int)*iWidth);
	if(shape==NULL) exit(-1);
	for(i=0;i<iWidth;i++) {
		shape[i] = (int *)malloc(sizeof(int)*iHeight);
		if(shape[i]==NULL) exit(-1);
	}
	for(i=0;i<iWidth;i++)
		for(j=0;j<iHeight;j++)	shape[i][j] = 0; 

	shape[px][py]=1;
	motion_weight[px][py]=0;

	int tag;
	bool close;
	int m,n,s;

	for(int k=1;k<iWidth-1;k++) {
		tag = 0;
		for(i=-k;i<=k;i++) {
			for(j=-k;j<=k;j++) {
				s=1;
				if(px+i-s>=0 && px+i+s<=iWidth-1 && py+j-s>=0 && py+j+s<=iHeight-1) {
					if(shape[px+i][py+j]==0 && motion_object[px+i][py+j]==1) {
						close=false;
                            
						for(m=-s;m<=s;m++) {
							for(n=-s;n<=s;n++) {
								if(shape[px+i+m][py+j+n]==1) {
									close=true;
									break;
								}
							}
						}
                           
						if(close) {
							shape[px+i][py+j]=1;
							tag = 1;
						}					
					}
				}
			}
		}
		if(tag==0) break;
	} 

	for(i=0;i<iWidth;i++)
		for(j=0;j<iHeight;j++)
			if(shape[i][j]==1)	motion_weight[i][j]=0; 
	
	// bx0
	bx0=iWidth-1;
	int tmp=iWidth-1;
	for(j=0;j<iHeight;j++) {		
		for(i=0;i<iWidth;i++) {
			if(shape[i][j]==1){
				tmp=i;
				break;
			}
		}
		if(tmp<bx0) bx0 = tmp;
	}

	// by0		
	by0=iHeight-1;
	tmp=iHeight-1;
	for(i=0;i<iWidth;i++) {				
		for(j=0;j<iHeight;j++) {
			if(shape[i][j]==1){
				tmp=j;
				break;
			}
		}
		if(tmp<by0)	by0 = tmp;
	}

	// bx1
	bx1=0;
	tmp=0;
	for(j=iHeight-1;j>=0;j--) {		
		for(i=iWidth-1;i>=0;i--) {
			if(shape[i][j]==1){
				tmp=i;
				break;
			}
		}
		if(tmp>bx1)	bx1 = tmp;
	}

	// by1
	by1=0;
	tmp=0;
	for(i=iWidth-1;i>=0;i--) {				
		for(j=iHeight-1;j>=0;j--) {
			if(shape[i][j]==1){
				tmp=j;
				break;
			}
		}
		if(tmp>by1)	by1 = tmp;
	}
     
	bool isobject;
	if(bx1-bx0>minMotionX && by1-by0>minMotionY) {
		motion_candidate[num_obj].bx0 = bx0;
		motion_candidate[num_obj].by0 = by0;
		motion_candidate[num_obj].bx1 = bx1;
		motion_candidate[num_obj].by1 = by1;
		motion_candidate[num_obj].px = px;
		motion_candidate[num_obj].py = py;
		motion_candidate[num_obj].valid = 1;
            
		// make idmap
		for(i=motion_candidate[num_obj].bx0;i<motion_candidate[num_obj].bx1;i++) {
			for(j=motion_candidate[num_obj].by0;j<motion_candidate[num_obj].by1;j++) {
				if(shape[i][j]==1)  idmap[i][j] = num_obj+1;        
				else                idmap[i][j] = 0;       
			}
		}
            
		isobject = true;
	}
	else isobject = false;

	// release memery
	for(i=0;i<iWidth;i++) free(shape[i]); 
	free(shape); 

	return isobject;
} 

void CMotionTracker::SaveToImageFile(int ***img_color,CString ImageFile)
{
	CxImage *m_pImageFile = m_pImageFile = new CxImage;	
	m_pImageFile->Create(iWidth,iHeight,24,0);

	// Set Color
	COLORREF Color;	
	
	int i,j;
	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			Color = RGB(img_color[0][i][j],img_color[1][i][j],img_color[2][i][j]);
			m_pImageFile->SetPixelColor(i,j,Color);
		}
	}

	// Save result
	m_pImageFile->Save(ImageFile, CxImage::FindType(ImageFile));	

	delete m_pImageFile;
}

void CMotionTracker::ShowMotionBoxInfo(int num) {
	// color
	int i,j;

	int **pixels = (int **)malloc(sizeof(int)*iWidth);
	if(pixels==NULL) exit(-1);
	for(i=0;i<iWidth;i++) {
		pixels[i] = (int *)malloc(sizeof(int)*iHeight);
		if(pixels[i]==NULL) exit(-1);
	}
	for(i=0;i<iWidth;i++)
		for(j=0;j<iHeight;j++)	pixels[i][j] = idmap[i][j]; 

	int px0,py0,px1,py1;
	for(int k=0;k<MTG;k++) {
		if(motion_candidate[k].valid!=0) {
			if(!(motion_candidate[k].px==0 && motion_candidate[k].py==0))  {    
				px0 = motion_candidate[k].bx0;
				py0 = motion_candidate[k].by0;
				px1 = motion_candidate[k].bx1;
				py1 = motion_candidate[k].by1;
                    
				int lcolor = 255;
                DrawBox(pixels,lcolor,px0,py0,px1,py1);
			}
		}
	}  

	// initialize m_pImageFile
	CxImage *m_pImageFile = m_pImageFile = new CxImage;	
	m_pImageFile->Create(iWidth,iHeight,24,0);

	// Set Color
	COLORREF color;	
	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			if(pixels[i][j] == 255)		color = RGB(150,150,150);
			else if(pixels[i][j] == 0)  color = RGB(0,0,0);
			else if(pixels[i][j] == 1)	color = RGB(255,0,0);
			else if(pixels[i][j] == 2)	color = RGB(0,255,0);
			else if(pixels[i][j] == 3)	color = RGB(0,0,255);
			else						color = RGB(20*pixels[i][j],0,0);
			
			m_pImageFile->SetPixelColor(i,j,color);
		}
	}

	// Save idmap
//	CString ImageFile;
//	ImageFile.Format("data\\idmap%d.jpg",num);
//	m_pImageFile->Save(ImageFile, CxImage::FindType(ImageFile));		

	// release memery
	for(i=0;i<iWidth;i++) free(pixels[i]); 
	free(pixels); 

	delete m_pImageFile;
}    

void CMotionTracker::SaveMotionBoxInfo(int num) {
	int i;

	CString fname;
	fname.Format("data\\motion_info%d.txt",num);

	FILE *fp = fopen(fname,"w");
	for(i=0;i<MTG;i++) {
		fprintf(fp,"%d %d %d %d %d\n",motion_candidate[i].valid,
			motion_candidate[i].bx0,motion_candidate[i].by0,
			motion_candidate[i].bx1,motion_candidate[i].by1);
	}			
	fclose(fp);

}

void CMotionTracker::DrawBox(int **pixels,int lcolor,int px0,int py0,int px1,int py1) {
	int i,j;
	int lwidth=3;
	int l;
                    
	j = py0;
	for(i=px0;i<=px1;i++) {
		for(l=0;l<lwidth;l++) {
			pixels[i][j+l] = lcolor;            
		}
	}

	j = py1;
	for(i=px0;i<=px1;i++) {
		for(l=0;l<lwidth;l++) {
			pixels[i][j-l] = lcolor; 
		}
	}

	i = px0;
	for(j=py0;j<=py1;j++) {
		for(l=0;l<lwidth;l++) {
			pixels[i+l][j] = lcolor;
		}
	}

	i = px1;
	for(j=py0;j<=py1;j++) {
		for(l=0;l<lwidth;l++) {
			pixels[i-l][j] = lcolor;            
		}
	}                            
}

void CMotionTracker::LoadCurrentColorImageFromFile(CString ImageFile) {
	CxImage *m_pImageFile = new CxImage;	

	m_pImageFile->Load(ImageFile, CxImage::FindType(ImageFile));	

	// copy current color to previous
	int i,j;
/*	for(int k=0;k<3;k++) {
		for(i=0;i<iWidth;i++) {
			for(j=0;j<iHeight;j++) {			
				img_previous[k][i][j] = img_current[k][i][j];
			}
		}
	}  */
	
	// image buffer
	RGBQUAD color;
	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			color = m_pImageFile->GetPixelColor(i,j);
			
			img_current[0][i][j] = color.rgbRed;
			img_current[1][i][j] = color.rgbGreen;
			img_current[2][i][j] = color.rgbBlue;
		}
	}

	delete m_pImageFile;
}

