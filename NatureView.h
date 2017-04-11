
// NatureView.h : CNatureView ��Ľӿ�
//
#include "CudaComputing.cuh"

#pragma once


class CNatureView : public CView
{
protected: // �������л�����
	CNatureView();
	DECLARE_DYNCREATE(CNatureView)

// ����
public:
	CNatureDoc* GetDocument() const;

// ����
public:

// ��д
public:
	virtual void OnDraw(CDC* pDC);  // ��д�Ի��Ƹ���ͼ
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// ʵ��
public:
	virtual ~CNatureView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	CClientDC *m_pDC;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	virtual void OnInitialUpdate();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void drawAxis(void);
	bool SetupPixelFormat(void);
	void InitOpengl(void);
	void RenderScene(void);
	float Ex;
	float Ey;
	float Ez;


	float Cx;
	float Cy;
	float Cz;

	float ViewAngleX;
	float ViewAngleY;
	float ViewR;

	/*struct Node   //���ӽڵ�  
		{  
			int num;  
			Node* next;  
		};
	
		struct Grid   //�����ڵ�  
		{  
			Node* first;  
		};*/
	
	
	  afx_msg void OnTimer(UINT_PTR nIDEvent);
	  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	  bool LBIsDown;
      double m_SMouseX;
	  double m_SMouseY;

	 
	  

	  void drawBall(float x, float y, float z);
	  void InitialSPH(void);
	  afx_msg void OnStart();
	  
	  void drawOnePot(int index1, int index2, Vec* points, float* valueArray, Vec* normalArray);
	  void drawGuDian(int index, Vec* points, float* valueArray, Vec* normalArray);
	  int findTheForth(int a,int b,int c);
	  int findTheThird(int cIndex, int a, int b);
	  void drawGuXian(int index1, int index2, Vec* points, float* valueArray, Vec* normalArray);
	  void drawThreeInOne(int a, int b, int c, Vec* points, float* valueArray, Vec* normalArray);
	  int computGudian(int* nodes);
	  void drawTwoPots(int index1, int index2, int mianIndex, Vec* points, float* valueArray, Vec* normalArray);
	  BOOL IsFilled;
	  void MarchingCube(void);
	  void createMC(Vec* points);
};

#ifndef _DEBUG  // NatureView.cpp �еĵ��԰汾
inline CNatureDoc* CNatureView::GetDocument() const
   { return reinterpret_cast<CNatureDoc*>(m_pDocument); }
#endif

