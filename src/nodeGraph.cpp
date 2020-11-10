#include "nodeGraph.hpp"
using namespace astro;

#include "glfwKeys.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "ImGuiFileBrowser.h"
using namespace imgui_addons;

#include <fstream>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "viewSettings.hpp"
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


NodeGraph::NodeGraph(ViewSettings *settings)
  : mSettings(settings)
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
      setLocked(false);
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
                  // mNodes.emplace(id, newNode);
                  addNode(newNode);
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

              std::cout << "SS: '" << ss.str() << "'\n";

              while(!ss.eof())
                { // check if end of line
                  ss >> std::skipws;
                  if(ss.str().substr(ss.tellg()).empty()) { break; }
                  // read connection ids
                  int nId2, cId2;
                  ss >> nId2; ss >> cId2;
                  std::cout << "|  Connecting N" << nId << "[C" << cId << "] --> N" << nId2 << "[C" << cId2 << "] \n";
                  std::cout << "(" << mNodes[nId]->outputs().size() << "|" << mNodes[nId2]->inputs().size() << ")\n";
                  ConnectorBase *con2 = mNodes[nId2]->inputs()[cId2];
                  // force connection
                  if(!con->connect(con2)) { std::cout << "Failed to connect!\n"; }
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

Node* NodeGraph::addNode(const std::string &type)
{
  Node *n = makeNode(type);
  addNode(n);
  return n;
}

void NodeGraph::prepNode(Node *n)
{
  if(n)
    {
      if(mNodes.find(n->id()) != mNodes.end())
        { n->setid(Node::NEXT_ID++); }
      n->setGraph(this);
    }
}

void NodeGraph::addNode(Node *n)
{
  if(n && !mLocked)
    {
      mChangedSinceSave = true;
      prepNode(n);
      mNodes.emplace(n->id(), n);
      selectNode(n);
    }
}

void NodeGraph::clear()
{
  for(auto n : mNodes) { delete n.second; }
  mNodes.clear();
  Node::ACTIVE_IDS.clear();
  Node::NEXT_ID = 0;
  mSaveFile = "";
  mChangedSinceSave = false;
}

void NodeGraph::cut()
{
  for(auto n : mClipboard) { delete n; }
  mClipboard.clear();
  // move nodes to clipboard
  std::vector<Node*> selected = getSelected();
  
  for(auto n : selected)
    {
      mNodes.erase(n->id());
      mClipboard.push_back(n);
    }
}

void NodeGraph::copy()
{
  std::cout << "CLEARING CLIPBOARD...\n";
  for(auto n : mClipboard) { delete n; }
  mClipboard.clear();

  std::cout << "MAKING COPIES...\n";
  std::vector<Node*> copied = makeCopies(getSelected());
  for(auto n : copied)
    {
      std::cout << "COPYING...\n";
      mClipboard.push_back(n);
    }
}

void NodeGraph::paste()
{
  Vec2f avgPos;
  for(auto n : mClipboard)
    {
      avgPos += n->pos();
    }
  avgPos /= mClipboard.size();
  std::vector<Node*> copied = makeCopies(mClipboard);
  for(auto n : copied)
    {
      Node *pasted = makeNode(n->type());
      n->copyTo(pasted);
      pasted->setPos(Vec2f(ImGui::GetMousePos()) - avgPos + n->pos());
      addNode(pasted);
    }
}


bool NodeGraph::isConnecting()
{
  for(auto n : mNodes)
   {
     if(n.second->isConnecting())
       { return true; }
   }
  return false;
}

// called by nodes 
void NodeGraph::selectNode(Node *n)
{
  // TODO: hold shift/control to add/toggle nodes from selection?
  if(!n->isSelected() && !isSelectedHovered())
    { deselectAll(); }
  n->setSelected(true);
}
std::vector<Node*> NodeGraph::getSelected()
{
  std::vector<Node*> selected;
  for(auto n : mNodes)
    {
      if(n.second->isSelected()) { selected.push_back(n.second); }
    }
  return selected;
}

void NodeGraph::deselectAll()
{
  for(auto n : mNodes) { n.second->setSelected(false); }
  // select background window
  if(mDrawing) { ImGui::SetWindowFocus(); }
  else         { ImGui::SetNextWindowFocus(); BeginDraw(); EndDraw(); }
}

void NodeGraph::moveSelected(const Vec2f &dpos)
{
  if(!mLocked)// && isSelectedDragged())
    {
      for(auto n : mNodes)
        {
          if(n.second->isSelected())
            { n.second->setPos(n.second->pos() + dpos); }
        }
    }
}

std::vector<Node*> NodeGraph::makeCopies(const std::vector<Node*> &nodes)
{
  // get selected nodes
  std::cout << "  GETTING SELECTED NODES...\n";
  // std::vector<Node*> selected;
  // for(auto n : mNodes)
  //   {
  //     if(n.second->isSelected())
  //       { selected.push_back(n.second); }
  //   }

  // create node copies
  std::cout << "  CREATING NODE COPIES...\n";
  std::unordered_map<Node*, Node*> newNodes; // maps old node to new node
  std::unordered_map<Node*, std::vector<std::vector<int>>> iConnections; // input connections
  std::unordered_map<Node*, std::vector<std::vector<int>>> oConnections; // output connections
  for(auto n : nodes)
    {
      std::cout << "  MAKE NODE...\n";
      Node *nCopy = makeNode(n->type());
      std::cout << "  " << n->id() << "(" << n->type() << "):" << nCopy->id() << "(" << n->type() << ")\n";
      prepNode(nCopy);
      std::cout << "  COPY TO...\n";
      n->copyTo(nCopy); // copy child class data
      std::cout << "   ETC...\n";
      nCopy->notFirstFrame();
      nCopy->bringToFront();
      std::cout << "   EMPLACE...\n";
      newNodes.emplace(n, nCopy);
    }
  
  std::cout << "  ESTABLISHING CONNECTIONS...\n";
  // establish connections
  for(auto nIter : newNodes)
    {
      Node *nOld = nIter.first;
      Node *nNew = nIter.second;

      // copy output connections
      //  --> only if connected node was also copied
      std::cout << "  COPYING OUTPUT CONNECTIONS...\n";
      for(auto conOld : nOld->outputs()) // loop through old node's output connectors
        {
          for(auto conOtherOld : conOld->getConnected()) // loop through connections
            {
              // find copy of connected node
              auto iter = newNodes.find(conOtherOld->parent());
              
              if(iter != newNodes.end()) // add connection if old node was copied
                { // connected node was also copied
                  Node *nOtherNew = iter->second;
                  nNew->outputs()[conOld->conId()]->connect(nOtherNew->inputs()[conOtherOld->conId()]);
                }
              else
                {
                  auto iter2 = mNodes.find(conOtherOld->parent()->id());
                  if(iter2 != mNodes.end()) // add connection if old node was copied
                    { // connected node was also copied
                      Node *nOtherNew = iter2->second;
                      nNew->outputs()[conOld->conId()]->connect(nOtherNew->inputs()[conOtherOld->conId()]);
                    }
                }
            }
        }
      std::cout << "  COPYING INPUT CONNECTIONS...\n";
      // copy input connections
      //  --> always copy
      for(auto conOld : nOld->inputs()) // loop through old node's input connectors
        {
          for(auto conOtherOld : conOld->getConnected())
            {
              Node *nOtherOld = conOtherOld->parent();
              auto iter = newNodes.find(nOtherOld);
              // if(iter == newNodes.end())
              //   { iter = mNodes.find(conOtherOld->parent()); }
              if(iter == newNodes.end()) // add connection only if old node was NOT copied (if it was, already connected by output)
                { // connected node not copied --> connect to old node
                  nNew->inputs()[conOld->conId()]->connect(nOtherOld->outputs()[conOtherOld->conId()]);
                }
            }
        }
    }

  std::cout << "  SELECTING COPIES...\n";
  std::vector<Node*> copied;
  // select copied nodes
  for(auto n : newNodes)
    {
      std::cout << "   " << n.second->id() << "\n";
      n.second->setSelected(true);
      copied.push_back(n.second);
      // selectNode(n.second);
    }
  std::cout << "  DONE COPYING\n";
  return copied;
}

void NodeGraph::copySelected()
{
  if(!mClickCopied)
    {
      std::vector<Node*> newNodes = makeCopies(getSelected());
      deselectAll();
      // select copied nodes
      for(auto n : newNodes)
        {
          addNode(n);
          n->setSelected(true);
          // mNodes.emplace(n->id(), n);
          // selectNode(n.second);
        }
      mClickCopied = true;
    }
}

bool NodeGraph::isSelectedHovered()
{
  for(auto n : mNodes)
    {
      if(n.second->isSelected() && n.second->isHovered())
        { return true; }
    }
  return false;
}

bool NodeGraph::isSelectedActive()
{
  for(auto n : mNodes)
    {
      if(n.second->isSelected() && n.second->isActive())
        { return true; }
    }
  return false;
}

bool NodeGraph::isSelectedDragged()
{
  for(auto n : mNodes)
    {
      if(n.second->isSelected() && n.second->isDragging())
        { return true; }
    }
  return false;
}

void NodeGraph::setPos(const Vec2i &p)
{
  mViewPos = Vec2f(p.x, p.y);
  if(mDrawing) { ImGui::SetWindowPos(p); }
  else         { ImGui::SetNextWindowPos(p); BeginDraw(); EndDraw(); }
}

void NodeGraph::setSize(const Vec2i &s)
{
  mViewSize = Vec2f(s.x, s.y);
  if(mDrawing) { ImGui::SetWindowSize(s); }
  else         { ImGui::SetNextWindowSize(s); BeginDraw(); EndDraw(); }
}

void NodeGraph::drawLines(ImDrawList *drawList)
{
  for(float x = mViewPos.x; x < mViewPos.x + mViewSize.x; x += mSettings->graphLineSpacing.x)
    {
      Vec4f col = mSettings->graphLineColor;
      if(std::abs(mViewSize.x/2.0f - x) < mSettings->graphLineSpacing.x)
        { col = mSettings->graphAxesColor; } // axis origin
      
      drawList->AddLine(Vec2f(x, mViewPos.y), Vec2f(x, mViewPos.y+mViewSize.y), ImColor(col), mSettings->graphLineWidth);
    }
  for(float y = mViewPos.y; y < mViewPos.y + mViewSize.y; y += mSettings->graphLineSpacing.y)
    {
      Vec4f col = mSettings->graphLineColor;
      if(std::abs(mViewSize.y/2.0f - y) < mSettings->graphLineSpacing.x)
        { col = mSettings->graphAxesColor; } // axis origin
      drawList->AddLine(Vec2f(mViewPos.x, y), Vec2f(mViewPos.x+mViewSize.x, y), ImColor(col), mSettings->graphLineWidth);
    }
}

void NodeGraph::BeginDraw()
{
  // blank window over graph for drawing selection rect and other overlays
  ImGuiWindowFlags wFlags = (ImGuiWindowFlags_NoTitleBar        |
                             ImGuiWindowFlags_NoCollapse        |
                             ImGuiWindowFlags_NoMove            |
                             ImGuiWindowFlags_NoScrollbar       |
                             ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_NoResize          |
                             ImGuiWindowFlags_NoSavedSettings   |
                             ImGuiWindowFlags_NoBringToFrontOnFocus
                             );

  // global config
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0); // square frames by default
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  0);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, mSettings->graphBgColor);
  
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(0, 0));
  ImGui::Begin("nodeGraph", 0, wFlags);
  ImGui::PopStyleVar();
  mDrawing = true;
}

void NodeGraph::EndDraw()
{
  ImGui::End();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar(2);
  mDrawing = false;
}

void NodeGraph::draw()
{
  // draw with imgui
  BeginDraw();
  {
    mViewPos = ImGui::GetWindowPos();
    mViewSize = ImGui::GetWindowSize();
    ImDrawList *winDrawList = ImGui::GetWindowDrawList();
    ImDrawList *fgDrawList = ImGui::GetForegroundDrawList();
    if(mSettings->drawGraphLines) { drawLines(winDrawList); }
    
    ImGuiIO &io = ImGui::GetIO();

    // reset click copy flag if mouse released
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      { mClickCopied = false; }
    
    // draw nodes
    ImGui::PushClipRect(mViewPos, mViewSize, false);
    for(auto n : mNodes)
      {
        n.second->draw(winDrawList);
        // if(!mLocked && ImGui::IsItemHovered() && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        //   { selectNode(n.second); }
      }

    // draw connections
    for(auto n : mNodes)
      { n.second->drawConnections(ImGui::GetWindowDrawList()); }

    // DELETE key --> delete selected nodes
    if(!mLocked && ImGui::IsKeyPressed(GLFW_KEY_DELETE))
      {
        std::vector<int> erased;
        for(auto n : mNodes) // delete selected nodes
          {
            if(n.second->isSelected())
              { erased.push_back(n.second->id()); }
          }
          for(auto nid : erased)
            { delete mNodes[nid]; mNodes.erase(nid); }
      }

    bool active = false;
    bool hover = false;
    for(auto n : mNodes)
      { active |= n.second->isActive(); }
    for(auto n : mNodes)
      { hover |= n.second->isHovered(); }
    
    // node selection/highlighting
    if(ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
        // if(ImGui::IsKeyDown(GLFW_KEY_SHIFT))
        //   { // pan view center
        //     mCenter
        //   }
        // else
          {
            mSelecting = true;
            mSelectAnchor = ImGui::GetMousePos();
            mSelectRect.p1 = mSelectAnchor;
            mSelectRect.p2 = mSelectAnchor;
            deselectAll();
          }
      }
    if(mSelecting && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      {
        mSelecting = false;
        mSelectAnchor = Vec2f(0,0);
        mSelectRect.p1 = mSelectAnchor;
        mSelectRect.p2 = mSelectAnchor;
      }

    // selection rect (click+drag)
    if(mSelecting)
      {
        if(ImGui::IsMouseDragging(ImGuiMouseButton_Left))
          {
            Vec2f mpos = ImGui::GetMousePos();
            Rect2f select(Vec2f(std::min(mSelectAnchor.x, mpos.x), std::min(mSelectAnchor.y, mpos.y)),
                          Vec2f(std::max(mSelectAnchor.x, mpos.x), std::max(mSelectAnchor.y, mpos.y)));
            mSelectRect = select.fixed();
          }

        fgDrawList->AddRect(mSelectRect.p1, mSelectRect.p2, ImColor(Vec4f(1.0f,1.0f,1.0f,0.5f)), 0.0f, ImDrawCornerFlags_All, 3.0f);

        for(auto n : mNodes)
          {
            if(n.second->rect().intersects(mSelectRect))
              { n.second->setSelected(true); }
            else
              { n.second->setSelected(false); }
          }
     } 
    
    // right click menu (alternative to keyboard for adding new nodes)
    if(ImGui::BeginPopupContextVoid("nodeGraphContext"))
      {
        if(ImGui::MenuItem("Cut"))   { cut(); }
        if(ImGui::MenuItem("Copy"))  { copy(); }
        if(ImGui::MenuItem("Paste")) { paste(); }
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
                              {
                                Node *n = nIter->second.get();
                                addNode(n);
                              }
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
  EndDraw();
  
  // check for save/load
  if(mOpenSave)        { ImGui::OpenPopup("Save File"); mSaveDialogOpen = true; mOpenSave = false; deselectAll(); }
  else if(mOpenLoad)   { ImGui::OpenPopup("Load File"); mLoadDialogOpen = true; mOpenLoad = false; deselectAll(); }

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
    {
      loadFromFile(mFileDialog->selected_path);
      mLoadDialogOpen = false;
    }
}
