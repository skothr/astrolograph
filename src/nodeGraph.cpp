#include "nodeGraph.hpp"
using namespace astro;

#include "glfwKeys.hpp"
#include "imgui.h"
#include "ImGuiFileBrowser.h"
using namespace imgui_addons;

#include <fstream>
#include "tools.hpp"

#include "geometry.hpp"
#include "viewSettings.hpp"
#include "timeNode.hpp"
#include "locationNode.hpp"
#include "chartNode.hpp"
#include "progressNode.hpp"
#include "compareNode.hpp"
#include "chartDataNode.hpp"
#include "chartViewNode.hpp"
#include "aspectNode.hpp"
#include "plotNode.hpp"
#include "moonNode.hpp"


const std::unordered_map<std::string, NodeType> NodeGraph::NODE_TYPES =
  {{ "TimeNode",         {"TimeNode",         "Time Node",          [](){ return new TimeNode();      }} },
   { "TimeSpanNode",     {"TimeSpanNode",     "Time Span Node",     [](){ return new TimeSpanNode();  }} },
   { "LocationNode",     {"LocationNode",     "Location Node",      [](){ return new LocationNode();  }} },
   { "ChartNode",        {"ChartNode",        "Chart Node",         [](){ return new ChartNode();     }} },
   { "ProgressNode",     {"ProgressNode",     "Progress Node",      [](){ return new ProgressNode();  }} },
   { "ChartViewNode",    {"ChartViewNode",    "Chart View Node",    [](){ return new ChartViewNode(); }} },
   { "ChartCompareNode", {"ChartCompareNode", "Chart Compare Node", [](){ return new CompareNode();   }} },
   { "ChartDataNode",    {"ChartDataNode",    "Chart Data Node",    [](){ return new ChartDataNode(); }} },
   { "AspectNode",       {"AspectNode",       "Aspect Node",        [](){ return new AspectNode();    }} },
   { "PlotNode",         {"PlotNode",         "Plot Node",          [](){ return new PlotNode();      }} }, 
   { "MoonNode",         {"MoonNode",         "Moon Node",          [](){ return new MoonNode();      }} }, };

const std::vector<NodeGroup> NodeGraph::NODE_GROUPS =
  { {"Parameters",    {"TimeNode", "TimeSpanNode", "LocationNode"}},
    {"Calculation",   {"ChartNode", "ProgressNode"}},
    {"Visualization", {"ChartViewNode", "ChartCompareNode", "ChartDataNode", "AspectNode", "MoonNode", "PlotNode"}}, };


Node* NodeGraph::makeNode(const std::string &nodeType)
{
  const auto &iter = NODE_TYPES.find(nodeType);
  if(iter != NODE_TYPES.end()) { return iter->second.get(); }
  else                         { return nullptr; }
}

NodeGraph::NodeGraph(ViewSettings *viewSettings)
  : mViewSettings(viewSettings)
{
  if(!directoryExists(mProjectDir))
    { // make sure project directory exists
      std::cout << "Creating project directory (" << mProjectDir << ")...\n";
      if(!makeDirectory(mProjectDir))
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
bool NodeGraph::saveToFile(const std::string &path)
{
  json header      = json::object();
  json nodes       = json::array();
  json connections = json::array();
  
  // HEADER
  header["VERSION"] = SAVE_FILE_VERSION;
  header["center"]  = mGraphCenter.toString();
  header["scale"]   = mGraphScale;

  // NODES/SETTINGS
  for(auto n : mNodes)
    {
      json jsn = n.second->toJson();
      std::cout << "Saving NODE:\n" << jsn << "\n";
      nodes.push_back(jsn);
    }
  
  // CONNECTIONS (only outputs recorded)
  for(auto n : mNodes)
    {
      for(int i = 0; i < n.second->outputs().size(); i++)
        {
          if(n.second->outputs()[i]->getConnected().size() == 0) { continue; } // no connections

          // list all connections to node output connector [i] 
          for(auto con : n.second->outputs()[i]->getConnected())
            {
              json jsc = json::object();
              jsc["nodeId"]      = n.first;             // this node id
              jsc["conId"]       = i;                   // this connector id
              jsc["otherNodeId"] = con->parent()->id(); // other node id
              jsc["otherConId"]  = con->conId();        // other connector id
              connections.push_back(jsc);
            }
        }
    }

  // combine
  json js = json::object();
  js["header"]      = header;
  js["nodes"]       = nodes;
  js["connections"] = connections;

  // write to file
  std::ofstream f(path, std::ios::out);
  f << js.dump() << "\n";
  
  mSaveFile = path;
  mChangedSinceSave = false;
  return true;
}

bool NodeGraph::loadFromFile(const std::string &path)
{
  if(fileExists(path))
    {
      clear(); // clear nodes
      
      // read file
      std::cout << "=============================================================================================\n";
      std::cout << "= Reading file '" << path << "'...\n";
      std::ifstream f(path, std::ios::in);
      json js;
      f >> js;

      //// HEADER ////
      json header;
      if(js.contains("header")) { header = js["header"]; }
      else
        {
          std::cout << "ERROR: No header!\n";
          return false;
        }
      
      // check version
      std::string version = header["VERSION"];
      std::cout << "=   Save file version: " << version << "\n";
      if(version != SAVE_FILE_VERSION)
        { std::cout << "= WARNING: Save file may be out of date! (current version: " << SAVE_FILE_VERSION << ")\n"; }
      std::cout << "=============================================================================================\n";
      std::cout << "= Creating nodes...\n";

      // graph center/scale
      if(!header.contains("center")) { mGraphCenter.fromString(header["center"]); }
      if(!header.contains("scale"))  { mGraphScale = header["scale"];  }

      // set up nodes
      json nodes;
      if(js.contains("nodes")) { nodes = js["nodes"]; }
      for(auto &jsn : nodes)
        {
          std::cout << "LOADING NODE --> \n" << jsn << "\n";
          json nodeHeader; // get node header
          if(jsn.contains("header"))
            {
              nodeHeader = jsn["header"];
              if(nodeHeader.contains("nodeType"))
                {
                  std::string nodeType = nodeHeader["nodeType"];
                  std::cout << "TYPE: " << nodeType << "\n";
                  Node *n = makeNode(nodeType);
                  if(n)
                    {
                      n->setGraph(this);
                      n->fromJson(jsn);
                      mNodes.emplace(n->id(), n);
                    }
                }
              std::cout << "\n";
            }
        }
      
      // set up connections
      std::cout << "= Connecting nodes...\n";
      json connections;
      if(js.contains("connections")) { connections = js["connections"]; }
      for(auto &jsc : connections)
        {
          int nodeId      = jsc["nodeId"];
          int conId       = jsc["conId"];
          int otherNodeId = jsc["otherNodeId"];
          int otherConId  = jsc["otherConId"];

          std::cout << "NODE CONNECTION --> Node" << nodeId << "[" << conId << "]" << " --> Node" << otherNodeId << "[" << otherConId << "]\n";
          
          ConnectorBase *con1 = mNodes[nodeId]->outputs()[conId];
          ConnectorBase *con2 = mNodes[otherNodeId]->inputs()[otherConId];
          if(con1 && con2 && !con1->connect(con2, false)) { std::cout << "Failed to connect!\n"; }
        }
      // new node ids start right after maximum saved id
      int maxId = -1;
      for(auto n : mNodes)
        {
          if(n.first != n.second->id()) { std::cout << "WARNING: Node id doesn't match map id!\n"; }
          maxId = std::max(maxId, n.second->id());
        }
      Node::NEXT_ID = maxId + 1;

      mSaveFile = path;      
      for(auto n : mNodes)
        {
          n.second->update();
          n.second->setChanged(false);
        }
      mChangedSinceSave = false;
      std::cout     << "=============================================================================================\n";
      if(version != SAVE_FILE_VERSION)
        {
          std::cout << "= WARNING: Save file may be out of date! (current version: " << SAVE_FILE_VERSION << ")\n";
          std::cout << "=============================================================================================\n";
        }
      return true;
    }
  else { return false; }
}

void NodeGraph::openSaveDialog()
{
  if(mSaveFile.empty())
    { mOpenSave = true; }
  else // just save to same file (no dialog)
    { saveToFile(mSaveFile); }
}
void NodeGraph::openSaveAsDialog()
{ mOpenSave = true; }
void NodeGraph::openLoadDialog()
{ mOpenLoad = true; }

Node* NodeGraph::addNode(const std::string &type, bool select)
{
  if(!isSelectedActive())
    { // create a new node at the mouse cursor
      Node *n = makeNode(type);
      n->setPos(screenToGraph(ImGui::GetMousePos()) - n->size()/2.0f);
      addNode(n, select);
      return n;
    }
  else { return nullptr; }
}


void NodeGraph::placeNode(const std::string &type)
{
  if(!mLocked)
    {
      mPasting = false;
      mPlacing = true;
      mPlaceType = type;
      mPlaceNode = makeNode(type);
      mPlaceNode->setPos(screenToGraph(ImGui::GetMousePos()) - mPlaceNode->size()/2.0f);
      mPlaceNode->setGraph(this);

      // transparent alpha (place "ghost")
      Vec4f mask = mPlaceNode->getColorMask();
      mask.w = GHOST_ALPHA;
      mPlaceNode->setColorMask(mask);
    }
}

void NodeGraph::stopPlacing()
{
  mPlacing = false;
  mPlaceType = "";
  mPlaceNode = nullptr;
}

void NodeGraph::addNode(Node *n, bool select)
{
  if(n && !mLocked)
    {
      mChangedSinceSave = true;
      if(mNodes.find(n->id()) != mNodes.end()) { n->setId(Node::NEXT_ID++); }
      
      n->setGraph(this);
      mNodes.emplace(n->id(), n);
      if(select) { deselectAll(); n->setSelected(true); }

      // normal alpha
      Vec4f mask = n->getColorMask();
      mask.w = 1.0f;
      n->setColorMask(mask);
    }
}

void NodeGraph::clear()
{
  for(auto n : mNodes) { delete n.second; }
  mNodes.clear();
  mNodes = std::unordered_map<int, Node*>(); // clear nodes and free allocation
  Node::NEXT_ID = 0;
  mSaveFile = "";
  mGraphCenter = Vec2f(0,0);
  mGraphScale  = 1.0f;
  mChangedSinceSave = false;
}

void NodeGraph::cut()
{
  std::vector<Node*> selected = getSelected();
  if(!mLocked && selected.size() > 0)
    {
      // clear clipboard
      for(auto n : mClipboard) { delete n; }
      mClipboard.clear();
      
      // disconnect cut group from other nodes
      disconnectExternal(selected, true, true);
      for(auto n : selected) { mNodes.erase(n->id()); }
      
      // move selected nodes to clipboard
      mClipboard = selected;
      // hide connections until pasting
      for(auto n : mClipboard) { n->setShowConnections(false); }
      
      mChangedSinceSave = true;
    }
}

void NodeGraph::copy()
{
  std::vector<Node*> selected = getSelected();
  if(!mLocked && selected.size() > 0)
    {
      // disconnect cut group from other nodes
      // disconnectExternal(selected, false, true);
      
      std::cout << "CLEARING CLIPBOARD...\n";
      for(auto n : mClipboard) { delete n; }
      mClipboard.clear();

      std::cout << "MAKING COPIES...\n";
      mClipboard = makeCopies(selected, true);
      for(auto n : mClipboard) { n->setShowConnections(false); } // hide connections until pasting
    }
}

void NodeGraph::paste()
{
  if(!mLocked && mClipboard.size() > 0)
    {
      mPlacing = false;
      mPasting = true;

      for(auto n : mClipboard)
        { // transparent "ghost" alpha
          Vec4f mask = n->getColorMask();
          mask.w = GHOST_ALPHA;
          n->setColorMask(mask);
          n->setGraph(this);
        }
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
  n->setSelected(true);
}

void NodeGraph::select(const std::vector<Node*> &nodes)
{
  deselectAll();
  for(auto n : nodes) { n->setSelected(true); }
}

void NodeGraph::selectAll()
{ for(auto n : mNodes) { n.second->setSelected(true); } }

std::vector<Node*> NodeGraph::getSelected()
{
  std::vector<Node*> selected;
  for(auto n : mNodes)
    {
      if(n.second->isSelected()) { selected.push_back(n.second); }
    }
  return selected;
}

void NodeGraph::deselect(const std::vector<Node*> &nodes)
{
  for(auto n : nodes)
    { n->setSelected(false); }
}

void NodeGraph::deselectAll()
{
  for(auto n : mNodes)
    { n.second->setSelected(false); }
}

void NodeGraph::moveSelected(const Vec2f &dpos)
{
  if(!mLocked && isSelectedDragged())
    {
      for(auto n : mNodes)
        {
          if(n.second->isSelected())
            { n.second->setPos(n.second->pos() + dpos); }
        }
      mChangedSinceSave = true;
    }
}

void NodeGraph::fixPositions()
{
  // for(auto n : mNodes)
  //   {
  //     Rect2f r = n.second->rect();
  //     for(auto n2 : mNodes)
  //       {
  //         Rect2f r2 = n2.second->rect();
  //         if(n.second != n2.second && r.intersects(r2))
  //           {
  //             Rect2f insect = r.intersection(r2);
  //             // if(r.contains(insect.p1))
  //             //   {
  //             if(n2.second->isSelected())
  //               { n2.second->setPos(n2.second->pos() + insect.size()); }
  //             else
  //               { n.second->setPos(n.second->pos() + insect.size()); }
  //           }
  //       }
  //   }
}

std::vector<Node*> NodeGraph::makeCopies(const std::vector<Node*> &group, bool externalConnections)
{
  // create node copies
  std::cout << "  CREATING NODE COPIES...\n";
  std::vector<Node*> copies;          // copies of nodes from group
  std::unordered_map<Node*, Node*> oldToNew; // maps old node to new node
  std::unordered_map<Node*, Node*> newToOld; // maps new node to old node

  for(auto n : group)
    {
      Node *newNode = makeNode(n->type());
      n->copyTo(newNode);
      newNode->setGraph(this);
      copies.push_back(newNode);
      oldToNew.emplace(n, newNode);
      newToOld.emplace(newNode, n);
      std::cout << "Copying node --> pos=" << n->pos() << ", size=" << n->size() << ", id=" << n->id() << ", type=" << n->type() << "\n";
    }

  for(auto n : copies)
    {
      Node *old = newToOld[n];
      // loop through input connections
      // output connections will only be copied (as different node's input) if both nodes are in old group
      for(auto c : old->getInputConnections())
        {
          Node *other = nullptr;
          // look for connected node in copies
          for(auto n2 : copies)
            { if(c.nodeOut == newToOld[n2]->id()) { other = n2; break; } }
          
          if(!other && externalConnections)
            { // look for connected node in overall graph
              for(auto n2 : mNodes)
                { if(n2.second != old && c.nodeOut == n2.second->id()) { other = n2.second; break; } }
            }
          
          if(other) // connect
            { other->outputs()[c.conOut]->connect(n->inputs()[c.conIn]); }
          else if(externalConnections)
            { std::cout << "WARNING: Node connection not copied! ( " << c.nodeOut << "[" << c.conOut << "] --> [" << c.nodeIn << "[" << c.conIn << "] )\n"; }
        }
      std::cout << "Copied node --> pos=" << n->pos() << ", size=" << n->size() << ", id=" << n->id() << ", type=" << n->type() << "\n";
    }
  std::cout << copies.size() << "\n";
  return copies;
}

void NodeGraph::disconnectExternal(const std::vector<Node*> &group, bool disconnectInputs, bool disconnectOutputs)
{
  std::cout << "  DISCONNECTING EXTERNAL NODES...\n";
  for(auto n : group)
    {
      // loop through input connections
      if(disconnectInputs)
        {
          for(auto c : n->getInputConnections())
            {
              if(std::find(group.begin(), group.end(), mNodes[c.nodeOut]) == group.end())
                { n->inputs()[c.conIn]->disconnect(mNodes[c.nodeOut]->outputs()[c.conOut]); }
            }
        }
      // loop through output connections
      if(disconnectOutputs)
        {
          for(auto c : n->getOutputConnections())
            {
              if(std::find(group.begin(), group.end(), mNodes[c.nodeIn]) == group.end())
                { n->outputs()[c.conOut]->disconnect(mNodes[c.nodeIn]->inputs()[c.conIn]); }
            }
        }
    }
}

void NodeGraph::copySelected()
{
  if(!mClickCopied)
    {
      std::vector<Node*> newNodes = makeCopies(getSelected(), true);
      deselectAll();
      // select copied nodes
      for(auto n : newNodes)
        {
          n->setFirstFrame(false);
          n->setSelected(true);
          n->setDragging(true);
          n->bringToFront();
          addNode(n, false);
        }
      mChangedSinceSave = true;
      mClickCopied = true;
    }
}


bool NodeGraph::isHovered() const
{
  return Rect2f(mViewPos, mViewSize).contains(ImGui::GetMousePos());
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

void NodeGraph::setPos(const Vec2f &p)
{
  mViewPos = Vec2f(p.x, p.y);
  if(mDrawing) { ImGui::SetWindowPos(p); }
  else         { ImGui::SetNextWindowPos(p); BeginDraw(); EndDraw(); }
}

void NodeGraph::setSize(const Vec2f &s)
{
  mViewSize = Vec2f(s.x, s.y);
  if(mDrawing) { ImGui::SetWindowSize(s); }
  else         { ImGui::SetNextWindowSize(s); BeginDraw(); EndDraw(); }
}

void NodeGraph::drawLines(ImDrawList *drawList)
{
  Vec2f graphTL = screenToGraph(mViewPos);           // graph coordinates of top-left corner of view
  Vec2f graphBR = screenToGraph(mViewPos+mViewSize); // graph coordinates of bottom-right corner of view
  Vec2f gViewSize = screenToGraphVec(mViewSize);     // size of nodegraph view in graph space
  Vec2f firstOffset; // offset to draw initial line

  if(graphTL.x > 0.0f) { firstOffset.x = mViewSettings->graphLineSpacing.x-fmod(graphTL.x, mViewSettings->graphLineSpacing.x); }
  else                 { firstOffset.x = fmod(abs(graphTL.x), mViewSettings->graphLineSpacing.x); }
  if(graphTL.y > 0.0f) { firstOffset.y = mViewSettings->graphLineSpacing.y-fmod(graphTL.y, mViewSettings->graphLineSpacing.y); }
  else                 { firstOffset.y = fmod(abs(graphTL.y), mViewSettings->graphLineSpacing.y); }
  // draw grid lines
  if(mViewSettings->drawGraphLines)
    {  
      for(float x = firstOffset.x; x < gViewSize.x; x += mViewSettings->graphLineSpacing.x)
        {
          drawList->AddLine(graphToScreen(Vec2f(graphTL.x + x, graphTL.y)), graphToScreen(Vec2f(graphTL.x + x, graphBR.y)),
                            ImColor(mViewSettings->graphLineColor), mViewSettings->graphLineWidth);
        }
      for(float y = firstOffset.y; y < gViewSize.y; y += mViewSettings->graphLineSpacing.y)
        {
          drawList->AddLine(graphToScreen(Vec2f(graphTL.x, graphTL.y + y)), graphToScreen(Vec2f(graphBR.x, graphTL.y + y)),
                            ImColor(mViewSettings->graphLineColor), mViewSettings->graphLineWidth);
        }
    }
  // draw axes
  if(mViewSettings->drawGraphAxes)
    {
      drawList->AddLine(graphToScreen(Vec2f(0.0f, graphTL.y)), graphToScreen(Vec2f(0.0f, graphBR.y)),
                        ImColor(mViewSettings->graphAxesColor), mViewSettings->graphLineWidth);
      drawList->AddLine(graphToScreen(Vec2f(graphTL.x, 0.0f)), graphToScreen(Vec2f(graphBR.x, 0.0f)),
                        ImColor(mViewSettings->graphAxesColor), mViewSettings->graphLineWidth);
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
  // ImGui::PushStyleColor(ImGuiCol_ChildBorder, Vec4f());
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(0, 0));
  ImGui::PushStyleColor(ImGuiCol_ChildBg, mViewSettings->graphBgColor);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, mViewSettings->graphBgColor);
  ImGui::BeginChild("nodeGraph", mViewSize, true, wFlags);
  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar();
  mDrawing = true;
}

void NodeGraph::EndDraw()
{
  mDrawing = false;
  ImGui::EndChild();
  // ImGui::PopStyleColor(2);
  ImGui::PopStyleVar(2);
}

void NodeGraph::update()
{
  bool changed = false;
  for(auto n : mNodes)
    {
      n.second->update();
      changed |= n.second->hasChanged();
      n.second->setChanged(false);
    } // update nodes
  
  // TODO: Indicator for unsaved changed (file name tabs with asterisk?)
  
  mChangedSinceSave |= changed;
}

void NodeGraph::draw()
{  
  // draw with imgui
  BeginDraw();
  {
    // mViewPos = ImGui::GetWindowPos();
    // mViewSize = ImGui::GetWindowSize();
    Rect2f graphRect = screenToGraph(Rect2f(mViewPos, mViewPos + mViewSize));
    
    ImGuiIO &io = ImGui::GetIO();
    ImDrawList *winDrawList = ImGui::GetWindowDrawList();
    winDrawList->_FringeScale = getScale();
    ImDrawList *fgDrawList = ImGui::GetForegroundDrawList();
    fgDrawList->_FringeScale = getScale();
    
    // reset click copy flag if mouse released
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      { mClickCopied = false; }
    
    // update nodes (TODO: call in main loop? (main.cpp))
    update();
    
    // draw background graph lines
    drawLines(winDrawList); //  graph lines

    // fix node positions (no overlapping) -- TODO
    // fixPositions();
    
    // move selected nodes to front
    if(!mSelecting)
      {
        for(auto n : mNodes)
          { if(n.second->isSelected()) { n.second->bringToFront(); } }
      }
    // sort nodes by z value
    std::vector<std::pair<int, Node*>> sorted(mNodes.begin(), mNodes.end());
    std::sort(sorted.begin(), sorted.end(),
              [](const std::pair<int, Node*> &n1, const std::pair<int, Node*> &n2) -> bool
              { return (n1.second->getZ() < n2.second->getZ()) || (n1.second->getZ() == n2.second->getZ() && n1.first < n2.first); });
    // clean up z ordering (z = i)
    for(int i = 0; i < sorted.size(); i++) { sorted[i].second->setZ(i); }

    // block from front to back
    std::vector<bool> blocked(sorted.size(), false);
    bool mouseBlocked = false;
    for(int i = sorted.size()-1; i >= 0; i--)
      {
        Node *n = sorted[i].second;
        bool b = n->rect().contains(screenToGraph(ImGui::GetMousePos()));
        blocked[i] = mouseBlocked;
        mouseBlocked |= b;
      }
    // draw nodes
    for(int i = 0; i < sorted.size(); i++) // draw from back to front
      {
        Node *n = sorted[i].second;;
        sorted[i].second->draw(winDrawList, blocked[i]);
        if(mShowIds)
          {
            ImGui::SetCursorPos(graphToScreen(sorted[i].second->pos()) - mViewPos - Vec2f(0.0f, 20.0f));
            ImGui::Text("%d", sorted[i].second->id());
          }
      }
    // draw node connections
    for(auto n : sorted) { n.second->drawConnections(winDrawList); }


    if(!mLocked)
      {
    
        // deselect all nodes if escape pressed
        if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
          { deselectAll(); }
    
        // determine if mouse is hovering over a node, or if node UI is active
        bool active = false;
        bool hover = false;
        Vec2f offsetMouse = screenToGraph(ImGui::GetMousePos());
        for(auto n : mNodes)
          {
            active |= n.second->isActive();
            hover  |= n.second->isHovered() || n.second->rect().intersection(graphRect).contains(offsetMouse);
          }
        
        // DELETE key --> delete selected nodes
        if(!active && ImGui::IsKeyPressed(GLFW_KEY_DELETE))
          {
            std::vector<int> erased;
            for(auto n : mNodes) // delete selected nodes
              {
                if(n.second->isSelected())
                  { erased.push_back(n.second->id()); }
              }
            for(auto nid : erased)
              { delete mNodes[nid]; mNodes.erase(nid); mChangedSinceSave = true; }
          }
    
        // node selection/highlighting
        bool bgHover = ImGui::IsWindowHovered();
        bool lbClick = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        bool mbClick = ImGui::IsMouseClicked(ImGuiMouseButton_Middle);

        if(graphRect.contains(offsetMouse) && !active)
          {
            if(io.KeyCtrl && std::abs(io.MouseWheel) > 0.0f)
              { // zoom/scale
                Vec2f mposOld = screenToGraph(ImGui::GetMousePos());
                float scaleOld = mGraphScale;
                mGraphScale *= (io.MouseWheel > 0.0f ? 1.025f : 1.0f/1.025f);
                mGraphScale = std::max(mGraphScale, 0.25f);
                mGraphScale = std::min(mGraphScale, 4.0f);

                // center scaling on mouse
                Vec2f mposNew = screenToGraph(ImGui::GetMousePos());
                mGraphCenter += mposNew-mposOld;
              }
          }

        
        if(!mPlacing && !mPasting)
          {
            if(((ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT) && bgHover && lbClick) || mbClick) && graphRect.contains(offsetMouse))
              { // pan view center (SHIFT+leftclick+drag, or middleclick+drag)
                mPanning = true;
                mPanClick = screenToGraph(ImGui::GetMousePos());
                ImGui::ResetMouseDragDelta(lbClick ? ImGuiMouseButton_Left : ImGuiMouseButton_Middle);
              }
            else if(bgHover && lbClick)
              { // start drawing selection rectangle
                mSelecting = true;
                mSelectAnchor = screenToGraph(ImGui::GetMousePos());
                mSelectRect.p1 = mSelectAnchor;
                mSelectRect.p2 = mSelectAnchor;
                if(!io.KeyCtrl) { deselectAll(); }
              }

            bool lbUp = ImGui::IsMouseReleased(ImGuiMouseButton_Left);
            bool mbUp = ImGui::IsMouseReleased(ImGuiMouseButton_Middle);
            
            if(mSelecting && lbUp)
              { // stop selecting
                mSelecting = false;
                mSelectAnchor = Vec2f(0,0);
                mSelectRect.p1 = mSelectAnchor;
                mSelectRect.p2 = mSelectAnchor;
              }
            if(mPanning && (lbUp || mbUp))
              { mPanning = false; } // stop panning
    
            if(mSelecting)
              { // selection rect (click+drag)
                if(ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                  {
                    Vec2f mpos = screenToGraph(ImGui::GetMousePos());
                    Rect2f select(Vec2f(std::min(mSelectAnchor.x, mpos.x), std::min(mSelectAnchor.y, mpos.y)),
                                  Vec2f(std::max(mSelectAnchor.x, mpos.x), std::max(mSelectAnchor.y, mpos.y)));
                    mSelectRect = select.fixed().intersection(graphRect);
                  }
                // draw selection rect
                fgDrawList->AddRect(graphToScreen(mSelectRect.p1), graphToScreen(mSelectRect.p2), ImColor(Vec4f(1.0f,1.0f,1.0f,0.5f)), 0.0f, ImDrawCornerFlags_All, 3.0f);
                // select nodes that intersect selection rect
                for(auto n : mNodes) { n.second->setSelected((n.second->isSelected() && io.KeyCtrl) || n.second->rect().intersects(mSelectRect)); }
              }
          }
        else
          { // pan view center (only middleclick+drag -- shift used to multi-paste)
            if(mbClick && graphRect.contains(offsetMouse))
              {
                mPanning = true;
                mPanClick = screenToGraph(ImGui::GetMousePos());
                ImGui::ResetMouseDragDelta(lbClick ? ImGuiMouseButton_Left : ImGuiMouseButton_Middle);
              }
            bool mbUp = ImGui::IsMouseReleased(ImGuiMouseButton_Middle);
            if(mPanning && mbUp) { mPanning = false; } // stop panning
          }

        // PANNING
        if(mPanning)
          { // pan view (shift+click+drag, or middleclick+drag)
            bool lDrag = ImGui::IsMouseDragging(ImGuiMouseButton_Left);
            bool mDrag = ImGui::IsMouseDragging(ImGuiMouseButton_Middle);
            if(lDrag || mDrag)
              {
                mGraphCenter += screenToGraphVec(ImGui::GetMouseDragDelta(lDrag ? ImGuiMouseButton_Left : ImGuiMouseButton_Middle));
                ImGui::ResetMouseDragDelta(lDrag ? ImGuiMouseButton_Left : ImGuiMouseButton_Middle);
              }
          }
        // PLACING
        if(mPlacing)
          {
            if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
              {
                mPlacing = false;
                mPlaceType = "";
                mPlaceNode = nullptr;
              }
            else
              {
                mPlaceNode->setPos(screenToGraph(ImGui::GetMousePos()) - mPlaceNode->size()/2.0f);
                mPlaceNode->draw(winDrawList, true); // draw "ghost" under mouse
                mPlaceNode->drawConnections(winDrawList);

                if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                  {
                    addNode(mPlaceNode);
                    mPlaceNode = nullptr;
                    mPlacing = false;
                    if(ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
                      { // shift down -- keep placing
                        placeNode(mPlaceType);
                      }
                  }
              }
          }

        // PASTING
        else if(mPasting)
          {
            if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
              { mPasting = false; }
            else
              {
                Vec2f avgPos(0,0);
                for(auto n : mClipboard) { avgPos += n->rect().center(); }
                avgPos /= mClipboard.size();
  
                Vec2f offset = screenToGraph(ImGui::GetMousePos()) - avgPos;
                for(auto n : mClipboard)
                  {
                    n->setPos(n->pos() + offset);
                    n->draw(winDrawList, true); // draw "ghost" under mouse
                  }
                
                if(ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT))
                  { // ALT pastes without external connections (don't draw)
                    for(auto n : mClipboard) { n->setShowConnections(false); }
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
                    deselectAll();
                    std::vector<Node*> copied = makeCopies(mClipboard, true);
                    
                    // don't disable connections
                    for(auto n : mClipboard) { n->setShowConnections(true); };
                        
                    if(ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT))
                      { // ALT pastes without external connections
                        disconnectExternal(mClipboard, true, true);
                      }
                    
                    for(auto n : mClipboard)
                      { // normal alpha
                        Vec4f mask = n->getColorMask();
                        mask.w = 1.0f;
                        n->setColorMask(mask);
                        
                        n->setSelected(true);
                        n->setFirstFrame(true);
                        mNodes.emplace(n->id(), n);
                      }
                    mClipboard.clear();
                    mClipboard = copied;
                    for(auto n : mClipboard)
                      { // transparent "ghost" alpha
                        Vec4f mask = n->getColorMask();
                        mask.w = GHOST_ALPHA;
                        n->setColorMask(mask);
                      }
                    mChangedSinceSave = true;

                    if(!ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT))  // stop pasting unless shfit is held
                      { mPasting = false; }
                    else
                      { } // keep pasting -- enable copied nodes again
                    
                  }
              }
          }
        
        // right click menu (alternative to keyboard for adding new nodes)
        if(ImGui::BeginPopupContextWindow("nodeGraphContext"))
          {
            if(ImGui::MenuItem("Recenter"))    { mGraphCenter = Vec2f(0,0); }
            if(ImGui::MenuItem("Reset Scale")) { mGraphScale = 1.0f; }

            if(getSelected().size() > 0)
              {
                if(ImGui::MenuItem("Cut"))   { cut(); }
                if(ImGui::MenuItem("Copy"))  { copy(); }
              }
            if(mClipboard.size() > 0)
              {
                if(ImGui::MenuItem("Paste")) { paste(); }
              }
        
            if(ImGui::BeginMenu("Add Node"))
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
                                    // Node *n = nIter->second.get();
                                    // n->setPos(screenToGraph(ImGui::GetMousePos()));
                                    placeNode(nIter->first);//, true);
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
  }
  EndDraw();
  
  // handle save/load popups
  if(mOpenSave)        { ImGui::OpenPopup("Save File"); mSaveDialogOpen = true; mOpenSave = false; deselectAll(); }
  else if(mOpenLoad)   { ImGui::OpenPopup("Load File"); mLoadDialogOpen = true; mOpenLoad = false; deselectAll(); }

  if(mFileDialog->showFileDialog("Save File", ImGuiFileBrowser::DialogMode::SAVE, FILE_DIALOG_SIZE, ".ags"))
    {
      std::string path = mFileDialog->selected_path;
      std::string ext = getFileExtension(path);
      if(ext != ".ags")
        {
          std::cout << "EXT: '" << ext << "' --> adding .ags extension...\n";
          path += ".ags"; // fix extension
          std::cout << " --> " << path << "\n";
        }
      saveToFile(path);
      mSaveDialogOpen = false;
      //setLocked(false);
    }
  else if(mFileDialog->isClosed)
    {
      mSaveDialogOpen = false;
      //setLocked(false);
    }

  if(mFileDialog->showFileDialog("Load File", ImGuiFileBrowser::DialogMode::OPEN, FILE_DIALOG_SIZE, ".ags"))
    {
      loadFromFile(mFileDialog->selected_path);
      mLoadDialogOpen = false;
      //setLocked(false);
    }
  else if(mFileDialog->isClosed)
    {
      mLoadDialogOpen = false;
      //setLocked(false);
    }
}





// TODO: find path that doesn't intersect any node rects. (recursion?)

#define ORTHO_PADDING (CONNECTOR_SIZE/2.0f + CONNECTOR_PADDING + Vec2f(10.0f, 10.0f))
#define ORTHO_NODE_PADDING 10.0f

std::vector<Vec2f> NodeGraph::findOrthogonalPath(const Vec2f &start, const Vec2f &end)
{
  std::vector<Vec2f> path;

  path.push_back(start);

  Vec2f avg = (start + end)/2.0f;

  if(start.x+ORTHO_PADDING.x <= end.x-ORTHO_PADDING.x && std::abs(start.y - end.y) > 1.0f) // 
    {
      // bool collides = false;
      // Rect2f collideRect;
      // for(auto n : mNodes)
      //   {
      //     if(intersects(n.second->rect(), Vec2f(avg.x, start.y), Vec2f(avg.x, end.y)))
      //       {
      //         collides = true;
      //         collideRect = n.second->rect().expanded(ORTHO_NODE_PADDING);
      //         break;
      //       }
      //   }
      // if(collides)
      //   {
      //     float closerX = (std::abs(collideRect.p2.x - avg.x) < std::abs(collideRect.p1.x - avg.x) ? collideRect.p2.x : collideRect.p1.x);
      //     path.push_back(Vec2f(closerX, start.y));
      //     path.push_back(Vec2f(closerX, end.y));
      //   }
      // else
      {
        path.push_back(Vec2f(avg.x, start.y));
        path.push_back(Vec2f(avg.x, end.y));
      }
    }
  else if(end.x-ORTHO_PADDING.x <= start.x+ORTHO_PADDING.x)
    {
      Vec2f offsetStart = start + Vec2f(ORTHO_PADDING.x, 0);
      Vec2f offsetEnd   = end   - Vec2f(ORTHO_PADDING.x, 0);
      
      // bool collides = false;
      // Rect2f collideRect;
      // for(auto n : mNodes)
      //   {
      //     if(intersects(n.second->rect(), Vec2f(offsetStart.x, avg.y), Vec2f(offsetEnd.x, avg.y)))
      //       {
      //         collides = true;
      //         collideRect = n.second->rect().expanded(ORTHO_NODE_PADDING);
      //         break;
      //       }
      //   }
      
      // if(collides)
      //   {
      //     float closerX = (std::abs(collideRect.p2.x - avg.x) < std::abs(collideRect.p1.x - avg.x) ? collideRect.p2.x : collideRect.p1.x);
      //     path.push_back(Vec2f(closerX, start.y));
      //     path.push_back(Vec2f(closerX, end.y));
      //   }
      // else
      {
        path.push_back(offsetStart);
        path.push_back(Vec2f(offsetStart.x, avg.y));
        path.push_back(Vec2f(offsetEnd.x, avg.y));
        path.push_back(offsetEnd);
      }
    }
  
  path.push_back(end);
  return path;
}
