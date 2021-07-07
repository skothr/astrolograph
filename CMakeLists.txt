cmake_minimum_required(VERSION 3.6)

cmake_policy(SET CMP0072 NEW)   # opengl policy (to get rid of warning)
set(OpenGL_GL_PREFERENCE GLVND) #

project(astrolograph VERSION 0.7) # project name and version
configure_file(inc/version/version.hpp.in ../inc/version/version.hpp)

set(TARGET astrolograph)
set(ASTRO_ENABLE_CUDA TRUE)                        # set to TRUE to enable CUDA
set(CUDA_TOOLKIT_ROOT_DIR "/usr/local/cuda-11.2/") # set to location of CUDA source
set(ASTRO_CUDA_ARCH       "sm_75")                 # set for specific graphics card

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-O0 -g -Wall -pg")             # debug
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O2 -Wall -ftree-vectorize") # release

if (WIN32) # Windows -- WIP
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/x86_64-w64-mingw32/include -L/usr/x86_64-w64-mingw32/lib")
endif (WIN32)


#### build internal libs ####
include_directories(
  inc/
  inc/base/
  inc/astro/
  inc/nn/
  inc/field/
  )
add_library(astro STATIC
  # base
  src/base/astroWindow.cpp
  src/base/fileDialog.cpp
  src/base/groupNode.cpp
  src/base/imtools.cpp
  src/base/labelNode.cpp
  src/base/marketDataNode.cpp
  src/base/node.cpp
  src/base/nodeConnector.cpp
  src/base/nodeGraph.cpp
  src/base/nodeList.cpp
  src/base/plotNode.cpp
  src/base/plotWidget.cpp
  src/base/settingForm.cpp
  src/base/shapeBuffer.cpp
  src/base/viewSettings.cpp
  # astro
  src/astro/aspectNode.cpp
  src/astro/astro.cpp
  src/astro/chartCompare.cpp
  src/astro/chart.cpp
  src/astro/chartDataNode.cpp
  src/astro/chartNode.cpp
  src/astro/chartView.cpp
  src/astro/chartViewNode.cpp
  src/astro/chartOrbWidget.cpp
  src/astro/chartParamWidget.cpp
  src/astro/compareNode.cpp
  src/astro/dateTime.cpp
  src/astro/ephemeris.cpp
  src/astro/location.cpp
  src/astro/locationNode.cpp
  src/astro/locationWidget.cpp
  src/astro/moonNode.cpp
  src/astro/timeNode.cpp
  src/astro/timeWidget.cpp
  src/astro/vedicNode.cpp
  src/astro/progressNode.cpp
  # nn
  src/nn/neuralNetNode.cpp
  # field  
  src/field/cutools.cpp
  src/field/fftNode.cpp
  src/field/fieldViewNode.cpp
  src/field/fluidNode.cpp
  src/field/hyperFluidNode.cpp
  src/field/mandelbrotNode.cpp
  src/field/modularFluidNode.cpp
  src/field/staticFieldNode.cpp
  
  src/field/fieldChannels.cpp
  src/field/fieldOperators.cpp
  src/field/fieldDelOperators.cpp
  )
set_target_properties(astro PROPERTIES CXX_STANDARD 17)
target_compile_definitions(astro PRIVATE GLEW_STATIC)
target_compile_options(astro PRIVATE -static -Wno-deprecated -ftree-vectorize)

# build CUDA components
set(CUDA_LIB "")
if(ASTRO_ENABLE_CUDA) # enable CUDA
  set(CUDACXX "nvcc")
  set(CMAKE_CUDA_COMPILER "${CUDA_TOOLKIT_ROOT_DIR}/bin/nvcc")
  set(CMAKE_CUDA_FLAGS "-g") # -std=c++17")
  add_definitions(-DENABLE_CUDA)
  enable_language(CUDA)
  if(NOT DEFINED CMAKE_CUDA_STANDARD)
    set(CMAKE_CUDA_STANDARD 11)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
  endif()
  
  # build cuda lib
  include_directories(inc/cuda src/cuda src/cuda/helper ${CUDA_TOOLKIT_ROOT_DIR}/include)
  add_library(astrocuda STATIC
    src/field/cutools.cpp
    src/cuda/cutools.cu
    src/cuda/render.cu
    src/cuda/mandelbrot.cu
    src/cuda/fft.cu
    src/cuda/fluid.cu
    src/cuda/hyperfluid2.cu
    src/cuda/hyperfluid3.cu
    src/cuda/raytrace.cu
    # operator kernels
    src/cuda/field-abs.cu
    src/cuda/field-log.cu
    src/cuda/field-exp.cu
    src/cuda/field-add.cu
    src/cuda/field-mult.cu
    src/cuda/field-curl.cu
    src/cuda/field-divergence.cu
    src/cuda/field-gradient.cu
    src/cuda/field-negate.cu  
    )
  set_target_properties(astrocuda PROPERTIES CXX_STANDARD 17)
  set(CUDA_LIBRARIES astrocuda)
  target_link_libraries(astro ${CUDA_LIBRARIES} cufft)
endif()


#### build external libs ####

set(GLFW_ROOT_DIR "libs/glfw")       # glfw dir
set(GLEW_ROOT_DIR "libs/glew-2.1.0") # glew dir

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
  libs/magic_enum/include
  libs/nfd/include
  )
add_subdirectory(libs)
# curl
set(LIB_CURL curl)
# opengl
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
link_libraries(${OPENGL_LIBRARY_DIRS})
# glew
set(GLEW_LIBRARIES glew)
# OS-dependent
set(EXTRA_LIBS "")

if (UNIX) # Linux (GTK)
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
  
elseif (WIN32) # Windows -- WIP
  message("==> Compiling for Windows...")
  message("TODO: Windows support!")
  find_library(LIB_NFD nfd "${CMAKE_SOURCE_DIR}/libs/nfd/lib/windows/x64")
  message(STATUS "SSL LIBS:  ${LIB_SSL}")
  message(STATUS "ZLIB LIBS: ${LIB_ZLIB}")
  
elseif (APPLE) # MacOS -- WIP
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
message(STATUS " === CUDA LIBS: ==> ${CUDA_LIBRARIES}")
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
  ${CUDA_LIBRARIES}
  )

# link executable
message(STATUS "Building Astrolograph...")
add_executable(${TARGET} src/main.cpp)
set_target_properties(${TARGET} PROPERTIES CXX_STANDARD 17)
target_compile_options(${TARGET} PRIVATE -static -Wno-deprecated)
target_link_libraries(${TARGET}
  astro
  imgui
  glfw
  nlohmann_json::nlohmann_json
  ${LIB_CURL}
  ${LIB_NFD}
  ${EXTRA_LIBS}
  ${CUDA_LIBRARIES}
  # ${LIB_SSL} ${LIB_ZLIB} # <-- for Windows?
  )
