#include "cudaField.hpp"
#include "cutools.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>
#include "vector-operators.h"

#define BLOCKDIM_X 16
#define BLOCKDIM_Y 16


// TODO
__device__ float mandelSmooth(const float2 &z) { return 0.0f; }

// MANDELBROT KERNELS //
__global__ void calcMandelbrot_k(CudaField<float2> field, CudaFieldTex tex)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  
  if(ix < field.size.x && iy < field.size.y)
    {
      int    i  = iy*field.size.x + ix;
      float2 ip = float2{float(ix), float(iy)};
      float2 sz = float2{float(field.size.x),float(field.size.y)};
      float2 scale = float2{field.params.scale.x, field.params.scale.y};
      float2 offset = float2{field.params.offset.x, field.params.offset.y};
      
      float2 z0 = 4.0f * (ip / sz - float2{0.5, 0.5})*scale + offset;
      float2 z = z0;
      int num_iter = field.params.maxIter;
      int cutoff_2 = field.params.cutoff * field.params.cutoff;
      for(int j = 0; j < field.params.maxIter; j++)
        {
          z = float2 { z.x*z.x - z.y*z.y + z0.x, 2.0f*z.x*z.y + z0.y };
          float len_2 = z.x*z.x + z.y*z.y;
          if(len_2 > cutoff_2) { num_iter = j; break; }
        }
      if(field.params.hollow && num_iter == field.params.maxIter)
        {
          tex.dData[i] = float4{0.0f, 0.0f, 0.0f, 1.0f};
        }
      else
        {
          float c = float(num_iter)/float(field.params.maxIter);
          tex.dData[i] = float4{float(c), float(c), float(c), 1.0f};
        }
      field.dData[i] = z;
    }
}

extern "C" void calcMandelbrot(CudaField<float2> field, CudaFieldTex tex)
{
  if(field.size.x > 0 && field.size.y > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(field.size.x/(float)BLOCKDIM_X), (int)ceil(field.size.y/(float)BLOCKDIM_Y));
      if(!tex.map()) { std::cout << "====> WARNING(calcMandelbrot): Failed to map tex!\n"; return; }
      calcMandelbrot_k<<<grid, threads>>>(field, tex);
      //cudaDeviceSynchronize();
      tex.unmap();
      getLastCudaError("====> ERROR: calcMandelbrot_k failed!");
    }
}
