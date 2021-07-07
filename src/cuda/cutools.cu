
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>

#include "cudaField.hpp"
#include "vector-operators.h"
#include "field-operators.cuh"
#include "cutools.cuh"
#include "cutools.hpp"

#define BLOCKDIM_X 8
#define BLOCKDIM_Y 8


// find maximum  value in field
template<unsigned int blockSize>
__device__ void maxWarpReduce(volatile int *sdata, unsigned int tid)
{
  float *tdata = (float*)sdata;
  if(blockSize >= 64) { tdata[tid] = max(tdata[tid], tdata[tid+32]); }
  if(blockSize >= 32) { tdata[tid] = max(tdata[tid], tdata[tid+16]); }
  if(blockSize >= 16) { tdata[tid] = max(tdata[tid], tdata[tid+ 8]); }
  if(blockSize >=  8) { tdata[tid] = max(tdata[tid], tdata[tid+ 4]); }
  if(blockSize >=  4) { tdata[tid] = max(tdata[tid], tdata[tid+ 2]); }
  if(blockSize >=  2) { tdata[tid] = max(tdata[tid], tdata[tid+ 1]); }
}

template<typename T, unsigned int blockSize>
__global__ void fieldMax_k(CudaField<T> fieldIn, CudaField<float> fieldOut, unsigned int n)
{
  extern __shared__ int sdata[];
  unsigned int tid = threadIdx.x;
  unsigned int i   = blockIdx.x*(2*blockSize) + tid;
  unsigned int gridSize = blockSize*2*gridDim.x;
  
  float *tdata = (float*)sdata;
  float dMax = 0.0f;
  while(i < n)
    {
      float a = max(length(fieldIn.dData[i]), length(fieldIn.dData[i+blockSize]));
      dMax = max(dMax, a);
      i += gridSize;
    }
  tdata[tid] = dMax;
  __syncthreads();
  
  if(blockSize >= 512) { if(tid < 256) { tdata[tid] = max(tdata[tid], tdata[tid + 256]); } __syncthreads(); }
  if(blockSize >= 256) { if(tid < 128) { tdata[tid] = max(tdata[tid], tdata[tid + 128]); } __syncthreads(); }
  if(blockSize >= 128) { if(tid <  64) { tdata[tid] = max(tdata[tid], tdata[tid +  64]); } __syncthreads(); }
  if(tid < 32) { maxWarpReduce<blockSize>(sdata, tid); }
  if(tid == 0) { fieldOut.dData[blockIdx.x] = tdata[0]; }
}


template<unsigned int blockSize>
__global__ void fieldMaxTex_k(CudaFieldTex fieldIn, CudaField<float> fieldOut, unsigned int n)
{
  extern __shared__ int sdata[];
  unsigned int tid = threadIdx.x;
  unsigned int i   = blockIdx.x*(2*blockSize) + tid;
  unsigned int gridSize = blockSize*2*gridDim.x;
  
  float *tdata = (float*)sdata;
  float dMax = 0.0f;
  while(i < n)
    {
      float a = max(length(float3{fieldIn.dData[i].x, fieldIn.dData[i].y, fieldIn.dData[i].z}),
                    length(float3{fieldIn.dData[i+blockSize].x, fieldIn.dData[i+blockSize].y, fieldIn.dData[i+blockSize].z}));
      dMax = max(dMax, a);
      i += gridSize;
    }
  tdata[tid] = dMax;
  __syncthreads();
  
  if(blockSize >= 512) { if(tid < 256) { tdata[tid] = max(tdata[tid], tdata[tid + 256]); } __syncthreads(); }
  if(blockSize >= 256) { if(tid < 128) { tdata[tid] = max(tdata[tid], tdata[tid + 128]); } __syncthreads(); }
  if(blockSize >= 128) { if(tid <  64) { tdata[tid] = max(tdata[tid], tdata[tid +  64]); } __syncthreads(); }
  if(tid < 32) { maxWarpReduce<blockSize>(sdata, tid); }
  if(tid == 0) { fieldOut.dData[blockIdx.x] = tdata[0]; }
}

// multiplies two fields
template<typename T>
__global__ void multC_k(CudaField<T> src, CudaField<T> dst, float mult)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = src.dData[i]*mult;
    }
}

// multiplies two fields
__global__ void multTexC_k(CudaFieldTex src, CudaFieldTex dst, float mult)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      float4 val = src.dData[i]*mult;
      val.w = 1.0f;
      dst.dData[i] = val;
    }
}


template<typename T>
float fieldMax(CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst)
{
  // dim3 block(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));
  
  if(!field || !dst) { std::cout << "====> ERROR(fieldMax): field is null!\n"; return 0.0f; }

  // cast field to its underlying type and validate
  CudaField<T> *f  = reinterpret_cast<CudaField<T>*>(field);
  if(!f)             { std::cout << "====> ERROR(fieldMax): field type cast error! ("   << f->type() << ")\n"; return 0.0f; }
  else if(!f->map()) { std::cout << "====> ERROR(fieldMax): field texture map error! (" << f->type() << ")\n"; return 0.0f; }
  
  int threads = BLOCKDIM_X*BLOCKDIM_Y;
  int sz = field->size.x * field->size.y;

  dim3 dimBlock(threads, 1);
  dim3 dimGrid((int)ceil(sz/(float)(threads*2)), 1);
  size_t sharedMem = threads*sizeof(T);

  if(field->isTexture())
    {
      switch(threads)
        {
        case 512: fieldMaxTex_k<512> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case 256: fieldMaxTex_k<256> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case 128: fieldMaxTex_k<128> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case 64:  fieldMaxTex_k< 64> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case 32:  fieldMaxTex_k< 32> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case 16:  fieldMaxTex_k< 16> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case  8:  fieldMaxTex_k<  8> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case  4:  fieldMaxTex_k<  4> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case  2:  fieldMaxTex_k<  2> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case  1:  fieldMaxTex_k<  1> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        }
    }
  else
    {
      switch(threads)
        {
        case 512: fieldMax_k<T, 512> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case 256: fieldMax_k<T, 256> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case 128: fieldMax_k<T, 128> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case 64:  fieldMax_k<T,  64> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case 32:  fieldMax_k<T,  32> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case 16:  fieldMax_k<T,  16> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case  8:  fieldMax_k<T,   8> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case  4:  fieldMax_k<T,   4> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case  2:  fieldMax_k<T,   2> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case  1:  fieldMax_k<T,   1> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        }
    }
  cudaDeviceSynchronize();
  getLastCudaError("fieldMax_k failed!");

  dst->pullData();
  float totalMax = 0.0f;
  if(dst->hData)
    {
      for(int i = 0; i < dimGrid.x; i++)
        { totalMax = max(totalMax, dst->hData[i]); }
    }

  if(totalMax != 0.0f) // can't scale values by 1/0
    {
      CudaField<T> *fo  = reinterpret_cast<CudaField<T>*>(fieldOut);
      if(!fo)             { std::cout << "====> ERROR(fieldMax): fieldOut type cast error! ("   << fo->type() << ")\n"; f->unmap(); return 0.0f; }
      else if(!fo->map()) { std::cout << "====> ERROR(fieldMax): fieldOut texture map error! (" << fo->type() << ")\n"; f->unmap(); return 0.0f; }
  
      dimBlock = dim3(BLOCKDIM_X, BLOCKDIM_Y);
      dimGrid  = dim3((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));

      if(field->isTexture() && fieldOut->isTexture())
        {
          CudaFieldTex *fT  = reinterpret_cast<CudaFieldTex*>(field);
          CudaFieldTex *foT = reinterpret_cast<CudaFieldTex*>(fieldOut);
          multTexC_k <<<dimGrid, dimBlock>>>(*fT, *foT, 1.0f/totalMax);
        }
      else
        { multC_k<T> <<<dimGrid, dimBlock>>>(*f, *fo, 1.0f/totalMax); }
      getLastCudaError("multC_k failed!");
      fo->unmap(); f->unmap();
    }
  return totalMax;
}

template float fieldMax<int>    (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldMax<int2>   (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldMax<int3>   (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldMax<int4>   (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldMax<float>  (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldMax<float2> (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldMax<float3> (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldMax<float4> (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldMax<double> (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldMax<double2>(CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldMax<double3>(CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldMax<double4>(CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);






//// FIND SUM/AVERAGE (FOR FIELD NORMALIZATION) ////

// find sum of magnitudes in field
template<unsigned int blockSize>
__device__ void sumWarpReduce(volatile int *sdata, unsigned int tid)
{
  float *tdata = (float*)sdata;
  if(blockSize >= 64) { tdata[tid] += tdata[tid+32]; }
  if(blockSize >= 32) { tdata[tid] += tdata[tid+16]; }
  if(blockSize >= 16) { tdata[tid] += tdata[tid+ 8]; }
  if(blockSize >=  8) { tdata[tid] += tdata[tid+ 4]; }
  if(blockSize >=  4) { tdata[tid] += tdata[tid+ 2]; }
  if(blockSize >=  2) { tdata[tid] += tdata[tid+ 1]; }
}
template<typename T, unsigned int blockSize>
__global__ void fieldSum_k(CudaField<T> fieldIn, CudaField<float> fieldOut, unsigned int n)
{
  extern __shared__ int sdata[];
  unsigned int tid = threadIdx.x;
  unsigned int i   = blockIdx.x*(2*blockSize) + tid;
  unsigned int gridSize = blockSize*2*gridDim.x;
  
  float *tdata = (float*)sdata;
  tdata[tid]   = 0.0f;
  while(i < n) { tdata[tid] += length(fieldIn.dData[i]) + length(fieldIn.dData[i+blockSize]); i += gridSize; }
  __syncthreads();
  if(blockSize >= 512) { if(tid < 256) { tdata[tid] += tdata[tid+256]; } __syncthreads(); }
  if(blockSize >= 256) { if(tid < 128) { tdata[tid] += tdata[tid+128]; } __syncthreads(); }
  if(blockSize >= 128) { if(tid <  64) { tdata[tid] += tdata[tid+ 64]; } __syncthreads(); }
  if(tid < 32) { sumWarpReduce<blockSize>(sdata, tid); }
  if(tid == 0) { fieldOut.dData[blockIdx.x] = tdata[0]; }
}
template<unsigned int blockSize>
__global__ void fieldSumTex_k(CudaFieldTex fieldIn, CudaField<float> fieldOut, unsigned int n)
{
  extern __shared__ int sdata[];
  unsigned int tid      = threadIdx.x;
  unsigned int i        = blockIdx.x*(2*blockSize) + tid;
  unsigned int gridSize = blockSize*2*gridDim.x;

  float *tdata = (float*)sdata;
  tdata[tid]   = 0.0f;
  while(i < n)
    {
      tdata[tid] += length(float3{fieldIn.dData[i].x,           fieldIn.dData[i].y,           fieldIn.dData[i].z});
      tdata[tid] += length(float3{fieldIn.dData[i+blockSize].x, fieldIn.dData[i+blockSize].y, fieldIn.dData[i+blockSize].z});
      i += gridSize;
    }
  __syncthreads();
  if(blockSize >= 512) { if(tid < 256) { tdata[tid] += tdata[tid+256]; } __syncthreads(); }
  if(blockSize >= 256) { if(tid < 128) { tdata[tid] += tdata[tid+128]; } __syncthreads(); }
  if(blockSize >= 128) { if(tid <  64) { tdata[tid] += tdata[tid+ 64]; } __syncthreads(); }
  if(tid < 32) { sumWarpReduce<blockSize>(sdata, tid); }
  if(tid == 0) { fieldOut.dData[blockIdx.x] = tdata[0]; }
}

// normalize a field (returns average magnitude of values in field)
template<typename T>
float fieldNorm(CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst)
{
  if(!field || !dst) { std::cout << "====> ERROR(fieldNorm): field is null!\n"; return 0.0f; }
  // cast field to its underlying type and validate
  CudaField<T> *f  = reinterpret_cast<CudaField<T>*>(field);
  if(!f)             { std::cout << "====> ERROR(fieldNorm): field type cast error! ("   << f->type() << ")\n"; return 0.0f; }
  else if(!f->map()) { std::cout << "====> ERROR(fieldNorm): field texture map error! (" << f->type() << ")\n"; return 0.0f; }
  
  int threads = BLOCKDIM_X*BLOCKDIM_Y;
  int sz = field->size.x*field->size.y;

  dim3 dimBlock(threads, 1);
  dim3 dimGrid((int)ceil(sz/(float)(threads)), 1);
  size_t sharedMem = threads*sizeof(T);

  if(field->isTexture())
    {
      switch(threads)
        {
        case 512: fieldSumTex_k<512> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case 256: fieldSumTex_k<256> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case 128: fieldSumTex_k<128> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case 64:  fieldSumTex_k< 64> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case 32:  fieldSumTex_k< 32> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case 16:  fieldSumTex_k< 16> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case  8:  fieldSumTex_k<  8> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case  4:  fieldSumTex_k<  4> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case  2:  fieldSumTex_k<  2> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        case  1:  fieldSumTex_k<  1> <<< dimGrid, dimBlock, sharedMem >>>(*((CudaFieldTex*)f), *dst, sz); break;
        }
    }
  else
    {
      switch(threads)
        {
        case 512: fieldSum_k<T, 512> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case 256: fieldSum_k<T, 256> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case 128: fieldSum_k<T, 128> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case 64:  fieldSum_k<T,  64> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case 32:  fieldSum_k<T,  32> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case 16:  fieldSum_k<T,  16> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case  8:  fieldSum_k<T,   8> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case  4:  fieldSum_k<T,   4> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case  2:  fieldSum_k<T,   2> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        case  1:  fieldSum_k<T,   1> <<< dimGrid, dimBlock, sharedMem >>>(*f, *dst, sz); break;
        }
    }
  getLastCudaError("fieldSum_k failed!");

  dst->pullData();
  double sum = 0.0;
  if(dst->hData) { for(int i = 0; i < dimGrid.x; i++) { sum += (double)dst->hData[i]; } } 
  float avgLen = (float)(sum / (double)sz) * 20.0f; // average magnitude of values in field (NOTE: constant multiplier x20 needed???)

  if(avgLen != 0.0f) // can't normalize if zero average length (TODO: invalidate values?)
    {
      CudaField<T> *fo  = reinterpret_cast<CudaField<T>*>(fieldOut);
      if(!fo)             { std::cout << "====> ERROR(fieldNorm): fieldOut type cast error! ("   << fo->type() << ")\n"; f->unmap(); return 0.0f; }
      else if(!fo->map()) { std::cout << "====> ERROR(fieldNorm): fieldOut texture map error! (" << fo->type() << ")\n"; f->unmap(); return 0.0f; }
  
      dimBlock = dim3(BLOCKDIM_X, BLOCKDIM_Y);
      dimGrid  = dim3((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));

      if(field->isTexture() && fieldOut->isTexture())
        {
          CudaFieldTex *fT  = reinterpret_cast<CudaFieldTex*>(field);
          CudaFieldTex *foT = reinterpret_cast<CudaFieldTex*>(fieldOut);
          multTexC_k <<<dimGrid, dimBlock>>>(*fT, *foT, 1.0f/avgLen);
        }
      else
        { multC_k<T> <<<dimGrid, dimBlock>>>(*f, *fo, 1.0f/avgLen); }
      getLastCudaError("multC_k failed!");
      fo->unmap(); f->unmap();
    }
  return avgLen;
}

template float fieldNorm<int>    (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldNorm<int2>   (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldNorm<int3>   (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldNorm<int4>   (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldNorm<float>  (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldNorm<float2> (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldNorm<float3> (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldNorm<float4> (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldNorm<double> (CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldNorm<double2>(CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldNorm<double3>(CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template float fieldNorm<double4>(CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);


















//// CHANNELS ////



__global__ void combineChannels1_k(CudaField<float> srcX, CudaField<float> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < srcX.size.x && iy < srcX.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = srcX.dData[i];
    }
}

__global__ void combineChannels2_k(CudaField<float> srcX, CudaField<float> srcY, CudaField<float2> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < srcX.size.x && iy < srcX.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = float2{srcX.dData[i], srcY.dData[i]};
    }
}

__global__ void combineChannels3_k(CudaField<float> srcX, CudaField<float> srcY, CudaField<float> srcZ, CudaField<float3> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < srcX.size.x && iy < srcX.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = float3{srcX.dData[i], srcY.dData[i], srcZ.dData[i]};
    }
}

__global__ void combineChannels4_k(CudaField<float> srcX, CudaField<float> srcY, CudaField<float> srcZ, CudaField<float> srcW, CudaField<float4> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < srcX.size.x && iy < srcX.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = float4{srcX.dData[i], srcY.dData[i], srcZ.dData[i], srcW.dData[i]};
    }
}


void combineChannels(CudaFieldBase *fieldX, CudaFieldBase *dst)
{
  dim3 block(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(dst->size.x/(float)BLOCKDIM_X), (int)ceil(dst->size.y/(float)BLOCKDIM_Y));
  
  if(!fieldX || !dst) { std::cout << "====> ERROR(combineChannels): field is null!\n"; return; }

  // cast field to its underlying type and validate
  CudaField<float> *fX  = reinterpret_cast<CudaField<float>*>(fieldX);
  if(!fX)             { std::cout << "====> ERROR(combineChannels): fieldX type cast error! ("   << fX->type() << ")\n"; return; }
  else if(!fX->map()) { std::cout << "====> ERROR(combineChannels): fieldX texture map error! (" << fX->type() << ")\n"; return; }
  // cast field to its underlying type and validate
  CudaField<float> *d  = reinterpret_cast<CudaField<float>*>(dst);
  if(!d)             { std::cout << "====> ERROR(combineChannels): dst type cast error! ("   << d->type() << ")\n"; return; }
  else if(!d->map()) { std::cout << "====> ERROR(combineChannels): dst texture map error! (" << d->type() << ")\n"; return; }

  combineChannels1_k<<<grid, block>>>(*fX, *d);
  cudaDeviceSynchronize();
  getLastCudaError("combineChannels1_k failed!");
  fX->unmap(); d->unmap();
}

void combineChannels(CudaFieldBase *fieldX, CudaFieldBase *fieldY, CudaFieldBase *dst)
{
  dim3 block(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(dst->size.x/(float)BLOCKDIM_X), (int)ceil(dst->size.y/(float)BLOCKDIM_Y));
  if(!fieldX || !fieldY || !dst) { std::cout << "====> ERROR(combineChannels): field is null!\n"; return; }

  // cast field to its underlying type and validate
  CudaField<float> *fX  = reinterpret_cast<CudaField<float>*>(fieldX);
  if(!fX)             { std::cout << "====> ERROR(combineChannels): fieldX type cast error! ("   << fX->type() << ")\n"; return; }
  else if(!fX->map()) { std::cout << "====> ERROR(combineChannels): fieldX texture map error! (" << fX->type() << ")\n"; return; }
  CudaField<float> *fY  = reinterpret_cast<CudaField<float>*>(fieldY);
  if(!fY)             { std::cout << "====> ERROR(combineChannels): fieldY type cast error! ("   << fY->type() << ")\n"; return; }
  else if(!fY->map()) { std::cout << "====> ERROR(combineChannels): fieldY texture map error! (" << fY->type() << ")\n"; return; }
  CudaField<float2> *d  = reinterpret_cast<CudaField<float2>*>(dst);
  if(!d)             { std::cout << "====> ERROR(combineChannels): dst type cast error! ("   << d->type() << ")\n"; return; }
  else if(!d->map()) { std::cout << "====> ERROR(combineChannels): dst texture map error! (" << d->type() << ")\n"; return; }

  combineChannels2_k<<<grid, block>>>(*fX, *fY, *d);
  cudaDeviceSynchronize();
  getLastCudaError("combineChannels2_k failed!");
  fX->unmap(); fY->unmap(); d->unmap();
}

void combineChannels(CudaFieldBase *fieldX, CudaFieldBase *fieldY, CudaFieldBase *fieldZ, CudaFieldBase *dst)
{
  dim3 block(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(dst->size.x/(float)BLOCKDIM_X), (int)ceil(dst->size.y/(float)BLOCKDIM_Y));
  if(!fieldX || !fieldY || !fieldZ || !dst) { std::cout << "====> ERROR(combineChannels): field is null!\n"; return; }
  CudaField<float> *fX  = reinterpret_cast<CudaField<float>*>(fieldX);
  if(!fX)             { std::cout << "====> ERROR(combineChannels): fieldX type cast error! ("   << fX->type() << ")\n"; return; }
  else if(!fX->map()) { std::cout << "====> ERROR(combineChannels): fieldX texture map error! (" << fX->type() << ")\n"; return; }
  CudaField<float> *fY  = reinterpret_cast<CudaField<float>*>(fieldY);
  if(!fY)             { std::cout << "====> ERROR(combineChannels): fieldY type cast error! ("   << fY->type() << ")\n"; return; }
  else if(!fY->map()) { std::cout << "====> ERROR(combineChannels): fieldY texture map error! (" << fY->type() << ")\n"; return; }
  CudaField<float> *fZ  = reinterpret_cast<CudaField<float>*>(fieldZ);
  if(!fZ)             { std::cout << "====> ERROR(combineChannels): fieldZ type cast error! ("   << fZ->type() << ")\n"; return; }
  else if(!fZ->map()) { std::cout << "====> ERROR(combineChannels): fieldZ texture map error! (" << fZ->type() << ")\n"; return; }
  CudaField<float3> *d  = reinterpret_cast<CudaField<float3>*>(dst);
  if(!d)             { std::cout << "====> ERROR(combineChannels): dst type cast error! ("   << d->type() << ")\n"; return; }
  else if(!d->map()) { std::cout << "====> ERROR(combineChannels): dst texture map error! (" << d->type() << ")\n"; return; }

  combineChannels3_k<<<grid, block>>>(*fX, *fY, *fZ, *d);
  cudaDeviceSynchronize();
  getLastCudaError("combineChannels3_k failed!");
  fX->unmap(); fY->unmap(); fZ->unmap(); d->unmap();
}

void combineChannels(CudaFieldBase *fieldX, CudaFieldBase *fieldY, CudaFieldBase *fieldZ, CudaFieldBase *fieldW, CudaFieldBase *dst)
{
  dim3 block(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(dst->size.x/(float)BLOCKDIM_X), (int)ceil(dst->size.y/(float)BLOCKDIM_Y));
  if(!fieldX || !fieldY || !fieldZ || !fieldW || !dst) { std::cout << "====> ERROR(combineChannels): field is null!\n"; return; }
  CudaField<float> *fX  = reinterpret_cast<CudaField<float>*>(fieldX);
  if(!fX)             { std::cout << "====> ERROR(combineChannels): fieldX type cast error! ("   << fX->type() << ")\n"; return; }
  else if(!fX->map()) { std::cout << "====> ERROR(combineChannels): fieldX texture map error! (" << fX->type() << ")\n"; return; }
  CudaField<float> *fY  = reinterpret_cast<CudaField<float>*>(fieldY);
  if(!fY)             { std::cout << "====> ERROR(combineChannels): fieldX type cast error! ("   << fY->type() << ")\n"; return; }
  else if(!fY->map()) { std::cout << "====> ERROR(combineChannels): fieldX texture map error! (" << fY->type() << ")\n"; return; }
  CudaField<float> *fZ  = reinterpret_cast<CudaField<float>*>(fieldZ);
  if(!fZ)             { std::cout << "====> ERROR(combineChannels): fieldX type cast error! ("   << fZ->type() << ")\n"; return; }
  else if(!fZ->map()) { std::cout << "====> ERROR(combineChannels): fieldX texture map error! (" << fZ->type() << ")\n"; return; }
  CudaField<float> *fW  = reinterpret_cast<CudaField<float>*>(fieldW);
  if(!fW)             { std::cout << "====> ERROR(combineChannels): fieldX type cast error! ("   << fW->type() << ")\n"; return; }
  else if(!fW->map()) { std::cout << "====> ERROR(combineChannels): fieldX texture map error! (" << fW->type() << ")\n"; return; }
  CudaField<float4> *d  = reinterpret_cast<CudaField<float4>*>(dst);
  if(!d)             { std::cout << "====> ERROR(combineChannels): dst type cast error! ("   << d->type() << ")\n"; return; }
  else if(!d->map()) { std::cout << "====> ERROR(combineChannels): dst texture map error! (" << d->type() << ")\n"; return; }

  combineChannels4_k<<<grid, block>>>(*fX, *fY, *fZ, *fW, *d);
  cudaDeviceSynchronize();
  getLastCudaError("combineChannels4_k failed!");
  fX->unmap(); fY->unmap(); fZ->unmap(); fW->unmap(); d->unmap();
}












__global__ void splitChannels1_k(CudaField<float> src, CudaField<float> dstX)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y)
    {
      int i = iy*src.size.x + ix;
      dstX.dData[i] = src.dData[i];
    }
}

__global__ void splitChannels2_k(CudaField<float2> src, CudaField<float> dstX, CudaField<float> dstY)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y)
    {
      int i = iy*src.size.x + ix;
      dstX.dData[i] = src.dData[i].x;
      dstY.dData[i] = src.dData[i].y;
    }
}

__global__ void splitChannels3_k(CudaField<float3> src, CudaField<float> dstX, CudaField<float> dstY, CudaField<float> dstZ)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y)
    {
      int i = iy*src.size.x + ix;
      dstX.dData[i] = src.dData[i].x;
      dstY.dData[i] = src.dData[i].y;
      dstZ.dData[i] = src.dData[i].z;
    }
}

__global__ void splitChannels4_k(CudaField<float4> src, CudaField<float> dstX, CudaField<float> dstY, CudaField<float> dstZ, CudaField<float> dstW)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y)
    {
      int i = iy*src.size.x + ix;
      dstX.dData[i] = src.dData[i].x;
      dstY.dData[i] = src.dData[i].y;
      dstZ.dData[i] = src.dData[i].z;
      dstW.dData[i] = src.dData[i].w;
    }
}


void splitChannels(CudaFieldBase *field, CudaFieldBase *dstX)
{
  dim3 block(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));
  
  if(!field || !dstX) { std::cout << "====> ERROR(splitChannels): field is null!\n"; return; }

  // cast field to its underlying type and validate
  CudaField<float> *f  = reinterpret_cast<CudaField<float>*>(field);
  if(!f)             { std::cout << "====> ERROR(splitChannels): field type cast error! ("   << f->type() << ")\n"; return; }
  else if(!f->map()) { std::cout << "====> ERROR(splitChannels): field texture map error! (" << f->type() << ")\n"; return; }
  CudaField<float> *dX  = reinterpret_cast<CudaField<float>*>(dstX);
  if(!dX)             { std::cout << "====> ERROR(splitChannels): dstX type cast error! ("   << dX->type() << ")\n"; return; }
  else if(!dX->map()) { std::cout << "====> ERROR(splitChannels): dstX texture map error! (" << dX->type() << ")\n"; return; }

  splitChannels1_k<<<grid, block>>>(*f, *dX);
  cudaDeviceSynchronize();
  getLastCudaError("splitChannels1_k failed!");
  f->unmap(); dX->unmap();
}

void splitChannels(CudaFieldBase *field, CudaFieldBase *dstX, CudaFieldBase *dstY)
{
  dim3 block(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));
  if(!field || !dstX || !dstY) { std::cout << "====> ERROR(splitChannels): field is null!\n"; return; }

  // cast field to its underlying type and validate
  CudaField<float2> *f  = reinterpret_cast<CudaField<float2>*>(field);
  if(!f)             { std::cout << "====> ERROR(splitChannels): field type cast error! ("   << f->type() << ")\n"; return; }
  else if(!f->map()) { std::cout << "====> ERROR(splitChannels): field texture map error! (" << f->type() << ")\n"; return; }
  CudaField<float> *dX  = reinterpret_cast<CudaField<float>*>(dstX);
  if(!dX)             { std::cout << "====> ERROR(splitChannels): dstX type cast error! ("   << dX->type() << ")\n"; return; }
  else if(!dX->map()) { std::cout << "====> ERROR(splitChannels): dstX texture map error! (" << dX->type() << ")\n"; return; }
  CudaField<float> *dY  = reinterpret_cast<CudaField<float>*>(dstY);
  if(!dY)             { std::cout << "====> ERROR(splitChannels): dstY type cast error! ("   << dY->type() << ")\n"; return; }
  else if(!dY->map()) { std::cout << "====> ERROR(splitChannels): dstY texture map error! (" << dY->type() << ")\n"; return; }

  splitChannels2_k<<<grid, block>>>(*f, *dX, *dY);
  cudaDeviceSynchronize();
  getLastCudaError("splitChannels2_k failed!");
  f->unmap(); dX->unmap(); dY->unmap();
}

void splitChannels(CudaFieldBase *field, CudaFieldBase *dstX, CudaFieldBase *dstY, CudaFieldBase *dstZ)
{
  dim3 block(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));
  if(!field || !dstX || !dstY || !dstZ) { std::cout << "====> ERROR(splitChannels): field is null!\n"; return; }
  CudaField<float3> *f  = reinterpret_cast<CudaField<float3>*>(field);
  if(!f)             { std::cout << "====> ERROR(splitChannels): field type cast error! ("   << f->type() << ")\n"; return; }
  else if(!f->map()) { std::cout << "====> ERROR(splitChannels): field texture map error! (" << f->type() << ")\n"; return; }
  CudaField<float> *dX  = reinterpret_cast<CudaField<float>*>(dstX);
  if(!dX)             { std::cout << "====> ERROR(splitChannels): dstX type cast error! ("   << dX->type() << ")\n"; return; }
  else if(!dX->map()) { std::cout << "====> ERROR(splitChannels): dstX texture map error! (" << dX->type() << ")\n"; return; }
  CudaField<float> *dY  = reinterpret_cast<CudaField<float>*>(dstY);
  if(!dY)             { std::cout << "====> ERROR(splitChannels): dstY type cast error! ("   << dY->type() << ")\n"; return; }
  else if(!dY->map()) { std::cout << "====> ERROR(splitChannels): dstY texture map error! (" << dY->type() << ")\n"; return; }
  CudaField<float> *dZ  = reinterpret_cast<CudaField<float>*>(dstZ);
  if(!dZ)             { std::cout << "====> ERROR(splitChannels): dstZ type cast error! ("   << dZ->type() << ")\n"; return; }
  else if(!dZ->map()) { std::cout << "====> ERROR(splitChannels): dstZ texture map error! (" << dZ->type() << ")\n"; return; }

  splitChannels3_k<<<grid, block>>>(*f, *dX, *dY, *dZ);
  cudaDeviceSynchronize();
  getLastCudaError("splitChannels3_k failed!");
  f->unmap(); dX->unmap(); dY->unmap(); dZ->unmap();
}

void splitChannels(CudaFieldBase *field, CudaFieldBase *dstX, CudaFieldBase *dstY, CudaFieldBase *dstZ, CudaFieldBase *dstW)
{
  dim3 block(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));
  if(!field || !dstX || !dstY || !dstZ || !dstW) { std::cout << "====> ERROR(splitChannels): field is null!\n"; return; }
  CudaField<float4> *f  = reinterpret_cast<CudaField<float4>*>(field);
  if(!f)             { std::cout << "====> ERROR(splitChannels): field type cast error! ("   << f->type() << ")\n"; return; }
  else if(!f->map()) { std::cout << "====> ERROR(splitChannels): field texture map error! (" << f->type() << ")\n"; return; }
  CudaField<float> *dX  = reinterpret_cast<CudaField<float>*>(dstX);
  if(!dX)             { std::cout << "====> ERROR(splitChannels): dstX type cast error! ("   << dX->type() << ")\n"; return; }
  else if(!dX->map()) { std::cout << "====> ERROR(splitChannels): dstX texture map error! (" << dX->type() << ")\n"; return; }
  CudaField<float> *dY  = reinterpret_cast<CudaField<float>*>(dstY);
  if(!dY)             { std::cout << "====> ERROR(splitChannels): dstX type cast error! ("   << dY->type() << ")\n"; return; }
  else if(!dY->map()) { std::cout << "====> ERROR(splitChannels): dstX texture map error! (" << dY->type() << ")\n"; return; }
  CudaField<float> *dZ  = reinterpret_cast<CudaField<float>*>(dstZ);
  if(!dZ)             { std::cout << "====> ERROR(splitChannels): dstX type cast error! ("   << dZ->type() << ")\n"; return; }
  else if(!dZ->map()) { std::cout << "====> ERROR(splitChannels): dstX texture map error! (" << dZ->type() << ")\n"; return; }
  CudaField<float> *dW  = reinterpret_cast<CudaField<float>*>(dstW);
  if(!dW)             { std::cout << "====> ERROR(splitChannels): dstX type cast error! ("   << dW->type() << ")\n"; return; }
  else if(!dW->map()) { std::cout << "====> ERROR(splitChannels): dstX texture map error! (" << dW->type() << ")\n"; return; }

  splitChannels4_k<<<grid, block>>>(*f, *dX, *dY, *dZ, *dW);
  cudaDeviceSynchronize();
  getLastCudaError("splitChannels4_k failed!");
  f->unmap(); dX->unmap(); dY->unmap(); dZ->unmap(); dW->unmap();
}
