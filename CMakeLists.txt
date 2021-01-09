cmake_minimum_required(VERSION 3.6)
cmake_policy(SET CMP0072 NEW) # opengl policy
set(OpenGL_GL_PREFERENCE GLVND)

# set the project name and version
project(astrolograph VERSION 0.5)
configure_file(inc/version/version.hpp.in ../inc/version/version.hpp)

set(TARGET astrolograph)
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g -Wall -pg") # debug
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -Wall")      # release

if (WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/x86_64-w64-mingw32/include -L/usr/x86_64-w64-mingw32/lib")
endif (WIN32)

# build internal libs
include_directories(inc)
add_library(astro STATIC
  src/aspectNode.cpp
  src/astro.cpp
  src/astroWindow.cpp
  src/chartCompare.cpp
  src/chart.cpp
  src/chartDataNode.cpp
  src/chartNode.cpp
  src/chartView.cpp
  src/chartViewNode.cpp
  src/chartParamWidget.cpp
  src/compareNode.cpp
  src/dateTime.cpp
  src/ephemeris.cpp
  src/fileDialog.cpp
  src/groupNode.cpp
  src/location.cpp
  src/locationNode.cpp
  src/locationWidget.cpp
  src/moonNode.cpp
  src/node.cpp
  src/nodeConnector.cpp
  src/nodeGraph.cpp
  src/nodeList.cpp
  src/plotNode.cpp
  src/progressNode.cpp
  src/settingForm.cpp
  src/shapeBuffer.cpp
  src/timeNode.cpp
  src/timeWidget.cpp
  src/vedicNode.cpp
  src/viewSettings.cpp
  )
set_target_properties(astro PROPERTIES CXX_STANDARD 17)
target_compile_definitions(astro PRIVATE GLEW_STATIC)
target_compile_options(astro PRIVATE -static -Wno-deprecated)

# build external libs
set(GLFW_ROOT_DIR "libs/glfw")
set(GLEW_ROOT_DIR "libs/glew-2.1.0")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_GLOBAL}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS_GLOBAL}")
set(CMAKE_LIBRARY_LINKER_FLAGS "${CMAKE_LIBRARY_LINKER_FLAG_GLOBAL}")
include_directories(
  libs/imgui
  ${GLEW_ROOT_DIR}/include
  ${GLFW_ROOT_DIR}/include
  libs/stb
  libs/swe/src
  libs/date
  libs/json/include
  libs/nfd/include
  )
# add external dependencies
add_subdirectory(libs)

#find_library(LIB_CURL curl)
set(LIB_CURL curl)
# set(LIB_SSL /usr/x86_64-w64-mingw32/lib/libssl.a) #openssl)
# set(LIB_ZLIB /usr/x86_64-w64-mingw32/lib/libz.a) #zlib)
#set(LIB_CURL /usr/x86_64-w64-mingw32/lib/libcurl.a)

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

set(EXTRA_LIBS "")
if (UNIX) # GTK on Linux
  set(LINUX_USE_GTK 1) # set to false to link without GTK (via zenity)
  message("==> Compiling for Linux...")

  if (LINUX_USE_GTK)
    find_library(LIB_NFD nfd "${CMAKE_SOURCE_DIR}/libs/nfd/lib/linux/x64")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules (GTK3 REQUIRED gtk+-3.0)
    set(EXTRA_LIBS ${EXTRA_LIBS} ${GTK3_LIBRARIES})
  else ()
    find_library(LIB_NFD nfd "${CMAKE_SOURCE_DIR}/libs/nfd/lib/linux-zenity/x64")
  endif (LINUX_USE_GTK)
  
elseif (WIN32)
  message("==> Compiling for Windows...")
  message("TODO: Windows support!")
  find_library(LIB_NFD nfd "${CMAKE_SOURCE_DIR}/libs/nfd/lib/windows/x64")
  message(STATUS "SSL LIBS:  ${LIB_SSL}")
  message(STATUS "ZLIB LIBS: ${LIB_ZLIB}")
elseif (APPLE)
  message("==> Compiling for OSX...")
  message("TODO: OSX support!")
  find_library(LIB_NFD nfd "${CMAKE_SOURCE_DIR}/libs/nfd/lib/osx/x64")
  find_library(COCOA_LIBRARY Cocoa)
  find_library(OpenGL_LIBRARY OpenGL)
  find_library(IOKIT_LIBRARY IOKit)
  find_library(COREVIDEO_LIBRARY CoreVideo)
  set(EXTRA_LIBS ${COCOA_LIBRARY} ${OpenGL_LIBRARY} ${IOKIT_LIBRARY} ${COREVIDEO_LIBRARY})
endif ()

# print found libraries
message(STATUS " =")
message(STATUS " ==")
message(STATUS " ===")
message(STATUS " === GLEW LIBS: ==> ${GLEW_LIBRARIES}")
message(STATUS " === GL LIBS:   ==> ${OPENGL_LIBRARIES}")
message(STATUS " === CURL LIBS: ==> ${LIB_CURL}")
message(STATUS " === NFD LIBS:  ==> ${LIB_NFD}")
message(STATUS " === GTK3 LIBS: ==> ${EXTRA_LIBS}")
message(STATUS " ===")
message(STATUS " ==")
message(STATUS " =")

# link astro lib
target_link_libraries(astro
  imgui
  swe
  datetz
  ${LIB_CURL}
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${LIB_NFD}
  ${EXTRA_LIBS}
  )

# link executable
message(STATUS "Building Astrolograph...")
add_executable(${TARGET} main.cpp)
set_target_properties(${TARGET} PROPERTIES CXX_STANDARD 17)
target_compile_options(${TARGET} PRIVATE -static -Wno-deprecated)
target_link_libraries(${TARGET}
  astro
  imgui
  glfw
  nlohmann_json::nlohmann_json
  ${LIB_CURL}
  # ${LIB_SSL} ${LIB_ZLIB} # <-- for Windows?
  ${LIB_NFD}
  ${EXTRA_LIBS}
  )
