// astrolograph -- nodegraph-based tool for viewing astrological data/charts
#include "version/version.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>

#include "astro.hpp"
#include "nodeGraph.hpp"
#include "nodeList.hpp"
#include "viewSettings.hpp"
#include "moonNode.hpp"

#define ENABLE_IMGUI_VIEWPORTS false
#define ENABLE_IMGUI_DOCKING   false

#define WINDOW_W    2400
#define WINDOW_H    1600

#define SIDEBAR_W   512

#define GL_MAJOR 4 // OpenGL 4.4
#define GL_MINOR 4

#define ENABLE_IMGUI_DEMO true

astro::NodeGraph *graph = nullptr;
bool closing = false; // set to true when program is being closed
bool saving = false;  // set to true when user saving project before close
bool noSave = false;  // set to true if unsaved changes should be discarded



// GLFW error callback
void glfw_error_callback(int error, const char* description)
{
  fprintf(stderr, "GLFW ERROR (%d) --> %s\n", error, description);
}

void windowCloseCallback(GLFWwindow *window)
{
  if(graph->unsavedChanges())
    {
      std::cout << "Asking to save changes!\n";
      glfwSetWindowShouldClose(window, GLFW_FALSE);
      closing = true;
    }
  else
    { std::cout << "No unsaved changes!\n"; }
}

struct KeyShortcut
{
  int mods;
  int key;
  std::function<void()> action;
};

int main(int argc, char* argv[])
{
  // process command line arguments
  bool argVersion = false;
  for(int i = 0; i < argc; i++)
    {
      const char *arg = argv[i];
      int argLen = strlen(arg);

      if(argLen > 2 && arg[0] == '-' && arg[1] == '-')
        { // long name argument
          std::string argStr = std::string(&arg[2]);
          if(argStr == "version")
            {
              argVersion = true;
            }
          else
            { // unknown command
              std::cout << "Error: Unknown command '--" << argStr << "'!\n";
              return 1;
            }
        }
      else if(argLen > 1 && arg[0] == '-')
        { // short name argument(s)
          for(int j = 1; j < argLen; j++)
            {
              switch(arg[j])
                {
                case 'v':
                  argVersion = true;
                  break;
              
                default: // unknown command
                  std::cout << "Error: Unknown command '-" << arg[j] << "'!\n";
                  return 1;
                }
            }
        }
    }

  if(argVersion)
    { // print version and exit
      std::cout << "\n"
                << "Astrolograph Version: v" << ASTROLOGRAPH_VERSION_MAJOR << "." << ASTROLOGRAPH_VERSION_MINOR << "\n\n";
      return 0;
    }
  
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
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

  // maximized window
  //glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
  
  // create window with graphics context
  GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "AstroloGraph", NULL, NULL);
  if(window == NULL) { return 1; }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0); // Enable vsync
  
  // get screen size
  GLFWmonitor       *monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode    = glfwGetVideoMode(monitor);
  glfwSetWindowPos(window, (mode->width - WINDOW_W)/2, (mode->height - WINDOW_H)/2);

  std::cout << "Screen Size: " << mode->width <<  "x" << mode->height << "\n";
  
  // initialize OpenGL loader
  bool err = glewInit() != GLEW_OK;
  if(err) { fprintf(stderr, "Failed to initialize OpenGL loader!\n"); return 1; }

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
  //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // enable docking
  //io.ConfigDockingWithShift = true;                      // docking when shift is held
#endif // ENABLE_IMGUI_DOCKING
  
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

  // imgui context init
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  glfwSetWindowCloseCallback(window, windowCloseCallback); // callback when closing window

  astro::ViewSettings viewSettings;
  graph = new astro::NodeGraph(&viewSettings);

  astro::NodeList list(graph);
  
  // Our state
  bool showDemo     = false;
  Vec4f clearColor  = Vec4f(0.05f, 0.05f, 0.05f, 1.0f);
  Vec2f menuBarSize;

  // keyboard shortcut definitions
  const std::vector<KeyShortcut> shortcuts =
    { //// System/Files
      { GLFW_MOD_CONTROL,                GLFW_KEY_N,      [](){ graph->clear(); } },            // CTRL+N       --> new file
      { GLFW_MOD_CONTROL,                GLFW_KEY_O,      [](){ graph->openLoadDialog(); } },   // CTRL+O       --> open file
      { GLFW_MOD_CONTROL,                GLFW_KEY_S,      [](){ graph->openSaveDialog(); } },   // CTRL+S       --> save file
      { GLFW_MOD_CONTROL|GLFW_MOD_SHIFT, GLFW_KEY_S,      [](){ graph->openSaveAsDialog(); } }, // CTRL+SHIFT+S --> save file as (rename)
      { GLFW_MOD_CONTROL,                GLFW_KEY_ESCAPE, [](){ closing = true; } },            // CTRL+ALT+Q   --> quit program
      { GLFW_MOD_ALT,                    GLFW_KEY_V,      [&viewSettings](){ viewSettings.toggleWindow(); } },    // CTRL+ALT+V --> toggle view settings
      //// Graph Control
      { GLFW_MOD_CONTROL, GLFW_KEY_X, [](){ graph->cut(); } },                          // CTRL+X       --> cut
      { GLFW_MOD_CONTROL, GLFW_KEY_C, [](){ graph->copy(); } },                         // CTRL+C       --> copy
      { GLFW_MOD_CONTROL, GLFW_KEY_V, [](){ graph->paste(); } },                        // CTRL+V       --> paste
      { GLFW_MOD_CONTROL, GLFW_KEY_A, [](){ graph->selectAll(); } },                // CTRL+A       --> select all
      //// Node Creation
      { 0, GLFW_KEY_T, [](){ graph->addNode("TimeNode",         true); } }, // T --> new Time Node
      { 0, GLFW_KEY_S, [](){ graph->addNode("TimeSpanNode",     true); } }, // S --> new Time Span Node
      { 0, GLFW_KEY_L, [](){ graph->addNode("LocationNode",     true); } }, // L --> new Location Node
      { 0, GLFW_KEY_C, [](){ graph->addNode("ChartNode",        true); } }, // C --> new Chart Node
      { 0, GLFW_KEY_P, [](){ graph->addNode("ProgressNode",     true); } }, // P --> new Progress Node
      { 0, GLFW_KEY_V, [](){ graph->addNode("ChartViewNode",    true); } }, // V --> new Chart View Node
      { 0, GLFW_KEY_X, [](){ graph->addNode("ChartCompareNode", true); } }, // X --> new Chart Compare Node
      { 0, GLFW_KEY_D, [](){ graph->addNode("ChartDataNode",    true); } }, // D --> new Chart Data Node
      { 0, GLFW_KEY_A, [](){ graph->addNode("AspectNode",       true); } }, // A --> new Aspect Node
      { 0, GLFW_KEY_M, [](){ graph->addNode("MoonNode",         true); } }, // M --> new Moon Node
      //// Debug
      { GLFW_MOD_ALT, GLFW_KEY_D, [&showDemo](){ showDemo = !showDemo; } }, // ALT+D --> open ImGui demo window
    };
  
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

      ImGui::PushFont(viewSettings.mainFont);
      // get GLFW window size
      glfwGetFramebufferSize(window, &frameSize.x, &frameSize.y);
            
      //// KEY SHORTCUTS ////
      ImGuiIO& io   = ImGui::GetIO();
      int      mods = ((io.KeyCtrl  ? GLFW_MOD_CONTROL : 0) |
                       (io.KeyAlt   ? GLFW_MOD_ALT     : 0) |
                       (io.KeyShift ? GLFW_MOD_SHIFT   : 0) |
                       (io.KeySuper ? GLFW_MOD_SUPER   : 0));
      for(auto s : shortcuts)
        {
          if(mods == s.mods && ImGui::IsKeyPressed(s.key))// && !ImGui::GetIO().WantCaptureKeyboard)
            { s.action(); }
        }
      
      //// MENU BAR ////
      if(ImGui::BeginMainMenuBar())
        {
          if(ImGui::BeginMenu("File"))
            {
              if(ImGui::MenuItem("New"))     { graph->clear(); }
              if(ImGui::MenuItem("Open"))    { graph->openLoadDialog(); }
              if(ImGui::MenuItem("Save"))    { graph->openSaveDialog(); }
              if(ImGui::MenuItem("Save As")) { graph->openSaveAsDialog(); }
              if(ImGui::MenuItem("Exit"))    { closing = true; }
              ImGui::EndMenu();
            }
          if(ImGui::BeginMenu("Edit"))
            {
              if(ImGui::MenuItem("Cut"))   { graph->cut(); }
              if(ImGui::MenuItem("Copy"))  { graph->copy(); }
              if(ImGui::MenuItem("Paste")) { graph->paste(); }
              if(ImGui::BeginMenu("Add Node"))
                {
                  for(const auto &gIter : astro::NodeGraph::NODE_GROUPS)
                    {
                      if(ImGui::BeginMenu(gIter.name.c_str()))
                        {
                          for(const auto &type : gIter.types)
                            {
                              auto nIter = astro::NodeGraph::NODE_TYPES.find(type);
                              if(nIter != astro::NodeGraph::NODE_TYPES.end())
                                {
                                  if(ImGui::MenuItem(nIter->second.name.c_str()))
                                    { graph->addNode(nIter->second.get()); }
                                }
                            }
                          ImGui::EndMenu(); // gIter.name
                        }
                    }
                  ImGui::EndMenu(); // Add Node
                }
              ImGui::EndMenu(); // Edit
            }
          
          if(ImGui::BeginMenu("View"))
            {
              if(ImGui::MenuItem("Settings"))
                {
                  viewSettings.openWindow();
                }
              ImGui::EndMenu(); // View
            }
          ImGui::EndMainMenuBar();
        }
      menuBarSize = Vec2f(ImGui::GetItemRectMax()) - Vec2f(ImGui::GetItemRectMin());
      

      //// DRAWING ////
      bool settingsOpen = viewSettings.draw(frameSize);
      ImGuiWindowFlags wFlags = (ImGuiWindowFlags_NoTitleBar        |
                                 ImGuiWindowFlags_NoCollapse        |
                                 ImGuiWindowFlags_NoMove            |
                                 ImGuiWindowFlags_NoScrollbar       |
                                 ImGuiWindowFlags_NoScrollWithMouse |
                                 ImGuiWindowFlags_NoResize          |
                                 ImGuiWindowFlags_NoSavedSettings   |
                                 ImGuiWindowFlags_NoBringToFrontOnFocus
                                 );
      ImGui::SetNextWindowPos(Vec2f(0,menuBarSize.y));
      ImGui::SetNextWindowSize(Vec2f(frameSize.x, frameSize.y - menuBarSize.y));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0); // square frames by default
      ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  0);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(0, 0));
      ImGui::PushStyleColor(ImGuiCol_WindowBg, Vec4f(0,0,0,1));
      ImGui::Begin("##mainView", nullptr, wFlags);
      ImGui::PopStyleColor(1);
      ImGui::PopStyleVar(3);
      {
        static const Vec2f framePadding(5,5);
        static const int listWidth = 512;
        Vec2f graphPos = Vec2f(0.0f, menuBarSize.y);
        
        graph->setPos(graphPos + framePadding);
        graph->setSize(Vec2f(frameSize.x - 3*framePadding.x - listWidth, frameSize.y - menuBarSize.y - 2*framePadding.y));
        graph->showIds(showDemo);
        graph->draw();

        // graph->update(); // TEMP: currently called in graph->draw().  TODO: separate thread?
        
        list.setPos(Vec2f(frameSize.x - framePadding.x - listWidth, graphPos.y + framePadding.y));
        list.setSize(Vec2f(listWidth, frameSize.y - menuBarSize.y - 2*framePadding.y));
        list.draw();
      }
      ImGui::End();

      // unsaved changes popup (TODO: improve/fix)
      static bool popupOpen = false;
      if(closing && !saving)
        { popupOpen = true; ImGui::OpenPopup("Unsaved Changes"); }
      if(ImGui::BeginPopupModal("Unsaved Changes", &popupOpen, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
        {
          // ImGui::SetWindowFocus();
          if(graph->getSaveName().empty())
            { ImGui::TextUnformatted("Unsaved project has been modified. Do you want to save?"); }
          else
            { ImGui::Text("Project '%s' has been modified. Do you want to save?", graph->getSaveName().c_str()); }

          ImGui::SetNextItemWidth(50);
          if(ImGui::Button("Yes")) // save first
            {
              closing = true;
              saving = true;
              graph->openSaveDialog();
              ImGui::CloseCurrentPopup();
            }
          ImGui::SameLine();
          ImGui::SetNextItemWidth(50);
          if(ImGui::Button("No")) // close immediately
            {
              closing = true;
              noSave = true;
              ImGui::CloseCurrentPopup();
            }
          ImGui::SameLine();
          ImGui::SetNextItemWidth(50);
          if(ImGui::Button("Cancel"))
            {
              closing = false;
              ImGui::CloseCurrentPopup();
            }
          ImGui::EndPopup();
        }
      else
        { popupOpen = false; }

#if ENABLE_IMGUI_DEMO
      // show demo window if toggled
      if(showDemo) { ImGui::ShowDemoWindow(&showDemo); }
#endif

      graph->setLocked(settingsOpen || popupOpen || graph->saveOpen() || graph->loadOpen() || graph->isSaving() || graph->isLoading());
      
      if(closing && saving && graph->unsavedChanges() && !graph->saveOpen()) // save cancelled
        { } // closing = false; saving = false; }
      else if(closing && (!graph->unsavedChanges() || noSave)) // close window
        { glfwSetWindowShouldClose(window, GLFW_TRUE); }
      
      ImGui::PopFont();
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

  astro::MoonNode::cleanShaders();

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  delete graph;

  std::cout << "Done\n";  
  return 0;
}
