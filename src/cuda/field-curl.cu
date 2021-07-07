
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
//// CURL
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// curl two fields
template<typename T>
__global__ void fieldCurl_k(CudaField<T> src, CudaField<float> dst)
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

      float dYdX = ((src.dData[ip0].y - src.dData[i00].y) - (src.dData[i00].y - src.dData[in0].y)) / 2;
      float dXdY = ((src.dData[i0p].x - src.dData[i00].x) - (src.dData[i00].x - src.dData[i0n].x)) / 2;
      
      dst.dData[i00] = dYdX - dXdY; //((src.dData[i0p] - src.dData[i00]) - (src.dData[i00] - src.dData[i0n])) / 2;
      // dstY.dData[i00] = ((src.dData[i0p] - src.dData[i00]) - (src.dData[i00] - src.dData[i0n])) / 2;
    }
}

template<typename T>
void fieldCurl(CudaFieldBase *field, CudaFieldBase *dst)
{
  if(!field || !dst) { std::cout << "====> ERROR(fieldCurl): field is null!\n"; return; }
  // cast field1 to its underlying type and validate
  CudaField<T> *f  = reinterpret_cast<CudaField<T>*>(field);
  if(!f)             { std::cout << "====> ERROR(fieldCurl): field type cast error! ("   << f->type() << ")\n"; return; }
  else if(!f->map()) { std::cout << "====> ERROR(fieldCurl): field texture map error! (" << f->type() << ")\n"; return; }
  // cast dstX to its underlying type and validate
  CudaField<float> *d = reinterpret_cast<CudaField<float>*>(dst);
  if(!d)             { std::cout << "====> ERROR(fieldCurl): dst type error! ("       << d->typeStr() << ")\n"; f->unmap(); return; }
  else if(!d->map()) { std::cout << "====> ERROR(fieldCurl): dst texture map error (" << d->typeStr() << ")\n"; f->unmap(); return; }
  
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));
  fieldCurl_k<T> <<<grid, threads>>> (*f, *d);
  getLastCudaError("fieldCurl_k failed!");
  f->unmap(); d->unmap();
}

// template void fieldCurl<int>    (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldCurl<int2>   (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldCurl<int3>   (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldCurl<int4>   (CudaFieldBase *src, CudaFieldBase *dst);
// template void fieldCurl<float>  (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldCurl<float2> (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldCurl<float3> (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldCurl<float4> (CudaFieldBase *src, CudaFieldBase *dst);
// template void fieldCurl<double> (CudaFieldBase *src, CudaFieldBase *dst);
template void fieldCurl<double2>(CudaFieldBase *src, CudaFieldBase *dst);
template void fieldCurl<double3>(CudaFieldBase *src, CudaFieldBase *dst);
template void fieldCurl<double4>(CudaFieldBase *src, CudaFieldBase *dst);
