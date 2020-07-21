// LocalTracker.cpp: implementation of the CLocalTracker class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "HumanTracker.h"
#include "LocalTracker.h"

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
		objectid_previous[t].id = t+1;
	}

	iWidth = 704;
	iHeight = 480;	
}

CLocalTracker::~CLocalTracker()
{

}

void CLocalTracker::DoLocalTracker(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int **motion) {
	int t;
	UpdateNewObjectID(motion_candidate);
        
	int locked_id[MTG];
	for(t=0;t<MTG;t++)    locked_id[t] = 0;
        
	// check history and then update
	for(t=0;t<MTG;t++) {
		if(objectid_previous[t].lock==true) {            
			int cx_p = objectid_previous[t].hx0 + (objectid_previous[t].hx1-objectid_previous[t].hx0)/2;
			int cy_p = objectid_previous[t].hy0 + (objectid_previous[t].hy1-objectid_previous[t].hy0)/2;
                
			// find closer one which was detected in previous frame
			int closed_one = FindClosedOne(cx_p,cy_p,motion_candidate);       
                
			if(closed_one != -1) { //if there is a closed one                    
				int minDist=iWidth*iWidth;
				int mink = -1;
                            
				int x0_p = objectid_previous[t].hx0;
				int y0_p = objectid_previous[t].hy0;
				int x1_p = objectid_previous[t].hx1;
				int y1_p = objectid_previous[t].hy1;
				int hx_p = x0_p + (x1_p-x0_p)/2;
				int hy_p = y0_p + (y1_p-y0_p)/2;                    
                    
				// search the closer head candidate
				for(int k=0;k<NTG;k++) { 
					if(target[closed_one][k].valid==1 || target[closed_one][k].valid==2) { 
						int x0_n=target[closed_one][k].bx0;
						int y0_n=target[closed_one][k].by0;
						int x1_n=target[closed_one][k].bx1;
						int y1_n=target[closed_one][k].by1;                            
						int hx_n = x0_n + (x1_n-x0_n)/2;
						int hy_n = y0_n + (y1_n-y0_n)/2;
                            
						int newDist = (hx_n-hx_p)*(hx_n-hx_p)+(hy_n-hy_p)*(hy_n-hy_p);
                            
						if(newDist<minDist) {
							minDist = newDist;
							mink = k;
						}
					}  
				}

				int id_num = 0;
				if(locked_id[closed_one]==1) {
					for(int tl=0;tl<MTG;tl++) {
						if(motion_candidate[tl].valid==0) {
							id_num = tl;
							break;
						}
					}
				}
				else {
					id_num = closed_one;
				}
                                        
				if(mink != -1) {  // if there is a motion block which has valid=0 head candidate
					objectid_current[id_num].lock = true;
					objectid_current[id_num].id = objectid_previous[t].id;
                        
					objectid_current[id_num].valid = motion_candidate[closed_one].valid;
					objectid_current[id_num].bx0 = motion_candidate[closed_one].bx0;
					objectid_current[id_num].by0 = motion_candidate[closed_one].by0;
					objectid_current[id_num].bx1 = motion_candidate[closed_one].bx1;
					objectid_current[id_num].by1 = motion_candidate[closed_one].by1;
					objectid_current[id_num].px = motion_candidate[closed_one].px;
					objectid_current[id_num].py = motion_candidate[closed_one].py; 
                        
					objectid_current[id_num].hx0 = target[closed_one][mink].bx0;
					objectid_current[id_num].hy0 = target[closed_one][mink].by0;
					objectid_current[id_num].hx1 = target[closed_one][mink].bx1;
					objectid_current[id_num].hy1 = target[closed_one][mink].by1;
					objectid_current[id_num].hx = target[closed_one][mink].px;
					objectid_current[id_num].hy = target[closed_one][mink].py;
                            
					locked_id[id_num]++;
				}
				else  { // if there is no a motion candidate which has valid=0 head candidate, recover based on previous info.
					// update the object information from previous history
					objectid_current[id_num].lock = true;
					objectid_current[id_num].id = objectid_previous[t].id;
                            
					objectid_current[id_num].valid = motion_candidate[closed_one].valid;
					objectid_current[id_num].bx0 = motion_candidate[closed_one].bx0;
					objectid_current[id_num].by0 = motion_candidate[closed_one].by0;
					objectid_current[id_num].bx1 = motion_candidate[closed_one].bx1;
					objectid_current[id_num].by1 = motion_candidate[closed_one].by1;
					objectid_current[id_num].px = motion_candidate[closed_one].px;
					objectid_current[id_num].py = motion_candidate[closed_one].py;
                        
					int cx_n = objectid_current[id_num].bx0+(objectid_current[id_num].bx1-objectid_current[id_num].bx0)/2;                        
					int cy_n = objectid_previous[t].hy0 + (objectid_previous[t].hy1-objectid_previous[t].hy0)/2;

					int width_previous = objectid_previous[t].bx1-objectid_previous[t].bx0;
					int width_current = objectid_current[id_num].bx1-objectid_current[id_num].bx0;
					double rate_w = (double)width_current/width_previous;
					int width_head = (int)((objectid_previous[t].hx1-objectid_previous[t].hx0)*rate_w);                            
                                                        
					int height_previous = objectid_previous[t].by1-objectid_previous[t].by0;
					int height_current = objectid_current[id_num].by1-objectid_current[id_num].by0;
					double rate_h = (double)height_current/height_previous;
					int height_head = (int)((objectid_previous[t].hy1-objectid_previous[t].hy0)*rate_h);                            
                            
					objectid_current[id_num].hx0 = cx_n-width_head/2;
					objectid_current[id_num].hy0 = cy_n-height_head/2;
					objectid_current[id_num].hx1 = cx_n+width_head/2;
					objectid_current[id_num].hy1 = cy_n+height_head/2;
					objectid_current[id_num].hx = objectid_previous[t].hx;
					objectid_current[id_num].hy = objectid_previous[t].hy;
                                                    
					if(objectid_current[id_num].hx0>=objectid_current[id_num].bx0
						&& objectid_current[id_num].hy0>=objectid_current[id_num].by0
						&& objectid_current[id_num].hx1<=objectid_current[id_num].bx1
						&& objectid_current[id_num].hy1<=objectid_current[id_num].by1) {
						locked_id[id_num]++;
					}
					else {
						IntializeTheID(id_num);
					}                        
				}
			}
			else {  // delete the objectid if there is no closer motion block
				IntializeTheID(t);
			}
		}
	}     
                
	// if there is two blocks which have same motion block and head block
	for(t=0;t<MTG;t++) {
		int isOne=0;
		if(objectid_current[t].valid==1) {
			for(int m=0;m<MTG;m++) {
				if(objectid_current[m].valid==1) {
					if(t != m) { // if same motion block
						if(objectid_current[t].bx0==objectid_current[m].bx0 
							&& objectid_current[t].by0==objectid_current[m].by0
							&& objectid_current[t].bx1==objectid_current[m].bx1
							&& objectid_current[t].by1==objectid_current[m].by1
							&& objectid_current[t].hx0==objectid_current[m].hx0
							&& objectid_current[t].hy0==objectid_current[m].hy0
							&& objectid_current[t].hx1==objectid_current[m].hx1
							&& objectid_current[t].hy1==objectid_current[m].hy1) {
							isOne++;
						}
						if(isOne==1) {
							boolean isExist=false;
							for(int k=0;k<NTG;k++) {
								if(target[m][k].valid==1 || target[m][k].valid==2) {
									if(objectid_current[m].hx0!=target[t][k].bx0) {
										objectid_current[m].lock = true;                       
										objectid_current[m].hx0 = target[t][k].bx0;
										objectid_current[m].hy0 = target[t][k].by0;
										objectid_current[m].hx1 = target[t][k].bx1;
										objectid_current[m].hy1 = target[t][k].by1;
										objectid_current[m].hx = target[t][k].px;
										objectid_current[m].hy = target[t][k].py;
										isExist = true;
										break;
									}
								}
							}                  
							if(isExist==false) {
								IntializeTheID(m);
							}
						}
					}
				}                    
			}
		}
	}
                
	// if there is new one
	for(t=0;t<MTG;t++) {
		// if no more locked block is exist, the last of candidates are new objects.
		if(locked_id[t]==0 && objectid_current[t].valid==1) {
			boolean isvalidone=false;
			for(int k=0;k<NTG;k++) {
				if(target[t][k].valid==1) {  // lock
					objectid_current[t].lock = true;
					objectid_current[t].id = nid;
					nid ++;
                        
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
                        
					isvalidone = true;

					break;   // a motion block can have only one head candidate
				}                    
			}

			if(isvalidone==false) {  // if motion box doesn't have any head candidate(valid=1), delete it.
				IntializeTheID(t);
			}
		}
	}    
}

void CLocalTracker::IntializeTheID(int t) {
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
}

int CLocalTracker::FindClosedOne(int cx_p,int cy_p,MOTIONCANDIDATE motion_candidate[MTG]) {
	int min_dist=0, min_dist_id=-1;        
	int dist;
               
	min_dist=iWidth*iHeight;
	for(int t=0;t<MTG;t++) {                        
		if(motion_candidate[t].valid==1) {
			int cx_n = motion_candidate[t].bx0+(motion_candidate[t].bx1-motion_candidate[t].bx0)/2;
			int cy_n = motion_candidate[t].by0+(motion_candidate[t].by1-motion_candidate[t].by0)/2;
                
			dist = (cx_n-cx_p)*(cx_n-cx_p) + (cy_n-cy_p)*(cy_n-cy_p);
			if(dist<min_dist) {
				min_dist = dist;
				min_dist_id = t;
			}
		}
	}
        
	return min_dist_id;
}
    
void CLocalTracker::UpdateNewObjectID(MOTIONCANDIDATE motion_candidate[MTG]) {
	// update objectid_previous
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
		}
		else { // initialize
			IntializeTheID(t);
		} 
	} 
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
