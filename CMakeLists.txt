cmake_minimum_required(VERSION 3.6)

# set the project name and version
project(astrolograph VERSION 0.2)
configure_file(inc/version/version.hpp.in ../inc/version/version.hpp)

# includes
include_directories(inc)

# debug/release flags
set(TARGET astrolograph)
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g -Wall -pg") # debug
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -Wall")      # release

# global flags
set(EXTRA_FLAGS_CXX17 "-std=gnu++17")
set(GLOBAL_LIB_FLAGS "-DCURL_STATICLIB -DGLEW_STATIC -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DHAS_REMOTE_API=0 -DAUTO_DOWNLOAD=0")
set(CMAKE_CXX_FLAGS_GLOBAL "${CMAKE_CXX_FLAGS} ${GLOBAL_LIB_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS_GLOBAL "${CMAKE_EXE_LINKER_FLAGS} ${GLOBAL_LIB_FLAGS}")
set(CMAKE_LIBRARY_LINKER_FLAGS_GLOBAL "${CMAKE_LIBRARY_LINKER_FLAGS} ${GLOBAL_LIB_FLAGS}")

if (WIN32)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/x86_64-w64-mingw32")
endif (WIN32)

# build internal libs
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_GLOBAL} ${EXTRA_FLAGS_CXX17}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS_GLOBAL} ${EXTRA_FLAGS_CXX17}")
set(CMAKE_LIBRARY_LINKER_FLAGS "${CMAKE_LIBRARY_LINKER_FLAGS_GLOBAL} ${EXTRA_FLAGS_CXX17}")
add_library(astro STATIC
  src/aspectNode.cpp
  src/astro.cpp
  src/chartCompare.cpp
  src/chart.cpp
  src/chartDataNode.cpp
  src/chartNode.cpp
  src/chartView.cpp
  src/chartViewNode.cpp
  src/compareNode.cpp
  src/dateTime.cpp
  src/ephemeris.cpp
  src/location.cpp
  src/locationNode.cpp
  src/locationWidget.cpp
  src/node.cpp
  src/nodeGraph.cpp
  src/plotNode.cpp
  src/progressNode.cpp
  src/timeNode.cpp
  src/timeWidget.cpp
  src/viewSettings.cpp
  )

# build external libs
set(GLFW_ROOT_DIR "libs/glfw")
set(GLEW_ROOT_DIR "libs/glew-2.1.0")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_GLOBAL}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS_GLOBAL}")
set(CMAKE_LIBRARY_LINKER_FLAGS "${CMAKE_LIBRARY_LINKER_FLAG_GLOBALS}")
include_directories(
  libs/imgui
  ${GLEW_ROOT_DIR}/include
  libs/stb
  libs/swe/src
  libs/date
  )
add_subdirectory(libs)

find_library(LIB_CURL curl)

# glfw
# find_package(glfw3 REQUIRED)
# include_directories(${GLFW_INCLUDE_DIRS})
# link_libraries(${GLFW_LIBRARY_DIRS})
# opengl
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
link_libraries(${OPENGL_LIBRARY_DIRS})
# glew
set(GLEW_LIBRARIES glew)
# find_package(GLEW REQUIRED)
# include_directories(${GLEW_INCLUDE_DIRS})
# link_libraries(${GLEW_LIBRARY_DIRS})
# set(GLEW_LIBRARIES GLEWd)
message(STATUS "GLEW LIBS: ${GLEW_LIBRARIES}")
message(STATUS "CURL LIBS: ${LIB_CURL}")

target_link_libraries(astro stdc++fs imgui ${GLEW_LIBRARIES} swe datetz curl) # ${LIB_CURL})

# if (APPLE)
#     find_library(COCOA_LIBRARY Cocoa)
#     find_library(OpenGL_LIBRARY OpenGL)
#     find_library(IOKIT_LIBRARY IOKit)
#     find_library(COREVIDEO_LIBRARY CoreVideo)
#     SET(EXTRA_LIBS ${COCOA_LIBRARY} ${OpenGL_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
# endif (APPLE)
# if (WIN32)
# # nothing now
# endif (WIN32)

add_executable(${TARGET} main.cpp)
target_link_libraries(
  ${TARGET}
  stdc++fs
  astro
  imgui
  glfw
  datetz
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  curl
  )
