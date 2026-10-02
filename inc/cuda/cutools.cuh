#ifndef CUTOOLS_CUH
#define CUTOOLS_CUH

#include <cuda_runtime.h>
#include "cudaField.hpp"

template<typename T, unsigned int blockSize> __global__ void fieldMax_k(CudaField<T> fieldIn, CudaField<float> fieldOut, unsigned int n);
template<typename T> float fieldMax(CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);

template<typename T, unsigned int blockSize> __global__ void fieldSum_k(CudaField<T> fieldIn, CudaField<float> fieldOut, unsigned int n);
template<typename T> float fieldNorm(CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);

void combineChannels(CudaFieldBase *fieldX, CudaFieldBase *dst);
void combineChannels(CudaFieldBase *fieldX, CudaFieldBase *fieldY, CudaFieldBase *dst);
void combineChannels(CudaFieldBase *fieldX, CudaFieldBase *fieldY, CudaFieldBase *fieldZ, CudaFieldBase *dst);
void combineChannels(CudaFieldBase *fieldX, CudaFieldBase *fieldY, CudaFieldBase *fieldZ, CudaFieldBase *fieldW, CudaFieldBase *dst);

void splitChannels(CudaFieldBase *field, CudaFieldBase *dstX);
void splitChannels(CudaFieldBase *field, CudaFieldBase *dstX, CudaFieldBase *dstY);
void splitChannels(CudaFieldBase *field, CudaFieldBase *dstX, CudaFieldBase *dstY, CudaFieldBase *dstZ);
void splitChannels(CudaFieldBase *field, CudaFieldBase *dstX, CudaFieldBase *dstY, CudaFieldBase *dstZ, CudaFieldBase *dstW);


inline __device__ float2 cMult(float2 z0, float2 z1) { return float2{z0.x*z1.x - z0.y*z1.y, z0.x*z1.y + z1.x*z0.y}; }
inline __device__ float2 cConj(float2 z0)            { return float2{z0.x, -z0.y}; }
inline __device__ float2 cDiv (float2 z0, float2 z1)
{
  float denom = (z1.x*z1.x + z1.y*z1.y);
  return float2 {(z0.x*z1.x + z0.y*z1.y)/denom, (z0.y*z1.x - z0.x*z1.y)/denom};
}

#endif // CUTOOLS_CUH
