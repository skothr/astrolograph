#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <string>
#include <array>
#include <cmath>
#include <istream>
#include <ostream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <type_traits>

#include <vector_types.h>
#ifdef __NVCC__
#include <cuda_runtime.h>
#endif // __NVCC__


// Vector Template Base
template<typename T, int N>
struct Vector
{
  std::array<T, N> data;

  Vector()                              : data{{0}}          { }
  Vector(const Vector<T, N> &other)     : Vector(other.data) { }
  Vector(const std::array<T, N> &data_) : data(data_)        { }
  Vector(T val)                         : data(N, val)       { }
  
  Vector(const std::string &str)          { fromString(str); }
  template<typename U> // convert from other type
  Vector(const Vector<U, N> &other)       { for(int i = 0; i < N; i++) { data[i] = (T)other.data[i]; } }

#ifdef __NVCC__
  __host__ __device__  Vector(const float2 &cv)  { for(int i = 0; i < std::min(N, 2); i++) { data[i] = (T)( (const float*)(&cv))[i]; } }
  __host__ __device__  Vector(const float3 &cv)  { for(int i = 0; i < std::min(N, 3); i++) { data[i] = (T)( (const float*)(&cv))[i]; } }
  __host__ __device__  Vector(const float4 &cv)  { for(int i = 0; i < std::min(N, 4); i++) { data[i] = (T)( (const float*)(&cv))[i]; } }
  __host__ __device__  Vector(const double2 &cv) { for(int i = 0; i < std::min(N, 2); i++) { data[i] = (T)((const double*)(&cv))[i]; } }
  __host__ __device__  Vector(const double3 &cv) { for(int i = 0; i < std::min(N, 3); i++) { data[i] = (T)((const double*)(&cv))[i]; } }
  __host__ __device__  Vector(const double4 &cv) { for(int i = 0; i < std::min(N, 4); i++) { data[i] = (T)((const double*)(&cv))[i]; } }
  __host__ __device__  Vector<T, N>& operator=(const float2 &cv) { for(int i = 0; i < std::min(N, 2); i++) { data[i] = (T)((const float*)(&cv))[i]; } return *this; }
  __host__ __device__  Vector<T, N>& operator=(const float3 &cv) { for(int i = 0; i < std::min(N, 3); i++) { data[i] = (T)((const float*)(&cv))[i]; } return *this; }
  __host__ __device__  Vector<T, N>& operator=(const float4 &cv) { for(int i = 0; i < std::min(N, 4); i++) { data[i] = (T)((const float*)(&cv))[i]; } return *this; }
  __host__ __device__  Vector<T, N>& operator=(const double2 &cv){ for(int i = 0; i < std::min(N, 2); i++) { data[i] = (T)((const double*)(&cv))[i]; } return *this; }
  __host__ __device__  Vector<T, N>& operator=(const double3 &cv){ for(int i = 0; i < std::min(N, 3); i++) { data[i] = (T)((const double*)(&cv))[i]; } return *this; }
  __host__ __device__  Vector<T, N>& operator=(const double4 &cv){ for(int i = 0; i < std::min(N, 4); i++) { data[i] = (T)((const double*)(&cv))[i]; } return *this; }
#else
  Vector(const float2 &cv)  { for(int i = 0; i < std::min(N, 2); i++) { data[i] = (T)( (const float*)(&cv))[i]; } }
  Vector(const float3 &cv)  { for(int i = 0; i < std::min(N, 3); i++) { data[i] = (T)( (const float*)(&cv))[i]; } }
  Vector(const float4 &cv)  { for(int i = 0; i < std::min(N, 4); i++) { data[i] = (T)( (const float*)(&cv))[i]; } }
  Vector(const double2 &cv) { for(int i = 0; i < std::min(N, 2); i++) { data[i] = (T)((const double*)(&cv))[i]; } }
  Vector(const double3 &cv) { for(int i = 0; i < std::min(N, 3); i++) { data[i] = (T)((const double*)(&cv))[i]; } }
  Vector(const double4 &cv) { for(int i = 0; i < std::min(N, 4); i++) { data[i] = (T)((const double*)(&cv))[i]; } }
  Vector<T, N>& operator=(const float2 &cv) { for(int i = 0; i < std::min(N, 2); i++) { data[i] = (T)((const float*)(&cv))[i]; } return *this; }
  Vector<T, N>& operator=(const float3 &cv) { for(int i = 0; i < std::min(N, 3); i++) { data[i] = (T)((const float*)(&cv))[i]; } return *this; }
  Vector<T, N>& operator=(const float4 &cv) { for(int i = 0; i < std::min(N, 4); i++) { data[i] = (T)((const float*)(&cv))[i]; } return *this; }
  Vector<T, N>& operator=(const double2 &cv){ for(int i = 0; i < std::min(N, 2); i++) { data[i] = (T)((const double*)(&cv))[i]; } return *this; }
  Vector<T, N>& operator=(const double3 &cv){ for(int i = 0; i < std::min(N, 3); i++) { data[i] = (T)((const double*)(&cv))[i]; } return *this; }
  Vector<T, N>& operator=(const double4 &cv){ for(int i = 0; i < std::min(N, 4); i++) { data[i] = (T)((const double*)(&cv))[i]; } return *this; }
#endif // __NVCC__  

  std::string toString() const            { std::ostringstream ss;      ss << (*this); return ss.str(); }
  void fromString(const std::string &str) { std::istringstream ss(str); ss >> (*this); }

  T& operator[](int dim)             { return data[dim]; }
  const T& operator[](int dim) const { return data[dim]; }
  
  Vector<T, N>& operator=(T scalar)
  {
    for(int i = 0; i < N; i++) { data[i] = scalar; }
    return *this;
  }
  Vector<T, N>& operator=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++) { data[i] = other.data[i]; }
    return *this;
  }
  bool operator==(const Vector<T, N> &other) const
  {
    for(int i = 0; i < N; i++)
      { if(data[i] != other.data[i]) return false; }
    return true;
  }
  bool operator!=(const Vector<T, N> &other) const { return !(*this == other); }

  Vector<T, N>& operator+=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++) { data[i] += other.data[i]; }
    return *this;
  }
  Vector<T, N>& operator-=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++) { data[i] -= other.data[i]; }
    return *this;
  }

  void ceil()  { for(auto &d : data) { d = std::ceil(d);  } }
  void floor() { for(auto &d : data) { d = std::floor(d); } }
  Vector<T, N> getCeil() const  { Vector<T, N> v(*this); v.ceil(); return v; }
  Vector<T, N> getFloor() const { Vector<T, N> v(*this); v.floor(); return v; }

  T length2() const
  {
    T sqsum = T();
    for(auto d : data) { sqsum += d*d; }
    return sqsum;
  }
  T length() const                { return sqrt(length2()); }
  Vector<T, N> normalized() const { return Vector<T, N>(*this) / length(); }
  template<typename U>
  T dot(const Vector<U, N> &other)
  {
    T total = 0;
    for(int i = 0; i < N; i++) { total += data[i] * other.data[i]; }
    return total;
  }
};

//Shorthand typedefs
template<typename T> using Vec1 = Vector<T, 1>;
template<typename T> using Vec2 = Vector<T, 2>;
template<typename T> using Vec3 = Vector<T, 3>;
template<typename T> using Vec4 = Vector<T, 4>;
template<int N> using Vecf = Vector<float, N>;
template<int N> using Vecd = Vector<double, N>;
typedef Vector<int, 1>         Vec1i;
typedef Vector<int, 2>         Vec2i;
typedef Vector<int, 3>         Vec3i;
typedef Vector<int, 4>         Vec4i;
typedef Vector<float, 1>       Vec1f;
typedef Vector<float, 2>       Vec2f;
typedef Vector<float, 3>       Vec3f;
typedef Vector<float, 4>       Vec4f;
typedef Vector<double, 1>      Vec1d;
typedef Vector<double, 2>      Vec2d;
typedef Vector<double, 3>      Vec3d;
typedef Vector<double, 4>      Vec4d;
typedef Vector<long double, 1> Vec1l;
typedef Vector<long double, 2> Vec2l;
typedef Vector<long double, 3> Vec3l;
typedef Vector<long double, 4> Vec4l;

template<typename T, int N>
std::ostream& operator<<(std::ostream &os, const Vector<T, N> &v)
{
  os << "<";
  for(int i = 0; i < N; i++)
    { os << v.data[i] << ((i < N-1) ? ", " : ""); }
  os << ">";
  return os;
}
template<typename T, int N>
std::istream& operator>>(std::istream &is, Vector<T, N> &v)
{
  is.ignore(1,'<');
  for(int i = 0; i < N; i++) { is >> v.data[i]; is.ignore(1,','); }
  is.ignore(1,'>');
  return is;
}

template<typename T>
struct Vector<T, 2>
{
  static constexpr int N = 2;
  union
  {
    struct { T x, y; };
    std::array<T, N> data;
  };

  Vector()                              : x(0), y(0)                { }
  Vector(T x_, T y_)                    : x(x_), y(y_)              { }
  Vector(const Vector<T, N> &other)     : x(other.x), y(other.y)    { }
  Vector(const std::array<T, N> &data_) : x(data_[0]), y(data_[1])  { }
  Vector(T val)                         : x(val), y(val)            { }
  Vector(const std::string &str)        { fromString(str); }
  template<typename U> // convert from other type
  Vector(const Vector<U, N> &other)     { for(int i = 0; i < N; i++) { data[i] = (T)other.data[i]; } }
  
  Vector<T, 2>& operator=(const Vector<T, 2> &other) { data = other.data; return *this; }
  Vector<T, N>& operator=(T scalar)                  { for(int i = 0; i < N; i++) { data[i] = scalar; } return *this; }

#ifdef __NVCC__
  __host__ __device__  Vector(const float2 &cv)  : x((T)cv.x), y((T)cv.y) { }
  __host__ __device__  Vector(const double2 &cv) : x((T)cv.x), y((T)cv.y) { }
  __host__ __device__  Vector<T, N>& operator=(const float2 &cv)  { x = (T)cv.x; y = (T)cv.y; return *this; }
  __host__ __device__  Vector<T, N>& operator=(const double2 &cv) { x = (T)cv.x; y = (T)cv.y; return *this; }
#else
  Vector(const float2 &cv)  : x((T)cv.x), y((T)cv.y) { }
  Vector(const double2 &cv) : x((T)cv.x), y((T)cv.y) { }
  Vector<T, N>& operator=(const float2 &cv)  { x = (T)cv.x; y = (T)cv.y; return *this; }
  Vector<T, N>& operator=(const double2 &cv) { x = (T)cv.x; y = (T)cv.y; return *this; }
#endif // __NVCC__

  T& operator[](int dim)             { return data[dim]; }
  const T& operator[](int dim) const { return data[dim]; }
  
  std::string toString() const            { std::ostringstream ss;      ss << (*this); return ss.str(); }
  void fromString(const std::string &str) { std::istringstream ss(str); ss >> (*this); }
  
  bool operator==(const Vector<T, N> &other) const
  {
    for(int i = 0; i < N; i++)
      { if(data[i] != other.data[i]) return false; }
    return true;
  }
  bool operator!=(const Vector<T, N> &other) const { return !(*this == other); }
  
  Vector<T, N>& operator+=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++) { data[i] += other.data[i]; }
    return *this;
  }
  Vector<T, N>& operator-=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++) { data[i] -= other.data[i]; }
    return *this;
  }
  Vector<T, N> operator+(const Vector<T, N> &other) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++) { result.data[i] += other.data[i]; }
    return result;
  }
  Vector<T, N> operator-(const Vector<T, N> &other) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++) { result.data[i] -= other.data[i]; }
    return result;
  }
  Vector<T, N>& operator*=(T scalar)
  { for(int i = 0; i < N; i++) { data[i] *= scalar; } return *this; }
  Vector<T, N>& operator/=(T scalar)
  { for(int i = 0; i < N; i++) { data[i] /= scalar; } return *this; }
  Vector<T, N> operator*(T scalar) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++) { result.data[i] *= scalar; }
    return result;
  }
  Vector<T, N> operator/(T scalar) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++) { result.data[i] /= scalar; }
    return result;
  }

  Vector<T, N>& operator*=(const Vector<T, N> &other)
  { for(int i = 0; i < N; i++) { data[i] *= other.data[i]; } return *this; }
  Vector<T, N>& operator/=(const Vector<T, N> &other)
  { for(int i = 0; i < N; i++) { data[i] /= other.data[i]; } return *this; }
  Vector<T, N> operator*(const Vector<T, N> &other) const
  {
    Vector<T, N> result(data);
    for(int i = 0; i < N; i++) { result.data[i] *= other.data[i]; }
    return result;
  }
  Vector<T, N> operator/(const Vector<T, N> &other) const
  {
    Vector<T, N> result(data);
    for(int i = 0; i < N; i++) { result.data[i] /= other.data[i]; }
    return result;
  }
  
  void ceil()  { for(auto &d : data) { d = std::ceil(d);  } }
  void floor() { for(auto &d : data) { d = std::floor(d); } }
  Vector<T, N> getCeil() const  { Vector<T, N> v(*this);  v.ceil();  return v; }
  Vector<T, N> getFloor() const { Vector<T, N> v = *this; v.floor(); return v; }
  
  T length2() const
  {
    T sqsum = T();
    for(auto d : data) { sqsum += d*d; }
    return sqsum;
  }
  T length() const { return sqrt(length2()); }
  void normalize()                { (*this) /= length(); }
  Vector<T, N> normalized() const { return Vector<T, N>(*this) / length(); }
  template<typename U>
  T dot(const Vector<U, N> &other)
  {
    T total = 0;
    for(int i = 0; i < N; i++) { total += data[i] * other.data[i]; }
    return total;
  }
};
  
template<typename T>
struct Vector<T, 3>
{
  static constexpr int N = 3;
  union
  {
    struct { T x, y, z; };
    std::array<T, N> data;
  };

  Vector()                              : x(0), y(0), z(0)                        { }
  Vector(T x_, T y_, T z_)              : x(x_), y(y_), z(z_)                     { }
  Vector(const Vector<T, N> &other)     : x(other.x), y(other.y), z(other.z)      { }
  Vector(const std::array<T, N> &data_) : x(data_[0]), y(data_[1]), z(data_[2])   { }
  Vector(T val)                         : x(val), y(val), z(val)                  { }
  
  Vector(const std::string &str)        { fromString(str); }
  template<typename U> // convert from other type
  Vector(const Vector<U, N> &other)     { for(int i = 0; i < N; i++) { data[i] = (T)other.data[i]; } }
  
  Vector<T, 2>& operator=(const Vector<T, 2> &other) { data = other.data; return *this; }
  Vector<T, N>& operator=(T scalar)                  { for(int i = 0; i < N; i++) { data[i] = scalar; }    return *this; }
  
#ifdef __NVCC__
  __host__ __device__  Vector(const float3 &cv)  : x((T)cv.x), y((T)cv.y), z((T)cv.z) { }
  __host__ __device__  Vector(const double3 &cv) : x((T)cv.x), y((T)cv.y), z((T)cv.z) { }
  __host__ __device__  Vector<T, N>& operator=(const float3 &cv)  { x = (T)cv.x; y = (T)cv.y; z = (T)cv.z; return *this; }
  __host__ __device__  Vector<T, N>& operator=(const double3 &cv) { x = (T)cv.x; y = (T)cv.y; z = (T)cv.z; return *this; }
#else
  Vector(const float3 &cv)  : x((T)cv.x), y((T)cv.y), z((T)cv.z) { }
  Vector(const double3 &cv) : x((T)cv.x), y((T)cv.y), z((T)cv.z) { }
  Vector<T, N>& operator=(const float3 &cv)  { x = (T)cv.x; y = (T)cv.y; z = (T)cv.z; return *this; }
  Vector<T, N>& operator=(const double3 &cv) { x = (T)cv.x; y = (T)cv.y; z = (T)cv.z; return *this; }
#endif // __NVCC__
  
  T& operator[](int dim)             { return data[dim]; }
  const T& operator[](int dim) const { return data[dim]; }
  
  std::string toString() const            { std::ostringstream ss; ss << (*this); return ss.str(); }
  void fromString(const std::string &str) { std::istringstream ss(str); ss >> (*this); }

  bool operator==(const Vector<T, N> &other) const
  {
    for(int i = 0; i < N; i++)
      { if(data[i] != other.data[i]) return false; }
    return true;
  }
  bool operator!=(const Vector<T, N> &other) const { return !(*this == other); }
  
  Vector<T, N>& operator+=(const Vector<T, N> &other)
  { for(int i = 0; i < N; i++) { data[i] += other.data[i]; } return *this; }
  Vector<T, N>& operator-=(const Vector<T, N> &other)
  { for(int i = 0; i < N; i++) { data[i] -= other.data[i]; } return *this; }
  Vector<T, N> operator+(const Vector<T, N> &other) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++)
      { result.data[i] += other.data[i]; }
    return result;
  }
  Vector<T, N> operator-(const Vector<T, N> &other) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++) { result.data[i] -= other.data[i]; }
    return result;
  }
  
  Vector<T, N>& operator*=(T scalar)
  { for(int i = 0; i < N; i++) { data[i] *= scalar; } return *this; }
  Vector<T, N>& operator/=(T scalar)
  { for(int i = 0; i < N; i++) { data[i] /= scalar; } return *this; }
  Vector<T, N> operator*(T scalar) const
  {
    Vector<T, N> result(data);
    for(int i = 0; i < N; i++) { result.data[i] *= scalar; }
    return result;
  }
  Vector<T, N> operator/(T scalar) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++) { result.data[i] /= scalar; }
    return result;
  }
  
  Vector<T, N>& operator*=(const Vector<T, N> &other)
  { for(int i = 0; i < N; i++) { data[i] *= other.data[i]; } return *this; }
  Vector<T, N>& operator/=(const Vector<T, N> &other)
  { for(int i = 0; i < N; i++) { data[i] /= other.data[i]; } return *this; }
  Vector<T, N> operator*(const Vector<T, N> &other) const
  {
    Vector<T, N> result(data);
    for(int i = 0; i < N; i++) { result.data[i] *= other.data[i]; }
    return result;
  }
  Vector<T, N> operator/(const Vector<T, N> &other) const
  {
    Vector<T, N> result(data);
    for(int i = 0; i < N; i++) { result.data[i] /= other.data[i]; }
    return result;
  }
  
  void ceil()  { for(auto &d : data) { d = std::ceil(d); } }
  void floor() { for(auto &d : data) { d = std::floor(d); } }
  Vector<T, N> getCeil() const  { Vector<T, N> v(*this); v.ceil();  return v; }
  Vector<T, N> getFloor() const { Vector<T, N> v(*this); v.floor(); return v; }

  T length2() const
  {
    T sqsum = T();
    for(auto d : data) { sqsum += d*d; }
    return sqsum;
  }
  T length() const                { return sqrt(length2()); }
  void normalize()                { (*this) /= length(); }
  Vector<T, N> normalized() const { return Vector<T, N>(*this) / length(); }
  template<typename U>
  T dot(const Vector<U, N> &other)
  {
    T total = 0;
    for(int i = 0; i < N; i++) { total += data[i] * other.data[i]; }
    return total;
  }
};

template<typename T>
struct Vector<T, 4>
{
  static constexpr int N = 4;
  union
  {
    struct { T x, y, z, w; };
    std::array<T, N> data;
  };

  Vector()                              : x((T)0), y((T)0), z((T)0), w((T)1)                 { }
  Vector(T x_, T y_, T z_, T w_)        : x(x_), y(y_), z(z_), w(w_)                         { }
  Vector(const Vector<T, N> &other)     : x(other.x), y(other.y), z(other.z), w(other.w)     { }
  Vector(const std::array<T, N> &data_) : x(data_[0]), y(data_[1]), z(data_[2]), w(data_[3]) { }
  Vector(T val)                         : x(val), y(val), z(val), w(val)                     { }
  Vector(const std::string &str)        { fromString(str); }
  template<typename U> // convert from other type
  Vector(const Vector<U, N> &other)     { for(int i = 0; i < N; i++) { data[i] = (T)other.data[i]; } }
  
  Vector<T, 4>& operator=(const Vector<T, 4> &other) { data = other.data; return *this; }
  Vector<T, N>& operator=(T scalar)                  { for(int i = 0; i < N; i++) { data[i] = scalar; } return *this; }

#ifdef __NVCC__
  __host__ __device__ Vector(const float4 &cv)  : x((T)cv.x), y((T)cv.y), z((T)cv.z), w((T)cv.w) { }
  __host__ __device__ Vector(const double4 &cv) : x((T)cv.x), y((T)cv.y), z((T)cv.z), w((T)cv.w) { }
  __host__ __device__ Vector<T, N>& operator=(const float4 &cv)  { x = (T)cv.x; y = (T)cv.y; z = (T)cv.z; w = (T)cv.w; return *this; }
  __host__ __device__ Vector<T, N>& operator=(const double4 &cv) { x = (T)cv.x; y = (T)cv.y; z = (T)cv.z; w = (T)cv.w; return *this; }
#else
  Vector(const float4 &cv)  : x((T)cv.x), y((T)cv.y), z((T)cv.z), w((T)cv.w) { }
  Vector(const double4 &cv) : x((T)cv.x), y((T)cv.y), z((T)cv.z), w((T)cv.w) { }
  Vector<T, N>& operator=(const float4 &cv)  { x = (T)cv.x; y = (T)cv.y; z = (T)cv.z; w = (T)cv.w; return *this; }
  Vector<T, N>& operator=(const double4 &cv) { x = (T)cv.x; y = (T)cv.y; z = (T)cv.z; w = (T)cv.w; return *this; }
#endif // __NVCC__
  
  T& operator[](int dim)               { return data[dim]; }
  const T& operator[](int dim) const   { return data[dim]; }
  
  std::string toString() const            { std::ostringstream ss; ss << (*this); return ss.str(); }
  void fromString(const std::string &str) { std::istringstream ss(str); ss >> (*this); }

  bool operator==(const Vector<T, N> &other) const
  {
    for(int i = 0; i < N; i++)
      { if(data[i] != other.data[i]) return false; }
    return true;
  }
  bool operator!=(const Vector<T, N> &other) const { return !(*this == other); }

  Vector<T, N>& operator+=(const Vector<T, N> &other)
  { for(int i = 0; i < N; i++) { data[i] += other.data[i]; } return *this; }
  Vector<T, N>& operator-=(const Vector<T, N> &other)
  { for(int i = 0; i < N; i++) { data[i] -= other.data[i]; } return *this; }
  Vector<T, N> operator+(const Vector<T, N> &other) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++) { result.data[i] += other.data[i]; }
    return result;
  }
  Vector<T, N> operator-(const Vector<T, N> &other) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++) { result.data[i] -= other.data[i]; }
    return result;
  }

  Vector<T, N>& operator*=(T scalar)
  { for(int i = 0; i < N; i++) { data[i] *= scalar; } return *this; }
  Vector<T, N>& operator/=(T scalar)
  { for(int i = 0; i < N; i++) { data[i] /= scalar; } return *this; }
  
  Vector<T, N> operator*(T scalar) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++) { result.data[i] *= scalar; }
    return result;
  }
  Vector<T, N> operator/(T scalar) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++) { result.data[i] /= scalar; }
    return result;
  }
  
  Vector<T, N>& operator*=(const Vector<T, N> &other)
  { for(int i = 0; i < N; i++) { data[i] *= other.data[i]; } return *this; }
  Vector<T, N>& operator/=(const Vector<T, N> &other)
  { for(int i = 0; i < N; i++) { data[i] /= other.data[i]; } return *this; }
  Vector<T, N> operator*(const Vector<T, N> &other) const
  {
    Vector<T, N> result(data);
    for(int i = 0; i < N; i++) { result.data[i] *= other.data[i]; }
    return result;
  }
  Vector<T, N> operator/(const Vector<T, N> &other) const
  {
    Vector<T, N> result(data);
    for(int i = 0; i < N; i++) { result.data[i] /= other.data[i]; }
    return result;
  }
  
  void ceil()  { for(auto &d : data) { d = std::ceil(d);  } }
  void floor() { for(auto &d : data) { d = std::floor(d); } }
  Vector<T, N> getCeil() const  { Vector<T, N> v(*this); v.ceil(); return v; }
  Vector<T, N> getFloor() const { Vector<T, N> v(*this); v.floor(); return v; }

  T length2() const
  {
    T sqsum = T();
    for(auto d : data) { sqsum += d*d; }
    return sqsum;
  }
  T length() const { return sqrt(length2()); }
  void normalize()                { (*this) /= length(); }
  Vector<T, N> normalized() const { return Vector<T, N>(*this) / length(); }
  template<typename U>
  T dot(const Vector<U, N> &other)
  {
    T total = 0;
    for(int i = 0; i < N; i++) { total += data[i] * other.data[i]; }
    return total;
  }
};
template<typename T, int N>
inline Vector<T, N> operator-(const Vector<T, N> &v)
{
  Vector<T, N> result;
  for(int i = 0; i < N; i++) { result.data[i] = -v[i]; }
  return result;
}
template<typename T, int N>
inline Vector<T, N> operator*(T scalar, const Vector<T, N> &v)
{
  Vector<T, N> result;
  for(int i = 0; i < N; i++) { result.data[i] = scalar * v[i]; }
  return result;
}
template<typename T, int N>
inline Vector<T, N> operator/(T scalar, const Vector<T, N> &v)
{
  Vector<T, N> result;
  for(int i = 0; i < N; i++) { result.data[i] = scalar / v[i]; }
  return result;
}



// // Vector that can contain any number of elements (via std::vector)
// template<typename T>
// struct NVector
// {
//   int N = 0; // set to number of dimensions (size of data)
//   std::vector<T> data;

//   NVector(int dim)                     : data(dim, (T)0)                  { } // allocate dimension dize of zeros
//   NVector(int dim, T val)              : data(dim, val) : N(data_.size()) { } // allocate dimension dize of single value
//   NVector(const NVector<T> &other)     : NVector(other.data)              { }
//   NVector(const std::vector<T> &data_) : data(data_) : N(data_.size())    { }
  
//   NVector(const std::string &str)  { fromString(str); }
//   template<typename U> // convert from other type
//   NVector(const NVector<T> &other) { N = other.data.size(); for(int i = 0; i < std::min(N, other.N); i++) { data[i] = (T)other.data[i]; } }

// #ifdef __NVCC__
//   __host__ __device__  NVector(const float2 &cv)  { for(int i = 0; i < std::min(N, 2); i++) { data[i] = (T)( (const float*)(&cv))[i]; } }
//   __host__ __device__  NVector(const float3 &cv)  { for(int i = 0; i < std::min(N, 3); i++) { data[i] = (T)( (const float*)(&cv))[i]; } }
//   __host__ __device__  NVector(const float4 &cv)  { for(int i = 0; i < std::min(N, 4); i++) { data[i] = (T)( (const float*)(&cv))[i]; } }
//   __host__ __device__  NVector(const double2 &cv) { for(int i = 0; i < std::min(N, 2); i++) { data[i] = (T)((const double*)(&cv))[i]; } }
//   __host__ __device__  NVector(const double3 &cv) { for(int i = 0; i < std::min(N, 3); i++) { data[i] = (T)((const double*)(&cv))[i]; } }
//   __host__ __device__  NVector(const double4 &cv) { for(int i = 0; i < std::min(N, 4); i++) { data[i] = (T)((const double*)(&cv))[i]; } }
//   __host__ __device__  NVector<T>& operator=(const float2 &cv)
//   { for(int i = 0; i < std::min(N, 2); i++) { data[i] = (T)((const float*)(&cv))[i];  } return *this; }
//   __host__ __device__  NVector<T>& operator=(const float3 &cv)
//   { for(int i = 0; i < std::min(N, 3); i++) { data[i] = (T)((const float*)(&cv))[i];  } return *this; }
//   __host__ __device__  NVector<T>& operator=(const float4 &cv)
//   { for(int i = 0; i < std::min(N, 4); i++) { data[i] = (T)((const float*)(&cv))[i];  } return *this; }
//   __host__ __device__  NVector<T>& operator=(const double2 &cv)
//   { for(int i = 0; i < std::min(N, 2); i++) { data[i] = (T)((const double*)(&cv))[i]; } return *this; }
//   __host__ __device__  NVector<T>& operator=(const double3 &cv)
//   { for(int i = 0; i < std::min(N, 3); i++) { data[i] = (T)((const double*)(&cv))[i]; } return *this; }
//   __host__ __device__  NVector<T>& operator=(const double4 &cv)
//   { for(int i = 0; i < std::min(N, 4); i++) { data[i] = (T)((const double*)(&cv))[i]; } return *this; }
// #endif // __NVCC__  

//   std::string toString() const            { std::ostringstream ss;      ss << (*this); return ss.str(); }
//   void fromString(const std::string &str) { std::istringstream ss(str); ss >> (*this); N = size(); }

//   T& operator[](int dim)             { return data[dim]; }
//   const T& operator[](int dim) const { return data[dim]; }
  
//   NVector<T>& operator=(T scalar)
//   {
//     for(int i = 0; i < N; i++) { data[i] = scalar; }
//     return *this;
//   }
//   NVector<T>& operator=(const NVector<T> &other)
//   {
//     for(int i = 0; i < N; i++) { data[i] = other.data[i]; }
//     N = other.N;
//     return *this;
//   }
//   bool operator==(const NVector<T> &other) const
//   {
//     for(int i = 0; i < N; i++) { if(data[i] != other.data[i]) return false; }
//     return true;
//   }
//   bool operator!=(const NVector<T> &other) const { return !(*this == other); }

//   NVector<T>& operator+=(const NVector<T> &other)
//   {
//     for(int i = 0; i < N; i++) { data[i] += other.data[i]; }
//     return *this;
//   }
//   NVector<T>& operator-=(const NVector<T> &other)
//   {
//     for(int i = 0; i < N; i++) { data[i] -= other.data[i]; }
//     return *this;
//   }

//   void ceil()  { for(auto &d : data) { d = std::ceil(d);  } }
//   void floor() { for(auto &d : data) { d = std::floor(d); } }
//   NVector<T> getCeil() const  { NVector<T> v(*this); v.ceil(); return v; }
//   NVector<T> getFloor() const { NVector<T> v(*this); v.floor(); return v; }

//   T length2() const
//   {
//     T sqsum = T();
//     for(auto d : data) { sqsum += d*d; }
//     return sqsum;
//   }
//   T length() const                { return sqrt(length2()); }
//   NVector<T> normalized() const { return NVector<T>(*this) / length(); }
//   template<typename U>
//   T dot(const NVector<U, N> &other)
//   {
//     T total = 0;
//     for(int i = 0; i < N; i++) { total += data[i] * other.data[i]; }
//     return total;
//   }
// };









// glsl/cuda-like functions
template<typename T, int N>
inline Vector<T, N> normalize(const Vector<T, N> &v) { return v.normalized(); }
template<typename T, int N>
inline T length2(const Vector<T, N> &v) { return v.length2(); }
template<typename T, int N>
inline T length(const Vector<T, N> &v) { return v.length(); }
template<typename T, int N>
inline T dot(const Vector<T, N> &v1, const Vector<T, N> &v2) { return v1.dot(v2); }

template<typename T>
inline Vector<T, 2> cMult(const Vector<T, 2> &a, const Vector<T, 2> &b) { return Vector<T, 2>(a.x*a.x - a.y*a.y, a.x*b.y + a.y*b.x); }
template<typename T>
inline Vector<T, 2> cConj(const Vector<T, 2> &a) { return Vector<T, 2>(a.x, -a.y); }

template<typename T>
inline Vector<T, 4> qMult(const Vector<T, 4> &a, const Vector<T, 4> &b)
{
  return Vector<T, 4>(a.x*b.x - a.y*b.y - a.z*b.z - a.w*b.w,
                      a.x*b.y + a.y*b.x + a.z*b.w - a.w*b.z,
                      a.x*b.z - a.y*b.w + a.z*b.x + a.w*b.y,
                      a.x*b.w + a.y*b.z - a.z*b.y + a.w*b.x);
}
template<typename T>
inline Vector<T, 4> qConj(const Vector<T, 4> &a) { return Vector<T, 4>(a.x, -a.y, -a.z, -a.w); }


template<typename T>
inline Vector<T, 3> cross(const Vector<T, 3> &a, const Vector<T, 3> &b)
{ return Vector<T, 3>(a.y, a.z, a.x) * Vector<T, 3>(b.z, b.x, b.y) - Vector<T, 3>(a.z, a.x, a.y) * Vector<T, 3>(b.y, b.z, b.x); }

template<typename T>
inline Vector<T, 3> rotate(const Vector<T, 3> &v, const Vector<T, 3> &ax, T theta)
{ // rotate via quaternions
  T cos_t2 = (T)cos(theta/2.0);
  T sin_t2 = (T)sin(theta/2.0);
  Vector<T, 4> q1(0, v.x, v.y, v.z);
  Vector<T, 4> q2(cos_t2, ax.x*sin_t2, ax.y*sin_t2, ax.z*sin_t2);
  Vector<T, 4> q3 = qMult(qMult(q2, q1), qConj(q2));
  return Vector<T, 3>(q3.y, q3.z, q3.w);
}


#ifdef __NVCC__
__host__ __device__ inline int2    to_cuda(const Vec2i &v) { return int2   {v.x, v.y}; }
__host__ __device__ inline int3    to_cuda(const Vec3i &v) { return int3   {v.x, v.y, v.z}; }
__host__ __device__ inline int4    to_cuda(const Vec4i &v) { return int4   {v.x, v.y, v.z, v.w}; }
__host__ __device__ inline float2  to_cuda(const Vec2f &v) { return float2 {v.x, v.y}; }
__host__ __device__ inline float3  to_cuda(const Vec3f &v) { return float3 {v.x, v.y, v.z}; }
__host__ __device__ inline float4  to_cuda(const Vec4f &v) { return float4 {v.x, v.y, v.z, v.w}; }
__host__ __device__ inline double2 to_cuda(const Vec2d &v) { return double2{v.x, v.y}; }
__host__ __device__ inline double3 to_cuda(const Vec3d &v) { return double3{v.x, v.y, v.z}; }
__host__ __device__ inline double4 to_cuda(const Vec4d &v) { return double4{v.x, v.y, v.z, v.w}; }
#else
inline int2    to_cuda(const Vec2i &v) { return int2   {v.x, v.y}; }
inline int3    to_cuda(const Vec3i &v) { return int3   {v.x, v.y, v.z}; }
inline int4    to_cuda(const Vec4i &v) { return int4   {v.x, v.y, v.z, v.w}; }
inline float2  to_cuda(const Vec2f &v) { return float2 {v.x, v.y}; }
inline float3  to_cuda(const Vec3f &v) { return float3 {v.x, v.y, v.z}; }
inline float4  to_cuda(const Vec4f &v) { return float4 {v.x, v.y, v.z, v.w}; }
inline double2 to_cuda(const Vec2d &v) { return double2{v.x, v.y}; }
inline double3 to_cuda(const Vec3d &v) { return double3{v.x, v.y, v.z}; }
inline double4 to_cuda(const Vec4d &v) { return double4{v.x, v.y, v.z, v.w}; }
#endif // __NVCC__

#endif //VECTOR_HPP
