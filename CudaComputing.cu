#include "CudaComputing.cuh"
#include "cuda_runtime.h"
#include "device_functions.h"
#include "device_launch_parameters.h"
#include "Lock.h"
#include "math.h"
//#include "windows.h"
#include "stdio.h"

//#include "Dian.h"
#include <afx.h>
#include "book.h"

//#define SIZE (100*1024*1024)
#define SIZE (Amount)//这个相当于随机数组数，点数
#define ELEMENTS (Amount)//这个来自pool。1000个反而更快，难以至信,如果数量匹配就慢，数量快是没复制上
#define HASH_ENTRIES 4000//这个太多了导致速度太慢，真的只是这样吗，桶数，格数，原来是附着在粒子上的

const int Amount = 2000;
const int length=24;
const int width=14;
const int height=20;//可以加一个底面位置变量
const int inEvery=2;
float singleBian=0.5;

//sph 变量
const int JiaoDianNum=(length*inEvery+1)*(width*inEvery+1)*(height*inEvery+1);
const int NormalNum=((length-1)*inEvery+1)*((width-1)*inEvery+1)*((height-1)*inEvery+1);
const float Pi=3.1415926f;

//Voxel voxels[1500];

//Vec pointNormal[JiaoDianNum];
//float pointValue[JiaoDianNum];//角点的value值

Table myTable;//设置为全局变量，用来后面算法来调用以计算压力啊，密度呀，加速啦.好像也用不着设置为全局的变量了，一切都在其中进行了，难道不是这样吗，
              //但全局变量还是有一个好处，就是只声明一次，不用每个timer都调用一次
void initialSPH(){
 
}


//可以同时生成主机和设备的函数的两个版本
__device__ __host__ size_t hash(unsigned int value,size_t count){
    return value%count;
}

__device__ __host__ int pothash(Vec myPot){//二改
    //return value%count;
	int x=(int)floor(myPot.x*100);
	int y=(int)floor(myPot.y*100);
	int z=(int)floor(myPot.z*100);
	return x+20*y+200*z;
}

void initialize_table(Table &table,int entries,int elements){
	table.count=entries;
	cudaMalloc((Entry**)&table.entries,entries*sizeof(Entry*));//分配一个桶数组和一个节点池
	cudaMemset(table.entries,0,entries*sizeof(Entry*));
	cudaMalloc((Entry**)&table.pool,elements*sizeof(Entry));//////////////////////////////////
}

void free_table(Table &table){
	cudaFree(table.pool);
	cudaFree(table.entries);
}

__global__ void fill_entries(Table table,Table hostTable)
{
	int tid = threadIdx.x+blockIdx.x*blockDim.x;
	int stride=blockDim.x*gridDim.x;
	while(tid<table.count){
		if(hostTable.entries[tid]!=NULL)
			hostTable.entries[tid]=(Entry*)((size_t)hostTable.entries[tid]-(size_t)table.pool+(size_t)hostTable.pool);
		tid+=stride;
	}
}

//三个步骤，前两个相对简单，第一个是为散列表数据分配主机内存，第二个是能过cuadMemcpy()将GPU上的数据复制到这块内存。
void copy_table_to_host(const Table &table,Table &hostTable){
	hostTable.count=table.count;
	hostTable.entries=(Entry**)calloc(table.count,sizeof(Entry*));
	hostTable.pool=(Entry*)malloc(ELEMENTS*sizeof(Entry));////////////////////////////////////////////////

	cudaMemcpy(hostTable.entries,table.entries,table.count*sizeof(Entry*),cudaMemcpyDeviceToHost);
	cudaMemcpy(hostTable.pool,table.pool,ELEMENTS*sizeof(Entry),cudaMemcpyDeviceToHost);//////////////////////////////

	/*这个函数里，不能简单地将这些指针复制到主机，这些指针指向的地址在GPU上，它们在主机上不是有效指针。但是指针的相对
	偏移仍然是有效的。有效的主机指针为（x-table.pool)+hostTable.pool,此为第三步*/

	//这下面的两段是影响速度的关键
	for(int i=0;i<table.count;i++){
		if(hostTable.entries[i]!=NULL)
			hostTable.entries[i]=(Entry*)((size_t)hostTable.entries[i]-(size_t)table.pool+(size_t)hostTable.pool);
	}

	/*fill_entries<<<60,256>>>(table,hostTable);*/
	//是不是可以考虑将范围扩大
	for(int i=0;i<ELEMENTS;i++){
		if(hostTable.pool[i].next!=NULL)/////////////////////////////////////////
			hostTable.pool[i].next=(Entry*)((size_t)hostTable.pool[i].next-(size_t)table.pool+(size_t)hostTable.pool);
	}
}

void verify_table(const Table &dev_table){
	Table myTable;
	copy_table_to_host(dev_table,myTable);

	int count=0;
	for(size_t i=0;i<myTable.count;i++){
        
		Entry *current = myTable.entries[i];
		while(current!=NULL){
			++count;
			//if(pothash(current->pot)!=i)TRACE("%d hashed to %1d, but was located at %1d\n",current->key, pothash(current->pot),i);//为什么会出现这个错误呢？？？？？
			
			//unsigned int temp=current->key;
			TRACE("%d hashed to %1d, and was located at %1d\n",current->key, pothash(current->pot),i);
			//TRACE("i is %d",i);
			//TRACE("key is %d",temp);

			current = current->next;
	    }
	}
			TRACE("Count is %d",count);

	/*if(count!=ELEMENTS)
		printf("%d elements found in hash table.Should be %1d\n",count,ELEMENTS);
	else
		printf("all %d elements found in has table.\n",count);*/
	free(myTable.pool);
	free(myTable.entries);
}

//包括一个健数组，一个值数组，散列表本身以及一个锁数组。由于输入数据是两个数组，需要将索引线性化
__global__ void add_to_table(Vec *particles,Table table,Lock *lock){
	int tid = threadIdx.x+blockIdx.x*blockDim.x;
	int stride=blockDim.x*gridDim.x;

	//遍历数组，对数组中每个键能过散列函数（哈希）计算出这个健属于哪个桶。计算出目标桶后，线程会锁定这个桶，添加后，解锁这个桶

	while(tid<ELEMENTS){
		//unsigned int key = keys[tid];
		Vec tempDian = particles[tid];
		int hashValue = pothash(tempDian);//外部一算出来会好一些吗？？？
        //int hashValue = tempDian.value;
		//TRACE("value is %d",hashValue);

		//线程束是一个包含32个线程的集合，并且这些线程以步调一致的方式执行，每次在线程束中只有一个线程可以获取这个锁，防止32个线程束同时竞争
		for(int i=0;i<32;i++){
			if((tid%32)==i){
				//尽量让量子都在范围内活动
                Entry *location=&(table.pool[tid]);///////////////////////////////////////////////////////
				location->key=tid;
				location->pot=tempDian;

				lock[hashValue].lock();
				location->next = table.entries[hashValue];
				table.entries[hashValue] = location;
				lock[hashValue].unlock();
			}
		}
		tid+=stride;
	}
}

int* big_random_block( int size ) {
    int *data = (int*)malloc( size * sizeof(int) );
    //HANDLE_NULL( data );
    for (int i=0; i<size; i++)
        data[i] = rand();

    return data;
}
//vector3D *MyWall,point *r,float *p,float *pr,int number,float m,float po,float h)
__global__ void compute_D_P(Vec *particles,Table table,float *pr,float *p){
	int tid=threadIdx.x+blockIdx.x*blockDim.x;
    //SPH中的固定参数
	float h=0.012;
	float m=0.001;
	float po=1000;
    size_t index;
	Vec myPot;
	Vec tempPot;

	while(tid<Amount){
		//用来累加，中间变量的暂存
        float sum=0;
		float tempR;
		float temp;
		
		myPot=particles[tid];
		
		//首先确定点所在块
		int a=(int)floor(myPot.x*100);
	    int b=(int)floor(myPot.y*100);
	    int c=(int)floor(myPot.z*100);

		for(int i=a-1;i<=a+1;i++){
			for(int j=b-1;j<=b+1;j++){
				for(int k=c-1;k<=c+1;k++){
					if(i>=0&&i<=19&&j>=0&&j<=9&&k>=0&&k<=19){//二变化
					   index=k*200+j*20+i;//二变化
					   Entry *current = table.entries[index];
		               while(current!=NULL){
			             if(current->key!=tid){
							 tempPot=current->pot;
						     tempR=sqrt((tempPot.x-myPot.x)*(tempPot.x-myPot.x)+(tempPot.y-myPot.y)*(tempPot.y-myPot.y)+(tempPot.z-myPot.z)*(tempPot.z-myPot.z));
							 if(tempR<=h){
                               temp=h*h-tempR*tempR;
			                   sum+=m*pow(temp,3);			
							 }
						 }
			             current = current->next;
	                   }
					}
				}
			}
		}
		//pr[tid]=1*315/(64*3.1415926*pow(h,9));
		pr[tid]=sum*315/(64*3.1415926*pow(h,9));

		p[tid]=1*(pr[tid]-po);//可以用来比较

        tid+=blockDim.x*gridDim.x;
	}
}

__global__ void compute_A(Vec *particles,Vec *velocity,Vec *acceleration,Table table,float *pr,float *p){
	int tid=threadIdx.x+blockIdx.x*blockDim.x;
    //SPH中的固定参数
	float h=0.012;
	float m=0.001;
	float ur=3;
	float g=9.8;
	float Pi=3.1415926;
	size_t index;

	while(tid<Amount){
		//用来累加，中间变量的暂存
		float sumX=0;
		float sumY=0;
		float sumZ=0;

       // float sum=0;
		float tempR;
		//float temp;
		
      
		Vec myPot=particles[tid];
		Vec tempPot;
		int tempKey;
		//首先确定点所在块
		int a=(int)floor(myPot.x*100);
	    int b=(int)floor(myPot.y*100);
	    int c=(int)floor(myPot.z*100);

		for(int i=a-1;i<=a+1;i++){
			for(int j=b-1;j<=b+1;j++){
				for(int k=c-1;k<=c+1;k++){
					if(i>=0&&i<=19&&j>=0&&j<=9&&k>=0&&k<=19){//根据范围变化变化
					   index=k*200+j*20+i;
					   Entry *current = table.entries[index];
		               while(current!=NULL){
			             if(current->key!=tid){
							 tempPot=current->pot;
							 tempKey=current->key;
						     tempR=sqrt((tempPot.x-myPot.x)*(tempPot.x-myPot.x)+(tempPot.y-myPot.y)*(tempPot.y-myPot.y)+(tempPot.z-myPot.z)*(tempPot.z-myPot.z));
							 if(tempR<=h){
                               sumX+=(45*m/(Pi*pow(h,6)))*((p[tid]+p[tempKey])*pow((h-tempR),2)*(myPot.x-tempPot.x))/(2*pr[tid]*pr[tempKey])/tempR;
				               sumY+=(45*m/(Pi*pow(h,6)))*((p[tid]+p[tempKey])*pow((h-tempR),2)*(myPot.y-tempPot.y))/(2*pr[tid]*pr[tempKey])/tempR;
				               sumZ+=(45*m/(Pi*pow(h,6)))*((p[tid]+p[tempKey])*pow((h-tempR),2)*(myPot.z-tempPot.z))/(2*pr[tid]*pr[tempKey])/tempR;

				               sumX+=(45*m*ur/(Pi*pow(h,6)))*(velocity[tempKey].x-velocity[tid].x)*(h-tempR)/(pr[tid]*pr[tempKey]);
				               sumY+=(45*m*ur/(Pi*pow(h,6)))*(velocity[tempKey].y-velocity[tid].y)*(h-tempR)/(pr[tid]*pr[tempKey]);
				               sumZ+=(45*m*ur/(Pi*pow(h,6)))*(velocity[tempKey].z-velocity[tid].z)*(h-tempR)/(pr[tid]*pr[tempKey]);
				 			 }
						 }
			             current = current->next;
	                   }
					}
				}
			}
		}
		//sumX=0;
		//sumY=0;
		//sumZ=0;

		acceleration[tid].x=sumX;
 
 		acceleration[tid].y=sumY;
 
 		acceleration[tid].z=sumZ-g;

        tid+=blockDim.x*gridDim.x;
	}
}

__global__ void compute_V(Vec *velocity,Vec *acceleration){//没问题
	int tid=threadIdx.x+blockIdx.x*blockDim.x;
    while(tid<Amount){
        velocity[tid].x=velocity[tid].x+acceleration[tid].x*1/1000;
        velocity[tid].y=velocity[tid].y+acceleration[tid].y*1/1000;
        velocity[tid].z=velocity[tid].z+acceleration[tid].z*1/1000;
        tid+=blockDim.x*gridDim.x;
	}
}

__global__ void compute_P(Vec *particles,Vec *velocity,Vec *acceleration){//没问题
	int tid=threadIdx.x+blockIdx.x*blockDim.x;
    while(tid<Amount){
	    
		particles[tid].x=particles[tid].x+velocity[tid].x*1/1000;
		particles[tid].y=particles[tid].y+velocity[tid].y*1/1000;
		particles[tid].z=particles[tid].z+velocity[tid].z*1/1000;


		//方形边界,可以设置参数

		if(particles[tid].x<=0.00){
			particles[tid].x=0.00;
			velocity[tid].x=-velocity[tid].x/2;
			//acceleration[tid].x=0;
		}
		if(particles[tid].x>=0.199){
			particles[tid].x=0.199;
		    velocity[tid].x=-velocity[tid].x/2;
			//acceleration[tid].x=0;
		}
		if(particles[tid].y<=0.00){
			particles[tid].y=0.00;
			velocity[tid].y=-velocity[tid].y/2;
			//acceleration[tid].y=0;
		}
		if(particles[tid].y>=0.099){
			particles[tid].y=0.099;
			velocity[tid].y=-velocity[tid].y/2;
			//acceleration[tid].y=0;
		}

		if(particles[tid].z<=0.00){
			particles[tid].z=0.00;
			velocity[tid].z=-velocity[tid].z/3;//之前停下来恐因为过了界了
			acceleration[tid].z=-acceleration[tid].z;
		}
		if(particles[tid].z>=0.199){
			particles[tid].z=0.199;
			velocity[tid].z=-velocity[tid].z/2;//之前停下来恐因为过了界了
			//acceleration[tid].z=0;
		}
		tid+=blockDim.x*gridDim.x;
	}
}

//问题的关键是，真正的点是除以100以后的值，对我们有用的是再乘回一百的值

__global__ void computeJiaoDianValue(Vec *particles,float *pointValue){//这个应该加上table
	int tid=threadIdx.x+blockIdx.x*blockDim.x;
	//Point point;
	int a;
	int b;
	int c;

    float px;
	float py;
	float pz;

    float value;
    float radius;
    float R=1.3;

	int divZ=(length*inEvery+1)*(width*inEvery+1);
    int divY=(length*inEvery+1);
	float singleBian=0.5;

	if(tid<JiaoDianNum){
	   value=0;

	   /*可以改成长宽高*/
	   c=floor((float)tid/divZ);
	   b=floor(((float)tid-divZ*c)/divY);
	   a=tid-b*divY-c*divZ;

	   px=(float)a*singleBian;//根据设置的范围要改
	   py=(float)b*singleBian;
	   pz=(float)c*singleBian;

	   for(int j=0;j<Amount;j++){//太暴力了啊，需要改
	     radius=sqrt((px-particles[j].x)*(px-particles[j].x)+(py-particles[j].y)*(py-particles[j].y)+(pz-particles[j].z)*(pz-particles[j].z));
	     if(radius<=R){
	         value+=1000*(1-(radius/R)*(radius/R))*(1-(radius/R)*(radius/R));
	     }
	   }

	   value-=500;
       pointValue[tid]=value;

	   //tid+=blockIdx.x*blockDim.x;
    }   
}

//要改要改要改，一个r,一个point Value
void computeAllValue(Vec *particles,float *pointValue){
  // Voxel *dev_vox;
   Vec *dev_particles;
   float *dev_value;

   //GPU分配空间
   //cudaMalloc((Voxel**)&dev_vox,1500*sizeof(Voxel));
   cudaMalloc((void**)&dev_particles,Amount*sizeof(Vec));
   cudaMalloc((void**)&dev_value,JiaoDianNum*sizeof(float));

   //复制数据到GPU
   
  // cudaMemcpy(dev_vox,voxels,1500*sizeof(Voxel),cudaMemcpyHostToDevice);
   cudaMemcpy(dev_particles,particles,Amount*sizeof(Vec),cudaMemcpyHostToDevice);
   cudaMemcpy(dev_value,pointValue,JiaoDianNum*sizeof(float),cudaMemcpyHostToDevice);


   //初始化索引 
   
   computeJiaoDianValue<<<1000,512>>>(dev_particles,dev_value);
   
   //GPU复制回CPU
   //cudaMemcpy(voxels,dev_vox,1500*sizeof(Voxel),cudaMemcpyDeviceToHost);
   cudaMemcpy(particles,dev_particles,Amount*sizeof(Vec),cudaMemcpyDeviceToHost);
   cudaMemcpy(pointValue,dev_value,JiaoDianNum*sizeof(float),cudaMemcpyDeviceToHost);

   //释放内存
   //cudaFree(dev_vox);
   cudaFree(dev_particles);
   cudaFree(dev_value);
}

__global__ void computeSingleNormal(Vec *pointNormal,float *pointValue){
//关键要考虑好边界的问题
   int i=threadIdx.x+blockIdx.x*blockDim.x;//注意i
   int a;
   int b;
   int c;
   int divY=(length*inEvery+1);
   int divZ=(length*inEvery+1)*(width*inEvery+1);
   float singleBian=0.5;
   if(i<JiaoDianNum){
      c=floor((float)i/(float)divZ);
      b=floor(((float)i-(float)divZ*c)/(float)divY);
      a=i-b*divY-c*divZ;
	  if(a>0&&a<length*inEvery+1){
	    pointNormal[i].x=(pointValue[i+1]-pointValue[i-1])/(2*singleBian);
	  }
	  if(a==0&&a==length*inEvery+1){
	    pointNormal[i].x=0;
	  }
	  if(b>0&&b<width*inEvery+1){
	    pointNormal[i].y=(pointValue[i+divY]-pointValue[i-divY])/(2*singleBian);
	  }
	  if(b==0&&b==width*inEvery+1){
	    pointNormal[i].y=0;
	  }
	  if(c>0&&c<height*inEvery+1){
	    pointNormal[i].z=(pointValue[i+divZ]-pointValue[i-divZ])/(2*singleBian);
	  }
	  if(c==0&&c==height*inEvery+1){
	    pointNormal[i].z=0;
	  }
    //i+=blockDim.x*gridDim.x;
   }
}

void computeAllNormal(Vec *pointNormal,float *pointValue){
   Vec *dev_normal;
   float *dev_value;

   //GPU分配空间
   cudaMalloc((Vec**)&dev_normal,JiaoDianNum*sizeof(Vec));
   cudaMalloc((float**)&dev_value,JiaoDianNum*sizeof(float));

   //复制数据到GPU
   
  // cudaMemcpy(dev_vox,voxels,1500*sizeof(Voxel),cudaMemcpyHostToDevice);
   cudaMemcpy(dev_normal,pointNormal,JiaoDianNum*sizeof(Vec),cudaMemcpyHostToDevice);
   cudaMemcpy(dev_value,pointValue,JiaoDianNum*sizeof(float),cudaMemcpyHostToDevice);


   //初始化索引 
   

   //29435
   //computSingleV<<<150,128>>>(dev_vox,dev_r,dev_value);
  // computeJiaoDianValue<<<1000,1024>>>(dev_normal,dev_value);
   computeSingleNormal<<<1000,512>>>(dev_normal,dev_value);


   //GPU复制回CPU
   //cudaMemcpy(voxels,dev_vox,1500*sizeof(Voxel),cudaMemcpyDeviceToHost);
   cudaMemcpy(pointNormal,dev_normal,JiaoDianNum*sizeof(Vec),cudaMemcpyDeviceToHost);
   cudaMemcpy(pointValue,dev_value,JiaoDianNum*sizeof(float),cudaMemcpyDeviceToHost);

   //释放内存
   //cudaFree(dev_vox);
   cudaFree(dev_normal);
   cudaFree(dev_value);

}

Lock lock[HASH_ENTRIES];

int get_table(Vec *particles,Vec *velocities,Vec *accelerations,float *densities,float *pressures){
	//unsigned int buffer[10]={

	//cudaEvent_t start,stop;

	//cudaEventCreate(&start);
	//cudaEventCreate(&stop);
	//cudaEventRecord(start,0);


	//unsigned int *buffer = (unsigned int*)big_random_block(SIZE);//随机键数组
	//unsigned int *dev_keys;
	//void **dev_values;
	//cudaMalloc((void**)&dev_keys,SIZE*sizeof(unsigned int));
	//cudaMalloc((void**)&dev_values,SIZE);
	//cudaMemcpy(dev_keys,buffer,SIZE*sizeof(unsigned int),cudaMemcpyHostToDevice);//复制
    Table table;//这个table是在GPU里分配空间的
	initialize_table(table,HASH_ENTRIES,ELEMENTS);

	Vec *dev_particles;
	cudaMalloc((Vec**)&dev_particles,Amount*sizeof(Vec));//真是傻逼了,之前分配的空间太小了
	cudaMemcpy(dev_particles,particles,Amount*sizeof(Vec),cudaMemcpyHostToDevice);//复制

	

	//为散列表桶备好锁，每一个桶分配一个锁。不能只用一个锁，这样会降低性能，不要竞争
	Lock *dev_lock;
	cudaMalloc((Lock**)&dev_lock,HASH_ENTRIES*sizeof(Lock));
	cudaMemcpy(dev_lock,lock,HASH_ENTRIES*sizeof(Lock),cudaMemcpyHostToDevice);
	
	float *dev_density;
	cudaMalloc((float**)&dev_density,Amount*sizeof(float));
	cudaMemcpy(dev_density,densities,Amount*sizeof(float),cudaMemcpyHostToDevice);

    float *dev_press;
	cudaMalloc((float**)&dev_press,Amount*sizeof(float));
	cudaMemcpy(dev_press,pressures,Amount*sizeof(float),cudaMemcpyHostToDevice);

    Vec *dev_vel;
	cudaMalloc((Vec**)&dev_vel,Amount*sizeof(Vec));
	cudaMemcpy(dev_vel,velocities,Amount*sizeof(Vec),cudaMemcpyHostToDevice);

	Vec *dev_accel;
	cudaMalloc((Vec**)&dev_accel,Amount*sizeof(Vec));
	cudaMemcpy(dev_accel,accelerations,Amount*sizeof(Vec),cudaMemcpyHostToDevice);
	//健添加到散列表
	add_to_table<<<60,256>>>(dev_particles,table,dev_lock);

	//cudaEventRecord(stop,0);
	//cudaEventSynchronize(stop);
//	float elapsedTime;
//	cudaEventElapsedTime(&elapsedTime,start,stop);/////////////////////////////////////////

	//printf("Time to hash:%3.1f ms\n",elapsedTime);

	//I***************************************
	//verify_table(table);//需要输出桶数组以供后面再来使用，在这个大函数中对位置函数的值没有改变

	//cudaEventDestroy(start);
	//cudaEventDestroy(stop);
	
	compute_D_P<<<60,256>>>(dev_particles,table,dev_density,dev_press);

	/*float den[Amount];

	cudaMemcpy(den,dev_density,Amount*sizeof(float),cudaMemcpyDeviceToHost);

	for(int i=0;i<Amount;i++){
	    den[i];
	}*/

	compute_A<<<60,256>>>(dev_particles,dev_vel,dev_accel,table,dev_density,dev_press);

	compute_V<<<60,256>>>(dev_vel,dev_accel);

	compute_P<<<60,256>>>(dev_particles,dev_vel,dev_accel);
	//就让数据在GPU中传递，最后传出位置的数据就行
	//下一步的计算value等就也可以借助此邻接表，相信自己，加油，这个机制很好

	cudaMemcpy(accelerations,dev_accel,Amount*sizeof(Vec),cudaMemcpyDeviceToHost);

	cudaMemcpy(velocities,dev_vel,Amount*sizeof(Vec),cudaMemcpyDeviceToHost);

	cudaMemcpy(particles,dev_particles,Amount*sizeof(Vec),cudaMemcpyDeviceToHost);

	

	free_table(table);

	cudaFree(dev_lock);
	cudaFree(dev_particles);
	cudaFree(dev_density);
	cudaFree(dev_press);
	cudaFree(dev_vel);
	cudaFree(dev_accel);

	//之前分配了table lock particles的三个空间最后都要释放掉
	//cudaFree(dev_keys);
	//cudaFree(dev_values);
	//free(buffer);
	return 0;
}

__global__ void dot(float* a,float* b,float* c){
  __shared__ float cache[threadsPerBlock];//这个必须得是shared的
  int tid=threadIdx.x+blockIdx.x*blockDim.x;
  int cacheIndex=threadIdx.x;

  float temp=0;
  while(tid<N){
	  temp +=a[tid]*b[tid];
	  tid +=blockDim.x*gridDim.x;
  }

  cache[cacheIndex]=temp;

  __syncthreads();

  int i=blockDim.x/2;

  while(i!=0){//归约运算
     if(cacheIndex<i){
		 cache[cacheIndex]+=cache[cacheIndex+i];
	 }
	 __syncthreads();
	 i/=2;
  }
  if(cacheIndex==0){//最后归到0上了
	  c[blockIdx.x]=cache[0];
  }
}

void MultDot(float *a,float *b,float *partial_c){
	float *dev_a,*dev_b,*dev_partial_c;

	cudaMalloc((float**)&dev_a,N*sizeof(float));
	cudaMalloc((float**)&dev_b,N*sizeof(float));
	cudaMalloc((float**)&dev_partial_c,blocksPerGrid*sizeof(float));

	cudaMemcpy(dev_a,a,N*sizeof(float),cudaMemcpyHostToDevice);
	cudaMemcpy(dev_b,b,N*sizeof(float),cudaMemcpyHostToDevice);
	//cudaMemcpy(
	
	dot<<<blocksPerGrid,threadsPerBlock>>>(dev_a,dev_b,dev_partial_c);

	cudaMemcpy(partial_c,dev_partial_c,blocksPerGrid*sizeof(float),cudaMemcpyDeviceToHost);

	cudaFree(dev_a);
	cudaFree(dev_b);
	cudaFree(dev_partial_c);
}

/*
__global__ void getEveryNear(point *r,int number,float h){
  int i=threadIdx.x+blockIdx.x*blockDim.x;
  int j=threadIdx.y+blockIdx.y*blockDim.y;
   
 // float h=0.0055;//改数目要改

   while(i<number){
	   r[i].index=0;
       int k=j;
	   while(k<number){
		   float tempR;
		   
		   tempR=sqrt((r[i].x-r[k].x)*(r[i].x-r[k].x)+(r[i].y-r[k].y)*(r[i].y-r[k].y)+(r[i].z-r[k].z)*(r[i].z-r[k].z));
		   
	    if(tempR<h){
	     int tempIndex=r[i].index;
		 r[i].points[tempIndex]=k;
		 tempIndex++;
		 r[i].index=tempIndex;
		 if(tempIndex>12){
		 r[i].index=12;
		 }
	   } 
	   k+=blockDim.y*gridDim.y;
	  }
	 i+=blockDim.x*gridDim.x;
   }    
	   

	  
}
void getNearPoint(point *r,int number,float h){
   
   point *dev_r;
  
   cudaMalloc((point**)&dev_r,number*sizeof(point));
  
   
   cudaMemcpy(dev_r,r,number*sizeof(point),cudaMemcpyHostToDevice);
   //初始化索引 
   
   getEveryNear<<<100,128>>>(dev_r,number,h);
   //fillVoxels<<<200,10>>>();
   
   //GPU复制回CPU
   cudaMemcpy(r,dev_r,number*sizeof(point),cudaMemcpyDeviceToHost);
   //释放内存
   cudaFree(dev_r);
}
*/

/*
__global__ void computeTestKernel(Grid* g,int* test){
	int i=threadIdx.x;
	Node* tempNode;
	int temp;
	if(i<100){
	  temp=test[i]%8;

	  //tempNode=(Node*)malloc(sizeof(Node));

	 

      tempNode->num=test[i];
	  tempNode->next=g[i].first;
	  g[i].first->next=tempNode;
	  //g[temp].num=test[i];
	  
	}
}


void computeTest(Grid* g,int n1,int* test,int n2)
{
	Grid* dev_g;
	int* dev_test;

	cudaMalloc((Grid**)&dev_g,n1*sizeof(Grid));
	cudaMalloc((int**)&dev_test,n2*sizeof(int));

	cudaMemcpy(dev_g,g,n1*sizeof(Grid),cudaMemcpyHostToDevice);
	cudaMemcpy(dev_test,test,n2*sizeof(int),cudaMemcpyHostToDevice);

	
	computeTestKernel<<<100,1>>>(g,test);


	cudaMemcpy(g,dev_g,n1*sizeof(Grid),cudaMemcpyDeviceToHost);
 
	cudaFree(dev_g);
	cudaFree(dev_test);
}
*/
                  