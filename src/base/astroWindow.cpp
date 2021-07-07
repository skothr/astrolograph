#include "astroWindow.hpp"
using namespace astro;
#include <imgui.h>
#include <imgui_internal.h>
#include <nfd.h>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "version/version.hpp"
#include "tools.hpp"
#include "astroWindow.hpp"
#include "nodeGraph.hpp"
#include "nodeList.hpp"
#include "viewSettings.hpp"
#include "moonNode.hpp"
#include "fileDialog.hpp"
#include "setting.hpp"
#include "settingForm.hpp"

static AstroWindow *astroWin = nullptr; // TODO: remove global reference?
void AstroWindow::windowCloseCallback(GLFWwindow *window)
{
  astroWin->mClosing = true;
  if(astroWin->graph() && astroWin->graph()->unsavedChanges())
    {
      std::cout << "Unsaved changes!\n";
      glfwSetWindowShouldClose(window, GLFW_FALSE);
    }
  else { std::cout << "No unsaved changes!\n"; }
}
void AstroWindow::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  if(action == GLFW_PRESS || action == GLFW_RELEASE)
    { astroWin->keyPress(mods, key, (action == GLFW_PRESS)); }
}

void AstroWindow::keyPress(int mods, int key, bool press)
{
  if(key != GLFW_KEY_LEFT_CONTROL && key != GLFW_KEY_RIGHT_CONTROL && // (don't count modifiers as presses)
     key != GLFW_KEY_LEFT_SHIFT   && key != GLFW_KEY_RIGHT_SHIFT   &&
     key != GLFW_KEY_LEFT_ALT     && key != GLFW_KEY_RIGHT_ALT     &&
     key != GLFW_KEY_LEFT_SUPER   && key != GLFW_KEY_RIGHT_SUPER   &&
     key != GLFW_KEY_CAPS_LOCK    && key != GLFW_KEY_NUM_LOCK)
    {
      if(press) // key pressed -- add to sequence
        { mKeySequence.emplace_back(mods, key); }
      else
        { // key released --> clear sequence
          for(int i = 0; i < mKeySequence.size(); i++)
            {
              const KeyPress &k = mKeySequence[i];
              if(k.key == key)
                { // key released --> clear sequence after this key
                  mKeySequence.erase(mKeySequence.begin()+i, mKeySequence.end());
                  break;
                }
            }
        }
    }
}

#define ABIND(func)       std::bind(&AstroWindow::func, this)
#define ABINDV(func, ...) std::bind(&AstroWindow::func, this, __VA_ARGS__)
#define ABIND1(func)      std::bind(&AstroWindow::func, this, std::placeholders::_1)
#define ABIND2(func)      std::bind(&AstroWindow::func, this, std::placeholders::_1, std::placeholders::_2)

AstroWindow::AstroWindow(GLFWwindow *window)
  : mWindow(window)
{
  astroWin = this; // NOTE/TODO: only one window total allowed for now
  glEnable(GL_MULTISAMPLE); // enable antialiasing
  glfwSetWindowCloseCallback(mWindow, &AstroWindow::windowCloseCallback);  // callback when closing window
  glfwSetKeyCallback(mWindow, &keyCallback);                               // key event callback
  
  mViewSettings = new ViewSettings();
  mNodeList     = new NodeList(nullptr, mViewSettings);
  mFileDialog   = new FileDialog();
  newProject();
  
  mPopups =
    { Popup{ "Unsaved Projects",        Vec2f(0,0),     ABIND1(drawExitUnsavedAlert),
             ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize,
             ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse
            },
      Popup{ "Closing Unsaved Project", Vec2f(0,0),     ABIND1(drawClosingUnsavedAlert),
             ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize,
             ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse
            },
      Popup{ "About",                   Vec2f(512,256), ABIND1(drawAbout),
             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar,
             ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse,
             false,       // no child border
             false, false // fixed size
      },
      Popup{ "Key Bindings",            Vec2f(666,777), ABIND1(drawKeyBindings),
             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar,
             0, true,     // draw child border
             false, false // fixed size
      },
      Popup{ "View Settings",           Vec2f(550,768), ABIND1(drawViewSettings),
             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar,
             0, true,     // draw child border
             false, false // fixed size
      },
    };
  for(auto &p : mPopups) { mPopupMap.emplace(p.name, &p); }

  mDefaultKeyBindings =
    { //// System/Files
     KeyBinding("New Project",                           "Ctrl+N",       "Creates a new project",
                ABIND(newProject)),
     KeyBinding("Open Project",                          "Ctrl+O",       "Opens an existing project file",
                ABIND(projectOpen)),
     KeyBinding("Save Project",                          "Ctrl+S",       "Immediately saves current project",
                ABIND(projectSave)),
     KeyBinding("Save Project As",                       "Ctrl+Shift+S", "Saves the current project to a file",
                ABIND(projectSaveAs)),
     KeyBinding("Prev Project",                          "Alt+Left",     "Switch one project tab to the left",
                ABIND(prevProject)),
     KeyBinding("Next Project",                          "Alt+Right",    "Switch one project tab to the right",
                ABIND(nextProject)),
     KeyBinding("Exit",                                  "Ctrl+Escape",  "Quits program",
                ABIND(quit)),
     KeyBinding("Cancel",                                "Escape",       "Cancels current action, or closes popup window",
                ABIND(escape)),
     //// Popups
     KeyBinding("View Settings",                         "Alt+V",        "Open view settings window",
                ABINDV(togglePopup, "View Settings")),
     KeyBinding("Key Bindings",                          "Alt+K",        "Open key binding window",
                ABINDV(togglePopup, "Key Bindings")),
     KeyBinding("About",                                 "Alt+A",        "Open 'about' window",
                ABINDV(togglePopup, "About")),
     //// Graph
     KeyBinding("Cut",                                   "Ctrl+X",       "Cuts selected nodes",
                ABIND(cut)),
     KeyBinding("Copy",                                  "Ctrl+C",       "Copies selected nodes",
                ABIND(copy)),
     KeyBinding("Paste",                                 "Ctrl+V",       "Starts pasting node clipboard into graph",
                ABIND(startPasting)),
     KeyBinding("Undo",                                  "Ctrl+Z",       "Undoes last graph action",
                ABIND(undo)),
     KeyBinding("Redo",                                  "Ctrl+Shift+Z", "Redoes last graph action",
                ABIND(redo)),
     KeyBinding("Select All",                            "Ctrl+A",       "Selects all nodes in graph",
                ABIND(selectAll)),
     KeyBinding("Quit Placing",                          "Q",            "Stops placing or pasting any nodes",
                ABIND(quitPlacing)),
     KeyBinding("Group Nodes",                           "Ctrl+G",       "Groups selected nodes into a single composite node",
                ABIND(groupNodes)),
     KeyBinding("Ungroup Nodes",                         "Ctrl+Shift+G", "Explodes group node into its components",
                ABIND(ungroupNodes)),
     //// Adding Nodes
     KeyBinding("Add Label Node",                        "Z",            "Basic text display for organization",
                ABINDV(startPlacing, "LabelNode")),
     KeyBinding("Add Time Node",                         "T",            "Defines a date and time",
                ABINDV(startPlacing, "TimeNode")),
     KeyBinding("Add Time Shift Node",                   "Shift+T",      "Offsets a date/time",
                ABINDV(startPlacing, "TimeShiftNode")),
     KeyBinding("Add Time Span Node",                    "S",            "Defines a date/time range",
                ABINDV(startPlacing, "TimeSpanNode")),
     KeyBinding("Add Location Node",                     "L",            "Defines a location on earth",
                ABINDV(startPlacing, "LocationNode")),
     KeyBinding("Add Chart Node",                        "C",            "Links a date and time to the Swiss Ephemeris",
                ABINDV(startPlacing, "ChartNode")),
     KeyBinding("Add Progress Node",                     "P",            "Calculates secondary progressions",
                ABINDV(startPlacing, "ProgressNode")),
     KeyBinding("Add Chart View Node",                   "V",            "Displays chart data",
                ABINDV(startPlacing, "ChartViewNode")),
     KeyBinding("Add Chart Compare Node",                "X",            "Displays comparison between two charts",
                ABINDV(startPlacing, "ChartCompareNode")),
     KeyBinding("Add Chart Data Node",                   "D",            "Displays in-depth chart position data",
                ABINDV(startPlacing, "ChartDataNode")),
     KeyBinding("Add Aspect Node",                       "A",            "Lists aspects within a chart",
                ABINDV(startPlacing, "AspectNode")),
     KeyBinding("Add Moon Node",                         "M",            "Shows current phase of the moon for a chart",
                ABINDV(startPlacing, "MoonNode")),
     KeyBinding("Add Plot Node",                         "O",            "Plots positions over time and show retrogrades",
                ABINDV(startPlacing, "PlotNode")),
     KeyBinding("Add Market Data Node",                  "J",            "Loads market data for a ticker",
                ABINDV(startPlacing, "MarketDataNode")),

     // Nerual Net
     KeyBinding("Add Neural Net Node",                   "N",            "Simulates a neural network (WIP)",
                ABINDV(startPlacing, "NeuralNetNode")),

     // fields/fluids
     KeyBinding("Add Field View Node",                   "Shift+V",      "Displays the contents of a field",
                ABINDV(startPlacing, "FieldViewNode")),
     KeyBinding("Add Field Channel View Node",           "Ctrl+Shift+V", "Displays one field per color channel",
                ABINDV(startPlacing, "FieldChannelViewNode" )),
     KeyBinding("Add FFT Node",                          "E",            "Calculates Fourier transform of a field",
                ABINDV(startPlacing, "FFTNode")),
     KeyBinding("Add Fluid Node",                        "F",            "Simulates a vector field fluid",
                ABINDV(startPlacing, "FluidNode")),
     KeyBinding("Add HyperFluid Node",                   "Shift+F",      "Test for higher-dimensional fluids (WIP)",
                ABINDV(startPlacing, "HyperFluidNode")),
     KeyBinding("Add Mandelbrot Node",                   "Shift+M",      "Calculates the Mandelbrot Set",
                ABINDV(startPlacing, "MandelbrotNode")),

     // field operators
     KeyBinding("Add Field Add Node",                    "Shift+=",      "Adds two fields together (+)",
                ABINDV(startPlacing, "FieldAddNode")),
     KeyBinding("Add Field Abs Node",                    "Shift+\\",     "Magnitude of field (|)",
                ABINDV(startPlacing, "FieldAbsNode")),
     KeyBinding("Add Field Mult Node",                   "Shift+8",      "Multiplies two fields together (*)",
                ABINDV(startPlacing, "FieldMultNode")),
     KeyBinding("Add Field Neg Node",                    "-",            "Negates field (-)",
                ABINDV(startPlacing, "FieldNegNode")),
     KeyBinding("Add Field Max Node",                    "Shift+.",      "Scales field by maximum magnitude",
                ABINDV(startPlacing, "FieldMaxNode")),
     KeyBinding("Add Field Norm Node",                   "Shift+N",      "Scales field by average magnitude",
                ABINDV(startPlacing, "FieldNormNode")),

     // ∇
     KeyBinding("Add Field Gradient Node",               "Shift+G",      "Finds gradient of field over X and Y",
                ABINDV(startPlacing, "FieldGradNode")),
     //KeyBinding("Add Static Field",                      "Shift+S",      "Filled with static shape or pattern",
     //           ABINDV(startPlacing, "StaticFieldNode")),
     //KeyBinding("Add Fluid State Node",                  "Shift+S",      "State of a fluid field for simulation",
     //           ABINDV(startPlacing, "FluidStateNode")),
     //KeyBinding("Add Field Divergence Node",             "Shift+D",      "Finds divergence of field",
     //           ABINDV(startPlacing, "FieldDivNode")),
     
     //// Debug
     KeyBinding("Show ImGui Demo",                       "Alt+D",        "Toggles ImGui demo window (with widget examples, style editor, metrics, etc.)",
                [&](){ mShowDemo = !mShowDemo; } )
    };
  mKeyBindings = mDefaultKeyBindings;
  for(auto k : mKeyBindings) { if(k.sequence.size() > 0 && k.name == "Cancel") { mCancelKey = k.sequence.back().key; } } // get cancel key

  // key binding groups  
  mKeyBindingGroups =
    { {"Global Bindings", { "New Project", "Open Project", "Save Project", "Save Project As",
                            "Prev Project", "Next Project", "Exit", "Cancel" },
       {}},
      {"Popups",          { "View Settings", "Key Bindings", "About" },
       {}},
      {"Graph Bindings",  { "Cut", "Copy", "Paste", "Undo", "Redo", "Select All",
                            "Group Nodes", "Ungroup Nodes", "Quit Placing" },
       {}},
    };
  // key bindings to add node types for each node group
  for(auto &g : NodeGraph::NODE_GROUPS)
    {
      KeyBindingGroup kbg { g.name + " Nodes", { }, { } };
      for(auto &t : g.types)
        {
          auto iter = NodeGraph::NODE_TYPES.find(t);
          if(iter != NodeGraph::NODE_TYPES.end())
            {
              std::string kbName = "Add " + iter->second.name;
              bool found = false;
              for(auto &kb : mKeyBindings) // make sure binding exists
                {
                  if(kb.name == kbName)
                    {
                      kbg.bindings.push_back(kb.name);
                      found = true;
                      //break; // binding exists
                    }
                }
              if(!found)
                {
                  KeyBinding kb(kbName, "", "Add a " + kbName + ".", ABINDV(startPlacing, iter->second.typeName));
                  mKeyBindings.push_back(kb); mDefaultKeyBindings.push_back(kb);
                }
            }
        }
      mKeyBindingGroups.push_back(kbg);
    }
  
  std::vector<std::string> miscNames;
  std::vector<int>         miscIds;
  for(int i = 0; i < mKeyBindings.size(); i++)
    {
      bool found = false;
      for(auto &g : mKeyBindingGroups)
        {
          auto iter = std::find(g.bindings.begin(), g.bindings.end(), mKeyBindings[i].name);
          if(iter != g.bindings.end()) { g.ids.push_back(i); found = true; break; }
        }
      if(!found)
        { // add to misc group
          std::cout << "====> NOTE: Node type " << mKeyBindings[i].name << " was not found in defined Keybindings (adding to Misc group)\n";
          miscNames.push_back(mKeyBindings[i].name);
          miscIds.push_back(i);
        }
    }

  KeyBindingGroup miscGroup{ "Misc", {}, {} };
  for(int i = 0; i < miscNames.size(); i++)
    {
      miscGroup.bindings.push_back(miscNames[i]);
      miscGroup.ids.push_back(miscIds[i]);
    }
  if(miscGroup.bindings.size() > 0) { mKeyBindingGroups.push_back(miscGroup); }
}

AstroWindow::~AstroWindow()
{
  mUpdateThread.join();
  saveConfig();
  for(auto &p : mProjects) { if(p.graph)   { delete p.graph; } }
  mProjects.clear();
  if(mNodeList)     { delete mNodeList; }
  if(mFileDialog)   { delete mFileDialog; }
  if(mViewSettings) { delete mViewSettings; }
}

void AstroWindow::init()
{
  mViewSettings->init();
  loadConfig();
  // TODO: fix threading issues!
  mUpdateThread = std::thread(std::bind(&AstroWindow::update, this));
  
  ImGui::PushStyleColor(ImGuiCol_NavHighlight, Vec4f(0,0,0,0)); // no keyboard nav highlighting
  ImGui::GetStyle().TouchExtraPadding = Vec2f(3,3); // makes it easier to connect nodes
}

double AstroWindow::calcFps()
{
  tNow = CLOCK_T::now();
  dt = std::chrono::duration_cast<std::chrono::nanoseconds>(tNow - tLast).count()/1000000000.0;
  tLast = tNow;
  
  tDiff += dt; // convert to nanoseconds
  nFrames++;
  double fps = fpsLast;
  if(tDiff > FPS_UPDATE_INTERVAL)
    {
      fps = nFrames / tDiff;
      fpsLast = fps;
      tDiff = 0.0;
      nFrames = 0;
    }
  return fps;
}

//// DIRECTORIES ////

bool AstroWindow::checkProjectDir()
{
  if(!directoryExists(mProjectDir))
    { // make sure project directory exists
      std::cout << "Creating project directory (" << mProjectDir << ")...\n";
      if(!makeDirectory(mProjectDir)) { return false; }
    }
  return true;
}

std::vector<AstroProject*> AstroWindow::getUnsavedProjects()
{
  std::vector<AstroProject*> unsaved;
  for(auto &p : mProjects)
    {
      if(p.graph && p.graph->unsavedChanges())
        { unsaved.push_back(&p); }
    }
  return unsaved;
}


//// NODE ACTIONS ////
void AstroWindow::selectAll()    { if(activeProject()) { activeProject()->graph->selectAll(); } }
void AstroWindow::quitPlacing()  { stopPlacing(true, true); stopPasting(); }
void AstroWindow::groupNodes()   { if(activeProject()) { activeProject()->graph->groupSelected(); } }
void AstroWindow::ungroupNodes() { if(activeProject()) { activeProject()->graph->ungroupSelected(); } }

//// CUT/COPY/PASTE ////

void AstroWindow::cut()
{
  AstroProject *proj = activeProject();
  if(proj && !proj->graph->isLocked())
    {
      std::vector<Node*> selected = proj->graph->getSelected();
      if(selected.size() > 0)
        {
          for(auto n : mClipboard) { delete n; } // delete old clipboard (TODO: save clipboard stack)
          mClipboard.clear();

          // remove nodes from graph
          proj->graph->disconnectExternal(selected, true, true);
          proj->graph->removeNodes(selected, false); // (don't delete!)
          mClipboard = selected;
          
          // // find minimum ID (?)
          // int minId = INT_MAX;
          // for(auto n : mClipboard) { minId = std::min(minId, n->id()); }
          
          for(auto n : mClipboard)
            {
              n->setGraph(nullptr);         // 
              n->setShowConnections(false); // hide connections until pasting
            }
        }
    }
}

void AstroWindow::copy()
{
  AstroProject *proj = activeProject();
  if(proj && !proj->graph->isLocked())
    {
      std::vector<Node*> selected = proj->graph->getSelected();
      if(selected.size() > 0)
        {
          for(auto n : mClipboard) { delete n; } // delete old clipboard (TODO: save clipboard stack)
          mClipboard.clear();
          mClipboard = proj->graph->makeCopies(selected, true);

          // // find minimum ID
          // int minId = INT_MAX;
          // for(auto n : mClipboard) { minId = std::min(minId, n->id()); }
          
          for(auto n : mClipboard)
            {
              n->setGraph(nullptr);         // 
              n->setShowConnections(false); // hide connections until pasting
            }
        }
    }
}

void AstroWindow::startPasting()
{
  AstroProject *proj = activeProject();
  if(!mPasting && mClipboard.size() > 0 && proj && !proj->graph->isLocked())
    {
      stopPlacing();
      mPasting = true;
      
      for(auto n : mClipboard)
        { // transparent "ghost" alpha
          n->setGraph(proj->graph);
          Vec4f mask = n->getColorMask();
          mask.w = GHOST_ALPHA;
          n->setColorMask(mask);
          n->setShowConnections(true);
        }
    }
}
void AstroWindow::stopPasting()
{
  AstroProject *proj = activeProject();
  if(mPasting && proj && !proj->graph->isLocked())
    {
      mPasting = false;
      for(auto n : mClipboard)
        { n->setShowConnections(false); }
    }
}

void AstroWindow::handlePasting()
{
  AstroProject *proj = activeProject();
  if(mPasting && proj && !proj->graph->isLocked())
    {
      ImDrawList *winDrawList = proj->graph->getWinDrawList();
      // find average position of nodes
      Vec2f avgPos(0,0);
      for(auto n : mClipboard) { avgPos += n->rect().center(); }
      avgPos /= mClipboard.size();
  
      Vec2f offset = proj->graph->screenToGraph(ImGui::GetMousePos()) - avgPos;
      for(auto n : mClipboard)
        {
          n->setPos(n->pos() + offset);
          n->draw(winDrawList, true); // draw "ghost" under mouse
        }
                
      if(ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT))
        { // ALT pastes without external connections (don't draw)
          for(auto n : mClipboard)
            { n->setShowConnections(false); }
        }
      else
        { // show external connections
          for(auto n : mClipboard)
            {
              n->setShowConnections(true); 
              n->drawConnections(winDrawList);
            }
        }

      if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
          if(proj->graph->isHovered())
            { // clicked inside graph -- paste clipboard
              proj->graph->deselectAll();
              std::vector<Node*> copied = proj->graph->makeCopies(mClipboard, true);
                            
              for(auto n : copied)
                {
                  n->setPlacing(); // don't interact with ui after placing until mouse release
                  n->setShowConnections(true); // draw connections again
                };
                        
              if(ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT))
                { // ALT pastes without external connections
                  proj->graph->disconnectExternal(copied, true, true);
                }
              for(auto n : copied)
                { // normal alpha
                  n->setId(proj->graph->nextId() + n->id());
                  n->setSelected(true);
                  Vec4f mask = n->getColorMask(); mask.w = 1.0f;
                  n->setColorMask(mask);
                }
                            
              proj->graph->addNodes(copied);
              // proj->graph->setUnsaved(true);

              if(!ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT))  // stop pasting unless shift is held
                {
                  stopPasting();
                  // // hide connections    
                  // for(auto n : mClipboard)
                  //   { n->setShowConnections(false); }
                }
            }
          else // not hovered -- cancel pasting
            { stopPasting(); }
        }
    }
}


void AstroWindow::startPlacing(const std::string &type)
{
  AstroProject *proj = activeProject();
  if(proj && !proj->graph->isLocked())
    {
      stopPasting();
      mPlacing = true;
      
      mPlaceType = type;
      mPlaceNode = NodeGraph::makeNode(type);
      mPlaceNode->setId(-1);
      mPlaceNode->setPos(proj->graph->screenToGraph(ImGui::GetMousePos()) - mPlaceNode->size()/2.0f);
      mPlaceNode->setGraph(proj->graph);

      // transparent alpha (place "ghost")
      Vec4f mask = mPlaceNode->getColorMask();
      mask.w = GHOST_ALPHA;
      mPlaceNode->setColorMask(mask);
    }
}

void AstroWindow::stopPlacing(bool deleteNode, bool selectHovered)
{
  if(mPlacing)
    {
      if(mPlaceNode && deleteNode) { delete mPlaceNode; }
      mPlacing   = false;
      mPlaceType = "";
      mPlaceNode = nullptr;
    }
  else
    {

      AstroProject *proj = activeProject();
      if(selectHovered && proj && proj->graph->getHovered()) // start placing type of node mouse is hovering over
        { startPlacing(proj->graph->getHovered()->type()); }
    }
}

void AstroWindow::handlePlacing()
{
  AstroProject *proj = activeProject();
  if(mPlacing && proj && !proj->graph->isLocked())
    {
      ImDrawList *winDrawList = proj->graph->getWinDrawList();
      
      mPlaceNode->setPos(proj->graph->screenToGraph(ImGui::GetMousePos()) - mPlaceNode->size()/2.0f);
      mPlaceNode->draw(winDrawList, true); // draw "ghost" under mouse (always blocked)
      mPlaceNode->drawConnections(winDrawList);
                    
      if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
          if(proj->graph->isHovered())
            { // clicked inside graph -- place node.
              mPlaceNode->setId(proj->graph->nextId()++);
              if(ImGui::IsMouseDown(ImGuiMouseButton_Left)) { mPlaceNode->setPlacing(); } // don't interact with ui after placing until mouse release
              proj->graph->addNode(mPlaceNode);
              proj->graph->doneAdding();
              mPlacing = false; mPlaceNode = nullptr;
              
              if(ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT)) // if shift down, keep placing (with new node!)
                { startPlacing(mPlaceType); }
            }
          else { stopPlacing(); } // clicked outside of graph -- stop placing.
        }
    }
}


//// PROJECTS ////

AstroProject* AstroWindow::newProject()
{
  int id = 1;
  std::string name = "";
  do
    { // find new project name that hasn't been taken already
      name = std::string(NEW_PROJECT_NAME) + "" + std::to_string(id);
      bool duplicate = false;
      for(auto &p : mProjects)
        { if(p.name == name) { duplicate = true; break; } }
      if(!duplicate) { break; }
      id++;
    } while(true);
  mProjects.push_back({ "", name, new NodeGraph(this, mViewSettings), true });
  mActiveProject = mProjects.size()-1;
  AstroProject *proj = activeProject();
  
  mNodeList->setGraph(proj->graph);
  mFileDialog->setGraph(proj->graph);
  
  stopPlacing(); stopPasting();
  return activeProject();
}

void AstroWindow::projectOpen()
{
  stopPlacing(); stopPasting();
  if(checkProjectDir())
    { mFileDialog->open("Open Project", DIALOG_LOAD, DEFAULT_PROJECT_DIR, {".ags", "*.*"}); }
  else                  { std::cout << "ERROR: Could not create project directory!\n"; }
}

void AstroWindow::projectSave()
{
  stopPlacing(); stopPasting();
  if(checkProjectDir())
    {
      AstroProject *proj = activeProject();
      if(proj)
        {
          if(!proj->path.empty())
            {
              std::cout << "SAVING TO " << proj->path << "...\n";
              std::ofstream f(proj->path);
              f << std::setw(4) << proj->graph->toJSON();
              proj->graph->setUnsaved(false);
            }
          else { projectSaveAs(); }
        }
    }
  else { std::cout << "ERROR: Could not create project directory!\n"; }
}

void AstroWindow::projectSaveAs()
{
  stopPlacing(); stopPasting();
  if(checkProjectDir())
    {
      std::cout << "SAVING AS...\n";
      mFileDialog->open("Save Project As", DIALOG_SAVE, DEFAULT_PROJECT_DIR, {".ags", "*.*"});
    }
  else { std::cout << "ERROR: Could not create project directory!\n"; }
}

void AstroWindow::prevProject()
{
  mActiveProject--;
  if(mActiveProject < 0) { mActiveProject += mProjects.size(); }
}

void AstroWindow::nextProject()
{
  mActiveProject = ((mActiveProject+1) % mProjects.size());
}

void AstroWindow::escape()
{
  for(auto &p : mPopups)   { p.open = false; }   // close all popups
  if(!mCancelDebounce)
    {
      if(mClosing && !mNoSave) { mClosing = false; } // prevents unsaved data popup from reappearing when closing

      AstroProject *proj = activeProject();
      if(proj && !mPasting && !mPlacing) { proj->graph->deselectAll(); }
      stopPasting(); stopPlacing();

      mCancelDebounce = true;
    }
}

void AstroWindow::quit()
{
  stopPlacing(); stopPasting();
  std::vector<AstroProject*> unsaved = getUnsavedProjects();
  if(unsaved.size() == 0 || (mClosing && mNoSave))
    { glfwSetWindowShouldClose(mWindow, GLFW_TRUE); }
  else if(unsaved.size() > 0) { mClosing = true; }
}


bool AstroWindow::undo()
{
  AstroProject *proj = activeProject();
  return (proj ? proj->graph->undo() : false);
}

bool AstroWindow::redo()
{
  AstroProject *proj = activeProject();
  return (proj ? proj->graph->redo() : false);
}



//// CONFIG ////

void AstroWindow::loadConfig()
{
  std::cout << "LOADING CONFIG\n";
  if(fileExists(CONFIG_FILE_PATH))
    {
      std::ifstream f(CONFIG_FILE_PATH, std::ios::in);
      json js; f >> js;
      
      if(js.contains("ViewSettings"))
        {
          if(!mViewSettings->fromJSON(js["ViewSettings"]))
            { std::cout << "WARNING: Failed to load Astrograph config! (" << CONFIG_FILE_PATH << ")\n"; }
        }
      else { std::cout << "WARNING: config doesn't contain view settings!\n"; }
      
      if(js.contains("KeyBindings"))
        {
          json keyBindings = js["KeyBindings"];
          if(keyBindings.contains("main"))
            {
              json mainBindings = keyBindings["main"];
              for(auto &k : mKeyBindings)
                {
                  if(mainBindings.contains(k.name))
                    {
                      k.fromString(mainBindings[k.name]);
                      if(k.name == "Cancel") { mCancelKey = k.sequence.back().key; }
                    }
                }
            }
        }
      else { std::cout << "WARNING: config doesn't contain key bindings!\n"; }
    }
  else
    {
      std::cout << "Could not find " << CONFIG_FILE_PATH << "\n";
      saveConfig();
    }
}

void AstroWindow::saveConfig()
{
  std::cout << "SAVING CONFIG\n";
  json js = json::object();

  js["ViewSettings"] = mViewSettings->toJSON();
  
  json keyBindings   = json::object();
  json mainBindings  = json::object();

  for(auto &k : mKeyBindings) { mainBindings[k.name]  = k.toString(); }
  keyBindings["main"]  = mainBindings;
  js["KeyBindings"]    = keyBindings;
  
  std::ofstream f(CONFIG_FILE_PATH, std::ios::out);
  f << std::setw(JSON_SPACES) << js;
}

//// DRAWING ////

void AstroWindow::drawMenuBar()
{
  //// MENU BAR ////
  if(ImGui::BeginMainMenuBar())
    {
      if(ImGui::BeginMenu("File"))
        {
          if(ImGui::MenuItem("New"))     { newProject(); }
          if(ImGui::MenuItem("Open"))    { projectOpen(); }
          if(ImGui::MenuItem("Save"))    { projectSave(); }
          if(ImGui::MenuItem("Save As")) { projectSaveAs(); }
          if(ImGui::MenuItem("Exit"))    { quit(); }
          ImGui::EndMenu();
        }
      if(ImGui::BeginMenu("Edit"))
        {
          if(ImGui::MenuItem("Cut"))     { cut();    }
          if(ImGui::MenuItem("Copy"))    { copy();   }
          if(ImGui::MenuItem("Paste"))   { startPasting();  }
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
                                {
                                  AstroProject *proj = activeProject();
                                  if(proj) { startPlacing(nIter->first); }
                                }
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
          if(ImGui::MenuItem("Settings"))     { openPopup("View Settings"); }
          ImGui::EndMenu(); // View
        }
      if(ImGui::BeginMenu("Help"))
        {
          if(ImGui::MenuItem("About"))        { openPopup("About"); }
          if(ImGui::MenuItem("Key Bindings")) { openPopup("Key Bindings"); }
          ImGui::EndMenu(); // Help
        }
      ImGui::EndMainMenuBar();
    }  
}

void AstroWindow::drawTabs()
{
  Vec4f circleColor        = Vec4f(0.3f, 0.6f, 0.8f, 1.0f);
  Vec4f xColor             = Vec4f(1.0f, 1.0f, 1.0f, 1.0f);
  
  // button colors
  Vec4f inactiveColor      = Vec4f(0.2f,  0.2f,  0.2f,  1.0f);
  Vec4f hoveredColor       = Vec4f(0.35f, 0.35f, 0.35f, 1.0f);
  Vec4f clickedColor       = Vec4f(0.8f,  0.8f,  0.8f,  1.0f);  
  Vec4f activeColor        = Vec4f(0.5f,  0.5f,  0.5f,  1.0f);
  Vec4f activeHoveredColor = Vec4f(0.65f, 0.65f, 0.65f, 1.0f);
  Vec4f activeClickedColor = Vec4f(0.8f,  0.8f,  0.8f,  1.0f);

  float circlePadding = 2.0f;
  float xPadding      = 1.0f; // added to circle padding
  float xWidth        = 2.0f; // width of X lines
  
  ImDrawList *drawList = ImGui::GetWindowDrawList();
  ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + TABBAR_PADDING - GRAPH_PADDING);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, TABBAR_PADDING);
  ImGui::BeginGroup();
  for(int i = 0; i < mProjects.size(); i++)
    {
      AstroProject *proj = &mProjects[i];
      if(!proj) { continue; }
      
      bool selected = false;
      if(mActiveProject == mSelectedProject)
        { selected = true; mSelectedProject = -1; }
      
      bool active = (mActiveProject == i || selected); // if project is currently being shown
      ImGui::PushStyleColor(ImGuiCol_Button,        (active ? activeColor        : inactiveColor));
      ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (active ? activeHoveredColor : hoveredColor ));
      ImGui::PushStyleColor(ImGuiCol_ButtonActive,  (active ? activeClickedColor : clickedColor ));
      bool clicked = ImGui::Button((proj->name + (proj->graph->unsavedChanges() ? "*" : "  ") + "  ").c_str());
      ImGui::PopStyleColor(3);
      
      Vec2f bSize = Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin();      
      //if(ImGui::IsItemHovered())
        {
          Vec2f cbSize = Vec2f(bSize.y, bSize.y);

          ImGui::SameLine();
          Vec2f cp = ImGui::GetCursorPos();
          ImGui::SameLine(cp.x - 18 - cbSize.x);
          Vec2f p0 = ImGui::GetCursorScreenPos();
          
          if(ImGui::InvisibleButton(("##x"+std::to_string(i)).c_str(), cbSize))
            {
              proj->open = false;
              stopPasting(); stopPlacing();
            }
          
          float circleRad    = cbSize.x/2.0f-circlePadding; // circle radius
          Vec2f circleCenter = p0 + cbSize/2.0f;            // center of circle
          Vec2f mpos         = ImGui::GetMousePos();
          Vec2f mdiff        = mpos - circleCenter;
          if(sqrt(mdiff.x*mdiff.x + mdiff.y*mdiff.y) <= circleRad + circlePadding)
            {
              if(clicked)
                { 
                  proj->open = false;
                  stopPasting(); stopPlacing();
                }
              drawList->AddCircleFilled(circleCenter, circleRad, ImColor(circleColor), 32);
            }

          // draw close button X
          Rect2f xRect(p0+cbSize/4.0f-Vec2f(1,1),  // NOTE: needs offset by 1px (?)
                       p0+cbSize*3.0f/4.0f);
          xRect.expand(-(circlePadding+xPadding)); // add padding          
          drawList->AddLine(xRect.p1, xRect.p2, ImColor(xColor), xWidth);
          drawList->AddLine(Vec2f(xRect.p2.x, xRect.p1.y), Vec2f(xRect.p1.x, xRect.p2.y), ImColor(xColor), xWidth);
        }
      if(clicked || selected)
        {
          stopPlacing(); stopPasting();
          if(proj->open && i != mClosingProject) { mActiveProject = i; }
        }
        if(i < mProjects.size() - 1) { ImGui::SameLine(); }
    }
  ImGui::EndGroup();
  ImGui::PopStyleVar(); // ItemInnerSpacing

  for(int i = 0; i < mProjects.size(); i++)
    {
      AstroProject *proj = &mProjects[i];
      if(!proj->open)
        {
          if(proj->graph->unsavedChanges())
            { // unsaved changes -- prompt to discard
              proj->open = true;
              mClosingProject = i;
              openPopup("Closing Unsaved Project");
              break;
            }
          else
            { // no unsaved changes -- destroy project
              if(mProjects.size() > 1)
                {
                  if(proj->graph) { delete proj->graph; proj->graph = nullptr; };
                  mProjects.erase(mProjects.begin() + i);
                  if(mActiveProject > i || (mActiveProject == i && i == mProjects.size())) { mActiveProject--; }
                  break;
                }
              else
                {
                  quit();
                  break;
                }
              break;
            }
        }
    }
}

void AstroWindow::handleFileDialog()
{
  // check/update file dialog
  if(mFileDialog->check())
    {
      if(mFileDialog->success())
        {
          std::string path = mFileDialog->getPath();
          if(mFileDialog->getType() == DIALOG_LOAD)
            {
              // check if already opened
              for(int i = 0; i < mProjects.size(); i++)
                {
                  if(mProjects[i].path == path)
                    { // switch to project tab instead of opening new project
                      mActiveProject = i;
                      return;
                    }
                }
              // load new project from file
              newProject();
              AstroProject *proj = activeProject();
              if(proj)
                {
                  proj->path = path;
                  proj->name = getBaseName(path);
                  std::ifstream f(path, std::ios::in);
                  json js; f >> js;
                  proj->graph->fromJSON(js);
                  proj->graph->setUnsaved(false);
                }
            }
          else if(mFileDialog->getType() == DIALOG_SAVE)
            { // save active project to file
              AstroProject *proj = activeProject();
              if(proj)
                {
                  proj->path = path;
                  proj->name = getBaseName(path);
                  std::ofstream f(path);
                  json js = proj->graph->toJSON();
                  f << js;
                  proj->graph->setUnsaved(false);
                }
            }
        }
      else { std::cout << "No file selected!!\n"; }
    }
}



//// POPUPS ////

void AstroWindow::openPopup(const std::string &name)
{
  stopPlacing(); stopPasting();
  for(auto &iter : mPopupMap)
    { iter.second->open = (iter.first == name); } // only one popup open at a time
}

void AstroWindow::togglePopup(const std::string &name)
{
  stopPlacing(); stopPasting();
  for(auto &iter : mPopupMap)
    { iter.second->open = ((iter.first == name) ? !iter.second->open : false); } // only one popup open at a time
}

void AstroWindow::drawPopup(Popup &p)
{
  AstroProject *proj = activeProject();
  Vec2f wPos;     // popup window position
  Vec2f wSize;    // popup window size
  Vec2f sizeDiff; // size difference between popup window and child window
  Vec2f textSize; // size of title text
  Vec2f padding  = POPUP_PADDING;
  std::string popupId = "##" + p.name;
  std::string childId = "##" + p.name + "-child";
  if(p.open)
    {
      if(proj) { proj->graph->setLocked(true); }
      ImGui::OpenPopup(popupId.c_str());
    }
  
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
  if(ImGui::BeginPopupModal(popupId.c_str(), &p.open, (p.popupFlags |
                                                       ImGuiWindowFlags_AlwaysAutoResize |   // resize based on child window size
                                                       ImGuiWindowFlags_NoScrollbar      |   // dont scroll outer popup window
                                                       ImGuiWindowFlags_NoScrollWithMouse)))
    {
      padding = (p.childBorder ? padding : Vec2f(0,0)); // if no border, dont add more padding to child
      
      // popup title
      ImGui::PushFont(mViewSettings->titleFont);
      {
        textSize = ImGui::CalcTextSize(p.name.c_str());
        sizeDiff = Vec2f(0, textSize.y) + 2.0f*padding;
        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos())+Vec2f((p.size.x + sizeDiff.x - textSize.x)/2.0f, 0));
        ImGui::TextUnformatted(p.name.c_str());
      }
      ImGui::PopFont();
      ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
      
      if(p.size.x > 0.0f && p.size.y > 0.0f)
        {
          wPos = ((Vec2f(mFrameSize) - (p.size+sizeDiff))/2.0f);
          ImGui::SetWindowPos(wPos);
        }

      bool hover = false;
      if(p.draw)
        {
          Vec2f p0 = ImGui::GetCursorScreenPos();
          if(ImGui::BeginChild(childId.c_str(), p.size, p.childBorder, p.childFlags))
            {
              ImGui::BeginGroup();
              p.draw(p); // draw callback
              ImGui::EndGroup();

              Vec2f contentSize = Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin();
              if(p.size.x <= 0.0f || p.size.y <= 0.0f) // if size(x/y) < 0, automatic resizing based on contents
                {
                  if(p.size.x <= 0.0f || p.autoSizeX) { p.size.x = contentSize.x + 2.0f*padding.x; }
                  if(p.size.y <= 0.0f || p.autoSizeY) { p.size.y = contentSize.y + 2.0f*padding.y; }
                  ImGui::SetWindowSize(childId.c_str(), p.size);
                }
              hover |= (ImGui::IsItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows) ||
                        Rect2f(p0, p0+p.size).contains(ImGui::GetMousePos())); // without rect.contains() test, popup is closed when scrollbar is clicked (?)
              wSize = (Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin()) + sizeDiff;
            }
        }
      ImGui::EndChild();
      
      hover |= ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
      wPos   = ImGui::GetWindowPos();
      
      if(!mBindingEdit && (!hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
        { // cancel if clicked outside of window
          mClosing = false;
          mNoSave  = false;
          p.open   = false;
        }
      ImGui::EndPopup();
    }
  ImGui::PopStyleVar();
  
  if(p.size.x >= 0.0f && p.size.y >= 0.0f)
    { ImGui::SetWindowSize(popupId.c_str(), wSize); }
  
  if(!p.open)
    {
      if(proj)        { proj->graph->setLocked(false); }
      if(p.autoSizeX) { p.size.x = -1.0f; }
      if(p.autoSizeY) { p.size.y = -1.0f; }
    }
}

void AstroWindow::drawAbout(Popup &popup)
{
  Vec2f textSize = ImGui::CalcTextSize("Astrolograph");
  ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos())+Vec2f((popup.size.x - textSize.x)/2.0f, 0));
  ImGui::TextUnformatted("Astrolograph");

  std::string versionStr;
  std::stringstream ss;
  ss << "Version: v" << ASTROLOGRAPH_VERSION_MAJOR << "." << ASTROLOGRAPH_VERSION_MINOR;
  textSize = ImGui::CalcTextSize(ss.str().c_str());
  ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f((popup.size.x - textSize.x)/2.0f, 0));
  ImGui::TextUnformatted(ss.str().c_str());
}

void AstroWindow::drawViewSettings(Popup &popup)
{
  ImGuiIO &io = ImGui::GetIO();
  ImGuiStyle& style = ImGui::GetStyle();
  
  bool openBefore = popup.open;
  
  bool busy = false; // if true, a sub-popup (e.g. choosing color) is open
  busy |= mViewSettings->form()->draw(1.0f, busy);

  if(busy) { mCancelDebounce = true; }
  
  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();
  if(ImGui::Button("Close")) { popup.open = false; }
  ImGui::SameLine();
  if(ImGui::Button("Reset")) { mViewSettings->reset(); }

  if(!busy) // darken background
    { style.Colors[ImGuiCol_ModalWindowDimBg] = Vec4f(0,0,0, 0.4f); }
  else // un-darken screen to see results
    { style.Colors[ImGuiCol_ModalWindowDimBg] = Vec4f(0,0,0, 0.0f); }

  if(openBefore && !popup.open) { saveConfig(); }
}

// opened when exiting program with unsaved projects
void AstroWindow::drawExitUnsavedAlert(Popup &popup)
{
  std::vector<AstroProject*> unsaved = getUnsavedProjects();
  if(unsaved.size() == 0) // nothing to save -- exit
    { mClosing = true; mNoSave  = true; quit(); }
  else
    {
      ImGui::TextUnformatted("Warning: Exiting with unsaved projects:");
      ImGui::Indent();
      for(auto p : unsaved)
        { ImGui::Text("%-32s (%s)", p->name.c_str(), (p->path.empty() ? "[unsaved]" : p->path.c_str())); }
      ImGui::Unindent();
      
      ImGui::SetNextItemWidth(50);
      if(ImGui::Button("Cancel"))   // CANCEL -- don't exit
        { mClosing = false; mNoSave = false; popup.open = false; }
      ImGui::SameLine();
      ImGui::SetNextItemWidth(50);
      if(ImGui::Button("Discard"))  // DISCARD -- exit without saving
        { mClosing = true; mNoSave  = true; quit(); }
    }
}

// opened when closing an unsaved project
void AstroWindow::drawClosingUnsavedAlert(Popup &popup)
{
  ImGui::Text("Warning: Project '%s' has been modified.", closingProject()->name.c_str());
  if(!closingProject()) { popup.open = true; }
  
  ImGui::SetNextItemWidth(50);
  if(ImGui::Button("Cancel"))   // CANCEL -- don't close project
    {
      if(closingProject()) { closingProject()->open = true; }
      mClosingProject = -1;
      popup.open = false;
    }
  ImGui::SameLine();
  ImGui::SetNextItemWidth(50);
  if(ImGui::Button("Discard"))  // DISCARD -- close project without saving
    {
      if(closingProject())
        {
          if(mProjects.size() > 1)
            {
              if(closingProject()->graph) { delete closingProject()->graph; closingProject()->graph = nullptr; };
              mProjects.erase(mProjects.begin() + mClosingProject);
              if(mActiveProject >= mClosingProject) { mActiveProject--; }
            }
          else
            { // no projects -- quit program
              mClosing = true;
              mNoSave  = true;
              mActiveProject = -1;
              quit();
            }
        }
      mClosingProject = -1;
      popup.open = false;
      if(mProjects.size() == 0) { quit(); }
    }
}


//// KEY BINDINGS ////

// draw single key binding //
void AstroWindow::drawKeyBinding(KeyBinding &kb, const KeyBinding &defaultKb)
{
  ImGui::BeginGroup();
  {
    ImGui::Text("%-32s  %s", kb.name.c_str(), kb.toString().c_str());
    ImGui::SameLine(420);
    if(mBindingEdit == &kb)
      { ImGui::Button("<Press Keys>"); }
    else
      {
        if(ImGui::Button(("Set##"+kb.name).c_str()) && !mBindingEdit)
          {
            mOldBinding = kb;
            kb.sequence.clear();
            mBindingEdit = &kb;
          }
        if(kb != defaultKb)
          { // reset button 
            ImGui::SameLine();
            if(ImGui::Button(("Reset##"+kb.name).c_str()) && !mBindingEdit)
              { kb = defaultKb; }
          }
      }
  }
  ImGui::EndGroup();
  if(ImGui::IsItemHovered())
    { ImGui::SetTooltip("%s", kb.description.c_str()); }
}

// draw key binding popup //
void AstroWindow::drawKeyBindings(Popup &popup)
{
  AstroProject *proj = activeProject();
  for(auto &g : mKeyBindingGroups)
    {
      ImGui::Indent();
      ImGui::TextUnformatted(g.name.c_str());
      
      ImGui::Unindent(); ImGui::Separator(); ImGui::Spacing(); ImGui::Indent();
      ImGui::Indent();
      for(int i = 0; i < g.bindings.size(); i++)
        {
          KeyBinding *kb  = (g.ids[i] < mKeyBindings.size()        ? &mKeyBindings[g.ids[i]]        : nullptr);
          KeyBinding *kbd = (g.ids[i] < mDefaultKeyBindings.size() ? &mDefaultKeyBindings[g.ids[i]] : nullptr);
          // auto kb  = std::find(mKeyBindings.begin(),        mKeyBindings.end(),        );
          // auto kbd = std::find(mDefaultKeyBindings.begin(), mDefaultKeyBindings.end(), g.ids[i]);
          
          if(kb && kbd) { drawKeyBinding(*kb, *kbd); }
          else { std::cout << "====> WARNING: Missing key binding! --> " << g.ids[i] << " / " << g.bindings[i] << "\n"; }
        }
      ImGui::Unindent(); ImGui::Unindent();
      ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(0.0f, 16.0f));
      ImGui::Separator();
    }
  // ImGui::Unindent();
}

// TODO: Bindings for NodeList, ViewSettings, etc.
void AstroWindow::handleKeyBindings()
{
  AstroProject *proj = activeProject();
  if(mBindingEdit)
    { // user setting new key binding
      mBindingEdit->sequence = mKeySequence;
      if((mBindingEdit->sequence.size() > 0 && mBindingEdit->sequence.back().key != GLFW_KEY_UNKNOWN))
        {
          // check if binding is available
          bool found = false;          
          for(auto &s : mKeyBindings)
            {
              if(&s != mBindingEdit && s == *mBindingEdit)
                {
                  std::cout << "Key binding is already in use! (" << s.name << ")\n";
                  mBindingEdit->sequence = mOldBinding.sequence;
                  found = true;
                  break;
                }
            }
          // update cancel key
          if(found && mBindingEdit->name == "Cancel")
            { mCancelKey = mBindingEdit->sequence.back().key; }
          // sequence complete -- reset
          mKeySequence.clear();
          mBindingEdit = nullptr;
        }
    }
  else if(!mBindingEdit && mKeySequence.size() > 0 && mKeySequence.back().key != GLFW_KEY_UNKNOWN)
    {     
      bool handled = false;
      // handle bindings
      if(!handled)
        {
          bool popupOpen = false;
          for(auto &p : mPopups) { if(p.open) { popupOpen = true; break; } }
          for(auto &s : mKeyBindings)
            { if((popupOpen || !ImGui::GetIO().WantCaptureKeyboard) && s.check(mKeySequence)) { mKeySequence.clear(); handled = true; break; } }
        }
    }
}

void AstroWindow::draw(const Vec2i &frameSize)
{
  mFrameSize = frameSize;
  ImGui::PushFont(astroWin->viewSettings()->mainFont); // main font
  
  //// DRAWING ////
  ImGuiWindowFlags wFlags = (ImGuiWindowFlags_NoTitleBar        |
                             ImGuiWindowFlags_NoCollapse        |
                             ImGuiWindowFlags_NoMove            |
                             ImGuiWindowFlags_NoScrollbar       |
                             ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_NoResize          |
                             ImGuiWindowFlags_NoSavedSettings   |
                             ImGuiWindowFlags_NoBringToFrontOnFocus
                             );
  const Vec2f padding = GRAPH_PADDING;
  int listWidth = (mNodeList->isCollapsed() ? mNodeList->getWidth() : 512.0f);
  
  // draw menu bar
  drawMenuBar();
  Vec2f mbSize = Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin();

  // draw rest of window
  ImGui::SetNextWindowPos(Vec2f(0,mbSize.y));
  ImGui::SetNextWindowSize(Vec2f(frameSize.x, frameSize.y - mbSize.y));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0); // square frames by default
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  0);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, padding);//Vec2f(0,0));
  ImGui::PushStyleColor(ImGuiCol_WindowBg, Vec4f(0,0,0,1));
  ImGui::Begin("##mainView", nullptr, wFlags); // ImGui window covering full application window
  ImGui::PopStyleColor(1);
  ImGui::PopStyleVar(3);
  {
    // call any key binding actions (must be called from main thread)
    handleKeyBindings();
    AstroProject *proj = activeProject();
    for(auto &k : mKeyBindings) { k.update(); }
    proj = activeProject(); // in case switched to new project
    
    drawTabs();
    Vec2f tbSize = Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin() + Vec2f(0.0f, 2*TABBAR_PADDING.y - padding.y);
    
    Vec2f graphPos = Vec2f(padding.x, mbSize.y + tbSize.y + TABBAR_PADDING.y - padding.y);
    Vec2f graphSize = Vec2f(frameSize.x - 3*padding.x - listWidth, frameSize.y - mbSize.y - tbSize.y - 2*padding.y);
    if(proj && proj->open)
      {
        proj->graph->setPos(graphPos);
        proj->graph->setSize(graphSize);
        proj->graph->showIds(mShowDemo);
        proj->graph->draw(); // draw graph
        
        // draw node ghosts within graph window
        proj->graph->BeginDraw();
        {
          handlePlacing();
          handlePasting();
        
          // add actions to graph right click menu
          if(ImGui::BeginPopupContextWindow("nodeGraphContext"))
            {
              // CUT/COPY
              bool nodesSelected = (proj->graph->getSelected().size() > 0);
              if(!nodesSelected)
                { // disabled if nothing selected
                  ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                  ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                }
              if(ImGui::MenuItem("Cut"))   { cut(); }
              if(ImGui::MenuItem("Copy"))  { copy(); }
              if(!nodesSelected)
                { ImGui::PopItemFlag(); ImGui::PopStyleVar(); }
            
              // PASTE
              if(mClipboard.size() == 0)
                { // disabled if clipboard empty
                  ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                  ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
                }
              if(ImGui::MenuItem("Paste")) { startPasting(); }
              if(mClipboard.size() == 0)
                { ImGui::PopItemFlag(); ImGui::PopStyleVar(); }
            
              // ADD NODE
              if(ImGui::BeginMenu("Add Node"))
                {
                  for(const auto &gIter : NodeGraph::NODE_GROUPS)
                    {
                      if(ImGui::BeginMenu(gIter.name.c_str()))
                        {
                          for(const auto &type : gIter.types)
                            {
                              auto nIter = NodeGraph::NODE_TYPES.find(type);
                              if(nIter != NodeGraph::NODE_TYPES.end())
                                {
                                  if(ImGui::MenuItem(nIter->second.name.c_str()))
                                    { startPlacing(nIter->first); }
                                }
                            }
                          ImGui::EndMenu();
                        }
                    }
                  ImGui::EndMenu();
                }
              ImGui::EndPopup();
            }
        }
        proj->graph->EndDraw();
        
        //if(!closingProject()) { proj->graph->update(dt); }
        mNodeList->setGraph(proj->graph);
      }
    
    if(mNodeList->isCollapsed())
      {
        mNodeList->setPos(Vec2f(frameSize.x - 2.0f*padding.x - listWidth, graphPos.y));
      }
    else
      {
        mNodeList->setPos(Vec2f(frameSize.x - padding.x - listWidth, graphPos.y));
        mNodeList->setSize(Vec2f(listWidth, graphSize.y));
      }
    mNodeList->draw();
  }
  ImGui::End();

  if(mShowFps)
    { // FPS counter
      std::ostringstream ss;
      ss << ((int)calcFps()) << " FPS";
      AstroProject *proj = activeProject();
      if(proj)
        {
          Vec2f center = proj->graph->getCenter();
          float scale = proj->graph->getScale();
          ss << " --> " << center << " / " << std::fixed << std::setprecision(6) << (1.0f/scale);
        }
      std::string str = ss.str();
      ImGui::PushFont(mViewSettings->titleFont);
      Vec2f tSize = ImGui::CalcTextSize(str.c_str());
      ImGui::GetForegroundDrawList()->AddText(Vec2f(15.0f, frameSize.y - tSize.y - 15.0f), ImColor(Vec4f(1,1,1,1)),
                                              str.c_str(), str.c_str()+(str.end() - str.begin()));
      ImGui::PopFont(); // titleFont
    }
  
  // check for exit without saving
  if(mClosing && getUnsavedProjects().size() > 0)
    { openPopup("Unsaved Projects"); }
  
  // draw popups
  for(auto &p : mPopups) { drawPopup(p); }
  
  // handle file dialog (outside main window!)
  AstroProject *proj = activeProject();
  mFileDialog->setGraph(proj->graph);
  handleFileDialog();
  
  // update escape debounce
  if(mCancelDebounce && (ImGui::IsKeyReleased(mCancelKey))) { mCancelDebounce = false; }
  
#if ENABLE_IMGUI_DEMO
  if(mShowDemo) { ImGui::ShowDemoWindow(&mShowDemo); } // show imgui demo window
#endif
  
  ImGui::PopFont(); // main font
}

void AstroWindow::update()
{
  AstroProject *proj = activeProject();
  if(proj && proj->graph) { proj->graph->update(dt); }
}
