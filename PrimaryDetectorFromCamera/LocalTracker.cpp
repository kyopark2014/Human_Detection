// LocalTracker.cpp: implementation of the CLocalTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HumanTracker.h"
#include "LocalTracker.h"
#include "math.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CLocalTracker::CLocalTracker()
{
	nid = 100;

	for(int t=0;t<MTG;t++) {
		objectid_estimate[t].id = t+1;
	}

	iWidth = ImageWidth/RATE;
	iHeight = ImageHeight/RATE;	

	locked_index = (int *)malloc(sizeof(int)*MTG);
	if(locked_index==NULL) exit(-1);

	int i,j;
	lib_id = (int *)malloc(sizeof(int)*MTG);
	for(i=0;i<MTG;i++) lib_id[i] = 0;
	lib_valid = (int *)malloc(sizeof(int)*MTG);
	for(i=0;i<MTG;i++) lib_valid[i] = 0;
	

	lib_histogram = (long **)malloc(sizeof(long)*MTG);
	if(lib_histogram==NULL) exit(-1);
	for(i=0;i<MTG;i++) {
		lib_histogram[i] = (long *)malloc(sizeof(long)*256);
		if(lib_histogram[i]==NULL) exit(-1);
	}
	for(i=0;i<MTG;i++)
		for(j=0;j<256;j++)	lib_histogram[i][j] = 0; 

	motion_histogram = (long **)malloc(sizeof(long)*MTG);
	if(motion_histogram==NULL) exit(-1);
	for(i=0;i<MTG;i++) {
		motion_histogram[i] = (long *)malloc(sizeof(long)*256);
		if(motion_histogram[i]==NULL) exit(-1);
	}
	for(i=0;i<MTG;i++)
		for(j=0;j<256;j++)	motion_histogram[i][j] = 0; 

	// histogram
	objectid_current_histogram = (long **)malloc(sizeof(long)*MTG);
	if(objectid_current_histogram==NULL) exit(-1);
	for(i=0;i<MTG;i++) {
		objectid_current_histogram[i] = (long *)malloc(sizeof(long)*256);
		if(objectid_current_histogram[i]==NULL) exit(-1);
	}
	for(i=0;i<MTG;i++)
		for(j=0;j<256;j++)	objectid_current_histogram[i][j] = 0; 

	objectid_estimate_histogram = (long **)malloc(sizeof(long)*MTG);
	if(objectid_estimate_histogram==NULL) exit(-1);
	for(i=0;i<MTG;i++) {
		objectid_estimate_histogram[i] = (long *)malloc(sizeof(long)*256);
		if(objectid_estimate_histogram[i]==NULL) exit(-1);
	}
	for(i=0;i<MTG;i++)
		for(j=0;j<256;j++)	objectid_estimate_histogram[i][j] = 0; 

	objectid_previous_histogram = (long **)malloc(sizeof(long)*MTG);
	if(objectid_previous_histogram==NULL) exit(-1);
	for(i=0;i<MTG;i++) {
		objectid_previous_histogram[i] = (long *)malloc(sizeof(long)*256);
		if(objectid_previous_histogram[i]==NULL) exit(-1);
	}
	for(i=0;i<MTG;i++)
		for(j=0;j<256;j++)	objectid_previous_histogram[i][j] = 0; 

	ms = new MotionSplit();

	for(i=0;i<MTG;i++) {
		objectid_current[i].lock=false;
		objectid_current[i].valid=0;
		objectid_current[i].id=-1;
		objectid_current[i].bx0=0;
		objectid_current[i].by0=0;
		objectid_current[i].bx1=0;
		objectid_current[i].by1=0;
		objectid_current[i].px=0;
		objectid_current[i].py=0;		
		objectid_current[i].hx0=0;
		objectid_current[i].hy0=0;
		objectid_current[i].hx1=0;
		objectid_current[i].hy1=0;
		objectid_current[i].hx=0;
		objectid_current[i].hy=0;
		objectid_current[i].overlappedone=false;
	}
}

CLocalTracker::~CLocalTracker()
{
	int i;

	free(locked_index);

	for(i=0;i<MTG;i++) {
		free(motion_histogram[i]); free(lib_histogram[i]);
	}
	free(motion_histogram); free(lib_histogram); 

	free(lib_id);
	free(lib_valid);

	for(i=0;i<MTG;i++) {
		free(objectid_current_histogram[i]);
		free(objectid_estimate_histogram[i]);
		free(objectid_previous_histogram[i]);
	}		
	free(objectid_current_histogram);
	free(objectid_estimate_histogram);
	free(objectid_previous_histogram);
}

void CLocalTracker::DoLocalTracker(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int **motion,int ***img_current) {
	// residue - minimum distance between estimate object and current object
	residue = MIN_DIST*MIN_DIST+MIN_DIST*MIN_DIST;        

	// update current info from detected motion info.
	UpdateCurrentInfo(motion_candidate,motion,img_current);

	// link estimate and current state.
	LinkEstimateObjectwithCurrentObject(motion_candidate,target,motion,img_current);

	// if there are overlapped motions, split them.
	SplitOverlappedMotion(motion_candidate,target,motion,img_current);

	// if there are new objects, insert the info.
	InsertNewObject(motion_candidate,target,motion,img_current);

	// delete old library.
	UpdateLibraryInfo(); 

	// copy current info to previous info.
	UpdatePreviousInfo();  

	// Jinseok - estimate next state from previous info.
	UpdateEstimateInfo();
}

void CLocalTracker::UpdateEstimateInfo() {
	// update objectid_estimate
	// Initially, the estimate state is the same as previous state (No estimate)
	for(int t=0;t<MTG;t++) { 
		objectid_estimate[t].id= objectid_previous[t].id;
		objectid_estimate[t].lock= objectid_previous[t].lock;
            
		objectid_estimate[t].valid = objectid_previous[t].valid;
		objectid_estimate[t].bx0 = objectid_previous[t].bx0;
		objectid_estimate[t].by0 = objectid_previous[t].by0;
		objectid_estimate[t].bx1 = objectid_previous[t].bx1;
		objectid_estimate[t].by1 = objectid_previous[t].by1;
		objectid_estimate[t].px = objectid_previous[t].px;
		objectid_estimate[t].py = objectid_previous[t].py;            
		objectid_estimate[t].hx0 = objectid_previous[t].hx0;
		objectid_estimate[t].hy0 = objectid_previous[t].hy0;
		objectid_estimate[t].hx1 = objectid_previous[t].hx1;
		objectid_estimate[t].hy1 = objectid_previous[t].hy1;
		objectid_estimate[t].hx = objectid_previous[t].hx;
		objectid_estimate[t].hy = objectid_previous[t].hy;                   
            
		for(int q=0;q<256;q++) 
			objectid_estimate_histogram[t][q] = objectid_previous_histogram[t][q];
    }
}

void CLocalTracker::UpdatePreviousInfo() {
	for(int t=0;t<MTG;t++) {
		objectid_previous[t].id= objectid_current[t].id;
		objectid_previous[t].lock= objectid_current[t].lock;
		
		objectid_previous[t].valid= objectid_current[t].valid;
		objectid_previous[t].bx0= objectid_current[t].bx0;
		objectid_previous[t].by0= objectid_current[t].by0;
		objectid_previous[t].bx1= objectid_current[t].bx1;
		objectid_previous[t].by1= objectid_current[t].by1;
		objectid_previous[t].px= objectid_current[t].px;
		objectid_previous[t].py= objectid_current[t].py;
		objectid_previous[t].hx0= objectid_current[t].hx0;
		objectid_previous[t].hy0= objectid_current[t].hy0;
		objectid_previous[t].hx1= objectid_current[t].hx1;
		objectid_previous[t].hy1= objectid_current[t].hy1;
		objectid_previous[t].hx= objectid_current[t].hx;
		objectid_previous[t].hy= objectid_current[t].hy; 

		for(int q=0;q<256;q++) 
			objectid_previous_histogram[t][q] = objectid_current_histogram[t][q];
	}
}

void CLocalTracker::UpdateCurrentInfo(MOTIONCANDIDATE motion_candidate[MTG],int **motion,int ***img_current) {
	for(int t=0;t<MTG;t++) {
		// update current one
		if(motion_candidate[t].valid==1) {
			objectid_current[t].id = -1;
			objectid_current[t].lock = false;
			
			objectid_current[t].valid = motion_candidate[t].valid;
			objectid_current[t].bx0 = motion_candidate[t].bx0;
			objectid_current[t].by0 = motion_candidate[t].by0;
			objectid_current[t].bx1 = motion_candidate[t].bx1;
			objectid_current[t].by1 = motion_candidate[t].by1;
			objectid_current[t].px = motion_candidate[t].px;
			objectid_current[t].py = motion_candidate[t].py;                            
			objectid_current[t].hx0 = 0;
			objectid_current[t].hy0 = 0;
			objectid_current[t].hx1 = 0;
			objectid_current[t].hy1 = 0;
			objectid_current[t].hx = 0;
			objectid_current[t].hy = 0;

			// get histogram
			int bx0 = motion_candidate[t].bx0;
			int by0 = motion_candidate[t].by0;
			int bx1 = motion_candidate[t].bx1;
			int by1 = motion_candidate[t].by1; 

			objectid_current_histogram[t] = GetHistogram(bx0,by0,bx1,by1,motion,img_current); 

			objectid_current[t].overlappedone = false; 
		}
		else { // initialize
			IntializeObjectInfo(t);
		} 
	}  
}

void CLocalTracker::LinkEstimateObjectwithCurrentObject(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int **motion,int ***img_current) {
	int t;
	for(t=0;t<MTG;t++)    locked_index[t] = 0;

	int *closeindex = (int *)malloc(sizeof(int)*MTG);
	for(t=0;t<MTG;t++)    closeindex[t] = -1;

	// search the most close one from esitmate histograms
	for(t=0;t<MTG;t++) {
		if(objectid_estimate[t].lock==true) {            
			// find closer one which was detected in estimate frame
			closeindex[t] = FindCloseOneUsingHistogram(t,motion,img_current);
		}
	}        
	// if there is no close one which has similar histogram, choose close one which has the shortest distance between them
	for(t=0;t<MTG;t++) {
		if(objectid_estimate[t].lock==true && closeindex[t]==-1) {
			closeindex[t] = FindCloseOneUsingDistance(t);       
		}
	}
        
	// check history and then update
	for(t=0;t<MTG;t++) {
		if(objectid_estimate[t].lock==true) {            
			// find closer one which was detected in estimate frame
			int close_one = closeindex[t];       
                
			if(close_one != -1) { //if there is a closed one                    
				int min_dist=iWidth*iWidth;
				int mink = -1;
                            
				int x0_p = objectid_estimate[t].bx0;
				int y0_p = objectid_estimate[t].by0;
				int x1_p = objectid_estimate[t].bx1;
				int y1_p = objectid_estimate[t].by1;
				int hx_p = x0_p + (x1_p-x0_p)/2;
				int hy_p = y0_p + (y1_p-y0_p)/2;                    
                    
				// search the closer head candidate
				for(int k=0;k<NTG;k++) { 
					if(target[close_one][k].valid==1 || target[close_one][k].valid==2) { 
						int x0_n=target[close_one][k].bx0;
						int y0_n=target[close_one][k].by0;
						int x1_n=target[close_one][k].bx1;
						int y1_n=target[close_one][k].by1;                            
						int hx_n = x0_n + (x1_n-x0_n)/2;
						int hy_n = y0_n + (y1_n-y0_n)/2;
                            
						int dist = (hx_n-hx_p)*(hx_n-hx_p)+(hy_n-hy_p)*(hy_n-hy_p);
                            
						if(dist<min_dist) {
							min_dist = dist;
							mink = k;
						}
					}  
				}

				int index = 0;
				if(locked_index[close_one]==1) {
					for(int tl=0;tl<MTG;tl++) {
						if(motion_candidate[tl].valid==0) {
							index = tl;
							break;
						}
					}
				}
				else {
					index = close_one;
				}
                                        
				if(mink != -1) {  // if there is a motion block which has valid=0 head candidate
					objectid_current[index].lock = true;
					objectid_current[index].id = objectid_estimate[t].id;
                        
					objectid_current[index].valid = motion_candidate[close_one].valid;
					objectid_current[index].bx0 = motion_candidate[close_one].bx0;
					objectid_current[index].by0 = motion_candidate[close_one].by0;
					objectid_current[index].bx1 = motion_candidate[close_one].bx1;
					objectid_current[index].by1 = motion_candidate[close_one].by1;
					objectid_current[index].px = motion_candidate[close_one].px;
					objectid_current[index].py = motion_candidate[close_one].py; 
                        
					objectid_current[index].hx0 = target[close_one][mink].bx0;
					objectid_current[index].hy0 = target[close_one][mink].by0;
					objectid_current[index].hx1 = target[close_one][mink].bx1;
					objectid_current[index].hy1 = target[close_one][mink].by1;
					objectid_current[index].hx = target[close_one][mink].px;
					objectid_current[index].hy = target[close_one][mink].py;
                            
					locked_index[index]++;
				}
				else  { // if there is no a motion candidate which has valid=0 head candidate, recover based on estimate info.
					// update the object information from estimate history
					objectid_current[index].lock = true;
					objectid_current[index].id = objectid_estimate[t].id;
                            
					objectid_current[index].valid = motion_candidate[close_one].valid;
					objectid_current[index].bx0 = motion_candidate[close_one].bx0;
					objectid_current[index].by0 = motion_candidate[close_one].by0;
					objectid_current[index].bx1 = motion_candidate[close_one].bx1;
					objectid_current[index].by1 = motion_candidate[close_one].by1;
					objectid_current[index].px = motion_candidate[close_one].px;
					objectid_current[index].py = motion_candidate[close_one].py;
                        
					int width_estimate = objectid_estimate[t].bx1-objectid_estimate[t].bx0;
					int width_current = objectid_current[index].bx1-objectid_current[index].bx0;
					double rate_w = (double)width_current/width_estimate;
					int width_head = (int)((objectid_estimate[t].hx1-objectid_estimate[t].hx0)*rate_w);                            
                                                        
					int height_estimate = objectid_estimate[t].by1-objectid_estimate[t].by0;
					int height_current = objectid_current[index].by1-objectid_current[index].by0;
					double rate_h = (double)height_current/height_estimate;
					int height_head = (int)((objectid_estimate[t].hy1-objectid_estimate[t].hy0)*rate_h);                            
                            
					int bx0 = objectid_current[index].bx0;
					int by0 = objectid_current[index].by0;
					int bx1 = objectid_current[index].bx1;
					int by1 = objectid_current[index].by1;                        
                        
					GetExpectedHeadPosition(motion,bx0,by0,bx1,by1,width_head,height_head);

					objectid_current[index].hx0 = bx0+hx0_e;
					objectid_current[index].hy0 = by0+hy0_e;
					objectid_current[index].hx1 = bx0+hx1_e;
					objectid_current[index].hy1 = by0+hy1_e;
					objectid_current[index].hx = bx0+hx0_e+(hx1_e-hx0_e)/2;
					objectid_current[index].hy = by0+hy0_e+(hy1_e-hy0_e)/2;
                                                    
					// for exception
					if(objectid_current[index].hx0>=objectid_current[index].bx0
						&& objectid_current[index].hy0>=objectid_current[index].by0
						&& objectid_current[index].hx1<=objectid_current[index].bx1
						&& objectid_current[index].hy1<=objectid_current[index].by1) {
						locked_index[index]++;
					}
					else {
						IntializeObjectInfo(index);
					}                        
				}
			}
			else {  // delete the objectid if there is no closer motion block
				IntializeObjectInfo(t);
			}
		}
	}     

	free(closeindex);
}

void CLocalTracker::SplitOverlappedMotion(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int **motion,int ***img_current) {
	int i,j,t,s,n,k;

	for(t=0;t<MTG;t++) {
		if(objectid_current[t].valid==1) {                
			int n_overlappedmotion;   // the number of overapped motion candidates		
			int *overlappedindex = (int *)malloc(sizeof(int)*MTG); // the index of overlapped motion candidates
			for(i=0;i<MTG;i++) overlappedindex[i]=-1;
			
			overlappedindex[0] = t;
			n_overlappedmotion = 0;

			for(i=t+1;i<MTG;i++) {
				if(objectid_current[i].valid==1) {
					if(objectid_current[t].bx0==objectid_current[i].bx0 
						&& objectid_current[t].by0==objectid_current[i].by0 
						&& objectid_current[t].bx1==objectid_current[i].bx1 
						&& objectid_current[t].by1==objectid_current[i].by1) {            
						n_overlappedmotion++;
						
						overlappedindex[n_overlappedmotion] = i;
					}                            
				}                    
			}

			// split the motion.
			// even if the motion is not overlapped, use DomotionSplitter for finding the head
			int mx0 = objectid_current[t].bx0;  
			int my0 = objectid_current[t].by0;
			int mx1 = objectid_current[t].bx1;
			int my1 = objectid_current[t].by1;
			ms->DoMotionSplitter(motion,mx0,my0,mx1,my1,n_overlappedmotion+1,MIN_HEAD);
			
			if(n_overlappedmotion==0 && objectid_current[t].overlappedone==false) {
				int index_lib=-1;
				for(int j=0;j<MTG;j++) {
					if(lib_valid[j]==1 && lib_id[j]==objectid_current[t].id) {
						index_lib = j;
						break;
					}
				}       
				
				if(index_lib!=-1) {  // update histogram
					for(i=0;i<256;i++) {                                
						lib_histogram[index_lib][i] = objectid_current_histogram[t][i]; 
					}
				}    
				
				// UpdateObjectInfo(motion_candidate,target,motion,img_current,objectid_current[t].id,mx0,my0,t,0);
				UpdateObjectInfo(motion_candidate,target,mx0,my0,t);
			}
			
			if(n_overlappedmotion>=1) {  // if some motion candidates are overlapped                            
			    int *matchedindex = (int *)malloc(sizeof(int)*(n_overlappedmotion+1));
				for(i=0;i<=n_overlappedmotion;i++) matchedindex[i]=-1;
				
				int *overlappedid = (int *)malloc(sizeof(int)*(n_overlappedmotion+1)); 
				int *overlappedcx_e = (int *)malloc(sizeof(int)*(n_overlappedmotion+1)); 
				int *overlappedcy_e = (int *)malloc(sizeof(int)*(n_overlappedmotion+1)); 
				for(i=0;i<=n_overlappedmotion;i++) {
					overlappedid[i] = objectid_current[overlappedindex[i]].id;
					
					int index=-1;
					for(int j=0;j<MTG;j++) {
						if(objectid_estimate[j].id==overlappedid[i]) {
							index=j;
						}
					}
					overlappedcx_e[i] = objectid_estimate[index].bx0+(objectid_estimate[index].bx1-objectid_estimate[index].bx0)/2;
					overlappedcy_e[i] = objectid_estimate[index].by0+(objectid_estimate[index].by1-objectid_estimate[index].by0)/2;
				}   
				
				// get histogram of splitted motion blocks
				long **histo_p = (long **)malloc(sizeof(long)*(n_overlappedmotion+1));
				if(histo_p==NULL) exit(-1);
				for(i=0;i<=n_overlappedmotion;i++) {
					histo_p[i] = (long *)malloc(sizeof(long)*256);
					if(histo_p[i]==NULL) exit(-1);
				}
				for(i=0;i<=n_overlappedmotion;i++) {
					// get histogram of a new splitted motion candidate
					int bx0 = mx0+ms->bx0[i];
					int by0 = my0+ms->by0[i];
					int bx1 = mx0+ms->bx1[i];
					int by1 = my0+ms->by1[i];               
					histo_p[i] = GetHistogram(bx0,by0,bx1,by1,motion,img_current);
				}
				
				// compare current overlapped motion candidates with splitted motion candidates
				int *isaccupiedindex = (int *)malloc(sizeof(int)*(n_overlappedmotion+1)); // the index of overlapped motion candidates			
				for(i=0;i<=n_overlappedmotion;i++) isaccupiedindex[i]=0;                            
							
				// for debugging
				double *corr_max = (double *)malloc(sizeof(double)*(n_overlappedmotion+1)); 
				for(i=0;i<=n_overlappedmotion;i++) corr_max[i]=-1;
							
				double **corr_table = (double **)malloc(sizeof(double)*(n_overlappedmotion+1));
				if(corr_table==NULL) exit(-1);
				for(i=0;i<=n_overlappedmotion;i++) {
					corr_table[i] = (double *)malloc(sizeof(double)*(n_overlappedmotion+1));
					if(corr_table[i]==NULL) exit(-1);
				}				
				int **truth_table = (int **)malloc(sizeof(int)*(n_overlappedmotion+1));
				if(truth_table==NULL) exit(-1);
				for(i=0;i<=n_overlappedmotion;i++) {
					truth_table[i] = (int *)malloc(sizeof(int)*(n_overlappedmotion+1));
					if(truth_table[i]==NULL) exit(-1);
				}
				for(i=0;i<=n_overlappedmotion;i++) {
					for(j=0;j<=n_overlappedmotion;j++) truth_table[i][j] = 1;
				}
				
				for(i=0;i<=n_overlappedmotion;i++) {   
					for(j=0;j<=n_overlappedmotion;j++) {   
						int index_lib = -1;  // lib
						for(k=0;k<MTG;k++) {
							if(lib_id[k]==overlappedid[j]) { 
								index_lib = k;
								break;
							}
						}
					
						corr_table[i][j] = GetCorrelation(histo_p[i],lib_histogram[index_lib]);  
					}
				}
				
				// distance
				double **dist_table = (double **)malloc(sizeof(double)*(n_overlappedmotion+1));
				if(dist_table==NULL) exit(-1);
				for(i=0;i<=n_overlappedmotion;i++) {
					dist_table[i] = (double *)malloc(sizeof(double)*(n_overlappedmotion+1));
					if(dist_table[i]==NULL) exit(-1);
				}

				for(i=0;i<=n_overlappedmotion;i++) {
					// current center of motion block
					int cx_p = mx0 + ms->bx0[i] + (ms->bx1[i]-ms->bx0[i])/2;
					int cy_p = my0 + ms->by0[i] + (ms->by1[i]-ms->by0[i])/2;                    
										
					for(j=0;j<=n_overlappedmotion;j++) {
						int cx_n = overlappedcx_e[j];
                        int cy_n = overlappedcy_e[j];

						dist_table[i][j] = (cx_n-cx_p)*(cx_n-cx_p) + (cy_n-cy_p)*(cy_n-cy_p);
					}
				}
				
				for(n=0;n<=n_overlappedmotion;n++) {
					double max_corr=-1;
					int max_hist_i=-1,max_hist_j=-1;
					for(i=0;i<=n_overlappedmotion;i++) {   // lib
						for(j=0;j<=n_overlappedmotion;j++) {
							if(corr_table[i][j]>max_corr && truth_table[i][j]==1) {
								max_corr=corr_table[i][j];
								max_hist_i=i;
								max_hist_j=j;                                    
							}
						}
					}
					if(max_corr>MIN_CORR) {
						for(s=0;s<=n_overlappedmotion;s++) truth_table[s][max_hist_j]=0;;
						for(s=0;s<=n_overlappedmotion;s++) truth_table[max_hist_i][s]=0;
						
						corr_max[max_hist_i] = corr_table[max_hist_i][max_hist_j];                        
						matchedindex[max_hist_i] = max_hist_j;
						isaccupiedindex[max_hist_i]=1;
					}
				}
				
				// find shorter distance
				double *dist_min = (double *)malloc(sizeof(double)*(n_overlappedmotion+1)); // for debugging
				for(i=0;i<=n_overlappedmotion;i++) dist_min[i]=0;
				
				for(n=0;n<=n_overlappedmotion;n++) {
					double min_dist=iWidth*iWidth+iHeight*iHeight;
					int min_dist_i=-1,min_dist_j=-1;
					for(j=0;j<=n_overlappedmotion;j++) {
						for(i=0;i<=n_overlappedmotion;i++) {
							if(dist_table[i][j]<min_dist && truth_table[i][j]==1) {
								min_dist = dist_table[i][j];
								min_dist_i=i;
								min_dist_j=j;                                        
							}
						}
					}
					if(min_dist_i!=-1) {
						for(s=0;s<=n_overlappedmotion;s++) truth_table[s][min_dist_j]=0;;
						for(s=0;s<=n_overlappedmotion;s++) truth_table[min_dist_i][s]=0;
						
						if(dist_table[min_dist_i][min_dist_j]<residue) {
							matchedindex[min_dist_i] = min_dist_j;
							dist_min[min_dist_i] = dist_table[min_dist_i][min_dist_j];                                
							isaccupiedindex[min_dist_i] = 1;
						}                                                        
						else {
							//########## Error of local tracking (make a new object) ############
							matchedindex[min_dist_i] = min_dist_j;
							dist_min[min_dist_i] = dist_table[min_dist_i][min_dist_j];                                
							isaccupiedindex[min_dist_i] = 1;                                
						}
					}
				}
				
				// update the overlapped object information from splitted motion such as head position
				for(k=0;k<=n_overlappedmotion;k++) {
					UpdateOverlappedObjectInfo(motion_candidate,target,overlappedindex[matchedindex[k]],mx0,my0,k,overlappedid[matchedindex[k]],histo_p[k]);
				}

				// free
				for(i=0;i<=n_overlappedmotion;i++) {
					free(dist_table[i]);
					free(corr_table[i]);
					free(truth_table[i]);
					free(histo_p[i]);
				}
				free(dist_table);
				free(corr_table);
				free(truth_table);
				free(histo_p);

				free(matchedindex);
				free(overlappedid); free(isaccupiedindex); free(dist_min); free(corr_max);
				free(overlappedcx_e); free(overlappedcy_e);
				
			}
			free(overlappedindex);
		}
	}
}

void CLocalTracker::InsertNewObject(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int **motion,int ***img_current) {
	// if there is new one
	for(int t=0;t<MTG;t++) {            
	// if no more locked block is exist, the last of candidates are new objects.
		if(locked_index[t]==0 && objectid_current[t].valid==1) {
			bool isNew=false;                
			bool hasValidone=false;
			bool isException=false;
			int index_sameone=-1;
                
			// correlaton comparision with libraries
			double corr=0,max_corr=-1;
			int index_close=-1;
			for(int i=0;i<MTG;i++) {
				if(lib_valid[i]==1) {
					corr = GetCorrelation(lib_histogram[i],objectid_current_histogram[t]);
					if(corr>max_corr) {
						max_corr = corr;
						index_close = i; // choose the close one which has the most similar histogram
					}
				}
			}
                
			if(max_corr<MIN_EXCEPTIONCORR) {
				isNew=true; isException=false;
			} 
			else {  // check the exception
				for(int i=0;i<MTG;i++) {
					if(objectid_current[i].valid==1) {
						if(objectid_current[i].id==lib_id[index_close]) {
							index_sameone = i;
							double corr_lt = GetCorrelation(objectid_current_histogram[i],lib_histogram[index_close]);
                                
							if(max_corr>corr_lt) {
								isNew = false;
								isException = true;
							}
							else {
								isNew = true;
								isException = false;
							}
                                
							break;
						}
					}
				}
			} 
                
			if(isException==true) {
				// delete this motion object
				objectid_current[index_sameone].lock = false;
				objectid_current[index_sameone].id = -1;
				objectid_current[index_sameone].valid = 0;
				objectid_current[index_sameone].bx0 = 0;
				objectid_current[index_sameone].by0 = 0;
				objectid_current[index_sameone].bx1 = 0;
				objectid_current[index_sameone].by1 = 0;
				objectid_current[index_sameone].px = 0;
				objectid_current[index_sameone].py = 0;
                    
				objectid_current[index_sameone].hx0 = 0;
				objectid_current[index_sameone].hy0 = 0;
                objectid_current[index_sameone].hx1 = 0;
                objectid_current[index_sameone].hy1 = 0;
                objectid_current[index_sameone].hx = 0;
                objectid_current[index_sameone].hy = 0;
                    
                for(int j=0;j<256;j++)
                    objectid_current_histogram[index_sameone][j]=0;
                    
                // update new one as index_sameone id
				for(int k=0;k<NTG;k++) {
					if(target[t][k].valid==1) {  // lock
						objectid_current[t].lock = true;
						objectid_current[t].id = lib_id[index_close];
						objectid_current[t].valid = motion_candidate[t].valid;
						objectid_current[t].bx0 = motion_candidate[t].bx0;
						objectid_current[t].by0 = motion_candidate[t].by0;
						objectid_current[t].bx1 = motion_candidate[t].bx1;
						objectid_current[t].by1 = motion_candidate[t].by1;
						objectid_current[t].px = motion_candidate[t].px;
						objectid_current[t].py = motion_candidate[t].py;
						objectid_current[t].hx0 = target[t][k].bx0;
						objectid_current[t].hy0 = target[t][k].by0;
						objectid_current[t].hx1 = target[t][k].bx1;
						objectid_current[t].hy1 = target[t][k].by1;
						objectid_current[t].hx = target[t][k].px;
						objectid_current[t].hy = target[t][k].py;
						hasValidone = true;
                            
						// make a histogram library
						int index_lib=-1;
						for(int j=0;j<MTG;j++) {
							if(lib_valid[j]==0) {
								index_lib = j;
								break;
							}
						}
						if(index_lib!=-1) {  // update histogram if there is space
							for(int i=0;i<256;i++) {
								lib_histogram[index_lib][i] = objectid_current_histogram[t][i];
							}
							lib_id[index_lib] = nid;
							lib_valid[index_lib] = 1;
						}
						break;   // a motion block can have only one head candidate
					}
				}
			}
                
			if(isNew==true) {
				for(int k=0;k<NTG;k++) {
					if(target[t][k].valid==1) {  // lock
						objectid_current[t].lock = true;
						objectid_current[t].id = nid;
						objectid_current[t].valid = motion_candidate[t].valid;
						objectid_current[t].bx0 = motion_candidate[t].bx0;
						objectid_current[t].by0 = motion_candidate[t].by0;
						objectid_current[t].bx1 = motion_candidate[t].bx1;
						objectid_current[t].by1 = motion_candidate[t].by1;
						objectid_current[t].px = motion_candidate[t].px;
						objectid_current[t].py = motion_candidate[t].py;
						objectid_current[t].hx0 = target[t][k].bx0;
						objectid_current[t].hy0 = target[t][k].by0;
						objectid_current[t].hx1 = target[t][k].bx1;
						objectid_current[t].hy1 = target[t][k].by1;
						objectid_current[t].hx = target[t][k].px;
						objectid_current[t].hy = target[t][k].py;
						hasValidone = true;
                            
						// make a histogram library
						int index_lib=-1;
						for(int j=0;j<MTG;j++) {
							if(lib_valid[j]==0) {
								index_lib = j;
								break;
							}
						}
						if(index_lib!=-1) {  // update histogram if there is space
							for(int i=0;i<256;i++) {
								lib_histogram[index_lib][i] = objectid_current_histogram[t][i];
							}
							lib_id[index_lib] = nid;
							lib_valid[index_lib] = 1;
						}
                            
						// increase the number of index
						nid++;
                            
						break;   // a motion block can have only one head candidate
					}
				}
			}
                
			if(hasValidone==false) {  // if motion box doesn't have any head candidate(valid=1), delete it.
				// since the current id is overlapped esitmate id, the esitmate id is deleted
				IntializeObjectInfo(t);
			} 
		}
	}
}

long *CLocalTracker::GetHistogram(int bx0,int by0,int bx1,int by1,int **motion,int ***img_current) {
	int i,j;
	long *histo = (long *)malloc(sizeof(long)*256);
	for(i=0;i<256;i++) histo[i]=0;
        
	for(i=bx0;i<=bx1;i++) {
		for(j=by0;j<=by1;j++) {
			if(motion[i][j]==1) {
				histo[img_current[0][i][j]]++;
				histo[img_current[1][i][j]]++;
				histo[img_current[2][i][j]]++;
			}
		}
	}
                
	return histo;
}


void CLocalTracker::UpdateObjectInfo(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int mx0,int my0,int n) {
	int i,j;

	int bx0 = mx0+ms->bx0[0];  // k is the index in splitted motions
	int by0 = my0+ms->by0[0];
	int bx1 = mx0+ms->bx1[0];
	int by1 = my0+ms->by1[0];
	
	int hx0=0,hy0=0,hx1=0,hy1=0;
	int tx0,ty0,tx1,ty1;

	// if the new motion block has detected head, use the head for the motion, otherwise use the head from motionsplitter
	int ishead=0;
	for(i=0;i<MTG;i++) {            
		if(motion_candidate[i].valid==1) {
			for(j=0;j<NTG;j++) {
				if(target[i][j].valid==1) {
					tx0 = target[i][j].bx0; 
					ty0 = target[i][j].by0; 
					tx1 = target[i][j].bx1; 
					ty1 = target[i][j].by1;
					if(tx0>=bx0 && ty0>=by0 && tx1<=bx1 && ty1<=by1) {
						hx0 = tx0;
						hy0 = ty0;
						hx1 = tx1;
						hy1 = ty1; 
						ishead=1;                        
						break;
					}
				}
			}
		}
	}
	if(ishead==0) { // if head candidate(valid==1) does not exist, search head candidate (valid==2)
		for(i=0;i<MTG;i++) {            
			if(motion_candidate[i].valid==1) {
				for(j=0;j<NTG;j++) {
					if(target[i][j].valid==2) {
						tx0 = target[i][j].bx0; 
						ty0 = target[i][j].by0; 
						tx1 = target[i][j].bx1; 
						ty1 = target[i][j].by1;
						if(tx0>=bx0 && ty0>=by0 && tx1<=bx1 && ty1<=by1) {
							hx0 = tx0;
							hy0 = ty0;
							hx1 = tx1;
							hy1 = ty1; 
							ishead=1;
							break;
						}                        
					}
				}
			}
		}
	}	
	if(ishead==0) { // there are no head candidate(valid==1) and head candidate(valid==2), use the expected face candidate from motionsplitter.
		hx0 = mx0+ms->hx0[0];
		hy0 = my0+ms->hy0[0];
		hx1 = mx0+ms->hx1[0];
		hy1 = my0+ms->hy1[0];              
	}
	
	objectid_current[n].hx0 = hx0;
	objectid_current[n].hy0 = hy0;
	objectid_current[n].hx1 = hx1;
	objectid_current[n].hy1 = hy1;
	objectid_current[n].hx = hx0+(hx1-hx0)/2;
	objectid_current[n].hy = hy0+(hy1-hy0)/2;	
}

void CLocalTracker::UpdateOverlappedObjectInfo(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int n,int mx0,int my0,int k,int id,long *histo) {
	int i,j;

	int bx0 = mx0+ms->bx0[k];  // k is the index in splitted motions
	int by0 = my0+ms->by0[k];
	int bx1 = mx0+ms->bx1[k];
	int by1 = my0+ms->by1[k];
	
	int hx0=0,hy0=0,hx1=0,hy1=0;
	int tx0,ty0,tx1,ty1;
	// if the new motion block has detected head, use the head for the motion, otherwise use the head from motionsplitter
	int ishead=0;
	for(i=0;i<MTG;i++) {            
		for(j=0;j<NTG;j++) {
			if(target[i][j].valid==1) {
				tx0 = target[i][j].bx0; 
				ty0 = target[i][j].by0; 
				tx1 = target[i][j].bx1; 
				ty1 = target[i][j].by1;
				if(tx0>=bx0 && ty0>=by0 && tx1<=bx1 && ty1<=by1) {
					hx0 = tx0;
					hy0 = ty0;
					hx1 = tx1;
					hy1 = ty1; 
					ishead=1;                        
					break;
				}
			}
		}
	}
	if(ishead==0) { // if head candidate(valid==1) does not exist, search head candidate (valid==2)
		for(i=0;i<MTG;i++) {            
			for(j=0;j<NTG;j++) {
				if(target[i][j].valid==2) {
					tx0 = target[i][j].bx0; 
					ty0 = target[i][j].by0; 
					tx1 = target[i][j].bx1; 
					ty1 = target[i][j].by1;
					if(tx0>=bx0 && ty0>=by0 && tx1<=bx1 && ty1<=by1) {
						hx0 = tx0;
						hy0 = ty0;
						hx1 = tx1;
						hy1 = ty1; 
						ishead=1;
						break;
					}                        
				}
			}
		}
	}	
	if(ishead==0) { // there are no head candidate(valid==1) and head candidate(valid==2), use the expected face candidate from motionsplitter.
		hx0 = mx0+ms->hx0[k];
		hy0 = my0+ms->hy0[k];
		hx1 = mx0+ms->hx1[k];
		hy1 = my0+ms->hy1[k];              
	}
	
	objectid_current[n].lock = true;
	
	// for exception
	objectid_current[n].id = id;
	
	objectid_current[n].valid = 1;
	objectid_current[n].bx0 = bx0;
	objectid_current[n].by0 = by0;
	objectid_current[n].bx1 = bx1;
	objectid_current[n].by1 = by1;
	objectid_current[n].px = bx0+(bx1-bx0)/2;
	objectid_current[n].py = by0+(by1-by0)/2;
	
	objectid_current[n].hx0 = hx0;
	objectid_current[n].hy0 = hy0;
	objectid_current[n].hx1 = hx1;
	objectid_current[n].hy1 = hy1;
	objectid_current[n].hx = hx0+(hx1-hx0)/2;
	objectid_current[n].hy = hy0+(hy1-hy0)/2;
	
	for(i=0;i<256;i++)
		objectid_current_histogram[n][i] = histo[i];

	// for display
	objectid_current[n].overlappedone=true;                                                               
}

void CLocalTracker::GetExpectedHeadPosition(int **motion,int bx0,int by0,int bx1,int by1,int hW,int hH) {        
	int i,j;

	int w = bx1-bx0+1;
	int h = by1-by0+1;
 
	// get small motion image
	int **motion_s = (int **)malloc(sizeof(int)*w);
	if(motion_s==NULL) exit(-1);
	for(i=0;i<w;i++) {
		motion_s[i] = (int *)malloc(sizeof(int)*h);
		if(motion_s[i]==NULL) exit(-1);
	}
	for(i=0;i<w;i++)
		for(j=0;j<h;j++)	motion_s[i][j] = motion[bx0+i][by0+j]; 
                            
	// get histogram of motion        
	int *motion_histo = GetMotionHisto(motion_s,w,h);

	// get max position of histogram
	int max_histo=0;
	int center = w/2;
	for(int n=0;n<w;n++) {
		if(motion_histo[n]>max_histo) {
			max_histo = motion_histo[n];
			center = n;
		}
	}
        
	// make mask for head
	int **mask_head = (int **)malloc(sizeof(int)*hW);
	if(mask_head==NULL) exit(-1);
	for(i=0;i<hW;i++) {
		mask_head[i] = (int *)malloc(sizeof(int)*hH);
		if(mask_head[i]==NULL) exit(-1);
	}
	int mask_cx = hW/2;
	int mask_cy = hH/2;
	for(int m=0;m<hW;m++) {
		for(int n=0;n<hH;n++) {
			if((mask_cx-m)*(mask_cx-m)+(mask_cy-n)*(mask_cy-n)<(hW/2)*(hW/2))
				mask_head[m][n] = 1;
			else
				mask_head[m][n] = 0;
		}
	}
                
	int top_head=0; // search the position of head top 
	for(i=0;i<h;i++) {
		if(motion_s[center][i]==1) {
			top_head=i;
			break;
		}
	}

	int t;
	int max_weight_head = 0;
	int center_head = w/2;
	for(t=hW/2;t<w-hW/2;t++) {
		int weight_head = 0;
		for(m=0;m<hW;m++) {
			for(n=0;n<hH;n++) {
				if(m+t-hW/2>=0 && m+t-hW/2<w && n+top_head>=0 && n+top_head<h)
					weight_head = weight_head + mask_head[m][n]*motion_s[m+t-hW/2][n+top_head];
			}
		}
                
		if(weight_head>max_weight_head) {
			max_weight_head = weight_head;
			center_head = t;
		}

		hx0_e = center_head-hW/2;
		hy0_e = top_head;
		hx1_e = center_head+hW/2;
		hy1_e = top_head+hH;    
	}

	free(motion_histo);

	for(i=0;i<w;i++) {
		free(motion_s[i]);
	}
	free(motion_s);	
}

int *CLocalTracker::GetMotionHisto(int **motion_s,int Width,int Height) {
	int *motion_histo = (int *)malloc(sizeof(int)*Width);
	for(int i=0;i<Width;i++) motion_histo[i]=0;

	for(int m=0;m<Width;m++) {
		for(int n=0;n<Height;n++) {
			if(motion_s[m][n]==1)
				motion_histo[m]++;                
		}
	}
        
	return motion_histo;
}

void CLocalTracker::IntializeObjectInfo(int t) {
	objectid_current[t].lock = false;
	objectid_current[t].id = -1;
        
	objectid_current[t].valid = 0;
	objectid_current[t].bx0 = 0;
	objectid_current[t].by0 = 0;
	objectid_current[t].bx1 = 0;
	objectid_current[t].by1 = 0;
	objectid_current[t].px = 0;
	objectid_current[t].py = 0;

	objectid_current[t].hx0 = 0;
	objectid_current[t].hy0 = 0;
	objectid_current[t].hx1 = 0;
	objectid_current[t].hy1 = 0;
	objectid_current[t].hx = 0;
	objectid_current[t].hy = 0;

	for(int q=0;q<256;q++) 
		objectid_current_histogram[t][q] = 0;
}

int CLocalTracker::FindCloseOneUsingHistogram(int t,int **motion,int ***img_current) {
	int i;
	int esitmate_id = objectid_estimate[t].id;
        
	// search t-th histogram from library
	int index_lib=-1;
	for(i=0;i<MTG;i++) {                        
		if(lib_id[i]==esitmate_id) {
			index_lib = i;
			break;
		}            
	}

	int index_current=-1;        
	if(index_lib != -1) {
		double corr=0,max_corr=-1;
		for(i=0;i<MTG;i++) {                        
			if(objectid_current[i].valid==1) {                        
				corr = GetCorrelation(lib_histogram[index_lib],objectid_current_histogram[i]);
				if(corr>max_corr) {
					max_corr = corr;
					index_current = i; // choose the close one which has the most similar histogram
				}
			}
		}
		if(max_corr<MIN_CORR) index_current = -1;
	}

	return index_current;
}

int CLocalTracker::FindCloseOneUsingDistance(int t) {
	int esitmate_id = objectid_estimate[t].id; 
        
	int index_current=-1;        
	int cx_p = objectid_estimate[t].bx0 + (objectid_estimate[t].bx1-objectid_estimate[t].bx0)/2;            
	int cy_p = objectid_estimate[t].by0 + (objectid_estimate[t].by1-objectid_estimate[t].by0)/2;
                
	int min_dist = 0;
	int i0 = 0;
	for(int i=0;i<MTG;i++) {                        
		if(objectid_current[i].valid==1) {                        
			int cx_n = objectid_current[i].bx0+(objectid_current[i].bx1-objectid_current[i].bx0)/2;
			int cy_n = objectid_current[i].by0+(objectid_current[i].by1-objectid_current[i].by0)/2;                
			min_dist = (cx_n-cx_p)*(cx_n-cx_p) + (cy_n-cy_p)*(cy_n-cy_p);                
			index_current = i;
                
			i0 = i;
			break;
		}
	}
	if(min_dist!=0) {
		for(int i=i0+1;i<MTG;i++) {                        
			if(objectid_current[i].valid==1) {                        
				int cx_n = objectid_current[i].bx0+(objectid_current[i].bx1-objectid_current[i].bx0)/2;
				int cy_n = objectid_current[i].by0+(objectid_current[i].by1-objectid_current[i].by0)/2;                
				int dist = (cx_n-cx_p)*(cx_n-cx_p) + (cy_n-cy_p)*(cy_n-cy_p);
                
				if(dist<min_dist) {
					min_dist = dist;
					index_current = i;
				}
			}
		}
	}          
        
	// all motion bounding box is too far away from current one
	if(min_dist>residue) {
		index_current = -1;
	}
                
	return index_current;
}
    

void CLocalTracker::UpdateLibraryInfo() {
	// delete old library
	for(int t=0;t<MTG;t++) {
		if(lib_valid[t]==1) {
			int isexist=0;
			for(int i=0;i<MTG;i++) {
				if(lib_id[t]==objectid_current[i].id) {
					isexist=1;                    
					break;
				}
			}
			if(isexist==0) {
				lib_valid[t] = 0;
				lib_id[t] = -1;
				for(int i=0;i<256;i++) {
					lib_histogram[t][i]=0;
				}
			}
		}
	}
}
    
double CLocalTracker::GetCorrelation(long *t,long *r) {
	double u=0,d1=0,d2=0;
	double A=0,d=0;
	int k;

	for(k=0;k<256;k++) { // last color context do not used to delete background (white)
		u = u+t[k]*r[k];
		d1 = d1+t[k]*t[k];
		d2 = d2+r[k]*r[k];
	}

	d = sqrt(d1)*sqrt(d2); 
         
	if(d==0) A = 1;
	else if(u/d>1) A = 1;
	else A = 1.0-acos(u/d);

	return A;
}
    
HUMANID CLocalTracker::GetMotion(int k)
{
	HUMANID tg;

	tg.valid = objectid_current[k].valid;
	tg.bx0 = objectid_current[k].bx0;
	tg.by0 = objectid_current[k].by0;
	tg.bx1 = objectid_current[k].bx1;
	tg.by1 = objectid_current[k].by1;
	tg.hx0 = objectid_current[k].hx0;
	tg.hy0 = objectid_current[k].hy0;
	tg.hx1 = objectid_current[k].hx1;
	tg.hy1 = objectid_current[k].hy1;

	return tg;
}
