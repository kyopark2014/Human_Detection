// Detection.h: interface for the Detection class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DETECTION_H__3304D808_8DD6_4D1E_9392_E231C6E956C5__INCLUDED_)
#define AFX_DETECTION_H__3304D808_8DD6_4D1E_9392_E231C6E956C5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "parameters.h"

struct CANDIDATE
{
	int bx0,by0,bx1,by1;
	int px,py;
	int valid;
};

struct LIBRARY
{
	RGBQUAD color;
	int dep;
	double At;
	bool enable;
};

class CDetection  
{
public:
	CDetection(int Width_s,int Height_s);
	virtual ~CDetection();

	void Initialize();
	void Finalize();

	int Width,Height;
	double MaxColorWeight;
	int pxr,pyr;

	double minMotion;
	int motionDip;
	double vThreshold;

	CANDIDATE candidate[NTG]; // The CANDIDATE based on color

	void DoDetection(int ***img_current,int **motion,int Width,int Height);
	void FindFace(int ***img_current,int **motion);
	void WeightDistribution();
	void Threshold();	
	void HumanDetection(int **motion);
	void IsolateObject(int num,int px,int py);

protected:
	double **gauss;
	void GaussDistribution();
	double gnoise(void);

	double GetCorrelation(int t[3],int r[3]);
	double CheckWeight(int px,int py);

	int **object;
	int **detect;
	double **weight;

};

#endif // !defined(AFX_DETECTION_H__3304D808_8DD6_4D1E_9392_E231C6E956C5__INCLUDED_)
