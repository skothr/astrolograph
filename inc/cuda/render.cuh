#ifndef RENDER_CUH
#define RENDER_CUH

#include <cuda_runtime.h>

template<typename T> __global__ void renderTex_k(CudaField<T>   field1, CudaFieldTex  tex);
template<typename T>            void renderTex  (CudaFieldBase *field1, CudaFieldTex *tex);


#endif // RENDER_CUH
