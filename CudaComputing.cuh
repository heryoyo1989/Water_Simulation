#ifndef _CUDACOMPUTING_CUH_

#define _CUDACOMPUTING_CUH_

//#include "Dian.h"

#define imin(a,b)(a<b?a:b)
//这一定要定义好


//const int Elements = 1000;
//const int Hash_entries = 8000;
/*const int length=10;
const int width=10;
const int height=20;//可以加一个底面位置变量
const int inEvery=2;
//float singleBian=0.5;

//sph 变量
const int JiaoDianNum=(length*inEvery+1)*(width*inEvery+1)*(height*inEvery+1);
//const int NormalNum=((length-1)*inEvery+1)*((width-1)*inEvery+1)*((height-1)*inEvery+1);
const float Pi=3.1415926f;


const int divZ=(length*inEvery+1)*(width*inEvery+1);
const int divY=(width*inEvery+1);//注意方向变了，有些东西就得改*/


const int N=33*1024;//正好33个
const int threadsPerBlock=256;
const int blocksPerGrid=imin(32,(N+threadsPerBlock-1)/threadsPerBlock);

struct Vec{
	float x;
	float y;
	float z;
};

struct Entry{
	unsigned int key;
	Vec pot;//pot就代替了value，之前的value为void* value，也就是说数据类型是空的，即可以自定义
	Entry *next;
};


struct Table{
	size_t count;
	Entry **entries;
	Entry *pool;
};




int get_table(Vec *particles,Vec *velocities,Vec *accelerations,float *densities,float *pressures);
//要输入数组，再改变
//void computeTest(Grid *g,int n1,int* test,int n2);

void MultDot(float *a,float *b,float *partial_c);//不加分号，后就expect

void computeAllValue(Vec *particles,float *pointValue);

void computeAllNormal(Vec *pointNormal,float *pointValue);

#endif