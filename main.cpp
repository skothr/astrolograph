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
      glfwSetWindowShouldClose(window, GLFW_FALSE);
      closing = true;
    }
  else
    {
      std::cout << "No unsaved changes!\n";
    }
}

// struct KeyShortcut
// {
//   int key;
//   std::string keys;
// };

int main(int argc, char* argv[])
{
  // // tz test
  // auto t = date::make_zoned(date::current_zone(), std::chrono::system_clock::now());
  // std::cout << "TZ CURRENT TIMEZONE: " << t << "\n";

  // // astro::Location::getTimezoneCurl(astro::Location(NYSE_LAT, NYSE_LON, NYSE_ALT));
  // astro::Location test(NYSE_LAT, NYSE_LON, NYSE_ALT);
  // std::cout << "*TZ OFFSET (TODAY): " << test.getTimezoneOffset(astro::DateTime(2020, 11, 1, 11, 24, 0.0)) << "\n";
  // std::cout << "*TZ OFFSET (YESTERDAY): " << test.getTimezoneOffset(astro::DateTime(2020, 10, 1, 11, 24, 0.0)) << "\n";
  
  // set up window
  glfwSetErrorCallback(glfw_error_callback);
  if(!glfwInit())
    { return 1; }

  // decide GL+GLSL versions
#ifdef __APPLE__
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 440";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
  // GL 4.4 + GLSL 130
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

  graph = new astro::NodeGraph();
 
  // main loop
  while(!glfwWindowShouldClose(window))
    {
      // handle events
      glfwPollEvents();

      // start imgui frame
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      // get GLFW window size
      int windowW, windowH;
      glfwGetFramebufferSize(window, &windowW, &windowH);

      //// KEY SHORTCUTS ////
      ImGuiIO& io = ImGui::GetIO();
      if(io.KeyCtrl && io.KeyAlt && ImGui::IsKeyPressed(GLFW_KEY_Q)) // (CTRL+ALT+Q) -- EXIT
        { closing = true; }
      // add nodes (TODO: remove or refine shortcuts)
      if(io.KeyCtrl && ImGui::IsKeyPressed(GLFW_KEY_N)) // (CTRL+N) -- CREATE NEW CHART VIEW
        { graph->addNode(new astro::ChartNode()); }
      if(io.KeyCtrl && ImGui::IsKeyPressed(GLFW_KEY_M)) // (CTRL+M) -- CREATE NEW CHART DATA VIEW
        { graph->addNode(new astro::ChartDataNode()); }
      if(io.KeyCtrl && ImGui::IsKeyPressed(GLFW_KEY_D)) // (CTRL+D) -- CREATE NEW DATE NODE
        { graph->addNode(new astro::TimeNode()); }
      if(io.KeyCtrl && ImGui::IsKeyPressed(GLFW_KEY_L)) // (CTRL+L) -- CREATE NEW LOCATION NODE
        { graph->addNode(new astro::LocationNode()); }
      // debug/demo
      if(ImGui::IsKeyPressed(GLFW_KEY_D) && io.KeyAlt)  // (ALT+D)  -- SHOW IMGUI DEMO WINDOW
        { showDemo = !showDemo; }
      
      // show demo window if toggled
      if(showDemo) { ImGui::ShowDemoWindow(&showDemo); }
      
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
                  if(ImGui::MenuItem("Time Node"))
                    { graph->addNode(new astro::TimeNode()); }
                  if(ImGui::MenuItem("Location Node"))
                    { graph->addNode(new astro::LocationNode()); }
                  if(ImGui::MenuItem("Chart Node"))
                    { graph->addNode(new astro::ChartNode()); }
                  if(ImGui::MenuItem("Chart Data Node"))
                    { graph->addNode(new astro::ChartDataNode()); }
                  if(ImGui::MenuItem("Aspect Node"))
                    { graph->addNode(new astro::AspectNode()); }
                  ImGui::EndMenu();
                }
              ImGui::EndMenu();
            }
          ImGui::EndMainMenuBar();
        }
      graph->draw();

      // unsaved changes popup
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

      // Rendering
      ImGui::Render();
      glViewport(0, 0, windowW, windowH);
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
