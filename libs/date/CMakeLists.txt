cmake_minimum_required(VERSION 3.6)

include_directories(.)
add_library(datetz STATIC tz.cpp)
target_compile_definitions(datetz PRIVATE HAS_REMOTE_API)  # no remote API
target_compile_definitions(datetz PRIVATE AUTO_DOWNLOAD=0) # tzData included -- don't auto-download
target_compile_definitions(datetz PRIVATE INSTALL=.)       # set root directory as location of tzData install
set_target_properties(datetz PROPERTIES CXX_STANDARD 17)
