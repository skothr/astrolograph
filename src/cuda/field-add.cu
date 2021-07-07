
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
//// ADDITION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// add two fields
template<typename T>
__global__ void fieldAdd_k(CudaField<T> field1, CudaField<T> field2, CudaField<T> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < field1.size.x && iy < field1.size.y && ix < field2.size.x && iy < field2.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      T val = field1.dData[i] + field2.dData[i];
      dst.dData[i] = val;
    }
}

template<typename T>
void fieldAdd(CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst)
{
  if(!field1 || !field2 || !dst)
    {
      std::cout << "====> ERROR(fieldAddC): Null argument(s)!  --> "
                << (field1 ? "" : "field1") << " | " << (field2 ? "" : "field2") << " | " << (dst ? "" : "dst") << "\n";
      return;
    }
  if(!field1 || !field2 || !dst) { std::cout << "====> ERROR(fieldAdd): field is null!\n"; return; }
  // cast field1 to its underlying type and validate
  CudaField<T> *f1  = reinterpret_cast<CudaField<T>*>(field1);
  if(!f1)             { std::cout << "====> ERROR(fieldAdd): field1 type cast error! ("   << f1->type() << ")\n"; return; }
  else if(!f1->map()) { std::cout << "====> ERROR(fieldAdd): field1 texture map error! (" << f1->type() << ")\n"; return; }
  // cast field2 to its underlying type and validate
  CudaField<T> *f2  = reinterpret_cast<CudaField<T>*>(field2);
  if(!f2)             { std::cout << "====> ERROR(fieldAdd): field2 type cast error! ("   << f2->type() << ")\n"; return; }
  else if(!f2->map()) { std::cout << "====> ERROR(fieldAdd): field2 texture map error! (" << f2->type() << ")\n"; return; }
  // cast dst to its underlying type and validate
  CudaField<T> *d  = reinterpret_cast<CudaField<T>*>(dst);
  if(!d)             { std::cout << "====> ERROR(fieldAdd): dst type cast error! ("   << d->type() << ")\n"; return; }
  else if(!d->map()) { std::cout << "====> ERROR(fieldAdd): dst texture map error! (" << d->type() << ")\n"; return; }

  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field1->size.x/(float)BLOCKDIM_X), (int)ceil(field1->size.y/(float)BLOCKDIM_Y));  
  fieldAdd_k<T> <<<grid, threads>>> (*f1, *f2, *d);
  getLastCudaError("fieldAdd_k failed!");
  f1->unmap(); f2->unmap(); d->unmap();
}

// add field by a constant
template<typename T>
__global__ void fieldAddC_k(CudaField<T> field1, CudaField<T> dst, T offset)//, bool isTex)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  if(ix < field1.size.x && iy < field1.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      T val = field1.dData[i] + offset;
      dst.dData[i] = val;
    }
}

template<typename T>
void fieldAddC(CudaFieldBase *field1, CudaFieldBase *dst, T offset)
{
  if(!field1 || !dst)
    {
      std::cout << "====> ERROR(fieldAddC): Null argument(s)!  --> "
                << (field1 ? "" : "field1") << (!field1 && !dst ? " / " : "  ") << (dst ? "" : "dst") << "\n";
      return;
    }
  
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field1->size.x/(float)BLOCKDIM_X), (int)ceil(field1->size.y/(float)BLOCKDIM_Y));

  // check data sizes
  if(field1->size.x != dst->size.x || field1->size.y != dst->size.y || field1->dataSize != dst->dataSize)
    { std::cout << "====> ERROR(fieldAddC): field1 and dst dataSizes do not match! (field1: " << field1->type() << " / dst: " << dst->type() << ")\n"; return; }
  
  // cast field1 to its underlying type and validate
  CudaField<T> *f1  = reinterpret_cast<CudaField<T>*>(field1);
  if(!f1)             { std::cout << "====> ERROR(fieldAddC): field1 type cast error! ("   << f1->type() << ")\n"; return; }
  else if(!f1->map()) { std::cout << "====> ERROR(fieldAddC): field1 texture map error! (" << f1->type() << ")\n"; return; }
  // cast dst to its underlying type and validate
  CudaField<T> *d = reinterpret_cast<CudaField<T>*>(dst);
  if(!d)             { std::cout << "====> ERROR(fieldAddC): dst type error! ("       << d->typeStr() << ")\n"; return; }
  else if(!d->map()) { std::cout << "====> ERROR(fieldAddC): dst texture map error (" << d->typeStr() << ")\n"; f1->unmap(); return; }
  
  fieldAddC_k<T> <<<grid, threads>>> (*f1, *d, offset);
  getLastCudaError("fieldAddC_k failed!");
  
  f1->unmap(); d->unmap();
}

template void fieldAdd<int>     (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldAdd<int2>    (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldAdd<int3>    (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldAdd<int4>    (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldAdd<float>   (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldAdd<float2>  (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldAdd<float3>  (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldAdd<float4>  (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldAdd<double>  (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldAdd<double2> (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldAdd<double3> (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldAdd<double4> (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldAddC<int>    (CudaFieldBase *field1, CudaFieldBase *dst, int     offset);
template void fieldAddC<int2>   (CudaFieldBase *field1, CudaFieldBase *dst, int2    offset);
template void fieldAddC<int3>   (CudaFieldBase *field1, CudaFieldBase *dst, int3    offset);
template void fieldAddC<int4>   (CudaFieldBase *field1, CudaFieldBase *dst, int4    offset);
template void fieldAddC<float>  (CudaFieldBase *field1, CudaFieldBase *dst, float   offset);
template void fieldAddC<float2> (CudaFieldBase *field1, CudaFieldBase *dst, float2  offset);
template void fieldAddC<float3> (CudaFieldBase *field1, CudaFieldBase *dst, float3  offset);
template void fieldAddC<float4> (CudaFieldBase *field1, CudaFieldBase *dst, float4  offset);
template void fieldAddC<double> (CudaFieldBase *field1, CudaFieldBase *dst, double  offset);
template void fieldAddC<double2>(CudaFieldBase *field1, CudaFieldBase *dst, double2 offset);
template void fieldAddC<double3>(CudaFieldBase *field1, CudaFieldBase *dst, double3 offset);
template void fieldAddC<double4>(CudaFieldBase *field1, CudaFieldBase *dst, double4 offset);
