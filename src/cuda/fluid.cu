#include "cudaField.hpp"
#include "cutools.hpp"
#include "cutools.cuh"

#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <cufft.h>
#include <helper_cuda.h>
#include <curand.h>
#include <curand_kernel.h>

#include "vector-operators.h"

#define BLOCKDIM_X 16
#define BLOCKDIM_Y 16

// fills texture data with solid color
__global__ void clearFluid_k(CudaFluid<float> fluid)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < fluid.size.x && iy < fluid.size.y)
    {
      int i = iy*fluid.size.x + ix;
      float2 p = float2{(ix-fluid.size.x/2.0f)/float(fluid.size.x), (iy-fluid.size.y/2.0f)/float(fluid.size.y) };
      fluid.d.dData[i]   = 1.0f;
      fluid.vx.dData[i]  = 0.0f;
      fluid.vy.dData[i]  = 0.0f;
      fluid.p.dData[i]   = 0.0f;//(p.x*p.x + p.y*p.y);
      fluid.div.dData[i] = 0.0f;//(p.x*p.x + p.y*p.y);
      fluid.wv.dData[i]  = float2{0.0f, 0.0f};
       //fluid.wv.dData[i] = float2{p.y, p.x}/8.0f;
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
      fluid.d.dData[i]   = 1.0f;//dist > 1.0f/4.0f ? dist : 0.0f;
      fluid.vx.dData[i]  = dist > 1.0f/4.0f ? 0.0f : 10.0*p.x*(1-dist);
      fluid.vy.dData[i]  = dist > 1.0f/4.0f ? 0.0f : 10.0*p.y*(1-dist);
      fluid.p.dData[i]   = 0.0f;//(p.x*p.x+ p.y*p.y);
      fluid.div.dData[i] = 0.0f;
      fluid.wv.dData[i]  = (dist > 1.0f/4.0f ? 0.0f : 1.0f) * float2{(p.x*p.x+ p.y*p.y), (p.x*p.x+ p.y*p.y)} * ((fluid.params.ftype & FLUIDFORCE_WV) ? fluid.params.wvf/4.0f : 0.0f);
    }
}

#define SIN_FREQ 8
__global__ void fillFluidPattern_k(CudaFluid<float> fluid)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < fluid.size.x && iy < fluid.size.y)
    {
      int i = iy*fluid.size.x + ix;
      float2 p = float2{(ix-fluid.size.x/2.0f)/float(fluid.size.x), (iy-fluid.size.y/2.0f)/float(fluid.size.y) };
      float dist = sqrt(p.x*p.x + p.y*p.y);
      // fluid.d.dData[i]   = ((1.0+sin(2.0*M_PI*SIN_FREQ*(ix/(float)fluid.size.x)))/2.0 *
      //                       (1.0+sin(2.0*M_PI*SIN_FREQ*(iy/(float)fluid.size.y)))/2.0);
      
      int xm = (ix % (fluid.size.x/SIN_FREQ)) < (fluid.size.x/SIN_FREQ/2) ? 1 : 0;// + 2*((iy/SIN_FREQ) % 2);
      int ym = (iy % (fluid.size.y/SIN_FREQ)) < (fluid.size.y/SIN_FREQ/2) ? 1 : 0; 
      fluid.d.dData[i]   = xm == ym ? 1.0 : 0.0;
      // fluid.vx.dData[i]  = dist > 1.0f/4.0f ? 0.0f : p.x;
      // fluid.vy.dData[i]  = dist > 1.0f/4.0f ? 0.0f : p.y;
      // fluid.p.dData[i]   = 0.0f;//(p.x*p.x+ p.y*p.y);
      // fluid.div.dData[i] = 0.0f;
      // fluid.wv.dData[i]  = (dist > 1.0f/4.0f ? 1.0f : -1.0f) * float2{(p.x*p.x+ p.y*p.y), (p.x*p.x+ p.y*p.y)} * ((fluid.params.ftype & FLUIDFORCE_WV) ? fluid.params.wvf/4.0f : 0.0f);
    }
}

__global__ void advect_k(CudaFluid<float> src, CudaFluid<float> dst, int lb)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  if(ix < src.size.x && iy < src.size.y)
    {
      int i = iy*src.size.x + ix;
      int w = src.size.x; int h = src.size.y;
      
      float dt = src.params.dt;
      
      float2 u0 = float2{tex2DD(src.vx.dData, ix+0.5f, iy+0.5f, w, h), tex2DD(src.vy.dData, ix+0.5f, iy+0.5f, w, h)};
      float2 u  = u0;
      
      float2 lastPos = float2{ float(ix) - dt*u.x + 0.5f, float(iy) - dt*u.y + 0.5f };
      //float2 lastPos = float2{ float(ix) - dt*u.x-0.5f, float(iy) - dt*u.y-0.5f };
      if(lastPos.x < 0.5f) { lastPos.x = 0.5f; } else if(lastPos.x > src.size.x-0.5f) { lastPos.x = src.size.x-0.5f; }
      if(lastPos.y < 0.5f) { lastPos.y = 0.5f; } else if(lastPos.y > src.size.y-0.5f) { lastPos.y = src.size.y-0.5f; }
      float2 nextPos = float2{ float(ix) + dt*u.x + 0.5f, float(iy) + dt*u.y + 0.5f };
      if(nextPos.x < 0.5f) { nextPos.x = 0.5f; } else if(nextPos.x > src.size.x-0.5f) { nextPos.x = src.size.x-0.5f; }
      if(nextPos.y < 0.5f) { nextPos.y = 0.5f; } else if(nextPos.y > src.size.y-0.5f) { nextPos.y = src.size.y-0.5f; }
      
      float2 lastU = float2{tex2DD(src.vx.dData, lastPos.x, lastPos.y, w, h), tex2DD(src.vy.dData, lastPos.x, lastPos.y, w, h) };
      float2 nextU = float2{tex2DD(src.vx.dData, nextPos.x, nextPos.y, w, h), tex2DD(src.vy.dData, nextPos.x, nextPos.y, w, h) };
      
      // ∂u/∂t = -[dot(u, ∇)](u) + v * ∇^2(u) + F
      // ∂ρ/∂t = -[dot(u, ∇)](ρ) + k * ∇^2(ρ) + S
      float2 lastUxp1 = float2{tex2DD(src.vx.dData, lastPos.x+1, lastPos.y, w, h), tex2DD(src.vy.dData, lastPos.x+1, lastPos.y, w, h) }; // x + 1
      float2 lastUxp2 = float2{tex2DD(src.vx.dData, lastPos.x+2, lastPos.y, w, h), tex2DD(src.vy.dData, lastPos.x+2, lastPos.y, w, h) }; // x + 2
      float2 lastUxn1 = float2{tex2DD(src.vx.dData, lastPos.x-1, lastPos.y, w, h), tex2DD(src.vy.dData, lastPos.x-1, lastPos.y, w, h) }; // x - 1
      float2 lastUxn2 = float2{tex2DD(src.vx.dData, lastPos.x-2, lastPos.y, w, h), tex2DD(src.vy.dData, lastPos.x-2, lastPos.y, w, h) }; // x - 2

      float2 lastUdx  = lastUxp1 + lastUxn1 - 2.0f*lastU;
      float2 lastUd2x = lastUxp2 + lastUxn2 - 4.0f*lastUxp1 - 4.0f*lastUxn1 + 6.0f*lastU;
      
      float2 lastUyp1 = float2{tex2DD(src.vx.dData, lastPos.x, lastPos.y+1, w, h), tex2DD(src.vy.dData, lastPos.x, lastPos.y+1, w, h) }; // y + 1
      float2 lastUyp2 = float2{tex2DD(src.vx.dData, lastPos.x, lastPos.y+2, w, h), tex2DD(src.vy.dData, lastPos.x, lastPos.y+2, w, h) }; // y + 2
      float2 lastUyn1 = float2{tex2DD(src.vx.dData, lastPos.x, lastPos.y-1, w, h), tex2DD(src.vy.dData, lastPos.x, lastPos.y-1, w, h) }; // y - 1
      float2 lastUyn2 = float2{tex2DD(src.vx.dData, lastPos.x, lastPos.y-2, w, h), tex2DD(src.vy.dData, lastPos.x, lastPos.y-2, w, h) }; // y - 2
      
      float2 lastUdy  = lastUyp1 + lastUyn1 - 2.0f*lastU;
      float2 lastUd2y = lastUyp2 + lastUyn2 - 4.0f*lastUyp1 - 4.0f*lastUyn1 + 6.0f*lastU;

      // float2 nextU = float2{tex2DD(src.vx.dData, nextPos.x, nextPos.y, w, h), tex2DD(src.vy.dData, nextPos.x, nextPos.y, w, h) };
      // float2 lastUdy = float2{tex2DD(src.vx.dData, lastPos.x+1,   lastPos.y+1,   w, h),
      //                         tex2DD(src.vy.dData, lastPos.x,     lastPos.y+1+1, w, h) } - lastU;
      // float2 lastUd2x = float2{tex2DD(src.vx.dData, lastPos.x+1+1, lastPos.y,     w, h),
      //                         tex2DD(src.vy.dData, lastPos.x+1,   lastPos.y+1,   w, h) } - lastU;
      // float2 lastUd2y = float2{tex2DD(src.vx.dData, lastPos.x+1,   lastPos.y+1,   w, h),
      //                         tex2DD(src.vy.dData, lastPos.x,     lastPos.y+1+1, w, h) } - lastU;
      float2 nextUdx = float2{tex2DD(src.vx.dData, nextPos.x+1+1, nextPos.y,     w, h),
                              tex2DD(src.vy.dData, nextPos.x+1,   nextPos.y+1,   w, h) } - nextU;
      float2 nextUdy = float2{tex2DD(src.vx.dData, nextPos.x+1,   nextPos.y+1,   w, h),
                              tex2DD(src.vy.dData, nextPos.x,     nextPos.y+1+1, w, h) } - nextU;

      float  d      = tex2DD(src.d.dData,  lastPos.x, lastPos.y, w, h);
      float  p      = tex2DD(src.p.dData,  lastPos.x, lastPos.y, w, h);
      float2 lastWv = tex2DD(src.wv.dData, lastPos.x, lastPos.y, w, h);

      
      u = lastU; //(lastU + u)/2;

      // VISCOSITY
      //   NOTE: uses absolute value of timestep to avoid explosion (set viscosity < 0 for same effect as -timestep)
      if(src.params.applyVisc && ix > 0 && iy > 0 && ix < src.size.x-1 && iy < src.size.y-1)
        {
          float2 du_dt = float2{0.0f,0.0f}; //-0.25*(u.x*nextUdx + u.y*nextUdy);
          du_dt += src.params.viscosity*(lastUd2x + lastUd2y);
          // float2 du_dt = -0.5*(abs(u.x)*lastUdx + abs(u.y)*lastUdy) + src.params.viscosity*(lastUd2x + lastUd2y);
          //float2 du_dt = {0.0, 0.0}; //-src.params.viscosity*(lastUd2x + lastUd2y);
          u -= abs(dt)*du_dt;//*d;
        }
      
      if(isnan(u.x))  { u.x  = 0.0; }
      if(isnan(u.y))  { u.y  = 0.0; }
      
      //float  lastD  = tex2DD(src.d.dData, lastPos.x, lastPos.y, w, h);
      //float2 dd_dt = -(d*lastUdx + d*lastUdy) + src.params.viscosity*(lastUd2x + lastUd2y);
      
      // float2 nextWv = tex2DD(src.wv.dData, nextPos.x, nextPos.y, w, h);

      // float newD      = lastD/(1.0 + abs(dt))*0.99;//*params->dissipation);
      // float newP      = lastP/(1.0 + abs(dt))*0.99;

      float newD = d;
      float newP = p;
      
      if(isnan(newD)) { newD = 0.0; }
      if(isnan(newP)) { newP = 0.0; }
      
      // u = (u0 + lastU - nextU);
      
      // u.x += u0.y*newP*dt;
      // u.y += u0.x*newP*dt;

      // newP += length(u)*dt;
#define MAX_UX (float)64.0f
#define MAX_UY (float)64.0f
      
      u.x = (abs(u.x) > MAX_UX ? (u.x < 0.0 ? -1.0f : 1.0f)*MAX_UX : u.x);
      u.y = (abs(u.y) > MAX_UY ? (u.y < 0.0 ? -1.0f : 1.0f)*MAX_UY : u.y);
      // u.x      = (u.x<0.0f?-1.0f:1.0f)*min(abs(u.x), (float)src.size.x/2.0f);
      // u.y      = (u.y<0.0f?-1.0f:1.0f)*min(abs(u.y), (float)src.size.y/2.0f);
      //newP     = (newP < 0.0f ? 0.0f : min(abs(u.y), 4.0f));
      
      float2 wv00    = float2{tex2DD(src.wv.dData, ix+0.5f, iy+0.5f, w, h)}; //texGet(src.wv.dData, ix, iy, w, h);
      //float2 wv      = (lastWv+nextWv)/2.0f;
      float2 wv      = lastWv; //wv00; //(lastWv);// (wv00 + lastWv - nextWv);
      if(src.params.applyChaos && src.params.chaos != 0.0)
        {
          // float2 wvNX = tex2DD(src.wv.dData, lastPos.x+0.5f-1.0f, lastPos.y+0.5f,      w, h);//texGet(src.wv.dData, ix-1, iy, w, h);
          // float2 wvPX = tex2DD(src.wv.dData, lastPos.x+0.5f+1.0f, lastPos.y+0.5f,      w, h);//texGet(src.wv.dData, ix+1, iy, w, h);
          // float2 wvNY = tex2DD(src.wv.dData, lastPos.x+0.5f,      lastPos.y+0.5f-1.0f, w, h);//texGet(src.wv.dData, ix, iy-1, w, h);
          // float2 wvPY = tex2DD(src.wv.dData, lastPos.x+0.5f,      lastPos.y+0.5f+1.0f, w, h);//texGet(src.wv.dData, ix, iy+1, w, h);
          
          // float2 dwvX = ((wvPX - wv00) - (wv00 - wvNX))/(2.0);
          // float2 dwvY = ((wvPY - wv00) - (wv00 - wvNY))/(2.0);
          
          // float2 dwvDt = cMult(dwvX, dwvY);
          //float2 dwvDt = float2{dwvX.x*dwvY.x - dwvX.y*dwvY.y};
          
          // wv = lastWv;
          // wv += (cMult(cDiv(wv, u/length(u)), cDiv(wv, u/length(u))))*dt;
          // wv.x = (wv.x >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.x), 4.0f);
          // wv.y = (wv.y >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.y), 4.0f);
          // if(u.x > 0 && !isnan(u.x) and u.y > 0 && !isnan(u.y))
          //   {
          //     float2 wv2 = cDiv(u, u0);
          //     u -= dt*src.params.wvf*wv*wv00*(cMult(wv2, wv2)+u); //+= dt*src.params.wvf*cMult(u0, cDiv(u, wv00));
          //   }
          // else
          //   {
          //     u += dt*src.params.wvf*wv; //+= dt*src.params.wvf*cMult(u0, cDiv(u, wv00));
          //   }

          
          // float2 norm = normalize(u);
          // wv += (cMult(cDiv(wv, norm), cDiv(wv, norm)))*dt;
          // wv.x = (wv.x >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.x), 4.0f);
          // wv.y = (wv.y >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.y), 4.0f);
          // u += src.params.chaos*(cMult(wv, norm/2.0f))*dt;

          float2 norm = normalize(u);
          if(isnan(norm.x) || norm.x == 0 || isnan(norm.y) || norm.y == 0) { norm = float2{u.x < 0 ? -1.0f : 1.0f, u.y < 0 ? -1.0f : 1.0f}; }

          wv.x += dt*src.params.chaos*newP;
          // wv -= dt*src.params.chaos*(cDiv(u-nextU, lastU-u));
          
          //float2 oriented = (norm.x == 0 && norm.y == 0 ? wv : cDiv(norm, wv)); // <-- more correct?
          float2 oriented = (norm.x == 0 && norm.y == 0 ? wv : cDiv(wv, norm)); // <-- realistic vortices!
          if(isnan(oriented.x) || isnan(oriented.y)) { oriented = wv; }
          
          wv += (cMult(oriented, oriented))*dt;//cDiv(wv, norm), cDiv(wv, norm)))*dt;
          // wv += (u0-u)*dt;
          
          //wv.x += newD*newP;
          
          wv.x = (wv.x >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.x), 4.0f);
          wv.y = (wv.y >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.y), 4.0f);
          u += dt*src.params.chaos*cMult(oriented, u);

          //
          // //// VERY FLUID-LIKE; REALISM -- WAVE VECTOR APPEARS 3D SHADED ////
          // float2 norm = normalize(u);
          // wv += (cMult(cDiv(wv, norm), cDiv(wv, norm)))*dt;
          // wv.x = (wv.x >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.x), 4.0f);
          // wv.y = (wv.y >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.y), 4.0f);
          // u += src.params.chaos*wv*dt;
          // //// COOL WV FIRE ////
          // wv += (cMult(cDiv(wv, u), cDiv(wv, u)))*dt;
          // wv.x = (wv.x >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.x), 4.0f);
          // wv.y = (wv.y >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.y), 4.0f);
          // u += src.params.chaos*wv*dt;
        }

      if(ix == 0)                 { u.x = 0; wv.x = 0; } //  abs(u.x); 
      else if(ix == src.size.x-1) { u.x = 0; wv.x = 0; } // -abs(u.x); 
      if(iy == 0)                 { u.y = 0; wv.x = 0; } //  abs(u.y); 
      else if(iy == src.size.y-1) { u.y = 0; wv.x = 0; } // -abs(u.y); 
      
      dst.vx.dData[i]  = u.x;//(ix == 0 || ix == src.size.x-1 || iy == 0 || iy == src.size.y-1) ? 0.0f : u.x;
      dst.vy.dData[i]  = u.y;//(ix == 0 || ix == src.size.x-1 || iy == 0 || iy == src.size.y-1) ? 0.0f : u.y;
      dst.wv.dData[i]  = wv;
      dst.d.dData[i]   = newD;
      dst.p.dData[i]   = newP;
      dst.div.dData[i] = src.div.dData[i];
    }
}

__global__ void diffuse_k(CudaFluid<float> src, CudaFluid<float> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  int kRad = src.params.diffuseRad;
  if(ix < src.size.x-kRad && iy < src.size.y-kRad && ix > kRad && iy > kRad)
    {
      int i = iy*src.size.x + ix;
      float dt = src.params.dt;
      
      float2 u    = float2{0.0f, 0.0f};
      float2 wv   = float2{0.0f, 0.0f};
      //float  d    = 0.0f;
      //float  p    = 0.0f;
      float  mult = 0.0f;
      
      for(int x = -kRad; x <= kRad; x++)
        for(int y = -kRad; y <= kRad; y++)
          {
            float dist = (x==0 && y==0) ? 1.0f : sqrt(float(x*x + y*y)) / dt;
            mult += 1.0f/dist;
            int li = src.size.x*(iy+y) + ix+x;
            u.x += src.vx.dData[li]/dist;
            u.y += src.vy.dData[li]/dist;
            wv  += src.wv.dData[li]/dist;
            // d   += src.d.dData [li]/dist;
            // p   += src.p.dData [li]/dist;
          }
      dst.vx.dData[i]  = u.x / mult;
      dst.vy.dData[i]  = u.y / mult;
      dst.wv.dData[i]  = wv  / mult;
      // dst.d.dData[i]   = d   / mult;
      // dst.p.dData[i]   = p   / mult;
      // dst.div.dData[i] = src.div.dData[i];
    }
}

__device__ int IX(int ix, int iy, int SX) { return (iy*SX + ix); }

// f --> field component data
// b --> boundary int?
__device__ void setBounds(float *f, int b, int SX, int SY)
{
  // for(int i = 0; i < S; i++)
  //   {
  //     f[IX(0,    i,    SX)] = (b == 1 ? –f[IX(1,  i,  SX)] : f[IX(1,  i, SX)]);
  //     f[IX(SX+1, i,    SX)] = (b == 1 ? –f[IX(SX, i,  SX)] : f[IX(SX, i, SX)]);
  //     f[IX(i,    0,    SX)] = (b == 2 ? –f[IX(i,  1,  SX)] : f[IX(i,  1, SX)]);
  //     f[IX(i,    SY+1, SX)] = (b == 2 ? –f[IX(i,  SY, SX)] : f[IX(i,  SY, SX)]);
  //   }
  // f[IX(0,    0,    SX)] = 0.5*(f[IX(1,  0,    SX)] + f[IX(0,    1, SX)]);
  // f[IX(0,    SY+1, SX)] = 0.5*(f[IX(1,  SY+1, SX)] + f[IX(0,    SY, SX)]);
  // f[IX(SX+1, 0,    SX)] = 0.5*(f[IX(SX, 0,    SX)] + f[IX(SX+1, 1, SX)]);
  // f[IX(SX+1, SY+1, SX)] = 0.5*(f[IX(SX, SY+1, SX)] + f[IX(SX+1, SY, SX)]);
}


__global__ void preProject_k(CudaFluid<float> src, CudaFluid<float> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  if(ix < src.size.x && iy < src.size.y)
    {
      int i = iy*src.size.x + ix;

      float hx = 1.0f/src.size.x;
      float hy = 1.0f/src.size.y;
      int   SX = src.size.x;
      //int   SY = src.size.y;
      
      dst.vx.dData[i]  = src.vx.dData[i];
      dst.vy.dData[i]  = src.vy.dData[i];
      dst.wv.dData[i]  = src.wv.dData[i];
      dst.d.dData[i]   = src.d.dData[i];


      if(ix > 0 && iy > 0 && ix < src.size.x-1 && iy < src.size.y-1)
        {
          dst.div.dData[i] = -0.5f*(hx*(src.vx.dData[IX(ix+1, iy, SX)] - src.vx.dData[IX(ix-1, iy, SX)]) +
                                    hy*(src.vy.dData[IX(ix, iy+1, SX)] - src.vy.dData[IX(ix, iy-1, SX)]));
          dst.p.dData[i] = 0.0f;
        }
      
      if(ix == 0)                 { dst.vx.dData[i] = 0; }// abs(dst.vx.dData[i])/2.0f; }
      else if(ix == src.size.x-1) { dst.vx.dData[i] = 0; }//-abs(dst.vx.dData[i])/2.0f; }
      if(iy == 0)                 { dst.vy.dData[i] = 0; }// abs(dst.vy.dData[i])/2.0f; }
      else if(iy == src.size.y-1) { dst.vy.dData[i] = 0; }//-abs(dst.vy.dData[i])/2.0f; }
    }
}

__global__ void postProject_k(CudaFluid<float> src, CudaFluid<float> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  if(ix < src.size.x && iy < src.size.y)
    {
      int i = iy*src.size.x + ix;
      float hx = 1.0f/src.size.x;
      float hy = 1.0f/src.size.y;
      int   SX = src.size.x;

      if(ix > 0 && iy > 0 && ix < src.size.x-1 && iy < src.size.y-1)
        {
          dst.vx.dData[i] -= 0.5f*(dst.p.dData[IX(ix+1, iy,   SX)] - dst.p.dData[IX(ix-1, iy,   SX)]) / hx;
          dst.vy.dData[i] -= 0.5f*(dst.p.dData[IX(ix,   iy+1, SX)] - dst.p.dData[IX(ix,   iy-1, SX)]) / hy;
        }
      if(ix == 0)                 { dst.vx.dData[i] = 0; }// abs(dst.vx.dData[i])/2.0f; }
      else if(ix == src.size.x-1) { dst.vx.dData[i] = 0; }//-abs(dst.vx.dData[i])/2.0f; }
      if(iy == 0)                 { dst.vy.dData[i] = 0; }// abs(dst.vy.dData[i])/2.0f; }
      else if(iy == src.size.y-1) { dst.vy.dData[i] = 0; }//-abs(dst.vy.dData[i])/2.0f; }
    }
}

// project velocities for mass conservation
__global__ void project_k(CudaFluid<float> src, CudaFluid<float> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  int   i  = iy*src.size.x + ix;
  float hx = 1.0f/src.size.x;
  float hy = 1.0f/src.size.y;
  int   SX = src.size.x;
  
  //if(ix > 0 && iy > 0 && ix < src.size.x-1 && iy < src.size.y-1)
  if(ix > 0 && iy > 0 && ix < src.size.x-1 && iy < src.size.y-1)
    {
      dst.p.dData[i] = (dst.div.dData[IX(ix, iy, SX)] +
                        dst.p.dData[IX(ix-1, iy, SX)] + dst.p.dData[IX(ix+1, iy, SX)] +
                        dst.p.dData[IX(ix, iy-1, SX)] + dst.p.dData[IX(ix, iy+1, SX)]) / 4.0f;
    }
  if(ix == 0)                 { dst.vx.dData[i] =  abs(dst.vx.dData[i])/2.0f; }
  else if(ix == src.size.x-1) { dst.vx.dData[i] = -abs(dst.vx.dData[i])/2.0f; }
  if(iy == 0)                 { dst.vy.dData[i] =  abs(dst.vy.dData[i])/2.0f; }
  else if(iy == src.size.y-1) { dst.vy.dData[i] = -abs(dst.vy.dData[i])/2.0f; }
}
// fix boundary values
__global__ void fixBounds_k(CudaFluid<float> src, CudaFluid<float> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  //int   i  = iy*src.size.x + ix;
  float hx = 1.0f/src.size.x;
  float hy = 1.0f/src.size.y;
  //int   SX = src.size.x;
  //int   SY = src.size.y;
      
  if(ix > 0 && iy > 0 && ix < src.size.x-1 && iy < src.size.y-1)
    {

    }
}

__forceinline__ __device__ float lineDist(float2 l1, float2 l2, float2 p0)
{
  return abs((l2.y - l1.y)*p0.x - (l2.x - l1.x)*p0.y + l2.x*l1.y - l2.y*l1.x) / sqrt((l2.y-l1.y)*(l2.y-l1.y) + (l2.x-l1.x)*(l2.x-l1.x));
}

__forceinline__ __device__ float2 closestPoint(float2 l1, float2 l2, float2 p0)
{
  float2 n = normalize(l2 - l1);
  float2 v = normalize(p0 - l1);
  float  d = dot(v, n);
  return l1 + n*d;
}

__global__ void addForces_k(CudaFluid<float> src, CudaFluid<float> dst, CudaFluid<float> prev)
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
      float mvLen = length(mv);
      
      float  d    = src.d.dData[i];
      float  p    = src.p.dData[i];
      float2 u    = float2{src.vx.dData[i], src.vy.dData[i]};
      float2 norm = u / length(u);
      float2 wv   = src.wv.dData[i];
      
      float2 pp = float2{float(ix + 0.5f)/w, float(iy + 0.5f)/h}; // pixel pos

      float2 diff; float dist;
      if(mvLen > 0.0f)
        { // calculate force based on distance from mouse movement line segment
          if((dot(normalize(forcePoint2 - forcePoint1), normalize(pp - forcePoint1)) <= 0) ||
             (dot(normalize(forcePoint1 - forcePoint2), normalize(pp - forcePoint2)) <= 0)) // (dot(v1, v2) < 0) --> angle < pi/2
            {
              float distp1 = length(pp - forcePoint1); // distance to last mouse pos
              float distp2 = length(pp - forcePoint2); // distance to current mouse pos
              if(distp1 < distp2)
                { diff = pp - forcePoint1; dist = distp1; }
              else
                { diff = pp - forcePoint2; dist = distp2; }
            } // part of semicircle caps at line endpoints (TODO: improve?)
          else
            { // part of rectangular overlap area -- get perpendicular distance to line
              dist = lineDist(forcePoint1, forcePoint2, pp);
              //diff = pp - closestPoint(forcePoint1, forcePoint2, pp);

              diff = normalize(mv);
              diff = float2{diff.y, -diff.x};
              if(dot(diff, normalize(forcePoint1 - pp)) < 0.0f || dot(diff, normalize(forcePoint2 - pp)) < 0.0f) { diff = -diff; }
              diff *= dist;
            }
        }
      else if(mvLen == 0.0f)
        { // only one point
          diff = pp - forcePoint2;
          dist = length(diff);
        }
      
      float  dist2 = dist*dist;
      float2 pDiff = pp - forcePoint2;
      // if(params->movedLast && length(pp-forcePoint1) < params->forceRad)
      //   { pDiff -= prev.f.dData[i]; }
      float  pDist = length(pDiff);

      if(params->mdown)
        {
          // only applies force around current mouse pos
          float mult = 0.0;
          if(pDist <= params->forceRad)
            { mult = (1.0f - smoothstep(0.0f, 1.0f, pDist/params->forceRad)) / sqrt(params->forceRad); }

          // applies force between previous and current mouse pos
          float mult2 = (1.0f - smoothstep(0.0f, 1.0f, dist/params->forceRad)) / (params->forceRad);

          if(dist < params->forceRad)
            {
              if(params->ftype & FLUIDFORCE_PUSH)
                { // push force
                  //float mult2 = exp(-dist/(2*FORCE_SIGMA*FORCE_SIGMA));
                  //mult2 = mult2*mult2;
                  u.x += mv.x * mult2 * params->vfPush;
                  u.y += mv.y * mult2 * params->vfPush;
                  if(params->ftype & FLUIDFORCE_WV)
                    {
                      wv += float2{0.1,0.3} * mvLen * mult2 * params->wvf;
                      //wv.y += mvLen * mult2 * params->wvf;
                    }
                }
          
              if(params->ftype & FLUIDFORCE_DENSITY)  { d += mult2*params->df; }
              if(params->ftype & FLUIDFORCE_PRESSURE) { p += mult2*params->pf; }
          
              if(params->ftype & FLUIDFORCE_IN)
                { // inward forceRadius
                  u.x += -pDiff.x*mult2*params->vfIn;
                  u.y += -pDiff.y*mult2*params->vfIn;
                  if(params->ftype & FLUIDFORCE_WV)
                    {
                      wv.x += -pDist*mult2*params->wvf;
                      // wv.y += -pDist*mult*params->wvf;
                    }
                }
              if(params->ftype & FLUIDFORCE_OUT)
                { // outward force
                  u.x += pDiff.x*mult2*params->vfOut;
                  u.y += pDiff.y*mult2*params->vfOut;
                  if(params->ftype & FLUIDFORCE_WV)
                    {
                      wv.x += pDist*mult2*params->wvf;
                      // wv.y += pDist*mult*params->wvf;
                    }
                }
              if(params->ftype & FLUIDFORCE_CW)
                { // clockwise force
                  u.x += -pDiff.y*mult2*params->vfCw;
                  u.y += pDiff.x*mult2*params->vfCw;
                  if(params->ftype & FLUIDFORCE_WV)
                    {
                      //wv.x += -pDiff.y*norm.x*mult*params->wvf;
                      wv.y += pDist*mult2*params->wvf;
                    }
                }
              if(params->ftype & FLUIDFORCE_CCW)
                { // counter-clockwise
                  u.x += pDiff.y*mult2*params->vfCcw;
                  u.y += -pDiff.x*mult2*params->vfCcw;
                  if(params->ftype & FLUIDFORCE_WV)
                    {
                      //wv.x += pDiff.y*norm.x*mult*params->wvf;
                      wv.y += -pDist*mult2*params->wvf;
                    }
                }
            }
        }

      //if(u.y*params->gravity > 0 && abs(u.y) < abs(params->gravity))
      {
        u.y += dt*params->gravity;//*params->density; // GRAVITY
      }
      
      dst.vx.dData[i]   = u.x;
      dst.vy.dData[i]   = u.y;
      dst.wv.dData[i]   = wv;
      dst.d.dData[i]    = d;
      dst.p.dData[i]    = p;
      dst.div.dData[i]  = src.div.dData[i];
      //dst.f.dData[i] = u - float2{src.vx.dData[i], src.vy.dData[i]};
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
      int i = iy*w + ix;
      // just copy for now
      dst.vx.dData[i]  = src.vx.dData[i];
      dst.vy.dData[i]  = src.vy.dData[i];
      dst.wv.dData[i]  = src.wv.dData[i];
      dst.d.dData[i]   = src.d.dData[i];
      dst.p.dData[i]   = src.p.dData[i];
      dst.div.dData[i] = src.div.dData[i];
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

extern "C" void fillFluidPattern(CudaFluid<float> fluid)
{
  if(fluid.size.x > 0 && fluid.size.y > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(fluid.size.x/(float)BLOCKDIM_X), (int)ceil(fluid.size.y/(float)BLOCKDIM_Y));
      fillFluidPattern_k<<<grid, threads>>>(fluid);
      getLastCudaError("====> ERROR: fillFluidPattern_k failed!");
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

extern "C" void fluidProject(CudaFluid<float> src, CudaFluid<float> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size == src.size)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      preProject_k<<<grid, threads>>>(src, dst);
      for(int i = 0; i < src.params.projectIter; i++) { project_k<<<grid, threads>>>(src, dst); }
      postProject_k<<<grid, threads>>>(src, dst);

      // CudaFluid<float> *psrc = &src;
      // CudaFluid<float> *pdst = &dst;
      
      // preProject_k<<<grid, threads>>>(*psrc, *pdst); std::swap(psrc, pdst);
      // for(int i = 0; i < src.params.projectIter; i++) { project_k<<<grid, threads>>>(*psrc, *pdst); std::swap(psrc, pdst); }
      // postProject_k<<<grid, threads>>>(*psrc, *pdst);

      // if(pdst == &src)
      //   {
      //     postProject_k<<<grid, threads>>>(*pdst, *psrc);
      //   }
      
      getLastCudaError("====> ERROR: project_k failed!");
    }
}

extern "C" void fluidAddForces(CudaFluid<float> src, CudaFluid<float> dst, CudaFluid<float> prev)
{
  if(src.size.x > 0 && src.size.y > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      addForces_k<<<grid, threads>>>(src, dst, prev);
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
      dst.unmap();
      getLastCudaError("====> ERROR: renderFluid_k failed!");
    }
}
