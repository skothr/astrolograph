
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
//// MULTIPLICATION
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// multiply two fields
template<typename T>
__global__ void fieldMult_k(CudaField<T> field1, CudaField<T> field2, CudaField<T> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < field1.size.x && iy < field1.size.y && ix < field2.size.x && iy < field2.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      T val = field1.dData[i]*field2.dData[i];
      dst.dData[i] = val;
    }
}

template<typename T>
void fieldMult(CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst)
{
  if(!field1 || !field2 || !dst) { std::cout << "====> ERROR(fieldMult): field is null!\n"; return; }
  // cast field1 to its underlying type and validate
  CudaField<T> *f1  = reinterpret_cast<CudaField<T>*>(field1);
  if(!f1)             { std::cout << "====> ERROR(fieldMult): field1 type cast error! ("   << f1->type() << ")\n"; return; }
  else if(!f1->map()) { std::cout << "====> ERROR(fieldMult): field1 texture map error! (" << f1->type() << ")\n"; return; }
  // cast field2 to its underlying type and validate
  CudaField<T> *f2  = reinterpret_cast<CudaField<T>*>(field2);
  if(!f2)             { std::cout << "====> ERROR(fieldMult): field2 type cast error! ("   << f2->type() << ")\n"; return; }
  else if(!f2->map()) { std::cout << "====> ERROR(fieldMult): field2 texture map error! (" << f2->type() << ")\n"; return; }
  // cast dst to its underlying type and validate
  CudaField<T> *d  = reinterpret_cast<CudaField<T>*>(dst);
  if(!d)              { std::cout << "====> ERROR(fieldMult): dst type cast error! ("   << d->type() << ")\n"; return; }
  else if(!d->map())  { std::cout << "====> ERROR(fieldMult): dst texture map error! (" << d->type() << ")\n"; return; }

  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field1->size.x/(float)BLOCKDIM_X), (int)ceil(field1->size.y/(float)BLOCKDIM_Y));
  fieldMult_k<T> <<<grid, threads>>> (*f1, *f2, *d);
  getLastCudaError("fieldMult_k failed!");
  f1->unmap(); f2->unmap(); d->unmap();
}

// multiply field by a constant
template<typename T>
__global__ void fieldMultC_k(CudaField<T> field1, CudaField<T> dst, T constant)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  if(ix < field1.size.x && iy < field1.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      T val = field1.dData[i]*constant;
      dst.dData[i] = val;
    }
}

template<typename T>
void fieldMultC(CudaFieldBase *field, CudaFieldBase *dst, T constant)
{
  if(!field || !dst)  { std::cout << "====> ERROR(fieldMultC): field is null!\n"; return; }
  // cast field to its underlying type and validate
  CudaField<T> *f  = reinterpret_cast<CudaField<T>*>(field);
  if(!f)              { std::cout << "====> ERROR(fieldMultC): field type cast error! ("   << f->type() << ")\n"; return; }
  else if(!f->map())  { std::cout << "====> ERROR(fieldMultC): field texture map error! (" << f->type() << ")\n"; return; }
  // cast dst to its underlying type and validate
  CudaField<T> *d  = reinterpret_cast<CudaField<T>*>(dst);
  if(!d)              { std::cout << "====> ERROR(fieldMultC): dst type cast error! ("   << d->type() << ")\n"; return; }
  else if(!d->map())  { std::cout << "====> ERROR(fieldMultC): dst texture map error! (" << d->type() << ")\n"; return; }
  
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field->size.x/(float)BLOCKDIM_X), (int)ceil(field->size.y/(float)BLOCKDIM_Y));
  fieldMultC_k<T> <<<grid, threads>>> (*f, *d, constant);
  getLastCudaError("fieldMultC_k failed!");
  f->unmap(); d->unmap();
}

// instantiation
template void fieldMult<int>     (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldMult<int2>    (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldMult<int3>    (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldMult<int4>    (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldMult<float>   (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldMult<float2>  (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldMult<float3>  (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldMult<float4>  (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldMult<double>  (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldMult<double2> (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldMult<double3> (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldMult<double4> (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
template void fieldMultC<int>    (CudaFieldBase *field,  CudaFieldBase *dst,    int     constant);
template void fieldMultC<int2>   (CudaFieldBase *field,  CudaFieldBase *dst,    int2    constant);
template void fieldMultC<int3>   (CudaFieldBase *field,  CudaFieldBase *dst,    int3    constant);
template void fieldMultC<int4>   (CudaFieldBase *field,  CudaFieldBase *dst,    int4    constant);
template void fieldMultC<float>  (CudaFieldBase *field,  CudaFieldBase *dst,    float   constant);
template void fieldMultC<float2> (CudaFieldBase *field,  CudaFieldBase *dst,    float2  constant);
template void fieldMultC<float3> (CudaFieldBase *field,  CudaFieldBase *dst,    float3  constant);
template void fieldMultC<float4> (CudaFieldBase *field,  CudaFieldBase *dst,    float4  constant);
template void fieldMultC<double> (CudaFieldBase *field,  CudaFieldBase *dst,    double  constant);
template void fieldMultC<double2>(CudaFieldBase *field,  CudaFieldBase *dst,    double2 constant);
template void fieldMultC<double3>(CudaFieldBase *field,  CudaFieldBase *dst,    double3 constant);
template void fieldMultC<double4>(CudaFieldBase *field,  CudaFieldBase *dst,    double4 constant);
