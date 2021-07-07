#ifndef FIELD_OPERATORS_CUH
#define FIELD_OPERATORS_CUH

#include <cuda_runtime.h>

// #define M_E 2.7182818284590452353602874713526624977572470937

// ABSOLUTE VALUE //
template<typename T> __global__ void fieldAbs_k (CudaField<T> src, CudaField<T> dst);

// LOG //
template<typename T> __global__ void fieldLog_k (CudaField<T> src, CudaField<T> dst, double base);
// EXP //
template<typename T> __global__ void fieldExp_k (CudaField<T> src, CudaField<T> dst);


// ADDITION //
// add field
template<typename T> __global__ void fieldAdd_k (CudaField<T> field1, CudaField<T> field2, CudaField<T> dst);
// add constant
template<typename T> __global__ void fieldAddC_k(CudaField<T> field1, CudaField<T> dst, T constant);



// MULTIPLICATION //
// multiply by field
template<typename T> __global__ void fieldMult_k (CudaField<T> field1, CudaField<T> field2, CudaField<T> dst);
// multiply by constant
template<typename T> __global__ void fieldMultC_k(CudaField<T> field1, CudaField<T> dst, T constant);




// GRADIENT //
// X/Y gradient of field
template<typename T> __global__ void fieldGradient_k (CudaField<T> src, CudaField<T> dstX, CudaField<T> dstY);

// DIVERGENCE //
// divergence of field
template<typename T> __global__ void fieldDivergence_k (CudaField<T> src, CudaField<float> dst);

// CURL //
// curl of field
template<typename T> __global__ void fieldCurl_k (CudaField<T> src, CudaField<float> dst);



#endif // FIELD_OPERATORS_CUH
