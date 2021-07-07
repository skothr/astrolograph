
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
//// NEGATE FIELD
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// negate two fields
template<typename T>
__global__ void fieldNegate_k(CudaField<T> src, CudaField<T> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = (0 - src.dData[i]); // type-independant negation --> (0 - value)
    }
}
template<typename T>
void fieldNegate(CudaFieldBase *field, CudaFieldBase *dst)
{
  if(!field || !dst) { std::cout << "====> ERROR(fieldNegate): field is null!\n"; return; }
  // cast field1 to its underlying type and validate
  CudaField<T> *f  = reinterpret_cast<CudaField<T>*>(field);
  if(!f)             { std::cout << "====> ERROR(fieldNegate): field type cast error! ("   << f->type() << ")\n"; return; }
  else if(!f->map()) { std::cout << "====> ERROR(fieldNegate): field texture map error! (" << f->type() << ")\n"; return; }
  // cast dst to its underlying type and validate
  CudaField<T> *d = reinterpret_cast<CudaField<T>*>(dst);
  if(!d)             { std::cout << "====> ERROR(fieldNegate): dst type error! ("       << d->typeStr() << ")\n"; return; }
  else if(!d->map()) { std::cout << "====> ERROR(fieldNegate): dst texture map error (" << d->typeStr() << ")\n"; f->unmap(); return; }

  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));  
  fieldNegate_k<T> <<<grid, threads>>> (*f, *d);
  getLastCudaError("fieldNegate_k failed!");
  f->unmap(); d->unmap();
}

template void fieldNegate<int>     (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldNegate<int2>    (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldNegate<int3>    (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldNegate<int4>    (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldNegate<float>   (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldNegate<float2>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldNegate<float3>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldNegate<float4>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldNegate<double>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldNegate<double2> (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldNegate<double3> (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldNegate<double4> (CudaFieldBase *src, CudaFieldBase *dst);
