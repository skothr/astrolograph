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
///// 3D ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define BLOCKDIM_X 8
#define BLOCKDIM_Y 8
#define BLOCKDIM_Z 8

#define TOL 0.0000001 // tolerance/epsilon to make sure ray fully intersects


// fills texture data with solid color
__global__ void clearFluidH3_k(HyperFluid<3> fluid)
{
  long unsigned int ix = blockIdx.x*blockDim.x + threadIdx.x;
  long unsigned int iy = blockIdx.y*blockDim.y + threadIdx.y;
  long unsigned int iz = blockIdx.z*blockDim.z + threadIdx.z;
  if(ix < fluid.size.x && iy < fluid.size.y && iz < fluid.size.z)
    {
      long unsigned int i = fluid.pindex(ix, iy, iz);
      // float2 p = float2{(ix-fluid.size.x/2.0f)/float(fluid.size.x),
      //                   (iy-fluid.size.y/2.0f)/float(fluid.size.y) };
      fluid.vel.dData[i] = float3{0.0f, 0.0f, 0.0f};
      fluid.d.dData[i]    = 0.0f;
      fluid.p.dData[i]    = 0.0f;
      fluid.wv.dData[i]   = float2{0.0f, 0.0f};
      fluid.div.dData[i]  = 0.0f;
    }
}

__global__ void fillFluidCircleH3_k(HyperFluid<3> fluid)
{
  long unsigned int ix = blockIdx.x*blockDim.x + threadIdx.x;
  long unsigned int iy = blockIdx.y*blockDim.y + threadIdx.y;
  long unsigned int iz = blockIdx.z*blockDim.z + threadIdx.z;
  if(ix < fluid.size.x && iy < fluid.size.y && iz < fluid.size.z)
    {
      long unsigned int i = fluid.pindex(ix, iy, iz);
      float3 p = float3{(ix-fluid.size.x/2.0f)/float(fluid.size.x),
                        (iy-fluid.size.y/2.0f)/float(fluid.size.y),
                        (iz-fluid.size.z/2.0f)/float(fluid.size.z) };
      float dist = sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
      fluid.vel.dData[i] = (dist > 1.0f/4.0f ? float3{0.0f, 0.0f, 0.0f} : p/dist)*10.0f;
      fluid.d.dData[i]    = (dist > 1.0f/4.0f ? 0.0f : 1.0f);
      fluid.p.dData[i]    = 0.0f;
      fluid.wv.dData[i]   = float2{0.0f, 0.0f};
      fluid.div.dData[i]  = 0.0f;
    }
}

#define SIN_FREQ 8
__global__ void fillFluidPatternH3_k(HyperFluid<3> fluid)
{
  long unsigned int ix = blockIdx.x*blockDim.x + threadIdx.x;
  long unsigned int iy = blockIdx.y*blockDim.y + threadIdx.y;
  long unsigned int iz = blockIdx.z*blockDim.z + threadIdx.z;
  if(ix < fluid.size.x && iy < fluid.size.y && iz < fluid.size.z)
    {
      long unsigned int i = fluid.pindex(ix, iy, iz);
      float3 p = float3{(ix-fluid.size.x/2.0f)/float(fluid.size.x),
                        (iy-fluid.size.y/2.0f)/float(fluid.size.y),
                        (iz-fluid.size.z/2.0f)/float(fluid.size.z) };
      float dist = sqrt(p.x*p.x + p.y*p.y + p.z*p.z);
      
      int3 c = int3{(fluid.size.x/SIN_FREQ/2), (fluid.size.y/SIN_FREQ/2), (fluid.size.z/SIN_FREQ/2)};
      int xm = (ix % (fluid.size.x/SIN_FREQ) < c.x ? 1 : 0);
      int ym = (iy % (fluid.size.y/SIN_FREQ) < c.y ? 1 : 0);
      int zm = (iz % (fluid.size.z/SIN_FREQ) < c.z ? 1 : 0);
      
      // fluid.vel.dData[i] = ((xm == ym) ^ (zm) ? float3{1.0f, 1.0f, 1.0f}/dist/4 : float3{0.0f, 0.0f, 0.0f});
      fluid.d.dData[i]    = ((xm == ym) ^ (zm) ? 1.0f/dist : 0.0f);
      // fluid.p.dData[i]    = 0.0f;
      // fluid.wv.dData[i]   = float2{0.0f, 0.0f};
      // fluid.div.dData[i]  = 0.0f;
      // fluid.vel.dData[i] = float2{ix/(float)fluid.size.x/(iz==0 ? 1 : (iz/(float)fluid.size.z)), iy/(float)fluid.size.y/(iz==0 ? 1 : (iz/(float)fluid.size.z))};
    }
}

__global__ void advectH3_k(HyperFluid<3> src, HyperFluid<3> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  int iz = blockIdx.z*blockDim.z + threadIdx.z;

  if(ix < src.size.x && iy < src.size.y && iz < src.size.z)// && ix < dst.size.x && iy < dst.size.y && iz < dst.size.z)
    {
      int i = src.pindex(ix, iy, iz);
      int w = src.size.x; int h = src.size.y; int d = src.size.z;
      float dt = src.params.dt;
      
      float3 u0 = tex3DD(src.vel.dData, ix+0.5f, iy+0.5f, iz+0.5f, w, h, d);
      float3 u = u0;
      
      float3 lastPos = float3{ float(ix) - dt*u.x + 0.5f, float(iy) - dt*u.y + 0.5f, float(iz) - dt*u.z + 0.5f };
      if(lastPos.x < 0.5f) { lastPos.x = 0.5f; } else if(lastPos.x > src.size.x-0.5f) { lastPos.x = src.size.x-0.5f; }
      if(lastPos.y < 0.5f) { lastPos.y = 0.5f; } else if(lastPos.y > src.size.y-0.5f) { lastPos.y = src.size.y-0.5f; }
      if(lastPos.z < 0.5f) { lastPos.z = 0.5f; } else if(lastPos.z > src.size.z-0.5f) { lastPos.z = src.size.z-0.5f; }
      
      float3 lastU = tex3DD(src.vel.dData, lastPos.x, lastPos.y, lastPos.z, w, h, d);
      
      // // ∂u/∂t = -[dot(u, ∇)](u) + v * ∇^2(u) + F
      // // ∂ρ/∂t = -[dot(u, ∇)](ρ) + k * ∇^2(ρ) + S
      // float3 lastUxp1 = float3{tex2DD(src.vx.dData, lastPos.x+1, lastPos.y, w, h), tex2DD(src.vy.dData, lastPos.x+1, lastPos.y, w, h) }; // x + 1
      // float3 lastUxp2 = float3{tex2DD(src.vx.dData, lastPos.x+2, lastPos.y, w, h), tex2DD(src.vy.dData, lastPos.x+2, lastPos.y, w, h) }; // x + 2
      // float3 lastUxn1 = float3{tex2DD(src.vx.dData, lastPos.x-1, lastPos.y, w, h), tex2DD(src.vy.dData, lastPos.x-1, lastPos.y, w, h) }; // x - 1
      // float3 lastUxn2 = float3{tex2DD(src.vx.dData, lastPos.x-2, lastPos.y, w, h), tex2DD(src.vy.dData, lastPos.x-2, lastPos.y, w, h) }; // x - 2

      // float3 lastUdx  = lastUxp1 + lastUxn1 - 2.0f*lastU;
      // float3 lastUd2x = lastUxp2 + lastUxn2 - 4.0f*lastUxp1 - 4.0f*lastUxn1 + 6.0f*lastU;
      
      // float3 lastUyp1 = float3{tex2DD(src.vx.dData, lastPos.x, lastPos.y+1, w, h), tex2DD(src.vy.dData, lastPos.x, lastPos.y+1, w, h) }; // y + 1
      // float3 lastUyp2 = float3{tex2DD(src.vx.dData, lastPos.x, lastPos.y+2, w, h), tex2DD(src.vy.dData, lastPos.x, lastPos.y+2, w, h) }; // y + 2
      // float3 lastUyn1 = float3{tex2DD(src.vx.dData, lastPos.x, lastPos.y-1, w, h), tex2DD(src.vy.dData, lastPos.x, lastPos.y-1, w, h) }; // y - 1
      // float3 lastUyn2 = float3{tex2DD(src.vx.dData, lastPos.x, lastPos.y-2, w, h), tex2DD(src.vy.dData, lastPos.x, lastPos.y-2, w, h) }; // y - 2
      
      // float3 lastUdy  = lastUyp1 + lastUyn1 - 2.0f*lastU;
      // float3 lastUd2y = lastUyp2 + lastUyn2 - 4.0f*lastUyp1 - 4.0f*lastUyn1 + 6.0f*lastU;


      u = lastU;

      
      // float3 nextPos = float3{ float(ix) + dt*u.x + 0.5f, float(iy) + dt*u.y + 0.5f };
      // if(nextPos.x < 0.5f) { nextPos.x = 0.5f; } else if(nextPos.x > src.size.x-0.5f) { nextPos.x = src.size.x-0.5f; }
      // if(nextPos.y < 0.5f) { nextPos.y = 0.5f; } else if(nextPos.y > src.size.y-0.5f) { nextPos.y = src.size.y-0.5f; }
      // // float3 nextU = float3{tex2DD(src.vx.dData, nextPos.x, nextPos.y, w, h), tex2DD(src.vy.dData, nextPos.x, nextPos.y, w, h) };      
      // // float3 lastUdy = float3{tex2DD(src.vx.dData, lastPos.x+1,   lastPos.y+1,   w, h),
      // //                         tex2DD(src.vy.dData, lastPos.x,     lastPos.y+1+1, w, h) } - lastU;
      // // float3 lastUd2x = float3{tex2DD(src.vx.dData, lastPos.x+1+1, lastPos.y,     w, h),
      // //                         tex2DD(src.vy.dData, lastPos.x+1,   lastPos.y+1,   w, h) } - lastU;
      // // float3 lastUd2y = float3{tex2DD(src.vx.dData, lastPos.x+1,   lastPos.y+1,   w, h),
      // //                         tex2DD(src.vy.dData, lastPos.x,     lastPos.y+1+1, w, h) } - lastU;
      // // float3 nextUdx = float3{tex2DD(src.vx.dData, nextPos.x+1+1, nextPos.y,     w, h),
      // //                         tex2DD(src.vy.dData, nextPos.x+1,   nextPos.y+1,   w, h) } - nextU;
      // // float3 nextUdy = float3{tex2DD(src.vx.dData, nextPos.x+1,   nextPos.y+1,   w, h),
      // //                         tex2DD(src.vy.dData, nextPos.x,     nextPos.y+1+1, w, h) } - nextU;

      // float3 du_dt = -(u.x*lastUdx + u.y*lastUdy) + src.params.viscosity*(lastUd2x + lastUd2y);
      // u += dt*du_dt;
      
      // if(isnan(u.x))  { u.x  = 0.0; }
      // if(isnan(u.y))  { u.y  = 0.0; }
      
      float  dn = tex3DD(src.d.dData, lastPos.x, lastPos.y, lastPos.z, w, h, d);
      //float  lastD  = tex2DD(src.d.dData, lastPos.x, lastPos.y, w, h);
      //float3 dd_dt = -(d*lastUdx + d*lastUdy) + src.params.viscosity*(lastUd2x + lastUd2y);
      
      float  p  = tex3DD(src.p.dData, lastPos.x, lastPos.y, lastPos.z, w, h, d);
      float2 lastWv = tex3DD(src.wv.dData, lastPos.x, lastPos.y, lastPos.z, w, h, d);
      // // float3 nextWv = tex2DD(src.wv.dData, nextPos.x, nextPos.y, w, h);

      // // float newD      = lastD/(1.0 + abs(dt))*0.99;//*params->dissipation);
      // // float newP      = lastP/(1.0 + abs(dt))*0.99;

      float newD = dn;
      float newP = p;
      
      if(isnan(newD)) { newD = 0.0; }
      if(isnan(newP)) { newP = 0.0; }
         
      // // u = (lastU+nextU)/2.0f;
      // // u = (u0 + lastU - nextU);
      
      // // u.x += u0.y*newP*dt;
      // // u.y += u0.x*newP*dt;

      // // newP += length(u)*dt;

      // u.x      = (u.x<0.0f?-1.0f:1.0f)*min(abs(u.x), 4.0f);
      // u.y      = (u.y<0.0f?-1.0f:1.0f)*min(abs(u.y), 4.0f);
      // newP     = (newP < 0.0f ? 0.0f : min(abs(u.y), 4.0f));
      
      // float3 wv00    = float3{tex2DD(src.wv.dData, ix+0.5f, iy+0.5f, w, h)}; //texGet(src.wv.dData, ix, iy, w, h);
      // //float3 wv      = (lastWv+nextWv)/2.0f;
      // float3 wv      = (lastWv);// (wv00 + lastWv - nextWv);
      // if(src.params.applyChaos && src.params.chaos != 0.0)
      //   {
      //     // float3 wvNX = tex2DD(src.wv.dData, lastPos.x+0.5f-1.0f, lastPos.y+0.5f,      w, h);//texGet(src.wv.dData, ix-1, iy, w, h);
      //     // float3 wvPX = tex2DD(src.wv.dData, lastPos.x+0.5f+1.0f, lastPos.y+0.5f,      w, h);//texGet(src.wv.dData, ix+1, iy, w, h);
      //     // float3 wvNY = tex2DD(src.wv.dData, lastPos.x+0.5f,      lastPos.y+0.5f-1.0f, w, h);//texGet(src.wv.dData, ix, iy-1, w, h);
      //     // float3 wvPY = tex2DD(src.wv.dData, lastPos.x+0.5f,      lastPos.y+0.5f+1.0f, w, h);//texGet(src.wv.dData, ix, iy+1, w, h);
          
      //     // float3 dwvX = ((wvPX - wv00) - (wv00 - wvNX))/(2.0);
      //     // float3 dwvY = ((wvPY - wv00) - (wv00 - wvNY))/(2.0);
          
      //     // float3 dwvDt = cMult(dwvX, dwvY);
      //     //float3 dwvDt = float3{dwvX.x*dwvY.x - dwvX.y*dwvY.y};
          
      //     // wv = lastWv;
      //     // wv += (cMult(cDiv(wv, u/length(u)), cDiv(wv, u/length(u))))*dt;
      //     // wv.x = (wv.x >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.x), 4.0f);
      //     // wv.y = (wv.y >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.y), 4.0f);
      //     // if(u.x > 0 && !isnan(u.x) and u.y > 0 && !isnan(u.y))
      //     //   {
      //     //     float3 wv2 = cDiv(u, u0);
      //     //     u -= dt*src.params.wvf*wv*wv00*(cMult(wv2, wv2)+u); //+= dt*src.params.wvf*cMult(u0, cDiv(u, wv00));
      //     //   }
      //     // else
      //     //   {
      //     //     u += dt*src.params.wvf*wv; //+= dt*src.params.wvf*cMult(u0, cDiv(u, wv00));
      //     //   }

          
      //     // float3 norm = normalize(u);
      //     // wv += (cMult(cDiv(wv, norm), cDiv(wv, norm)))*dt;
      //     // wv.x = (wv.x >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.x), 4.0f);
      //     // wv.y = (wv.y >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.y), 4.0f);
      //     // u += src.params.chaos*(cMult(wv, norm/2.0f))*dt;

      //     float3 norm = normalize(u);
      //     float3 oriented = cDiv(wv, norm);
      //     wv += (cMult(oriented, oriented))*dt;//cDiv(wv, norm), cDiv(wv, norm)))*dt;
      //     // wv += (u0-u)*dt;
      //     wv.x = (wv.x >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.x), 4.0f);
      //     wv.y = (wv.y >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.y), 4.0f);
      //     u += dt*src.params.chaos*wv;
          
      //     // //// VERY FLUID-LIKE; REALISM -- WAVE VECTOR APPEARS 3D SHADED ////
      //     // float3 norm = normalize(u);
      //     // wv += (cMult(cDiv(wv, norm), cDiv(wv, norm)))*dt;
      //     // wv.x = (wv.x >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.x), 4.0f);
      //     // wv.y = (wv.y >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.y), 4.0f);
      //     // u += src.params.chaos*wv*dt;
      //     // //// COOL WV FIRE ////
      //     // wv += (cMult(cDiv(wv, u), cDiv(wv, u)))*dt;
      //     // wv.x = (wv.x >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.x), 4.0f);
      //     // wv.y = (wv.y >= 0.0f ? 1.0f : -1.0f)*min(abs(wv.y), 4.0f);
      //     // u += src.params.chaos*wv*dt;
      //   }

      dst.vel.dData[i] = u;
      dst.d.dData[i]    = newD;
      dst.p.dData[i]    = newP;
      dst.wv.dData[i]   = lastWv;
      dst.div.dData[i]  = src.div.dData[i];
    }
}



__global__ void diffuseH3_k(HyperFluid<3> src, HyperFluid<3> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  int iz = blockIdx.z*blockDim.z + threadIdx.z;

  int kRad = src.params.diffuseRad;
  if(ix < src.size.x-kRad && iy < src.size.y-kRad && iz < src.size.z-kRad && ix > kRad && iy > kRad && iz > kRad)
    {
      int i = src.pindex(ix, iy, iz);
      float dt = src.params.dt;
      
      float3 u    = float3{0.0f, 0.0f, 0.0f};
      float  d    = 0.0f;
      float  p    = 0.0f;
      float2 wv   = float2{0.0f, 0.0f};
      float  mult = 0.0f;
      
      for(int x = -kRad; x <= kRad; x++)
        for(int y = -kRad; y <= kRad; y++)
          for(int z = -kRad; z <= kRad; z++)
            {
              float dist = (x==0 && y==0 && z==0) ? 1.0f : sqrt(float(x*x + y*y + z*z)) / dt;
              if(isnan(dist) || dist == 0.0f) { continue; }
              
              mult += 1.0f/dist;
              int li = src.pindex(ix+x, iy+y, iz+z);
              u  += src.vel.dData[li]/dist;
              d  += src.d.dData[li]/dist;
              p  += src.p.dData[li]/dist;
              wv += src.wv.dData[li]/dist;
            }
      dst.vel.dData[i] = (mult != 0.0f && !isnan(mult) && !isnan(u))  ? (u / mult)  : src.vel.dData[i];
      dst.d.dData[i]    = (mult != 0.0f && !isnan(mult) && !isnan(d))  ? (d / mult)  : src.d.dData[i];
      dst.p.dData[i]    = (mult != 0.0f && !isnan(mult) && !isnan(p))  ? (p / mult)  : src.p.dData[i];
      dst.wv.dData[i]   = (mult != 0.0f && !isnan(mult) && !isnan(wv)) ? (wv / mult) : src.wv.dData[i];
      dst.div.dData[i]  = src.div.dData[i];
    }
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

__global__ void preProjectH3_k(HyperFluid<3> src, HyperFluid<3> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  int iz = blockIdx.z*blockDim.z + threadIdx.z;

  if(ix < src.size.x && iy < src.size.y && iz < src.size.z)
    {
      int i = src.pindex(ix, iy, iz);
      float hx = 1.0f/src.size.x;
      float hy = 1.0f/src.size.y;
      float hz = 1.0f/src.size.z;
      
      if(ix > 0 && iy > 0 && iz > 0 && ix < src.size.x-1 && iy < src.size.y-1 && iz < src.size.z-1)
        {
          dst.div.dData[i] = -0.5f*(hx*(src.vel.dData[src.pindex(ix+1, iy, iz)].x - src.vel.dData[src.pindex(ix-1, iy, iz)].x) +
                                    hy*(src.vel.dData[src.pindex(ix, iy+1, iz)].y - src.vel.dData[src.pindex(ix, iy-1, iz)].y) +
                                    hz*(src.vel.dData[src.pindex(ix, iy, iz+1)].z - src.vel.dData[src.pindex(ix, iy, iz-1)].z));
          dst.p.dData[i] = 0.0f;
        }
      else
        {
          dst.p.dData[i]    = src.p.dData[i];
          dst.div.dData[i]  = src.div.dData[i];
        }
      dst.d.dData[i]    = src.d.dData[i];
      dst.wv.dData[i]   = src.wv.dData[i];
    }
}

__global__ void postProjectH3_k(HyperFluid<3> src, HyperFluid<3> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  int iz = blockIdx.z*blockDim.z + threadIdx.z;

  if(ix < dst.size.x && iy < dst.size.y && iz < dst.size.z)
    {
      int i = dst.pindex(ix, iy, iz);
      float hx = 1.0f/dst.size.x;
      float hy = 1.0f/dst.size.y;
      float hz = 1.0f/dst.size.z;
      
      // dst.vel.dData[i] = dst.vel.dData[i];
      // dst.p.dData[i]   = dst.p.dData[i];
      // dst.div.dData[i] = dst.div.dData[i];

      if(ix > 0 && iy > 0 && iz > 0 && ix < dst.size.x-1 && iy < dst.size.y-1 && iz < dst.size.z-1)
        {
          dst.vel.dData[i].x -= 0.5f*(dst.p.dData[dst.pindex(ix+1, iy,   iz)] - dst.p.dData[dst.pindex(ix-1, iy,   iz)]) / hx;
          dst.vel.dData[i].y -= 0.5f*(dst.p.dData[dst.pindex(ix,   iy+1, iz)] - dst.p.dData[dst.pindex(ix,   iy-1, iz)]) / hy;
          dst.vel.dData[i].z -= 0.5f*(dst.p.dData[dst.pindex(ix,   iy, iz+1)] - dst.p.dData[dst.pindex(ix,   iy, iz-1)]) / hz;
        }
      dst.d.dData[i]    = dst.d.dData[i];
      dst.wv.dData[i]   = dst.wv.dData[i];
    }
}

// project velocities for mass conservation
__global__ void projectH3_k(HyperFluid<3> src, HyperFluid<3> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  int iz = blockIdx.z*blockDim.z + threadIdx.z;

  if(ix < dst.size.x && iy < dst.size.y && iz < dst.size.z)
    {
      int i = dst.pindex(ix, iy, iz);
      
      float hx = 1.0f/dst.size.x;
      float hy = 1.0f/dst.size.y;
      float hz = 1.0f/dst.size.z;
      
      if(ix > 0 && iy > 0 && iz > 0 && ix < dst.size.x-1 && iy < dst.size.y-1 && iz < dst.size.z-1)
        {
          dst.p.dData[i] = (dst.div.dData[dst.pindex(ix, iy, iz)] +
                            dst.p.dData[dst.pindex(ix-1, iy, iz)] + dst.p.dData[dst.pindex(ix+1, iy, iz)] +
                            dst.p.dData[dst.pindex(ix, iy-1, iz)] + dst.p.dData[dst.pindex(ix, iy+1, iz)] +
                            dst.p.dData[dst.pindex(ix, iy, iz-1)] + dst.p.dData[dst.pindex(ix, iy, iz+1)]) / 6.0f;
        }
      if(ix == 0)                 { dst.vel.dData[i].x =  abs(dst.vel.dData[i].x)/2.0f; }
      else if(ix == dst.size.x-1) { dst.vel.dData[i].x = -abs(dst.vel.dData[i].x)/2.0f; }
      if(iy == 0)                 { dst.vel.dData[i].y =  abs(dst.vel.dData[i].y)/2.0f; }
      else if(iy == dst.size.y-1) { dst.vel.dData[i].y = -abs(dst.vel.dData[i].y)/2.0f; }
      if(iz == 0)                 { dst.vel.dData[i].z =  abs(dst.vel.dData[i].z)/2.0f; }
      else if(iz == dst.size.z-1) { dst.vel.dData[i].z = -abs(dst.vel.dData[i].z)/2.0f; }
    }
}


// __forceinline__ __device__ float lineDist3(float2 l1, float2 l2, float2 p0)
// {
//   return abs((l2.y - l1.y)*p0.x - (l2.x - l1.x)*p0.y + l2.x*l1.y - l2.y*l1.x) / sqrt((l2.y-l1.y)*(l2.y-l1.y) + (l2.x-l1.x)*(l2.x-l1.x));
// }

// __forceinline__ __device__ float2 closestPoint3(float2 l1, float2 l2, float2 p0)
// {
//   float2 n = normalize(l2 - l1);
//   float2 v = normalize(p0 - l1);
//   float  d = dot(v, n);
//   return l1 + n*d;
// }

__global__ void addForcesH3_k(HyperFluid<3> src, HyperFluid<3> dst)
{
  int ix = blockIdx.x*blockDim.x + threadIdx.x;
  int iy = blockIdx.y*blockDim.y + threadIdx.y;
  int iz = blockIdx.z*blockDim.z + threadIdx.z;

  if(ix < src.size.x && iy < src.size.y && iz < src.size.z)
    {
      int i = src.pindex(ix, iy, iz);
      float dt = src.params.dt;
      HyperFluidParams *params = &src.params;

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
      
      dst.vel.dData[i] = src.vel.dData[i] + float3{0.0f, 0.0f, dt*params->gravity};
      dst.d.dData[i]    = src.d.dData[i];
      dst.p.dData[i]    = src.p.dData[i];
      dst.wv.dData[i]   = src.wv.dData[i];
      dst.div.dData[i]  = src.div.dData[i];
    }
}



__device__ double planeIntersect(const double3 &p, const double3 &n, const double3 &rPos, const double3 &rDir)
{
  double denom = dot(n, rDir);
  double t = -1.0;
  if(abs(denom) > TOL) { t = dot((p - rPos), n) / denom; }
  return t; // no intersection if t < 0
}


// render 3D --> raytrace field
//  - field assumed to be size (1,1,1) in 3D space
__device__ double2 cubeIntersect(const double3 &fPos, const double3 &fSize, const double3 &rPos, const double3 &rDir)
{
  double tnx = (fPos.x - rPos.x)           / rDir.x;
  double tpx = (fPos.x - rPos.x + fSize.x) / rDir.x;
  double tny = (fPos.y - rPos.y)           / rDir.y;
  double tpy = (fPos.y - rPos.y + fSize.y) / rDir.y;
  double tnz = (fPos.z - rPos.z)           / rDir.z;
  double tpz = (fPos.z - rPos.z + fSize.z) / rDir.z;
  double tmin = max(max(min(tnx, tpx), min(tny, tpy)), min(tnz, tpz));
  double tmax = min(min(max(tnx, tpx), max(tny, tpy)), max(tnz, tpz));
  return (tmin > tmax) ? double2{-1.0, -1.0} : double2{tmin, tmax};
}



// __device__ double2 cubeMarch(const double3 &fPos, const double3 &fSize, const double3 &rPos, const double3 &rDir)
// {
//   //float4 color = float4{0.0f, 0.0f, 0.0f, 1.0f};

//   double tnx = (fPos.x - rPos.x)           / rDir.x;
//   double tpx = (fPos.x - rPos.x + fSize.x) / rDir.x;
//   double tny = (fPos.y - rPos.y)           / rDir.y;
//   double tpy = (fPos.y - rPos.y + fSize.y) / rDir.y;
//   double tnz = (fPos.z - rPos.z)           / rDir.z;
//   double tpz = (fPos.z - rPos.z + fSize.z) / rDir.z;
  
//   double tmin = max(max(min(tnx, tpx), min(tny, tpy)), min(tnz, tpz));
//   double tmax = min(min(max(tnx, tpx), max(tny, tpy)), max(tnz, tpz));

//   if(tmin > tmax) { return double2{-1.0, -1.0}; } // no intersection
//   else            { return double2{tmin, tmax}; }
// }





//#define FIELD_POS     double3{-0.5, -0.5, -0.5}      // position of field (ratio of field size)
//#define DIM_BASE_SIZE 64.0                           // dim size equivalent to 1.0 unit in world space
#define BG_COLOR      float4{0.1f, 0.1f, 0.1f, 1.0f} // color of background behind field
#define FAIL_COLOR    float4{1.0f, 0.0f, 1.0f, 1.0f} // color returned on failure/error

__device__ double3 rayProject(const double3 &p, const double3 &d, double t)       { return p + d*t; }

// inf --> inverse normalized field (pos/size)
__device__ double3 worldToField(const double3 &wp, const double3 &foffset, const double3 &fscale)
{ return (wp/fscale - foffset); }

// // nfs --> normalized field size
// __device__ double3 fieldToWorld(const double3 &fp, const double3 &foffset, const double3 &fscale)
// { return (fp*nfsize + infpos); }


// render 3D --> raytrace field
__device__ float4 rayTraceFluid(const HyperFluid<3> &src, const double3 &fPos, const double3 &fSize, const double3 &rPos, const double3 &rDir)
{
  double3 fs = double3{(double)src.size.x, (double)src.size.y, (double)src.size.z}; // field size as double
  double2 tp = cubeIntersect(fPos, fSize, rPos, rDir); // returns {tmin, tmax} // <-- TODO: both needed?
  double  t  = tp.x;                                   // tmin
  if(t < 0.0) { return BG_COLOR; }
  else
    {
      double3 wp = rayProject(rPos, rDir, t+TOL); // world-space pos of primary intersection
      double3 fp = (wp - fPos) / fSize * fs;
      // cube marching
      int i = src.pindex((int)fp.x, (int)fp.y, (int)fp.z);
      float4 color = float4{0.0f, 0.0f, 0.0f, 0.0f};
      while(color.x < 1 && color.y < 1 && color.z < 1 && t <= tp.y+TOL)
        {
          if(i < 0 || i >= src.size.x*src.size.y*src.size.z)
            { color = FAIL_COLOR; break; }
          float3 v = src.vel.dData[i];
          float  d = src.d.dData[i];
          color += float4{abs(v.x), abs(v.y), abs(v.z), 0.0f}*d;
          
          double3 fp2 = fp;
          while((int)fp2.x == (int)fp.x && (int)fp2.y == (int)fp.y && (int)fp2.z == (int)fp.z && t <= tp.y+TOL)
            {
              t += 0.005;
              double3 wp2 = rayProject(rPos, rDir, t+TOL);
              fp2 = (wp2 - fPos) / fSize * fs;
            }
          fp = fp2;
          i = src.pindex((int)fp.x, (int)fp.y, (int)fp.z);
        }
      return float4{color.x, color.y, color.z, 1.0f};
    }
}

// render 3D --> raytrace field
__global__ void renderFluidH3_k(HyperFluid<3> src, CudaFieldTex dst, double3 fPos, double3 fSize,
                                double3 camPos, double3 camDir, double3 up, double3 right, double fov)
{
  long unsigned int ix = blockIdx.x*blockDim.x + threadIdx.x;
  long unsigned int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < dst.size.x && iy < dst.size.y)
    {
      double tf2 = tan(fov/2.0);
      double2 hs = double2{(double)dst.size.x, (double)dst.size.y}/2.0;
      
      double3 rayDir = normalize(camDir + right*tf2*((ix-dst.size.x/2.0)/dst.size.x) + up*tf2*((iy-dst.size.y/2.0)/dst.size.y));
      float4 color   = rayTraceFluid(src, fPos, fSize, camPos, rayDir);
      
      long unsigned int di = ix + iy*dst.size.x;
      dst.dData[di] = (color.w < 0.0f ? float4{0.0f, 0.0f, 0.0f, 1.0f} : color);
    }
}


// render 3D --> raytrace field
__global__ void renderFluidH3Slice_k(HyperFluid<3> src, CudaFieldTex dst, double3 fPos, double3 fSize,
                                     double3 camPos, double3 camDir, double3 up, double3 right, double fov,
                                     int sdim, int si)
{
  long unsigned int ix = blockIdx.x*blockDim.x + threadIdx.x;
  long unsigned int iy = blockIdx.y*blockDim.y + threadIdx.y;
  if(ix < dst.size.x && iy < dst.size.y)
    {
      double tf2 = tan(fov/2.0);
      double2 hs = double2{(double)dst.size.x, (double)dst.size.y}/2.0;
      
      double3 rayDir  = normalize(camDir + right*tf2*((ix-dst.size.x/2.0)/dst.size.x) + up*tf2*((iy-dst.size.y/2.0)/dst.size.y));
      double3 pCenter = double3{0.0, 0.0, 0.0};
      double3 pNorm   = double3{0.0, 0.0, 0.0};
      switch(sdim)
        {
        case 0: pCenter.x = fPos.x + (si + 0.5)*fSize.x/(double)src.size.x; pNorm.x = 1.0; break;
        case 1: pCenter.y = fPos.y + (si + 0.5)*fSize.y/(double)src.size.y; pNorm.y = 1.0; break;
        case 2: pCenter.z = fPos.z + (si + 0.5)*fSize.z/(double)src.size.z; pNorm.z = 1.0; break;
        }
      double t = planeIntersect(pCenter, pNorm, camPos, rayDir);

      float4 color;
      if(t < 0.0) { color = BG_COLOR; }
      else
        {
          double3 p = rayProject(camPos, rayDir, t);
          int3 fp;
          switch(sdim)
            {
            case 0: fp = int3{si, (int)((p.y - fPos.y)/fSize.y*src.size.y),     (int)((p.z - fPos.z)/fSize.z*src.size.z)};     break;
            case 1: fp = int3{    (int)((p.x - fPos.x)/fSize.x*src.size.x), si, (int)((p.z - fPos.z)/fSize.z*src.size.z)};     break;
            case 2: fp = int3{    (int)((p.x - fPos.x)/fSize.x*src.size.x),     (int)((p.y - fPos.y)/fSize.y*src.size.y), si}; break;
            }

          if(fp.x >= 0 && fp.x < src.size.x && fp.y >= 0 && fp.y < src.size.y && fp.z >= 0 && fp.z < src.size.z)
            {
              int fi = src.pindex(fp.x, fp.y, fp.z);
              float3 v = src.vel.dData[fi];
              float  d = src.d.dData[fi];
              color = float4{abs(v.x), abs(v.y), abs(v.z), 0.0f}*d;
              color.w = 1.0f;
            }
          else { color = BG_COLOR; }
        }
      
      long unsigned int di = ix + iy*dst.size.x;
      dst.dData[di] = color;
    }
}













void clearHFluid3(HyperFluid<3> fluid)
{
  if(fluid.size.x > 0 && fluid.size.y > 0 && fluid.size.z > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y, BLOCKDIM_Z);
      dim3 grid((int)ceil(fluid.size.x/(float)BLOCKDIM_X),
                (int)ceil(fluid.size.y/(float)BLOCKDIM_Y),
                (int)ceil(fluid.size.z/(float)BLOCKDIM_Z));
      clearFluidH3_k<<<grid, threads>>>(fluid);
      getLastCudaError("====> ERROR: clearFluidH3_k failed!");
    }
}

void fillHFluidCircle3(HyperFluid<3> fluid)
{
  if(fluid.size.x > 0 && fluid.size.y > 0 && fluid.size.z > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y, BLOCKDIM_Z);
      dim3 grid((int)ceil(fluid.size.x/(float)BLOCKDIM_X),
                (int)ceil(fluid.size.y/(float)BLOCKDIM_Y),
                (int)ceil(fluid.size.z/(float)BLOCKDIM_Z));
      fillFluidCircleH3_k<<<grid, threads>>>(fluid);
      getLastCudaError("====> ERROR: fillFluidCircleH3_k failed!");
    }
}

void fillHFluidPattern3(HyperFluid<3> fluid)
{
  if(fluid.size.x > 0 && fluid.size.y > 0 && fluid.size.z > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y, BLOCKDIM_Z);
      dim3 grid((int)ceil(fluid.size.x/(float)BLOCKDIM_X),
                (int)ceil(fluid.size.y/(float)BLOCKDIM_Y),
                (int)ceil(fluid.size.z/(float)BLOCKDIM_Z));
      fillFluidPatternH3_k<<<grid, threads>>>(fluid);
      getLastCudaError("====> ERROR: fillFluidPatternH3_k failed!");
    }
}

void advectHyper3(HyperFluid<3> src, HyperFluid<3> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && src.size.z > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y, BLOCKDIM_Z);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X),
                (int)ceil(src.size.y/(float)BLOCKDIM_Y),
                (int)ceil(src.size.z/(float)BLOCKDIM_Z));
      advectH3_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: advectH3_k failed!");
    }
}

extern "C" void diffuseHyper3(HyperFluid<3> src, HyperFluid<3> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && src.size.z > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y, BLOCKDIM_Z);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X),
                (int)ceil(src.size.y/(float)BLOCKDIM_Y),
                (int)ceil(src.size.z/(float)BLOCKDIM_Z));
      diffuseH3_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: diffuseH3_k failed!");
    }
}

extern "C" void projectHyper3(HyperFluid<3> src, HyperFluid<3> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && src.size.z > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y, BLOCKDIM_Z);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X),
                (int)ceil(src.size.y/(float)BLOCKDIM_Y),
                (int)ceil(src.size.z/(float)BLOCKDIM_Z));
      
      preProjectH3_k<<<grid, threads>>>(src, dst);
      for(int i = 0; i < src.params.projectIter; i++) // iterate to converge
        { projectH3_k<<<grid, threads>>>(src, dst); }
      postProjectH3_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: projection kernel failed! (projectHyper3)");
    }
}

extern "C" void addForcesHyper3(HyperFluid<3> src, HyperFluid<3> dst)
{
  if(src.size.x > 0 && src.size.y > 0 && src.size.z > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y, BLOCKDIM_Z);
      dim3 grid((int)ceil(src.size.x/(float)BLOCKDIM_X),
                (int)ceil(src.size.y/(float)BLOCKDIM_Y),
                (int)ceil(src.size.z/(float)BLOCKDIM_Z));
      addForcesH3_k<<<grid, threads>>>(src, dst);
      getLastCudaError("====> ERROR: addForcesH3_k failed!");
    }
}

extern "C" void renderHFluid3(HyperFluid<3> src, CudaFieldTex dst, double3 fPos, double3 fSize,
                              double3 camPos, double3 camDir, double3 up, double3 right, double fov)
{
  if(src.size.x > 0 && src.size.y > 0 && src.size.z > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y); // rendered to 2D texture
      dim3 grid((int)ceil(dst.size.x/(float)BLOCKDIM_X),
                (int)ceil(dst.size.y/(float)BLOCKDIM_Y));
      bool mapped = dst.mapped;
      if(!mapped) { dst.map(); }
      renderFluidH3_k<<<grid, threads>>>(src, dst, fPos, fSize, camPos, camDir, up, right, fov);
      getLastCudaError("====> ERROR: renderFluidH3_k failed!");
      if(!mapped) { dst.unmap(); }
    }
  else
    { std::cout << "Skipped HyperFluid<3> render --> " << src.size << " / " << dst.size << " \n"; }
}

extern "C" void renderHFluid3Slice(HyperFluid<3> src, CudaFieldTex dst, double3 fPos, double3 fSize,
                                   double3 camPos, double3 camDir, double3 up, double3 right, double fov,
                                   int sdim, int si)
{
  if(src.size.x > 0 && src.size.y > 0 && src.size.z > 0)
    {
      dim3 threads(BLOCKDIM_X, BLOCKDIM_Y);
      dim3 grid((int)ceil(dst.size.x/(float)BLOCKDIM_X),
                (int)ceil(dst.size.y/(float)BLOCKDIM_Y));
      bool mapped = dst.mapped;
      if(!mapped) { dst.map(); }
      renderFluidH3Slice_k<<<grid, threads>>>(src, dst, fPos, fSize, camPos, camDir, up, right, fov, sdim, si);
      getLastCudaError("====> ERROR: renderFluidH3Slice_k failed!");
      if(!mapped) { dst.unmap(); }
    }
  else
    { std::cout << "Skipped HyperFluid<3> slice render --> " << src.size << " / " << dst.size << " \n"; }
}
