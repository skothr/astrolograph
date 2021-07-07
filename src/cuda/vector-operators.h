#ifndef CUDA_VECTOR_OPERATORS_H
#define CUDA_VECTOR_OPERATORS_H

#ifdef ENABLE_CUDA
#include <cuda_runtime.h>
#include <vector_types.h>
#include <ostream>

// printing
__host__ inline std::ostream& operator<<(std::ostream &os, const int2    &v) { os << "< " << v.x << ", " << v.y << " >"; return os; }
__host__ inline std::ostream& operator<<(std::ostream &os, const int3    &v) { os << "< " << v.x << ", " << v.y << ", " << v.z << " >"; return os; }
__host__ inline std::ostream& operator<<(std::ostream &os, const int4    &v) { os << "< " << v.x << ", " << v.y << ", " << v.z << ", " << v.w << " >"; return os; }
__host__ inline std::ostream& operator<<(std::ostream &os, const float2  &v) { os << "< " << v.x << ", " << v.y << " >"; return os; }
__host__ inline std::ostream& operator<<(std::ostream &os, const float3  &v) { os << "< " << v.x << ", " << v.y << ", " << v.z << " >"; return os; }
__host__ inline std::ostream& operator<<(std::ostream &os, const float4  &v) { os << "< " << v.x << ", " << v.y << ", " << v.z << ", " << v.w << " >"; return os; }
__host__ inline std::ostream& operator<<(std::ostream &os, const double2 &v) { os << "< " << v.x << ", " << v.y << " >"; return os; }
__host__ inline std::ostream& operator<<(std::ostream &os, const double3 &v) { os << "< " << v.x << ", " << v.y << ", " << v.z << " >"; return os; }
__host__ inline std::ostream& operator<<(std::ostream &os, const double4 &v) { os << "< " << v.x << ", " << v.y << ", " << v.z << ", " << v.w << " >"; return os; }

// NEGATION
__host__ __device__  inline float2  operator-(const float2  &u) { return float2 {u.x, u.y}; }
__host__ __device__  inline float3  operator-(const float3  &u) { return float3 {u.x, u.y, u.z}; }
__host__ __device__  inline float4  operator-(const float4  &u) { return float4 {u.x, u.y, u.z, u.w}; }
__host__ __device__  inline double2 operator-(const double2 &u) { return double2{-u.x, -u.y}; }
__host__ __device__  inline double3 operator-(const double3 &u) { return double3{-u.x, -u.y, -u.z}; }
__host__ __device__  inline double4 operator-(const double4 &u) { return double4{-u.x, -u.y, -u.z, -u.w}; }

// VECTOR + VECTOR
__host__ __device__  inline int2     operator+(const int2    &u, const int2    &v) { return int2   {u.x+v.x, u.y+v.y}; }
__host__ __device__  inline int3     operator+(const int3    &u, const int3    &v) { return int3   {u.x+v.x, u.y+v.y, u.z+v.z}; }
__host__ __device__  inline int4     operator+(const int4    &u, const int4    &v) { return int4   {u.x+v.x, u.y+v.y, u.z+v.z, u.w+v.w}; }
__host__ __device__  inline float2   operator+(const float2  &u, const float2  &v) { return float2 {u.x+v.x, u.y+v.y}; }
__host__ __device__  inline float3   operator+(const float3  &u, const float3  &v) { return float3 {u.x+v.x, u.y+v.y, u.z+v.z}; }
__host__ __device__  inline float4   operator+(const float4  &u, const float4  &v) { return float4 {u.x+v.x, u.y+v.y, u.z+v.z, u.w+v.w}; }
__host__ __device__  inline double2  operator+(const double2 &u, const double2 &v) { return double2{u.x+v.x, u.y+v.y}; }
__host__ __device__  inline double3  operator+(const double3 &u, const double3 &v) { return double3{u.x+v.x, u.y+v.y, u.z+v.z}; }
__host__ __device__  inline double4  operator+(const double4 &u, const double4 &v) { return double4{u.x+v.x, u.y+v.y, u.z+v.z, u.w+v.w}; }
__host__ __device__  inline int2&    operator+=(int2    &u, const int2    &v)  { u = u + v; return u; }
__host__ __device__  inline int3&    operator+=(int3    &u, const int3    &v)  { u = u + v; return u; }
__host__ __device__  inline int4&    operator+=(int4    &u, const int4    &v)  { u = u + v; return u; }
__host__ __device__  inline float2&  operator+=(float2  &u, const float2  &v)  { u = u + v; return u; }
__host__ __device__  inline float3&  operator+=(float3  &u, const float3  &v)  { u = u + v; return u; }
__host__ __device__  inline float4&  operator+=(float4  &u, const float4  &v)  { u = u + v; return u; }
__host__ __device__  inline double2& operator+=(double2 &u, const double2 &v)  { u = u + v; return u; }
__host__ __device__  inline double3& operator+=(double3 &u, const double3 &v)  { u = u + v; return u; }
__host__ __device__  inline double4& operator+=(double4 &u, const double4 &v)  { u = u + v; return u; }
// VECTOR - VECTOR
__host__ __device__  inline int2     operator-(const int2    &u, const int2    &v) { return int2   {u.x-v.x, u.y-v.y}; }
__host__ __device__  inline int3     operator-(const int3    &u, const int3    &v) { return int3   {u.x-v.x, u.y-v.y, u.z-v.z}; }
__host__ __device__  inline int4     operator-(const int4    &u, const int4    &v) { return int4   {u.x-v.x, u.y-v.y, u.z-v.z, u.w-v.w}; }
__host__ __device__  inline float2   operator-(const float2  &u, const float2  &v) { return float2 {u.x-v.x, u.y-v.y}; }
__host__ __device__  inline float3   operator-(const float3  &u, const float3  &v) { return float3 {u.x-v.x, u.y-v.y, u.z-v.z}; }
__host__ __device__  inline float4   operator-(const float4  &u, const float4  &v) { return float4 {u.x-v.x, u.y-v.y, u.z-v.z, u.w-v.w}; }
__host__ __device__  inline double2  operator-(const double2 &u, const double2 &v) { return double2{u.x-v.x, u.y-v.y}; }
__host__ __device__  inline double3  operator-(const double3 &u, const double3 &v) { return double3{u.x-v.x, u.y-v.y, u.z-v.z}; }
__host__ __device__  inline double4  operator-(const double4 &u, const double4 &v) { return double4{u.x-v.x, u.y-v.y, u.z-v.z, u.w-v.w}; }
__host__ __device__  inline int2&    operator-=(int2    &u, const int2    &v)  { u = u - v; return u; }
__host__ __device__  inline int3&    operator-=(int3    &u, const int3    &v)  { u = u - v; return u; }
__host__ __device__  inline int4&    operator-=(int4    &u, const int4    &v)  { u = u - v; return u; }
__host__ __device__  inline float2&  operator-=(float2  &u, const float2  &v)  { u = u - v; return u; }
__host__ __device__  inline float3&  operator-=(float3  &u, const float3  &v)  { u = u - v; return u; }
__host__ __device__  inline float4&  operator-=(float4  &u, const float4  &v)  { u = u - v; return u; }
__host__ __device__  inline double2& operator-=(double2 &u, const double2 &v)  { u = u - v; return u; }
__host__ __device__  inline double3& operator-=(double3 &u, const double3 &v)  { u = u - v; return u; }
__host__ __device__  inline double4& operator-=(double4 &u, const double4 &v)  { u = u - v; return u; }
// VECTOR * VECTOR
__host__ __device__  inline int2     operator*(const int2    &u, const int2    &v) { return int2   {u.x*v.x, u.y*v.y}; }
__host__ __device__  inline int3     operator*(const int3    &u, const int3    &v) { return int3   {u.x*v.x, u.y*v.y, u.z*v.z}; }
__host__ __device__  inline int4     operator*(const int4    &u, const int4    &v) { return int4   {u.x*v.x, u.y*v.y, u.z*v.z, u.w*v.w}; }
__host__ __device__  inline float2   operator*(const float2  &u, const float2  &v) { return float2 {u.x*v.x, u.y*v.y}; }
__host__ __device__  inline float3   operator*(const float3  &u, const float3  &v) { return float3 {u.x*v.x, u.y*v.y, u.z*v.z}; }
__host__ __device__  inline float4   operator*(const float4  &u, const float4  &v) { return float4 {u.x*v.x, u.y*v.y, u.z*v.z, u.w*v.w}; }
__host__ __device__  inline double2  operator*(const double2 &u, const double2 &v) { return double2{u.x*v.x, u.y*v.y}; }
__host__ __device__  inline double3  operator*(const double3 &u, const double3 &v) { return double3{u.x*v.x, u.y*v.y, u.z*v.z}; }
__host__ __device__  inline double4  operator*(const double4 &u, const double4 &v) { return double4{u.x*v.x, u.y*v.y, u.z*v.z, u.w*v.w}; }
__host__ __device__  inline int2&    operator*=(int2    &u, const int2    &v)  { u = u * v; return u; }
__host__ __device__  inline int3&    operator*=(int3    &u, const int3    &v)  { u = u * v; return u; }
__host__ __device__  inline int4&    operator*=(int4    &u, const int4    &v)  { u = u * v; return u; }
__host__ __device__  inline float2&  operator*=(float2  &u, const float2  &v)  { u = u * v; return u; }
__host__ __device__  inline float3&  operator*=(float3  &u, const float3  &v)  { u = u * v; return u; }
__host__ __device__  inline float4&  operator*=(float4  &u, const float4  &v)  { u = u * v; return u; }
__host__ __device__  inline double2& operator*=(double2 &u, const double2 &v)  { u = u * v; return u; }
__host__ __device__  inline double3& operator*=(double3 &u, const double3 &v)  { u = u * v; return u; }
__host__ __device__  inline double4& operator*=(double4 &u, const double4 &v)  { u = u * v; return u; }
// VECTOR / VECTOR
__host__ __device__  inline int2     operator/(const int2    &u, const int2    &v) { return int2   {u.x/v.x, u.y/v.y}; }
__host__ __device__  inline int3     operator/(const int3    &u, const int3    &v) { return int3   {u.x/v.x, u.y/v.y, u.z/v.z}; }
__host__ __device__  inline int4     operator/(const int4    &u, const int4    &v) { return int4   {u.x/v.x, u.y/v.y, u.z/v.z, u.w/v.w}; }
__host__ __device__  inline float2   operator/(const float2  &u, const float2  &v) { return float2 {u.x/v.x, u.y/v.y}; }
__host__ __device__  inline float3   operator/(const float3  &u, const float3  &v) { return float3 {u.x/v.x, u.y/v.y, u.z/v.z}; }
__host__ __device__  inline float4   operator/(const float4  &u, const float4  &v) { return float4 {u.x/v.x, u.y/v.y, u.z/v.z, u.w/v.w}; }
__host__ __device__  inline double2  operator/(const double2 &u, const double2 &v) { return double2{u.x/v.x, u.y/v.y}; }
__host__ __device__  inline double3  operator/(const double3 &u, const double3 &v) { return double3{u.x/v.x, u.y/v.y, u.z/v.z}; }
__host__ __device__  inline double4  operator/(const double4 &u, const double4 &v) { return double4{u.x/v.x, u.y/v.y, u.z/v.z, u.w/v.w}; }
__host__ __device__  inline int2&    operator/=(int2    &u, const int2    &v)  { u = u / v; return u; }
__host__ __device__  inline int3&    operator/=(int3    &u, const int3    &v)  { u = u / v; return u; }
__host__ __device__  inline int4&    operator/=(int4    &u, const int4    &v)  { u = u / v; return u; }
__host__ __device__  inline float2&  operator/=(float2  &u, const float2  &v)  { u = u / v; return u; }
__host__ __device__  inline float3&  operator/=(float3  &u, const float3  &v)  { u = u / v; return u; }
__host__ __device__  inline float4&  operator/=(float4  &u, const float4  &v)  { u = u / v; return u; }
__host__ __device__  inline double2& operator/=(double2 &u, const double2 &v)  { u = u / v; return u; }
__host__ __device__  inline double3& operator/=(double3 &u, const double3 &v)  { u = u / v; return u; }
__host__ __device__  inline double4& operator/=(double4 &u, const double4 &v)  { u = u / v; return u; }

// VECTOR + SCALAR
__host__ __device__  inline int2     operator+(const int2    &u, const int    &s) { return int2   {u.x+s, u.y+s}; }
__host__ __device__  inline int3     operator+(const int3    &u, const int    &s) { return int3   {u.x+s, u.y+s, u.z+s}; }
__host__ __device__  inline int4     operator+(const int4    &u, const int    &s) { return int4   {u.x+s, u.y+s, u.z+s, u.w+s}; }
__host__ __device__  inline float2   operator+(const float2  &u, const float  &s) { return float2 {u.x+s, u.y+s}; }
__host__ __device__  inline float3   operator+(const float3  &u, const float  &s) { return float3 {u.x+s, u.y+s, u.z+s}; }
__host__ __device__  inline float4   operator+(const float4  &u, const float  &s) { return float4 {u.x+s, u.y+s, u.z+s, u.w+s}; }
__host__ __device__  inline double2  operator+(const double2 &u, const double &s) { return double2{u.x+s, u.y+s}; }
__host__ __device__  inline double3  operator+(const double3 &u, const double &s) { return double3{u.x+s, u.y+s, u.z+s}; }
__host__ __device__  inline double4  operator+(const double4 &u, const double &s) { return double4{u.x+s, u.y+s, u.z+s, u.w+s}; }
__host__ __device__  inline int2&    operator+=(int2    &u, int    s)  { u = u + s; return u; }
__host__ __device__  inline int3&    operator+=(int3    &u, int    s)  { u = u + s; return u; }
__host__ __device__  inline int4&    operator+=(int4    &u, int    s)  { u = u + s; return u; }
__host__ __device__  inline float2&  operator+=(float2  &u, float  s)  { u = u + s; return u; }
__host__ __device__  inline float3&  operator+=(float3  &u, float  s)  { u = u + s; return u; }
__host__ __device__  inline float4&  operator+=(float4  &u, float  s)  { u = u + s; return u; }
__host__ __device__  inline double2& operator+=(double2 &u, double s)  { u = u + s; return u; }
__host__ __device__  inline double3& operator+=(double3 &u, double s)  { u = u + s; return u; }
__host__ __device__  inline double4& operator+=(double4 &u, double s)  { u = u + s; return u; }
// VECTOR - SCALAR
__host__ __device__  inline int2     operator-(const int2    &u, const int    &s) { return int2   {u.x-s, u.y-s}; }
__host__ __device__  inline int3     operator-(const int3    &u, const int    &s) { return int3   {u.x-s, u.y-s, u.z-s}; }
__host__ __device__  inline int4     operator-(const int4    &u, const int    &s) { return int4   {u.x-s, u.y-s, u.z-s, u.w-s}; }
__host__ __device__  inline float2   operator-(const float2  &u, const float  &s) { return float2 {u.x-s, u.y-s}; }
__host__ __device__  inline float3   operator-(const float3  &u, const float  &s) { return float3 {u.x-s, u.y-s, u.z-s}; }
__host__ __device__  inline float4   operator-(const float4  &u, const float  &s) { return float4 {u.x-s, u.y-s, u.z-s, u.w-s}; }
__host__ __device__  inline double2  operator-(const double2 &u, const double &s) { return double2{u.x-s, u.y-s}; }
__host__ __device__  inline double3  operator-(const double3 &u, const double &s) { return double3{u.x-s, u.y-s, u.z-s}; }
__host__ __device__  inline double4  operator-(const double4 &u, const double &s) { return double4{u.x-s, u.y-s, u.z-s, u.w-s}; }
__host__ __device__  inline int2&    operator-=(int2    &u, int    s)  { u = u - s; return u; }
__host__ __device__  inline int3&    operator-=(int3    &u, int    s)  { u = u - s; return u; }
__host__ __device__  inline int4&    operator-=(int4    &u, int    s)  { u = u - s; return u; }
__host__ __device__  inline float2&  operator-=(float2  &u, float  s)  { u = u - s; return u; }
__host__ __device__  inline float3&  operator-=(float3  &u, float  s)  { u = u - s; return u; }
__host__ __device__  inline float4&  operator-=(float4  &u, float  s)  { u = u - s; return u; }
__host__ __device__  inline double2& operator-=(double2 &u, double s)  { u = u - s; return u; }
__host__ __device__  inline double3& operator-=(double3 &u, double s)  { u = u - s; return u; }
__host__ __device__  inline double4& operator-=(double4 &u, double s)  { u = u - s; return u; }
// VECTOR * SCALAR
__host__ __device__  inline int2     operator*(const int2    &u, const int    &s) { return int2   {u.x*s, u.y*s}; }
__host__ __device__  inline int3     operator*(const int3    &u, const int    &s) { return int3   {u.x*s, u.y*s, u.z*s}; }
__host__ __device__  inline int4     operator*(const int4    &u, const int    &s) { return int4   {u.x*s, u.y*s, u.z*s, u.w*s}; }
__host__ __device__  inline float2   operator*(const float2  &u, const float  &s) { return float2 {u.x*s, u.y*s}; }
__host__ __device__  inline float3   operator*(const float3  &u, const float  &s) { return float3 {u.x*s, u.y*s, u.z*s}; }
__host__ __device__  inline float4   operator*(const float4  &u, const float  &s) { return float4 {u.x*s, u.y*s, u.z*s, u.w*s}; }
__host__ __device__  inline double2  operator*(const double2 &u, const double &s) { return double2{u.x*s, u.y*s}; }
__host__ __device__  inline double3  operator*(const double3 &u, const double &s) { return double3{u.x*s, u.y*s, u.z*s}; }
__host__ __device__  inline double4  operator*(const double4 &u, const double &s) { return double4{u.x*s, u.y*s, u.z*s, u.w*s}; }
__host__ __device__  inline int2&    operator*=(int2    &u, int    s)  { u = u * s; return u; }
__host__ __device__  inline int3&    operator*=(int3    &u, int    s)  { u = u * s; return u; }
__host__ __device__  inline int4&    operator*=(int4    &u, int    s)  { u = u * s; return u; }
__host__ __device__  inline float2&  operator*=(float2  &u, float  s)  { u = u * s; return u; }
__host__ __device__  inline float3&  operator*=(float3  &u, float  s)  { u = u * s; return u; }
__host__ __device__  inline float4&  operator*=(float4  &u, float  s)  { u = u * s; return u; }
__host__ __device__  inline double2& operator*=(double2 &u, double s)  { u = u * s; return u; }
__host__ __device__  inline double3& operator*=(double3 &u, double s)  { u = u * s; return u; }
__host__ __device__  inline double4& operator*=(double4 &u, double s)  { u = u * s; return u; }
// VECTOR / SCALAR
__host__ __device__  inline int2     operator/(const int2    &u, const int    &s) { return int2   {u.x/s, u.y/s}; }
__host__ __device__  inline int3     operator/(const int3    &u, const int    &s) { return int3   {u.x/s, u.y/s, u.z/s}; }
__host__ __device__  inline int4     operator/(const int4    &u, const int    &s) { return int4   {u.x/s, u.y/s, u.z/s, u.w/s}; }
__host__ __device__  inline float2   operator/(const float2  &u, const float  &s) { return float2 {u.x/s, u.y/s}; }
__host__ __device__  inline float3   operator/(const float3  &u, const float  &s) { return float3 {u.x/s, u.y/s, u.z/s}; }
__host__ __device__  inline float4   operator/(const float4  &u, const float  &s) { return float4 {u.x/s, u.y/s, u.z/s, u.w/s}; }
__host__ __device__  inline double2  operator/(const double2 &u, const double &s) { return double2{u.x/s, u.y/s}; }
__host__ __device__  inline double3  operator/(const double3 &u, const double &s) { return double3{u.x/s, u.y/s, u.z/s}; }
__host__ __device__  inline double4  operator/(const double4 &u, const double &s) { return double4{u.x/s, u.y/s, u.z/s, u.w/s}; }
__host__ __device__  inline int2&    operator/=(int2    &u, int    s)  { u = u / s; return u; }
__host__ __device__  inline int3&    operator/=(int3    &u, int    s)  { u = u / s; return u; }
__host__ __device__  inline int4&    operator/=(int4    &u, int    s)  { u = u / s; return u; }
__host__ __device__  inline float2&  operator/=(float2  &u, float  s)  { u = u / s; return u; }
__host__ __device__  inline float3&  operator/=(float3  &u, float  s)  { u = u / s; return u; }
__host__ __device__  inline float4&  operator/=(float4  &u, float  s)  { u = u / s; return u; }
__host__ __device__  inline double2& operator/=(double2 &u, double s)  { u = u / s; return u; }
__host__ __device__  inline double3& operator/=(double3 &u, double s)  { u = u / s; return u; }
__host__ __device__  inline double4& operator/=(double4 &u, double s)  { u = u / s; return u; }

// SCALAR + VECTOR
__host__ __device__  inline int2     operator+(const int    &s, const int2    &u)  { return int2   {u.x+s, u.y+s}; }
__host__ __device__  inline int3     operator+(const int    &s, const int3    &u)  { return int3   {u.x+s, u.y+s, u.z+s}; }
__host__ __device__  inline int4     operator+(const int    &s, const int4    &u)  { return int4   {u.x+s, u.y+s, u.z+s, u.w+s}; }
__host__ __device__  inline float2   operator+(const float  &s, const float2  &u)  { return float2 {u.x+s, u.y+s}; }
__host__ __device__  inline float3   operator+(const float  &s, const float3  &u)  { return float3 {u.x+s, u.y+s, u.z+s}; }
__host__ __device__  inline float4   operator+(const float  &s, const float4  &u)  { return float4 {u.x+s, u.y+s, u.z+s, u.w+s}; }
__host__ __device__  inline double2  operator+(const double &s, const double2 &u)  { return double2{u.x+s, u.y+s}; }
__host__ __device__  inline double3  operator+(const double &s, const double3 &u)  { return double3{u.x+s, u.y+s, u.z+s}; }
__host__ __device__  inline double4  operator+(const double &s, const double4 &u)  { return double4{u.x+s, u.y+s, u.z+s, u.w+s}; }
// SCALAR - VECTOR
__host__ __device__  inline int2     operator-(const int    &s, const int2    &u)  { return int2   {u.x-s, u.y-s}; }
__host__ __device__  inline int3     operator-(const int    &s, const int3    &u)  { return int3   {u.x-s, u.y-s, u.z-s}; }
__host__ __device__  inline int4     operator-(const int    &s, const int4    &u)  { return int4   {u.x-s, u.y-s, u.z-s, u.w-s}; }
__host__ __device__  inline float2   operator-(const float  &s, const float2  &u)  { return float2 {u.x-s, u.y-s}; }
__host__ __device__  inline float3   operator-(const float  &s, const float3  &u)  { return float3 {u.x-s, u.y-s, u.z-s}; }
__host__ __device__  inline float4   operator-(const float  &s, const float4  &u)  { return float4 {u.x-s, u.y-s, u.z-s, u.w-s}; }
__host__ __device__  inline double2  operator-(const double &s, const double2 &u)  { return double2{u.x-s, u.y-s}; }
__host__ __device__  inline double3  operator-(const double &s, const double3 &u)  { return double3{u.x-s, u.y-s, u.z-s}; }
__host__ __device__  inline double4  operator-(const double &s, const double4 &u)  { return double4{u.x-s, u.y-s, u.z-s, u.w-s}; }
// SCALAR * VECTOR
__host__ __device__  inline int2     operator*(const int    &s, const int2    &u)  { return int2   {u.x*s, u.y*s}; }
__host__ __device__  inline int3     operator*(const int    &s, const int3    &u)  { return int3   {u.x*s, u.y*s, u.z*s}; }
__host__ __device__  inline int4     operator*(const int    &s, const int4    &u)  { return int4   {u.x*s, u.y*s, u.z*s, u.w*s}; }
__host__ __device__  inline float2   operator*(const float  &s, const float2  &u)  { return float2 {u.x*s, u.y*s}; }
__host__ __device__  inline float3   operator*(const float  &s, const float3  &u)  { return float3 {u.x*s, u.y*s, u.z*s}; }
__host__ __device__  inline float4   operator*(const float  &s, const float4  &u)  { return float4 {u.x*s, u.y*s, u.z*s, u.w*s}; }
__host__ __device__  inline double2  operator*(const double &s, const double2 &u)  { return double2{u.x*s, u.y*s}; }
__host__ __device__  inline double3  operator*(const double &s, const double3 &u)  { return double3{u.x*s, u.y*s, u.z*s}; }
__host__ __device__  inline double4  operator*(const double &s, const double4 &u)  { return double4{u.x*s, u.y*s, u.z*s, u.w*s}; }
// SCALAR / VECTOR
__host__ __device__  inline int2     operator/(const int    &s, const int2    &u)  { return int2   {u.x/s, u.y/s}; }
__host__ __device__  inline int3     operator/(const int    &s, const int3    &u)  { return int3   {u.x/s, u.y/s, u.z/s}; }
__host__ __device__  inline int4     operator/(const int    &s, const int4    &u)  { return int4   {u.x/s, u.y/s, u.z/s, u.w/s}; }
__host__ __device__  inline float2   operator/(const float  &s, const float2  &u)  { return float2 {u.x/s, u.y/s}; }
__host__ __device__  inline float3   operator/(const float  &s, const float3  &u)  { return float3 {u.x/s, u.y/s, u.z/s}; }
__host__ __device__  inline float4   operator/(const float  &s, const float4  &u)  { return float4 {u.x/s, u.y/s, u.z/s, u.w/s}; }
__host__ __device__  inline double2  operator/(const double &s, const double2 &u)  { return double2{u.x/s, u.y/s}; }
__host__ __device__  inline double3  operator/(const double &s, const double3 &u)  { return double3{u.x/s, u.y/s, u.z/s}; }
__host__ __device__  inline double4  operator/(const double &s, const double4 &u)  { return double4{u.x/s, u.y/s, u.z/s, u.w/s}; }


// VECTOR LENGTH^2
__host__ __device__  inline float   length2(const int     &v) { return float(v*v); }
__host__ __device__  inline float   length2(const int2    &v) { return (float(v.x*v.x + v.y*v.y)); }
__host__ __device__  inline float   length2(const int3    &v) { return (float(v.x*v.x + v.y*v.y + v.z*v.z)); }
__host__ __device__  inline float   length2(const int4    &v) { return (float(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w)); }
__host__ __device__  inline float   length2(const float   &v) { return v*v; }
__host__ __device__  inline float   length2(const float2  &v) { return (v.x*v.x + v.y*v.y); }
__host__ __device__  inline float   length2(const float3  &v) { return (v.x*v.x + v.y*v.y + v.z*v.z); }
__host__ __device__  inline float   length2(const float4  &v) { return (v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w); }
__host__ __device__  inline double  length2(const double  &v) { return v*v; }
__host__ __device__  inline double  length2(const double2 &v) { return (v.x*v.x + v.y*v.y); }
__host__ __device__  inline double  length2(const double3 &v) { return (v.x*v.x + v.y*v.y + v.z*v.z); }
__host__ __device__  inline double  length2(const double4 &v) { return (v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w); }
// VECTOR LENGTH
__host__ __device__  inline float   length(const int     &v) { return abs(float(v)); }
__host__ __device__  inline float   length(const int2    &v) { return sqrt(float(v.x*v.x + v.y*v.y)); }
__host__ __device__  inline float   length(const int3    &v) { return sqrt(float(v.x*v.x + v.y*v.y + v.z*v.z)); }
__host__ __device__  inline float   length(const int4    &v) { return sqrt(float(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w)); }
__host__ __device__  inline float   length(const float   &v) { return abs(v); }
__host__ __device__  inline float   length(const float2  &v) { return sqrt(v.x*v.x + v.y*v.y); }
__host__ __device__  inline float   length(const float3  &v) { return sqrt(v.x*v.x + v.y*v.y + v.z*v.z); }
__host__ __device__  inline float   length(const float4  &v) { return sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w); }
__host__ __device__  inline double  length(const double  &v) { return abs(v); }
__host__ __device__  inline double  length(const double2 &v) { return sqrt(v.x*v.x + v.y*v.y); }
__host__ __device__  inline double  length(const double3 &v) { return sqrt(v.x*v.x + v.y*v.y + v.z*v.z); }
__host__ __device__  inline double  length(const double4 &v) { return sqrt(v.x*v.x + v.y*v.y + v.z*v.z + v.w*v.w); }

// NORMALIZATION
__host__ __device__  inline float2   normalize(const float2  &v) { return v / length(v); }
__host__ __device__  inline float3   normalize(const float3  &v) { return v / length(v); }
__host__ __device__  inline float4   normalize(const float4  &v) { return v / length(v); }
__host__ __device__  inline double2  normalize(const double2 &v) { return v / length(v); }
__host__ __device__  inline double3  normalize(const double3 &v) { return v / length(v); }
__host__ __device__  inline double4  normalize(const double4 &v) { return v / length(v); }

// DOT PRODUCT
__host__ __device__  inline float   dot(const float2  &u, const float2  &v) { return (u.x*v.x + u.y*v.y); }
__host__ __device__  inline float   dot(const float3  &u, const float3  &v) { return (u.x*v.x + u.y*v.y + u.z*v.z); }
__host__ __device__  inline float   dot(const float4  &u, const float4  &v) { return (u.x*v.x + u.y*v.y + u.z*v.z + u.w*v.w); }
__host__ __device__  inline double  dot(const double2 &u, const double2 &v) { return (u.x*v.x + u.y*v.y); }
__host__ __device__  inline double  dot(const double3 &u, const double3 &v) { return (u.x*v.x + u.y*v.y + u.z*v.z); }
__host__ __device__  inline double  dot(const double4 &u, const double4 &v) { return (u.x*v.x + u.y*v.y + u.z*v.z + u.w*v.w); }

inline __host__ __device__ float fminf(float a, float b) { return a < b ? a : b; }
inline __host__ __device__ float fmaxf(float a, float b) { return a > b ? a : b; }
inline __host__ __device__ int   imax (int a,   int   b) { return a > b ? a : b; }
inline __host__ __device__ int   imin (int a,   int   b) { return a < b ? a : b; }


// MAX
__host__ __device__  inline int2    max(const int2    &u, const int2    &v) { return (length2(u) > length2(v) ? u : v); }
__host__ __device__  inline int3    max(const int3    &u, const int3    &v) { return (length2(u) > length2(v) ? u : v); }
__host__ __device__  inline int4    max(const int4    &u, const int4    &v) { return (length2(u) > length2(v) ? u : v); }
__host__ __device__  inline float2  max(const float2  &u, const float2  &v) { return (length2(u) > length2(v) ? u : v); }
__host__ __device__  inline float3  max(const float3  &u, const float3  &v) { return (length2(u) > length2(v) ? u : v); }
__host__ __device__  inline float4  max(const float4  &u, const float4  &v) { return (length2(u) > length2(v) ? u : v); }
__host__ __device__  inline double2 max(const double2 &u, const double2 &v) { return (length2(u) > length2(v) ? u : v); }
__host__ __device__  inline double3 max(const double3 &u, const double3 &v) { return (length2(u) > length2(v) ? u : v); }
__host__ __device__  inline double4 max(const double4 &u, const double4 &v) { return (length2(u) > length2(v) ? u : v); } 

// ABS
__host__ __device__  inline int2     abs(const int2    &v)  { return int2   {v.x, v.y}; }
__host__ __device__  inline int3     abs(const int3    &v)  { return int3   {v.x, v.y, v.z}; }
__host__ __device__  inline int4     abs(const int4    &v)  { return int4   {v.x, v.y, v.z, v.w}; }
__host__ __device__  inline float2   abs(const float2  &v)  { return float2 {v.x, v.y}; }
__host__ __device__  inline float3   abs(const float3  &v)  { return float3 {v.x, v.y, v.z}; }
__host__ __device__  inline float4   abs(const float4  &v)  { return float4 {v.x, v.y, v.z, v.w}; }
__host__ __device__  inline double2  abs(const double2 &v)  { return double2{v.x, v.y}; }
__host__ __device__  inline double3  abs(const double3 &v)  { return double3{v.x, v.y, v.z}; }
__host__ __device__  inline double4  abs(const double4 &v)  { return double4{v.x, v.y, v.z, v.w}; }

// LOG
__host__ __device__  inline int      log(const int     &v)  { return (int)log((float)v);}
__host__ __device__  inline int2     log(const int2    &v)  { return int2{(int)log((float)v.x),(int)log((float)v.y)};}
__host__ __device__  inline int3     log(const int3    &v)  { return int3{(int)log((float)v.x),(int)log((float)v.y),(int)log((float)v.z)};}
__host__ __device__  inline int4     log(const int4    &v)  { return int4{(int)log((float)v.x),(int)log((float)v.y),(int)log((float)v.z),(int)log((float)v.w)};}
__host__ __device__  inline float2   log(const float2  &v)  { return float2 {(float)log(v.x), (float)log(v.y)}; }
__host__ __device__  inline float3   log(const float3  &v)  { return float3 {(float)log(v.x), (float)log(v.y), (float)log(v.z)}; }
__host__ __device__  inline float4   log(const float4  &v)  { return float4 {(float)log(v.x), (float)log(v.y), (float)log(v.z), (float)log(v.w)}; }
__host__ __device__  inline double2  log(const double2 &v)  { return double2{log((float)v.x), log((float)v.y)}; }
__host__ __device__  inline double3  log(const double3 &v)  { return double3{log((float)v.x), log((float)v.y), log((float)v.z)}; }
__host__ __device__  inline double4  log(const double4 &v)  { return double4{log((float)v.x), log((float)v.y), log((float)v.z), log((float)v.w)}; }

// NEGLOG
__host__ __device__  inline int      neglog(const int     &v)  { return (int)log((float)abs(v))*(v < 0 ? -1 : 1); }
__host__ __device__  inline int2     neglog(const int2    &v)  { return int2{  (int)log((float)abs(v.x))*(v.x < 0 ? -1 : 1),
                                                                               (int)log((float)abs(v.y))*(v.y < 0 ? -1 : 1) }; }
__host__ __device__  inline int3     neglog(const int3    &v)  { return int3{  (int)log((float)abs(v.x))*(v.x < 0 ? -1 : 1),
                                                                               (int)log((float)abs(v.y))*(v.y < 0 ? -1 : 1),
                                                                               (int)log((float)abs(v.z))*(v.z < 0 ? -1 : 1) };}
__host__ __device__  inline int4     neglog(const int4    &v)  { return int4{  (int)log((float)v.x)*(v.x < 0 ? -1 : 1),
                                                                               (int)log((float)v.y)*(v.x < 0 ? -1 : 1),
                                                                               (int)log((float)v.z)*(v.x < 0 ? -1 : 1),
                                                                               (int)log((float)v.w)*(v.x < 0 ? -1 : 1) };}
__host__ __device__  inline float2   neglog(const float2  &v)  { return float2 {  (float)log(v.x)*(v.x < 0 ? -1 : 1), (float)log(v.y)*(v.y < 0 ? -1 : 1)}; }
__host__ __device__  inline float3   neglog(const float3  &v)  { return float3 {  (float)log(v.x)*(v.x < 0 ? -1 : 1), (float)log(v.y)*(v.y < 0 ? -1 : 1),
                                                                                  (float)log(v.z)*(v.z < 0 ? -1 : 1)}; }
__host__ __device__  inline float4   neglog(const float4  &v)  { return float4 {  (float)log(v.x)*(v.x < 0 ? -1 : 1), (float)log(v.y)*(v.y < 0 ? -1 : 1),
                                                                                  (float)log(v.z)*(v.z < 0 ? -1 : 1), (float)log(v.w)*(v.w < 0 ? -1 : 1)}; }
__host__ __device__  inline double2  neglog(const double2 &v)  { return double2{  (double)log((float)v.x)*(v.x < 0 ? -1 : 1),
                                                                                  (double)log((float)v.y)*(v.y < 0 ? -1 : 1) }; }
__host__ __device__  inline double3  neglog(const double3 &v)  { return double3{  (double)log((float)v.x)*(v.x < 0 ? -1 : 1),
                                                                                  (double)log((float)v.y)*(v.y < 0 ? -1 : 1),
                                                                                  (double)log((float)v.z)*(v.z < 0 ? -1 : 1) }; }
__host__ __device__  inline double4  neglog(const double4 &v)  { return double4{  (double)log((float)v.x)*(v.x < 0 ? -1 : 1),
                                                                                  (double)log((float)v.y)*(v.y < 0 ? -1 : 1),
                                                                                  (double)log((float)v.z)*(v.z < 0 ? -1 : 1),
                                                                                  (double)log((float)v.w)*(v.w < 0 ? -1 : 1) }; }

// EXP
__host__ __device__  inline int      exp(const int     &v)  { return (int)exp((float)v); }
__host__ __device__  inline int2     exp(const int2    &v)  { return int2{(int)exp((float)v.x),(int)exp((float)v.y)};}
__host__ __device__  inline int3     exp(const int3    &v)  { return int3{(int)exp((float)v.x),(int)exp((float)v.y),(int)exp((float)v.z)};}
__host__ __device__  inline int4     exp(const int4    &v)  { return int4{(int)exp((float)v.x),(int)exp((float)v.y),(int)exp((float)v.z),(int)exp((float)v.w)};}
__host__ __device__  inline float2   exp(const float2  &v)  { return float2 {(float) exp(v.x), (float)exp(v.y)}; }
__host__ __device__  inline float3   exp(const float3  &v)  { return float3 {(float) exp(v.x), (float)exp(v.y), (float)exp(v.z)}; }
__host__ __device__  inline float4   exp(const float4  &v)  { return float4 {(float) exp(v.x), (float)exp(v.y), (float)exp(v.z), (float)exp(v.w)}; }
__host__ __device__  inline double2  exp(const double2 &v)  { return double2{  (double)exp((float)v.x), (double)exp((float)v.y) }; }
__host__ __device__  inline double3  exp(const double3 &v)  { return double3{  (double)exp((float)v.x), (double)exp((float)v.y),
                                                                               (double)exp((float)v.z)}; }
__host__ __device__  inline double4  exp(const double4 &v)  { return double4{  (double)exp((float)v.x), (double)exp((float)v.y),
                                                                               (double)exp((float)v.z), (double)exp((float)v.w) }; }



////////////////////////////////////////////////////////////////////////////////
// array data
////////////////////////////////////////////////////////////////////////////////

__host__ __device__  inline const int*     arr(const int     &v)  { return (int   *)&v;   }
__host__ __device__  inline const int*     arr(const int2    &v)  { return (int   *)&v.x; }
__host__ __device__  inline const int*     arr(const int3    &v)  { return (int   *)&v.x; }
__host__ __device__  inline const int*     arr(const int4    &v)  { return (int   *)&v.x; }
__host__ __device__  inline const float*   arr(const float   &v)  { return (float *)&v;   }
__host__ __device__  inline const float*   arr(const float2  &v)  { return (float *)&v.x; }
__host__ __device__  inline const float*   arr(const float3  &v)  { return (float *)&v.x; }
__host__ __device__  inline const float*   arr(const float4  &v)  { return (float *)&v.x; }
__host__ __device__  inline const double*  arr(const double  &v)  { return (double*)&v;   }
__host__ __device__  inline const double*  arr(const double2 &v)  { return (double*)&v.x; }
__host__ __device__  inline const double*  arr(const double3 &v)  { return (double*)&v.x; }
__host__ __device__  inline const double*  arr(const double4 &v)  { return (double*)&v.x; }



////////////////////////////////////////////////////////////////////////////////
// constructors
////////////////////////////////////////////////////////////////////////////////


inline __host__ __device__ float2 make_float2(float s)
{ return make_float2(s, s); }
inline __host__ __device__ float2 make_float2(float3 a)
{ return make_float2(a.x, a.y); }
inline __host__ __device__ float2 make_float2(int2 a)
{ return make_float2(float(a.x), float(a.y)); }
inline __host__ __device__ float2 make_float2(uint2 a)
{ return make_float2(float(a.x), float(a.y)); }

inline __host__ __device__ double2 make_double2(double s)
{ return make_double2(s, s); }
inline __host__ __device__ double2 make_double2(double3 a)
{ return make_double2(a.x, a.y); }
inline __host__ __device__ double2 make_double2(int2 a)
{ return make_double2(double(a.x), double(a.y)); }
inline __host__ __device__ double2 make_double2(uint2 a)
{ return make_double2(double(a.x), double(a.y)); }

inline __host__ __device__ int2 make_int2(int s)
{ return make_int2(s, s); }
inline __host__ __device__ int2 make_int2(int3 a)
{ return make_int2(a.x, a.y); }
inline __host__ __device__ int2 make_int2(uint2 a)
{ return make_int2(int(a.x), int(a.y)); }
inline __host__ __device__ int2 make_int2(float2 a)
{ return make_int2(int(a.x), int(a.y)); }

inline __host__ __device__ uint2 make_uint2(uint s)
{ return make_uint2(s, s); }
inline __host__ __device__ uint2 make_uint2(uint3 a)
{ return make_uint2(a.x, a.y); }
inline __host__ __device__ uint2 make_uint2(int2 a)
{ return make_uint2(uint(a.x), uint(a.y)); }

inline __host__ __device__ float3 make_float3(float s)
{ return make_float3(s, s, s); }
inline __host__ __device__ float3 make_float3(float2 a)
{ return make_float3(a.x, a.y, 0.0f); }
inline __host__ __device__ float3 make_float3(float2 a, float s)
{ return make_float3(a.x, a.y, s); }
inline __host__ __device__ float3 make_float3(float4 a)
{ return make_float3(a.x, a.y, a.z); }
inline __host__ __device__ float3 make_float3(int3 a)
{ return make_float3(float(a.x), float(a.y), float(a.z)); }
inline __host__ __device__ float3 make_float3(uint3 a)
{ return make_float3(float(a.x), float(a.y), float(a.z)); }

inline __host__ __device__ double3 make_double3(double s)
{ return make_double3(s, s, s); }
inline __host__ __device__ double3 make_double3(double2 a)
{ return make_double3(a.x, a.y, 0.0); }
inline __host__ __device__ double3 make_double3(double2 a, double s)
{ return make_double3(a.x, a.y, s); }
inline __host__ __device__ double3 make_double3(double4 a)
{ return make_double3(a.x, a.y, a.z); }
inline __host__ __device__ double3 make_double3(int3 a)
{ return make_double3(double(a.x), double(a.y), double(a.z)); }
inline __host__ __device__ double3 make_double3(uint3 a)
{ return make_double3(double(a.x), double(a.y), double(a.z)); }

inline __host__ __device__ int3 make_int3(int s)
{ return make_int3(s, s, s); }
inline __host__ __device__ int3 make_int3(int2 a)
{ return make_int3(a.x, a.y, 0); }
inline __host__ __device__ int3 make_int3(int2 a, int s)
{ return make_int3(a.x, a.y, s); }
inline __host__ __device__ int3 make_int3(uint3 a)
{ return make_int3(int(a.x), int(a.y), int(a.z)); }
inline __host__ __device__ int3 make_int3(float3 a)
{ return make_int3(int(a.x), int(a.y), int(a.z)); }

inline __host__ __device__ uint3 make_uint3(uint s)
{ return make_uint3(s, s, s); }
inline __host__ __device__ uint3 make_uint3(uint2 a)
{ return make_uint3(a.x, a.y, 0); }
inline __host__ __device__ uint3 make_uint3(uint2 a, uint s)
{ return make_uint3(a.x, a.y, s); }
inline __host__ __device__ uint3 make_uint3(uint4 a)
{ return make_uint3(a.x, a.y, a.z); }
inline __host__ __device__ uint3 make_uint3(int3 a)
{ return make_uint3(uint(a.x), uint(a.y), uint(a.z)); }

inline __host__ __device__ float4 make_float4(float s)
{ return make_float4(s, s, s, s); }
inline __host__ __device__ float4 make_float4(float3 a)
{ return make_float4(a.x, a.y, a.z, 0.0f); }
inline __host__ __device__ float4 make_float4(float3 a, float w)
{ return make_float4(a.x, a.y, a.z, w); }
inline __host__ __device__ float4 make_float4(int4 a)
{ return make_float4(float(a.x), float(a.y), float(a.z), float(a.w)); }
inline __host__ __device__ float4 make_float4(uint4 a)
{ return make_float4(float(a.x), float(a.y), float(a.z), float(a.w)); }

inline __host__ __device__ double4 make_double4(double s)
{ return make_double4(s, s, s, s); }
inline __host__ __device__ double4 make_double4(double3 a)
{ return make_double4(a.x, a.y, a.z, 0.0); }
inline __host__ __device__ double4 make_double4(double3 a, double w)
{ return make_double4(a.x, a.y, a.z, w); }
inline __host__ __device__ double4 make_double4(int4 a)
{ return make_double4(double(a.x), double(a.y), double(a.z), double(a.w)); }
inline __host__ __device__ double4 make_double4(uint4 a)
{ return make_double4(double(a.x), double(a.y), double(a.z), double(a.w)); }

inline __host__ __device__ int4 make_int4(int s)
{ return make_int4(s, s, s, s); }
inline __host__ __device__ int4 make_int4(int3 a)
{ return make_int4(a.x, a.y, a.z, 0); }
inline __host__ __device__ int4 make_int4(int3 a, int w)
{ return make_int4(a.x, a.y, a.z, w); }
inline __host__ __device__ int4 make_int4(uint4 a)
{ return make_int4(int(a.x), int(a.y), int(a.z), int(a.w)); }
inline __host__ __device__ int4 make_int4(float4 a)
{ return make_int4(int(a.x), int(a.y), int(a.z), int(a.w)); }


inline __host__ __device__ uint4 make_uint4(uint s)
{ return make_uint4(s, s, s, s); }
inline __host__ __device__ uint4 make_uint4(uint3 a)
{ return make_uint4(a.x, a.y, a.z, 0); }
inline __host__ __device__ uint4 make_uint4(uint3 a, uint w)
{ return make_uint4(a.x, a.y, a.z, w); }
inline __host__ __device__ uint4 make_uint4(int4 a)
{ return make_uint4(uint(a.x), uint(a.y), uint(a.z), uint(a.w)); }


////////////////////////////////////////////////////////////////////////////////
// clamp
// - clamp the value v to be in the range [a, b]
////////////////////////////////////////////////////////////////////////////////

inline __device__ __host__ float clamp(float f, float a, float b) { return fmaxf(a, fminf(f, b)); }
inline __device__ __host__ int   clamp(int   f, int   a, int   b) { return  imax(a,  imin(f, b)); }
inline __device__ __host__ uint  clamp(uint  f, uint  a, uint  b) { return  imax(a,  imin(f, b)); }

inline __device__ __host__ float2 clamp(float2 v, float a, float b)
{ return make_float2(clamp(v.x, a, b), clamp(v.y, a, b)); }
inline __device__ __host__ float2 clamp(float2 v, float2 a, float2 b)
{ return make_float2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y)); }
inline __device__ __host__ float3 clamp(float3 v, float a, float b)
{ return make_float3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b)); }
inline __device__ __host__ float3 clamp(float3 v, float3 a, float3 b)
{ return make_float3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z)); }
inline __device__ __host__ float4 clamp(float4 v, float a, float b)
{ return make_float4(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b), clamp(v.w, a, b)); }
inline __device__ __host__ float4 clamp(float4 v, float4 a, float4 b)
{ return make_float4(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w)); }

inline __device__ __host__ int2 clamp(int2 v, int a, int b)   { return make_int2(clamp(v.x, a, b), clamp(v.y, a, b)); }
inline __device__ __host__ int2 clamp(int2 v, int2 a, int2 b) { return make_int2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y)); }
inline __device__ __host__ int3 clamp(int3 v, int a, int b)   { return make_int3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b)); }
inline __device__ __host__ int3 clamp(int3 v, int3 a, int3 b) { return make_int3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z)); }
inline __device__ __host__ int4 clamp(int4 v, int a, int b)   { return make_int4(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b), clamp(v.w, a, b)); }
inline __device__ __host__ int4 clamp(int4 v, int4 a, int4 b) { return make_int4(clamp(v.x, a.x,b.x), clamp(v.y, a.y, b.y),
                                                                                 clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w)); }

inline __device__ __host__ uint2 clamp(uint2 v, uint a, uint b)   { return make_uint2(clamp(v.x, a, b), clamp(v.y, a, b)); }
inline __device__ __host__ uint2 clamp(uint2 v, uint2 a, uint2 b) { return make_uint2(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y)); }
inline __device__ __host__ uint3 clamp(uint3 v, uint a, uint b)   { return make_uint3(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b)); }
inline __device__ __host__ uint3 clamp(uint3 v, uint3 a, uint3 b) { return make_uint3(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y), clamp(v.z, a.z, b.z)); }
inline __device__ __host__ uint4 clamp(uint4 v, uint a, uint b)   { return make_uint4(clamp(v.x, a, b), clamp(v.y, a, b), clamp(v.z, a, b), clamp(v.w, a, b)); }
inline __device__ __host__ uint4 clamp(uint4 v, uint4 a, uint4 b) { return make_uint4(clamp(v.x, a.x, b.x), clamp(v.y, a.y, b.y),
                                                                                      clamp(v.z, a.z, b.z), clamp(v.w, a.w, b.w)); }

////////////////////////////////////////////////////////////////////////////////
// smoothstep
// - returns 0 if x < a
// - returns 1 if x > b
// - otherwise returns smooth interpolation between 0 and 1 based on x
////////////////////////////////////////////////////////////////////////////////

inline __device__ __host__ float smoothstep(float a, float b, float x)
{ float y = clamp((x - a) / (b - a), 0.0f, 1.0f); return (y*y*(3.0f - (2.0f*y))); }
inline __device__ __host__ float2 smoothstep(float2 a, float2 b, float2 x)
{ float2 y = clamp((x - a) / (b - a), 0.0f, 1.0f); return (y*y*(make_float2(3.0f) - (make_float2(2.0f)*y))); }
inline __device__ __host__ float3 smoothstep(float3 a, float3 b, float3 x)
{ float3 y = clamp((x - a) / (b - a), 0.0f, 1.0f); return (y*y*(make_float3(3.0f) - (make_float3(2.0f)*y))); }
inline __device__ __host__ float4 smoothstep(float4 a, float4 b, float4 x)
{ float4 y = clamp((x - a) / (b - a), 0.0f, 1.0f); return (y*y*(make_float4(3.0f) - (make_float4(2.0f)*y))); }




// cross product
inline __device__ float3  cross(const float3  &a, const float3  &b)
{ return float3 {a.y, a.z, a.x} * float3 {b.z, b.x, b.y} - float3 {a.z, a.x, a.y} * float3 {b.y, b.z, b.x}; }
inline __device__ double3 cross(const double3 &a, const double3 &b)
{ return double3{a.y, a.z, a.x} * double3{b.z, b.x, b.y} - double3{a.z, a.x, a.y} * double3{b.y, b.z, b.x}; }



#endif // ENABLE_CUDA

#endif // CUDA_VECTOR_OPERATORS_H
