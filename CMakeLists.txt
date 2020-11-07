cmake_minimum_required(VERSION 3.6)
# set the project name and version
project(astrolograph VERSION 0.1)
configure_file(inc/version/version.hpp.in ../inc/version/version.hpp)

set(TARGET astrolograph)
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g -pg")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3")

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -std=gnu++17 -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DHAS_REMOTE_API=0 -DAUTO_DOWNLOAD=0")
set(CMAKE_EXE_LINKER_FLAGS
  "${CMAKE_EXE_LINKER_FLAGS} -std=gnu++17 -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DHAS_REMOTE_API=0 -DAUTO_DOWNLOAD=0")
set(CMAKE_LIBRARY_LINKER_FLAGS
  "${CMAKE_LIBRARY_LINKER_FLAGS} -std=gnu++17 -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DHAS_REMOTE_API=0 -DAUTO_DOWNLOAD=0")

include_directories(
  inc
  libs/imgui
  libs/stb
  libs/swe/src
  libs/date
  )

add_subdirectory(libs)
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
  src/progressNode.cpp
  src/timeNode.cpp
  src/timeWidget.cpp
  )

target_link_libraries(astro stdc++fs imgui swe datetz)

# find_library(LIB_SWE swe)
find_library(LIB_CURL curl)
# find_library(LIB_CURL date)

# glfw
find_package(glfw3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
link_libraries(${GLFW_LIBRARY_DIRS})

# opengl
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# glew
find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

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
# target_link_libraries(${TARGET} imgui)
target_link_libraries(
  ${TARGET}
  stdc++fs
  astro
  imgui
  glfw
  datetz
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${EXTRA_LIBS}
  ${LIB_SWE}
  ${LIB_CURL}
  )
