// Detection.cpp: implementation of the Detection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Detection.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define randomize() (srand(time(0)))
#define random(x) (rand() %x)

CDetection::CDetection(int Width_s,int Height_s)
{
	Width = Width_s;
	Height = Height_s;
}

CDetection::~CDetection()
{
}

void CDetection::DoDetection(int ***img_current,int **motion,int Width,int Height)
{
	int i;

	// Initialize
	Initialize();

	minMotion = 0.05;
	motionDip = 20;
	vThreshold = 0.3;
    for(i=0;i<NTG;i++) {
		candidate[i].bx0 = 0;
		candidate[i].by0 = 0;
		candidate[i].bx1 = 0;
		candidate[i].by1 = 0;
		candidate[i].px = 0;
		candidate[i].py = 0;
		candidate[i].valid = 0;            
	}
	
	// Detected image based on correlation
	FindFace(img_current,motion);

	// Weight Distribution
	WeightDistribution();

	// Theshold
	Threshold();

	// Human detection
	HumanDetection(motion); 

	// delete allocated memory
	Finalize(); 
}

void CDetection::FindFace(int ***img_current,int **motion)
{
	int i,j,k;

	// detection based on hyperspectral algorithm
	LIBRARY lib[NLIB];
	lib[0].color.rgbRed = 100;
	lib[0].color.rgbGreen = 70;
	lib[0].color.rgbBlue = 72;
	lib[0].dep = 30;
	lib[0].At = 0.90;
	lib[0].enable = true;

	lib[1].color.rgbRed = 19;
	lib[1].color.rgbGreen = 23;
	lib[1].color.rgbBlue = 22;
	lib[1].dep = 50;
	lib[1].At = 1.00;
	lib[1].enable = false;

	int t[3]={0,};
	int r[3]={0,};  // my hat	
	
	int dep=0;
	double A;

	for(i=0;i<Width;i++) {
		for(j=0;j<Height;j++) {
			if(motion[i][j]==1) {
				t[0] = img_current[0][i][j];
				t[1] = img_current[1][i][j];
				t[2] = img_current[2][i][j];

				for(k=0;k<NLIB;k++) {
					if(lib[k].enable) {
						dep = lib[k].dep;

						r[0]=lib[k].color.rgbRed;
						r[1]=lib[k].color.rgbGreen;
						r[2]=lib[k].color.rgbBlue;

						A = GetCorrelation(t,r);

						if(A>lib[k].At) {
							if(t[0]>r[0]-dep && t[0]<r[0]+dep) {
								if(t[1]>r[1]-dep && t[1]<r[1]+dep) {
									if(t[2]>r[2]-dep && t[2]<r[2]+dep) {
										detect[i][j] = 1;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void CDetection::WeightDistribution() 
{
	int i,j;

	MaxColorWeight = 0;
	for(i=0;i<Width;i++) {
		for(j=0;j<Height;j++) {
			weight[i][j] = CheckWeight(i,j);
			if(weight[i][j]>MaxColorWeight) {
				MaxColorWeight = weight[i][j];
				pxr = i;
				pyr = j;
			}
		}
	}
}

double CDetection::CheckWeight(int px,int py)
{
	int x0=0,y0=0,x1=0,y1=0;
	double weight=0;
	int i,j,m,n;

	for(i=0;i<PW;i+=4) {
		for(j=0;j<PH;j+=4) {
			m = px+i-PW/2;
			n = py+j-PH/2;
			if((m>=0 && m<Width) && (n>=0 && n<Height)) {
				if(detect[m][n]==1) weight += gauss[i][j];
			}
		}
	}
	return weight; 
}


void CDetection::Threshold() 
{
	int i,j;

	// Theshold
	for(i=0;i<Width;i++) {
		for(j=0;j<Height;j++) {
			if(weight[i][j]>MaxColorWeight*vThreshold) object[i][j] = 1;
			else								object[i][j] = 0;
		}
	} 
	
	// Get max weight
	double max;
	for(int k=0;k<NTG;k++) {
		candidate[k].valid = 0;
		max = 0;
		pxr = 0;
		pyr = 0;
		for(i=1;i<Width-1;i++) {
			for(j=1;j<Height-1;j++) {
				if(weight[i][j]>max) {
					max = weight[i][j];
					pxr = i;
					pyr = j;
				}
			}
		} 

		//Rock
		if(max>MaxColorWeight*vThreshold) {
			IsolateObject(k,pxr,pyr);		
		}
		else break;		
	}
}


void CDetection::HumanDetection(int **motion)
{
	int k;

	// Delete not valid lock windows
	int minX=10,minY=10;
	for(k=0;k<NTG;k++) {
		if(candidate[k].valid!=0) {
			// if the detected candidate is too small, make it unvalid.
			if((candidate[k].bx1-candidate[k].bx0)*(candidate[k].by1-candidate[k].by0)<minX*minY) candidate[k].valid = 0;
		}
	} 
        
	// human model
	int max_id = -1;
	int maxheight = 0;
	for(k=0;k<NTG;k++) {            
		if(candidate[k].valid!=0) {
			if(candidate[k].by0>maxheight) {
				max_id = k;
				maxheight = candidate[k].by0;                    
			}
		}
	}
	if(max_id != -1)  candidate[max_id].valid = 1;       
        
	// if the candidate is below 1/4 of height, it is classified to an other object
	for(k=0;k<NTG;k++) {            
		if(candidate[k].valid!=0) {
			if(!(candidate[k].by1>Height*3/4.))
				candidate[k].valid = 3;                
		}
	}               

    // expend the head
    for(k=0;k<NTG;k++) {            
        if(candidate[k].valid==1) {
            int cx = candidate[k].bx0+(candidate[k].bx1-candidate[k].bx0)/2;                
            int ny = candidate[k].by1;
            while(true) {
                if(ny<Height-1 && motion[cx][ny]==1) ny++;
                else break;                        
            }
            candidate[k].by1 = ny;
                
            int cy = candidate[k].by0+(candidate[k].by1-candidate[k].by0)/2;
			int nx0 = candidate[k].bx0;
			while(true) {
				if(nx0>0 && motion[nx0][cy]==1) nx0--;
				else break;                        
			}
			candidate[k].bx0 = nx0;

			int nx1 = candidate[k].bx1;
			while(true) {
				if(nx1<Width-1 && motion[nx1][cy]==1) nx1++;
				else break;                        
			}
			candidate[k].bx1 = nx1;
		}
	}  
} 

double CDetection::gnoise(void)	/* normal random variate generator */
{				        /* mean m, standard deviation s */
	double x1=0,x2=0,w=0;
	double y1=0,y2=0;
	int use_last=0;
	
	if(use_last)		        /* use value from previous call */
	{
		y1 = y2;
		use_last = 0;
	}
	else
	{
		do {
			x1 = 2.0*random(100000)/(100000.-1) - 1.0;
			x2 = 2.0*random(100000)/(100000.-1) - 1.0;
			w = x1*x1 + x2*x2;
		} while (w>=1.0);

		w = sqrt((-2.0*log(w))/w);
		y1 = x1*w;
		y2 = x2*w;
		use_last = 1;
	}

	return y1;
}

double CDetection::GetCorrelation(int t[3],int r[3])
{
	double u=0,d1=0,d2=0;
	double A=0,d=0;
	int k;

	for(k=0;k<3;k++) {
		u = u+t[k]*r[k];
		d1 = d1+t[k]*t[k];
		d2 = d2+r[k]*r[k];
	}

	d = sqrt(d1)*sqrt(d2); 
         
	if(d==0) A = 1;
	else A = 1.0-acos(u/d);

	return A;
}

void CDetection::GaussDistribution()
{
	int Nx,Ny,i,j;
	double sigma=8;
	double dep=0.5;
	double x,y;

	Nx = PW;
	Ny = PH;

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

void CDetection::IsolateObject(int num,int px,int py)
{
	int i,j;
	int bx0,by0,bx1,by1;

	int **shape;
	shape = (int **)malloc(sizeof(int)*Width);
	if(shape==NULL) exit(-1);
	for(i=0;i<Width;i++) {
		shape[i] = (int *)malloc(sizeof(int)*Height);
		if(shape[i]==NULL) exit(-1);
	}
	for(i=0;i<Width;i++)
		for(j=0;j<Height;j++) shape[i][j] = 0;

	shape[px][py]=1;
	weight[px][py]=0;

	int k,s,m,n;
	int tag;
	bool close;
	for(k=1;k<Width-1;k++) {
		tag = 0;
		for(i=-k;i<=k;i++) {
			for(j=-k;j<=k;j++) {
				s=1;
				if(px+i-s>=0 && px+i+s<=Width-1 && py+j-s>=0 && py+j+s<=Height-1) {
					if(shape[px+i][py+j]==0 && object[px+i][py+j]==1) {
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

	for(i=0;i<Width;i++) 
		for(j=0;j<Height;j++)
			if(shape[i][j]==1)	weight[i][j]=0;

	int tmp=Width-1;
	// bx0
	bx0=Width-1;
	for(j=0;j<Height;j++) {		
		for(i=0;i<Width;i++) {
			if(shape[i][j]==1){
				tmp=i;
				break;
			}
		}
		if(tmp<bx0) bx0 = tmp;
	}

	// by0		
	by0=Height-1;
	tmp=Height-1;
	for(i=0;i<Width;i++) {				
		for(j=0;j<Height;j++) {
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
	for(j=Height-1;j>=0;j--) {		
		for(i=Width-1;i>=0;i--) {
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
	for(i=Width-1;i>=0;i--) {				
		for(j=Height-1;j>=0;j--) {
			if(shape[i][j]==1){
				tmp=j;
				break;
			}
		}
		if(tmp>by1)	by1 = tmp;
	}

	for(i=0;i<Width;i++) free(shape[i]);
		free(shape);

	candidate[num].bx0 = bx0;
	candidate[num].by0 = by0;
	candidate[num].bx1 = bx1;
	candidate[num].by1 = by1;
	candidate[num].px = px;
	candidate[num].py = py;
	candidate[num].valid = 2;
}

void CDetection::Initialize()
{
	int i,j;

	// Initialize gauss distribution
	GaussDistribution();

	detect = (int **)malloc(sizeof(int)*Width);
	if(detect==NULL) exit(-1);
	for(i=0;i<Width;i++) {
		detect[i] = (int *)malloc(sizeof(int)*Height);
		if(detect[i]==NULL) exit(-1);
	}
	for(i=0;i<Width;i++)
		for(j=0;j<Height;j++) detect[i][j] = 0;

	weight = (double **)malloc(sizeof(double)*Width);
	if(weight==NULL) exit(-1);
	for(i=0;i<Width;i++) {
		weight[i] = (double *)malloc(sizeof(double)*Height);
		if(weight[i]==NULL) exit(-1);
	}
	for(i=0;i<Width;i++)
		for(j=0;j<Height;j++) weight[i][j] = 0;

	// object
	object = (int **)malloc(sizeof(int)*Width);
	if(object==NULL) exit(-1);
	for(i=0;i<Width;i++) {
		object[i] = (int *)malloc(sizeof(int)*Height);
		if(object[i]==NULL) exit(-1);
	}
	for(i=0;i<Width;i++)
		for(j=0;j<Height;j++) object[i][j] = 0;
}

void CDetection::Finalize()
{
	int i;

	for(i=0;i<Width;i++) {
		free(detect[i]);
		free(weight[i]);
		free(object[i]);
	}
	free(detect); free(weight); free(object);

	// gauss distribution
	for(i=0;i<PH;i++) free(gauss[i]);
	free(gauss); 
}
