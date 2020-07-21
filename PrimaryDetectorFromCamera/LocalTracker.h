// LocalTracker.h: interface for the CLocalTracker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOCALTRACKER_H__5A583B1D_4A4B_47F4_932B_589ECBCA09D1__INCLUDED_)
#define AFX_LOCALTRACKER_H__5A583B1D_4A4B_47F4_932B_589ECBCA09D1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "motiontracker.h"
#include "detection.h"
#include "parameters.h"
#include "motionsplit.h"

struct HUMANID
{
	bool lock;
	int id;
	int bx0,by0,bx1,by1,px,py,valid;
	int hx0,hy0,hx1,hy1,hx,hy;
	bool overlappedone;
};

class CLocalTracker  
{
public:
	CLocalTracker();
	virtual ~CLocalTracker();

	MotionSplit *ms;

	int nid;
	int iWidth,iHeight;

	void DoLocalTracker(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int **motion,int ***img_current);
	HUMANID GetMotion(int k);

	HUMANID objectid_current[MTG];
	HUMANID objectid_previous[MTG];
	HUMANID objectid_estimate[MTG];

	long **objectid_previous_histogram;
	long **objectid_estimate_histogram;
	long **objectid_current_histogram;

	int hx0_e,hy0_e,hx1_e,hy1_e;
	int residue; 

	// histogram library
    int *lib_id;
    int *lib_valid;
    long **lib_histogram;
    long **motion_histogram;   
	int *locked_index;
    
	
protected:
	void IntializeObjectInfo(int t);
	int FindClosedOne(int cx_p,int cy_p,MOTIONCANDIDATE motion_candidate[MTG]);
	void UpdateCurrentInfo(MOTIONCANDIDATE motion_candidate[MTG],int **motion,int ***img_current);
	void LinkEstimateObjectwithCurrentObject(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int **motion,int ***img_current);
	void UpdatePreviousInfo();
	void UpdateEstimateInfo();
	long *GetHistogram(int bx0,int by0,int bx1,int by1,int **motion,int ***img_current);
	int FindCloseOneUsingHistogram(int t,int **motion,int ***img_current);
	double GetCorrelation(long *t,long *r);
	int FindCloseOneUsingDistance(int t);
	void GetExpectedHeadPosition(int **motion,int bx0,int by0,int bx1,int by1,int hW,int hH);
	int *GetMotionHisto(int **motion_s,int Width,int Height);
	void InsertNewObject(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int **motion,int ***img_current);
	void UpdateLibraryInfo();
	void SplitOverlappedMotion(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int **motion,int ***img_current);
	void UpdateOverlappedObjectInfo(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int n,int mx0,int my0,int k,int id,long *histo);
	void UpdateObjectInfo(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int mx0,int my0,int n);
};

#endif // !defined(AFX_LOCALTRACKER_H__5A583B1D_4A4B_47F4_932B_589ECBCA09D1__INCLUDED_)
