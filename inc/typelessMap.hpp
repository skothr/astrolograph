#ifndef TYPELESS_MAP_HPP
#define TYPELESS_MAP_HPP


namespace astro
{
  // template<typename T> MapValue;
  
  // struct MapValueBase
  // {
  //   template<typename T> void set(const T &v) { ((MapValue<T>*)this)->set(v); }
  //   template<typename T> const T& get() const { return ((MapValue<T>*)this)->get(); }
  //   template<typename T> T& get()             { return ((MapValue<T>*)this)->get(); }
  // };

  // template<typename T>
  // struct MapValue : public MapValueBase
  // {
  //   T value;

  //   MapValue(const T&v) : value(v) { }
    
  //   void set(const T &v) { value = v; }
  //   const T& get() const { return value; }
  //   T& get()             { return value; }
  // };
  
};

#endif // TYPELESS_MAP_HPP
