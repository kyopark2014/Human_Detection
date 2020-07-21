// MotionTracker.h: interface for the CMotionTracker class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOTIONTRACKER_H__394CBA49_B6F5_44AB_BE2E_FA03B542AB52__INCLUDED_)
#define AFX_MOTIONTRACKER_H__394CBA49_B6F5_44AB_BE2E_FA03B542AB52__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "detection.h"
#include "parameters.h"

struct MOTIONCANDIDATE
{
	int bx0,by0,bx1,by1;
	int px,py;
	int valid;
};

class CMotionTracker  
{
public:
	CMotionTracker();
	virtual ~CMotionTracker();

	int **motion;
	int **motion_object;
	int ***img_current;
	int ***img_back;
	int ***img_previous;
	int rate;

	double maxMotionWeight;

	int iWidth,iHeight;
	MOTIONCANDIDATE motion_candidate[MTG];

	void DoMotonTracker();
	void GaussDistribution();
	void VerifyCandidate(int k,double minMotionRate);
	
	void Initialize();
	void FinalizeMemory();
	void LoadCurrentColorImageFromFile(CString ImageFile);
	void ShowMotionBoxInfo(int num);
	void SaveMotionBoxInfo(int num);
	void GetMotionInfo();
	void InitProcessingMemory();
	void GetMotionObject();
	double CheckMotionWeight(int px,int py);
	void GetObjectInfo();
	bool IsolateObject(int num_obj,int px,int py);
	void DrawBox(int **pixels,int lcolor,int px0,int py0,int px1,int py1);
	void SaveToImageFile(int ***img_color,CString ImageFile);
};

#endif // !defined(AFX_MOTIONTRACKER_H__394CBA49_B6F5_44AB_BE2E_FA03B542AB52__INCLUDED_)
