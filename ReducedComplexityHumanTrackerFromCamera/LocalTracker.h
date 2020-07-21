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

struct HUMANID
{
	bool lock;
	int id;
	int bx0,by0,bx1,by1,px,py,valid;
	int hx0,hy0,hx1,hy1,hx,hy;
};

class CLocalTracker  
{
public:
	CLocalTracker();
	virtual ~CLocalTracker();

	int nid;
	int iWidth,iHeight;

	void DoLocalTracker(MOTIONCANDIDATE motion_candidate[MTG],CANDIDATE target[MTG][NTG],int **motion);
	HUMANID GetMotion(int k);

	HUMANID objectid_current[MTG];
	HUMANID objectid_previous[MTG];
	
protected:
	void IntializeTheID(int t);
	int FindClosedOne(int cx_p,int cy_p,MOTIONCANDIDATE motion_candidate[MTG]);
	void UpdateNewObjectID(MOTIONCANDIDATE motion_candidate[MTG]); 
};

#endif // !defined(AFX_LOCALTRACKER_H__5A583B1D_4A4B_47F4_932B_589ECBCA09D1__INCLUDED_)
