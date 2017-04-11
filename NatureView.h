
// NatureView.h : CNatureView 类的接口
//
#include "CudaComputing.cuh"

#pragma once


class CNatureView : public CView
{
protected: // 仅从序列化创建
	CNatureView();
	DECLARE_DYNCREATE(CNatureView)

// 特性
public:
	CNatureDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CNatureView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
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

	/*struct Node   //粒子节点  
		{  
			int num;  
			Node* next;  
		};
	
		struct Grid   //网格表节点  
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

#ifndef _DEBUG  // NatureView.cpp 中的调试版本
inline CNatureDoc* CNatureView::GetDocument() const
   { return reinterpret_cast<CNatureDoc*>(m_pDocument); }
#endif

