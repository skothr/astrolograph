#ifndef CUTOOLS_HPP
#define CUTOOLS_HPP

#include <cuda_runtime.h>
#include <helper_cuda.h>
#include <helper_functions.h>
#include <string>

extern bool gCudaInitialized;
extern int  gDevId;

// setup
bool initDevice(int devId=-1);
extern "C" void fillTex(float4 *data, int w, int h, float4 color);

struct CudaFieldTex;
extern bool writeTexture(const std::string &path, CudaFieldTex *tex);

// device helpers

template<typename T> __device__ inline bool isnan(T v)                { return isnan(v); }
template<>           __device__ inline bool isnan<float2> (float2 v)  { return (isnan(v.x) || isnan(v.y)); }
template<>           __device__ inline bool isnan<float3> (float3 v)  { return (isnan(v.x) || isnan(v.y) || isnan(v.z)); }
template<>           __device__ inline bool isnan<float4> (float4 v)  { return (isnan(v.x) || isnan(v.y) || isnan(v.z) || isnan(v.w)); }
template<>           __device__ inline bool isnan<double2>(double2 v) { return (isnan(v.x) || isnan(v.y)); }
template<>           __device__ inline bool isnan<double3>(double3 v) { return (isnan(v.x) || isnan(v.y) || isnan(v.z)); }
template<>           __device__ inline bool isnan<double4>(double4 v) { return (isnan(v.x) || isnan(v.y) || isnan(v.z) || isnan(v.w)); }

template<typename T> __device__ T lerp(T x0, T x1, float alpha) { return x1*alpha + x0*(1.0f-alpha); }

// currently implements WRAP addressing
template<typename T>
__device__ T texGet(T *tex, int x, int y, int w, int h)
{
  if(x < 0) { x = 0; } else if(x >= w) { x = w-1; }
  if(y < 0) { y = 0; } else if(y >= h) { y = h-1; }
  return tex[y*w + x];
}
// currently implements WRAP addressing
template<typename T>
__device__ void texPut(T *tex, T val, int x, int y, int w, int h)
{
  if(x < 0) { x = 0; } if(x >= w) { x = w-1; }
  if(y < 0) { y = 0; } if(y >= h) { y = h-1; }
  tex[y*w + x] = val;
}
template<typename T>
__device__ T tex2DD(T *tex, float x, float y, int w, int h)
{
  x -= 0.5f; y -= 0.5f;
  int2    p = int2  {int(floor(x)), int(floor(y))}; // integer position
  float2 fp = float2{x-p.x, y-p.y};                 // fractional position
  
  if(p.x > 0 && p.x < w-1 && p.y > 0 && p.y < h-1)
    {
      T result = lerp<T>(lerp<T>(texGet(tex, p.x, p.y,   w, h), texGet(tex, p.x+1, p.y,   w, h), fp.x),
                         lerp<T>(texGet(tex, p.x, p.y+1, w, h), texGet(tex, p.x+1, p.y+1, w, h), fp.x), fp.y);
      return T(isnan(result) ? T{0.0} : result);
    }
  else { return T{0.0f}; }
}



//
//// 3D ////
//


// currently implements WRAP addressing
template<typename T>
__device__ T texGet3(T *tex, int x, int y, int z, int w, int h, int d)
{
  if(x < 0) { x = 0; } else if(x >= w) { x = w-1; }
  if(y < 0) { y = 0; } else if(y >= h) { y = h-1; }
  if(z < 0) { z = 0; } else if(z >= d) { z = d-1; }
  return tex[x + w*(y + h*(z))];
}
// currently implements WRAP addressing
template<typename T>
__device__ void texPut3(T *tex, T val, int x, int y, int z, int w, int h, int d)
{
  if(x < 0) { x = 0; } if(x >= w) { x = w-1; }
  if(y < 0) { y = 0; } if(y >= h) { y = h-1; }
  if(z < 0) { z = 0; } if(z >= d) { z = d-1; }
  tex[x + w*(y + h*(z))] = val;
}
template<typename T>
__device__ T tex3DD(T *tex, float x, float y, float z, int w, int h, int d)
{
  x -= 0.5f; y -= 0.5f; z -= 0.5f;
  int3    p = int3  {int(floor(x)), int(floor(y)), int(floor(z))}; // integer position
  float3 fp = float3{x-p.x, y-p.y, z-p.z};                         // fractional position
  
  if(p.x > 0 && p.x < w-1 && p.y > 0 && p.y < h-1 && p.z > 0 && p.z < d-1)
    {
      T resultz1 = lerp<T>(lerp<T>(texGet3(tex, p.x, p.y,   p.z,   w, h, d), texGet3(tex, p.x+1, p.y,   p.z,   w, h, d), fp.x),
                           lerp<T>(texGet3(tex, p.x, p.y+1, p.z,   w, h, d), texGet3(tex, p.x+1, p.y+1, p.z,   w, h, d), fp.x), fp.y);
      T resultz2 = lerp<T>(lerp<T>(texGet3(tex, p.x, p.y,   p.z+1, w, h, d), texGet3(tex, p.x+1, p.y,   p.z+1, w, h, d), fp.x),
                           lerp<T>(texGet3(tex, p.x, p.y+1, p.z+1, w, h, d), texGet3(tex, p.x+1, p.y+1, p.z+1, w, h, d), fp.x), fp.y);
      T result = lerp<T>(resultz1, resultz2, fp.z);
      return T(isnan(result) ? T{0.0} : result);
    }
  else { return T{0.0f}; }
}



//#include "cudaField.hpp"

// maximization reduction

// template<typename T, unsigned int blockSize> __global__ void fieldMax_k(CudaField<T> fieldIn, CudaField<T> fieldOut, unsigned int n);
class CudaFieldBase;
template<typename T> class CudaField;
template<typename T> float fieldMax(CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);
template<typename T> float fieldNorm(CudaFieldBase *field, CudaFieldBase *fieldOut, CudaField<float> *dst);


void combineChannels(CudaFieldBase *fieldX, CudaFieldBase *dst);
void combineChannels(CudaFieldBase *fieldX, CudaFieldBase *fieldY, CudaFieldBase *dst);
void combineChannels(CudaFieldBase *fieldX, CudaFieldBase *fieldY, CudaFieldBase *fieldZ, CudaFieldBase *dst);
void combineChannels(CudaFieldBase *fieldX, CudaFieldBase *fieldY, CudaFieldBase *fieldZ, CudaFieldBase *fieldW, CudaFieldBase *dst);

void splitChannels(CudaFieldBase *field, CudaFieldBase *dstX);
void splitChannels(CudaFieldBase *field, CudaFieldBase *dstX, CudaFieldBase *dstY);
void splitChannels(CudaFieldBase *field, CudaFieldBase *dstX, CudaFieldBase *dstY, CudaFieldBase *dstZ);
void splitChannels(CudaFieldBase *field, CudaFieldBase *dstX, CudaFieldBase *dstY, CudaFieldBase *dstZ, CudaFieldBase *dstW);

#endif // CUTOOLS_HPP
