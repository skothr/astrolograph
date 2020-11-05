#include "nodeGraph.hpp"
using namespace astro;

#include "glfwKeys.hpp"
#include "imgui.h"
#include "ImGuiFileBrowser.h"
using namespace imgui_addons;

#include <fstream>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "timeNode.hpp"
#include "locationNode.hpp"
#include "chartNode.hpp"
#include "progressNode.hpp"
#include "compareNode.hpp"
#include "chartDataNode.hpp"
#include "chartViewNode.hpp"
#include "aspectNode.hpp"


const std::unordered_map<std::string, NodeType> NodeGraph::NODE_TYPES =
  {{ "TimeNode",         {"TimeNode",         "Time Node",          [](){ return new TimeNode();      }} },
   { "TimeSpanNode",     {"TimeSpanNode",     "Time Span Node",     [](){ return new TimeSpanNode();  }} },
   { "LocationNode",     {"LocationNode",     "Location Node",      [](){ return new LocationNode();  }} },
   { "ChartNode",        {"ChartNode",        "Chart Node",         [](){ return new ChartNode();     }} },
   { "ProgressNode",     {"ProgressNode",     "Progress Node",      [](){ return new ProgressNode();  }} },
   { "ChartViewNode",    {"ChartViewNode",    "Chart View Node",    [](){ return new ChartViewNode(); }} },
   { "ChartCompareNode", {"ChartCompareNode", "Chart Compare Node", [](){ return new CompareNode();   }} },
   { "ChartDataNode",    {"ChartDataNode",    "Chart Data Node",    [](){ return new ChartDataNode(); }} },
   { "AspectNode",       {"AspectNode",       "Aspect Node",        [](){ return new AspectNode();    }} }, };

const std::vector<NodeGroup> NodeGraph::NODE_GROUPS =
  { {"Parameters",    {"TimeNode", "TimeSpanNode", "LocationNode"}},
    {"Calculation",   {"ChartNode", "ProgressNode"}},
    {"Visualization", {"ChartViewNode", "ChartCompareNode", "ChartDataNode", "AspectNode"}}, };


Node* NodeGraph::makeNode(const std::string &nodeType)
{
  const auto &iter = NODE_TYPES.find(nodeType);
  if(iter != NODE_TYPES.end()) { return iter->second.get(); }
  else                         { return nullptr; }
}


NodeGraph::NodeGraph()
{
  if(!fs::exists(mProjectDir))
    { // make sure project directory exists
      std::cout << "Creating project directory (" << mProjectDir << ")...\n";
      if(!fs::create_directory(mProjectDir))
        { std::cout << "ERROR: Could not create project directory!\n"; }
    }
  mFileDialog = new ImGuiFileBrowser(mProjectDir);
}

NodeGraph::~NodeGraph()
{
  clear();
  delete mFileDialog;
}

void NodeGraph::addNode(const std::string &type)
{
  addNode(makeNode(type));
}

void NodeGraph::addNode(Node *n)
{
  if(n)
    {
      n->setGraph(this);
      mNodes.emplace(n->id(), n);
      mChangedSinceSave = true;
    }
}

void NodeGraph::clear()
{
  for(auto n : mNodes)
    { delete n.second; }
  mNodes.clear();
  Node::ACTIVE_IDS.clear();
  Node::NEXT_ID = 0;
  mSaveFile = "";
  mChangedSinceSave = false;
}



/////
// TODO: SAVE CONFIRMATIONS --> currently only done when program is exiting (from main.cpp)
//       ALSO --> mark modified when node positions changed
/////


/**
* NODEGRAPH FILE FORMAT:
*  
*  NODE [nodeType] [nodeName] [nodeID] [nodePos] [nodeSize(?)] [...]   // rest of line specified for each node subclass
*
*  [...]
*  
*  // (after all nodes defined)
*  CONN [nodeID1] [INPUT/OUTPUT] [connectorIndex1] [nodeID2] [INPUT/OUTPUT] [connectorIndex2]   // specifies node connection
*  
**/
bool NodeGraph::saveToFile(const std::string &path)
{
  std::ofstream f(path, std::ios::out);
  // save nodes
  for(auto n : mNodes)
    {
      std::ostringstream ss;
      ss << n.second->toSaveString() << "\n";
      std::cout << "--> " << ss.str();
      f << ss.str();
    }
  // save connections
  for(auto n : mNodes)
    {
      for(int i = 0; i < n.second->outputs().size(); i++)
        {
          if(n.second->outputs()[i]->getConnected().size() == 0)
            { continue; } // no connections
          
          std::ostringstream ss;
          ss << "CON " << n.first << " OUTPUT " << i;
          // list all connected node ids
          for(auto con : n.second->outputs()[i]->getConnected())
            { ss << " " << con->parent()->id() << " " << con->conId(); }
          ss << "\n";
          std::cout << "--> " << ss.str();
          f << ss.str();
        }
    }
  mSaveFile = path;
  mChangedSinceSave = false;
  return true;
}

bool NodeGraph::loadFromFile(const std::string &path)
{
  if(fs::exists(path) && fs::is_regular_file(path))
    {
      clear();
      std::ifstream f(path, std::ios::in);

      std::cout << "READING FILE '" << path << "'...\n";
      std::string line;
      while(std::getline(f, line))
        {
          if(line.empty() || line == "\n") { continue; }
          std::istringstream ss(line);
          std::string lineType, type, name;
          int id;
          Vec2f pos;
          ss >> lineType;

          if(lineType == "NODE")
            {
              std::map<std::string, std::string> saveHeader = Node::getSaveHeader(line);
              type = saveHeader["nodeType"];
              name = saveHeader["nodeName"];
              pos.fromString(saveHeader["nodePos"]);
              ss.str(saveHeader["nodeId"]); ss >> id;
              
              std::cout << " --> ADDING NODE '" << name << "': type=" << type << " | id=" << id << " | pos=" << pos << "\n";
              Node *newNode = makeNode(type);
              if(newNode)
                {
                  newNode->fromSaveString(line);
                  std::cout << "N" << id << " --> " << newNode->id() << "\n";
                  mNodes.emplace(id, newNode);
                }
              else
                { std::cout << "ERROR: Could not load Node '" << name << "'!\n"; }
            }
          else if(lineType == "CON")
            {// TODO: load connections
              std::stringstream ss(line);
              std::string temp, io;
              int nId, cId;
              ss >> temp; // "CON "
              ss >> nId;  // node id
              ss >> io;   // always OUTPUT (TODO: ?)
              ss >> cId;  // output connector index

              ConnectorBase *con = mNodes[nId]->outputs()[cId];

              while(ss)
                {
                  int nId2;
                  int cId2;
                  ss >> nId2; ss >> cId2;
                  std::cout << "|  Connecting " << nId << "/" << cId << " to " << nId2 << "/" << cId2 << "...\n";
                  std::cout << "(" << mNodes[nId]->outputs().size() << "|" << mNodes[nId2]->inputs().size() << ")\n";
                  ConnectorBase *con2 = mNodes[nId2]->inputs()[cId2];
                  if(!con->connect(con2))
                    { std::cout << "Failed to connect!\n"; }
                }
            }
        }
      // new node ids start right after maximum saved id
      int maxId = -1;
      for(auto n : mNodes)
        {
          if(n.first != n.second->id())
            { std::cout << "WARNING: Node id doesn't match map id!\n"; }
          maxId = std::max(maxId, n.second->id());
        }
      Node::NEXT_ID = maxId + 1;

      mSaveFile = path;      
      mChangedSinceSave = false;
      return true;
    }
  else
    { return false; }
}

void NodeGraph::openSaveDialog()
{
  if(mSaveFile.empty())
    { mOpenSave = true; }
  else // just save to same file (no dialog)
    { saveToFile(mSaveFile); }
}
void NodeGraph::openSaveAsDialog()
{
  mOpenSave = true;
}
void NodeGraph::openLoadDialog()
{
  mOpenLoad = true;
}

void NodeGraph::draw(const Vec2i &viewSize)
{
  // TODO: window selection/highlighting
  
  // draw nodes
  std::vector<int> deleted;
  std::vector<int> active;
  for(auto n : mNodes)
    {
      bool state = n.second->draw();
      if(!state)                      // node removed
        { deleted.push_back(n.first); }
      else if(n.second->isSelected()) // node window active
        { active.push_back(n.first); }
    }
  if(ImGui::IsKeyPressed(GLFW_KEY_DELETE) && !ImGui::GetIO().WantCaptureKeyboard)
    {
      for(auto nid : active) // delete active nodes
        { deleted.insert(deleted.end(), active.begin(), active.end()); }
    }
  
  // delete any that were closed
  for(auto nid : deleted) { delete mNodes[nid]; mNodes.erase(nid); mChangedSinceSave = true; }
  
  if(ImGui::IsKeyPressed(GLFW_KEY_S) && ImGui::GetIO().KeyCtrl)
    {
      if(ImGui::GetIO().KeyShift) { openSaveAsDialog(); } // Ctrl+Shift+S --> Save As
      else                        { openSaveDialog(); }   // Ctrl+S       --> Save
    }
  else if(ImGui::IsKeyPressed(GLFW_KEY_O) && ImGui::GetIO().KeyCtrl)
    { openLoadDialog(); }

  // check for save/load
  if(mOpenSave)        { ImGui::OpenPopup("Save File"); mSaveDialogOpen = true; mOpenSave = false; }
  else if(mOpenLoad)   { ImGui::OpenPopup("Load File"); mOpenLoad = false; }

  if(mFileDialog->showFileDialog("Save File", ImGuiFileBrowser::DialogMode::SAVE, FILE_DIALOG_SIZE, ".ags"))
    {
      fs::path fp = mFileDialog->selected_path;
      if(fp.extension() != ".ags") { fp += ".ags"; } // fix extension
      saveToFile(fp.string());
      mSaveDialogOpen = false;
    }
  else if(mFileDialog->isClosed)
    { mSaveDialogOpen = false; }

  
  if(mFileDialog->showFileDialog("Load File", ImGuiFileBrowser::DialogMode::OPEN, FILE_DIALOG_SIZE, ".ags"))
    { loadFromFile(mFileDialog->selected_path); }
  
  // right click menu (alternative to keyboard for adding new nodes)
  if(ImGui::BeginPopupContextVoid("nodeGraphContext"))
    {
      if(ImGui::BeginMenu("New"))
        {
          for(const auto &gIter : NODE_GROUPS)
            {
              if(ImGui::BeginMenu(gIter.name.c_str()))
                {
                  for(const auto &type : gIter.types)
                    {
                      auto nIter = NODE_TYPES.find(type);
                      if(nIter != NODE_TYPES.end())
                        {
                          if(ImGui::MenuItem(nIter->second.name.c_str()))
                            { addNode(nIter->second.get()); }
                        }
                    }
                  ImGui::EndMenu();
                }
            }
          ImGui::EndMenu();
        }
      // if(ImGui::MenuItem("Cut"))   { } // TODO (?)
      // if(ImGui::MenuItem("Copy"))  { } // TODO (?)
      // if(ImGui::MenuItem("Paste")) { } // TODO (?)
      ImGui::EndPopup();
    }
}
