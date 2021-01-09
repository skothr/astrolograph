cmake_minimum_required(VERSION 3.6)
include_directories(.)
include_directories(../../inc) # for vector.hpp
add_library(imgui STATIC
  # imconfig.h
  # imgui.h
  # imgui_impl_glfw.h
  # imgui_impl_opengl3.h
  # imgui_internal.h
  # imstb_rectpack.h
  # imstb_textedit.h
  # imstb_truetype.h
  # ImGuiFileBrowser.h  
  imgui.cpp
  imgui_impl_glfw.cpp
  imgui_impl_opengl3.cpp
  imgui_demo.cpp
  imgui_draw.cpp
  imgui_widgets.cpp
  ImGuiFileBrowser.cpp
  )
set_target_properties(imgui PROPERTIES CXX_STANDARD 17)
target_compile_definitions(imgui PRIVATE GLEW_STATIC IMGUI_IMPL_OPENGL_LOADER_GLEW FLOOR_IMGUI_POSITIONS=0)
target_compile_options(imgui PRIVATE -O0)
