
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
__global__ void fieldGradient_k(CudaField<T> src, CudaField<T> dstX, CudaField<T> dstY)
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
      
      dstX.dData[i00] = ((src.dData[ip0] - src.dData[i00]) - (src.dData[i00] - src.dData[in0])) / 2;
      dstY.dData[i00] = ((src.dData[i0p] - src.dData[i00]) - (src.dData[i00] - src.dData[i0n])) / 2;
    }
}

template<typename T>
void fieldGradient(CudaFieldBase *field, CudaFieldBase *dstX, CudaFieldBase *dstY)
{
  if(!field || !dstX || !dstY) { std::cout << "====> ERROR(fieldGradient): field is null!\n"; return; }
  // cast field1 to its underlying type and validate
  CudaField<T> *f  = reinterpret_cast<CudaField<T>*>(field);
  if(!f)             { std::cout << "====> ERROR(fieldGradient): field type cast error! ("   << f->type() << ")\n"; return; }
  else if(!f->map()) { std::cout << "====> ERROR(fieldGradient): field texture map error! (" << f->type() << ")\n"; return; }
  // cast dstX to its underlying type and validate
  CudaField<T> *dx = reinterpret_cast<CudaField<T>*>(dstX);
  if(!dx)             { std::cout << "====> ERROR(fieldGradient): dstX type error! ("       << dx->typeStr() << ")\n"; f->unmap(); return; }
  else if(!dx->map()) { std::cout << "====> ERROR(fieldGradient): dstX texture map error (" << dx->typeStr() << ")\n"; f->unmap(); return; }
  // cast dstY to its underlying type and validate
  CudaField<T> *dy = reinterpret_cast<CudaField<T>*>(dstY);
  if(!dy)             { std::cout << "====> ERROR(fieldGradient): dstY type error! ("       << dy->typeStr() << ")\n"; f->unmap(); dx->unmap(); return; }
  else if(!dy->map()) { std::cout << "====> ERROR(fieldGradient): dstY texture map error (" << dy->typeStr() << ")\n"; f->unmap(); dx->unmap(); return; }

  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));  
  fieldGradient_k<T> <<<grid, threads>>> (*f, *dx, *dy);
  getLastCudaError("fieldGradient_k failed!");
  f->unmap(); dx->unmap(); dy->unmap();
}

template void fieldGradient<int>    (CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);
template void fieldGradient<int2>   (CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);
template void fieldGradient<int3>   (CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);
template void fieldGradient<int4>   (CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);
template void fieldGradient<float>  (CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);
template void fieldGradient<float2> (CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);
template void fieldGradient<float3> (CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);
template void fieldGradient<float4> (CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);
template void fieldGradient<double> (CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);
template void fieldGradient<double2>(CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);
template void fieldGradient<double3>(CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);
template void fieldGradient<double4>(CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);
