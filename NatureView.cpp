
// NatureView.cpp : CNatureView ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "Nature.h"
#endif

#include "NatureDoc.h"
#include "NatureView.h"
#include "CudaComputing.cuh"


//OpenGL������
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <gl/glaux.h>

//��ѧ������
#include <math.h>

#include <stdio.h>

#include "Dian.h"

#include "Line.h"

//����Cuda����ͷ�ļ�



#ifdef _DEBUG
#define new DEBUG_NEW
#endif
const int length=24;
const int width=14;
const int height=20;//���Լ�һ������λ�ñ���
const int inEvery=2;
const float singleBian=0.5;

//sph ����
const int Amount = 2000;
const int JiaoDianNum=(length*inEvery+1)*(width*inEvery+1)*(height*inEvery+1);
const int NormalNum=((length-1)*inEvery+1)*((width-1)*inEvery+1)*((height-1)*inEvery+1);
const float Pi=3.1415926f;

const int divZ=(length*inEvery+1)*(width*inEvery+1);
const int divY=(length*inEvery+1);

Vec Particles[Amount];
Vec Accelerations[Amount];
Vec Velocities[Amount];
float Densities[Amount];
float Pressures[Amount];
Vec PointNormal[JiaoDianNum];
float PointValue[JiaoDianNum];//�ǵ��valueֵ

//MC�в�����
int bian[12][2]=
{0,1,
1,2,
2,3,
3,0,
0,4,
1,5,
2,6,
3,7,
4,5,
5,6,
6,7,
7,4};

int mian[6][4]=
{0,1,2,3,
0,1,5,4,
1,2,6,5,
2,3,7,6,
0,3,7,4,
4,5,6,7};

int gudian[8][3]=
{1,3,4,
0,2,5,
1,3,6,
0,2,7,
0,5,7,
4,6,1,
2,5,7,
3,4,6};


// CNatureView

IMPLEMENT_DYNCREATE(CNatureView, CView)

BEGIN_MESSAGE_MAP(CNatureView, CView)
	// ��׼��ӡ����
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CNatureView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_KEYDOWN()
	ON_COMMAND(ID_START, &CNatureView::OnStart)
END_MESSAGE_MAP()

// CNatureView ����/����


//Voxel voxels[1500];


CNatureView::CNatureView()
	: Ex(0),
	  Ey(0),
	  Ez(0),
	  Cx(0),
	  Cy(0),
	  Cz(5)
	  , LBIsDown(false)
	  , m_SMouseX(0)
	  , m_SMouseY(0)
	  , IsFilled(0)
{
	// TODO: �ڴ˴���ӹ������
	ViewAngleX=3.1415926/4;
	ViewAngleY=3.1415926/4;
	ViewR=60;
	m_pDC=NULL;
	IsFilled=TRUE;

//	bian[12][2]={0,1,1,2,2,3,3,0,0,4,1,5,2,6,3,7,4,5,5,6,6,7,7,4};

	//mian[6][4]={0,1,2,3,0,1,5,4,1,2,6,5,2,3,7,6,0,3,7,4,4,5,6,7};

	//gudian[8][3]={1,3,4,0,2,5,1,3,6,0,2,7,0,5,7,4,6,1,2,5,7,3,4,6};
}

CNatureView::~CNatureView()
{
}

BOOL CNatureView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ
	cs.style |= (WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	return CView::PreCreateWindow(cs);
}

// CNatureView ����

void CNatureView::OnDraw(CDC* /*pDC*/)
{
	CNatureDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: �ڴ˴�Ϊ����������ӻ��ƴ���
	RenderScene();
}


// CNatureView ��ӡ


void CNatureView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CNatureView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// Ĭ��׼��
	return DoPreparePrinting(pInfo);
}

void CNatureView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӷ���Ĵ�ӡǰ���еĳ�ʼ������
}

void CNatureView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: ��Ӵ�ӡ����е��������
}

void CNatureView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CNatureView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CNatureView ���

#ifdef _DEBUG
void CNatureView::AssertValid() const
{
	CView::AssertValid();
}

void CNatureView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CNatureDoc* CNatureView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CNatureDoc)));
	return (CNatureDoc*)m_pDocument;
}
#endif //_DEBUG


// CNatureView ��Ϣ�������


void CNatureView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	GLdouble aspect_ratio; // width/height ratio

	int w=cx;
    int h=cy;
	aspect_ratio = (GLdouble)cx/(GLdouble)cy;

    GLfloat nRange=1.0f;
    //�������Ϊ0
    if(h==0)
       h=1;

    //�����ӿ��봰��ƥ��
    glViewport(0,0,w,h);

    //������������ϵͳ
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0f, aspect_ratio, .01f, 200.0f);//����ά

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glLoadIdentity();
}


int CNatureView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  �ڴ������ר�õĴ�������
	InitOpengl();
	InitialSPH();
	return 0;
}


void CNatureView::OnDestroy()
{
	CView::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������

	// TODO: �ڴ˴������Ϣ����������
	HGLRC hrc;
	hrc = ::wglGetCurrentContext();
	::wglMakeCurrent(NULL,NULL);
	if(hrc)
		::wglDeleteContext(hrc);
	if(m_pDC)
	delete m_pDC;
}


void CNatureView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	LBIsDown = true;

	m_SMouseX= point.x;
	m_SMouseY= point.y;

	CView::OnLButtonDown(nFlags, point);
}


void CNatureView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	LBIsDown = false;
	CView::OnLButtonUp(nFlags, point);
}


void CNatureView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if(LBIsDown ==true)
	{

		double	m_iMouseX = point.x;
		double  m_iMouseY = point.y;
		//double	m_iMouseY = point.y;

		// ViewX+=(m_iMouseX-m_SMouseX)/2000;

		ViewAngleX+=(m_iMouseX-m_SMouseX)/200;

		ViewAngleY-=(m_iMouseY-m_SMouseY)/200;
		//Yangle+=(m_iMouseY-m_SMouseY)/2000

		if(ViewAngleY>=3.1415926)ViewAngleY=3.1415926;

		//if(ViewAngleY>=3.1415926)ViewAngleY=3.1415926;

		if(ViewAngleY<=0.000000001)ViewAngleY=0.000000001;

		Invalidate(true);
	}
	m_SMouseX=point.x;
	m_SMouseY=point.y;

	CView::OnMouseMove(nFlags, point);
}


void CNatureView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: �ڴ����ר�ô����/����û���
	// TODO: �ڴ����ר�ô����/����û���
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//glEnable(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	

	glNewList(0, GL_COMPILE);
	glEnable(GL_COLOR_MATERIAL);
	GLfloat mat_ambient[]={0.1, 0.1, 0.1, 0.1};			//ԭ���ϵĻ�����ɫ
	GLfloat mat_diffuse[]={0.1, 0.1, 0.1, 0.2};			//ԭ���ϵ�ɢ����ɫ
	GLfloat mat_specular[]={0.6, 0.6, 0.6, 0.6};			//ԭ���ϵķ�����ɫ
	GLfloat mat_emission[]={0.3, 0.4, 0.4, 0.2};			//ԭ���ϵķ�ɢ��ɫ
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, 0.3);
	glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
	glEndList();


	glNewList(1, GL_COMPILE);
	glEnable(GL_LIGHTING);
	GLfloat light0_position[]={15.0, 15.0, 15.0, 0};
	GLfloat light0_spot_direction[]={-1.0, -1.0, -1.0, 0.0};
	GLfloat light0_ambient[]={0.2, 0.2, 0.2, 0.1};
	GLfloat ligh0_diffuse[]={0.2, 0.2, 0.2, 1.0};
	GLfloat light0_specular[]={0.5, 0.6, 0.6, 1.0};
	glLightfv(GL_LIGHT0, GL_POSITION, light0_position);
	//glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 45);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light0_spot_direction);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ligh0_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light0_specular);
	glEnable(GL_LIGHT0);
	glEndList();

	glNewList(2,GL_COMPILE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0,0.0,1.0,1.0f);
	glutSolidSphere(0.2,4,4);
	//glutSolidTeapot(0.2);
	//glutSolidTorus(1.8,2,10,10);
	glEndList();
}


BOOL CNatureView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	//return CView::OnEraseBkgnd(pDC);

	return TRUE;
}


void CNatureView::drawAxis(void)//������һ���Ĳ���
{
	glColor3f(1.0f,0.0f,0.0f);
	glLineWidth(1);
	glBegin(GL_LINES);

	glVertex3f(0,0,0);
	glVertex3f(100,0,0);

	glEnd();

	glColor3f(0.0f,1.0f,0.0f);
	glLineWidth(1);
	glBegin(GL_LINES);

	glVertex3f(0,0,0);
	glVertex3f(0,100,0);

	glEnd();


	glColor3f(0.0f,0.0f,1.0f);
	glLineWidth(1);
	glBegin(GL_LINES);

	glVertex3f(0,0,0);
	glVertex3f(0,0,100);


	glEnd();
}


bool CNatureView::SetupPixelFormat(void)
{
	static PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL|
		PFD_DOUBLEBUFFER, 
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		32,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
	int pixelformat;

	if((pixelformat = ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd)) == 0)
	{
		MessageBox(LPCTSTR("ChoosePixelFormat failed"));
		return FALSE;
	}

	if(SetPixelFormat(m_pDC->GetSafeHdc(), pixelformat, &pfd) == FALSE)
	{
		MessageBox(LPCTSTR("SetPixelFormat failed"));
		return FALSE;
	}

	return TRUE;;
}


void CNatureView::InitOpengl(void)
{
	PIXELFORMATDESCRIPTOR pfd;
	int n;
	HGLRC hrc;

	m_pDC=new CClientDC(this);

	ASSERT(m_pDC != NULL);

	if(!SetupPixelFormat())
		return;

	n=::GetPixelFormat(m_pDC->GetSafeHdc());

	::DescribePixelFormat(m_pDC->GetSafeHdc(), n,sizeof(pfd),&pfd);

	hrc=wglCreateContext(m_pDC->GetSafeHdc());
	wglMakeCurrent(m_pDC->GetSafeHdc(),hrc);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
}



void CNatureView::RenderScene(void)
{
	Ex=ViewR*sin(ViewAngleY)*sin(ViewAngleX)+Cx;
	Ey=ViewR*sin(ViewAngleY)*cos(ViewAngleX)+Cy;
	Ez=ViewR*cos(ViewAngleY)+Cz;
	//����������ɫΪ��ɫ

	
	//�����ɫ����������Ȼ�����
	//glClear(GL_DEPTH_BUFFER_BIT); 
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glShadeModel(GL_FLAT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0f,0.0f,0.0f,0.8f);
	glColor4f(0.0,0.55,1.0,1.0f);
	//�����ջ��������glPopMatrix()���Ӧ

	glLoadIdentity();

	gluLookAt(Ex,Ey,Ez,Cx,Cy,Cz,0,0,1);
	glCallList(0);
	glCallList(1);

	drawAxis();

	/*for(int i=0;i<Amount;i++){
		
		drawBall(100*Particles[i].x,100*Particles[i].y,100*Particles[i].z);
	}*/

	for(int i=0;i<Amount;i++){
		Particles[i].x=Particles[i].x*100;
		Particles[i].y=Particles[i].y*100;
		Particles[i].z=Particles[i].z*100;
	}
	//�ѵ��ǳ�ʼ����λ�ã���
	computeAllValue(Particles,PointValue);

	computeAllNormal(PointNormal,PointValue);

	for(int i=0;i<Amount;i++){
		Particles[i].x=Particles[i].x/100;
		Particles[i].y=Particles[i].y/100;
		Particles[i].z=Particles[i].z/100;
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(0.0,0.55,1.0,0.6f);
	
	MarchingCube();
	/****************************************/

	//����һ���˸�Ԫ�ص����飬��������ڽӱ�
//	Grid grids[8];
	

	
	//����GPU����CUDA����
	//computeTest(grids,8,test,100);

	//cudaAdd();

	//�ؼ���ֱ����û����

	//������һ���Ƿ�ɹ�
	/*for(int i=0;i<8;i++)
	{
		printf("��%d��\n",i);
		
		while(grids[i].first!=NULL)
		{
			//printf("   ����%d��\n",grids[i].first->num);  
			//grids[i].first=grids[i].first->next;
		}
		

	}*/


	/****************************************/

	//void MultDot(float *a,float *b,float *partial_c){

	SwapBuffers(wglGetCurrentDC());//����ʡ��
}



void CNatureView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	
	for(int i=0;i<Amount;i++){
		Particles[i].x=Particles[i].x*100;
		Particles[i].y=Particles[i].y*100;
		Particles[i].z=Particles[i].z*100;
	}
	//�ѵ��ǳ�ʼ����λ�ã���
	computeAllValue(Particles,PointValue);

	computeAllNormal(PointNormal,PointValue);

	for(int i=0;i<Amount;i++){
		Particles[i].x=Particles[i].x/100;
		Particles[i].y=Particles[i].y/100;
		Particles[i].z=Particles[i].z/100;
	}

	get_table(Particles,Velocities,Accelerations,Densities,Pressures);

	//���ɵ�����,�ŵ���ʼ���ĵط�

   Invalidate();

	

	CView::OnTimer(nIDEvent);
}


void CNatureView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	if(nChar==40){
		ViewR+=0.2;
		Invalidate();
	}
	if(nChar==38){
		ViewR-=0.2;
		if(ViewR<=2)ViewR=2;
		Invalidate();
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}


void CNatureView::drawBall(float x, float y, float z)
{
	glPushMatrix();
	glTranslatef(x,y,z);
	glCallList(2);

	glPopMatrix();
	glFlush();
}


void CNatureView::InitialSPH(void)
{
	for(int i=0;i<Amount;i++){
		int a=i/100;
		int b=(i-a*100)/10;
		int c=i-100*a-10*b;

		Particles[i].x=(float)c/100;//�����ڷ�Χ��
		Particles[i].y=(float)b/100;
		Particles[i].z=(float)a/100;
		//drawBall(Particles[i].x,Particles[i].y,Particles[i].z);

		Accelerations[i].x=0;
		Accelerations[i].y=0;
		Accelerations[i].z=0;

		Velocities[i].x=0;
		Velocities[i].y=0;
		Velocities[i].z=0;

		Densities[i]=0;
		Pressures[i]=0;

		//PointValue[i]=0;
	}
}

void CNatureView::OnStart()
{
	// TODO: �ڴ���������������
	InitialSPH();
	SetTimer(1,10,NULL);
}

void CNatureView::drawOnePot(int index1, int index2, Vec* points, float* valueArray, Vec* normalArray)
{
	glNormal3f((normalArray[index1].x*valueArray[index2]/(valueArray[index1]+valueArray[index2])+normalArray[index2].x*valueArray[index1]/(valueArray[index1]+valueArray[index2])),(normalArray[index1].y*valueArray[index2]/(valueArray[index1]+valueArray[index2])+normalArray[index2].y*valueArray[index1]/(valueArray[index1]+valueArray[index2])),(normalArray[index1].z*valueArray[index2]/(valueArray[index1]+valueArray[index2])+normalArray[index2].z*valueArray[index1]/(valueArray[index1]+valueArray[index2])));

	glVertex3f((points[index1].x*valueArray[index2]/(valueArray[index1]+valueArray[index2])+points[index2].x*valueArray[index1]/(valueArray[index1]+valueArray[index2])),(points[index1].y*valueArray[index2]/(valueArray[index1]+valueArray[index2])+points[index2].y*valueArray[index1]/(valueArray[index1]+valueArray[index2])),(points[index1].z*valueArray[index2]/(valueArray[index1]+valueArray[index2])+points[index2].z*valueArray[index1]/(valueArray[index1]+valueArray[index2])));

}

void CNatureView::drawGuDian(int index, Vec* points, float* valueArray, Vec* normalArray)
{
	if(IsFilled){
		glBegin(GL_TRIANGLE_STRIP);
	}
	if(!IsFilled){
		//glLineWidth(20);

		glBegin(GL_LINE_LOOP);

	}
	//glColor3f(0.7f,1.0f,0.7f);
	for(int i=0;i<3;i++){
		int tempIndex=gudian[index][i];
		//glVertex3f((points[index].x+points[tempIndex].x)/2,(points[index].y+points[tempIndex].y)/2,(points[index].z+points[tempIndex].z)/2);
		drawOnePot(index,tempIndex,points,valueArray,normalArray);
	}
	glEnd();
}

int CNatureView::findTheForth(int a,int b,int c)
{
	int theMian=-1;
	int theIndex=-1;
	for(int i=0;i<6;i++){
		int count=0;
		for(int j=0;j<4;j++){
			if(mian[i][j]==a||mian[i][j]==b||mian[i][j]==c){
				count++;
			}
		}
		if(count==3){
			theMian=i;
			break;
		}
	}
	if(theMian!=-1){
		for(int k=0;k<4;k++){
			if(mian[theMian][k]!=a&&mian[theMian][k]!=b&&mian[theMian][k]!=c){
				theIndex=mian[theMian][k];
			}
		}
	}
	return(theIndex);
}

int CNatureView::findTheThird(int cIndex, int a, int b)
{
	int theThird=-1;
	for(int i=0;i<3;i++){
		int tempIndex=gudian[cIndex][i];
		if(tempIndex!=a&&tempIndex!=b){
			theThird = tempIndex;
			break;
		}
	}
	//printf("��������Ϊ%d\n",theThird);
	return(theThird);
}

void CNatureView::drawGuXian(int index1, int index2, Vec* points, float* valueArray, Vec* normalArray)
{
	int nearIndex1=-1;
	int nearIndex2=-1; 
	//��ƺ�˳��
	for(int i=0;i<3;i++){
		int tempIndex=gudian[index1][i];
		if(tempIndex!=index2){
			if(nearIndex1==-1&&nearIndex2==-1){
				nearIndex1=tempIndex;
			}
			else if(nearIndex1!=-1&&nearIndex2==-1){
				nearIndex2=tempIndex;	
			}
			//glVertex3f((points[index1].x+points[tempIndex].x)/2,(points[index1].y+points[tempIndex].y)/2,(points[index1].z+points[tempIndex].z)/2);
			//drawOnePot(index1,tempIndex,points,valueArray,normalArray);
		}
	}
	int nearIndex3=findTheForth(index1,index2,nearIndex1);
	int nearIndex4=findTheForth(index1,index2,nearIndex2);

	if(IsFilled){
		glBegin(GL_TRIANGLE_STRIP);
		drawOnePot(index1,nearIndex1,points,valueArray,normalArray);
		drawOnePot(index1,nearIndex2,points,valueArray,normalArray);
		//�ҵ���index1,index2,nearIndex1�����
		//glVertex3f((points[index2].x+points[nearIndex3].x)/2,(points[index2].y+points[nearIndex3].y)/2,(points[index2].z+points[nearIndex3].z)/2);
		drawOnePot(index2,nearIndex3,points,valueArray,normalArray);
		//�ҵ���index1,index2,nearIndex2�����
		//glVertex3f((points[index2].x+points[nearIndex4].x)/2,(points[index2].y+points[nearIndex4].y)/2,(points[index2].z+points[nearIndex4].z)/2);
		drawOnePot(index2,nearIndex4,points,valueArray,normalArray);
		glEnd();
	}

	if(!IsFilled){
		glBegin(GL_LINE_LOOP);
		drawOnePot(index1,nearIndex1,points,valueArray,normalArray);
		drawOnePot(index1,nearIndex2,points,valueArray,normalArray);
		drawOnePot(index2,nearIndex3,points,valueArray,normalArray);
		glEnd();

		glBegin(GL_LINE_LOOP);
		drawOnePot(index1,nearIndex2,points,valueArray,normalArray);
		//�ҵ���index1,index2,nearIndex1�����
		//glVertex3f((points[index2].x+points[nearIndex3].x)/2,(points[index2].y+points[nearIndex3].y)/2,(points[index2].z+points[nearIndex3].z)/2);
		drawOnePot(index2,nearIndex3,points,valueArray,normalArray);
		//�ҵ���index1,index2,nearIndex2�����
		//glVertex3f((points[index2].x+points[nearIndex4].x)/2,(points[index2].y+points[nearIndex4].y)/2,(points[index2].z+points[nearIndex4].z)/2);
		drawOnePot(index2,nearIndex4,points,valueArray,normalArray);
		glEnd();
	}
}

void CNatureView::drawThreeInOne(int a, int b, int c, Vec* points, float* valueArray, Vec* normalArray)
{
	 int nodes[3];
		  nodes[0]=a;
		  nodes[1]=b;
		  nodes[2]=c;
	      int index1=-1;
		  int index2=-1;
		  int index3=-1;
		  int nearIndex1;
		  for(int i=0;i<3;i++){
			   int tempIndex=nodes[i];
			   int count=0;
			   for(int j=0;j<3;j++){
					if(gudian[tempIndex][j]==nodes[0]||gudian[tempIndex][j]==nodes[1]||gudian[tempIndex][j]==nodes[2]){
						 count++;
					}
			  }
			  if(count==2){
				 index1=tempIndex;
			     for(int k=0;k<3;k++){
					if(index2==-1&&index3==-1&&nodes[k]!=index1){
					index2=nodes[k];
				    }
				    else if(index2!=-1&&index3==-1&&nodes[k]!=index1){
					index3=nodes[k];
					//printf("3 is %d\n",index3);
				    }
				   
				   if(gudian[index1][k]!=nodes[0]&&gudian[index1][k]!=nodes[1]&&gudian[index1][k]!=nodes[2]){
				      nearIndex1=gudian[index1][k]; 
					  //printf("first is %d\n",nearIndex1);
					  //glVertex3f((points[index1].x+points[nearIndex1].x)/2,(points[index1].y+points[nearIndex1].y)/2,(points[index1].z+points[nearIndex1].z)/2);
					  
				   }
				 }
				 break;
			  }
		  }
		  int nearIndex2=findTheForth(index1,index2,nearIndex1);

		  int nearIndex3=findTheForth(index1,index3,nearIndex1);

		  int nearIndex4=findTheForth(index1,index2,index3);
		  if(IsFilled){
		   glBegin(GL_TRIANGLE_STRIP);		 

		  drawOnePot(index1,nearIndex1,points,valueArray,normalArray);
		  //glVertex3f((points[index2].x+points[nearIndex2].x)/2,(points[index2].y+points[nearIndex2].y)/2,(points[index2].z+points[nearIndex2].z)/2);
		  drawOnePot(index2,nearIndex2,points,valueArray,normalArray);

		  //glVertex3f((points[index3].x+points[nearIndex3].x)/2,(points[index3].y+points[nearIndex3].y)/2,(points[index3].z+points[nearIndex3].z)/2);
		  drawOnePot(index3,nearIndex3,points,valueArray,normalArray);

		  //glVertex3f((points[index2].x+points[nearIndex4].x)/2,(points[index2].y+points[nearIndex4].y)/2,(points[index2].z+points[nearIndex4].z)/2);
		  drawOnePot(index2,nearIndex4,points,valueArray,normalArray);
		  
		  //glVertex3f((points[index3].x+points[nearIndex4].x)/2,(points[index3].y+points[nearIndex4].y)/2,(points[index3].z+points[nearIndex4].z)/2);
		  drawOnePot(index3,nearIndex4,points,valueArray,normalArray);
		  glEnd();
		  
		  }

		  if(!IsFilled){
		   glBegin(GL_LINE_LOOP);
		    drawOnePot(index1,nearIndex1,points,valueArray,normalArray);
		    drawOnePot(index2,nearIndex2,points,valueArray,normalArray);
		    drawOnePot(index3,nearIndex3,points,valueArray,normalArray);
		   glEnd();
		   glBegin(GL_LINE_LOOP);
		    drawOnePot(index2,nearIndex2,points,valueArray,normalArray);
		    drawOnePot(index3,nearIndex3,points,valueArray,normalArray);
		    drawOnePot(index2,nearIndex4,points,valueArray,normalArray);
		   glEnd();
		   glBegin(GL_LINE_LOOP);
		    drawOnePot(index3,nearIndex3,points,valueArray,normalArray);
		   drawOnePot(index2,nearIndex4,points,valueArray,normalArray);
		   drawOnePot(index3,nearIndex4,points,valueArray,normalArray);

		   glEnd();

		  /*drawOnePot(index1,nearIndex1,points,valueArray,normalArray);
		  drawOnePot(index2,nearIndex2,points,valueArray,normalArray);
		  drawOnePot(index3,nearIndex3,points,valueArray,normalArray);
		  drawOnePot(index2,nearIndex4,points,valueArray,normalArray);
		  drawOnePot(index3,nearIndex4,points,valueArray,normalArray);

		  glEnd();*/
		  
		  }
}

int CNatureView::computGudian(int* nodes)
{
	int gudianNumber=0;
	for(int i=0;i<4;i++){
		int tempIndex=nodes[i];
		int count=0;
		for(int j=0;j<3;j++){
			if(gudian[tempIndex][j]!=nodes[0]&&gudian[tempIndex][j]!=nodes[1]&&gudian[tempIndex][j]!=nodes[2]&&gudian[tempIndex][j]!=nodes[3]){
				count++;
			}
		}
		if(count==3){
			gudianNumber++;
		}
	}
	return(gudianNumber);
}

void CNatureView::drawTwoPots(int index1, int index2, int mianIndex, Vec* points, float* valueArray, Vec* normalArray)
{
	//ͨ��mianIndex���ҵ�������������ĵ�
	int index3,index4,index5,index6;
	/*for(int i=0;i<8;i++){
		normalArray[i].x=-normalArray[i].x;
		normalArray[i].y=-normalArray[i].y;
		normalArray[i].z=-normalArray[i].z;
	}*/
	for(int i=0;i<4;i++){
	  int tempIndex=mian[mianIndex][i];
	  if(tempIndex!=index1&&tempIndex!=index2){
      index3=tempIndex;
	  break;
	  }
	}
	index4=findTheForth(index1,index2,index3);

	index5=findTheThird(index1,index3,index4);
	index6=findTheThird(index2,index3,index4);
	
	//���ģ��
	if(IsFilled){
	glBegin(GL_TRIANGLES);
	//glBegin(GL_TRIANGLE_STRIP);

	/*drawOnePot(index1,index3,points,valueArray,normalArray);
	drawOnePot(index2,index3,points,valueArray,normalArray);
	drawOnePot(index1,index5,points,valueArray,normalArray);
	drawOnePot(index2,index6,points,valueArray,normalArray);
    drawOnePot(index2,index4,points,valueArray,normalArray);
    drawOnePot(index1,index4,points,valueArray,normalArray);*/

	/*glNormal3f(normalArray[index3].x,normalArray[index3].y,normalArray[index3].z);
	glVertex3f(points[index3].x,points[index3].y,points[index3].z);
	drawOnePot(index1,index3,points,valueArray,normalArray);
	drawOnePot(index2,index3,points,valueArray,normalArray);

	glNormal3f(normalArray[index4].x,normalArray[index4].y,normalArray[index4].z);
	glVertex3f(points[index4].x,points[index4].y,points[index4].z);
	drawOnePot(index1,index4,points,valueArray,normalArray);
	drawOnePot(index2,index4,points,valueArray,normalArray);
	*/

	drawOnePot(index1,index5,points,valueArray,normalArray);
	drawOnePot(index1,index3,points,valueArray,normalArray);
	drawOnePot(index3,index2,points,valueArray,normalArray);

    drawOnePot(index1,index5,points,valueArray,normalArray);
	drawOnePot(index2,index3,points,valueArray,normalArray);
    drawOnePot(index2,index6,points,valueArray,normalArray);
	

	drawOnePot(index1,index5,points,valueArray,normalArray);
    drawOnePot(index1,index4,points,valueArray,normalArray);
	drawOnePot(index2,index4,points,valueArray,normalArray);

	drawOnePot(index2,index6,points,valueArray,normalArray);
    drawOnePot(index2,index4,points,valueArray,normalArray);
	drawOnePot(index1,index5,points,valueArray,normalArray);

	glEnd();
	
	}

	if(!IsFilled){
	glBegin(GL_LINE_LOOP);
	drawOnePot(index1,index5,points,valueArray,normalArray);
	drawOnePot(index1,index3,points,valueArray,normalArray);
	drawOnePot(index3,index2,points,valueArray,normalArray);
	glEnd();


	glBegin(GL_LINE_LOOP);
    drawOnePot(index1,index5,points,valueArray,normalArray);
	drawOnePot(index2,index3,points,valueArray,normalArray);
    drawOnePot(index2,index6,points,valueArray,normalArray);
	glEnd();

	
	glBegin(GL_LINE_LOOP);
	drawOnePot(index1,index5,points,valueArray,normalArray);
    drawOnePot(index1,index4,points,valueArray,normalArray);
	drawOnePot(index2,index4,points,valueArray,normalArray);
	glEnd();

	glBegin(GL_LINE_LOOP);
	drawOnePot(index2,index6,points,valueArray,normalArray);
    drawOnePot(index2,index4,points,valueArray,normalArray);
	drawOnePot(index1,index5,points,valueArray,normalArray);
	glEnd();
	
	}
}

void CNatureView::createMC(Vec* points)
{
	float singleBian=0.5;
	char signArray[9];
	//char signArray[9];
	float valueArray[8];//Ҫ���********************************
	float realValue[8];
	Vec normalArray[8];//���Ҳ��ǰ���
	//������һ�ζ�Ҫ��д
	//��ͨ��point��λ�õõ�value������ֵ
	for(int i=0;i<8;i++){
	  int a=(points[i].x)/singleBian;//��λ���й�
	  int b=(points[i].y)/singleBian;
	  int c=(points[i].z)/singleBian;
	  int tempIndex=divY*b+divZ*c+a;
	  //float tempValue=computeValue(points[i]);
	  float tempValue=PointValue[tempIndex];
	  //printf("index is %d\nvalue is %f\n", tempIndex,tempValue);
	  valueArray[i]=abs(tempValue);//�����ʱ���ǲ��ǿ��Ը�һ��
	  realValue[i]=tempValue;
	  Vec tempNormal=PointNormal[tempIndex];
	  normalArray[i]=tempNormal;
	 if(tempValue>=0){
	  signArray[i]='0';
	  }
	  if(tempValue<0){
	  signArray[i]='1';
	  }
	}
	
	//����Ҫ���ݵ�ֵ������״̬��
	//�õ������Ľ������һ������,�õ�������
	int nodeNumber;
    int numberof1=0;
	int numberof0=0;

	int *nodes;
	int nodesfor1[8];
	int nodesfor0[8];

	int bianTaken[12];
	int mianTaken[6];
	int bianTakenNumber=0;
	int mianTakenNumber=0;

	memset(bianTaken,0,12*sizeof(int));
	memset(mianTaken,0,6*sizeof(int));

	
	
	for(int i=0;i<8;i++){
	 if(signArray[i]=='0'){
		 nodesfor0[numberof0]=i;
		 numberof0++;
	 }
	 if(signArray[i]=='1'){
		 nodesfor1[numberof1]=i;
		 numberof1++;
	 }
	}
	if(numberof0>=numberof1){
		nodeNumber=numberof1;
		nodes=nodesfor1;
	}
	if(numberof0<numberof1){
		nodeNumber=numberof0;
		nodes=nodesfor0;
	}
    //ɨ��˱��������ߡ����桢�µ�����������������
	for(int i=0;i<nodeNumber;i++){
        if(nodes[i]==0){
			bianTaken[0]++;
			bianTaken[3]++;
			bianTaken[4]++;
			mianTaken[0]++;
			mianTaken[1]++;
			mianTaken[4]++;
		}
		if(nodes[i]==1){
			bianTaken[0]++;
			bianTaken[1]++;
			bianTaken[5]++;
			mianTaken[0]++;
			mianTaken[1]++;
			mianTaken[2]++;
		}
		if(nodes[i]==2){
			bianTaken[1]++;
			bianTaken[2]++;
			bianTaken[6]++;
			mianTaken[0]++;
			mianTaken[2]++;
			mianTaken[3]++;
		}
		if(nodes[i]==3){
			bianTaken[2]++;
			bianTaken[3]++;
			bianTaken[7]++;
			mianTaken[0]++;
			mianTaken[3]++;
			mianTaken[4]++;
		}
		if(nodes[i]==4){
			bianTaken[4]++;
			bianTaken[8]++;
			bianTaken[11]++;
			mianTaken[1]++;
			mianTaken[4]++;
			mianTaken[5]++;
		}
		if(nodes[i]==5){
			bianTaken[5]++;
			bianTaken[8]++;
			bianTaken[9]++;
			mianTaken[1]++;
			mianTaken[2]++;
			mianTaken[5]++;
		}
		if(nodes[i]==6){
			bianTaken[6]++;
			bianTaken[9]++;
			bianTaken[10]++;
			mianTaken[2]++;
			mianTaken[3]++;
			mianTaken[5]++;
		}
		if(nodes[i]==7){
			bianTaken[7]++;
			bianTaken[10]++;
			bianTaken[11]++;
			mianTaken[3]++;
			mianTaken[4]++;
			mianTaken[5]++;
		}
    }

	for(int i=0;i<6;i++){
	   if(mianTaken[i]==4){
	    mianTakenNumber++;
	   }
	}
	for(int i=0;i<12;i++){
	   if(bianTaken[i]==2){
	    bianTakenNumber++;
	   }
	}
	//����״̬�����������������õ�����Ƭ
	if(nodeNumber==0){
       return;//���0
	}
	if(nodeNumber==1){
		//���1
       //�ӱ��в��ҵ��������������������������㣬�Ӷ��õ�һ������Ƭ
		int index=nodes[0];
		//printf("%d\n",index);
		drawGuDian(index,points,valueArray,normalArray);
	}
	if(nodeNumber==2){
		//���2
		int index1=nodes[0];
		int index2=nodes[1];
		if(bianTakenNumber==1){
		//printf("�������˰�");
		//������߶�Ӧ�ĸ�����ɵ�����Ƭ
		//�������������������ߣ�����������ƽ�е����飬��һ��ȡһ�㣬�ڶ���ƽ��λ��ȡһ��
		 
		 drawGuXian(index1,index2,points,valueArray,normalArray);
		}

		//���3��4
		if(bianTakenNumber==0){
			for(int j=0;j<6;j++){
			  if(j==5&&mianTaken[5]!=2){
			    for(int i=0;i<nodeNumber;i++){
			    int index=nodes[i];
			    //drawGuDian(index,points,valueArray,normalArray);
			    }
				break;
			  }
			  if(mianTaken[j]==2){
			//������j����������Ҫ��mianIndex;
			  float a0=valueArray[0];
			  float a1=valueArray[1]-valueArray[0];
			  float a2=valueArray[4]-valueArray[0];
			  float a3=valueArray[3]-valueArray[0];
			  float a4=valueArray[0]-valueArray[1]+valueArray[5]-valueArray[4];
			  float a5=valueArray[0]-valueArray[3]+valueArray[7]-valueArray[4];
			  float a6=valueArray[0]-valueArray[1]+valueArray[2]-valueArray[3];
			  float a7=valueArray[1]-valueArray[0]+valueArray[3]-valueArray[2]+valueArray[4]-valueArray[5]+valueArray[6]-valueArray[7];
			  float b0,b1,b2,b3;
			  float x0,y0,z0;
			  float cx,cy,cz;
			  float value;
			 // int index1,index2;

			    if(j==0){
					y0=points[0].y;
					b0=a0+a2*y0;
					b1=a1+a4*y0;
					b2=a3+a5*y0;
					b3=a6+a7*y0;
					cx=-b2/b3;
					cz=-b1/b3;
					value=b0+b1*cx+b2*cz+b3*cx*cz-500;
					
					//�ȽϽ����߽���ֵ������ֵ�������԰��Ż�ͼ
				}

				if(j==1){
					z0=points[0].z;
					b0=a0+a3*z0;
					b1=a1+a6*z0;
					b2=a2+a5*z0;
					b3=a4+a7*z0;
					cx=-b2/b3;
					cy=-b1/b3;
					value=b0+b1*cx+b2*cy+b3*cx*cy-500;
				}
				if(j==2){
					x0=points[1].x;
					b0=a0+a1*x0;
					b1=a2+a4*x0;
					b2=a3+a5*x0;
					b3=a5+a7*x0;
					cy=-b2/b3;
					cz=-b1/b3;
					value=b0+b1*cy+b2*cz+b3*cy*cz-500;
				}
				if(j==3){
					z0=points[2].z;
					b0=a0+a3*z0;
					b1=a1+a6*z0;
					b2=a2+a5*z0;
					b3=a4+a7*z0;
					cx=-b2/b3;
					cy=-b1/b3;
					value=b0+b1*cx+b2*cy+b3*cx*cy-500;
				}
				if(j==4){
					x0=points[0].x;
					//x0=points[1].x;
					b0=a0+a1*x0;
					b1=a2+a4*x0;
					b2=a3+a5*x0;
					b3=a5+a7*x0;
					cy=-b2/b3;
					cz=-b1/b3;
					value=b0+b1*cy+b2*cz+b3*cy*cz-500;
				}
				if(j==5){
					y0=points[4].y;
					b0=a0+a2*y0;
					b1=a1+a4*y0;
					b2=a3+a5*y0;
					b3=a6+a7*y0;
					cx=-b2/b3;
					cz=-b1/b3;
					value=b0+b1*cx+b2*cz+b3*cx*cz-500;
				}
				
				   // index1=nodes[0];
					//index2=nodes[1];

					if(value>=0&&realValue[index1]>=0&&realValue[index2]>=0){
						drawTwoPots(index1,index2,j,points,valueArray,normalArray);
						//drawGuDian(index1,points,valueArray,normalArray);
					    //drawGuDian(index2,points,valueArray,normalArray);
					}

					if(value>=0&&realValue[index1]<0&&realValue[index2]<0){
			            //drawGuDian(index1,points,valueArray,normalArray);
					   // drawGuDian(index2,points,valueArray,normalArray);
						drawTwoPots(index1,index2,j,points,valueArray,normalArray);

						//printf("�п�����\n");
					}

					if(value<0&&realValue[index1]<0&&realValue[index2]<0){
						
						//drawTwoPots(index1,index2,j,points,valueArray,normalArray);
						drawGuDian(index1,points,valueArray,normalArray);
					    drawGuDian(index2,points,valueArray,normalArray);

					}


					if(value<0&&realValue[index1]>=0&&realValue[index2]>=0){
					    //drawGuDian(index1,points,valueArray,normalArray);
					    //drawGuDian(index2,points,valueArray,normalArray);
						drawTwoPots(index1,index2,j,points,valueArray,normalArray);
					}
					break;
			  }
			}

		//�˴��ж����ԣ����кü�����������治����
		//�����ж��������ڵ�ƽ������������a0��a7�Ĳ�ͬ
		//�ɰ˸��ǵ�ĺ���ֵ�Եõ�˫���ߵĽ����߽���
		//�ܹ������߽��㺯��ֵ���ж�����һ������Ի�ͼ




		//�µ�ͺû���
			
		}
	}
	if(nodeNumber==3){
		//���5
		if(bianTakenNumber==2){
          drawThreeInOne(nodes[0],nodes[1],nodes[2],points,valueArray,normalArray);
		}
		//���6
		if(bianTakenNumber==1){
			//�ҵ��µ�͹±ߣ��ֱ�
		   int index1=nodes[0];
		   int index2=nodes[1];
		   int index3=nodes[2];
		   for(int i=0;i<12;i++){
			   int count=0;
			   for(int j=0;j<2;j++){
                  if(bian[i][j]==index1||bian[i][j]==index2||bian[i][j]==index3){
				  count++;
				  }
			   }
			   if(count==2){
				   drawGuXian(bian[i][0],bian[i][1],points,valueArray,normalArray);
                   for(int k=0;k<3;k++){
		              if(nodes[k]!=bian[i][0]&&nodes[k]!=bian[i][1]){
					  drawGuDian(nodes[k],points,valueArray,normalArray);
					  drawGuDian(index3,points,valueArray,normalArray);
					  }
		           }
				   break;
			   }
		   }
		}
		//���7
		if(bianTakenNumber==0){
		  for(int i=0;i<nodeNumber;i++){
			 int index=nodes[i];
			 drawGuDian(index,points,valueArray,normalArray);
		  }
		}
	}
	if(nodeNumber==4){
		if(bianTakenNumber==4){
		//���ȴ��ĸ������ҵ�һ��Խ��ߣ��ҵ�����Խ��ߵ���ڵ�
	    int index1=nodes[0];
		int index2;
		int index3;
		int index4;
		int nearIndex1;
		int nearIndex2;
		int nearIndex3;
		int nearIndex4;
		for(int i=1;i<4;i++){
		     if(nodes[i]!=gudian[index1][0]&&nodes[i]!=gudian[index1][1]&&nodes[i]!=gudian[index1][2]){
			   index3=nodes[i];
			   break;
			 }
		}
		for(int i=1;i<4;i++){
		    if(nodes[i]!=index1&&nodes[i]!=index3){
            index2=nodes[i];
			break;
 			}
		}
		index4=findTheForth(index1,index2,index3);
		for(int i=0;i<3;i++){
            if(gudian[index1][i]!=index2&&gudian[index1][i]!=index4){
			nearIndex1=gudian[index1][i];
			break;
			}
		}
		nearIndex2=findTheForth(index1,index2,nearIndex1);

		nearIndex3=findTheForth(index2,index3,nearIndex2);

		nearIndex4=findTheForth(index1,index4,nearIndex1);

		if(IsFilled){
		glBegin(GL_TRIANGLE_STRIP);

		//glVertex3f((points[index1].x+points[nearIndex1].x)/2,(points[index1].y+points[nearIndex1].y)/2,(points[index1].z+points[nearIndex1].z)/2);
		drawOnePot(index1,nearIndex1,points,valueArray,normalArray);

		//glVertex3f((points[index2].x+points[nearIndex2].x)/2,(points[index2].y+points[nearIndex2].y)/2,(points[index2].z+points[nearIndex2].z)/2);
		drawOnePot(index2,nearIndex2,points,valueArray,normalArray);

		//glVertex3f((points[index4].x+points[nearIndex4].x)/2,(points[index4].y+points[nearIndex4].y)/2,(points[index4].z+points[nearIndex4].z)/2);
		drawOnePot(index4,nearIndex4,points,valueArray,normalArray);

		//glVertex3f((points[index3].x+points[nearIndex3].x)/2,(points[index3].y+points[nearIndex3].y)/2,(points[index3].z+points[nearIndex3].z)/2);
		drawOnePot(index3,nearIndex3,points,valueArray,normalArray);

		glEnd();
		
		}

		if(!IsFilled){
		glBegin(GL_LINE_LOOP);

		drawOnePot(index1,nearIndex1,points,valueArray,normalArray);

		drawOnePot(index2,nearIndex2,points,valueArray,normalArray);

		drawOnePot(index4,nearIndex4,points,valueArray,normalArray);
        
		glEnd();

		glBegin(GL_LINE_LOOP);

		drawOnePot(index2,nearIndex2,points,valueArray,normalArray);

		drawOnePot(index4,nearIndex4,points,valueArray,normalArray);

		drawOnePot(index3,nearIndex3,points,valueArray,normalArray);

		glEnd();
		
		}
		

	}
		//�������������
		if(bianTakenNumber==3){
			bool HaveFull=false;
			int index;
			for(int i=0;i<4;i++){
			  int tempIndex=nodes[i];
			  int count=0;
			  for(int j=0;j<3;j++){
				  if(gudian[tempIndex][j]==nodes[0]||gudian[tempIndex][j]==nodes[1]||gudian[tempIndex][j]==nodes[2]||gudian[tempIndex][j]==nodes[3]){
				   count++;
				  }
			  }
			  if(count==3){
			   HaveFull=true;
			   index=tempIndex;
			   break;
			  }
			}
			if(HaveFull){
			   //printf("center is %d\n",index);
			   int tempArray[4];
			   tempArray[0]=index;
			   int newIndex=1;
			   for(int i=0;i<4;i++){
			     if(nodes[i]!=index){
				 tempArray[newIndex]=nodes[i];
				 newIndex++;
				 }
			   }
			   int nearIndex1=findTheForth(tempArray[0],tempArray[1],tempArray[2]);
			   int nearIndex2=findTheForth(tempArray[0],tempArray[1],tempArray[3]);
			   int nearIndex3=findTheForth(tempArray[0],tempArray[2],tempArray[3]);
			   if(IsFilled){
			   glBegin(GL_TRIANGLE_STRIP);
	 	       //glVertex3f((points[tempArray[1]].x+points[nearIndex1].x)/2,(points[tempArray[1]].y+points[nearIndex1].y)/2,(points[tempArray[1]].z+points[nearIndex1].z)/2);
			   drawOnePot(tempArray[1],nearIndex1,points,valueArray,normalArray);
			   //glVertex3f((points[tempArray[2]].x+points[nearIndex1].x)/2,(points[tempArray[2]].y+points[nearIndex1].y)/2,(points[tempArray[2]].z+points[nearIndex1].z)/2);
			   drawOnePot(tempArray[2],nearIndex1,points,valueArray,normalArray);
			  // glVertex3f((points[tempArray[1]].x+points[nearIndex2].x)/2,(points[tempArray[1]].y+points[nearIndex2].y)/2,(points[tempArray[1]].z+points[nearIndex2].z)/2);
			   drawOnePot(tempArray[1],nearIndex2,points,valueArray,normalArray);
			   
			  // glVertex3f((points[tempArray[2]].x+points[nearIndex3].x)/2,(points[tempArray[2]].y+points[nearIndex3].y)/2,(points[tempArray[2]].z+points[nearIndex3].z)/2);
			   drawOnePot(tempArray[2],nearIndex3,points,valueArray,normalArray);
			   //glVertex3f((points[tempArray[3]].x+points[nearIndex2].x)/2,(points[tempArray[3]].y+points[nearIndex2].y)/2,(points[tempArray[3]].z+points[nearIndex2].z)/2);
			   drawOnePot(tempArray[3],nearIndex2,points,valueArray,normalArray);
			   //glVertex3f((points[tempArray[3]].x+points[nearIndex3].x)/2,(points[tempArray[3]].y+points[nearIndex3].y)/2,(points[tempArray[3]].z+points[nearIndex3].z)/2);
			   drawOnePot(tempArray[3],nearIndex3,points,valueArray,normalArray);
			   glEnd();
			   }
			   if(!IsFilled){

			   glBegin(GL_LINE_LOOP);
			   drawOnePot(tempArray[1],nearIndex1,points,valueArray,normalArray);
			   drawOnePot(tempArray[2],nearIndex1,points,valueArray,normalArray);
			   drawOnePot(tempArray[1],nearIndex2,points,valueArray,normalArray);
               glEnd();

			   glBegin(GL_LINE_LOOP);
			   drawOnePot(tempArray[2],nearIndex1,points,valueArray,normalArray);
			   drawOnePot(tempArray[1],nearIndex2,points,valueArray,normalArray);
			   drawOnePot(tempArray[2],nearIndex3,points,valueArray,normalArray);
               glEnd();

			   glBegin(GL_LINE_LOOP);
			   drawOnePot(tempArray[1],nearIndex2,points,valueArray,normalArray);
			   drawOnePot(tempArray[2],nearIndex3,points,valueArray,normalArray);
			   drawOnePot(tempArray[3],nearIndex2,points,valueArray,normalArray);
               glEnd();

			   glBegin(GL_LINE_LOOP);
			   drawOnePot(tempArray[2],nearIndex3,points,valueArray,normalArray);
			   drawOnePot(tempArray[3],nearIndex2,points,valueArray,normalArray);
			   drawOnePot(tempArray[3],nearIndex3,points,valueArray,normalArray);
               glEnd();


			   /*drawOnePot(tempArray[1],nearIndex1,points,valueArray,normalArray);
			   drawOnePot(tempArray[2],nearIndex1,points,valueArray,normalArray);
			   drawOnePot(tempArray[1],nearIndex2,points,valueArray,normalArray);
			   
			   drawOnePot(tempArray[2],nearIndex3,points,valueArray,normalArray);
			   drawOnePot(tempArray[3],nearIndex2,points,valueArray,normalArray);
			   drawOnePot(tempArray[3],nearIndex3,points,valueArray,normalArray);*/
			 
			   }
			   
			}
			if(!HaveFull){
				int tempArray[4];
				//���Ȱ��ĸ����˳���ź�
				for(int i=0;i<4;i++){
				  int tempIndex=nodes[i];
				  int count=0;
				  for(int j=0;j<3;j++){
					  if(gudian[tempIndex][j]==nodes[0]||gudian[tempIndex][j]==nodes[1]||gudian[tempIndex][j]==nodes[2]||gudian[tempIndex][j]==nodes[3]){
					    count++;
					  }
				  } 
				  if(count==1){
	                  tempArray[0]=tempIndex;			  
  					  break;
				  }
				}
				for(int i=0;i<3;i++){
					for(int j=0;j<4;j++){
                      if(gudian[tempArray[0]][i]==nodes[j]){
					    tempArray[1]=nodes[j];
						break;
					  }
					}
				}
				for(int i=0;i<3;i++){
					for(int j=0;j<4;j++){
                      if(gudian[tempArray[1]][i]==nodes[j]&&gudian[tempArray[1]][i]!=tempArray[0]){
					    tempArray[2]=nodes[j];
						break;
					  }
					}
				}
				for(int i=0;i<3;i++){
					for(int j=0;j<4;j++){
                      if(gudian[tempArray[2]][i]==nodes[j]&&gudian[tempArray[2]][i]!=tempArray[1]){
					    tempArray[3]=nodes[j];
						break;
					  }
					}
				}
			   int nearIndex1=findTheForth(tempArray[0],tempArray[1],tempArray[2]);
			   int nearIndex2=findTheForth(tempArray[1],tempArray[2],tempArray[3]);
			   int nearIndex3=findTheForth(nearIndex1,tempArray[2],tempArray[3]);
			   int nearIndex4=findTheForth(nearIndex2,nearIndex3,tempArray[3]);
			
			   if(IsFilled){
			   glBegin(GL_TRIANGLES);
			   //glVertex3f((points[tempArray[0]].x+points[nearIndex1].x)/2,(points[tempArray[0]].y+points[nearIndex1].y)/2,(points[tempArray[0]].z+points[nearIndex1].z)/2);
			     drawOnePot(tempArray[0],nearIndex1,points,valueArray,normalArray);
			   //glVertex3f((points[tempArray[0]].x+points[nearIndex4].x)/2,(points[tempArray[0]].y+points[nearIndex4].y)/2,(points[tempArray[0]].z+points[nearIndex4].z)/2);
			     drawOnePot(tempArray[0],nearIndex4,points,valueArray,normalArray);
			  // glVertex3f((points[tempArray[2]].x+points[nearIndex1].x)/2,(points[tempArray[2]].y+points[nearIndex1].y)/2,(points[tempArray[2]].z+points[nearIndex1].z)/2);
			     drawOnePot(tempArray[2],nearIndex1,points,valueArray,normalArray);

			  // glVertex3f((points[tempArray[0]].x+points[nearIndex4].x)/2,(points[tempArray[0]].y+points[nearIndex4].y)/2,(points[tempArray[0]].z+points[nearIndex4].z)/2);
			     drawOnePot(tempArray[0],nearIndex4,points,valueArray,normalArray);
			   //glVertex3f((points[tempArray[2]].x+points[nearIndex1].x)/2,(points[tempArray[2]].y+points[nearIndex1].y)/2,(points[tempArray[2]].z+points[nearIndex1].z)/2);
			     drawOnePot(tempArray[2],nearIndex1,points,valueArray,normalArray);
			   //glVertex3f((points[tempArray[3]].x+points[nearIndex2].x)/2,(points[tempArray[3]].y+points[nearIndex2].y)/2,(points[tempArray[3]].z+points[nearIndex2].z)/2);
			     drawOnePot(tempArray[3],nearIndex2,points,valueArray,normalArray);

			  // glVertex3f((points[tempArray[0]].x+points[nearIndex4].x)/2,(points[tempArray[0]].y+points[nearIndex4].y)/2,(points[tempArray[0]].z+points[nearIndex4].z)/2);
			     drawOnePot(tempArray[0],nearIndex4,points,valueArray,normalArray);
			  // glVertex3f((points[tempArray[1]].x+points[nearIndex2].x)/2,(points[tempArray[1]].y+points[nearIndex2].y)/2,(points[tempArray[1]].z+points[nearIndex2].z)/2);
			     drawOnePot(tempArray[1],nearIndex2,points,valueArray,normalArray);
			  // glVertex3f((points[tempArray[3]].x+points[nearIndex2].x)/2,(points[tempArray[3]].y+points[nearIndex2].y)/2,(points[tempArray[3]].z+points[nearIndex2].z)/2);
			     drawOnePot(tempArray[3],nearIndex2,points,valueArray,normalArray);

			  // glVertex3f((points[tempArray[3]].x+points[nearIndex3].x)/2,(points[tempArray[3]].y+points[nearIndex3].y)/2,(points[tempArray[3]].z+points[nearIndex3].z)/2);
			     drawOnePot(tempArray[3],nearIndex3,points,valueArray,normalArray);
			  // glVertex3f((points[tempArray[3]].x+points[nearIndex2].x)/2,(points[tempArray[3]].y+points[nearIndex2].y)/2,(points[tempArray[3]].z+points[nearIndex2].z)/2);
			     drawOnePot(tempArray[3],nearIndex2,points,valueArray,normalArray);
			   //glVertex3f((points[tempArray[2]].x+points[nearIndex1].x)/2,(points[tempArray[2]].y+points[nearIndex1].y)/2,(points[tempArray[2]].z+points[nearIndex1].z)/2);
			     drawOnePot(tempArray[2],nearIndex1,points,valueArray,normalArray);
			   glEnd();
			   
			   }
			   if(!IsFilled){
			   glBegin(GL_LINE_LOOP);
			     drawOnePot(tempArray[0],nearIndex1,points,valueArray,normalArray);
			     drawOnePot(tempArray[0],nearIndex4,points,valueArray,normalArray);
			     drawOnePot(tempArray[2],nearIndex1,points,valueArray,normalArray);
			   glEnd();
              glBegin(GL_LINE_LOOP);
			     drawOnePot(tempArray[0],nearIndex4,points,valueArray,normalArray);
			     drawOnePot(tempArray[2],nearIndex1,points,valueArray,normalArray);
			     drawOnePot(tempArray[3],nearIndex2,points,valueArray,normalArray);
			   glEnd();
              glBegin(GL_LINE_LOOP);
			     drawOnePot(tempArray[0],nearIndex4,points,valueArray,normalArray);
			     drawOnePot(tempArray[1],nearIndex2,points,valueArray,normalArray);
			     drawOnePot(tempArray[3],nearIndex2,points,valueArray,normalArray);
			   glEnd();
              glBegin(GL_LINE_LOOP);
			     drawOnePot(tempArray[3],nearIndex3,points,valueArray,normalArray);
			     drawOnePot(tempArray[3],nearIndex2,points,valueArray,normalArray);
			     drawOnePot(tempArray[2],nearIndex1,points,valueArray,normalArray);
			   glEnd();
			   }
			   
			}
		}
		//�������������
		if(bianTakenNumber==2){
			int gudianNumber=computGudian(nodes);
			if(gudianNumber==0){
			for(int i=0;i<12;i++){
			  if(bianTaken[i]==2){
			   drawGuXian(bian[i][0],bian[i][1],points,valueArray,normalArray);
			  }
			}

			}
			if(gudianNumber==1){
            int index=-1;
			int indexArray[3];
			for(int i=0;i<4;i++){
			int tempIndex=nodes[i];
			int count=0;
				for(int j=0;j<3;j++){
				  if(gudian[tempIndex][j]!=nodes[0]&&gudian[tempIndex][j]!=nodes[1]&&gudian[tempIndex][j]!=nodes[2]&&gudian[tempIndex][j]!=nodes[3]){
				     count++;
				  }
				}
			 if(count==3){
			   index=tempIndex;
			   drawGuDian(index,points,valueArray,normalArray);
			   break;
			 }
			}
			int count2=0;
			for(int i=0;i<4;i++){
			   if(nodes[i]!=index){
			    indexArray[count2]=nodes[i];
				count2++;
			   }
			}
			drawThreeInOne(indexArray[0],indexArray[1],indexArray[2],points,valueArray,normalArray);
			}
		}
		if(bianTakenNumber==0){
			for(int i=0;i<nodeNumber;i++){
			 int index=nodes[i];
			 drawGuDian(index,points,valueArray,normalArray);
		  }
		}
	}
}

void CNatureView::MarchingCube(void)
{
	float singleBian=0.5;
	for(float i=0;i<length;i+=singleBian){//��λ���й�
		for(float j=0;j<width;j+=singleBian){
			for(float k=0;k<height;k+=singleBian){
				Vec points[8];

				points[0].x=i;
				points[0].y=j;
				points[0].z=k;

				points[1].x=i+singleBian;
				points[1].y=j;
				points[1].z=k;

				points[2].x=i+singleBian;
				points[2].y=j;
				points[2].z=k+singleBian;

				points[3].x=i;
				points[3].y=j;
				points[3].z=k+singleBian;

				points[4].x=i;
				points[4].y=j+singleBian;
				points[4].z=k;

				points[5].x=i+singleBian;
				points[5].y=j+singleBian;
				points[5].z=k;

				points[6].x=i+singleBian;
				points[6].y=j+singleBian;
				points[6].z=k+singleBian;

				points[7].x=i;
				points[7].y=j+singleBian;
				points[7].z=k+singleBian;

				//����õ�value��һ���б�

				createMC(points);
				//���㺯��ֵ�Եõ�״̬��
				//����ֵ��
				//�����
			}
		}
	}
}


