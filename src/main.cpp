// astrolograph -- nodegraph-based tool for viewing astrological data/charts
#include "version/version.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <chrono>
#include <stdio.h>
#include <ctype.h>

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
#define GL_MINOR 5
#define GLSL_VERSION "#version 450"

#define SIDEBAR_W 512

// GLFW error callback
void glfw_error_callback(int error, const char* description)
{ std::cerr << "GLFW ERROR (" << error << ") --> " << description << "\n"; }

char toLower(char c) { return std::tolower(c); }


void printChart(astro::Chart &chart, const std::string &name, astro::DateTime &dt, astro::Location &loc, bool debug);

int main(int argc, char* argv[])
{
  ArgParser parser({ new Argument<bool>           ("help",       'h',  "Print this message and exit."),
                     new Argument<bool>           ("version",    'v',  "Print version number and exit."),
                     // command-line calculations
                     new Argument<bool>           ("findAspect", '\0', "Command --> find next specified aspect (single command-line calculation)."),
                     new Argument<bool>           ("findRx",     'r',  "Command --> find retrograde stations of object(s) (single command-line calculation)."),
                     new Argument<bool>           ("position",   'p',  "Command --> print object position(s)."),
                     new Argument<bool>           ("chart",      'c',  "Command --> Print all positions in chart."),
                     // calculation parameters
                     new Argument<std::string>    ("file",       'f',  "Specify CSV file with bulk data for --chart."),
                     new Argument<std::string>    ("output",     'o',  "Specify CSV file to output data for --chart."),
                     new Argument<bool>           ("debug",      'd',  "Print debug info during calculation."),
                     new Argument<astro::DateTime>("time",       't',  "Sets date/time for single command-line calculation."),
                     new Argument<astro::DateTime>("time2",      '\0', "Sets second date/time for single command-line calculation."),
                     new Argument<astro::Location>("location",   'l',  "Sets location for single command-line calculation."),
                     new Argument<std::string>    ("obj1",       '1',  "Defines first object for single command-line calculation."),
                     new Argument<std::string>    ("obj2",       '2',  "Defines second object for single command-line calculation."),
                     new Argument<std::string>    ("aspect",     'a',  "Defines aspect type for single command-line calculation."),
                     new Argument<double>         ("error",      'e',  "Defines maximum error for single command-line calculation."),
                     new Argument<double>         ("minRange",   'm',  "Defines minimum time range to check for single command-line calculation."),
    });
  if(!parser.parse(argc, argv)) { std::cerr << "Failed to parse arguments!\n\n"; parser.printHelp(); return 1; }
  
  bool argVersion = parser.getValue<bool>("version");
  bool argHelp    = parser.getValue<bool>("help");
  bool argDebug   = parser.getValue<bool>("debug");
  
  if(argDebug) { parser.printArgs(); }
  
  bool quit = false;
  if(argVersion || argHelp)
    { // print version and exit
      std::cout << "\n" << "Astrolograph Version: v" << ASTROLOGRAPH_VERSION_MAJOR << "." << ASTROLOGRAPH_VERSION_MINOR << "\n\n";
      quit = true;
    }
  if(argHelp)
    { // print help message and exit
      parser.printHelp();
      quit = true;
    }
  
  
  bool success = true;
  if(parser.getValue<bool>("findAspect"))
    {
      astro::DateTime argDt = astro::DateTime::now();
      if(parser.isValid("time"))     { argDt   = parser.getValue<astro::DateTime>("time"); } 
      astro::Location argLoc;
      if(parser.isValid("location")) { argLoc  = parser.getValue<astro::Location>("location"); } 
      std::string     argObj1 = "";
      if(parser.isValid("obj1"))     { argObj1 = parser.getValue<std::string>("obj1"); }
      std::string     argObj2 = "";
      if(parser.isValid("obj2"))     { argObj2 = parser.getValue<std::string>("obj2"); }
      std::string     argAsp  = "";
      if(parser.isValid("aspect"))   { argAsp  = parser.getValue<std::string>("aspect"); }
      
      std::transform(argObj1.begin(), argObj1.end(), argObj1.begin(), toLower);
      std::transform(argObj2.begin(), argObj2.end(), argObj2.begin(), toLower);
      std::transform(argAsp.begin(),  argAsp.end(),  argAsp.begin(),  toLower);

      std::cout << "===================================================\n";
      std::cout << "=== PARAMETERS -->\n";
      std::cout << "= TIME:     " << argDt   << "\n";
      std::cout << "= LOCATION: " << argLoc  << "\n";
      std::cout << "= OBJECT 1: " << argObj1 << "\n";
      std::cout << "= OBJECT 2: " << argObj2 << "\n";
      std::cout << "= ASPECT:   " << argLoc  << "\n";
      std::cout << "===================================================\n";

      astro::ObjType     o1    = astro::getObjId(argObj1);
      astro::ObjType     o2    = astro::getObjId(argObj2);
      astro::AspectInfo *aInfo = astro::getAspectInfo(argAsp);
      if(o1 == astro::OBJ_INVALID && o2 == astro::OBJ_INVALID)
        { std::cout << "ERROR: Invalid value for " << (o1 == astro::OBJ_INVALID ? "--obj1" : "--obj2") << "!\n";   success = false; }
      if(!aInfo)
        { std::cout << "ERROR: Invalid value for --aspect!\n"; success = false; }

      astro::AspectType a  = (aInfo ? astro::getAspectInfo(argAsp)->type : astro::ASPECT_CONJUNCTION);
      
      astro::Chart chart(argDt, argLoc);
      chart.update();
      chart.setDebug(argDebug);
      
      std::cout << "\n";
      if(o1 != astro::OBJ_INVALID && o2 != astro::OBJ_INVALID)
        {
          double error = 0.0;
          int numSteps = 0;
          auto t0 = std::chrono::system_clock::now();
          astro::DateTime suPeak = chart.findAspectPeak(argDt, argDt, o1, o2, a, 1.0/3600.0/10.0, &error, &numSteps);
          auto t1 = std::chrono::system_clock::now();
          std::cout << "Next peak (" << std::setprecision(8) << std::setw(20) << std::left
                    <<  argObj1<< " / "  << std::setw(20) << argObj2
                    << " " << std::setw(20) << argAsp << ")" << " -->  " << suPeak << " (steps: " << std::right << std::setw(4) << numSteps
                    << ", error: " << angle_string(error) << " / " << error
                    << ", dt: " << (std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count()/1000000000.0) << "s)\n";
          if(numSteps >= MAX_STEPS)
            { std::cout << "WARNING: astro::Chart::findAspectPeak() reached maximum number of steps (may be inaccurate -- check error)\n"; }
        }
      else if(success)
        {
          astro::ObjType o = (o1 != astro::OBJ_INVALID ? o1 : o2);
          double error = 0.0;
          int numSteps = 0;
          for(int oo = 0; oo < astro::OBJ_END; oo++)
            {
              if(o == oo) { continue; }
              auto t0 = std::chrono::system_clock::now();
              astro::DateTime suPeak = chart.findAspectPeak(argDt, argDt, o, oo, a, 1.0/3600.0/10.0, &error, &numSteps);
              auto t1 = std::chrono::system_clock::now();
              std::cout << "Next peak (" << std::setprecision(8) << std::left
                        << std::setw(20) << astro::getObjName(o) << " / "  << std::setw(20) << astro::getObjName(oo)
                        << " " << std::setw(20) << argAsp << ")" << " -->  " << suPeak << " (steps: " << std::right << std::setw(4) << numSteps
                        << ", error: " << angle_string(error) << " / " << error
                        << ", dt: " << (std::chrono::duration_cast<std::chrono::nanoseconds>(t1-t0).count()/1000000000.0) << "s)\n";
              if(numSteps >= MAX_STEPS)
                { std::cout << "WARNING: astro::Chart::findAspectPeak() reached maximum number of steps (may be inaccurate -- check error)\n"; }
            }
        }
      std::cout << "\n";
      quit = true;
    }
  else if(parser.getValue<bool>("position"))
    {
      astro::DateTime argDt = astro::DateTime::now();
      if(parser.isValid("time"))     { argDt = parser.getValue<astro::DateTime>("time"); } 
      astro::Location argLoc; // default --> NYSE
      if(parser.isValid("location")) { argLoc = parser.getValue<astro::Location>("location"); } 
      std::string     argObj1 = "";
      if(parser.isValid("obj1"))     { argObj1 = parser.getValue<std::string>("obj1"); }
      std::string     argObj2 = "";
      if(parser.isValid("obj2"))     { argObj2 = parser.getValue<std::string>("obj2"); }
      
      std::transform(argObj1.begin(), argObj1.end(), argObj1.begin(), toLower);
      std::transform(argObj2.begin(), argObj2.end(), argObj2.begin(), toLower);

      std::cout << "===================================================\n";
      std::cout << "=== PARAMETERS -->\n";
      std::cout << "= TIME:     " << argDt   << "\n";
      std::cout << "= LOCATION: " << argLoc  << "\n";
      std::cout << "= OBJECT 1: " << argObj1 << "\n";
      std::cout << "= OBJECT 2: " << argObj2 << "\n";
      std::cout << "===================================================\n";

      astro::ObjType o1 = astro::getObjId(argObj1);
      astro::ObjType o2 = astro::getObjId(argObj2);
      
      astro::Chart chart(argDt, argLoc);
      chart.setDebug(argDebug);
      chart.update();
      if(o1 != astro::OBJ_INVALID) { std::cout << std::left << std::setw(20) << astro::getObjName(o1) << "  -->  " << chart.getObjectData(o1)->longitude << "\n"; }
      if(o2 != astro::OBJ_INVALID) { std::cout << std::left << std::setw(20) << astro::getObjName(o2) << "  -->  " << chart.getObjectData(o2)->longitude << "\n"; }
      std::cout << "\n";
      quit = true;
    }
  else if(parser.getValue<bool>("chart"))
    { // print all position in a chart
      
      astro::DateTime argDt = astro::DateTime::now();
      astro::Location argLoc; // default --> NYSE
      std::string     outFile = "";

      if(parser.isValid("file"))
        {
          if(parser.isValid("output"))
            {
              outFile = parser.getValue<std::string>("output");
              std::cout << "==> output file: " << outFile << "\n";
            }

          std::string path = parser.getValue<std::string>("file");
          std::ifstream f(path);
          std::string line;
          int nameCol = -1;
          int dtCol   = -1;
          int locCol  = -1;
          bool header  = true;
          bool success = true;
          bool append  = false;
          // parse CSV
          while(std::getline(f, line, '\n'))
            {
              std::string noCommas = line; noCommas.erase(std::remove(noCommas.begin(), noCommas.end(), ','), noCommas.end());
              if(line.empty() || line == "\n" || noCommas.empty() || noCommas == "\n") { continue; }
              if(header)
                { // parse header
                  int i = 0;
                  std::string col;
                  std::stringstream ss(line);
                  while(std::getline(ss, col, ','))
                    {
                      std::transform(col.begin(), col.end(), col.begin(), [](unsigned char c) { return std::tolower(c); });
                      std::cout << "COL: " << col << "\n";
                      if(nameCol < 0 && col.find("name") != std::string::npos)             { nameCol = i; }
                      else if(dtCol < 0 && col.find("time") != std::string::npos)        { dtCol   = i; }
                      else if(locCol < 0 && col.find("coordinates") != std::string::npos) { locCol  = i; }
                      else if(col.empty()) { continue; }
                      i++;
                    }
                  if(nameCol < 0) { std::cout << "ERROR: Couldn't find name column in CSV!\n";     success = false; }
                  if(dtCol   < 0) { std::cout << "ERROR: Couldn't find time column in CSV!\n";     success = false; }
                  if(locCol  < 0) { std::cout << "ERROR: Couldn't find location column in CSV!\n"; success = false; }
                  std::cout << "nameCol=" << nameCol << ", " << "dtCol=" << dtCol << ", " << "locCol=" << locCol << "\n\n";
                  header = false;
                  if(!success) { break; }
                }
              else
                {
                  std::string col;
                  std::stringstream ss(line);
                  std::vector<std::string> columns;
                  while(std::getline(ss, col, ',')) { columns.push_back(col); }
                  
                  std::string name   = columns[nameCol];
                  std::string dtStr  = columns[dtCol];
                  std::string locStr = columns[locCol];

                  std::cout << "NAME:       " << name   << "\n";
                  std::cout << "DT STRING:  " << dtStr  << "\n";
                  std::cout << "LOC STRING: " << locStr << "\n\n";

                  astro::DateTime dt = astro::DateTime::now();
                  if(!dtStr.empty()) { dt = astro::DateTime(dtStr); }
                  astro::Location loc;
                  if(!locStr.empty()) { loc = astro::Location(locStr); }

                  // dt.fromSaveString(dtStr);
                  // loc.fromSaveString(locStr);

                  astro::Chart chart;
                  printChart(chart, name, dt, loc, argDebug);
                  std::cout << "================================================================\n";

                  if(parser.isValid("output"))
                    {
                      chart.outputToFile(outFile, name, append, (!dtStr.empty() && !locStr.empty()));
                      append = true;
                    }
                }
            }
        }
      else
        { // single calculation
          if(parser.isValid("time"))     { argDt  = parser.getValue<astro::DateTime>("time"); } 
          if(parser.isValid("location")) { argLoc = parser.getValue<astro::Location>("location"); }
          astro::Chart chart;
          printChart(chart, "", argDt, argLoc, argDebug);
        }
      std::cout << "\n";
      quit = true;
    }
  else if(parser.getValue<bool>("findRx"))
    {
      astro::DateTime argDt = astro::DateTime::now();
      if(parser.isValid("time"))     { argDt = parser.getValue<astro::DateTime>("time"); } 
      astro::DateTime argDt2 = astro::DateTime::now();
      if(parser.isValid("time2"))    { argDt2 = parser.getValue<astro::DateTime>("time2"); } 
      astro::Location argLoc; // default --> NYSE
      if(parser.isValid("location")) { argLoc = parser.getValue<astro::Location>("location"); } 
      std::string     argObj1 = "";
      if(parser.isValid("obj1"))     { argObj1 = parser.getValue<std::string>("obj1"); }
      std::string     argObj2 = "";
      if(parser.isValid("obj2"))     { argObj2 = parser.getValue<std::string>("obj2"); }
      double     argErr = 0.005;
      if(parser.isValid("error"))    { argErr = parser.getValue<double>("error"); }
      double argMinRange = (1.0/(24.0*60.0*20.0));
      if(parser.isValid("minRange")) { argMinRange = parser.getValue<double>("minRange"); }

      std::transform(argObj1.begin(), argObj1.end(), argObj1.begin(), toLower);
      std::transform(argObj2.begin(), argObj2.end(), argObj2.begin(), toLower);

      argLoc.updateTimezone();
      argDt.setUtcOffset(argLoc.utcOffset);
      argDt.setDstOffset(argLoc.dstOffset);
      argDt2.setUtcOffset(argLoc.utcOffset);
      argDt2.setDstOffset(argLoc.dstOffset);

      std::cout << "===================================================\n";
      std::cout << "=== PARAMETERS -->\n";
      std::cout << "= START TIME: " << argDt        << "\n";
      std::cout << "= END TIME:   " << argDt2       << "\n";
      std::cout << "= LOCATION:   " << argLoc       << "\n";
      std::cout << "= OBJECT 1:   " << argObj1      << "\n";
      std::cout << "= OBJECT 2:   " << argObj2      << "\n";
      std::cout << "= MAX ERROR:  " << argErr       << "\n";
      std::cout << "= MIN RANGE:  " << argMinRange  << "\n";
      std::cout << "===================================================\n";

      astro::ObjType o1 = astro::getObjId(argObj1);
      astro::ObjType o2 = astro::getObjId(argObj2);
      astro::Chart chart(argDt, argLoc); chart.setDebug(argDebug); chart.update();

      int    steps   = 0;
      // double error   = 0.0;
      // bool   success = false;

      std::vector<astro::ObjRx> stations;
      chart.findRxStations(argDt, argDt2, o1, stations, argErr, argMinRange, &steps, 0);
      
      if(stations.size() > 0)
        {
          std::cout << "\n Found Retrogrades:   (steps: " << steps << ")\n";
          for(auto &s : stations)
            { std::cout << "   ==> " << s.rxStation << "  -->  " << s.dxStation << "\n"; }
        }
      else { std::cout << "No retrogrades found!\n"; }
      std::cout << "\n";
      quit = true;
    }
  
  if(quit) { return (success ? 0 : 1); }
  
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
  const char* glsl_version = GLSL_VERSION;
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



void printChart(astro::Chart &chart, const std::string &name, astro::DateTime &dt, astro::Location &loc, bool debug)
{
  loc.updateTimezone();
  dt.setDstOffset(loc.utcOffset - loc.getTimezoneOffset(dt));
  dt.setUtcOffset(loc.utcOffset);
  std::cout << "\n"
            << "================================================================\n"
            << "==== " << name << "\n"
            << "================================================================\n\n";
  
  chart.setDate(dt);
  chart.setLocation(loc);
  chart.setDebug(debug);
  chart.update();

  double utcOffset = loc.utcOffset;
  double dstOffset = dt.dstOffset();
  std::cout << "    " << dt.toShortString(true, true, false)
            << " (UTC" << (utcOffset < 0 ? "" : "+") << (int)utcOffset << (dstOffset == 0.0 ? "" : " +DST") << ")\n"
            << "    " << loc.toString() << "\n";
      
  int maxLenObj = 0;
  for(int o = 0; o < astro::OBJ_END; o++)
    { maxLenObj = std::max(maxLenObj, (int)astro::getObjName(o).size()); }
  int maxLenAsp = 0;
  for(int a = 0; a < astro::ASPECT_COUNT; a++)
    { maxLenAsp = std::max(maxLenAsp, (int)astro::getAspectName(a).size()); }
  std::cout << "\n";
  
  std::cout << "    Angles:\n";
  for(int o = astro::ANGLE_OFFSET; o < astro::OBJ_END; o++)
    { std::cout << "      " << std::left << std::setw(7) << astro::getObjName(o) << " " << chart.getObjString(chart.getObjectData(o)->longitude) << "\n"; }
  std::cout << "\n";
  
  std::cout << "    Houses (" << astro::HOUSE_SYSTEM_NAMES[chart.getHouseSystem()] << "):\n";
  for(int h = 1; h <= 12; h++)
    {
      std::string hString = std::to_string(h);
      std::vector<std::string> rulers = astro::getSignRulers(chart.getSign(chart.getHouseCusp(h)));
      std::string rulerStr = ""; for(int i = 0; i < rulers.size()-1; i++) { rulerStr += rulers[i] + " / "; } rulerStr += rulers.back();
      
      if(h == 1)      { hString += "st"; } else if(h == 2) { hString += "nd"; } else if(h == 3) { hString += "rd"; } else { hString += "th"; }
      std::cout << "      " << std::left << std::setw(4) << hString << "    " << chart.getObjString(chart.getHouseCusp(h))
                << "  |  ruler: " << rulerStr << "\n";
    }
  std::cout << "\n";
  
  std::cout << "    Placements:\n";
  for(int o = astro::OBJ_SUN; o < astro::ANGLE_OFFSET; o++)
    {
      if(o != astro::OBJ_QUAOAR)
        {
          double angle = chart.getObjectData(o)->longitude;
          int house = chart.getHouse(angle);
          std::string hString = std::to_string(house);
          if(house == 1)      { hString += "st"; } else if(house == 2) { hString += "nd"; } else if(house == 3) { hString += "rd"; } else { hString += "th"; }
          std::cout << "      " << std::left << std::setw(maxLenObj+1) << astro::getObjName(o) << " " << chart.getObjString(angle) << "  ( "
                    << std::right << std::setw(4) << hString << " house )\n";
        }
    }
  std::cout << "\n";

  astro::ChartParams params;
  std::vector<astro::ChartAspect> aspects = chart.calcAspects(params, false);
  // sort aspects by orb
  std::sort(aspects.begin(), aspects.end(), [](const astro::ChartAspect &a, const astro::ChartAspect &b) -> bool
                                            {
                                              if(std::abs(a.orb - b.orb) < 0.001)
                                                { // differentiate by aspect type, then object types
                                                  if(a.type < b.type)      { return true;  }
                                                  else if(a.obj1 < b.obj1) { return true;  }
                                                  else if(a.obj2 < b.obj2) { return true;  }
                                                  else                     { return false; }
                                                }
                                              else // return smaller orb
                                                { return (a.orb < b.orb); }
                                            } );
  
  std::cout << "    Aspects (< 1°):\n";
  for(int i = 0; i < aspects.size(); i++)
    {
      astro::AspectType aspType = aspects[i].type;
      astro::ObjType o1Type = aspects[i].obj1->type;
      astro::ObjType o2Type = aspects[i].obj2->type;
      if(aspects[i].orb <= 1.0 && o1Type != astro::OBJ_QUAOAR && o2Type != astro::OBJ_QUAOAR &&
         !((o1Type == astro::OBJ_NORTHNODE && o2Type == astro::OBJ_SOUTHNODE) || (o1Type == astro::OBJ_SOUTHNODE && o2Type == astro::OBJ_NORTHNODE)))
        {
          std::string o1Name = astro::getObjName(o1Type);
          std::string o2Name = astro::getObjName(o2Type);
          std::string aspName = astro::getAspectName(aspType);
          std::cout << "      " << std::left << std::setw(maxLenObj+1) << o1Name << " "
                    << std::left << std::setw(maxLenAsp+1) << aspName << " "
                    << std::left << std::setw(maxLenObj+1) << o2Name
                    << " ( " << angle_string(aspects[i].orb, true, false, false, 1) << " )\n";
        }
    }
  std::cout << "\n";
}
