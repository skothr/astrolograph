
#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>

#include <type_traits>

#include "vector-operators.h"
#include "field-operators.h"
#include "field-operators.cuh"

#include "cutools.hpp"
#include "cudaField.hpp"


#define BLOCKDIM_X 16
#define BLOCKDIM_Y 16

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// GRADIENT
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// gradient two fields
template<typename T>
__global__ void fieldDivergence_k(CudaField<T> src, CudaField<float> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix > 0 && ix < src.size.x-1 && iy > 0 && iy < src.size.y-1)
    {
      int i00 = iy*src.size.x + ix;

      int ip0 = iy*src.size.x + ix + 1;
      int in0 = iy*src.size.x + ix - 1;
      
      // int i0p = (iy+1)*src.size.x + ix;
      // int i0n = (iy-1)*src.size.x + ix;
      
      dst.dData[i00] = ((src.dData[ip0] - src.dData[i00]) - (src.dData[i00] - src.dData[in0])) / 2;
    }
}

template<>
__global__ void fieldDivergence_k<float2>(CudaField<float2> src, CudaField<float> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix > 0 && ix < src.size.x-1 && iy > 0 && iy < src.size.y-1)
    {
      int i00 = iy*src.size.x + ix;

      int ip0 = iy*src.size.x + ix + 1;
      int in0 = iy*src.size.x + ix - 1;
      
      int i0p = (iy+1)*src.size.x + ix;
      int i0n = (iy-1)*src.size.x + ix;
      
      dst.dData[i00] = (((src.dData[ip0].x - src.dData[i00].x) - (src.dData[i00].x - src.dData[in0].x)) / 2
                        + ((src.dData[i0p].y - src.dData[i00].y) - (src.dData[i00].y - src.dData[i0n].y)) / 2);
    }
}

template<typename T>
void fieldDivergence(CudaFieldBase *field, CudaFieldBase *dst)
{
  if(!field || !dst) { std::cout << "====> ERROR(fieldDivergence): field is null!\n"; return; }
  // cast field1 to its underlying type and validate
  CudaField<T> *f  = reinterpret_cast<CudaField<T>*>(field);
  if(!f)             { std::cout << "====> ERROR(fieldDivergence): field type cast error! ("   << f->type() << ")\n"; return; }
  else if(!f->map()) { std::cout << "====> ERROR(fieldDivergence): field texture map error! (" << f->type() << ")\n"; return; }
  // cast dst to its underlying type and validate
  CudaField<float> *d = reinterpret_cast<CudaField<float>*>(dst);
  if(!d)             { std::cout << "====> ERROR(fieldDivergence): dst type error! ("       << d->typeStr() << ")\n"; f->unmap(); return; }
  else if(!d->map()) { std::cout << "====> ERROR(fieldDivergence): dst texture map error (" << d->typeStr() << ")\n"; f->unmap(); return; }
  
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));
  fieldDivergence_k<T> <<<grid, threads>>> (*f, *d);
  getLastCudaError("fieldDivergence_k failed!");
  f->unmap(); d->unmap();
}

template void fieldDivergence<int>    (CudaFieldBase *src, CudaFieldBase *dst);
// template void fieldDivergence<int2>   (CudaFieldBase *src, CudaFieldBase *dst);
// template void fieldDivergence<int3>   (CudaFieldBase *src, CudaFieldBase *dst);
// template void fieldDivergence<int4>   (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldDivergence<float>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldDivergence<float2> (CudaFieldBase *src, CudaFieldBase *dst);
// template void fieldDivergence<float3> (CudaFieldBase *src, CudaFieldBase *dst);
// template void fieldDivergence<float4> (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldDivergence<double> (CudaFieldBase *src, CudaFieldBase *dst);
// template void fieldDivergence<double2>(CudaFieldBase *src, CudaFieldBase *dst);
// template void fieldDivergence<double3>(CudaFieldBase *src, CudaFieldBase *dst);
// template void fieldDivergence<double4>(CudaFieldBase *src, CudaFieldBase *dst);
