
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
//// ABSOLUTE VALUE
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// abs two fields
template<typename T>
__global__ void fieldAbs_k(CudaField<T> src, CudaField<T> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = abs(src.dData[i]);
    }
}
template<typename T>
void fieldAbs(CudaFieldBase *field, CudaFieldBase *dst)
{
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));
  if(!field || !dst) { std::cout << "====> ERROR(fieldAbs): field is null!\n"; return; }
  // cast field1 to its underlying type and validate
  CudaField<T> *f  = reinterpret_cast<CudaField<T>*>(field);
  if(!f)             { std::cout << "====> ERROR(fieldAbs): field type cast error! ("   << f->type() << ")\n"; return; }
  else if(!f->map()) { std::cout << "====> ERROR(fieldAbs): field texture map error! (" << f->type() << ")\n"; return; }
  // cast dst to its underlying type and validate
  CudaField<T> *d = reinterpret_cast<CudaField<T>*>(dst);
  if(!d)             { std::cout << "====> ERROR(fieldAbs): dst type error! ("       << d->typeStr() << ")\n"; return; }
  else if(!d->map()) { std::cout << "====> ERROR(fieldAbs): dst texture map error (" << d->typeStr() << ")\n"; f->unmap(); return; }
  
  fieldAbs_k<T> <<<grid, threads>>> (*f, *d);
  getLastCudaError("fieldAbs_k failed!");

  if(f) { f->unmap(); }
  if(d) { d->unmap(); }
}

template void fieldAbs<int>     (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldAbs<int2>    (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldAbs<int3>    (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldAbs<int4>    (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldAbs<float>   (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldAbs<float2>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldAbs<float3>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldAbs<float4>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldAbs<double>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldAbs<double2> (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldAbs<double3> (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldAbs<double4> (CudaFieldBase *src, CudaFieldBase *dst);
