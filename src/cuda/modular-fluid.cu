#include "cudaField.hpp"
#include "cutools.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <cufft.h>
#include <helper_cuda.h>
#include <curand.h>
#include <curand_kernel.h>

#include "vector-operators.h"

#define BLOCKDIM_X 8
#define BLOCKDIM_Y 8

// fills texture data with solid color
__global__ void clearFluid_k(CudaFluid<float> fluid)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < fluid.size.x && iy < fluid.size.y)
    {
      int i = iy*fluid.size.x + ix;
      float2 p = float2{(ix-fluid.size.x/2.0f)/float(fluid.size.x), (iy-fluid.size.y/2.0f)/float(fluid.size.y) };
      fluid.d.dData[i]    = 1.0f;
      fluid.vx.dData[i]   = 0.0f;
      fluid.vy.dData[i]   = 0.0f;
      fluid.p.dData[i]    = (p.x*p.x+ p.y*p.y);
      fluid.wv.dData[i]   = float2{0.0f, 0.0f};
      // fluid.wv.dData[i] = float2{p.y, p.x}/8.0f;
    }
}

__global__ void fillFluidCircle_k(CudaFluid<float> fluid)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < fluid.size.x && iy < fluid.size.y)
    {
      int i = iy*fluid.size.x + ix;
      float2 p = float2{(ix-fluid.size.x/2.0f)/float(fluid.size.x), (iy-fluid.size.y/2.0f)/float(fluid.size.y) };
      float dist = sqrt(p.x*p.x + p.y*p.y);
      fluid.d.dData[i]    = 1.0f;
      fluid.vx.dData[i]   = dist > 1.0f/4.0f ? 0.0f : p.x;
      fluid.vy.dData[i]   = dist > 1.0f/4.0f ? 0.0f : p.y;
      fluid.p.dData[i]    = (p.x*p.x+ p.y*p.y);
      fluid.wv.dData[i]   = (dist > 1.0f/4.0f ? float2{p.y, p.x} : float2{0.0f, 0.0f});
    }
}

__device__ float2 cMult(float2 z0, float2 z1)
{
  return float2{z0.x*z1.x - z0.y*z1.y, z0.x*z1.y + z1.x*z0.y};
}

__global__ void advect_k(CudaFluid<float> src, CudaFluid<float> dst, int lb)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*src.size.x + ix;
      int w = src.size.x; int h = src.size.y;
      
      float dt = src.params.dt;


      
      float2 u0 = float2{tex2DD(src.vx.dData, ix+0.5f, iy, w, h), tex2DD(src.vy.dData, ix, iy+0.5f, w, h)};
      float2 u = u0;
      
      float2 lastPos = float2{ float(ix) - dt*u.x, float(iy) - dt*u.y };
      float2 nextPos = float2{ float(ix) + dt*u.x, float(iy) + dt*u.y };
      
      float2 lastU = float2{tex2DD(src.vx.dData, lastPos.x+0.5f, lastPos.y+0.5f, w, h),
                            tex2DD(src.vy.dData, lastPos.x+0.5f, lastPos.y+0.5f, w, h) };
      float2 nextU = float2{tex2DD(src.vx.dData, nextPos.x+0.5f, nextPos.y+0.5f, w, h),
                            tex2DD(src.vy.dData, nextPos.x+0.5f, nextPos.y+0.5f, w, h) };
      
      float2 lastUdx = float2{tex2DD(src.vx.dData, lastPos.x+1+1, lastPos.y,     w, h),
                              tex2DD(src.vy.dData, lastPos.x+1,   lastPos.y+1,   w, h) } - lastU;
      float2 lastUdy = float2{tex2DD(src.vx.dData, lastPos.x+1,   lastPos.y+1,   w, h),
                              tex2DD(src.vy.dData, lastPos.x,     lastPos.y+1+1, w, h) } - lastU;
      float2 nextUdx = float2{tex2DD(src.vx.dData, nextPos.x+1+1, nextPos.y,     w, h),
                              tex2DD(src.vy.dData, nextPos.x+1,   nextPos.y+1,   w, h) } - nextU;
      float2 nextUdy = float2{tex2DD(src.vx.dData, nextPos.x+1,   nextPos.y+1,   w, h),
                              tex2DD(src.vy.dData, nextPos.x,     nextPos.y+1+1, w, h) } - nextU;
      
      float  lastD  = tex2DD(src.d.dData,  lastPos.x+0.5f, lastPos.y+0.5f, w, h);
      float  lastP  = tex2DD(src.p.dData,  lastPos.x+0.5f, lastPos.y+0.5f, w, h);
      float2 lastWv = tex2DD(src.wv.dData, lastPos.x+0.5f, lastPos.y+0.5f, w, h);

      // float newD      = lastD/(1.0 + abs(dt))*0.99;//*params->dissipation);
      // float newP      = lastP/(1.0 + abs(dt))*0.99;

      float newD = lastD;
      float newP = lastP;

      if(isnan(lastU.x))  { lastU.x  = 0.0; }
      if(isnan(lastU.y))  { lastU.y  = 0.0; }
      if(isnan(newD))     { newD     = 0.0; }
      if(isnan(newP))     { newP     = 0.0; }

      u = lastU;
      
      // u.x += u0.y*newP*dt;
      // u.y += u0.x*newP*dt;

      // newP += length(u)*dt;

      u.x      = (u.x<0.0f?-1.0f:1.0f)*min(abs(u.x), 4.0f);
      u.y      = (u.y<0.0f?-1.0f:1.0f)*min(abs(u.y), 4.0f);
      newP     = (newP < 0.0f ? 0.0f : min(abs(u.y), 4.0f));

      
      float2 wv00    = texGet(src.wv.dData, ix, iy, w, h);
      float2 wv      = float2{0.0f, 0.0f};
      if(src.params.applyChaos)
        {
          float2 wvNX = tex2DD(src.wv.dData, lastPos.x+0.5f-1.0f, lastPos.y+0.5f,      w, h);//texGet(src.wv.dData, ix-1, iy, w, h);
          float2 wvPX = tex2DD(src.wv.dData, lastPos.x+0.5f+1.0f, lastPos.y+0.5f,      w, h);//texGet(src.wv.dData, ix+1, iy, w, h);
          float2 wvNY = tex2DD(src.wv.dData, lastPos.x+0.5f,      lastPos.y+0.5f-1.0f, w, h);//texGet(src.wv.dData, ix, iy-1, w, h);
          float2 wvPY = tex2DD(src.wv.dData, lastPos.x+0.5f,      lastPos.y+0.5f+1.0f, w, h);//texGet(src.wv.dData, ix, iy+1, w, h);
          
          float2 dwvX = ((wvPX - wv00) - (wv00 - wvNX))/(2.0);
          float2 dwvY = ((wvPY - wv00) - (wv00 - wvNY))/(2.0);
          
          float2 dwvDt = cMult(dwvX, dwvY);
          //float2 dwvDt = float2{dwvX.x*dwvY.x - dwvX.y*dwvY.y};

          wv =  lastWv;
          wv -= dwvDt*dt;
          wv += cMult(float2{-abs(wv.x), wv.y}, float2{-abs(wv.x), wv.y})*dt;//*src.params.wvf;// + lastU;// + wv;
          wv.x = (wv.x >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.x), 4.0f);
          wv.y = (wv.y >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.y), 4.0f);
          
          u.x += wv.x*dt;
          u.y += wv.y*dt;
        }

      dst.vx.dData[i]   = u.x;
      dst.vy.dData[i]   = u.y;
      dst.wv.dData[i]   = wv;
      dst.d.dData[i]    = newD;
      dst.p.dData[i]    = newP;
    }
}

__global__ void diffuse_k(CudaFluid<float> src, CudaFluid<float> dst)
{
#define KERNEL_SIZE 2
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix+KERNEL_SIZE < src.size.x && iy+KERNEL_SIZE < src.size.y && ix > KERNEL_SIZE && iy > KERNEL_SIZE)
    {
      int i = iy*src.size.x + ix;
      float dt = src.params.dt;
      
      float2 u    = float2{0.0f, 0.0f};
      float2 wv   = float2{0.0f, 0.0f};
      float  d    = 0.0f;
      float  p    = 0.0f;
      float  mult = 0.0f;
      
      for(int x = -KERNEL_SIZE; x <= KERNEL_SIZE; x++)
        for(int y = -KERNEL_SIZE; y <= KERNEL_SIZE; y++)
          {
            float dist = (x==0 && y==0) ? 1.0f : sqrt(float(x*x + y*y)) / dt;
            mult += 1.0f/dist;
            int li = src.size.x*(iy+y) + ix+x;
            u.x += src.vx.dData[li]/dist;
            u.y += src.vy.dData[li]/dist;
            wv  += src.wv.dData[li]/dist;
            d   += src.d.dData [li]/dist;
            p   += src.p.dData [li]/dist;
          }
      dst.vx.dData[i] = u.x / mult;
      dst.vy.dData[i] = u.y / mult;
      dst.wv.dData[i] = wv  / mult;
      dst.d.dData[i]  = d   / mult;
      dst.p.dData[i]  = p   / mult;
    }
}


__forceinline__ __device__ float lineDist(float2 l1, float2 l2, float2 p0)
{
  return abs((l2.y - l1.y)*p0.x - (l2.x - l1.x)*p0.y + l2.x*l1.y - l2.y*l1.x)/sqrt((l2.y-l1.y)*(l2.y-l1.y) + (l2.x-l1.x)*(l2.x-l1.x));
}

__forceinline__ __device__ float2 closestPoint(float2 l1, float2 l2, float2 p0)
{
  float2 n = normalize(l2 - l1);
  float2 v = normalize(p0 - l1);
  float  d = dot(v, n);
  return l1 + n*d;
}

__global__ void addForces_k(CudaFluid<float> src)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  int w = src.size.x; int h = src.size.y;

  if(ix < w && iy < h)
    {
      int i = iy*w + ix;
      float dt = src.params.dt;
      FluidParams *params = &src.params;

      float2 forcePoint1 = params->mp;
      float2 forcePoint2 = params->mpLast;
      float2 mv = forcePoint1 - forcePoint2;

      float d = src.d.dData[i];
      float p = src.p.dData[i];
      float2 u = float2{src.vx.dData[i], src.vy.dData[i]};

      
      float2 wv = src.wv.dData[i];

      float2 pp = float2{float(ix)/w, float(iy)/h};

      // calculate force based on distance from mouse movement line segment
      float2 diff = closestPoint(forcePoint1, forcePoint2, pp);
      float  dist = lineDist(forcePoint1, forcePoint2, pp);

      if((dot(normalize(forcePoint2 - forcePoint1), normalize(pp - forcePoint1)) <= 0) ||
         (dot(normalize(forcePoint1 - forcePoint2), normalize(pp - forcePoint2)) <= 0) )
        {
          float distp1 = length(forcePoint1 - pp);
          float distp2 = length(forcePoint2 - pp);

          if(distp1 < distp2)
            {
              diff = pp - forcePoint1;
              dist = distp1;
            }
          else
            {
              diff = pp - forcePoint2;
              dist = distp2;
            }
        }
      float  dist2 = dist*dist;
      float2 pDiff = pp - forcePoint1;
      float  pDist = length(pDiff);
      
      if((params->mdown))
        {
          float mult = 0.0;
          //float dMult = 0.0;

          if(dist < params->forceRad-0.001)
            {
              //mult = 1.0/(1.0 + ((pDiff.x*pDiff.x+pDiff.y*pDiff.y)*dist2)/(params->forceRad*params->forceRad));
              mult = smoothstep(0.001, 0.999, 1.0 - params->forceRad);
              mult = mult;
            }

#define FORCE_SIGMA (sqrt(params->forceRad/3))
          
          if(params->ftype & FLUIDFORCE_PUSH)
            { // push force
              if(dist < params->forceRad-0.001)
                {
                  ///float mult2 = params->vfPush * smoothstep(0.001, 1.0, 1.0 - dist/params->forceRadius);
                  float mult2 = 1.0f;// * exp(-dist/(2*FORCE_SIGMA*FORCE_SIGMA));
                  //mult2 = mult2*mult2;
                  u.x += mv.x * mult2 * params->vfPush;// * dt;
                  u.y += mv.y * mult2 * params->vfPush;// * dt;
                  if(src.params.applyChaos)
                    {
                      wv.x -= mv.x * mult2 * params->wvf;
                      wv.y -= mv.y * mult2 * params->wvf;
                    }
                }
            }

          if(!isnan(mult))
            {
              if(params->ftype & FLUIDFORCE_DENSITY)  { d += mult*params->df; }
              if(params->ftype & FLUIDFORCE_PRESSURE) { p += mult*params->pf; }
            }
          
          if(params->ftype & FLUIDFORCE_IN)
            { // inward forceRadius
              u.x += -pDiff.x*mult*params->vfIn;
              u.y += -pDiff.y*mult*params->vfIn;
              if(params->ftype & FLUIDFORCE_WV)
                {
                  wv.x -= -pDiff.x*mult*params->wvf;
                  wv.y -= -pDiff.y*mult*params->wvf;
                }
            }
          if(params->ftype & FLUIDFORCE_OUT)
            { // outward force
              u.x += pDiff.x*mult*params->vfOut;
              u.y += pDiff.y*mult*params->vfOut;
              if(params->ftype & FLUIDFORCE_WV)
                {
                  wv.x -= pDiff.x*mult*params->wvf;
                  wv.y -= pDiff.y*mult*params->wvf;
                }
            }
          if(params->ftype & FLUIDFORCE_CW)
            { // clockwise force
              u.x += pDiff.y*mult*params->vfCw;
              u.y += -pDiff.x*mult*params->vfCw;
              if(params->ftype & FLUIDFORCE_WV)
                {
                  wv.x -= pDiff.y*mult*params->wvf;
                  wv.y -= -pDiff.x*mult*params->wvf;
                }
            }
          if(params->ftype & FLUIDFORCE_CCW)
            { // counter-clockwise force
              u.x += -pDiff.y*mult*params->vfCcw;
              u.y += pDiff.x*mult*params->vfCcw;
              if(params->ftype & FLUIDFORCE_WV)
                {
                  wv.x -= -pDiff.y*mult*params->wvf;
                  wv.y -= pDiff.x*mult*params->wvf;
                }
           }
          
          // wv += float2{-pDiff.y, pDiff.x}*mult*params->wvf;
        }

      //if(u.y*params->gravity > 0 && abs(u.y) < abs(params->gravity))
        {
          u.y += dt*params->gravity;//*params->density; // GRAVITY
        }

        // for(int i = 0; i < params->numSources; i++)
        //   {
        //     float2 sDiff = pp - params->sources[i].pos;
        //     float sDist = length(sDiff);

        //     if(sDist < params->sources[i].radius)
        //       {
        //         u.x += sDiff.x * params->sources[i].force * dt;
        //         u.y += sDiff.y * params->sources[i].force * dt;
        //       }
        //   }
      
        src.d.dData[i]    = d;
        src.p.dData[i]    = p;
        src.vx.dData[i]   = u.x;
        src.vy.dData[i]   = u.y;
        src.wv.dData[i]   = wv;
    }
}

  __global__ void renderFluid_k(CudaFluid<float> src, CudaFieldTex dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = iy*src.size.x + ix;
      dst.dData[i] = float4{abs(src.vx.dData[i]), abs(src.vy.dData[i]), abs(src.wv.dData[i].x+src.wv.dData[i].y), 1.0f};
    }
}

__global__ void updateVelocity_k(CudaFluid<float> src, CudaFluid<float> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  int w = src.size.x; int h = src.size.y;

  if(ix < w && iy < h)
    {
      //int i = iy*w + ix;
      // TODO
    }
}


extern "C" void clearFluid(CudaFluid<float> fluid)
{
  dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
  dim3 grid((int)ceil(fluid.size.x/(float)BLOCKDIM_X), (int)ceil(fluid.size.y/(float)BLOCKDIM_Y));
  clearFluid_k<<<grid, threads>>>(fluid);
  getLastCudaError("====> ERROR: clearFluid_k failed!");
}

extern "C" void fillFluidCircle(CudaFluid<float> fluid)
{
  if(fluid.size.x > 0 && fluid.size.y > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(fluid.size.x/(float)BLOCKDIM_X), (int)ceil(fluid.size.y/(float)BLOCKDIM_Y));
      fillFluidCircle_k<<<grid, threads>>>(fluid);
      getLastCudaError("====> ERROR: fillFluidCircle_k failed!");
    }
}

extern "C" void fluidAdvection(CudaFluid<float> src, CudaFluid<float> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size == src.size)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      advect_k<<<grid, threads>>>(src, dst, BLOCKDIM_X/threads.y);
      getLastCudaError("====> ERROR: advect_k failed!");
    }
}

extern "C" void fluidDiffusion(CudaFluid<float> src, CudaFluid<float> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size == src.size)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      diffuse_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: diffuse_k failed!");
    }
}

extern "C" void fluidAddForces(CudaFluid<float> src)
{
  if(src.size.x > 0 && src.size.y > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      addForces_k<<<grid, threads>>>(src);
      getLastCudaError("====> ERROR: addForces_k failed!");
    }
}

extern "C" void fluidUpdateVel(CudaFluid<float> src, CudaFluid<float> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size == src.size)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      updateVelocity_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: updateVelocity_k failed!");
    }
}

extern "C" void renderFluid(CudaFluid<float> src, CudaFieldTex dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size == src.size)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      dst.map();
      renderFluid_k<<<grid, threads>>>(src, dst);
      //cudaDeviceSynchronize();
      dst.unmap();
      getLastCudaError("====> ERROR: renderFluid_k failed!");
    }
}
