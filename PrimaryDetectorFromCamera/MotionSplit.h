// MotionSplit.h: interface for the MotionSplit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOTIONSPLIT_H__D9B3351E_1F4C_4B4E_9E30_945C98DF119C__INCLUDED_)
#define AFX_MOTIONSPLIT_H__D9B3351E_1F4C_4B4E_9E30_945C98DF119C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class MotionSplit  
{
public:
	MotionSplit();
	virtual ~MotionSplit();

	int *bx0,*by0,*bx1,*by1,*cx,*hx0,*hy0,*hx1,*hy1;

	void DoMotionSplitter(int **motion,int mx0,int my0,int mx1,int my1,int n_motion,int min_head);
	int *GetMotions(int *motion_histo,int n_motion,int min_head,int iWidth);
	double *GetKernel(int *x, int N, int sigma);
	double *Convolution(int *motion_histo,double *weight,int N,int iWidth);

};

#endif // !defined(AFX_MOTIONSPLIT_H__D9B3351E_1F4C_4B4E_9E30_945C98DF119C__INCLUDED_)
