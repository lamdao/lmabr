#ifndef __TYPEDEFS_H
#define __TYPEDEFS_H
//--------------------------------------------------------------------------
#include <float.h>
//--------------------------------------------------------------------------
#ifdef USE_DOUBLE
#define VMAX	DBL_MAX
#define VMIN	-DBL_MAX
#define dtype	double
#else
#define VMAX	FLT_MAX
#define VMIN	-FLT_MAX
#define dtype	float
#endif
//---------------------------------------------------------------------------
#define DIM	7
//------------------------------------------------------------------------
#ifndef uchar
#define uchar unsigned char
#endif
//------------------------------------------------------------------------
#ifndef __CUDACC__
#define DCVAL	const
#define DFAPI
#define DXAPI
#else
#define DCVAL	__constant__
#define DFAPI	__device__
#define DXAPI	__global__
#endif
//--------------------------------------------------------------------------
#ifdef _WIN32
#define DLAPI	extern "C" __declspec(dllexport)
#else
#define DLAPI	extern "C"
#endif
//--------------------------------------------------------------------------
#endif
