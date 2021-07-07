
#include "cudaField.hpp"
#include "cutools.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>
#include "vector-operators.h"

#define BLOCKDIM_X 16
#define BLOCKDIM_Y 16

// shifts 2D FFT to be centered around zero
__global__ void FFTShift_k(CudaField<float2> src, CudaField<float2> dst, int2 odd, bool shift, bool scale)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y)
    {
      int i = iy*src.size.x + ix;
      int xw = shift ? (((ix >= src.size.x/2) ? (ix - src.size.x/2) : ix + src.size.x/2) + odd.x) : ix;
      int yw = shift ? (((iy >= src.size.y/2) ? (iy - src.size.y/2) : iy + src.size.y/2) + odd.y) : iy;
      if(xw < src.size.x && yw < src.size.y) { dst.dData[i] = src.dData[yw*src.size.x + xw] / (scale ? (float(src.size.x+src.size.y)) : 1.0f); }
    }
}
// shifts 2D FFT to be centered around zero
extern "C" void fftShift(CudaField<float2> src, CudaField<float2> dst, bool shift, bool scale)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size.x == src.size.x && dst.size.y == src.size.y)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      FFTShift_k<<<grid, threads>>>(src, dst, int2{(src.size.x % 2 == 0 ? 0 : 1), (src.size.y % 2 == 0 ? 0 : 1)}, shift, scale);
      getLastCudaError("====> ERROR: FFTShift_k failed!");
    }
}
