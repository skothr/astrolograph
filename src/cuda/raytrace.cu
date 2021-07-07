#include "raytrace.cuh"

#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <cufft.h>
#include <helper_cuda.h>
#include <curand.h>
#include <curand_kernel.h>

// // render 3D --> raytrace field
// //  - field assumed to be size (1,1,1) in 3D space
// __device__ float4 rayTraceFluid(HyperFluid<3> &src, float3 fPos, float3 rPos, float3 rDir)
// {
//   //float4 color = float4{0.0f, 0.0f, 0.0f, 1.0f};

//   float tnx = (fPos.x - rPos.x)     / rDir.x;
//   float tpx = (fPos.x - rPos.x + 1) / rDir.x;
//   float tny = (fPos.y - rPos.y)     / rDir.y;
//   float tpy = (fPos.y - rPos.y + 1) / rDir.y;
//   float tnz = (fPos.z - rPos.z)     / rDir.z;
//   float tpz = (fPos.z - rPos.z + 1) / rDir.z;

//   float tmin = min(min(min(tnx, tpx), min(tny, tpy)), min(tnz, tpz));
//   float tmax = max(max(max(tnx, tpx), max(tny, tpy)), max(tnz, tpz));

//   if(tmin > tmax) // no intersection
//     { return float4{0.0f, 0.0f, 0.0f, -1.0f}; }
//   else
//     { return float4{1.0f, 1.0f, 1.0f, 1.0f}; }
// }
