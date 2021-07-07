
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
//// EXP (e^x)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template<typename T>
__global__ void fieldExp_k(CudaField<T> src, CudaField<T> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      T d = src.dData[i];
      dst.dData[i] = exp(d);
    }
}

template<typename T>
void fieldExp(CudaFieldBase *field, CudaFieldBase *dst)
{
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));
  if(!field || !dst) { std::cout << "====> ERROR(fieldExp): field is null!\n"; return; }
  // cast field1 to its underlying type and validate
  CudaField<T> *f  = reinterpret_cast<CudaField<T>*>(field);
  if(!f)             { std::cout << "====> ERROR(fieldExp): field type cast error! ("   << f->type() << ")\n"; return; }
  else if(!f->map()) { std::cout << "====> ERROR(fieldExp): field texture map error! (" << f->type() << ")\n"; return; }
  // cast dst to its underlying type and validate
  CudaField<T> *d = reinterpret_cast<CudaField<T>*>(dst);
  if(!d)             { std::cout << "====> ERROR(fieldExp): dst type error! ("       << d->typeStr() << ")\n"; return; }
  else if(!d->map()) { std::cout << "====> ERROR(fieldExp): dst texture map error (" << d->typeStr() << ")\n"; f->unmap(); return; }

  fieldExp_k<T> <<<grid, threads>>> (*f, *d);
  getLastCudaError("fieldLog_k failed!");

  if(f) { f->unmap(); }
  if(d) { d->unmap(); }
}

template void fieldExp<int>     (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldExp<int2>    (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldExp<int3>    (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldExp<int4>    (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldExp<float>   (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldExp<float2>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldExp<float3>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldExp<float4>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldExp<double>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldExp<double2> (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldExp<double3> (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldExp<double4> (CudaFieldBase *src, CudaFieldBase *dst);
