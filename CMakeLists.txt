cmake_minimum_required(VERSION 3.6)
project(astrolograph)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include/swe -std=gnu++17 -lstdc++fs -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DHAS_REMOTE_API=0 -DAUTO_DOWNLOAD=0")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -std=gnu++17 -lstdc++fs -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DHAS_REMOTE_API=0 -DAUTO_DOWNLOAD=0")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_LIBRARY_LINKER_FLAGS} -std=gnu++17 -lstdc++fs -DIMGUI_IMPL_OPENGL_LOADER_GLEW -DHAS_REMOTE_API=0 -DAUTO_DOWNLOAD=0")

set(CMAKE_VERBOSE_MAKEFILE 1)
set(APP_VERSION_MAJOR 1)
set(APP_VERSION_MINOR 0)
set(APP_TARGET astrolograph)

include_directories(
  inc
  libs
  libs/imgui
  libs/date
  # libs/date/date
  )

add_subdirectory(libs)
add_subdirectory(inc)
add_subdirectory(src)
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

target_link_libraries(astro imgui datetz stdc++fs)

# include_directories(${PROJECT_SOURCE_DIR})

find_library(LIB_SWE swe)
find_library(LIB_CURL curl)
find_library(LIB_CURL date)

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

add_executable(${APP_TARGET} main.cpp)
# target_link_libraries(${APP_TARGET} imgui)
target_link_libraries(
  ${APP_TARGET}
  imgui
  datetz
  astro
  glfw
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${EXTRA_LIBS}
  ${LIB_SWE}
  ${LIB_CURL}
  )
