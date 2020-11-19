#ifndef GEOMETRY_HPP
#define GEOMETRY_HPP

#include "vector.hpp"
#include "rect.hpp"

// helpers
template<typename T>
inline Vector<T, 2> toPolar(const Vector<T, 2> &p)     { return Vector<T, 2>(sqrt(p.x*p.x + p.y*p.y), atan2(p.y, p.x)); }
template<typename T>
inline Vector<T, 2> toCartesian(const Vector<T, 2> &p) { return Vector<T, 2>(p.x*cos(p.y), p.x*sin(p.y)); }

template<typename T>
inline Rect<T> toPolar(const Rect<T> &r)               { return Rect<T>(toPolar(r.p1), toPolar(r.p2)); }
template<typename T>
inline Rect<T> toCartesian(const Rect<T> &r)           { return Rect<T>(toCartesian(r.p1), toCartesian(r.p2)); }

template<typename T, int N>
inline Vector<T, N> lerp(const Vector<T, N> &x0, const Vector<T, N> &x1, T alpha)
{ return x0 * alpha + x1 * ((T)1 - alpha); }
template<typename T>
inline T lerp(T x0, T x1, T alpha)
{ return x0 * alpha + x1 * ((T)1 - alpha); }

// line/line intersection
inline bool intersects(const Vec2f &p1a, const Vec2f &p1b, const Vec2f &p2a, const Vec2f &p2b)
{
  float d = (p1b.x - p1a.x) * (p2b.y - p2a.y) - (p1b.y - p1a.y) * (p2b.x - p2a.x);
  if(d == 0.0f) { return false; }
  
  float r = (((p1a.y - p2a.y) * (p2b.x - p2a.x) - (p1a.x - p2a.x) * (p2b.y - p2a.y)) / d);
  float s = (((p1a.y - p2a.y) * (p1b.x - p1a.x) - (p1a.x - p2a.x) * (p1b.y - p1b.y)) / d);
  return (r >= 0.0f && r <= 1.0f && s >= 0.0f && s <= 1.0f);
}
inline Vec2f intersection(const Vec2f &p1a, const Vec2f &p1b, const Vec2f &p2a, const Vec2f &p2b)
{
  float d = (p1b.x - p1a.x) * (p2b.y - p2a.y) - (p1b.y - p1a.y) * (p2b.x - p2a.x);
  if(d == 0.0f) { return Vec2f(0,0); }
  float r = (((p1a.y - p2a.y) * (p2b.x - p2a.x) - (p1a.x - p2a.x) * (p2b.y - p2a.y)) / d);
  float s = (((p1a.y - p2a.y) * (p1b.x - p1a.x) - (p1a.x - p2a.x) * (p1b.y - p1b.y)) / d);

  return Vec2f(p1a.x + r*(p1b.x-p1a.x), p1a.y + r*(p1b.y-p1a.y));
}

// rect/line intersection
inline bool intersects(const Rect2f &r, const Vec2f &p1, const Vec2f &p2)
{
  bool top    = intersects(r.p1, Vec2f(r.p2.x, r.p1.y), p1, p2);
  bool bottom = intersects(r.p2, Vec2f(r.p1.x, r.p2.y), p1, p2);
  bool left   = intersects(r.p1, Vec2f(r.p1.x, r.p2.y), p1, p2);
  bool right  = intersects(r.p2, Vec2f(r.p2.x, r.p1.y), p1, p2);
  return (top || bottom || left || right);
}
// TODO: optimize!
inline std::vector<Vec2f> intersection(const Rect2f &r, const Vec2f &p1, const Vec2f &p2)
{
  std::vector<Vec2f> intersectPoints;
  
  if(intersects(r.p1, Vec2f(r.p2.x, r.p1.y), p1, p2)) // top
    { intersectPoints.push_back(intersection(r.p1, Vec2f(r.p2.x, r.p1.y), p1, p2)); }
  if(intersects(r.p2, Vec2f(r.p1.x, r.p2.y), p1, p2)) // bottom
    { intersectPoints.push_back(intersection(r.p2, Vec2f(r.p1.x, r.p2.y), p1, p2)); }
  if(intersects(r.p1, Vec2f(r.p1.x, r.p2.y), p1, p2)) // left
    { intersectPoints.push_back(intersection(r.p1, Vec2f(r.p1.x, r.p2.y), p1, p2)); }
  if(intersects(r.p2, Vec2f(r.p2.x, r.p1.y), p1, p2)) // right
    { intersectPoints.push_back(intersection(r.p2, Vec2f(r.p2.x, r.p1.y), p1, p2)); }
  
  return intersectPoints;
}



#endif // GEOMETRY_HPP
