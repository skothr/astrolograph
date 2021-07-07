#ifndef RANGE_HPP
#define RANGE_HPP

template<typename T>
struct Range
{
  T lower; T upper;

  Range(const T &l=(T)0, const T &u=(T)0) : lower(l),  upper(u)  { }
  Range(const Range &r)                   : lower(r.lower), upper(r.upper) { }
  
  // assignment
  Range& operator=(const Range &r) { lower = r.lower; upper = r.upper; return *this; }
  
  // equivalence
  bool operator==(const Range &r) const { return (lower == r.lower && upper == r.upper); }
  bool operator!=(const Range &r) const { return (lower != r.lower || upper != r.upper); }
  
  // check if a point is within the range (inclusive)
  bool contains(const T &x) const { return (x >= lower && x <= upper); }

  // return point clipped to within range
  bool clip(const T &x) const { return std::max(std::min(x, upper), lower); }
  
  // span (size of space contained within range)
  T span() const { return upper - lower; }
  
  // expand or shrink range
  void extend(const T &amount)    { lower -= amount; upper += amount; }
  Range extended(const T &amount) { return Range(lower - amount, upper + amount); }

  // extend to contain the given data point
  void fit(const T &x) { lower = std::min(lower, x); upper = std::max(upper, x); }
};


#endif // RANGE_HPP
