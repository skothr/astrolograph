cmake_minimum_required(VERSION 3.6)

#include_directories(./src)
include_directories(./include)
add_library(nfd STATIC
  ./lib/linux/x64/libnfd.a
  )
