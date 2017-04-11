#ifndef __LOCK_H__
#define __LOCK_H__

#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <sm_11_atomic_functions.h>

struct Lock {
	int *mutex;
	Lock( void ) {
		cudaMalloc( (void**)&mutex, sizeof(int) ) ;
		cudaMemset( mutex, 0, sizeof(int) ) ;
	}
	~Lock( void ) {
		cudaFree( mutex );
	}
	__device__ void lock( void ) {
		while(atomicCAS( mutex, 0, 1 ) != 0 );//compute_20,sm_20
	}
	__device__ void unlock( void ) {
		atomicExch( mutex, 0 );
	}
};

#endif