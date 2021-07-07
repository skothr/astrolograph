
#include "cudaField.hpp"
#include "cutools.hpp"
#include "render.cuh"

#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <helper_cuda.h>
#include "vector-operators.h"

#define BLOCKDIM_X 16
#define BLOCKDIM_Y 16



// RENDER FIELD VALUE MAGNITUDES TO DIFFERENT COLOR CHANNELS IN TEXTURE //
template<typename T>
__global__ void renderTexChannel_k(CudaField<T> field1, CudaFieldTex tex, ColorChannel channels,
                                   Vec2f rrange, Vec2f grange, Vec2f brange, Vec2f arange)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix >= 0 && iy >= 0 && ix < field1.size.x && iy < field1.size.y && ix < tex.size.x && iy < tex.size.y)
    {
      int i = iy*field1.size.x + ix;
      T     val = field1.dData[i];
      float len = float(length(val));
      if((int)channels & (int)CHANNEL_R) { tex.dData[i].x = ((len - rrange.x)/(rrange.y - rrange.x)); }
      if((int)channels & (int)CHANNEL_G) { tex.dData[i].y = ((len - grange.x)/(grange.y - grange.x)); }
      if((int)channels & (int)CHANNEL_B) { tex.dData[i].z = ((len - brange.x)/(brange.y - brange.x)); }
      if((int)channels & (int)CHANNEL_A) { tex.dData[i].w = ((len - arange.x)/(arange.y - arange.x)); }
    }
}

__global__ void renderTexTexChannel_k(CudaFieldTex field1, CudaFieldTex tex, ColorChannel channels)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix >= 0 && iy >= 0 && ix < field1.size.x && iy < field1.size.y && ix < tex.size.x && iy < tex.size.y)
    {
      int i = iy*field1.size.x + ix;
      float4 val = field1.dData[i];
      val.w = 0.0f; // don't include alpha
      float len = float(length(val));//*tex.mult;
      if((int)channels & (int)CHANNEL_R) { tex.dData[i].x = len; }
      if((int)channels & (int)CHANNEL_G) { tex.dData[i].y = len; }
      if((int)channels & (int)CHANNEL_B) { tex.dData[i].z = len; }
      if((int)channels & (int)CHANNEL_A) { tex.dData[i].w = len; }
    }
} 

template<typename T>
void renderTexChannel(CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                      const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange)
{
  if(channels == CHANNEL_NONE)  { std::cout << "====> WARNING(renderTexChannel): No Channels!\n"; return; }
  if(field1->size != tex->size) { std::cout << "====> WARNING(renderTexChannel): Sizes don't match! (" << field1->size << " / " << tex->size << ")\n"; return; }
  
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(tex->size.x/(float)BLOCKDIM_X), (int)ceil(tex->size.y/(float)BLOCKDIM_Y));

  CudaField<T> *f1  = reinterpret_cast<CudaField<T>*>(field1);
  
  if(!f1->map())  { std::cout << "====> ERROR(renderTexChannel): Failed to map field1! (" << field1->typeStr() << ")\n"; return; }
  if(!tex->map()) { std::cout << "====> ERROR(renderTexChannel): Failed to map tex! ("    << tex->typeStr()    << ")\n"; f1->unmap(); return; }
  if(f1->isTexture()) { renderTexTexChannel_k <<<grid, threads>>>(*reinterpret_cast<CudaFieldTex*>(f1), *tex, channels); }
  else                { renderTexChannel_k<T> <<<grid, threads>>>(*f1, *tex, channels, rrange, grange, brange, arange); }
  getLastCudaError("====> ERROR: renderTexChannel_k / renderTexTexChannel_k failed!");
  tex->unmap(); f1->unmap();
}

// template instantiation
template void renderTexChannel<int>    (CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                                        const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange);
template void renderTexChannel<int2>   (CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                                        const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange);
template void renderTexChannel<int3>   (CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                                        const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange);
template void renderTexChannel<int4>   (CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                                        const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange);
template void renderTexChannel<float>  (CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                                        const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange);
template void renderTexChannel<float2> (CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                                        const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange);
template void renderTexChannel<float3> (CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                                        const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange);
template void renderTexChannel<float4> (CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                                        const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange);
template void renderTexChannel<double> (CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                                        const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange);
template void renderTexChannel<double2>(CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                                        const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange);
template void renderTexChannel<double3>(CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                                        const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange);
template void renderTexChannel<double4>(CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels,
                                        const Vec2f &rrange, const Vec2f &grange, const Vec2f &brange, const Vec2f &arange);



__global__ void fillTexChannel_k(CudaFieldTex tex, ColorChannel channels, float value)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix >= 0 && iy >= 0 && ix < tex.size.x && iy < tex.size.y)
    {
      int i = iy*tex.size.x + ix;
      if((int)channels & (int)CHANNEL_R) { tex.dData[i].x = value; }
      if((int)channels & (int)CHANNEL_G) { tex.dData[i].y = value; }
      if((int)channels & (int)CHANNEL_B) { tex.dData[i].z = value; }
      if((int)channels & (int)CHANNEL_A) { tex.dData[i].w = value; }
    }
}


void fillTexChannel(CudaFieldTex *tex, ColorChannel channels, float value)
{
  if(channels == CHANNEL_NONE)  { std::cout << "====> WARNING(fillTexChannel): No Channels!\n"; return; }
  
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(tex->size.x/(float)BLOCKDIM_X), (int)ceil(tex->size.y/(float)BLOCKDIM_Y));
  
  if(!tex->map()) { std::cout << "====> ERROR(fillTexChannel): Failed to map tex! (" << tex->typeStr() << ")\n"; return; }
  fillTexChannel_k <<<grid, threads>>>(*tex, channels, value);
  getLastCudaError("====> ERROR: fillTexChannel_k / renderTexTexChannel_k failed!");
  tex->unmap();
}





// NEW/IMPROVED texture render kernel (uses magnitude of field values)
template<typename T>
__global__ void renderTex_k(CudaField<T> field1, CudaFieldTex tex)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < field1.size.x && iy < field1.size.y)
    {
      int i = iy*field1.size.x + ix;
      T val = field1.dData[i];
      float  len   = float(length(val))*tex.mult;
      float4 tval  = float4{len, len, len, 1.0};
      tex.dData[i] = tval;
    }
}

__global__ void renderTexTex_k(CudaFieldTex field1, CudaFieldTex tex)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < field1.size.x && iy < field1.size.y)
    {
      int i = iy*field1.size.x + ix;
      float4 val = field1.dData[i];
      val.w = 0.0f; // don't include alpha
      float  len   = float(length(val))*tex.mult;
      float4 tval  = float4{len, len, len, 1.0};
      tex.dData[i] = tval;
    }
}

template<typename T>
void renderTex(CudaFieldBase *field1, CudaFieldTex *tex)
{
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(tex->size.x/(float)BLOCKDIM_X), (int)ceil(tex->size.y/(float)BLOCKDIM_Y));


  CudaField<T> *f1  = reinterpret_cast<CudaField<T>*>(field1);
  if(!f1->map())  { std::cout << "====> ERROR(renderTex): Failed to map field1! (" << field1->typeStr() << ")\n"; return; }
  if(!tex->map()) { std::cout << "====> ERROR(renderTex): Failed to map tex! ("    << tex->typeStr()    << ")\n"; f1->unmap(); return; }

  if(f1->isTexture()) { renderTexTex_k <<<grid, threads>>>(*reinterpret_cast<CudaFieldTex*>(f1), *tex); }
  else                { renderTex_k<T> <<<grid, threads>>>(*f1, *tex); }
  getLastCudaError("====> ERROR: renderTex_k failed!");
  
  tex->unmap(); f1->unmap();
}







// template instantiation
template void renderTex<int>    (CudaFieldBase *field1, CudaFieldTex *tex);
template void renderTex<int2>   (CudaFieldBase *field1, CudaFieldTex *tex);
template void renderTex<int3>   (CudaFieldBase *field1, CudaFieldTex *tex);
template void renderTex<int4>   (CudaFieldBase *field1, CudaFieldTex *tex);
template void renderTex<float>  (CudaFieldBase *field1, CudaFieldTex *tex);
template void renderTex<float2> (CudaFieldBase *field1, CudaFieldTex *tex);
template void renderTex<float3> (CudaFieldBase *field1, CudaFieldTex *tex);
template void renderTex<float4> (CudaFieldBase *field1, CudaFieldTex *tex);
template void renderTex<double> (CudaFieldBase *field1, CudaFieldTex *tex);
template void renderTex<double2>(CudaFieldBase *field1, CudaFieldTex *tex);
template void renderTex<double3>(CudaFieldBase *field1, CudaFieldTex *tex);
template void renderTex<double4>(CudaFieldBase *field1, CudaFieldTex *tex);















// fills texture data with solid color
__global__ void fillTex_k(float4 *data, int w, int h, float4 color)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < w && iy < h)
    {
      int i  = iy*w + ix;
      data[i] = color;
    }
}
extern "C" void fillTex(float4 *data, int w, int h, float4 color)
{
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(w/(float)BLOCKDIM_X), (int)ceil(h/(float)BLOCKDIM_Y));
  fillTex_k<<<grid, threads>>>(data, w, h, color);
  getLastCudaError("====> ERROR: fillField_k failed!");
}

// multiplies two fields
__global__ void mult_k(CudaField<float> field1, CudaField<float> field2, CudaField<float> dst, float mult)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < field1.size.x && iy < field1.size.y && ix < field2.size.x && iy < field2.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = field1.dData[i]*field2.dData[i]*mult;
    }
}
extern "C" void multFields(CudaField<float> field1, CudaField<float> field2, CudaField<float> dst, float mult)
{
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field1.size.x/(float)BLOCKDIM_X), (int)ceil(field1.size.y/(float)BLOCKDIM_Y));
  mult_k<<<grid, threads>>>(field1, field2, dst, mult);
  getLastCudaError("====> ERROR: mult_k failed!");
}

__global__ void mult2_k(CudaField<float2> field1, CudaField<float2> field2, CudaField<float2> dst, float mult)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < field1.size.x && iy < field1.size.y && ix < field2.size.x && iy < field2.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = field1.dData[i]*field2.dData[i]*mult*field1.params.texMult*field2.params.texMult*dst.params.texMult;
    }
}
extern "C" void multFields2(CudaField<float2> field1, CudaField<float2> field2, CudaField<float2> dst, float mult)
{
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field1.size.x/(float)BLOCKDIM_X), (int)ceil(field1.size.y/(float)BLOCKDIM_Y));
  mult2_k<<<grid, threads>>>(field1, field2, dst, mult);
  getLastCudaError("====> ERROR: mult2_k failed!");
}

__global__ void multTex_k(CudaFieldTex field1, CudaFieldTex field2, CudaFieldTex dst, float mult)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < field1.size.x && iy < field1.size.y && ix < field2.size.x && iy < field2.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = field1.dData[i]*field2.dData[i]*mult*field1.mult*field2.mult*dst.mult;
    }
}
extern "C" void multTextures(CudaFieldTex field1, CudaFieldTex field2, CudaFieldTex dst, float mult)
{
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field1.size.x/(float)BLOCKDIM_X), (int)ceil(field1.size.y/(float)BLOCKDIM_Y));

  //cudaDeviceSynchronize();
  getLastCudaError("====> ERROR: before mapping! (multTextures)!");
  if(!field1.map()) { std::cout << "====> ERROR: Failed to map field1!\n"; return; }
  if(!field2.map()) { std::cout << "====> ERROR: Failed to map field2!\n"; field1.unmap(); return; }
  if(!dst.map())    { std::cout << "====> ERROR: Failed to map dst!\n"; field2.unmap(); field1.unmap(); return; }
  multTex_k<<<grid, threads>>>(field1, field2, dst, mult);
  //cudaDeviceSynchronize();
  getLastCudaError("====> ERROR: multTex_k failed!");
  dst.unmap();
  field2.unmap();
  field1.unmap();
}


__global__ void multFieldConst_k(CudaField<float> field, CudaField<float> dst, float mult)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < field.size.x && iy < field.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      dst.dData[i] = field.dData[i]*mult;
    }
}
extern "C" void multFieldConst(CudaField<float> field, CudaField<float> dst, float mult)
{
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field.size.x/(float)BLOCKDIM_X), (int)ceil(field.size.y/(float)BLOCKDIM_Y));
  getLastCudaError("====> ERROR: before mapping! (multFieldConst)!");
  multFieldConst_k<<<grid, threads>>>(field, dst, mult);
  getLastCudaError("====> ERROR: multFieldConst_k failed!");
}

__global__ void multTextureConst_k(CudaFieldTex field, CudaFieldTex dst, float mult)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < field.size.x && iy < field.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*dst.size.x + ix;
      float4 d     = abs(field.dData[i])*mult*field.mult*dst.mult;
      dst.dData[i] = float4{d.x, d.y, d.z, 1.0};
    }
}
extern "C" void multTextureConst(CudaFieldTex field, CudaFieldTex dst, float mult)
{
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(field.size.x/(float)BLOCKDIM_X), (int)ceil(field.size.y/(float)BLOCKDIM_Y));

  //cudaDeviceSynchronize();
  getLastCudaError("====> ERROR: before mapping! (multTextureConst)!");
  if(!field.map()) { std::cout << "====> ERROR: Failed to map field1!\n"; return; }
  if(!dst.map())   { std::cout << "====> ERROR: Failed to map dst!\n"; field.unmap(); return; }
  multTextureConst_k<<<grid, threads>>>(field, dst, mult);
  //cudaDeviceSynchronize();
  getLastCudaError("====> ERROR: multTextureConst_k failed!");
  dst.unmap();
  field.unmap();
}


// loads magnitude of src texture into dst data
__global__ void loadTexMagnitude_k(CudaFieldTex src, CudaField<float> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  
  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*src.size.x + ix;
      float4 d = src.dData[i]*src.mult*dst.params.texMult;
      dst.dData[i] = sqrt(d.x*d.x + d.y*d.y + d.z*d.z);
    }
}
// loads magnitude of src texture into dst data
__global__ void loadTexMagnitude2_k(CudaFieldTex src, CudaField<float2> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  
  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*src.size.x + ix;
      float4 d = src.dData[i]*src.mult*dst.params.texMult;
      dst.dData[i] = float2{sqrt(d.x*d.x + d.y*d.y + d.z*d.z), 0.0f};
    }
}
// loads magnitude of src texture into dst data
extern "C" void loadTexMagnitude(CudaFieldTex src, CudaField<float> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && src.size == dst.size)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      if(!src.map()) { std::cout << "====> ERROR(loadTexMagnitude): Failed to map texture!\n"; return; }
      loadTexMagnitude_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: loadTexMagnitude_k failed!");
      src.unmap();
    }
}
// loads magnitude of src texture into dst data
extern "C" void loadTexMagnitude2(CudaFieldTex src, CudaField<float2> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && src.size == dst.size)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      if(!src.map()) { std::cout << "====> ERROR(loadTexMagnitude2): Failed to map texture!\n"; return; }
      loadTexMagnitude2_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: loadTexMagnitude2_k failed!");
      src.unmap();
    }
}


// loads magnitude of src field into dst data
__global__ void loadFieldMagnitude_k(CudaField<float> src, CudaField<float2> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  
  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*src.size.x + ix;
      float d = src.dData[i];
      dst.dData[i] = float2{d*src.params.texMult*dst.params.texMult, 0.0f};
    }
}
// loads magnitude of src fieldture into dst data
extern "C" void loadFieldMagnitude(CudaField<float> src, CudaField<float2> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && src.size == dst.size)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      loadFieldMagnitude_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: loadFieldMagnitude_k failed!");
    }
}

// loads magnitude of src field into dst data
__global__ void loadFieldMagnitude2_k(CudaField<float2> src, CudaField<float2> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  
  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*src.size.x + ix;
      float2 d = src.dData[i];
      dst.dData[i] = float2{sqrt(d.x*d.x + d.y*d.y)*src.params.texMult*dst.params.texMult, 0.0f};
    }
}
// loads magnitude of src fieldture into dst data
extern "C" void loadFieldMagnitude2(CudaField<float2> src, CudaField<float2> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && src.size == dst.size)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      loadFieldMagnitude2_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: loadFieldMagnitude2_k failed!");
    }
}


// loads magnitude of src field into dst data
__global__ void renderMagnitude_k(CudaField<float> src, CudaFieldTex dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  
  if(ix < src.size.x && iy < src.size.y && src.size.x == dst.size.x && src.size.y == dst.size.y)
    {
      int i = iy*src.size.x + ix;
      float d = abs(src.dData[i])*src.params.texMult*dst.mult;
      dst.dData[i] = float4{d, d, d, 1.0f};
    }
}
// loads magnitude of src fieldture into dst data
extern "C" void renderMagnitude(CudaField<float> src, CudaFieldTex dst)
{
  if(src.size.x > 0 && src.size.y > 0 && src.size.x == dst.size.x && src.size.y == dst.size.y)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      if(!dst.map()) { std::cout << "====> ERROR(renderMagnitude): Failed to map dst texture!\n"; return; }
      renderMagnitude_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: renderMagnitude_k failed!");
      dst.unmap();
    }
}


// FFT RENDERING //

// renders real values to texture (greyscale)
__global__ void renderFFT2_real_k(CudaField<float2> src, CudaFieldTex dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y)
    {
      int i  = iy*src.size.x + ix;
      float d = abs(src.dData[i].x)*dst.mult*src.params.texMult;
      dst.dData[i] = float4{d, d, d, 1.0f};
    }
}
// renders imaginary values to texture (greyscale)
__global__ void renderFFT2_imag_k(CudaField<float2> src, CudaFieldTex dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y)
    {
      int i  = iy*src.size.x + ix;
      float d = abs(src.dData[i].y)*dst.mult*src.params.texMult;
      dst.dData[i] = float4{d, d, d, 1.0f};
    }
}
// renders magnitude of complex vectors to texture (greyscale
__global__ void renderFFT2_magnitude_k(CudaField<float2> src, CudaFieldTex dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y)
    {
      int i  = iy*src.size.x + ix;
      float2 d = src.dData[i];
      float  c = sqrt(d.x*d.x + d.y*d.y)*dst.mult*src.params.texMult;
      dst.dData[i] = float4{c, c, c, 1.0f};
    }
}

// renders phase of complex vectors to texture (greyscale
__global__ void renderFFT2_phase_k(CudaField<float2> src, CudaFieldTex dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y)
    {
      int i  = iy*src.size.x + ix;
      
      float2 d = src.dData[i];
      float  c = atan2(d.y, d.x);
      c = (c < 0.0f ? float(M_PI+c) : c)*dst.mult*src.params.texMult/float(2.0*M_PI);
      dst.dData[i] = float4{c, c, c, 1.0f};
    }
}

// renders magnitude of complex vectors to texture (greyscale
__global__ void renderFFT2_combined_k(CudaField<float2> src, CudaFieldTex dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y)
    {
      int i  = iy*src.size.x + ix;
      
      float2 d = src.dData[i];
      dst.dData[i] = float4{d.x, d.y, 0.0f, 1.0f};
      // float angle = atan2(d.y, d.x)/float(2.0*M_PI);
      // dst.dData[i] = float4{sqrt(d.x*d.x+d.y*d.y)*dst.mult*src.params.texMult,
      //                       (angle < 0.0f ? float(M_PI+angle) : angle)*dst.mult*src.params.texMult,
      //                       0.0f, 1.0f};
    }
}


// renders real values to texture (greyscale)
extern "C" void renderFft_real(CudaField<float2> src, CudaFieldTex dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size.x == src.size.x && dst.size.y == src.size.y)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      if(!dst.map()) { std::cout << "====> ERROR(renderFft_real): Failed to map dst texture!\n"; return; }
      renderFFT2_real_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: renderFFT2_real_k failed!");
      dst.unmap();
    }
}
// renders complex values to texture (greyscale)
extern "C" void renderFft_imag(CudaField<float2> src, CudaFieldTex dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size.x == src.size.x && dst.size.y == src.size.y)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      if(!dst.map()) { std::cout << "====> ERROR(renderFft_imag): Failed to map dst texture!\n"; return; }
      renderFFT2_imag_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: renderFFT2_imag_k failed!");
      dst.unmap();
    }
}
// renders complex magnitude to texture (greyscale)
extern "C" void renderFft_magnitude(CudaField<float2> src, CudaFieldTex dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size.x == src.size.x && dst.size.y == src.size.y)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      if(!dst.map()) { std::cout << "====> ERROR(renderFft_magnitude): Failed to map dst texture!\n"; return; }
      renderFFT2_magnitude_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: renderFFT2_magnitude_k failed!");
      dst.unmap();
    }
}
// renders complex phase to texture (greyscale)
extern "C" void renderFft_phase(CudaField<float2> src, CudaFieldTex dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size.x == src.size.x && dst.size.y == src.size.y)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      if(!dst.map()) { std::cout << "====> ERROR(renderFft_phase): Failed to map dst texture!\n"; return; }
      renderFFT2_phase_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: renderFFT2_phase_k failed!");
      dst.unmap();
    }
}

// renders complex magnitude to texture (red=magnitude, green=phase, blue=combination)
extern "C" void renderFft_combined(CudaField<float2> src, CudaFieldTex dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size == src.size)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      if(!dst.map()) { std::cout << "====> ERROR(renderFft_combined): Failed to map dst texture!\n"; return; }
      renderFFT2_combined_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: renderFFT2_combined_k failed!");
      dst.unmap();
    }
}















