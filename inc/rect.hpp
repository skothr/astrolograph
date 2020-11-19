#ifndef RECT_HPP
#define RECT_HPP

#include "vector.hpp"
#include <cmath>
#include <ostream>

template<typename T>
struct Rect
{
  Vector<T, 2> p1; // top-left point
  Vector<T, 2> p2; // bottom-right point

  Rect() : p1(0,0), p2(0,0)                                                 { }
  Rect(const Rect<T> &other) : p1(other.p1), p2(other.p2)                   { }
  Rect(const Vector<T, 2> &_p1, const Vector<T, 2> &_p2) : p1(_p1), p2(_p2) { }
  Rect<T>& operator=(const Rect<T> &other)     { p1=other.p1; p2=other.p2; return *this; }
  bool operator==(const Rect<T> &other) const  { return (p1 == other.p1 && p2 == other.p2); }
  bool operator!=(const Rect<T> &other) const  { return (p1 != other.p1 && p2 != other.p2); }

  // offset by vector (whole rect)
  Rect<T>&      operator+=(const Vector<T, 2> &offset)       { p1 += offset; p2 += offset; return *this; }
  const Rect<T> operator+ (const Vector<T, 2> &offset) const { return Rect<T>(p1+offset, p2+offset); }
  Rect<T>&      operator-=(const Vector<T, 2> &offset)       { p1 -= offset; p2 -= offset; return *this; }
  const Rect<T> operator- (const Vector<T, 2> &offset) const { return Rect<T>(p1-offset, p2-offset); }
  
  Rect<T>&      operator*=(T scale)       { p1 *= scale; p2 *= scale; return *this; }
  const Rect<T> operator* (T scale) const { return Rect<T>(p1*scale, p2*scale); }
  Rect<T>&      operator/=(T scale)       { p1 /= scale; p2 /= scale; return *this; }
  const Rect<T> operator/ (T scale) const { return Rect<T>(p1/scale, p2/scale); }

  const Vector<T, 2>& pos() const { return p1; }
  Vector<T, 2> size() const       { return (p2 - p1); }
  Vector<T, 2> center() const     { return (p2 + p1) / 2.0; }

  void move(const Vector<T, 2> &dp)     { p1 += dp; p2 += dp; }
  void setPos(const Vector<T, 2> &p)    { p2 = p+size(); p1 = p; }
  void setSize(const Vector<T, 2> &s)   { p2 = p1+s; }
  void setCenter(const Vector<T, 2> &c) { Vector<T, 2> hs = size()/2.0; p1 = c-hs; p2 = c+hs; }
  
  bool valid() const { return (p1.x < p2.x && p1.y < p2.y); }
  void fix()
  {
    p1 = Vector<T, 2>(std::min(p1.x, p2.x), std::min(p1.y, p2.y)); // min
    p2 = Vector<T, 2>(std::max(p1.x, p2.x), std::max(p1.y, p2.y)); // max
  }
  Rect<T> fixed() const
  { return Rect<T>(Vector<T, 2>(std::min(p1.x, p2.x), std::min(p1.y, p2.y)), Vector<T, 2>(std::max(p1.x, p2.x), std::max(p1.y, p2.y))); }
  
  bool contains(const Vector<T, 2> &p) const
  { return (p.x >= p1.x && p.x <= p2.x && p.y > p1.y && p.y <= p2.y); }
  // rect/rect intersection
  bool intersects(const Rect<T> &other) const
  { return !((p1.x > other.p2.x || other.p1.x > p2.x) || (p1.y > other.p2.y || other.p1.y > p2.y)); }
  Rect<T> intersection(const Rect<T> &other) const
  { return Rect<T>(Vector<T, 2>(std::max(p1.x, other.p1.x), std::max(p1.y, other.p1.y)), Vector<T, 2>(std::min(p2.x, other.p2.x), std::min(p2.y, other.p2.y))); }

  // extend to contain other rect
  Rect<T> combined(const Rect<T> &other) const
  { return Rect<T>(Vector<T, 2>(std::min(p1.x, other.p1.x), std::min(p1.y, other.p1.y)), Vector<T, 2>(std::max(p2.x, other.p2.x), std::max(p2.y, other.p2.y)) ); }
  Rect<T> combined(const Vector<T, 2> &p) const
  { return Rect<T>(Vector<T, 2>(std::min(p1.x, p.x),        std::min(p1.y, p.y)),        Vector<T, 2>(std::max(p2.x, p.x),        std::max(p2.y, p.y)) ); }
  Rect<T> combine(const Rect<T> &other)
  {
    p1 = Vector<T, 2>(std::min(p1.x, other.p1.x), std::min(p1.y, other.p1.y));
    p2 = Vector<T, 2>(std::max(p2.x, other.p2.x), std::max(p2.y, other.p2.y));
    return *this;
  }
  Rect<T> combine(const Vector<T, 2> &p)
  {
    p1 = Vector<T, 2>(std::min(p1.x, p.x), std::min(p1.y, p.y));
    p2 = Vector<T, 2>(std::max(p2.x, p.x), std::max(p2.y, p.y));
    return *this;
  }

  // evenly expand sides of rect by dist
  const Rect<T>& expand(T dist)                    { *this = expanded(dist); return *this; }
  const Rect<T>& expand(const Vector<T, 2> &dist)  { *this = expanded(dist); return *this; }
  Rect<T> expanded(T dist) const                   { return expanded(Vector<T, 2>(dist, dist)); }
  Rect<T> expanded(const Vector<T, 2> &dist) const { return Rect<T>(p1-dist, p2+dist); }
  
  // clamps point coordinates to be inside rect
  Vector<T, 2> clampPoint(Vector<T, 2> p) const
  {
    if(p.x < p1.x) { p.x = p1.x; } else if(p.x > p2.x) { p.x = p2.x; }
    if(p.y < p1.y) { p.y = p1.y; } else if(p.y > p2.y) { p.y = p2.y; }
    return p;
  }
};

template<typename T>
inline std::ostream& operator<<(std::ostream &os, const Rect<T> &rect)
{
  os << "RECT[" << rect.p1 << " --> " << rect.p2 << "]";
  return os;
}

// shorthand
typedef Rect<float> Rect2f;

#endif // RECT_HPP
