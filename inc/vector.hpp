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

// Vector Template Base
template<typename T, int N>
struct Vector
{
  std::array<T, N> data;

  Vector()
    : data{{0}}
  { }
  Vector(const Vector<T, N> &other)
    : Vector(other.data)
  { }
  Vector(const std::array<T, N> &data_)
    : Vector()
  {
    for(int i = 0; i < N; i++)
      { data[i] = data_[i]; }
  }
  Vector(const std::string &str) { fromString(str); }

  std::string toString() const            { std::ostringstream ss; ss << (*this); return ss.str(); }
  void fromString(const std::string &str) { std::istringstream ss(str); ss >> (*this); }
  
  bool operator==(const Vector<T, N> &other) const
  {
    for(int i = 0; i < N; i++)
        if(data[i] != other.data[i])
          { return false; }
    return true;
  }
  bool operator!=(const Vector<T, N> &other) const
  { return !(*this == other); }

  Vector<T, N>& operator+=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++)
      { data[i] += other.data[i]; }
    return *this;
  }
  Vector<T, N>& operator-=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++)
      { data[i] -= other.data[i]; }
    return *this;
  }

  void ceil()
  { for(auto &d : data) { d = std::ceil(d); } }
  Vector<T, N> getCeil() const
  {
    Vector<T, N> v = *this;
    v.ceil();
    return v;
  }
  void floor()
  { for(auto &d : data) { d = std::floor(d); } }
  Vector<T, N> getFloor() const
  {
    Vector<T, N> v = *this;
    v.floor();
    return v;
  }

  T length2() const
  {
    T sqsum = T();
    for(auto d : data)
      { sqsum += d*d; }
    return sqsum;
  }
  T length() const
  { return sqrt(length2()); }
  Vector<T, N> normalized() const
  {
    std::array<T, N> nData = data;
    T len = length();
    for(auto &d : nData)
      { d /= len; }
    return Vector<T, N>(nData);
  } 
};


//Shorthand typedefs
template<typename T>
using Vec1 = Vector<T, 1>;
template<typename T>
using Vec2 = Vector<T, 2>;
template<typename T>
using Vec3 = Vector<T, 3>;
template<typename T>
using Vec4 = Vector<T, 4>;

template<int N>
using Vecf = Vector<float, N>;
template<int N>
using Vecd = Vector<double, N>;

typedef Vector<int, 1> Vec1i;
typedef Vector<int, 2> Vec2i;
typedef Vector<int, 3> Vec3i;
typedef Vector<int, 4> Vec4i;
typedef Vector<float, 1> Vec1f;
typedef Vector<float, 2> Vec2f;
typedef Vector<float, 3> Vec3f;
typedef Vector<float, 4> Vec4f;
typedef Vector<double, 1> Vec1d;
typedef Vector<double, 2> Vec2d;
typedef Vector<double, 3> Vec3d;
typedef Vector<double, 4> Vec4d;
typedef Vector<long double, 1> Vec1l;
typedef Vector<long double, 2> Vec2l;
typedef Vector<long double, 3> Vec3l;
typedef Vector<long double, 4> Vec4l;

template<typename T, int N>
std::ostream& operator<<(std::ostream &os, const Vector<T, N> &v)
{
  os << "<";
  for(int i = 0; i < N; i++)
    {
      os << v.data[i];
      if(i < N-1)
        { os << ", "; }
    }
  os << ">";
  return os;
}
template<typename T, int N>
std::istream& operator>>(std::istream &is, Vector<T, N> &v)
{
  is.ignore(1,'<');
  for(int i = 0; i < N; i++)
    { is >> v.data[i]; is.ignore(1,','); }
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
  Vector(const std::string &str)     { fromString(str); }

  T& operator[](int dim)             { return data[dim]; }
  const T& operator[](int dim) const { return data[dim]; }
  
  std::string toString() const            { std::ostringstream ss; ss << (*this); return ss.str(); }
  void fromString(const std::string &str) { std::istringstream ss(str); ss >> (*this); }

  Vector<T, 2>& operator=(const Vector<T, 2> &other)
  {
    for(int i = 0; i < N; i++)
      { data[i] = other.data[i]; }
    return *this;
  }
  bool operator==(const Vector<T, N> &other) const
  {
    for(int i = 0; i < N; i++)
        if(data[i] != other.data[i])
          { return false; }
    return true;
  }
  bool operator!=(const Vector<T, N> &other) const
  { return !(*this == other); }
  
  
  Vector<T, N>& operator+=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++)
      { data[i] += other.data[i]; }
    return *this;
  }
  Vector<T, N>& operator-=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++)
      { data[i] -= other.data[i]; }
    return *this;
  }
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
    for(int i = 0; i < N; i++)
      { result.data[i] -= other.data[i]; }
    return result;
  }

  Vector<T, N>& operator*=(T scalar)
  {
    for(int i = 0; i < N; i++)
      { data[i] *= scalar; }
    return *this;
  }
  Vector<T, N>& operator/=(T scalar)
  {
    for(int i = 0; i < N; i++)
      { data[i] /= scalar; }
    return *this;
  }
  Vector<T, N> operator*(T scalar) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++)
      { result.data[i] *= scalar; }
    return result;
  }
  Vector<T, N> operator/(T scalar) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++)
      { result.data[i] /= scalar; }
    return result;
  }

  Vector<T, N>& operator*=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++)
      { data[i] *= other.data[i]; }
    return *this;
  }
  Vector<T, N>& operator/=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++)
      { data[i] /= other.data[i]; }
    return *this;
  }
  Vector<T, N> operator*(const Vector<T, N> &other) const
  {
    Vector<T, N> result(data);
    for(int i = 0; i < N; i++)
      { result.data[i] *= other.data[i]; }
    return result;
  }
  Vector<T, N> operator/(const Vector<T, N> &other) const
  {
    Vector<T, N> result(data);
    for(int i = 0; i < N; i++)
      { result.data[i] /= other.data[i]; }
    return result;
  }
  
  void ceil()
  { for(auto &d : data) { d = std::ceil(d); } }
  Vector<T, N> getCeil() const
  {
    Vector<T, N> v = *this;
    v.ceil();
    return v;
  }
  void floor()
  { for(auto &d : data) { d = std::floor(d); } }
  Vector<T, N> getFloor() const
  {
    Vector<T, N> v = *this;
    v.floor();
    return v;
  }
  
  T length2() const
  {
    T sqsum = T();
    for(auto d : data)
      { sqsum += d*d; }
    return sqsum;
  }
  T length() const
  { return sqrt(length2()); }
  Vector<T, N> normalized() const
  {
    std::array<T, N> nData = data;
    T len = length();
    for(auto &d : nData)
      { d /= len; }
    return Vector<T, N>(nData);
  }
};
  
template<typename T>
struct Vector<T, 3>
{
  static constexpr int N = 3;
  union
  {
    std::array<T, N> data;
    struct
    { T x, y, z; };
  };

  Vector()                                : x(0), y(0), z(0)                        { }
  Vector(T x_, T y_, T z_)                : x(x_), y(y_), z(z_)                     { }
  Vector(const Vector<T, N> &other)       : x(other.x), y(other.y), z(other.z)      { }
  Vector(const std::array<T, N> &data_)   : x(data_[0]), y(data_[1]), z(data_[2])   { }
  Vector(const std::string &str)       { fromString(str); }
  
  T& operator[](int dim)               { return data[dim]; }
  const T& operator[](int dim) const   { return data[dim]; }
  
  std::string toString() const            { std::ostringstream ss; ss << (*this); return ss.str(); }
  void fromString(const std::string &str) { std::istringstream ss(str); ss >> (*this); }
  
  bool operator==(const Vector<T, N> &other) const
  {
    for(int i = 0; i < N; i++)
      {
        if(data[i] != other.data[i])
          { return false; }
      }
    return true;
  }
  bool operator!=(const Vector<T, N> &other) const
  { return !(*this == other); }
  
  Vector<T, N>& operator+=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++)
      { data[i] += other.data[i]; }
    return *this;
  }
  Vector<T, N>& operator-=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++)
      { data[i] -= other.data[i]; }
    return *this;
  }
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
    for(int i = 0; i < N; i++)
      { result.data[i] -= other.data[i]; }
    return result;
  }
  
  Vector<T, N>& operator*=(T scalar)
  {
    for(int i = 0; i < N; i++)
      { data[i] *= scalar; }
    return *this;
  }
  Vector<T, N>& operator/=(T scalar)
  {
    for(int i = 0; i < N; i++)
      { data[i] /= scalar; }
    return *this;
  }
  Vector<T, N> operator*(T scalar) const
  {
    Vector<T, N> result(data);
    for(int i = 0; i < N; i++)
      { result.data[i] *= scalar; }
    return result;
  }
  Vector<T, N> operator/(T scalar) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++)
      { result.data[i] /= scalar; }
    return result;
  }
  
  void ceil()
  { for(auto &d : data) { d = std::ceil(d); } }
  Vector<T, N> getCeil() const
  {
    Vector<T, N> v = *this;
    v.ceil();
    return v;
  }
  void floor()
  { for(auto &d : data) { d = std::floor(d); } }
  Vector<T, N> getFloor() const
  {
    Vector<T, N> v = *this;
    v.floor();
    return v;
  }

  T length2() const
  {
    T sqsum = T();
    for(auto d : data)
      { sqsum += d*d; }
    return sqsum;
  }
  T length() const
  { return sqrt(length2()); }
  Vector<T, N> normalized() const
  {
    std::array<T, N> nData = data;
    T len = length();
    for(auto &d : nData)
      { d /= len; }
    return Vector<T, N>(nData);
  }
};
  
template<typename T>
struct Vector<T, 4>
{
  static constexpr int N = 4;
  union
  {
    struct
    { T x, y, z, w; };
    std::array<T, N> data;
  };

  Vector()                                 : x((T)0), y((T)0), z((T)0), w((T)1)  { }
  Vector(T x_, T y_, T z_, T w_)           : x(x_), y(y_), z(z_), w(w_)          { }
  Vector(const Vector<T, N> &other)        : x(other.x), y(other.y),
                                             z(other.z), w(other.w)              { }
  Vector(const std::array<T, N> &data_)    : x(data_[0]), y(data_[1]),
                                             z(data_[2]), w(data_[3])            { }
  Vector(const std::string &str)       { fromString(str); }
  
  Vector<T, 4>& operator=(const Vector<T, 4> &other)
  {
    for(int i = 0; i < N; i++)
      { data[i] = other.data[i]; }
    return *this;
  }
  
  T& operator[](int dim)               { return data[dim]; }
  const T& operator[](int dim) const   { return data[dim]; }
  
  std::string toString() const            { std::ostringstream ss; ss << (*this); return ss.str(); }
  void fromString(const std::string &str) { std::istringstream ss(str); ss >> (*this); }

  bool operator==(const Vector<T, N> &other) const
  {
    for(int i = 0; i < N; i++)
        if(data[i] != other.data[i])
          { return false; }
    return true;
  }
  bool operator!=(const Vector<T, N> &other) const
  { return !(*this == other); }

  Vector<T, N>& operator+=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++)
      { data[i] += other.data[i]; }
    return *this;
  }
  Vector<T, N>& operator-=(const Vector<T, N> &other)
  {
    for(int i = 0; i < N; i++)
      { data[i] -= other.data[i]; }
    return *this;
  }
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
    for(int i = 0; i < N; i++)
      { result.data[i] -= other.data[i]; }
    return result;
  }

  
  Vector<T, N>& operator*=(T scalar)
  {
    for(int i = 0; i < N; i++)
      { data[i] *= scalar; }
    return *this;
  }
  Vector<T, N>& operator/=(T scalar)
  {
    for(int i = 0; i < N; i++)
      { data[i] /= scalar; }
    return *this;
  }
  
  Vector<T, N> operator*(T scalar) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++)
      { result.data[i] *= scalar; }
    return result;
  }
  Vector<T, N> operator/(T scalar) const
  {
    Vector<T, N> result(*this);
    for(int i = 0; i < N; i++)
      { result.data[i] /= scalar; }
    return result;
  }
  
  void ceil()
  { for(auto &d : data) { d = std::ceil(d); } }
  Vector<T, N> getCeil() const
  {
    Vector<T, N> v = *this;
    v.ceil();
    return v;
  }
  void floor()
  { for(auto &d : data) { d = std::floor(d); } }
  Vector<T, N> getFloor() const
  {
    Vector<T, N> v = *this;
    v.floor();
    return v;
  }

  T length2() const
  {
    T sqsum = T();
    for(auto d : data)
      { sqsum += d*d; }
    return sqsum;
  }
  T length() const
  { return sqrt(length2()); }
  
  Vector<T, N> normalized() const
  {
    std::array<T, N> nData = data;
    T len = length();
    for(auto &d : nData)
      { d /= len; }
    return Vector<T, N>(nData);
  }
};

template<typename T, int N>
inline Vector<T, N> normalize(const Vector<T, N> &v)
{ return v.normalized(); }
template<typename T, int N>
inline T length2(const Vector<T, N> &v)
{ return v.length2(); }
template<typename T, int N>
inline T length(const Vector<T, N> &v)
{ return v.length(); }

template<typename T, int N>
inline Vector<T, N> operator-(const Vector<T, N> &v)
{
  Vector<T, N> result;
  for(int i = 0; i < N; i++)
    { result.data[i] = -v[i]; }
  return result;
}
template<typename T, int N>
inline Vector<T, N> operator*(T scalar, const Vector<T, N> &v)
{
  Vector<T, N> result;
  for(int i = 0; i < N; i++)
    { result.data[i] = scalar * v[i]; }
  return result;
}
template<typename T, int N>
inline Vector<T, N> operator/(T scalar, const Vector<T, N> &v)
{
  Vector<T, N> result;
  for(int i = 0; i < N; i++)
    { result.data[i] = scalar / v[i]; }
  return result;
}

#endif //VECTOR_HPP
