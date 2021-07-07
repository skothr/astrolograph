#ifndef FIELD_OPERATORS_H
#define FIELD_OPERATORS_H

#include "cudaField.hpp"
#include "cutools.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>

#include "vector-operators.h"

#define BLOCKDIM_X 16
#define BLOCKDIM_Y 16


// ADDITION //
// add two fields
template<typename T>__global__ void fieldAdd_k (CudaField<T>   field1, CudaField<T>   field2, CudaField<T>   dst);
template<typename T>           void fieldAdd   (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
// add constant to field
template<typename T>__global__ void fieldAddC_k(CudaField<T>   field1, CudaField<T>   dst, T constant);
template<typename T>           void fieldAddC  (CudaFieldBase *field1, CudaFieldBase *dst, T constant);


// NEGATION //
// negative of field values
template<typename T, unsigned int blockSize> __global__ void fieldNegate_k(CudaField<T> src, CudaField<T> dst, unsigned int n);
template<typename T> void fieldNegate(CudaFieldBase *src, CudaFieldBase *dst);

// ABSOLUTE VALUE //
template<typename T>__global__ void fieldAbs_k (CudaField<T>   src, CudaField<T>   dst);
template<typename T>           void fieldAbs   (CudaFieldBase *src, CudaFieldBase *dst);

// LOG //
template<typename T>           void fieldLog   (CudaFieldBase *src, CudaFieldBase *dst, double base=M_E);
// EXP //
template<typename T>           void fieldExp   (CudaFieldBase *src, CudaFieldBase *dst);



// MULTIPLICATION //
// multiply two fields
template<typename T> __global__ void fieldMult_k (CudaField<T>   field1, CudaField<T>   field2, CudaField<T>   dst);
template<typename T>            void fieldMult   (CudaFieldBase *field1, CudaFieldBase *field2, CudaFieldBase *dst);
// multiply field by constant
template<typename T> __global__ void fieldMultC_k(CudaField<T>   field1, CudaField<T>   dst, T constant);
template<typename T>            void fieldMultC  (CudaFieldBase *field1, CudaFieldBase *dst, T constant);


// GRADIENT //
// find gradient of field
template<typename T> __global__ void fieldGradient_k(CudaField<T>   src, CudaField<T>   dstX, CudaField<T>   dstY);
template<typename T>            void fieldGradient  (CudaFieldBase *src, CudaFieldBase *dstX, CudaFieldBase *dstY);


// // DIRECTIONAL DERIVATIVE //
// // find directional derivative of field
// template<typename T>__global__ void fieldDDerivative_k(CudaField<T>   src, CudaField<T>   dst);
// template<typename T>           void fieldDDerivative  (CudaFieldBase *src, CudaFieldBase *dst);

// DIVERGENCE //
// find divergence of field
template<typename T>__global__ void fieldDivergence_k(CudaField<T>   src, CudaField<float> dst);
template<typename T>           void fieldDivergence  (CudaFieldBase *src, CudaFieldBase   *dst);

// CURL //
// find curl of field
template<typename T>__global__ void fieldCurl_k(CudaField<T>   src, CudaField<float> dst);
template<typename T>           void fieldCurl  (CudaFieldBase *src, CudaFieldBase   *dst);


#endif // FIELD_OPERATORS_H
