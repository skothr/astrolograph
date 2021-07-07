#include "hyper-field.hpp"
#include "cutools.hpp"
#include "cutools.cuh"
#include "raytrace.cuh"
#include "vector-operators.h"

#include <stdio.h>
#include <stdlib.h>
#include <cuda_runtime.h>
#include <cufft.h>
#include <helper_cuda.h>
#include <curand.h>
#include <curand_kernel.h>


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// 2D ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define BLOCKDIM_X 16
#define BLOCKDIM_Y 16



// fills texture data with solid color
__global__ void clearFluidH2_k(HyperFluid<2> fluid)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix >= 0 && iy >= 0 && ix < fluid.size.x && iy < fluid.size.y)
    {
      int i = fluid.pindex(ix, iy);
      //float2 p = float2{(ix-fluid.size.x/2.0f)/float(fluid.size.x), (iy-fluid.size.y/2.0f)/float(fluid.size.y) };
      fluid.vel.dData[i] = float2{0.0f, 0.0f};
    }
}

__global__ void fillFluidCircleH2_k(HyperFluid<2> fluid)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix >= 0 && iy >= 0 && ix < fluid.size.x && iy < fluid.size.y)
    {
      int i = fluid.pindex(ix, iy);
      float2 p = float2{(ix-fluid.size.x/2.0f)/float(fluid.size.x), (iy-fluid.size.y/2.0f)/float(fluid.size.y) };
      float dist = sqrt(p.x*p.x + p.y*p.y);
      fluid.vel.dData[i] = (dist > 1.0f/4.0f ? float2{0.0f, 0.0f} : p);
    }
}

#define SIN_FREQ 8
__global__ void fillFluidPatternH2_k(HyperFluid<2> fluid)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix >= 0 && iy >= 0 && ix < fluid.size.x && iy < fluid.size.y)
    {
      int i = fluid.pindex(ix, iy);
      float2 p = float2{(ix-fluid.size.x/2.0f)/float(fluid.size.x), (iy-fluid.size.y/2.0f)/float(fluid.size.y) };
      float dist = sqrt(p.x*p.x + p.y*p.y);

      int2 c = int2{(fluid.size.x/SIN_FREQ/2), (fluid.size.y/SIN_FREQ/2)};
      
      int xm = (ix % (fluid.size.x/SIN_FREQ) < c.x ? 1 : 0);
      int ym = (iy % (fluid.size.y/SIN_FREQ) < c.y ? 1 : 0);
      fluid.vel.dData[i] = (xm == ym ? float2{1.0f, 1.0f} : float2{0.0f, 0.0f});
      // fluid.vel.dData[i] = float2{ix/(float)fluid.size.x, iy/(float)fluid.size.y};
    }
}



__global__ void advectH_k(HyperFluid<2> src, HyperFluid<2> dst, int lb)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  if(ix < src.size.x && iy < src.size.y && ix < dst.size.x && iy < dst.size.y)
    {
      int i = src.pindex(ix, iy);
      int w = src.size.x; int h = src.size.y;
      
      float dt = src.params.dt;
      
      float2 u0 = tex2DD(src.vel.dData, ix+0.5f, iy+0.5f, w, h);
      float2 u = u0;
      
      float2 lastPos = float2{ float(ix) - dt*u.x + 0.5f, float(iy) - dt*u.y + 0.5f };
      if(lastPos.x < 0.5f) { lastPos.x = 0.5f; } else if(lastPos.x > src.size.x-0.5f) { lastPos.x = src.size.x-0.5f; }
      if(lastPos.y < 0.5f) { lastPos.y = 0.5f; } else if(lastPos.y > src.size.y-0.5f) { lastPos.y = src.size.y-0.5f; }
      // float2 nextPos = float2{ float(ix) + dt*u.x + 0.5f, float(iy) + dt*u.y + 0.5f };
      // if(nextPos.x < 0.5f) { nextPos.x = 0.5f; } else if(nextPos.x > src.size.x-0.5f) { nextPos.x = src.size.x-0.5f; }
      // if(nextPos.y < 0.5f) { nextPos.y = 0.5f; } else if(nextPos.y > src.size.y-0.5f) { nextPos.y = src.size.y-0.5f; }
      
      float2 lastU = tex2DD(src.vel.dData, lastPos.x, lastPos.y, w, h);
      
      // ∂u/∂t = -[dot(u, ∇)](u) + v * ∇^2(u) + F
      // ∂ρ/∂t = -[dot(u, ∇)](ρ) + k * ∇^2(ρ) + S
      float2 lastUxp1 = tex2DD(src.vel.dData, lastPos.x+1, lastPos.y, w, h); // x + 1
      float2 lastUxp2 = tex2DD(src.vel.dData, lastPos.x+2, lastPos.y, w, h); // x + 2
      float2 lastUxn1 = tex2DD(src.vel.dData, lastPos.x-1, lastPos.y, w, h); // x - 1
      float2 lastUxn2 = tex2DD(src.vel.dData, lastPos.x-2, lastPos.y, w, h); // x - 2

      float2 lastUdx  = lastUxp1 + lastUxn1 - 2.0f*lastU;
      float2 lastUd2x = lastUxp2 + lastUxn2 - 4.0f*lastUxp1 - 4.0f*lastUxn1 + 6.0f*lastU;
      
      float2 lastUyp1 = tex2DD(src.vel.dData, lastPos.x, lastPos.y+1, w, h); // y + 1
      float2 lastUyp2 = tex2DD(src.vel.dData, lastPos.x, lastPos.y+2, w, h); // y + 2
      float2 lastUyn1 = tex2DD(src.vel.dData, lastPos.x, lastPos.y-1, w, h); // y - 1
      float2 lastUyn2 = tex2DD(src.vel.dData, lastPos.x, lastPos.y-2, w, h); // y - 2
      
      float2 lastUdy  = lastUyp1 + lastUyn1 - 2.0f*lastU;
      float2 lastUd2y = lastUyp2 + lastUyn2 - 4.0f*lastUyp1 - 4.0f*lastUyn1 + 6.0f*lastU;

      // float2 nextU = float2{tex2DD(src.vx.dData, nextPos.x, nextPos.y, w, h), tex2DD(src.vy.dData, nextPos.x, nextPos.y, w, h) };      
      // float2 lastUdy = float2{tex2DD(src.vx.dData, lastPos.x+1,   lastPos.y+1,   w, h),
      //                         tex2DD(src.vy.dData, lastPos.x,     lastPos.y+1+1, w, h) } - lastU;
      // float2 lastUd2x = float2{tex2DD(src.vx.dData, lastPos.x+1+1, lastPos.y,     w, h),
      //                         tex2DD(src.vy.dData, lastPos.x+1,   lastPos.y+1,   w, h) } - lastU;
      // float2 lastUd2y = float2{tex2DD(src.vx.dData, lastPos.x+1,   lastPos.y+1,   w, h),
      //                         tex2DD(src.vy.dData, lastPos.x,     lastPos.y+1+1, w, h) } - lastU;
      // float2 nextUdx = float2{tex2DD(src.vx.dData, nextPos.x+1+1, nextPos.y,     w, h),
      //                         tex2DD(src.vy.dData, nextPos.x+1,   nextPos.y+1,   w, h) } - nextU;
      // float2 nextUdy = float2{tex2DD(src.vx.dData, nextPos.x+1,   nextPos.y+1,   w, h),
      //                         tex2DD(src.vy.dData, nextPos.x,     nextPos.y+1+1, w, h) } - nextU;

      float2 du_dt = -(u.x*lastUdx + u.y*lastUdy) + src.params.viscosity*(lastUd2x + lastUd2y);
      u += dt*du_dt;
      
      if(isnan(u.x))  { u.x  = 0.0; }
      if(isnan(u.y))  { u.y  = 0.0; }
      
      float  d  = tex2DD(src.d.dData, lastPos.x, lastPos.y, w, h);
      //float  lastD  = tex2DD(src.d.dData, lastPos.x, lastPos.y, w, h);
      //float2 dd_dt = -(d*lastUdx + d*lastUdy) + src.params.viscosity*(lastUd2x + lastUd2y);
      
      float  p      = tex2DD(src.p.dData,  lastPos.x, lastPos.y, w, h);
      float2 lastWv = tex2DD(src.wv.dData, lastPos.x, lastPos.y, w, h);
      // float2 nextWv = tex2DD(src.wv.dData, nextPos.x, nextPos.y, w, h);

      // float newD      = lastD/(1.0 + abs(dt))*0.99;//*params->dissipation);
      // float newP      = lastP/(1.0 + abs(dt))*0.99;

      float newD = d;
      float newP = p;
      
      if(isnan(newD)) { newD = 0.0; }
      if(isnan(newP)) { newP = 0.0; }
         
      // u = (lastU+nextU)/2.0f;
      // u = (u0 + lastU - nextU);
      
      // u.x += u0.y*newP*dt;
      // u.y += u0.x*newP*dt;

      // newP += length(u)*dt;

      u.x      = (u.x<0.0f?-1.0f:1.0f)*min(abs(u.x), 4.0f);
      u.y      = (u.y<0.0f?-1.0f:1.0f)*min(abs(u.y), 4.0f);
      newP     = (newP < 0.0f ? 0.0f : min(abs(u.y), 4.0f));
      
      float2 wv00    = float2{tex2DD(src.wv.dData, ix+0.5f, iy+0.5f, w, h)}; //texGet(src.wv.dData, ix, iy, w, h);
      //float2 wv      = (lastWv+nextWv)/2.0f;
      float2 wv      = (lastWv);// (wv00 + lastWv - nextWv);
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
          float2 oriented = cDiv(wv, norm);
          wv += (cMult(oriented, oriented))*dt;//cDiv(wv, norm), cDiv(wv, norm)))*dt;
          // wv += (u0-u)*dt;
          wv.x = (wv.x >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.x), 4.0f);
          wv.y = (wv.y >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.y), 4.0f);
          u += dt*src.params.chaos*wv;
          
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

      dst.vel.dData[i] = u; //src.vel.dData[i];
      dst.d.dData[i]   = newD; //src.d.dData[i];
      dst.p.dData[i]   = newP; //src.p.dData[i];
      dst.wv.dData[i]  = wv; //src.wv.dData[i];
      dst.div.dData[i] = src.div.dData[i];
    }
}

__global__ void diffuseH_k(HyperFluid<2> src, HyperFluid<2> dst)
{
  //int ix = blockIdx.x*blockDim.x + threadIdx.x;
  //int iy = blockIdx.y*blockDim.y + threadIdx.y;

  // int kRad = 0; //src.params.diffuseRad;
  // if(ix < src.size.x-kRad && iy < src.size.y-kRad && ix > kRad && iy > kRad)
  //   {
  //     int i = src.pindex(ix, iy);
  //     //float dt = src.params.dt;
      
  //     float2 u    = float2{0.0f, 0.0f};
  //     float  mult = 0.0f;
      
  //     for(int x = -kRad; x <= kRad; x++)
  //       for(int y = -kRad; y <= kRad; y++)
  //         {
  //           float dist = (x==0 && y==0) ? 1.0f : sqrt(float(x*x + y*y)); // / dt;
  //           mult += 1.0f/dist;
  //           int li = src.pindex(ix+x, iy+y);
  //           u += src.vel.dData[li]/dist;
  //         }
  //     dst.vel.dData[i] = src.vel.dData[i];
  //   }
}

// __device__ int IX(int ix, int iy, int SX) { return (iy*SX + ix); }

// f --> field component data
// b --> boundary int?
// __device__ void setBounds(float *f, int b, int SX, int SY)
// {
//   // for(int i = 0; i < S; i++)
//   //   {
//   //     f[IX(0,    i,    SX)] = (b == 1 ? –f[IX(1,  i,  SX)] : f[IX(1,  i, SX)]);
//   //     f[IX(SX+1, i,    SX)] = (b == 1 ? –f[IX(SX, i,  SX)] : f[IX(SX, i, SX)]);
//   //     f[IX(i,    0,    SX)] = (b == 2 ? –f[IX(i,  1,  SX)] : f[IX(i,  1, SX)]);
//   //     f[IX(i,    SY+1, SX)] = (b == 2 ? –f[IX(i,  SY, SX)] : f[IX(i,  SY, SX)]);
//   //   }
//   // f[IX(0,    0,    SX)] = 0.5*(f[IX(1,  0,    SX)] + f[IX(0,    1, SX)]);
//   // f[IX(0,    SY+1, SX)] = 0.5*(f[IX(1,  SY+1, SX)] + f[IX(0,    SY, SX)]);
//   // f[IX(SX+1, 0,    SX)] = 0.5*(f[IX(SX, 0,    SX)] + f[IX(SX+1, 1, SX)]);
//   // f[IX(SX+1, SY+1, SX)] = 0.5*(f[IX(SX, SY+1, SX)] + f[IX(SX+1, SY, SX)]);
// }


__global__ void preProjectH_k(HyperFluid<2> src, HyperFluid<2> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  if(ix < src.size.x && iy < src.size.y)
    {
      int i = src.pindex(ix, iy);

      // float hx = 1.0f/src.size.x;
      // float hy = 1.0f/src.size.y;
      // int   SX = src.size.x;
      // int   SY = src.size.y;
      
      // dst.vel.dData[i] = src.vel.dData[i];
    //   dst.vx.dData[i]  = src.vx.dData[i];
    //   dst.vy.dData[i]  = src.vy.dData[i];
    //   dst.wv.dData[i]  = src.wv.dData[i];
    //   dst.d.dData[i]   = src.d.dData[i];
    //   // dst.p.dData[i]   = src.p.dData[i];
    //   // dst.div.dData[i] = src.div.dData[i];
        
    //   // for(int x = 0; x < src.size.x; x++)
    //   //   for(int y = 0; y < src.size.y; y++)
    //   //     {
    //   if(ix > 0 && iy > 0 && ix < src.size.x-1 && iy < src.size.y-1)
    //     {
    //       dst.div.dData[i] = -0.5f*(hx*(src.vx.dData[IX(ix+1, iy, SX)] - src.vx.dData[IX(ix-1, iy, SX)]) +
    //                                 hy*(src.vy.dData[IX(ix, iy+1, SX)] - src.vy.dData[IX(ix, iy-1, SX)]));
    //       dst.p.dData[i] = 0.0f;
    //     }
    //   else if(ix == 0)
    //     {
          
    //       if(iy == 0)
    //         {
    //           dst.p.dData[i]   = (src.p.dData  [IX(1, iy, SX)] + src.p.dData  [IX(0, iy, SX)])/2.0f;
    //           dst.div.dData[i] = (src.div.dData[IX(1, iy, SX)] + src.div.dData[IX(0, iy, SX)])/2.0f;
    //         }
    //       else if(iy == src.size.y-1)
    //         {
    //           dst.p.dData[i]   = (src.p.dData  [IX(SX-1, iy, SX)] + src.p.dData  [IX(SX-1, iy-1, SX)])/2.0f;
    //           dst.div.dData[i] = (src.div.dData[IX(SX-1, iy, SX)] + src.div.dData[IX(SX-1, iy-1, SX)])/2.0f;
    //         }
    //       else
    //         {
    //           dst.p.dData[i]   = src.p.dData  [IX(1, iy, SX)];
    //           dst.div.dData[i] = src.div.dData[IX(1, iy, SX)];
    //         }
    //     }
    //   else if(ix == src.size.x-1)
    //     {
    //       dst.p.dData[i]   = src.p.dData  [IX(SX-1, iy, SX)];
    //       dst.div.dData[i] = src.div.dData[IX(SX-1, iy, SX)];
    //     }
    //   else if(iy == 0)
    //     {
    //       if(ix == src.size.x-1)
    //         {
    //           dst.p.dData[i]   = (src.p.dData  [IX(ix, 1, SX)] + src.p.dData  [IX(ix, 0, SX)])/2.0f;
    //           dst.div.dData[i] = (src.div.dData[IX(ix, 1, SX)] + src.div.dData[IX(ix, 0, SX)])/2.0f;
    //         }
    //       else if(ix == src.size.x-1)
    //         {
    //           dst.p.dData[i]   = (src.p.dData  [IX(ix, SY-1, SX)] + src.p.dData  [IX(ix, SY-2, SX)])/2.0f;
    //           dst.div.dData[i] = (src.div.dData[IX(ix, SY-1, SX)] + src.div.dData[IX(ix, SY-2, SX)])/2.0f;
    //         }
    //       else
    //         {
    //           dst.p.dData[i]   = src.p.dData  [IX(ix, 1, SX)];
    //           dst.div.dData[i] = src.div.dData[IX(ix, 1, SX)];
    //         }

    //     }
    //   else if(iy == src.size.y-1)
    //     {
    //       dst.p.dData[i]   = src.p.dData  [IX(ix, SY-1, SX)];
    //       dst.div.dData[i] = src.div.dData[IX(ix, SY-1, SX)];
    //     }
    //   // }
    //   //setBounds(dst.div.dData, 0, SX, SY); setBounds(dst.p.dData, 0, SX, SY);
    }
}

__global__ void postProjectH_k(HyperFluid<2> src, HyperFluid<2> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  if(ix < src.size.x && iy < src.size.y)
    {
      int i = src.pindex(ix, iy);
      //dst.vel.dData[i] = src.vel.dData[i];
      // float hx = 1.0f/src.size.x;
      // float hy = 1.0f/src.size.y;
      // int   SX = src.size.x;
      // int   SY = src.size.y;

      // // for(int x = 0; x < src.size.x; x++)
      // //   for(int y = 0; y < src.size.y; y++)
      // //     {
      // if(ix > 0 && iy > 0 && ix < src.size.x-1 && iy < src.size.y-1)
      //   {
      //     dst.vx.dData[i] -= 0.5f*(dst.p.dData[IX(ix+1, iy,   SX)] - dst.p.dData[IX(ix-1, iy,   SX)]) / hx;
      //     dst.vy.dData[i] -= 0.5f*(dst.p.dData[IX(ix,   iy+1, SX)] - dst.p.dData[IX(ix,   iy-1, SX)]) / hy;
      //   }
      // else if(ix == 0)
      //   {
      //     if(ix == src.size.x-1)
      //       {
      //         dst.vx.dData[i] = -src.vx.dData[IX(1, iy, SX)];
      //         dst.vy.dData[i] = src.vy.dData[IX(1, iy, SX)];
      //       }
      //     else
      //       {
      //         dst.vx.dData[i] = -src.vx.dData[IX(1, iy, SX)];
      //         dst.vy.dData[i] = src.vy.dData[IX(1, iy, SX)];
      //       }
      //   }
      // else if(ix == src.size.x-1)
      //   {
      //     dst.vx.dData[i] = -src.vx.dData[IX(SX-1, iy, SX)];
      //     dst.vy.dData[i] = src.vy.dData[IX(SX-1, iy, SX)];
      //   }
      // else if(iy == 0)
      //   {
      //     dst.vx.dData[i] = src.vx.dData[IX(ix, 1, SX)];
      //     dst.vy.dData[i] = -src.vy.dData[IX(ix, 1, SX)];
      //   }
      // else if(iy == src.size.y-1)
      //   {
      //     dst.vx.dData[i] = src.vx.dData[IX(ix, SY-1, SX)];
      //     dst.vy.dData[i] = -src.vy.dData[IX(ix, SY-1, SX)];
      //   }

      // }
      //setBounds(dst.vx.dData, 1, SX, SY); setBounds(dst.vy.dData, 2, SX, SY);
    }
}

// project velocities for mass conservation
__global__ void projectH_k(HyperFluid<2> src, HyperFluid<2> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  int i = src.pindex(ix, iy);
  // float hx = 1.0f/src.size.x;
  // float hy = 1.0f/src.size.y;
  // int   SX = src.size.x;
  // int   SY = src.size.y;
      
  if(ix > 0 && iy > 0 && ix < src.size.x-1 && iy < src.size.y-1)
    {
      // dst.vel.dData[i] = src.vel.dData[i];
      // dst.vx.dData[i]  = src.vx.dData[i];
      // dst.vy.dData[i]  = src.vy.dData[i];
      // dst.wv.dData[i]  = src.wv.dData[i];
      // dst.d.dData[i]   = src.d.dData[i];
      // // dst.p.dData[i]   = src.p.dData[i];
      // // dst.div.dData[i] = src.div.dData[i];
        
      // // for(int x = 0; x < src.size.x; x++)
      // //   for(int y = 0; y < src.size.y; y++)
      // //     {
      // dst.div.dData[i] = -0.5*(hx*(dst.vx.dData[IX(ix+1, iy, SX)] - dst.vx.dData[IX(ix-1, iy, SX)]) +
      //                          hy*(dst.vy.dData[IX(ix, iy+1, SX)] - dst.vy.dData[IX(ix, iy-1, SX)]));
      // dst.p.dData[i] = 0;
      // // }
      // //setBounds(dst.div.dData, 0, SX, SY); setBounds(dst.p.dData, 0, SX, SY);
      
      //for(int k = 0; k < src.params.projectIter; k++)
      {
        // for(int x = 0; x < src.size.x; x++)
        //   for(int y = 0; y < src.size.y; y++)
        //     {
        // dst.p.dData[i] = (dst.div.dData[IX(ix, iy, SX)] +
        //                   dst.p.dData[IX(ix-1, iy, SX)] + dst.p.dData[IX(ix+1, iy, SX)] +
        //                   dst.p.dData[IX(ix, iy-1, SX)] + dst.p.dData[IX(ix, iy+1, SX)]) / 4.0f;
        //     }
        // setBounds(dst.p.dData, 0, SX, SY);
        // __syncthreads();
      }
      // // for(int x = 0; x < src.size.x; x++)
      // //   for(int y = 0; y < src.size.y; y++)
      // //     {
      // dst.vx.dData[i] -= 0.5*(dst.p.dData[IX(ix+1, iy,   SX)] - dst.p.dData[IX(ix-1, iy,   SX)]) / hx;
      // dst.vy.dData[i] -= 0.5*(dst.p.dData[IX(ix,   iy+1, SX)] - dst.p.dData[IX(ix,   iy-1, SX)]) / hy;
      // // }
      // //setBounds(dst.vx.dData, 1, SX, SY); setBounds(dst.vy.dData, 2, SX, SY);
      
    }
  // else if(ix == 0)
  //   {
  //     dst.p.dData[i] = dst.p.dData[IX(1, iy, SX)];
  //   }
  // else if(ix == src.size.x-1)
  //   {
  //     dst.p.dData[i] = dst.p.dData[IX(SX-1, iy, SX)];
  //   }
  // else if(iy == 0)
  //   {
  //     dst.p.dData[i] = dst.p.dData[IX(ix, 1, SX)];
  //   }
  // else if(iy == src.size.y-1)
  //   {
  //     dst.p.dData[i] = dst.p.dData[IX(ix, SY-1, SX)];
  //   }
}

// fix boundary values
__global__ void fixBoundsH_k(HyperFluid<2> src, HyperFluid<2> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;

  int i = src.pindex(ix, iy);
  // float hx = 1.0f/src.size.x;
  // float hy = 1.0f/src.size.y;
  // int   SX =src.size.x;
  // int   SY = src.size.y;
      
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

__global__ void addForcesH_k(HyperFluid<2> src, HyperFluid<2> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  int w = src.size.x; int h = src.size.y;

  if(ix < w && iy < h)
    {
      int i = src.pindex(ix, iy);
      //dst.vel.dData[i] = src.vel.dData[i];
      // float dt = src.params.dt;
      // FluidParams *params = &src.params;

      // float2 forcePoint1 = params->mp;
      // float2 forcePoint2 = params->mpLast;
      // float2 mv = forcePoint1 - forcePoint2;
      // float mvLen = length(mv);
      
      // float  d  = src.d.dData[i];
      // float  p  = src.p.dData[i];
      // float2 u  = float2{src.vx.dData[i], src.vy.dData[i]};
      // float2 norm = u / length(u);
      // float2 wv = src.wv.dData[i];

      // float2 pp = float2{float(ix + 0.5f)/w, float(iy + 0.5f)/h};

      // // calculate force based on distance from mouse movement line segment
      // float2 diff; float  dist;
      // if((dot(normalize(forcePoint2 - forcePoint1), normalize(pp - forcePoint1)) <= 0) ||
      //    (dot(normalize(forcePoint1 - forcePoint2), normalize(pp - forcePoint2)) <= 0))
      //   {
      //     float distp1 = length(forcePoint1 - pp);
      //     float distp2 = length(forcePoint2 - pp);

      //     if(distp1 < distp2)
      //       {
      //         diff = pp - forcePoint1;
      //         dist = distp1;
      //       }
      //     else
      //       {
      //         diff = pp - forcePoint2;
      //         dist = distp2;
      //       }
      //   }
      // else if(mvLen > 0.0f)
      //   {
      //     diff = closestPoint(forcePoint1, forcePoint2, pp);
      //     dist = lineDist    (forcePoint1, forcePoint2, pp);
      //   }
      // else
      //   {
      //     diff = pp - forcePoint1;
      //     dist = length(diff);
      //   }
      
      // float  dist2 = dist*dist;
      // float2 pDiff = pp - forcePoint1;
      // float  pDist = length(pDiff);
      
      // if(params->mdown)
      //   {
      //     float mult = 0.0;
      //     //float dMult = 0.0;
          
      //     if(pDist <= params->forceRad)
      //       {
      //         //mult = mvLen/(1.0 + (pDist)/(params->forceRad*params->forceRad));
      //         //mult = 1.0f/(1.0f + (pDist)/(params->forceRad*params->forceRad));
      //         //mult = 1.0/(1.0 + ((pDiff.x*pDiff.x+pDiff.y*pDiff.y)*dist2)/(params->forceRad*params->forceRad));
      //         mult = (1.0f - smoothstep(0.0f, 1.0f, pDist/params->forceRad)) / sqrt(params->forceRad);
      //         //mult = mult;
      //       }
      //     //mult *= (1.0f + mvLen));
      //     // #define FORCE_SIGMA (sqrt(params->forceRad/3.0f))
          
      //     if(params->ftype & FLUIDFORCE_PUSH)
      //       { // push force
      //         if(dist < params->forceRad)
      //           {
      //             float mult2 = (1.0f-smoothstep(0.0f, 1.0f, dist/params->forceRad)) / (params->forceRad);//1.0 - dist);
      //             //float mult2 = exp(-dist/(2*FORCE_SIGMA*FORCE_SIGMA));
      //             //mult2 = mult2*mult2;
      //             u.x += mv.x * mult2 * params->vfPush;
      //             u.y += mv.y * mult2 * params->vfPush;
      //             if(params->ftype & FLUIDFORCE_WV)
      //               {
      //                 wv += float2{0.2,0.1} * mvLen * mult2 * params->wvf;
      //                 //wv.y += mvLen * mult2 * params->wvf;
      //               }
      //           }
      //       }
          
      //     if(params->ftype & FLUIDFORCE_DENSITY)  { d += mult*params->df; }
      //     if(params->ftype & FLUIDFORCE_PRESSURE) { p += mult*params->pf; }
          
      //     if(params->ftype & FLUIDFORCE_IN)
      //       { // inward forceRadius
      //         u.x += -pDiff.x*mult*params->vfIn;
      //         u.y += -pDiff.y*mult*params->vfIn;
      //         if(params->ftype & FLUIDFORCE_WV)
      //           {
      //             wv.x += -pDist*mult*params->wvf;
      //             // wv.y += -pDist*mult*params->wvf;
      //           }
      //       }
      //     if(params->ftype & FLUIDFORCE_OUT)
      //       { // outward force
      //         u.x += pDiff.x*mult*params->vfOut;
      //         u.y += pDiff.y*mult*params->vfOut;
      //         if(params->ftype & FLUIDFORCE_WV)
      //           {
      //             wv.x += pDist*mult*params->wvf;
      //             // wv.y += pDist*mult*params->wvf;
      //           }
      //       }
      //     if(params->ftype & FLUIDFORCE_CW)
      //       { // clockwise force
      //         u.x += -pDiff.y*mult*params->vfCw;
      //         u.y += pDiff.x*mult*params->vfCw;
      //         if(params->ftype & FLUIDFORCE_WV)
      //           {
      //             //wv.x += -pDiff.y*norm.x*mult*params->wvf;
      //             wv.y += pDist*mult*params->wvf;
      //           }
      //       }
      //     if(params->ftype & FLUIDFORCE_CCW)
      //       { // counter-clockwise
      //         u.x += pDiff.y*mult*params->vfCcw;
      //         u.y += -pDiff.x*mult*params->vfCcw;
      //         if(params->ftype & FLUIDFORCE_WV)
      //           {
      //             //wv.x += pDiff.y*norm.x*mult*params->wvf;
      //             wv.y += -pDist*mult*params->wvf;
      //           }
      //       }
      //   }

      // //if(u.y*params->gravity > 0 && abs(u.y) < abs(params->gravity))
      // {
      //   u.y += dt*params->gravity;//*params->density; // GRAVITY
      // }

      // dst.vx.dData[i]  = u.x;
      // dst.vy.dData[i]  = u.y;
      // dst.wv.dData[i]  = wv;
      // dst.d.dData[i]   = d;
      // dst.p.dData[i]   = p;
      // dst.div.dData[i] = src.div.dData[i];
    }
}



__global__ void updateVelocityH_k(HyperFluid<2> src, HyperFluid<2> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  int w = src.size.x; int h = src.size.y;

  if(ix >= 0 && iy >= 0 && ix < w && iy < h)
    {
      long unsigned int i = src.pindex(ix, iy);
      // just copy for now
      // dst.vel.dData[i] = src.vel.dData[i];
    }
}


//// RENDERING ////

// render 2D --> directly render values as colors
__global__ void renderFluidH2_k(HyperFluid<2> src, CudaFieldTex dst)
{
  long unsigned int ix = blockIdx.x*blockDim.x + threadIdx.x;
  long unsigned int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < dst.size.x && iy < dst.size.y)
    {
      // long unsigned int i = src.pindex(ix, iy);
      float len    = length(src.vel.dData[src.pindex((int)((float)ix*((float)src.size.x/(float)dst.size.x)), (int)((float)iy*((float)src.size.y/(float)dst.size.y)))]);
      dst.dData[ix + iy*dst.size.x] = float4{len, len, len, 1.0f};
    }
}





void clearHFluid2(HyperFluid<2> fluid)
{
  if(fluid.size.x > 0 && fluid.size.y > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(fluid.size.x/(float)BLOCKDIM_X), (int)ceil(fluid.size.y/(float)BLOCKDIM_Y));
      std::cout << "CLEAR FLUID HYPER --> " << fluid.size << "\n";
      clearFluidH2_k<<<grid, threads>>>(fluid);
      getLastCudaError("====> ERROR: clearFluidH2_k failed!");
    }
}

void fillHFluidCircle2(HyperFluid<2> fluid)
{
  if(fluid.size.x > 0 && fluid.size.y > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(fluid.size.x/(float)BLOCKDIM_X), (int)ceil(fluid.size.y/(float)BLOCKDIM_Y));
      fillFluidCircleH2_k<<<grid, threads>>>(fluid);
      getLastCudaError("====> ERROR: fillFluidCircleH2_k failed!");
    }
}

void fillHFluidPattern2(HyperFluid<2> fluid)
{
  if(fluid.size.x > 0 && fluid.size.y > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(fluid.size.x/(float)BLOCKDIM_X), (int)ceil(fluid.size.y/(float)BLOCKDIM_Y));
      fillFluidPatternH2_k<<<grid, threads>>>(fluid);
      getLastCudaError("====> ERROR: fillFluidPatternH2_k failed!");
    }
}



void advectHyper(HyperFluid<2> src, HyperFluid<2> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size == src.size)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      advectH_k<<<grid, threads>>>(src, dst, BLOCKDIM_X/threads.y);
      getLastCudaError("====> ERROR: advectH_k failed!");
    }
}

extern "C" void diffuseHyper(HyperFluid<2> src, HyperFluid<2> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size == src.size)
    {      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      diffuseH_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: diffuseH_k failed!");
    }
}

extern "C" void projectHyper(HyperFluid<2> src, HyperFluid<2> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size == src.size)
    {
      // dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      // dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      // preProjecHt_k<<<grid, threads>>>(src, dst);
      // for(int i = 0; i < src.params.projectIter; i++) { projectH_k<<<grid, threads>>>(src, dst); }
      // postProjectH_k<<<grid, threads>>>(src, dst);
      // getLastCudaError("====> ERROR: projectH_k failed!");
    }
}

extern "C" void addForcesHyper(HyperFluid<2> src, HyperFluid<2> dst)
{
  if(src.size.x > 0 && src.size.y > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      addForcesH_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: addForcesH_k failed!");
    }
}

extern "C" void updateVelHyper(HyperFluid<2> src, HyperFluid<2> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && dst.size == src.size)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X), (int)ceil(src.size.y/(float)BLOCKDIM_Y));
      updateVelocityH_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: updateVelocityH_k failed!");
    }
}


extern "C" void renderHFluid2(HyperFluid<2> src, CudaFieldTex dst)
{
  if(dst.size.x > 0 && dst.size.y > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(dst.size.x/(float)BLOCKDIM_X), (int)ceil(dst.size.y/(float)BLOCKDIM_Y));
      bool mapped = dst.mapped;
      if(!mapped) { dst.map(); }
      renderFluidH2_k<<<grid, threads>>>(src, dst);
      if(!mapped) { dst.unmap(); }
      getLastCudaError("====> ERROR: renderFluidH2_k failed!");
    }
  else
    {
      std::cout << "Skipped HyperFluid<2> render --> " << src.size << " / " << dst.size << " \n";
    }
}
