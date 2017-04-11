#ifndef _CUDACOMPUTING_CUH_

#define _CUDACOMPUTING_CUH_

//#include "Dian.h"

#define imin(a,b)(a<b?a:b)
//��һ��Ҫ�����


//const int Elements = 1000;
//const int Hash_entries = 8000;
/*const int length=10;
const int width=10;
const int height=20;//���Լ�һ������λ�ñ���
const int inEvery=2;
//float singleBian=0.5;

//sph ����
const int JiaoDianNum=(length*inEvery+1)*(width*inEvery+1)*(height*inEvery+1);
//const int NormalNum=((length-1)*inEvery+1)*((width-1)*inEvery+1)*((height-1)*inEvery+1);
const float Pi=3.1415926f;


const int divZ=(length*inEvery+1)*(width*inEvery+1);
const int divY=(width*inEvery+1);//ע�ⷽ����ˣ���Щ�����͵ø�*/


const int N=33*1024;//����33��
const int threadsPerBlock=256;
const int blocksPerGrid=imin(32,(N+threadsPerBlock-1)/threadsPerBlock);

struct Vec{
	float x;
	float y;
	float z;
};

struct Entry{
	unsigned int key;
	Vec pot;//pot�ʹ�����value��֮ǰ��valueΪvoid* value��Ҳ����˵���������ǿյģ��������Զ���
	Entry *next;
};


struct Table{
	size_t count;
	Entry **entries;
	Entry *pool;
};




int get_table(Vec *particles,Vec *velocities,Vec *accelerations,float *densities,float *pressures);
//Ҫ�������飬�ٸı�
//void computeTest(Grid *g,int n1,int* test,int n2);

void MultDot(float *a,float *b,float *partial_c);//���ӷֺţ����expect

void computeAllValue(Vec *particles,float *pointValue);

void computeAllNormal(Vec *pointNormal,float *pointValue);

#endif