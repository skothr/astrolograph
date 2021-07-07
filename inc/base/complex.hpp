#ifndef COMPLEX_HPP
#define COMPLEX_HPP

#include "vector.hpp"

template<typename T=double>
struct Complex
{

private:
  T angle_;
public:
  T real; T imag;

  Complex(T a=T(0))              { fromAngle(a); }//: real(r),   imag(T(0))   { }
  Complex(T r, T i)              : real(r),   imag(i)   { }
  Complex(const Vector<T, 2> &v) : real(v.x), imag(v.y) { }
  Complex(const Complex &c)  : real(c.real), imag(c.imag) { }

  Complex& fromAngle(T a) { real = cos(a*2.0*M_PI); imag = sin(a*2.0*M_PI); return *this; }
  
  // operator T&()      { return angle(); }
  // operator T() const { return angle(); }

  double& angle()             { angle_ = atan2(imag, real)/(2.0*M_PI); return angle_; }
  const double& angle() const { return angle_; }

  // assignment
  Complex& operator=(T x)              { fromAngle(x); return *this; } //real = x; imag = 0; return *this; }
  Complex& operator=(const Complex &c) { real = c.real; imag = c.imag; return *this; }

  // equivalence
  bool operator==(const Complex &c) const { return (real == c.real && imag == c.imag); }
  bool operator!=(const Complex &c) const { return (real != c.real && imag != c.imag); }

  // negation
  Complex operator-() const { return Complex(-real, -imag); }
  Complex conj() const      { return Complex(real,  -imag); }

  // addition
  Complex& operator+=(const Complex &c)     { real += c.real; imag += c.imag; return *this; }
  Complex operator+(const Complex &c) const { return Complex(real+c.real, imag+c.imag); }

  // subtraction
  Complex& operator-=(const Complex &c)     { real -= c.real; imag -= c.imag; return *this; }
  Complex operator-(const Complex &c) const { return Complex(real-c.real, imag-c.imag); }

  // complex multiplication
  Complex& operator*=(const Complex &c)
  {
    T r = real*c.real - imag*c.imag;
    T i = real*c.imag + imag*c.real;
    real = r; imag = i;
    return *this;
  }
  Complex operator*(const Complex &c) const { return Complex(*this) *= c; }
  
  // complex division
  Complex& operator/=(const Complex &c)
  {
    real = (real*c.real + imag*c.imag) / (c.real*c.real + c.imag*c.imag);
    imag = (imag*c.real - real*c.imag) / (c.real*c.real + c.imag*c.imag);
    return *this;
  }
  Complex operator/(const Complex &c) const { return Complex(*this) /= c; }

  // scalar multiplication
  Complex& operator*=(T x) { real *= x; imag *= x; return *this; }
  Complex operator*(T x)   { return Complex(real*x, imag*x); }

  // scalar division
  Complex& operator/=(T x) { real /= x; imag /= x; return *this; }
  Complex operator/(T x)   { return Complex(real/x, imag/x); }
};

// pre-operators
template<typename T> //template<typename U>
inline Complex<T> operator+(T x, const Complex<T> &c) { return Complex<T>(x+c.real, c.imag); }
template<typename T> //template<typename U>
inline Complex<T> operator-(T x, const Complex<T> &c) { return Complex<T>(x-c.real, -c.imag); }
template<typename T> //template<typename U>
inline Complex<T> operator*(T x, const Complex<T> &c) { return Complex<T>(x*c.real, x*c.imag); }
template<typename T> //template<typename U>
inline Complex<T> operator/(T x, const Complex<T> &c) { return Complex<T>(x/c.real, x/c.imag); }



#endif // COMPLEX_HPP
