// MotionSplit.cpp: implementation of the MotionSplit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MotionSplitter.h"
#include "MotionSplit.h"
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MotionSplit::MotionSplit()
{
	
}

MotionSplit::~MotionSplit()
{
	free(bx0);
	free(by0);
	free(bx1);
	free(by1);
	free(hx0);
	free(hy0);
	free(hx1);
	free(hy1);	
	free(cx); 
}

void MotionSplit::DoMotionSplitter(int **motion,int mx0,int my0,int mx1,int my1,int n_motion,int min_head) {
	int iWidth = mx1-mx0+1;
	int iHeight = my1-my0+1;
	if(min_head>iWidth) min_head = iWidth-1;
	
	// get small motion image
	int i,j,k,m,n;
	int **motion_s = (int **)malloc(sizeof(int)*iWidth);
	if(motion_s==NULL) exit(-1);
	for(i=0;i<iWidth;i++) {
		motion_s[i] = (int *)malloc(sizeof(int)*iHeight);
		if(motion_s[i]==NULL) exit(-1);
	}
	for(i=0;i<iWidth;i++)
		for(j=0;j<iHeight;j++)	motion_s[i][j] = 0; 
        
	// copy motion data
	for(m=0;m<iWidth;m++)
		for(n=0;n<iHeight;n++) motion_s[m][n] = motion[mx0+m][my0+n];
            
	// get motion histogram
	int *motion_histo = (int *)malloc(sizeof(int)*iWidth);
	for(m=0;m<iWidth;m++) motion_histo[m]=0;
            
	for(m=0;m<iWidth;m++) {
		for(n=0;n<iHeight;n++) {
			if(motion_s[m][n]==1)   motion_histo[m]++;                
		}
	}
			
	// search head position
	int *position = GetMotions(motion_histo,n_motion,min_head,iWidth);
			
	// for exception case
	if(position[0]==0) {
		position[0] = iWidth/2;
	}
			
	for(n=0;n<n_motion;n++) {
		if(position[n]==0) {
			position[n] = position[0];
		}
	}
			
	// if there are several motions which have same center position, modify the center positions, manually.
	int cnt = 0;
	int *list_same = (int *)malloc(sizeof(int)*n_motion);
	for(n=0;n<n_motion;n++) list_same[n] = 0;
		
	for(n=1;n<n_motion;n++) {
		if(position[n]==position[0]) {
			cnt = cnt+1;
			list_same[cnt] = n;
		}
	}
	if(cnt>=1) {
		int diff = min_head/((cnt+1)*2);             
		int center = position[0];
		if(cnt==1) { 
			position[list_same[0]]=center-diff;
			position[list_same[1]]=center+diff;
		}
		else if(cnt==2) {
			position[list_same[0]]=center-diff;
			position[list_same[1]]=center;    
			position[list_same[2]]=center+diff;    
		}
		else if(cnt==3) {
			position[list_same[0]]=center-diff*2;
			position[list_same[1]]=center-diff;   
			position[list_same[2]]=center+diff;    
			position[list_same[3]]=center+diff*2;    
		}
		else if(cnt==4) {
			position[list_same[0]]=center-diff*2;    
			position[list_same[1]]=center-diff;
			position[list_same[2]]=center;    
			position[list_same[3]]=center+diff;
			position[list_same[4]]=center+diff*2;    
		}
	}
            
	// fine left and right ones which areas are expanded.
	int left_n=0,right_n=0;
	int left_pos=0,right_pos=0;
	if(n_motion>=2) {
		left_pos=position[0];
		for(k=1;k<n_motion;k++) {
			if(position[k]<left_pos) {
				left_pos = position[k];
				left_n = k;
			}
		}
               
		right_pos=position[0];
		for(k=1;k<n_motion;k++) {
			if(position[k]>right_pos) {
				right_pos = position[k];
				right_n = k;
			}
		}
	}
			
	// motion split
	bx0 = (int *)malloc(sizeof(int)*n_motion);
	by0 = (int *)malloc(sizeof(int)*n_motion);
	bx1 = (int *)malloc(sizeof(int)*n_motion);
	by1 = (int *)malloc(sizeof(int)*n_motion);
		
	cx = (int *)malloc(sizeof(int)*n_motion);
	if(n_motion==1) {
		bx0[0] = 0;
		by0[0] = 0;
		bx1[0] = iWidth-1;
		by1[0] = iHeight-1;
		cx[0] = position[0];
	}
			
	for(k=0;k<n_motion;k++) {
		if(n_motion>=2) {
			if(position[0]==position[1]) {
				bx0[k]=0;
				by0[k]=0;
				bx1[k]=iWidth-1;
				by1[k]=iHeight-1;
				cx[k]=position[0];
			}
			else {
				cx[k] = position[k];
						
				int nwidth=iWidth/n_motion;
				// bx0
				if(k==left_n)  {  
					bx0[k]=0;
				}
				else {
					bx0[k]=(int)(position[k]-nwidth/2);
					if(bx0[k]<0) { 
						bx0[k]=0; 
					}
				}
				// bx1
				if(k==right_n) {
					bx1[k] = iWidth-1;
				}
				else {
					bx1[k]=(int)(position[k]+nwidth/2);
					if(bx1[k]>iWidth) {
						bx1[k]=iWidth-1; 
					}
				}                    		
				// by0
				by0[k]=0; 
				int tag=0;
				for(int n=0;n<iHeight;n++) {
					for(int m=position[k]-min_head/2;m<=position[k]+min_head/2;m++) {
						if(m>=0 && m<iWidth) {
							if(motion_s[m][n]==1) {
								by0[k]=n;
								tag = 1;
								break;
							}
						}
					}
					if(tag==1)  break;
				}
				// by1
				by1[k]=iHeight-1; 
				tag=0;
				for(n=0;n<iHeight;n++) {
					for(m=position[k]-min_head/2;m<=position[k]+min_head/2;m++) {
						if(m>=0 && m<iWidth) {
							if(motion_s[m][iHeight-1-n]==1) {
								by1[k]=iHeight-1-n;
								tag = 1;
								break;
							}
						}
					}
					if(tag==1)  break;
				}
			}
		}
	}

	// search heads        
	hx0 = (int *)malloc(sizeof(int)*n_motion);
	for(i=0;i<n_motion;i++) hx0[i]=0;
	hy0 = (int *)malloc(sizeof(int)*n_motion);
	for(i=0;i<n_motion;i++) hy0[i]=0;
	hx1 = (int *)malloc(sizeof(int)*n_motion);
	for(i=0;i<n_motion;i++) hx1[i]=0;
	hy1 = (int *)malloc(sizeof(int)*n_motion);	
	for(i=0;i<n_motion;i++) hy1[i]=0;

	// make mask for head
	int **mask_head = (int **)malloc(sizeof(int)*min_head);
	if(mask_head==NULL) exit(-1);
	for(i=0;i<min_head;i++) {
		mask_head[i] = (int *)malloc(sizeof(int)*min_head);
		if(mask_head[i]==NULL) exit(-1);
	}
	for(i=0;i<min_head;i++)
		for(j=0;j<min_head;j++)	mask_head[i][j] = 0; 

	int mask_cx = min_head/2;
	int mask_cy = min_head/2;
	for(m=0;m<min_head;m++) {
		for(n=0;n<min_head;n++) {
			if((mask_cx-m)*(mask_cx-m)+(mask_cy-n)*(mask_cy-n)<(min_head/2)*(min_head/2))
				mask_head[m][n] = 1;
			else
				mask_head[m][n] = 0;
		}
	}

		
	FILE *fp2;
	fp2 = fopen("motion_s.txt","w");
	for(i=0;i<iWidth;i++) {
		for(j=0;j<iHeight;j++) {
			fprintf(fp2,"%d ",motion_s[i][j]);
		}
		fprintf(fp2,"\n");
	}
	fclose(fp2);
	        
	int *top_head = (int *)malloc(sizeof(int)*n_motion);
	for(i=0;i<n_motion;i++) top_head[i]=0;
	int *center_head = (int *)malloc(sizeof(int)*n_motion);
	for(i=0;i<n_motion;i++) center_head[i]=0;
	for(k=0;k<n_motion;k++) {            
		int center = cx[k];
		top_head[k] = by1[k]-min_head; // search the position of head top 
		for(int h=by1[k];h>=by0[k];h--) {
			if(motion_s[center][h]==1) {
				top_head[k]=h-min_head;
				break;
			}
		}

		int max_weight_head = 0;
		center_head[k] = min_head/2; // initialize to center            
		for(int t=bx0[k]+min_head/2;t<=bx1[k]-min_head/2;t++) {
			int weight_head = 0;
			for(m=0;m<min_head;m++) {
				for(n=0;n<min_head;n++) {
					i = m+t-min_head/2;
					j = n+top_head[k];
					if((i>=bx0[k] && i<=bx1[k]) && (j>=by0[k] && j<=by1[k]))
                            weight_head = weight_head + mask_head[m][n]*motion_s[i][j];
				}
			}            
                
			if(weight_head>max_weight_head) {
				max_weight_head = weight_head;
				center_head[k] = t;
			}
		}

		if(center_head[k]-min_head/2>0) hx0[k] = center_head[k]-min_head/2;
		else                            hx0[k] = 0;
		hy0[k] = top_head[k];
		if(center_head[k]+min_head/2<iWidth)    hx1[k] = center_head[k]+min_head/2;
		else                                    hx1[k] = iWidth-1;
		hy1[k] = top_head[k]+min_head;    
	}

	FILE *fp3;
	fp3 = fopen("top_head.txt","w");
	for(i=0;i<n_motion;i++) {
		fprintf(fp3,"%d ",top_head[i]);
	}
	fclose(fp3);


	for(i=0;i<iWidth;i++) {
		free(motion_s[i]);
	}
	free(motion_s);

	for(i=0;i<min_head;i++) {
		free(mask_head[i]);
	}
	free(mask_head);

	free(motion_histo);
	free(position);
	free(top_head);
	free(center_head);
}

int *MotionSplit::GetMotions(int *motion_histo,int n_motion,int min_head,int iWidth) {
	int N=21;
	int *x = (int *)malloc(sizeof(int)*N);

	int n;
	// cross validation
	for(n=0;n<N;n++) {			  
		x[n] = n-10;
	}
	int sigma = 10; 	   
	double *weight = GetKernel(x,N,sigma); 	   
	double *yhat = Convolution(motion_histo,weight,N,iWidth);		  
	
	// first derivative test
	double *motion_derv = (double *)malloc(sizeof(double)*iWidth);
	motion_derv[0] = 0;
	motion_derv[1] = 0;
	for(n=2;n<iWidth-2;n++) {
		motion_derv[n] = 1.0/12.0*(yhat[n-2]-8.0*yhat[n-1]+8.0*yhat[n+1]-yhat[n+2]);
	}
	motion_derv[iWidth-2] = 0;
	motion_derv[iWidth-1] = 0;
	
	double *zero_derv = (double *)malloc(sizeof(double)*iWidth);
	zero_derv[0] = 0;
	zero_derv[1] = 0;
	for(n=2;n<iWidth-2;n++) {
		if(fabs(motion_derv[n])<0.5)
			zero_derv[n] = 1;
		else
			zero_derv[n] = 0;
	}
	zero_derv[iWidth-2] = 0;
	zero_derv[iWidth-1] = 0;
	
	// find peak points
	int k;
	int tag = 0; 
	int derv_st=0, derv_end=0;
	for(n=2;n<iWidth-2;n++) {
		if(zero_derv[n]==1 && tag==0) {
			tag = 1;
			derv_st = n;
			derv_end = n;		 
		}
		else if(zero_derv[n]==1 && tag==1)
			derv_end = n;		 
		else if(zero_derv[n]==0 && tag==1) {
			tag = 0;		
			int cnt = derv_end-derv_st+1;
			
			double sum_other=0,cnt_other=0;
			for(k=derv_st-1-cnt;k<derv_st;k++) {
				if(k>=0 && k<iWidth) {
					sum_other = sum_other + yhat[k];
					cnt_other = cnt_other+1;
				}
			}
			for(k=derv_end;k<derv_end+cnt;k++) {
				if(k>=0 && k<iWidth) { 
					sum_other = sum_other + yhat[k];
					cnt_other = cnt_other+1;
				}
			}
			
			double sum_zero_derv=0, cnt_zero_derv=0;
			for(k=derv_st;k<=derv_end;k++) {
				sum_zero_derv = sum_zero_derv + yhat[k];
				cnt_zero_derv = cnt_zero_derv+1;
				zero_derv[k] = 0;
			}
			if(sum_zero_derv/cnt_zero_derv>sum_other/cnt_other) { 
				zero_derv[(int)(derv_st+(derv_end-derv_st)/2.0)] = 1;
			}
		}		
	}
	
	// find possible center of motions
	int *position = (int *)malloc(sizeof(int)*n_motion);
	for(int m=0;m<n_motion;m++) {
		double max=0;
		int pos=0;
		for(int n=2;n<iWidth-2;n++) {
			if(zero_derv[n]==1 && yhat[n]>max) {  // search max histogram
				max = yhat[n];
				pos = n;			
			}
		}
		
		if(pos>=0) {
			zero_derv[pos] = 0;
			// if one motion is detected, too close points are deleted
			for(k=-min_head/2;k<=min_head/2;k++) {
				int num = pos-k;
				if(num>=0 && num<iWidth) {
					zero_derv[num] = 0;
				}
			}
		}
		position[m] = pos;
	}
	
	free(weight);
	free(yhat);
	free(x);
	
	return position;
}

double *MotionSplit::GetKernel(int *x, int N, int sigma) {
	double *weight = (double *)malloc(sizeof(double)*N);
	double sum_v = 0;                
	int n;
	
	for(n=0;n<N;n++) {
		sum_v += exp(-(x[n]*x[n]*1.0)/(sigma*sigma*1.0));
	}
	
	for(n=0;n<N;n++) {
		weight[n] = exp(-(x[n]*x[n]*1.0)/(sigma*sigma*1.0))/sum_v;        
	}
	
	return weight;
}


double *MotionSplit::Convolution(int *motion_histo,double *weight,int N,int iWidth) {
	double *yhat = (double *)malloc(sizeof(double)*iWidth);
	
	for(int n=0;n<iWidth;n++) {
		double sum_y = 0;
		
		for(int m=0;m<N;m++) {
			int num = n-(int)(N/2)+m;  // int num = n - (int)(N/2)+m;
			if(num>=0 && num<iWidth) {
				sum_y += motion_histo[num]*weight[m];
			}
		}
		yhat[n] = sum_y;
	}
	
	return yhat;
}

