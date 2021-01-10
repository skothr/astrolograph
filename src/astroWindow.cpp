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
    { Popup{ "Unsaved Projects", Vec2f(0,0),
             std::bind(&AstroWindow::drawExitUnsavedAlert, this, std::placeholders::_1),
             ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize, // popup window flags -- keep title bar
             ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse // child flags -- simple message+buttons, no scrolling
            },
      Popup{ "Closing Unsaved Project", Vec2f(0,0),
             std::bind(&AstroWindow::drawClosingUnsavedAlert, this, std::placeholders::_1),
             ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_AlwaysAutoResize, // popup window flags -- keep title bar
             ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse // child flags -- simple message+buttons, no scrolling
            },
      Popup{ "About",        Vec2f(512,256),
             std::bind(&AstroWindow::drawAbout,        this, std::placeholders::_1),
             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar,
             ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse, // child flags -- simple message+buttons, no scrolling
             false,       // no child border
             false, false // fixed size
      },
      Popup{ "Key Bindings", Vec2f(550,768),
             std::bind(&AstroWindow::drawKeyBindings,  this, std::placeholders::_1),
             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar,
             0, true,     // draw child border
             false, false // fixed size
      },
      Popup{ "View Settings", Vec2f(550,768),
             std::bind(&AstroWindow::drawViewSettings, this, std::placeholders::_1),
             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar,
             0, true,     // draw child border
             false, false // fixed size
      },
    };
  for(auto &p : mPopups) { mPopupMap.emplace(p.name, &p); }
  
  mDefaultKeyBindings =
    { //// System/Files
     KeyBinding("New Project",      std::bind(&AstroWindow::newProject,    this),                  "Ctrl+N"       ), // new file
     KeyBinding("Open Project",     std::bind(&AstroWindow::projectOpen,   this),                  "Ctrl+O"       ), // open file
     KeyBinding("Save Project",     std::bind(&AstroWindow::projectSave,   this),                  "Ctrl+S"       ), // save file
     KeyBinding("Save Project As",  std::bind(&AstroWindow::projectSaveAs, this),                  "Ctrl+Shift+S" ), // save file as
     KeyBinding("Prev Project",     std::bind(&AstroWindow::prevProject,   this),                  "Alt+Left"     ), // preivous project
     KeyBinding("Next Project",     std::bind(&AstroWindow::nextProject,   this),                  "Alt+Right"    ), // next project
     KeyBinding("Exit",             std::bind(&AstroWindow::quit,          this),                  "Ctrl+Escape"  ), // quit program
     KeyBinding("Cancel",           std::bind(&AstroWindow::escape,        this),                  "Escape"       ), // cancel/back
     KeyBinding("View Settings",    std::bind(&AstroWindow::togglePopup,   this, "View Settings"), "Alt+V"        ), // toggle view settings
     KeyBinding("Key Bindings",     std::bind(&AstroWindow::togglePopup,   this, "Key Bindings"),  "Alt+K"        ), // toggle binding window
     KeyBinding("About",            std::bind(&AstroWindow::togglePopup,   this, "About"),         "Alt+A"        ), // toggle about window

     //// Graph
     KeyBinding("Cut",              std::bind(&AstroWindow::cut,           this),                  "Ctrl+X"       ), // cut
     KeyBinding("Copy",             std::bind(&AstroWindow::copy,          this),                  "Ctrl+C"       ), // copy
     KeyBinding("Paste",            std::bind(&AstroWindow::startPasting,  this),                  "Ctrl+V"       ), // paste
     KeyBinding("Undo Action",         [&](){ if(activeProject()) { activeProject()->graph->undo(); } }, "Ctrl+Z"       ), // undo
     KeyBinding("Redo Action",         [&](){ if(activeProject()) { activeProject()->graph->redo(); } }, "Ctrl+Shift+Z" ), // redo
     KeyBinding("Select All",          [&](){ if(activeProject()) { activeProject()->graph->selectAll(); } },
                "Ctrl+A"       ), // select all
     KeyBinding("Group Nodes",         [&](){ if(activeProject()) { activeProject()->graph->groupSelected(); } },
                "Ctrl+G"       ), // group selected nodes
     KeyBinding("Ungroup Nodes",       [&](){ if(activeProject()) { activeProject()->graph->ungroupSelected(); } },
                "Ctrl+Shift+G" ), // ungroup selected nodes
     KeyBinding("Quit Placing",        [&](){ stopPlacing(); stopPasting(); },    "Q" ), // stop placing/pasting
     //// Adding Nodes
     KeyBinding("Add Time Node",       [&](){ startPlacing("TimeNode"); },        "T" ), // T --> Time Node
     KeyBinding("Add Time Span Node",  [&](){ startPlacing("TimeSpanNode"); },    "S" ), // S --> Time Span Node
     KeyBinding("Add Location Node",   [&](){ startPlacing("LocationNode"); },    "L" ), // L --> Location Node
     KeyBinding("Add Chart Node",      [&](){ startPlacing("ChartNode");  },      "C" ), // C --> Chart Node
     KeyBinding("Add Progress Node",   [&](){ startPlacing("ProgressNode");  },   "P" ), // P --> Progress Node
     KeyBinding("Add Chart View Node", [&](){ startPlacing("ChartViewNode");  },  "V" ), // V --> Chart View Node
     KeyBinding("Add Compare Node",    [&](){ startPlacing("ChartCompareNode"); },"X" ), // X --> Chart Compare Node
     KeyBinding("Add Data Node",       [&](){ startPlacing("ChartDataNode"); },   "D" ), // D --> Chart Data Node
     KeyBinding("Add Aspect Node",     [&](){ startPlacing("AspectNode"); },      "A" ), // A --> Aspect Node
     KeyBinding("Add Moon Node",       [&](){ startPlacing("MoonNode"); },        "M" ), // M --> Moon Node
     
     //// Debug
     KeyBinding("Show ImGui Demo", [&](){ mShowDemo = !mShowDemo; },                               "Alt+D"        )  // toggle imgui demo
    };
  mKeyBindings = mDefaultKeyBindings;
  for(auto k : mKeyBindings) { if(k.sequence.size() > 0 && k.name == "Cancel") { mCancelKey = k.sequence.back().key; } } // get cancel key
}

AstroWindow::~AstroWindow()
{
  //mUpdateThread.join();
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
  //mUpdateThread = std::thread(std::bind(&AstroWindow::update, this));
  
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

//// CUT/COPY/PASTE ////

void AstroWindow::cut()
{
  AstroProject *proj = activeProject();
  if(proj && proj->graph && !proj->graph->isLocked())
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
  if(proj && proj->graph && !proj->graph->isLocked())
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
  if(!mPasting && mClipboard.size() > 0 && proj && proj->graph && !proj->graph->isLocked())
    {
      stopPlacing();
      mPasting = true;
      
      for(auto n : mClipboard)
        { // transparent "ghost" alpha
          n->setGraph(proj->graph);
          Vec4f mask = n->getColorMask();
          mask.w = GHOST_ALPHA;
          n->setColorMask(mask);
        }
    }
}
void AstroWindow::stopPasting()
{
  AstroProject *proj = activeProject();
  if(mPasting && proj && proj->graph && !proj->graph->isLocked())
    {
      mPasting = false;
    }
}

void AstroWindow::handlePasting()
{
  AstroProject *proj = activeProject();
  if(mPasting && proj && proj->graph && !proj->graph->isLocked())
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
  if(!mPlacing && proj && proj->graph && !proj->graph->isLocked())
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

void AstroWindow::stopPlacing(bool deleteNode)
{
  if(mPlacing)
    {
      if(mPlaceNode && deleteNode) { delete mPlaceNode; }
      mPlacing   = false;
      mPlaceType = "";
      mPlaceNode = nullptr;
    }
}

void AstroWindow::handlePlacing()
{
  // PLACING
  AstroProject *proj = activeProject();
  if(mPlacing && proj && proj->graph && !proj->graph->isLocked())
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
      name = std::string(NEW_PROJECT_NAME) + " " + std::to_string(id);
      bool duplicate = false;
      for(auto &p : mProjects)
        { if(p.name == name) { duplicate = true; break; } }
      if(!duplicate) { break; }
      id++;
    } while(true);
  mProjects.push_back({ "", name, new NodeGraph(this, mViewSettings), true });
  mActiveProject = mProjects.size()-1;
  mNodeList->setGraph(activeProject()->graph);
  mFileDialog->setGraph(activeProject()->graph);
  
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
      //if(mActiveProjectect && mActiveProjectect->graph)
      if(activeProject() && activeProject()->graph)
        {
          if(!activeProject()->path.empty())
            {
              std::cout << "SAVING TO " << activeProject()->path << "...\n";
              std::ofstream f(activeProject()->path);
              f << activeProject()->graph->toJSON();
              activeProject()->graph->setUnsaved(false);
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
  if(!mCancelDebounce)
    {
      std::cout << "ESCAPE!!\n";
      for(auto &p : mPopups)   { p.open = false; }   // close all popups
      if(mClosing && !mNoSave) { mClosing = false; } // prevents unsaved data popup from reappearing when closing

      if(!mPasting && !mPlacing) { activeProject()->graph->deselectAll(); }
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
          if(keyBindings.contains("graph"))
            {
              json graphBindings = keyBindings["graph"];
              for(auto &p : mProjects)
                {
                  for(auto &k : p.graph->getKeyBindings())
                    {
                      if(graphBindings.contains(k.name))
                        { k.fromString(graphBindings[k.name]); }
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
  json graphBindings = json::object();

  for(auto &k : mKeyBindings) { mainBindings[k.name]  = k.toString(); }
  if(activeProject() && activeProject()->graph)
    { for(auto &k : activeProject()->graph->getKeyBindings()) { graphBindings[k.name] = k.toString(); } }
  keyBindings["main"]  = mainBindings;
  keyBindings["graph"] = graphBindings;
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
                                  if(activeProject() && activeProject()->graph)
                                    { startPlacing(nIter->first); }
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
  ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + TABBAR_PADDING - GRAPH_PADDING);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, TABBAR_PADDING);
  ImGui::BeginGroup();
  for(int i = 0; i < mProjects.size(); i++)
    {
      AstroProject *proj = &mProjects[i];
      if(!proj || !proj->graph) { continue; }
      
      bool selected = false;
      if(mActiveProject == mSelectedProject)
        { selected = true; mSelectedProject = -1; }

      bool active = (mActiveProject == i || selected);
      if(active) { ImGui::PushStyleColor(ImGuiCol_Button, Vec4f(0.8, 0.3f, 0.8f, 1.0f)); }

      if(ImGui::Button((proj->name + (proj->graph->unsavedChanges() ? "*" : "")).c_str()) || selected)
        {
          stopPlacing(); stopPasting();
          if(i != mClosingProject) { mActiveProject = i; }
        }
      if(active) { ImGui::PopStyleColor(); }
      
      //if(ImGui::IsItemHovered()) // TODO: Combine close button into tab button
        {
          ImGui::SameLine();
          Vec2f cp = ImGui::GetCursorPos();
          ImGui::SameLine(cp.x - 18);
          if(ImGui::Button(("X##"+std::to_string(i)).c_str())) // close button
            { proj->open = false; stopPlacing(); stopPasting(); }
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
                  if(mActiveProject >= i) { mActiveProject--; }
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
              activeProject()->path = path;
              activeProject()->name = getBaseName(path);
              std::ifstream f(path, std::ios::in);
              json js; f >> js;
              activeProject()->graph->fromJSON(js);
              activeProject()->graph->setUnsaved(false);
            }
          else if(mFileDialog->getType() == DIALOG_SAVE)
            { // save active project to file
              activeProject()->path = path;
              activeProject()->name = getBaseName(path);
              if(activeProject() && activeProject()->graph)
                {
                  std::ofstream f(path);
                  json js = activeProject()->graph->toJSON();
                  f << js;
                  activeProject()->graph->setUnsaved(false);
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
      if(proj && proj->graph) { proj->graph->setLocked(true); }
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
      if(proj && proj->graph) { proj->graph->setLocked(false); }
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
  ImGui::Text("%-20s --> %s", kb.name.c_str(), kb.toString().c_str());
  ImGui::SameLine(400);
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

// draw key binding popup //
void AstroWindow::drawKeyBindings(Popup &popup)
{
  AstroProject *proj = activeProject();
  ImGui::TextUnformatted("Global Bindings");
  ImGui::Separator(); ImGui::Spacing(); ImGui::Indent();
  for(int i = 0; i < mKeyBindings.size(); i++) { drawKeyBinding(mKeyBindings[i], mDefaultKeyBindings[i]); }
  ImGui::Unindent();
  
  ImGui::TextUnformatted("Graph Bindings");
  ImGui::Separator(); ImGui::Spacing(); ImGui::Indent();
  if(proj && proj->graph)
    {
      for(int i = 0; i < proj->graph->getKeyBindings().size(); i++)
        {
          drawKeyBinding(proj->graph->getKeyBindings()[i],
                         proj->graph->getDefaultKeyBindings()[i]);
        }
    }
  ImGui::Unindent();
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
          if(proj && proj->graph)
            {
              for(auto &s : proj->graph->getKeyBindings())
                {
                  if(&s != mBindingEdit && s == *mBindingEdit)
                    {
                      std::cout << "Key binding is already in use! (" << s.name << ")\n";
                      mBindingEdit->sequence = mOldBinding.sequence;
                      found = true;
                      break;
                    }
                }
            }
          if(!found)
            {
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
            }
          else if(mBindingEdit->name == "Cancel") // update cancel key
            { mCancelKey = mBindingEdit->sequence.back().key; }
          mKeySequence.clear();
          mBindingEdit = nullptr;
        }
    }
  else if(!mBindingEdit && mKeySequence.size() > 0 && mKeySequence.back().key != GLFW_KEY_UNKNOWN)
    {     
      bool handled = false;
      // graph key bindings (checked first)
      if(proj && proj->graph)
        {
          if(proj->graph->isSelected())
            {
              for(auto &s : proj->graph->getKeyBindings())
                { if(s.check(mKeySequence)) { mKeySequence.clear(); handled = true; break; } }
            }
        }
      // global key bindings
      if(!handled)
        {
          for(auto &s : mKeyBindings)
            { if(s.check(mKeySequence)) { mKeySequence.clear(); handled = true; break; } }
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
  static const int listWidth = 512;

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
    if(proj && proj->graph && proj != closingProject())
      {
        for(auto &k : proj->graph->getKeyBindings())
          { k.update(); }
      }
    for(auto &k : mKeyBindings) { k.update(); }
    proj = activeProject(); // in case switched to new project
    
    drawTabs();
    Vec2f tbSize = Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin() + Vec2f(0.0f, 2*TABBAR_PADDING.y - padding.y);
    
    Vec2f graphPos = Vec2f(padding.x, mbSize.y + tbSize.y + TABBAR_PADDING.y - padding.y);
    Vec2f graphSize = Vec2f(frameSize.x - 3*padding.x - listWidth, frameSize.y - mbSize.y - tbSize.y - 2*padding.y);
    if(proj && proj->graph && proj->open)
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
        
        if(proj != closingProject()) { proj->graph->update(dt); }
        mNodeList->setGraph(proj->graph);
      }
    
    mNodeList->setPos(Vec2f(frameSize.x - padding.x - listWidth, graphPos.y));
    mNodeList->setSize(Vec2f(listWidth, graphSize.y));
    mNodeList->draw();
  }
  ImGui::End();

  if(mShowFps)
    { // FPS counter
      std::ostringstream ss;
      ss << ((int)calcFps()) << " FPS";
      ImGui::PushFont(mViewSettings->titleFont);
      Vec2f tSize = ImGui::CalcTextSize(ss.str().c_str());
      ImGui::GetForegroundDrawList()->AddText(Vec2f(15.0f, frameSize.y - tSize.y - 15.0f), ImColor(Vec4f(1,1,1,1)),
                                              ss.str().c_str(), ss.str().c_str()+(ss.str().end() - ss.str().begin()));
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
  // if(proj && proj->graph)
  //   {
  // //  for(auto &k : proj->graph->getKeyBindings()) { k.update(); }
  //     proj->graph->update(dt);
  //   }
}
