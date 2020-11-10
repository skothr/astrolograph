cmake_minimum_required(VERSION 3.6)
include_directories(.)
include_directories(../../inc)
add_library(
        imgui STATIC
        # ../../inc/vector.hpp
        imconfig.h
        imgui.h
        imgui_impl_glfw.h
        imgui_impl_opengl3.h
        imgui_internal.h
        imstb_rectpack.h
        imstb_textedit.h
        imstb_truetype.h
        ImGuiFileBrowser.h
        
        imgui_impl_opengl3.cpp
        imgui.cpp
        imgui_impl_glfw.cpp
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_widgets.cpp
        ImGuiFileBrowser.cpp
)
# file(GLOB all_SRCS
#         "*.h"
#         "*.hpp"
#         "*.c"
#         "*.cpp" )
