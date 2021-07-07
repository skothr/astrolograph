#ifndef HYPER_FIELD_HPP
#define HYPER_FIELD_HPP

#include <string>
#include <sstream>
#include <iostream>

#include <GL/glew.h>
#include <cuda.h>
#include <cuda_gl_interop.h>

#include "cudaField.hpp"
#include "cutools.hpp"
#include "vector.hpp"

// forward declarations
typedef void* ImTextureID;
//// HYPER FIELD PARAMS ////
struct HyperFieldParams
{ }; // size of field <W, H>
// struct MandelbrotParams : public FieldParams
// {
//   Vec2f offset  = Vec2f(0.0f, 0.0f);
//   Vec2f scale   = Vec2f(1.0f, 1.0f);
//   float cutoff  = 2.0f;
//   int   maxIter = 64;
//   bool  hollow  = true;
//   float texMult = 1.0f;
// };
// enum ForceType
//   {
//    FLUIDFORCE_NONE         = 0x00,
//    FLUIDFORCE_PUSH         = 0x01, // push with mouse velocity
//    FLUIDFORCE_OUT          = 0x02, // static outward force from point
//    FLUIDFORCE_IN           = 0x04, // static inward force from point
//    FLUIDFORCE_CW           = 0x08, // static clockwise force around point
//    FLUIDFORCE_CCW          = 0x10, // static counter-clockwise force around point
//    FLUIDFORCE_DENSITY      = 0x20, // adds density
//    FLUIDFORCE_PRESSURE     = 0x40, // adds pressure
//    FLUIDFORCE_WV           = 0x80, // adds to wv
//   };
// inline __host__ __device__ ForceType  operator~ (ForceType t)
// { return static_cast<ForceType>(~static_cast<int>(t)); }
// inline __host__ __device__ ForceType& operator|=(ForceType &t0, ForceType t1)
// { t0 = static_cast<ForceType>(static_cast<int>(t0) | static_cast<int>(t1)); return t0; }
// inline __host__ __device__ ForceType& operator&=(ForceType &t0, ForceType t1)
// { t0 = static_cast<ForceType>(static_cast<int>(t0) & static_cast<int>(t1)); return t0; }
// inline __host__ __device__ ForceType  operator| (ForceType t0, ForceType t1)
// { return static_cast<ForceType>(static_cast<int>(t0) | static_cast<int>(t1)); }
// inline __host__ __device__ ForceType  operator& (ForceType t0, ForceType t1)
// { return static_cast<ForceType>(static_cast<int>(t0) & static_cast<int>(t1)); }

//// HYPER FLUID PARAMS ////
struct HyperFluidParams : public HyperFieldParams
{
  Vec2f offset  = Vec2f(0.0f, 0.0f);
  Vec2f scale   = Vec2f(1.0f, 1.0f);
  float texMult = 1.0f;
  float dt      = 0.1f;
  bool  applyChaos  = true;
  float chaos       = 0.1f; // multiplier for WV chaotic modifier
  float viscosity   = 0.1f; // viscosity constant
  int   diffuseRad  = 2;    // radius of diffusion
  int   projectIter = 40;   // number of iterations in velocity projection
  float gravity = 0.0f;
  bool   mdown    = false;
  float2 mp       = float2{0.5f, 0.5f}; // range: [0.0f, 1.0f]
  float2 mpLast   = float2{0.5f, 0.5f}; // range: [0.0f, 1.0f]
  ForceType ftype    = FLUIDFORCE_NONE; // type(s) of forces being applied by the mouse
  float     forceRad = 0.01f;           // radius of force influence -- 0.5 means it would cover the entire field
  float vfPush = 1.0f; // mouse force velocity multiplier (FLUIDFORCE_PUSH)
  float vfOut  = 1.0f; // mouse force velocity multiplier (FLUIDFORCE_OUT)
  float vfIn   = 1.0f; // mouse force velocity multiplier (FLUIDFORCE_IN)
  float vfCw   = 1.0f; // mouse force velocity multiplier (FLUIDFORCE_CW)
  float vfCcw  = 1.0f; // mouse force velocity multiplier (FLUIDFORCE_CCW)
  float df     = 1.0f; // mouse force density multiplier  (FLUIDFORCE_DENSITY)
  float pf     = 1.0f; // mouse force pressure multiplier (FLUIDFORCE_PRESSURE)
  float wvf    = 1.0f; // mouse force wave vector multiplier (WV)
};



// enum FieldDataType
//   {
//    FIELD_INVALID = -1,
//    FIELD_FLOAT = 0, FIELD_FLOAT2,  FIELD_FLOAT3,  FIELD_FLOAT4,
//    FIELD_DOUBLE,    FIELD_DOUBLE2, FIELD_DOUBLE3, FIELD_DOUBLE4,
//    FIELD_INT,       FIELD_INT2,    FIELD_INT3,    FIELD_INT4,
//    FIELD_COUNT
//   };
// inline std::string to_string(FieldDataType t)
// {
//   switch(t)
//     {
//     case FIELD_INT:     return "int";     case FIELD_INT2:    return "int2";
//     case FIELD_INT3:    return "int3";    case FIELD_INT4:    return "int4";
//     case FIELD_FLOAT:   return "float";   case FIELD_FLOAT2:  return "float2";
//     case FIELD_FLOAT3:  return "float3";  case FIELD_FLOAT4:  return "float4";
//     case FIELD_DOUBLE:  return "double";  case FIELD_DOUBLE2: return "double2";
//     case FIELD_DOUBLE3: return "double3"; case FIELD_DOUBLE4: return "double4";
//     default: return "none";
//     }
// }
// inline int numChannels(FieldDataType t)
// {
//   switch(t)
//     {
//     case FIELD_INT:  case FIELD_FLOAT:  case FIELD_DOUBLE:  return 1;
//     case FIELD_INT2: case FIELD_FLOAT2: case FIELD_DOUBLE2: return 2;
//     case FIELD_INT3: case FIELD_FLOAT3: case FIELD_DOUBLE3: return 3;
//     case FIELD_INT4: case FIELD_FLOAT4: case FIELD_DOUBLE4: return 4;
//     default: return 0;
//     }
// }
// inline FieldDataType baseType(FieldDataType t)
// {
//   switch(t)
//     {
//     case FIELD_INT:    case FIELD_INT2:    case FIELD_INT3:    case FIELD_INT4:    return FIELD_INT;
//     case FIELD_FLOAT:  case FIELD_FLOAT2:  case FIELD_FLOAT3:  case FIELD_FLOAT4:  return FIELD_FLOAT;
//     case FIELD_DOUBLE: case FIELD_DOUBLE2: case FIELD_DOUBLE3: case FIELD_DOUBLE4: return FIELD_DOUBLE;
//     default: return FIELD_INVALID;
//     }
// }
// inline std::ostream& operator<<(std::ostream &os, FieldDataType t) { return (os << to_string(t)); }
// // print
// // inline std::ostream& operator<<(std::ostream &os, const int2 &v)    { os << "< " << v.x << ", " << v.y << " >"; return os; }
// // inline std::ostream& operator<<(std::ostream &os, const int3 &v)    { os << "< " << v.x << ", " << v.y << ", " << v.z << " >"; return os; }
// // inline std::ostream& operator<<(std::ostream &os, const int4 &v)    { os << "< " << v.x << ", " << v.y << ", " << v.z << ", " << v.w << " >"; return os; }
// // inline std::ostream& operator<<(std::ostream &os, const float2 &v)  { os << "< " << v.x << ", " << v.y << " >"; return os; }
// // inline std::ostream& operator<<(std::ostream &os, const float3 &v)  { os << "< " << v.x << ", " << v.y << ", " << v.z << " >"; return os; }
// // inline std::ostream& operator<<(std::ostream &os, const float4 &v)  { os << "< " << v.x << ", " << v.y << ", " << v.z << ", " << v.w << " >"; return os; }
// // inline std::ostream& operator<<(std::ostream &os, const double2 &v) { os << "< " << v.x << ", " << v.y << " >"; return os; }
// // inline std::ostream& operator<<(std::ostream &os, const double3 &v) { os << "< " << v.x << ", " << v.y << ", " << v.z << " >"; return os; }
// // inline std::ostream& operator<<(std::ostream &os, const double4 &v) { os << "< " << v.x << ", " << v.y << ", " << v.z << ", " << v.w << " >"; return os; }








//// HYPER FIELD BASE: base class for hyper-dimensional fields (dtype/dimension defined in child class) ////
template<int N>
class HyperFieldBase
{
public:
  Vector<int, N> size; int numCells = 0; int typeSize = 0; int dataSize = 0;

  // dimensions
  int dims()      const { return N; }
  int dim(int d)  const { return (d < N ? size[d] : 0); }
  // indexing
  //int pindex(const Vector<int, N> &p) const;
  // __device__ int ind(int ix) const                 { return ix; }
  // __device__ int ind(int ix, int iy) const         { return ix + iy*size.x; }
  // __device__ int ind(int ix, int iy, int iz) const { return ix + size.x*(iy + size.y*(iz)); }

  
  __host__ __device__ int pindex(int ix, int iy=-1, int iz=-1) const
  {
    if(N == 3)      { return ix + size.x*(iy + size.y*(iz)); }
    else if(N == 2) { return ix + size.x*iy; }
    else if(N == 1) { return ix; }
    else            { return 0;  }
  }
  int pindex(const Vector<int, N> &p) const
  {
    int idx = 0;
    for(int i = 0; i < N; i++)
      {
        int offset = p[i]; for(int j = 0; j < i; j++) { offset *= size[j]; }
        idx += offset;
      }
    return idx;
  }

  virtual bool allocated() const
  {
    bool alloc = (gCudaInitialized && dataSize > 0);
    for(int i = 0; i < N; i++) { alloc &= (size[i] > 0); }
    return alloc;
  }
  virtual std::string typeStr() const
  {
    std::stringstream ss;
    ss << "HyperField<" << type() << ">(" << size << " / " << numCells << "*" << typeSize << " = " << dataSize << ")";
    return ss.str();
  }
  virtual std::string hDataStr(const Vector<int, N> &pos) const { return ""; }

  // pure virtual functions
  virtual void create(const Vector<int, N> &sz) = 0;
  virtual void destroy()  = 0;
  virtual void pullData() = 0;
  virtual void pushData() = 0;
  virtual FieldType type() const { return FIELDTYPE_INVALID; }  
};

//// HYPER FIELD -- handles host/device data ////
template<typename DTYPE, int N>
class HyperField : public HyperFieldBase<N>
{
public:
  DTYPE *hData   = nullptr;
  DTYPE *dData   = nullptr;
  
  virtual void create(const Vector<int, N> &sz) override;
  virtual void destroy() override;
  virtual void pullData() override;
  virtual void pushData() override;
  
  virtual bool allocated() const override;
  virtual FieldType type() const override;
  virtual std::string hDataStr(const Vector<int, N> &pos) const override;
};

template<typename DTYPE, int N>
void HyperField<DTYPE, N>::create(const Vector<int, N> &sz)
{
  initDevice();
  int minsz = -1;
  for(int d = 0; d < N; d++) { minsz = (minsz < 0) ? sz[d] : std::min(minsz, (int)sz[d]); }
  if(gCudaInitialized && minsz > 0)
    {
      destroy();
      getLastCudaError("BEFORE HyperField::create");

      this->size = sz;
      this->numCells = 1;
      for(int d = 0; d < N; d++) { this->numCells *= sz[d]; }
      this->typeSize = sizeof(DTYPE);
      this->dataSize = this->numCells * this->typeSize;
      std::cout << "Creating Hyper-field ("  << sz << " | " << type() << ")...\n";
      std::cout << "  --> data: " << this->numCells << "*" << this->typeSize << " ==> " << this->dataSize << "\n";

      hData = (DTYPE*)malloc(this->dataSize);    // initialize host data
      memset(hData, 0, this->dataSize);
      cudaMalloc((void**)&dData, this->dataSize); // initialize device data
      cudaMemset(dData, 0, this->dataSize);

      getLastCudaError("HyperField::create(sz)");
      std::cout << "  (DONE)\n";
    }
  if(!gCudaInitialized) { std::cout << "====> WARNING(HyperField::create()): CUDA device not initialized!\n"; }
  if(minsz <= 0)        { std::cout << "====> WARNING(HyperField::create()): zero size! " << this->size << " / " << sz << "\n"; }
}

template<typename DTYPE, int N>
void HyperField<DTYPE, N>::destroy()
{
  if(allocated())
    {
      std::cout << "Destroying HyperField...\n";
      if(dData) { cudaFree(dData); dData = nullptr; }
      if(hData) { free(hData);     hData = nullptr; }
      this->size = Vector<int, N>(); this->numCells = 0; this->dataSize = 0;
      getLastCudaError("HyperField::destroy()");
      std::cout << "  (DONE)\n";
    }
}

template<typename DTYPE, int N>
bool HyperField<DTYPE, N>::allocated() const { return (HyperFieldBase<N>::allocated() && dData && hData); }

template<typename DTYPE, int N>
void HyperField<DTYPE, N>::pullData()
{
  if(allocated())
    {
      cudaMemcpy(hData, dData, this->dataSize, cudaMemcpyDeviceToHost); getLastCudaError("HyperField::pullData()\n");
    } else { std::cout << "====> WARNING(HyperField " << type() << "::pullData()): Field not allocated!\n"; }
}

template<typename DTYPE, int N>
void HyperField<DTYPE, N>::pushData()
{
  if(allocated())
    {
      cudaMemcpy(dData, hData, this->dataSize, cudaMemcpyHostToDevice); getLastCudaError("HyperField::pushData()\n");
    } else { std::cout << "====> WARNING(HyperField" << type() << "::pushData()): Field not allocated!\n"; }
}

template<typename DTYPE, int N>
FieldType HyperField<DTYPE, N>::type() const { return getFieldType<DTYPE>(); }
template<typename DTYPE, int N>
std::string HyperField<DTYPE, N>::hDataStr(const Vector<int, N> &p) const { std::stringstream ss; ss << hData[this->pindex(p)]; return ss.str(); }


template<int N> struct VelType    { using type = float; };
template<>      struct VelType<2> { using type = float2; };
template<>      struct VelType<3> { using type = float3; };
template<>      struct VelType<4> { using type = float4; };

//// HYPER "FLUID" -- handles host/device data ////
template<int N>
class HyperFluid : public HyperFieldBase<N>
{
public:
  using vType = typename VelType<N>::type;  // velocity type -- float2 for N=2, float3 for N=3, float4 for N=4

  HyperFluidParams params;
  HyperField<vType,  N> vel;
  HyperField<float,  N> d;
  HyperField<float,  N> p;
  HyperField<float,  N> div;
  HyperField<float2, N> wv;
  
  virtual void create(const Vector<int, N> &sz) override
  { vel.create(sz); d.create(sz); p.create(sz); div.create(sz); wv.create(sz); this->size = sz; }
  virtual void destroy() override
  { vel.destroy(); d.destroy(); p.destroy(); div.destroy(); wv.destroy(); this->size = Vector<int, N>(); }
  virtual bool allocated() const override
  { return (HyperFieldBase<N>::allocated() && vel.allocated() && d.allocated() && p.allocated() && div.allocated() && wv.allocated()); }

  virtual void pullData() override { vel.pullData(); d.pullData(); p.pullData(); div.pullData(); wv.pullData(); }
  virtual void pushData() override { vel.pushData(); d.pushData(); p.pushData(); div.pushData(); wv.pushData(); }
  virtual FieldType type() const override { return FIELDTYPE_FLOAT; }
};


// in hyperfluid.cu
extern "C" void clearHFluid2      (HyperFluid<2> fluid);
extern "C" void fillHFluidCircle2 (HyperFluid<2> fluid);
extern "C" void fillHFluidPattern2(HyperFluid<2> fluid);
extern "C" void advectHyper       (HyperFluid<2> src, HyperFluid<2> dst);
extern "C" void diffuseHyper      (HyperFluid<2> src, HyperFluid<2> dst);
extern "C" void projectHyper      (HyperFluid<2> src, HyperFluid<2> dst);
extern "C" void addForcesHyper    (HyperFluid<2> src, HyperFluid<2> dst);
extern "C" void updateVelHyper    (HyperFluid<2> src, HyperFluid<2> dst);
extern "C" void renderHFluid2     (HyperFluid<2> src, CudaFieldTex  dst);

extern "C" void clearHFluid3      (HyperFluid<3> fluid);
extern "C" void fillHFluidCircle3 (HyperFluid<3> fluid);
extern "C" void fillHFluidPattern3(HyperFluid<3> fluid);
extern "C" void advectHyper3      (HyperFluid<3> src, HyperFluid<3> dst);
extern "C" void diffuseHyper3     (HyperFluid<3> src, HyperFluid<3> dst);
extern "C" void projectHyper3     (HyperFluid<3> src, HyperFluid<3> dst);
extern "C" void addForcesHyper3   (HyperFluid<3> src, HyperFluid<3> dst);
//extern "C" void updateVelHyper3   (HyperFluid<3> src, HyperFluid<3> dst);
extern "C" void renderHFluid3     (HyperFluid<3> src, CudaFieldTex  dst, double3 fPos, double3 fSize,
                                   double3 camPos, double3 camDir, double3 up, double3 right, double fov);
extern "C" void renderHFluid3Slice(HyperFluid<3> src, CudaFieldTex  dst, double3 fPos, double3 fSize,
                                   double3 camPos, double3 camDir, double3 up, double3 right, double fov,
                                   int sdim, int si);


 
#endif // HYPER_FIELD_HPP
