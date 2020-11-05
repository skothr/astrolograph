// astrolograph -- nodegraph-based tool for viewing astrological data/charts

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

#include "astro.hpp"
#include "nodeGraph.hpp"
#include "chart.hpp"
#include "chartView.hpp"
#include "chartNode.hpp"
#include "timeNode.hpp"
#include "locationNode.hpp"
#include "chartDataNode.hpp"
#include "aspectNode.hpp"

#include "date/tz.h"

#define WINDOW_W    2400
#define WINDOW_H    1600

#define SIDEBAR_W   512

#define FONT_PATH   "./res/fonts/UbuntuMono-R.ttf"
#define DEFAULT_FONT_HEIGHT 16.0f

#define GL_MAJOR 4 // OpenGL 4.4
#define GL_MINOR 4

astro::NodeGraph *graph = new astro::NodeGraph();
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
  glfwWindowHint(GLFW_MAXIMIZED, GL_TRUE);
  
  // create window with graphics context
  GLFWwindow* window = glfwCreateWindow(WINDOW_W, WINDOW_H, "AstroloGraph", NULL, NULL);
  if(window == NULL) { return 1; }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0); // Enable vsync

  // initialize OpenGL loader
  bool err = glewInit() != GLEW_OK;
  if(err) { fprintf(stderr, "Failed to initialize OpenGL loader!\n"); return 1; }

  // set up imgui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.IniFilename = NULL; // disable .ini file

  // set dark imgui style
  ImGui::StyleColorsDark();
  
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  glfwSetWindowCloseCallback(window, windowCloseCallback);

  // load fonts
  io.Fonts->AddFontFromFileTTF(FONT_PATH, DEFAULT_FONT_HEIGHT);

  // Our state
  bool showDemo     = false;
  ImVec4 clearColor = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);

  // keyboard shortcut definitions
  const std::vector<KeyShortcut> shortcuts =
    { //// System/Files
      { GLFW_MOD_CONTROL,                GLFW_KEY_N, [](){ graph->clear(); } },            // CTRL+N       --> new file
      { GLFW_MOD_CONTROL,                GLFW_KEY_O, [](){ graph->openLoadDialog(); } },   // CTRL+O       --> open file
      { GLFW_MOD_CONTROL,                GLFW_KEY_S, [](){ graph->openSaveDialog(); } },   // CTRL+S       --> save file
      { GLFW_MOD_CONTROL|GLFW_MOD_SHIFT, GLFW_KEY_S, [](){ graph->openSaveAsDialog(); } }, // CTRL+SHIFT+S --> save file as (rename)
      { GLFW_MOD_CONTROL|GLFW_MOD_ALT,   GLFW_KEY_Q, [&closing](){ closing = true; } },    // CTRL+ALT+Q   --> quit program
      //// Graph Control
      // { GLFW_MOD_CONTROL, GLFW_KEY_X, [](){ graph->cut(); } },                          // CTRL+X       --> cut
      // { GLFW_MOD_CONTROL, GLFW_KEY_C, [](){ graph->copy(); } },                         // CTRL+C       --> copy
      // { GLFW_MOD_CONTROL, GLFW_KEY_V, [](){ graph->paste(); } },                        // CTRL+V       --> paste
      //// Node Creation (CTRL+ALT+[key])
      { 0, GLFW_KEY_T, [](){ graph->addNode("TimeNode"); } },        // T --> new Time Node
      { 0, GLFW_KEY_S, [](){ graph->addNode("TimeSpanNode"); } },    // S --> new Time Span Node
      { 0, GLFW_KEY_L, [](){ graph->addNode("LocationNode"); } },    // L --> new Location Node
      { 0, GLFW_KEY_C, [](){ graph->addNode("ChartNode"); } },       // C --> new Chart Node
      { 0, GLFW_KEY_P, [](){ graph->addNode("ProgressNode"); } },    // P --> new Progress Node
      { 0, GLFW_KEY_V, [](){ graph->addNode("ChartViewNode"); } },   // V --> new Chart View Node
      { 0, GLFW_KEY_X, [](){ graph->addNode("ChartCompareNode");} }, // X --> new Chart Compare Node
      { 0, GLFW_KEY_D, [](){ graph->addNode("ChartDataNode"); } },   // D --> new Chart Data Node
      { 0, GLFW_KEY_A, [](){ graph->addNode("AspectNode"); } },      // A --> new Aspect Node
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

      // get GLFW window size
      glfwGetFramebufferSize(window, &frameSize.x, &frameSize.y);

      //// KEY SHORTCUTS ////
      ImGuiIO& io = ImGui::GetIO();
      int mods = ((io.KeyCtrl  ? GLFW_MOD_CONTROL : 0) |
                  (io.KeyAlt   ? GLFW_MOD_ALT     : 0) |
                  (io.KeyShift ? GLFW_MOD_SHIFT   : 0) |
                  (io.KeySuper ? GLFW_MOD_SUPER   : 0));
      for(auto s : shortcuts)
        {
          if(ImGui::IsKeyPressed(s.key))
            {
              if(mods == s.mods)
                { s.action(); }
            }
        }
      
      // menu bar
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
              // if(ImGui::MenuItem("Cut"))  { std::cout << "CUT -- TODO\n"; }
              // if(ImGui::MenuItem("Copy")) { std::cout << "COPY -- TODO\n"; }
              // if(ImGui::MenuItem("Paste")) { std::cout << "PASTE -- TODO\n"; }
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
          ImGui::EndMainMenuBar();
        }
      
      graph->draw(frameSize);
      
      // unsaved changes popup (TODO: improve/fix)
      if(closing && !saving)
        { ImGui::OpenPopup("Unsaved Changes"); }
      if(ImGui::BeginPopupModal("Unsaved Changes", NULL, ImGuiWindowFlags_NoMove))
        {
          if(graph->getSaveName().empty())
            { ImGui::TextUnformatted("project has been modified -- do you want to save?"); }
          else
            { ImGui::Text("%s has been modified -- do you want to save?", graph->getSaveName().c_str()); }
          
          if(ImGui::Button("Yes")) // save first
            {
              closing = true;
              saving = true;
              graph->openSaveAsDialog();
              ImGui::CloseCurrentPopup();
            }
          ImGui::SameLine();
          if(ImGui::Button("No")) // close immediately
            {
              closing = true;
              noSave = true;
              ImGui::CloseCurrentPopup();
            }
          ImGui::SameLine();
          if(ImGui::Button("Cancel"))
            {
              closing = false;
              ImGui::CloseCurrentPopup();
            }
          ImGui::EndPopup();
        }
      ImGui::EndFrame();
      
      if(closing && saving && graph->unsavedChanges() && !graph->saveOpen()) // save cancelled
        {
          // closing = false;
          // saving = false;
        }
      else if(closing && (!graph->unsavedChanges() || noSave)) // close window
        { glfwSetWindowShouldClose(window, GLFW_TRUE); }

      
      // show demo window if toggled
      if(showDemo) { ImGui::ShowDemoWindow(&showDemo); }

      
      //// RENDERING ////
      ImGui::Render();
      glViewport(0, 0, frameSize.x, frameSize.y);
      glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
      glfwSwapBuffers(window);
    }

  std::cout << "Cleaning...\n";

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
