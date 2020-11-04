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

#endif // GEOMETRY_HPP
