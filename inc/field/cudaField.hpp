#ifndef CUDA_FIELD_HPP
#define CUDA_FIELD_HPP

#include <string>
#include <sstream>
#include <iostream>

#include <GL/glew.h>
#include <cuda.h>
#include <cuda_gl_interop.h>
#include <vector-operators.h>

#include "cutools.hpp"
#include "vector.hpp"

// forward declarations
typedef void* ImTextureID;

//// CUDA FIELD PARAMS ////

struct FieldParams
{

}; // size of field <W, H>

struct MandelbrotParams : public FieldParams
{
  Vec2f offset  = Vec2f(0.0f, 0.0f);
  Vec2f scale   = Vec2f(1.0f, 1.0f);
  float cutoff  = 2.0f;
  int   maxIter = 64;
  bool  hollow  = true;
  float texMult = 1.0f;
};

enum ForceType
  {
   FLUIDFORCE_NONE         = 0x00,
   FLUIDFORCE_PUSH         = 0x01, // push with mouse velocity
   FLUIDFORCE_OUT          = 0x02, // static outward force from point
   FLUIDFORCE_IN           = 0x04, // static inward force from point
   FLUIDFORCE_CW           = 0x08, // static clockwise force around point
   FLUIDFORCE_CCW          = 0x10, // static counter-clockwise force around point
   FLUIDFORCE_DENSITY      = 0x20, // adds density
   FLUIDFORCE_PRESSURE     = 0x40, // adds pressure
   FLUIDFORCE_WV           = 0x80,  // adds to wv
  };
inline __host__ __device__ ForceType  operator~ (ForceType t)
{ return static_cast<ForceType>(~static_cast<int>(t)); }
inline __host__ __device__ ForceType& operator|=(ForceType &t0, ForceType t1)
{ t0 = static_cast<ForceType>(static_cast<int>(t0) | static_cast<int>(t1)); return t0; }
inline __host__ __device__ ForceType& operator&=(ForceType &t0, ForceType t1)
{ t0 = static_cast<ForceType>(static_cast<int>(t0) & static_cast<int>(t1)); return t0; }
inline __host__ __device__ ForceType  operator| (ForceType t0, ForceType t1)
{ return static_cast<ForceType>(static_cast<int>(t0) | static_cast<int>(t1)); }
inline __host__ __device__ ForceType  operator& (ForceType t0, ForceType t1)
{ return static_cast<ForceType>(static_cast<int>(t0) & static_cast<int>(t1)); }

struct FluidParams : public FieldParams
{
  Vec2f offset  = Vec2f(0.0f, 0.0f);
  Vec2f scale   = Vec2f(1.0f, 1.0f);
  float texMult = 1.0f;

  float dt      = 0.1f;

  bool  applyChaos  = true;
  float chaos       = 0.1f; // multiplier for WV chaotic modifier
  float viscosity   = 0.1f; // viscosity constant
  int   diffuseRad  = 2;    // radius of diffusion
  int   projectIter = 20;   // number of iterations in velocity projection
  
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



enum FieldType
  {
   FIELDTYPE_INVALID = -1,
   FIELDTYPE_FLOAT = 0, FIELDTYPE_FLOAT2,  FIELDTYPE_FLOAT3,  FIELDTYPE_FLOAT4,
   FIELDTYPE_DOUBLE,    FIELDTYPE_DOUBLE2, FIELDTYPE_DOUBLE3, FIELDTYPE_DOUBLE4,
   FIELDTYPE_INT,       FIELDTYPE_INT2,    FIELDTYPE_INT3,    FIELDTYPE_INT4,
   FIELDTYPE_COUNT
  };
template<typename DTYPE> inline FieldType getFieldType() { return FIELDTYPE_INVALID; }
template<> inline FieldType getFieldType<int    >() { return FIELDTYPE_INT;     }
template<> inline FieldType getFieldType<int2   >() { return FIELDTYPE_INT2;    }
template<> inline FieldType getFieldType<int3   >() { return FIELDTYPE_INT3;    }
template<> inline FieldType getFieldType<int4   >() { return FIELDTYPE_INT4;    }
template<> inline FieldType getFieldType<float  >() { return FIELDTYPE_FLOAT;   }
template<> inline FieldType getFieldType<float2 >() { return FIELDTYPE_FLOAT2;  }
template<> inline FieldType getFieldType<float3 >() { return FIELDTYPE_FLOAT3;  }
template<> inline FieldType getFieldType<float4 >() { return FIELDTYPE_FLOAT4;  }
template<> inline FieldType getFieldType<double >() { return FIELDTYPE_DOUBLE;  }
template<> inline FieldType getFieldType<double2>() { return FIELDTYPE_DOUBLE2; }
template<> inline FieldType getFieldType<double3>() { return FIELDTYPE_DOUBLE3; }
template<> inline FieldType getFieldType<double4>() { return FIELDTYPE_DOUBLE4; }



inline std::string to_string(FieldType t)
{
  switch(t)
    {
    case FIELDTYPE_INT:     return "int";
    case FIELDTYPE_INT2:    return "int2";
    case FIELDTYPE_INT3:    return "int3";
    case FIELDTYPE_INT4:    return "int4";
    case FIELDTYPE_FLOAT:   return "float";
    case FIELDTYPE_FLOAT2:  return "float2";
    case FIELDTYPE_FLOAT3:  return "float3";
    case FIELDTYPE_FLOAT4:  return "float4";
    case FIELDTYPE_DOUBLE:  return "double";
    case FIELDTYPE_DOUBLE2: return "double2";
    case FIELDTYPE_DOUBLE3: return "double3";
    case FIELDTYPE_DOUBLE4: return "double4";
    default:                return "none";
    }
}
inline std::ostream& operator<<(std::ostream &os, FieldType t) { return (os << to_string(t)); }


inline int numChannels(FieldType t)
{
  switch(t)
    {
    case FIELDTYPE_INT:  case FIELDTYPE_FLOAT:  case FIELDTYPE_DOUBLE:  return 1;
    case FIELDTYPE_INT2: case FIELDTYPE_FLOAT2: case FIELDTYPE_DOUBLE2: return 2;
    case FIELDTYPE_INT3: case FIELDTYPE_FLOAT3: case FIELDTYPE_DOUBLE3: return 3;
    case FIELDTYPE_INT4: case FIELDTYPE_FLOAT4: case FIELDTYPE_DOUBLE4: return 4;
    default: return 0;
    }
}
inline FieldType baseType(FieldType t)
{
  switch(t)
    {
    case FIELDTYPE_INT:    case FIELDTYPE_INT2:    case FIELDTYPE_INT3:    case FIELDTYPE_INT4:    return FIELDTYPE_INT;
    case FIELDTYPE_FLOAT:  case FIELDTYPE_FLOAT2:  case FIELDTYPE_FLOAT3:  case FIELDTYPE_FLOAT4:  return FIELDTYPE_FLOAT;
    case FIELDTYPE_DOUBLE: case FIELDTYPE_DOUBLE2: case FIELDTYPE_DOUBLE3: case FIELDTYPE_DOUBLE4: return FIELDTYPE_DOUBLE;
    default:               return FIELDTYPE_INVALID;
    }
}

//// CUDA FIELD BASE ////
struct CudaFieldBase
{
public:
  Vec2i size         = Vec2i(0, 0);
  int   dataSize     = 0;
  int   typeSize     = 0;
  bool  fftShifted   = false;

  virtual bool allocated() const { return (gCudaInitialized && size.x > 0 && size.y > 0 && dataSize > 0 && typeSize > 0); }
  virtual void create(const Vec2i &sz) = 0;
  virtual void destroy()  = 0;
  virtual void pullData() = 0;
  virtual void pushData() = 0;
  virtual int  N()         const { return 1; }
  virtual bool isTexture() const { return false; }
  virtual bool isFluid()   const { return false; }
  virtual FieldType type() const { return FIELDTYPE_INVALID; }
  virtual std::string hDataStr(const Vec2i &pos) const { return ""; }
  
  std::string typeStr() const
  {
    std::stringstream ss; ss << "CudaField<" << type() << ">  -->  " << size << " = " << dataSize << "*" << typeSize
                             << (isTexture() ? " / TEX " : ")");
    return ss.str();
  }
};

//// CUDA FIELD ////
// --> contains raw field data
template<typename DATA_T>
struct CudaField : public CudaFieldBase
{
public:
  typedef DATA_T dtype;
  
  MandelbrotParams params;
  DATA_T *hData   = nullptr;
  DATA_T *dData   = nullptr;

  virtual bool allocated() const override { return (CudaFieldBase::allocated() && dData && hData); }
  
  virtual void create(const Vec2i &sz) override;
  virtual void destroy() override;
  
  virtual int  N()         const override { return 1; }
  virtual bool isTexture() const override { return false; }
  virtual bool isFluid()   const override { return false; }  

  virtual DATA_T* map() { return dData; }
  virtual void unmap()  { }
  
  virtual void pullData() override
  {
    if(allocated())
      {
        cudaMemcpy(hData, dData, dataSize*typeSize, cudaMemcpyDeviceToHost);
        getLastCudaError("CudaField::pullData()\n");
      } else { std::cout << "====> WARNING(CudaField<" << type() << ">::pullData()): Field not allocated!\n"; }
  }
  virtual void pushData() override
  {
    if(allocated())
      {
        cudaMemcpy(dData, hData, dataSize*typeSize, cudaMemcpyHostToDevice);
        getLastCudaError("CudaField::pushData()\n");
      } else { std::cout << "====> WARNING(CudaField<" << type() << ">::pushData()): Field not allocated!\n"; }
  }
  virtual FieldType type() const override;
  virtual std::string hDataStr(const Vec2i &pos) const override;
};

template<typename DATA_T> inline FieldType CudaField<DATA_T>::type() const { return FIELDTYPE_INVALID; }
template<> inline FieldType CudaField<int    >::type() const { return FIELDTYPE_INT;     }
template<> inline FieldType CudaField<int2   >::type() const { return FIELDTYPE_INT2;    }
template<> inline FieldType CudaField<int3   >::type() const { return FIELDTYPE_INT3;    }
template<> inline FieldType CudaField<int4   >::type() const { return FIELDTYPE_INT4;    }
template<> inline FieldType CudaField<float  >::type() const { return FIELDTYPE_FLOAT;   }
template<> inline FieldType CudaField<float2 >::type() const { return FIELDTYPE_FLOAT2;  }
template<> inline FieldType CudaField<float3 >::type() const { return FIELDTYPE_FLOAT3;  }
template<> inline FieldType CudaField<float4 >::type() const { return FIELDTYPE_FLOAT4;  }
template<> inline FieldType CudaField<double >::type() const { return FIELDTYPE_DOUBLE;  }
template<> inline FieldType CudaField<double2>::type() const { return FIELDTYPE_DOUBLE2; }
template<> inline FieldType CudaField<double3>::type() const { return FIELDTYPE_DOUBLE3; }
template<> inline FieldType CudaField<double4>::type() const { return FIELDTYPE_DOUBLE4; }

template<typename DATA_T> inline std::string CudaField<DATA_T>::hDataStr(const Vec2i &p) const { return "[INVALID TYPE]"; }
template<> inline std::string CudaField<int    >::hDataStr(const Vec2i &p) const { std::stringstream ss; ss << hData[p.y*size.x + p.x]; return ss.str(); }
template<> inline std::string CudaField<int2   >::hDataStr(const Vec2i &p) const { std::stringstream ss; ss << hData[p.y*size.x + p.x]; return ss.str(); }
template<> inline std::string CudaField<int3   >::hDataStr(const Vec2i &p) const { std::stringstream ss; ss << hData[p.y*size.x + p.x]; return ss.str(); }
template<> inline std::string CudaField<int4   >::hDataStr(const Vec2i &p) const { std::stringstream ss; ss << hData[p.y*size.x + p.x]; return ss.str(); }
template<> inline std::string CudaField<float  >::hDataStr(const Vec2i &p) const { std::stringstream ss; ss << hData[p.y*size.x + p.x]; return ss.str(); }
template<> inline std::string CudaField<float2 >::hDataStr(const Vec2i &p) const { std::stringstream ss; ss << hData[p.y*size.x + p.x]; return ss.str(); }
template<> inline std::string CudaField<float3 >::hDataStr(const Vec2i &p) const { std::stringstream ss; ss << hData[p.y*size.x + p.x]; return ss.str(); }
template<> inline std::string CudaField<float4 >::hDataStr(const Vec2i &p) const { std::stringstream ss; ss << hData[p.y*size.x + p.x]; return ss.str(); }
template<> inline std::string CudaField<double >::hDataStr(const Vec2i &p) const { std::stringstream ss; ss << hData[p.y*size.x + p.x]; return ss.str(); }
template<> inline std::string CudaField<double2>::hDataStr(const Vec2i &p) const { std::stringstream ss; ss << hData[p.y*size.x + p.x]; return ss.str(); }
template<> inline std::string CudaField<double3>::hDataStr(const Vec2i &p) const { std::stringstream ss; ss << hData[p.y*size.x + p.x]; return ss.str(); }
template<> inline std::string CudaField<double4>::hDataStr(const Vec2i &p) const { std::stringstream ss; ss << hData[p.y*size.x + p.x]; return ss.str(); }


template<typename DATA_T>
void CudaField<DATA_T>::create(const Vec2i &sz)
{
  initDevice();
  if(gCudaInitialized && sz.x > 0 && sz.y > 0)
    {
      destroy();
      getLastCudaError("BEFORE CudaField::create");

      size = sz; dataSize = sz.x*sz.y; typeSize = sizeof(DATA_T);
      std::cout << "Creating CUDA field (" << sz << " | " << type() << ")...\n";
      std::cout << "   --> dataSize: " << dataSize << " / " << typeSize << "\n";

      hData = (DATA_T*)malloc(typeSize*dataSize);    // initialize host data
      memset(hData, 0, typeSize*dataSize);
      cudaMalloc((void**)&dData, typeSize*dataSize); // initialize device data
      cudaMemset(dData, 0, typeSize*dataSize);

      getLastCudaError("CudaField::create(sz)");
      std::cout << "DONE\n";
    }
  if(!gCudaInitialized)          { std::cout << "====> WARNING(CudaField::create()): CUDA device not initialized!\n"; }
  if(size.x == 0 || size.y == 0) { std::cout << "====> WARNING(CudaField::create()): zero size! " << size << "\n"; }
}

template<typename DATA_T>
void CudaField<DATA_T>::destroy()
{
  if(allocated())
    {
      std::cout << "Destroying CudaField...\n";
      if(dData) { cudaFree(dData); dData = nullptr; }
      if(hData) { free    (hData); hData = nullptr; }
      size = Vec2i(0,0); dataSize = 0; fftShifted = false;
      getLastCudaError("CudaField::destroy(sz)");
      std::cout << "DONE\n";
    }
}


// tool for getting data type for templating
//template<typename F_TYPE> struct GetDataType      { using t = float;   };    // default assumes float field
// template<> struct GetDataType<CudaField<int>>     { using t = int;     };
// template<> struct GetDataType<CudaField<int2>>    { using t = int2;    };
// template<> struct GetDataType<CudaField<int3>>    { using t = int3;    };
// template<> struct GetDataType<CudaField<int4>>    { using t = int4;    };
// template<> struct GetDataType<CudaField<float>>   { using t = float;   };
// template<> struct GetDataType<CudaField<float2>>  { using t = float2;  };
// template<> struct GetDataType<CudaField<float3>>  { using t = float3;  };
// template<> struct GetDataType<CudaField<float4>>  { using t = float4;  };
// template<> struct GetDataType<CudaField<double>>  { using t = double;  };
// template<> struct GetDataType<CudaField<double2>> { using t = double2; };
// template<> struct GetDataType<CudaField<double3>> { using t = double3; };
// template<> struct GetDataType<CudaField<double4>> { using t = double4; };

// template<typename DATA_T>
// DATA_T* getDerived(CudaFieldBase *field) -> decltype(CudaField<DATA_T>->dtype) { return nullptr; }
// template<> auto getDerived<float>  (CudaFieldBase *&field) -> decltype(CudaField<>->dtype) { return reinterpret_cast<float2>(field);  }
// template<> auto getDerived<float2> (CudaFieldBase *&field) -> decltype(CudaField<>->dtype){ return reinterpret_cast<float2>(field);  }
// template<> auto getDerived<float3> (CudaFieldBase *&field) -> decltype(CudaField<double4>->dtype){ return reinterpret_cast<float3>(field);  }
// template<> auto getDerived<float4> (CudaFieldBase *&field) -> decltype(CudaField<double4>->dtype){ return reinterpret_cast<float4>(field);  }
// template<> auto getDerived<double> (CudaFieldBase *&field) -> decltype(CudaField<double4>->dtype){ return reinterpret_cast<double>(field);  }
// template<> auto getDerived<double2>>(CudaFieldBase *&field) -> decltype(CudaField<double4>->dtype){ return reinterpret_cast<double2>(field); }
// template<> auto getDerived<double3>(CudaFieldBase *&field) -> decltype(CudaField<double4>->dtype){ return reinterpret_cast<double3>(field); }
// template<> auto getDerived<double4>(CudaFieldBase *&field) -> decltype(CudaField<double4>->dtype){ return reinterpret_cast<double4>(field); }
// template<> auto getDerived<int>    (CudaFieldBase *&field) -> decltype(CudaField<double4>->dtype){ return reinterpret_cast<int>(field);     }
// template<> auto getDerived<int2>   (CudaFieldBase *&field) -> decltype(CudaField<double4>->dtype){ return reinterpret_cast<int2>(field);    }
// template<> auto getDerived<CudaField<int3>   (CudaFieldBase *&field) -> decltype(CudaField<double4>::dtype){ return reinterpret_cast<CudaField<int3>(field);    }
// template<> auto getDerived<CudaField<int4>   (CudaFieldBase *&field) -> decltype(CudaField<int4>->dtype){ return reinterpret_cast<CudaField<int4>(field);    } 

// template<typename DATA_T>
// auto getDataType(CudaFieldBase *field)     -> decltype(static_cast<CudaField<DATA_T>*>(field))
// {
//   return static_cast<CudaField<DATA_T>*>(field);
// }




//// CUDA TEXTURE ////
// --> contains texture data for a field
struct CudaFieldTex : public CudaField<float4>
{
  float  mult   = 1.0f;
  GLuint glTex  = 0;
  GLuint glPbo  = 0;
  cudaGraphicsResource *mPboResource = nullptr;
  bool   mapped = false;
  bool   bound  = false;

  virtual bool allocated() const override { return (gCudaInitialized && size.x > 0 && size.y > 0 && dataSize > 0 && typeSize > 0 && mPboResource); }
  
  virtual void create(const Vec2i &sz) override;
  virtual void destroy() override;

  virtual void pullData() override { map(); CudaField<float4>::pullData(); unmap(); }
  virtual void pushData() override { map(); CudaField<float4>::pushData(); unmap(); }
  virtual bool isTexture() const override { return true; }

  // GL interop
  void initGL();                  // initialize CUDA-->opengl interop
  void bind();   void release();  // texture data binding for use with opengl
  virtual float4* map() override; // texture data mapping to device pointer for rendering via CUDA kernel
  virtual void unmap() override;  // texture data unmapping
  ImTextureID* texId() const { return reinterpret_cast<ImTextureID*>(glTex); }
};

inline void CudaFieldTex::create(const Vec2i &sz)
{
  initDevice();
  if(gCudaInitialized && sz.x > 0 && sz.y > 0)
    {
      destroy();
      size = sz; dataSize = sz.x*sz.y; typeSize = sizeof(float4);
      std::cout << "Creating CudaFieldTex (" << sz << " | " << type() << ")...\n"
                << "   --> dataSize: " << dataSize << " / " << typeSize << "\n";
      hData = (float4*)malloc(typeSize*dataSize);    // initialize host data
      initGL();
      std::cout << "DONE\n";
      getLastCudaError("CudaFieldTex::create()");
    }
  
  if(!gCudaInitialized)          { std::cout << "====> WARNING(CudaTexField::create()): CUDA device not initialized!\n"; }
  if(size.x == 0 || size.y == 0) { std::cout << "====> WARNING(CudaTexField::create()): zero size! " << size << "\n"; }
  if(!mPboResource)              { std::cout << "====> WARNING(CudaFieldTex::map()): PBO resource not initialized!\n"; }
}

inline void CudaFieldTex::destroy()
{
  if(allocated())
    {
      std::cout << "Destroying CudaFieldTex...\n";
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
      if(hData)        { free(hData); }
      if(mPboResource) { cudaGraphicsUnregisterResource(mPboResource); mPboResource = nullptr; }
      if(glPbo > 0)    { glDeleteBuffers(1,  &glPbo); glPbo = 0; }
      if(glTex > 0)    { glDeleteTextures(1, &glTex); glTex = 0; }
      getLastCudaError("CudaFieldTex::destroy()");
      std::cout << "DONE\n";
    }
  size = Vec2i(0,0); dataSize = 0;
}

inline void CudaFieldTex::initGL()
{
  //if(gCudaInitialized)// && size.x > 0 && size.y > 0)// && !mPboResource)
  {
    // delete old buffers
    if(glTex > 0) { glDeleteTextures(1, &glTex); glTex = 0; }
    if(glPbo > 0) { glDeleteBuffers(1, &glPbo);  glPbo = 0; }
    if(mPboResource) { cudaGraphicsUnregisterResource(mPboResource); mPboResource = nullptr; }
    getLastCudaError("CudaFieldTex-->cudaGraphicsUnregisterResource()\n");
    // OpenGL texture
    glGenTextures(1, &glTex); glBindTexture(GL_TEXTURE_2D, glTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size.x, size.y, 0, GL_RGBA, GL_FLOAT, 0);
    glGenBuffers(1, &glPbo);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, glPbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, size.x*size.y*typeSize, 0, GL_STREAM_COPY);

    cudaGraphicsGLRegisterBuffer(&mPboResource, glPbo, cudaGraphicsMapFlagsWriteDiscard);
    getLastCudaError("CudaFieldTex-->cudaGraphicsUnregisterResource()\n");
    if(!mPboResource)
      {
        std::cout << "====> ERROR(CudaFieldTex::initGL()): mPboResource NUL --> failed to register!\n";
      }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
    getLastCudaError("CudaFieldTex::initGL()\n");
  }
  //else { std::cout << "====> WARNING: CudaFieldTex::initGL() --> uninitialized texture!\n"; }
}

inline void CudaFieldTex::bind()
{
  if(allocated() && !bound)
    {
      // load texture from pbos
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, glPbo);
      // load texture from pbos
      glBindTexture(GL_TEXTURE_2D, glTex);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size.x, size.y, GL_RGBA, GL_FLOAT, 0);
      bound = true;
    }
  else
    {
      if(!gCudaInitialized)          { std::cout << "====> WARNING(CudaTexField::bind()): CUDA device not initialized!\n"; }
      if(size.x == 0 || size.y == 0) { std::cout << "====> WARNING(CudaTexField::bind()): zero size! " << size << "\n";    }
      if(!mPboResource)              { std::cout << "====> WARNING(CudaFieldTex::bind()): PBO resource not initialized!\n";   }
    }
}

inline void CudaFieldTex::release()
{
  if(allocated() && bound)
    {
      glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
      glBindTexture(GL_TEXTURE_2D, 0);
      bound = false;
    }
  else
    {
      if(!gCudaInitialized)          { std::cout << "====> WARNING(CudaTexField::release()): CUDA device not initialized!\n";  }
      if(size.x == 0 || size.y == 0) { std::cout << "====> WARNING(CudaTexField::release()): zero size! " << size << "\n";     }
      if(!mPboResource)              { std::cout << "====> WARNING(CudaFieldTex::release()): PBO resource not initialized!\n"; }
    }
}


inline float4* CudaFieldTex::map()
{
  if(!mPboResource)
    {
      std::cout << "====> WARNING(CudaFieldTex::map()): PBO resource not initialized!\n";
      //create(size); // try to initialize
      return nullptr;
    }
  
  if(allocated())
    {
      if(!mapped)
        {
          int err = cudaGraphicsMapResources(1, &mPboResource, 0);
          if(err == CUDA_ERROR_ALREADY_MAPPED)
            {
              std::cout << "====> WARNING: CudaFieldTex already mapped! (" << err << ") --> cudaGraphicsMapResources\n";
              getLastCudaError("CudaFieldTex::map()\n");
              
              return nullptr;
              //mapped = true; return nullptr;
            }
          else if(err)
            {
              std::cout << "====> WARNING: CudaFieldTex failed to map! (" << err << ") --> cudaGraphicsMapResources\n";
              getLastCudaError("CudaFieldTex::map()\n");
              mapped = false; dData = nullptr; return nullptr;
            }
          
          size_t nbytes;
          err = cudaGraphicsResourceGetMappedPointer((void**)&dData, &nbytes, mPboResource);
          if(err == CUDA_ERROR_ALREADY_MAPPED)
            {
              std::cout << "====> WARNING: CudaFieldTex already mapped! (" << err << ") --> (cudaGraphicsResourceGetMappedPointer)\n";
              getLastCudaError("CudaFieldTex::map()\n");
              mapped = true; return nullptr;
            }
          else if(err)
            {
              std::cout << "====> WARNING: CudaFieldTex failed to map! (" << err << ") --> (cudaGraphicsResourceGetMappedPointer)\n";
              getLastCudaError("CudaFieldTex::map()\n");
              mapped = false; dData = nullptr; return nullptr;
            }
          mapped = true;
          //if(nbytes == 0) { return nullptr; } // TODO: check?
          getLastCudaError("CudaFieldTex::map()\n");
        }
      else { std::cout << "====> WARNING: CudaFieldTex::map() called on mapped texture!\n"; }      
      return dData;
    }
  else
    {
      if(!gCudaInitialized)          { std::cout << "====> WARNING(CudaFieldTex::map()): CUDA device not initialized!\n";  }
      if(size.x == 0 || size.y == 0) { std::cout << "====> WARNING(CudaFieldTex::map()): zero size! " << size << "\n";     }
      if(!mPboResource)              { std::cout << "====> WARNING(CudaFieldTex::map()): PBO resource not initialized!\n"; }
    }
  return nullptr;
}

inline void CudaFieldTex::unmap()
{
  if(gCudaInitialized && size.x > 0 && size.y > 0 && mPboResource)
    {
      if(mapped || dData)
        {
          int err = cudaGraphicsUnmapResources(1, &mPboResource, 0);
          getLastCudaError("CudaFieldTex::unmap()\n");
          if(err)
            {
              std::cout << "====> WARNING: CudaFieldTex failed to unmap (" << err << ")! --> cudaGraphicsUnmapResources\n";
              getLastCudaError("CudaFieldTex::unmap()\n");
              dData = nullptr; mapped = false; return;
            }
          getLastCudaError("CudaFieldTex::unmap()\n");
        }
      else { std::cout << "====> WARNING: CudaFieldTex::unmap() called on unmapped texture!\n"; }
    }
  else
    {
      if(!gCudaInitialized)          { std::cout << "====> WARNING(CudaFieldTex::unmap()): CUDA device not initialized!\n";  }
      if(size.x == 0 || size.y == 0) { std::cout << "====> WARNING(CudaFieldTex::unmap()): zero size! " << size << "\n";     }
      if(!mPboResource)              { std::cout << "====> WARNING(CudaFieldTex::unmap()): PBO resource not initialized!\n"; }
    }
  dData = nullptr; mapped = false;
}



template<typename T> inline T flerp(T x0, T x1, T alpha) { return x0 * alpha + x1 * ((T)1 - alpha); }

//// CUDA FLUID ////
// --> contains raw field data for a fluid simulation
template<typename VT>
struct CudaFluid : public CudaFieldBase
{
  FluidParams   params; // fluid parameters
  Vec2i         size;   // field size
  CudaField<VT> vx;     // X velocity
  CudaField<VT> vy;     // Y velocity
  CudaField<VT> d;      // density at each point
  CudaField<VT> p;      // pressure at each point
  CudaField<VT> div;    // divergence at each point
  CudaField<float2> wv; // wave vector

  virtual bool allocated() const override { return (vx.allocated() && vy.allocated() && d.allocated() && p.allocated() && div.allocated() && wv.allocated()); }

  virtual void create(const Vec2i &sz) override
  { if(sz.x > 0 && sz.y > 0) { vx.create(sz); vy.create(sz); d.create(sz); p.create(sz); div.create(sz); wv.create(sz); size = sz; } }
  virtual void destroy() override
  { if(allocated()) { vx.destroy(); vy.destroy(); d.destroy(); p.destroy(); div.destroy(); wv.destroy();  size = Vec2i(0,0); } }

  virtual void pullData() override { vx.pullData(); vy.pullData(); d.pullData(); p.pullData(); div.pullData(); wv.pullData(); }
  virtual void pushData() override { vx.pushData(); vy.pushData(); d.pushData(); p.pushData(); div.pushData(); wv.pushData(); }

  virtual bool isFluid() const override { return true; }
  virtual FieldType type() const override;

  // sample velocity (bilinear interpolation)
  inline Vector<VT, 2> sampleVel(const Vec2f &pos)
  {
    Vec2i ip = Vec2i(std::floor(pos.x), std::floor(pos.y)); // integer position
    Vec2f fp = Vec2f(pos.x-ip.x, pos.y-ip.y);               // fractional position
    if(ip.x >= 0 && ip.x < size.x-1 && ip.y >= 0 && ip.y <= size.y-1)
      {
        int i00 = ip.y*size.x + ip.x;
        int i01 = ip.y*size.x + ip.x + 1;
        int i10 = (ip.y + 1)*size.x + ip.x;
        int i11 = (ip.y + 1)*size.x + ip.x + 1;
        // bilinear interpolation
        if(i11 < vx.dataSize && i11 < vy.dataSize)
          {
            return Vector<VT, 2>(flerp<VT>(flerp<VT>(vx.hData[i00], vx.hData[i10], fp.x), flerp<VT>(vx.hData[i10], vx.hData[i11], fp.x), fp.y),
                                 flerp<VT>(flerp<VT>(vy.hData[i00], vy.hData[i10], fp.x), flerp<VT>(vy.hData[i10], vy.hData[i11], fp.x), fp.y));
          }
      }
    return Vector<VT, 2>();
  }
};

template<typename DATA_T>
inline FieldType CudaFluid<DATA_T>::type() const             { return FIELDTYPE_INVALID; }
template<> inline FieldType CudaFluid<float>  ::type() const { return FIELDTYPE_FLOAT; }
template<> inline FieldType CudaFluid<float2> ::type() const { return FIELDTYPE_FLOAT2; }
template<> inline FieldType CudaFluid<float3> ::type() const { return FIELDTYPE_FLOAT3; }
template<> inline FieldType CudaFluid<float4> ::type() const { return FIELDTYPE_FLOAT4; }
template<> inline FieldType CudaFluid<double> ::type() const { return FIELDTYPE_DOUBLE; }
template<> inline FieldType CudaFluid<double2>::type() const { return FIELDTYPE_DOUBLE2; }
template<> inline FieldType CudaFluid<double3>::type() const { return FIELDTYPE_DOUBLE3; }
template<> inline FieldType CudaFluid<double4>::type() const { return FIELDTYPE_DOUBLE4; }
template<> inline FieldType CudaFluid<int>    ::type() const { return FIELDTYPE_INT; }
template<> inline FieldType CudaFluid<int2>   ::type() const { return FIELDTYPE_INT2; }
template<> inline FieldType CudaFluid<int3>   ::type() const { return FIELDTYPE_INT3; }
template<> inline FieldType CudaFluid<int4>   ::type() const { return FIELDTYPE_INT4; }

inline CudaFieldBase* makeCudaField(FieldType t)
{
  switch(t)
    {
    case FIELDTYPE_INT:     return new CudaField<int>();
    case FIELDTYPE_INT2:    return new CudaField<int2>();
    case FIELDTYPE_INT3:    return new CudaField<int3>();
    case FIELDTYPE_INT4:    return new CudaField<int4>();
    case FIELDTYPE_FLOAT:   return new CudaField<float>();
    case FIELDTYPE_FLOAT2:  return new CudaField<float2>();
    case FIELDTYPE_FLOAT3:  return new CudaField<float3>();
    case FIELDTYPE_FLOAT4:  return new CudaField<float4>();
    case FIELDTYPE_DOUBLE:  return new CudaField<double>();
    case FIELDTYPE_DOUBLE2: return new CudaField<double2>();
    case FIELDTYPE_DOUBLE3: return new CudaField<double3>();
    case FIELDTYPE_DOUBLE4: return new CudaField<double4>();
    default:                return nullptr;
    }
}

enum ColorChannel
  {
   CHANNEL_NONE = 0x00,
   CHANNEL_R    = 0x01,
   CHANNEL_G    = 0x02,
   CHANNEL_B    = 0x04,
   CHANNEL_A    = 0x08,
  };

// COLOR CHANNELS //
__global__ void fillTexChannel_k(CudaFieldTex  tex, ColorChannel channels, float value);
void fillTexChannel  (CudaFieldTex *tex, ColorChannel channels, float value);

template<typename T> __global__ void renderTexChannel_k   (CudaField<T>   field1, CudaFieldTex  tex, ColorChannel channels,
                                                           Vec2f rrange, Vec2f grange, Vec2f brange, Vec2f arange);
__global__                      void renderTexTexChannel_k(CudaFieldTex   src,    CudaFieldTex  tex, ColorChannel channels);

template<typename T>            void renderTexChannel     (CudaFieldBase *field1, CudaFieldTex *tex, ColorChannel channels=CHANNEL_NONE,
                                                           const Vec2f &rrange=Vec2f(0,1), const Vec2f &grange=Vec2f(0,1),
                                                           const Vec2f &brange=Vec2f(0,1), const Vec2f &arange=Vec2f(0,1));

// NEW TEMPLATED //
template<typename T> __global__ void renderTex_k(CudaField<T>   field1, CudaFieldTex  tex);
template<typename T>            void renderTex  (CudaFieldBase *field1, CudaFieldTex *tex);

// in render.cu
extern "C" void renderFft_real     (CudaField<float2> src, CudaFieldTex dst);
extern "C" void renderFft_imag     (CudaField<float2> src, CudaFieldTex dst);
extern "C" void renderFft_magnitude(CudaField<float2> src, CudaFieldTex dst);
extern "C" void renderFft_phase    (CudaField<float2> src, CudaFieldTex dst);
extern "C" void renderFft_combined (CudaField<float2> src, CudaFieldTex dst);
extern "C" void renderMagnitude    (CudaField<float>  src, CudaFieldTex dst);
extern "C" void multFields         (CudaField<float>  field1, CudaField<float>  field2, CudaField<float>  dst, float mult=1.0f);
extern "C" void multFields2        (CudaField<float2> field1, CudaField<float2> field2, CudaField<float2> dst, float mult=1.0f);
extern "C" void multTextures       (CudaFieldTex      field1, CudaFieldTex      field2, CudaFieldTex      dst, float mult=1.0f);
extern "C" void multFieldConst     (CudaField<float>  field,  CudaField<float>  dst,    float mult=1.0f);
extern "C" void multTextureConst   (CudaFieldTex      field,  CudaFieldTex      dst,    float mult=1.0f);
// in cuda-tools.cu
extern "C" void loadTexMagnitude   (CudaFieldTex      src, CudaField<float>  dst);
extern "C" void loadTexMagnitude2  (CudaFieldTex      src, CudaField<float2> dst);
extern "C" void loadFieldMagnitude (CudaField<float>  src, CudaField<float2> dst);
extern "C" void loadFieldMagnitude2(CudaField<float2> src, CudaField<float2> dst);
extern "C" void complexMagnitude   (CudaField<float2> src, CudaField<float>  dst);
// in fluid.cu
extern "C" void clearFluid      (CudaFluid<float> fluid);
extern "C" void fillFluidCircle (CudaFluid<float> fluid);
extern "C" void fillFluidPattern(CudaFluid<float> fluid);
extern "C" void fluidAdvection  (CudaFluid<float> src, CudaFluid<float> dst);
extern "C" void fluidDiffusion  (CudaFluid<float> src, CudaFluid<float> dst);
extern "C" void fluidProject    (CudaFluid<float> src, CudaFluid<float> dst);
extern "C" void fluidAddForces  (CudaFluid<float> src, CudaFluid<float> dst);
extern "C" void fluidUpdateVel  (CudaFluid<float> src, CudaFluid<float> dst);
extern "C" void renderFluid     (CudaFluid<float> src, CudaFieldTex     dst);
// in mandelbrot.cu
extern "C" void calcMandelbrot (CudaField<float2> field, CudaFieldTex tex);
// in fft.cu
extern "C" void fftShift(CudaField<float2> src, CudaField<float2> dst, bool shift, bool scale);


#endif // CUDA_FIELD_HPP
