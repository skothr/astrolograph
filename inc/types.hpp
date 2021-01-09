#ifndef TYPES_HPP
#define TYPES_HPP

#include <unordered_map>
#include <string>
#include <typeinfo>
#include <typeindex>
#include <cstdint>

#include "vector.hpp"

#define BUILTIN_TYPE_NAMES 0

// name of bad/unknown type
inline static const std::string BAD_TYPE_NAME = "<badtype>";

// maps type index to simple type name
inline static std::unordered_map<std::type_index, std::string> TYPE_NAMES =
  {// basic integral types
   { std::type_index(typeid(char)),           "char"   },
   { std::type_index(typeid(short)),          "short"  },
   { std::type_index(typeid(int)),            "int"    },
   { std::type_index(typeid(long int)),       "long"   },
   { std::type_index(typeid(unsigned char)),  "uchar"  },
   { std::type_index(typeid(unsigned short)), "ushort" },
   { std::type_index(typeid(unsigned int)),   "uint"   },
   { std::type_index(typeid(unsigned long)),  "ulong"  },
   { std::type_index(typeid(float)),          "float"  },
   { std::type_index(typeid(double)),         "double" },
   // explicit-width integral types (cstdint)
   { std::type_index(typeid(int8_t)),         "int8"   },
   { std::type_index(typeid(int16_t)),        "int16"  },
   { std::type_index(typeid(int32_t)),        "int32"  },
   { std::type_index(typeid(int64_t)),        "int64"  },
   { std::type_index(typeid(uint8_t)),        "uint8"  },
   { std::type_index(typeid(uint16_t)),       "uint16" },
   { std::type_index(typeid(uint32_t)),       "uint32" },
   { std::type_index(typeid(uint64_t)),       "uint64" },
   // custom vector types
   { std::type_index(typeid(Vec2f)),          "Vec2i"  },
   { std::type_index(typeid(Vec3f)),          "Vec3i"  },
   { std::type_index(typeid(Vec4f)),          "Vec4i"  },
   { std::type_index(typeid(Vec2f)),          "Vec2f"  },
   { std::type_index(typeid(Vec3f)),          "Vec3f"  },
   { std::type_index(typeid(Vec4f)),          "Vec4f"  },
   { std::type_index(typeid(Vec2f)),          "Vec2d"  },
   { std::type_index(typeid(Vec3f)),          "Vec3d"  },
   { std::type_index(typeid(Vec4f)),          "Vec4d"  },
   // other types
   { std::type_index(typeid(std::string)),    "string" },
   { std::type_index(typeid(bool)),           "bool" },
  };

// maps type index to number of command-line arguments that must follow
inline static std::unordered_map<std::type_index, int> TYPE_NARGS =
  {// basic integral types
   { std::type_index(typeid(char)),           1 },
   { std::type_index(typeid(short)),          1 },
   { std::type_index(typeid(int)),            1 },
   { std::type_index(typeid(long int)),       1 },
   { std::type_index(typeid(unsigned char)),  1 },
   { std::type_index(typeid(unsigned short)), 1 },
   { std::type_index(typeid(unsigned int)),   1 },
   { std::type_index(typeid(unsigned long)),  1 },
   { std::type_index(typeid(float)),          1 },
   { std::type_index(typeid(double)),         1 },
   // explicit-width integral types (cstdint)
   { std::type_index(typeid(int8_t)),         1 },
   { std::type_index(typeid(int16_t)),        1 },
   { std::type_index(typeid(int32_t)),        1 },
   { std::type_index(typeid(int64_t)),        1 },
   { std::type_index(typeid(uint8_t)),        1 },
   { std::type_index(typeid(uint16_t)),       1 },
   { std::type_index(typeid(uint32_t)),       1 },
   { std::type_index(typeid(uint64_t)),       1 },
   // custom vector types
   { std::type_index(typeid(Vec2f)),          2 },
   { std::type_index(typeid(Vec3f)),          3 },
   { std::type_index(typeid(Vec4f)),          4 },
   { std::type_index(typeid(Vec2f)),          2 },
   { std::type_index(typeid(Vec3f)),          3 },
   { std::type_index(typeid(Vec4f)),          4 },
   { std::type_index(typeid(Vec2f)),          2 },
   { std::type_index(typeid(Vec3f)),          3 },
   { std::type_index(typeid(Vec4f)),          4 },
   // other types
   { std::type_index(typeid(std::string)),    1 },
   { std::type_index(typeid(bool)),           0 }, // no arguments -- presence/absence indicated true/false
  };

template<typename T>
const std::string& getTypeName()
{
  T t;
#if BUILTIN_TYPE_NAMES       // use built-in typeid names
  return typeid(T).name();
#else // !BUILTIN_TYPE_NAMES // use simple names (defined above)
  auto iter = TYPE_NAMES.find(std::type_index(typeid(t)));
  return (iter != TYPE_NAMES.end() ? iter->second : BAD_TYPE_NAME);
#endif
}

template<typename T>
int getTypeNargs()
{
  T t;
  auto iter = TYPE_NARGS.find(std::type_index(typeid(t)));
  return (iter != TYPE_NARGS.end() ? iter->second : 0);
}



#endif // TYPES_HPP
