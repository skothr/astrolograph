
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>

#include "vector-operators.h"
#include "field-operators.h"
#include "field-operators.cuh"

#include "cutools.hpp"
#include "cudaField.hpp"

#define BLOCKDIM_X 16
#define BLOCKDIM_Y 16


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// LOG
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
__global__ void fieldLog_k(CudaField<T> src, CudaField<T> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = neglog(src.dData[i]);
    }
}

template<typename T>
__global__ void fieldLogBase_k(CudaField<T> src, CudaField<T> dst, double base)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = neglog(src.dData[i]) / log(base); // change of base
    }
}

template<typename T>
void fieldLog(CudaFieldBase *field, CudaFieldBase *dst, double base)
{
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));
  if(!field || !dst) { std::cout << "====> ERROR(fieldLog): field is null!\n"; return; }
  // cast field1 to its underlying type and validate
  CudaField<T> *f  = reinterpret_cast<CudaField<T>*>(field);
  if(!f)             { std::cout << "====> ERROR(fieldLog): field type cast error! ("   << f->type() << ")\n"; return; }
  else if(!f->map()) { std::cout << "====> ERROR(fieldLog): field texture map error! (" << f->type() << ")\n"; return; }
  // cast dst to its underlying type and validate
  CudaField<T> *d = reinterpret_cast<CudaField<T>*>(dst);
  if(!d)             { std::cout << "====> ERROR(fieldLog): dst type error! ("       << d->typeStr() << ")\n"; return; }
  else if(!d->map()) { std::cout << "====> ERROR(fieldLog): dst texture map error (" << d->typeStr() << ")\n"; f->unmap(); return; }

  if(base == M_E)
    { fieldLog_k<T> <<<grid, threads>>> (*f, *d); }
  else
    { fieldLogBase_k<T> <<<grid, threads>>> (*f, *d, base); }
  getLastCudaError("fieldLog_k failed!");

  if(f) { f->unmap(); }
  if(d) { d->unmap(); }
}

template void fieldLog<int>     (CudaFieldBase *src, CudaFieldBase *dst, double base);
template void fieldLog<int2>    (CudaFieldBase *src, CudaFieldBase *dst, double base);
template void fieldLog<int3>    (CudaFieldBase *src, CudaFieldBase *dst, double base);
template void fieldLog<int4>    (CudaFieldBase *src, CudaFieldBase *dst, double base);
template void fieldLog<float>   (CudaFieldBase *src, CudaFieldBase *dst, double base);
template void fieldLog<float2>  (CudaFieldBase *src, CudaFieldBase *dst, double base);
template void fieldLog<float3>  (CudaFieldBase *src, CudaFieldBase *dst, double base);
template void fieldLog<float4>  (CudaFieldBase *src, CudaFieldBase *dst, double base);
template void fieldLog<double>  (CudaFieldBase *src, CudaFieldBase *dst, double base);
template void fieldLog<double2> (CudaFieldBase *src, CudaFieldBase *dst, double base);
template void fieldLog<double3> (CudaFieldBase *src, CudaFieldBase *dst, double base);
template void fieldLog<double4> (CudaFieldBase *src, CudaFieldBase *dst, double base);
