// astrolograph -- nodegraph-based tool for viewing astrological data/charts
#include "version/version.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <chrono>

#include "argParser.hpp"
#include "astro.hpp"
#include "astroWindow.hpp"
#include "nodeGraph.hpp"
#include "nodeList.hpp"
#include "viewSettings.hpp"
#include "moonNode.hpp"

#define ENABLE_IMGUI_VIEWPORTS 0
#define ENABLE_IMGUI_DOCKING   0
#define START_MAXIMIZED        1
#define WINDOW_W 2400
#define WINDOW_H 1600
#define GL_MAJOR 4
#define GL_MINOR 4

#define SIDEBAR_W 512

// GLFW error callback
void glfw_error_callback(int error, const char* description)
{ std::cerr << "GLFW ERROR (" << error << ") --> " << description << "\n"; }

int main(int argc, char* argv[])
{
  ArgParser parser({ new Argument<bool>("help",    'h', "Print this message and exit."),
                     new Argument<bool>("version", 'v', "Print version number and exit."),
    });
  if(!parser.parse(argc, argv)) { std::cerr << "Failed to parse arguments!\n\n"; parser.printHelp(); return 1; }

  bool argVersion = parser.getValue<bool>("version");
  bool argHelp    = parser.getValue<bool>("help");

  bool quit = false;
  if(argVersion || argHelp)
    { // print version and exit
      std::cout << "\n" << "Astrolograph Version: v" << ASTROLOGRAPH_VERSION_MAJOR << "." << ASTROLOGRAPH_VERSION_MINOR << "\n\n";
      quit = true;
    }
  if(argHelp)
    {
      parser.printHelp();
      quit = true;
    }
  if(quit) { return 0; }
  
  // print project version
  std::cout << "================================\n"
            << "Astrolograph (v" << ASTROLOGRAPH_VERSION_MAJOR << "." << ASTROLOGRAPH_VERSION_MINOR << ")\n"
            << "================================\n\n";

  // set up window
  glfwSetErrorCallback(glfw_error_callback);
  if(!glfwInit())
    { return 1; }

  // decide GL+GLSL versions
#ifdef __APPLE__
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
  // GL 4.4 + GLSL 440
  const char* glsl_version = "#version 440";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif


#if START_MAXIMIZED // start window maximized
  glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
#endif // START_MAXIMIZED
  
  glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
  
  // create window with graphics context
  GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "AstroloGraph", NULL, NULL);
  if(window == NULL) { return 1; }

  // get screen size
  GLFWmonitor       *monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode    = glfwGetVideoMode(monitor);
  std::cout << "Screen Size: " << mode->width <<  "x" << mode->height << "\n";
  
#if !START_MAXIMIZED // center window on screen
  glfwSetWindowPos(window, (mode->width - WINDOW_W)/2, (mode->height - WINDOW_H)/2);
#endif // START_MAXIMIZED
  
  // set window icon
  GLFWimage *appIcon = (GLFWimage*)astro::loadImageData("res/icons/app-icon-64.png");
  if(appIcon->pixels) { glfwSetWindowIcon(window, 1, appIcon); }

  // initialize gl context  
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0); // Enable vsync
  if(glewInit() != GLEW_OK) { std::cout << "Failed to initialize OpenGL loader!\n"; return 1; }

  // create astro window before imgui setup to preserve GLFW callbacks
  astro::AstroWindow *astroWindow = new astro::AstroWindow(window);
  
  // set up imgui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark(); // dark style
  
  // imgui context config
  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;                              // disable .ini file
#if ENABLE_IMGUI_VIEWPORTS
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  io.ConfigViewportsNoTaskBarIcon = true;
#endif // ENABLE_IMGUI_VIEWPORTS
#if ENABLE_IMGUI_DOCKING
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // enable docking
  io.ConfigDockingWithShift = true;                      // docking when shift is held
#endif // ENABLE_IMGUI_DOCKING
  
  //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls (NOTE: enables escape to close popups)
  
  // start imgui context
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  astroWindow->init();
  
  // Our state
  Vec4f clearColor  = Vec4f(0.05f, 0.05f, 0.05f, 1.0f);
  Vec2f menuBarSize;

  // main loop
  Vec2i frameSize(WINDOW_W, WINDOW_H); // size of current frame
  while(!glfwWindowShouldClose(window))
    {
      // handle events
      glfwPollEvents();
      // start imgui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();
      // get frame size
      glfwGetFramebufferSize(window, &frameSize.x, &frameSize.y);
      
      astroWindow->draw(frameSize);
      astroWindow->update();
      ImGui::EndFrame();
      
      //// RENDERING ////
      glUseProgram(0);
      ImGui::Render();
      
      // Update and Render additional Platform Windows (if viewports enabled)
      if(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) { ImGui::UpdatePlatformWindows(); ImGui::RenderPlatformWindowsDefault(); }
      
      glViewport(0, 0, frameSize.x, frameSize.y);
      glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      glfwSwapBuffers(window);
    }

  std::cout << "Cleaning...\n";
  {
    ImGui::PopStyleColor(); // ImGuiStyleCol_NavHighlight
    astro::MoonNode::cleanShaders();
    if(astroWindow) { delete astroWindow; }
    if(appIcon)  { delete appIcon;  }
    
    // imgui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  std::cout << "Done\n";  
  return 0;
}
