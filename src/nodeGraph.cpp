#include "nodeGraph.hpp"
using namespace astro;

#include "glfwKeys.hpp"
#include "imgui.h"

#include "nlohmann/json.hpp" // json definitions
using json = nlohmann::json;

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
#include "vedicNode.hpp"
#include "groupNode.hpp"

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
   { "MoonNode",         {"MoonNode",         "Moon Node",          [](){ return new MoonNode();      }} }, 
   { "VedicNode",        {"VedicNode",        "Vedic Node",         [](){ return new VedicNode();     }} }, };

const std::vector<NodeGroup> NodeGraph::NODE_GROUPS =
  { {"Parameters",    { "TimeNode", "TimeSpanNode", "LocationNode" }},
    {"Calculation",   { "ChartNode", "ProgressNode" }},
    {"Visualization", { "ChartViewNode", "ChartCompareNode", "ChartDataNode", "AspectNode", "MoonNode" }},
    {"WIP",           { "PlotNode", "VedicNode" }}, };

Node* NodeGraph::makeNode(const std::string &nodeType)
{
  const auto &iter = NODE_TYPES.find(nodeType);
  if(iter != NODE_TYPES.end()) { return iter->second.get(); }
  else                         { return nullptr; }
}

NodeGraph::NodeGraph(AstroWindow *window, ViewSettings *viewSettings)
  : mWindow(window), mViewSettings(viewSettings)
{
  mAddNodeJSON = new json;
  *mAddNodeJSON = json::array();

  mQuad = new QuadTree<Node>();
  
  mDefaultKeyBindings =
    { //// mGraph Control
     KeyBinding("Cut",                 [&](){ cut(); },                        "Ctrl+X"       ), // cut                    (default Ctrl+X)
     KeyBinding("Copy",                [&](){ copy(); },                       "Ctrl+C"       ), // copy                   (default Ctrl+C)
     KeyBinding("Paste",               [&](){ if(isHovered()) { paste(); } },  "Ctrl+V"       ), // paste                  (default Ctrl+V)
     KeyBinding("Undo Action",         std::bind(&NodeGraph::undo, this),      "Ctrl+Z"       ), // undo
     KeyBinding("Redo Action",         std::bind(&NodeGraph::redo, this),      "Ctrl+Shift+Z" ), // redo
     KeyBinding("Select All",          [&](){ selectAll(); },                  "Ctrl+A"       ), // select all             (default Ctrl+A)
     KeyBinding("Group Nodes",         [&](){ groupSelected(); },              "Ctrl+G"       ), // group selected nodes   (default Ctrl+G)
     KeyBinding("Ungroup Nodes",       [&](){ ungroupSelected(); },            "Ctrl+Shift+G" ), // ungroup selected nodes (default Ctrl+Shift+G)
     KeyBinding("Quit Placing",        [&](){ stopPlacing(); stopPasting(); }, "Q"            ), // stop placing/pasting   (default Q)
     //// Node Creation
     KeyBinding("Add Time Node",       [&](){ placeNode("TimeNode"); },        "T" ), // T --> Time Node
     KeyBinding("Add Time Span Node",  [&](){ placeNode("TimeSpanNode"); },    "S" ), // S --> Time Span Node
     KeyBinding("Add Location Node",   [&](){ placeNode("LocationNode"); },    "L" ), // L --> Location Node
     KeyBinding("Add Chart Node",      [&](){ placeNode("ChartNode");  },      "C" ), // C --> Chart Node
     KeyBinding("Add Progress Node",   [&](){ placeNode("ProgressNode");  },   "P" ), // P --> Progress Node
     KeyBinding("Add Chart View Node", [&](){ placeNode("ChartViewNode");  },  "V" ), // V --> Chart View Node
     KeyBinding("Add Compare Node",    [&](){ placeNode("ChartCompareNode"); },"X" ), // X --> Chart Compare Node
     KeyBinding("Add Data Node",       [&](){ placeNode("ChartDataNode"); },   "D" ), // D --> Chart Data Node
     KeyBinding("Add Aspect Node",     [&](){ placeNode("AspectNode"); },      "A" ), // A --> Aspect Node
     KeyBinding("Add Moon Node",       [&](){ placeNode("MoonNode"); },        "M" ), // M --> Moon Node
    };
  mKeyBindings = mDefaultKeyBindings;
}

NodeGraph::~NodeGraph()
{
  clear();
  if(mAddNodeJSON)  { delete mAddNodeJSON; }
  if(mQuad)         { delete mQuad;        }
}

json NodeGraph::toJSON() const
{
  json header      = json::object();
  json nodes       = json::array();
  json connections = json::array();
  
  // HEADER
  header["VERSION"] = SAVE_FILE_VERSION;
  header["center"]  = mGraphCenter.toString();
  header["scale"]   = mGraphScale;

  // nodes
  for(auto n : mNodes)
    {
      json jsn = n.second->toJSON();
      nodes.push_back(jsn);
    }  
  // connections -- (only outputs recorded)
  for(auto n : mNodes)
    {
      for(int i = 0; i < n.second->outputs().size(); i++)
        {
          if(n.second->outputs()[i]->getConnected().size() == 0)
            { continue; } // no connections
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
  // combine sections
  json js = json::object();
  js["header"]      = header;
  js["nodes"]       = nodes;
  js["connections"] = connections;
  return js;
}

bool NodeGraph::fromJSON(json js)
{
  clear(); // clear nodes
  
  //// HEADER ////
  json header;
  if(js.contains("header")) { header = js["header"]; }
  else { std::cout << "ERROR: No header!\n"; return false; }
      
  // check version
  std::string version = header["VERSION"];
  std::cout << "=   Save file version: " << version << "\n";
  if(version != SAVE_FILE_VERSION) { std::cout << "= WARNING: Save file may be out of date! (current version: " << SAVE_FILE_VERSION << ")\n"; }
  std::cout << "=============================================================================================\n";

  // graph center/scale
  std::cout << "= Reading header...\n";
  if(header.contains("center")) { mGraphCenter.fromString(header["center"]); }
  else { std::cout << "==  WARNING: Save file header doesn't contain node graph center.\n"; }
  if(header.contains("scale"))  { mGraphScale = header["scale"];  }
  else { std::cout << "==  WARNING: Save file header doesn't contain node graph scale.\n"; }
  std::cout << "= Using Graph Center: " << mGraphCenter << "  |  Scale: " << mGraphScale << "\n";
  std::cout << "= Creating nodes...\n";
  
  // set up nodes
  json nodes;
  if(js.contains("nodes")) { nodes = js["nodes"]; }
  for(auto &jsn : nodes)
    {
      json nodeHeader; // get node header
      if(jsn.contains("header"))
        {
          nodeHeader = jsn["header"];
          if(nodeHeader.contains("nodeType"))
            {
              std::string nodeType = nodeHeader["nodeType"];
              Node *n = makeNode(nodeType);
              if(n)
                {
                  n->setGraph(this);
                  n->fromJSON(jsn);
                  mNodes.emplace(n->id(), n);
                  mQuad->add(n, n->pos());
                }
            }
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
      // std::cout << "NODE CONNECTION --> Node" << nodeId << "[" << conId << "]" << " --> Node" << otherNodeId << "[" << otherConId << "]\n";
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
  NEXT_ID = maxId + 1;

  for(auto n : mNodes)
    {
      n.second->update();
      n.second->setChanged(false);
    }
  mChangedSinceSave = false;
  std::cout << "=============================================================================================\n";
  if(version != SAVE_FILE_VERSION)
    {
      std::cout << "= WARNING: Save file may be out of date -- file may not have loaded properly. (current version: " << SAVE_FILE_VERSION << ")\n";
      std::cout << "=============================================================================================\n";
    }
  return true;
}

void NodeGraph::placeNode(const std::string &type)
{
  if(!mLocked)
    {
      mPasting = false;
      mPlacing = true;
      mPlaceType = type;
      mPlaceNode = makeNode(type);
      mPlaceNode->setId(-1);
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
  if(mPlacing)
    {
      mPlacing = false;
      mPlaceType = "";
      if(mPlaceNode) { delete mPlaceNode; }
      mPlaceNode = nullptr;
    }
}

void NodeGraph::stopPasting()
{
  mPasting = false;
  // hide connections    
  for(auto n : mClipboard) { n->setShowConnections(false); }
}

void NodeGraph::addNode(Node *n, bool select)
{
  if(n && !mLocked)
    {
      std::cout << "ADDING NODE -->\n";
      std::cout << n << "\n";
      
      mChangedSinceSave = true;

      if(n->id() < 0) { n->setId(NEXT_ID++); }
      // TODO: check if id should be an offset?
      if(mNodes.find(n->id()) != mNodes.end())
        { n->setId(NEXT_ID++); } // new id if it already taken
      
      n->setGraph(this);
      mNodes.emplace(n->id(), n);
      mQuad->add(n, n->pos());
      //std::cout << *mQuad << "\n";
      if(select) { deselectAll(); n->setSelected(true); }

      // normal alpha
      Vec4f mask = n->getColorMask();
      mask.w = 1.0f;
      n->setColorMask(mask);

      mAddNodeJSON->push_back(n->id());
    }
}

void NodeGraph::doneAdding()
{
  if(mAddNodeJSON->size() > 0)
    { // adding complete --> add to undo stack
      mRedoStack.clear();
      mUndoStack.push_back(Action{ACTION_ADD_NODES, *mAddNodeJSON}); // add action
    }
  *mAddNodeJSON = json::array();
}


void NodeGraph::clear()
{
  mUndoStack.clear();
  for(auto n : mNodes) { delete n.second; }
  mNodes.clear();
  mNodes = std::unordered_map<int, Node*>(); // clear nodes and free allocation (?)
  mQuad->clear();
  NEXT_ID = 0;
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
      for(auto n : selected) { mNodes.erase(n->id()); mQuad->erase(n); }
      
      // move selected nodes to clipboard
      mClipboard = selected;

      int minId = INT_MAX;
      for(auto n : mClipboard) { minId = std::min(minId, n->id()); }

      for(auto n : mClipboard) // hide connections until pasting
        { n->setShowConnections(false); }
      mChangedSinceSave = true;
    }
}



void NodeGraph::copy()
{
  std::vector<Node*> selected = getSelected();
  if(!mLocked && selected.size() > 0)
    {
      std::cout << "CLEARING CLIPBOARD...\n";
      for(auto n : mClipboard) { delete n; }
      mClipboard.clear();

      std::cout << "MAKING COPIES...\n";
      mClipboard = makeCopies(selected, true);

      int minId = INT_MAX;
      for(auto n : mClipboard) { minId = std::min(minId, n->id()); }

      for(auto n : mClipboard)
        {
          n->setShowConnections(false); // hide connections until pasting
        }
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
        }
    }
}


bool NodeGraph::undo()
{
  std::cout << "UNDO!\n";
  if(mUndoStack.size() > 0)
    {
      Action a = mUndoStack.back();
      mUndoStack.pop_back();
      std::cout << "UNDOING --> ";
      switch(a.type)
        {
        case ACTION_ADD_NODES:
        case ACTION_COPY_NODES:
          {
            std::cout << "[ADD NODE]\n";
            json js = std::any_cast<json>(a.data);
            
            std::cout << " --> REMOVING NODES\n";// << std::setw(JSON_SPACES) << js << "\n";
            json removed = json::array();
            for(auto jsid : js)
              {
                int id = jsid.get<int>();
                auto iter = mNodes.find(id);
                if(iter != mNodes.end())
                  {
                    removed.push_back(iter->second->toJSON());
                    delete iter->second;
                    mQuad->erase(mNodes[id]);
                    mNodes.erase(id);
                  }
                else { std::cout << "WARNING: Could not find node to remove! (" << id << ")\n"; }
              }
            
            if(removed.size() > 0)
              {
                mRedoStack.push_back(Action{ACTION_ADD_NODES, removed});
              }
          }
          break;
        case ACTION_MOVE_NODES:
          {
            std::cout << "[MOVE NODES]\n";
            json js = std::any_cast<json>(a.data);
            std::cout << "UN-MOVING NODES\n";// << std::setw(JSON_SPACES) << js << "\n";
            Vec2f dpos = Vec2f(js["dpos"].get<std::string>());
            for(auto id : js["ids"])
              {
                Node *n = mNodes[id];
                n->setPos(n->pos() - dpos);
              }
            mRedoStack.push_back(Action{ACTION_MOVE_NODES, js});
          }
          break;
        default:
          std::cout << "ERROR: Unknown undo action type!\n";
          return false;
        }
      return true;
    }
  else
    {
      std::cout << " --> Nothing to undo! \n";
      return false;
    }
}

bool NodeGraph::redo()
{
  std::cout << "REDO!\n";
  if(mRedoStack.size() > 0)
    {
      Action a = mRedoStack.back();
      mRedoStack.pop_back();
      std::cout << "REDOING --> ";
      switch(a.type)
        {
        case ACTION_ADD_NODES:
        case ACTION_COPY_NODES:
          {
            std::cout << "[ADD NODE]\n";
            json js = std::any_cast<json>(a.data);
            std::cout << "ADDING NODES\n";// << std::setw(JSON_SPACES) << js << "\n";
            
            json added = json::array();
            for(auto jsn : js)
              {
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
                            n->fromJSON(jsn);
                            added.push_back(n->id());
                            mNodes.emplace(n->id(), n);
                            mQuad->add(n, n->pos());
                          }
                      }
                  }
              }
            if(added.size() > 0)
              {
                mUndoStack.push_back(Action{ACTION_ADD_NODES, added});
              }
          }
          break;
        case ACTION_MOVE_NODES:
          {
            std::cout << "[MOVE NODES]\n";
            json js = std::any_cast<json>(a.data);
            std::cout << "RE-MOVING NODES\n";// << std::setw(JSON_SPACES) << js << "\n";
            Vec2f dpos = Vec2f(js["dpos"].get<std::string>());
            for(auto id : js["ids"])
              {
                Node *n = mNodes[id];
                n->setPos(n->pos() + dpos);
              }
            mUndoStack.push_back(Action{ACTION_MOVE_NODES, js});
          }
          break;
        default:
          std::cout << "ERROR: Unknown redo action type!\n";
          return false;
        }
      return true;
    }
  else
    {
      std::cout << " --> Nothing to redo! \n";
      return false;
    }
  return true;
}



bool NodeGraph::isConnecting()
{
  ConnectorBase *connectingTo = nullptr;
  for(auto n : mNodes)
    { if(n.second->isConnecting()) { return true; } }
  return false;
}

ConnectorBase* NodeGraph::getConnectingTo()
{
  ConnectorBase *connectingTo = nullptr;
  for(auto n : mNodes)
    {
      if(n.second->isConnecting())
        {
          for(auto con : n.second->inputs())
            { if(con->isConnecting()) { connectingTo = con; break; } }
          if(connectingTo) { break; }
          for(auto con : n.second->outputs())
            { if(con->isConnecting()) { connectingTo = con; break; } }
          if(connectingTo) { break; }
        }
    }
  return connectingTo;
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
    { if(n.second->isSelected()) { selected.push_back(n.second); } }
  return selected;
}

void NodeGraph::deselect(const std::vector<Node*> &nodes)
{ for(auto n : nodes) { n->setSelected(false); } }
void NodeGraph::deselectAll()
{ for(auto n : mNodes) { n.second->setSelected(false); } }

void NodeGraph::moveSelected(const Vec2f &dpos)
{
  if(!mLocked && isSelectedDragged())
    {
      for(auto n : mNodes)
        {
          if(n.second->isSelected())
            { n.second->setPos(n.second->pos() + dpos); }
        }
      mNodeMoveDPos += dpos;
      mChangedSinceSave = true;
    }
}

void NodeGraph::doneMoving()
{
  if((std::abs(mNodeMoveDPos.x) > 0 || std::abs(mNodeMoveDPos.y) > 0) && !mCopying)
    { // move complete --> add to undo stack
      json js = json::object();
      js["dpos"] = mNodeMoveDPos.toString();
      json ids = json::array();
      for(auto n : mNodes)
        {
          if(n.second->isSelected()) { ids.push_back(n.second->id()); }
        }
      js["ids"] = ids;
      mUndoStack.push_back(Action{ ACTION_MOVE_NODES, js });
      mRedoStack.clear();
      mNodeMoveDPos = Vec2f(0,0);
    }
  mCopying = false;
}

// TODO: should make sure nodes don't overlap, and possibly fixed to grid
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

  int minId = INT_MAX;
  for(auto n : group)
    {
      minId = std::min(minId, n->id());
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

  for(auto n : copies)
    { n->setId(n->id()-minId); }
  
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
      json js = json::array();
      for(auto n : newNodes)
        {
          n->setFirstFrame(false);
          n->setSelected(true);
          n->setDragging(true);
          n->bringToFront();
          n->setId(NEXT_ID+n->id());
          mNodes.emplace(n->id(), n);
          mQuad->add(n, n->pos());
          //addNode(n, false);
          js.push_back(n->toJSON());
        }
      mUndoStack.push_back(Action{ACTION_COPY_NODES, js});
      // doneAdding();
      
      NEXT_ID += newNodes.size();
      mChangedSinceSave = true;
      mCopying = true;
      mClickCopied = true;
      mCopying = true;
    }
}

Node* NodeGraph::groupNodes(const std::vector<Node*> &nodes)
{
  // remove nodes from graph node list (handled by GroupNode)
  for(auto n : nodes)
    {
      if(mNodes.find(n->id()) != mNodes.end())
        { mNodes.erase(n->id()); mQuad->erase(n); }
    }
  return new GroupNode(nodes);
}


std::vector<Node*> NodeGraph::ungroupNodes(const std::vector<Node*> &nodes)
{
  std::vector<Node*> ungrouped;
  for(auto n : nodes)
    {
      if(n->type() == "GroupNode")
        {
          // remove group nodes from graph node list (handled by GroupNode)
          if(mNodes.find(n->id()) != mNodes.end())
            { mNodes.erase(n->id()); mQuad->erase(n); }
          // pop contents
          std::vector<Node*> contents = ((GroupNode*)n)->popContents();
          ungrouped.insert(ungrouped.end(), contents.begin(), contents.end());
          delete n;
        }
    }
  return ungrouped;
}

Node* NodeGraph::groupSelected()
{
  std::vector<Node*> selected = getSelected();
  Node *group = groupNodes(selected);
  addNode(group);
  return group;
}

std::vector<Node*> NodeGraph::ungroupSelected()
{
  std::vector<Node*> selected = getSelected();
  std::vector<Node*> ungrouped = ungroupNodes(selected);
  // add back to graph
  for(auto n : ungrouped) { addNode(n); }
  doneAdding();
  //deselectAll();
  select(ungrouped);
  return ungrouped;
}

bool NodeGraph::isHovered() const
{
  return Rect2f(mViewPos, mViewSize).contains(ImGui::GetMousePos());
}
bool NodeGraph::isSelectedHovered()
{
  for(auto n : mNodes)
    { if(n.second->isSelected() && n.second->isHovered()) { return true; } }
  return false;
}
bool NodeGraph::isSelectedActive()
{
  for(auto n : mNodes)
    { if(n.second->isSelected() && n.second->isActive()) { return true; } }
  return false;
}
bool NodeGraph::isSelectedDragged()
{
  for(auto n : mNodes)
    { if(n.second->isSelected() && n.second->isDragging()) { return true; } }
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

void NodeGraph::update(double dt)
{  
  if(!mScaling) { mLastT = std::chrono::high_resolution_clock::now(); }
  else { mDt += dt; }
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
    Vec2f offsetMouse = screenToGraph(ImGui::GetMousePos());
    
    ImGuiIO &io = ImGui::GetIO();
    ImDrawList *winDrawList = ImGui::GetWindowDrawList();
    ImDrawList *fgDrawList = ImGui::GetForegroundDrawList();
    winDrawList->_FringeScale = getScale();
    fgDrawList->_FringeScale  = getScale();
    
    // reset click copy flag if mouse released
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left))
      { mClickCopied = false; }

    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
        if(isHovered()) { mSelected = true;  }
        else            { mSelected = false; }
      }
    if(isHovered()) { mSelected = true; }
    
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

    // get hovered node
    mHoveredNode = nullptr;
    for(int i = 0; i < sorted.size(); i++) // draw from back to front
      {
        Node *n = sorted[i].second;
        if(!blocked[i] && n->rect().contains(screenToGraph(ImGui::GetMousePos())))
          { mHoveredNode = n; }
      }
    
    // draw nodes
    bool posChanged = false;
    for(int i = 0; i < sorted.size(); i++) // draw from back to front
      {
        Node *n = sorted[i].second;
        Vec2f p0 = n->pos();
        sorted[i].second->draw(winDrawList, blocked[i]);
        Vec2f p1 = n->pos();

        if(p1 != p0) { posChanged = true; }
        
        if(mShowIds)
          {
            ImGui::SetCursorPos(graphToScreen(sorted[i].second->pos()) - mViewPos - Vec2f(0.0f, 20.0f));
            ImGui::Text("%d", sorted[i].second->id());
          }
      }

    {
      // draw node connections
      for(auto n : sorted) { n.second->drawConnections(winDrawList); }

      // update quadtree
      static int numNodes = 0;
      if(posChanged || numNodes != mNodes.size()) { mQuad->update(); }
      // // draw division lines
      // std::vector<std::vector<Vec2f>> divLines;
      // mQuad->getDivisionLines(divLines);
      // float maxW = 10.0f;
      // for(int i = 0; i < divLines.size(); i++)
      //   {
      //     float lineW   = std::min(maxW, (float)(divLines.size() - i));
      //     Vec4f col = (i % 2 == 0 ? Vec4f(0.2f, 1.0f, 1.0f, 1.0f) : Vec4f(1.0f, 0.2f, 1.0f, 1.0f));
      //     for(int j = 0; j < divLines[i].size(); j += 2)
      //       {
      //         if(j+1 < divLines[i].size())
      //           { fgDrawList->AddLine(graphToScreen(divLines[i][j]), graphToScreen(divLines[i][j+1]), ImColor(col), lineW); }
      //       }
      //   }
      numNodes = mNodes.size();
    }
    
    if(!mLocked)
      {
        // deselect all nodes if escape pressed
        if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
          { deselectAll(); }
    
        // determine if mouse is hovering over a node, or if node UI is active
        bool active = false;
        bool hover = false;
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
              { delete mNodes[nid]; mQuad->erase(mNodes[nid]); mNodes.erase(nid); mChangedSinceSave = true; }
          }
    
        // node selection/highlighting
        bool bgHover = ImGui::IsWindowHovered();
        bool lbClick = ImGui::IsMouseClicked(ImGuiMouseButton_Left);
        bool mbClick = ImGui::IsMouseClicked(ImGuiMouseButton_Middle);
        
        if(graphRect.contains(offsetMouse) && (!active || isConnecting()))
          {
            if(io.KeyCtrl && std::abs(io.MouseWheel) > 0.0f)
              { // zoom/scale
                Vec2f mposOld = screenToGraph(ImGui::GetMousePos());
                float scaleOld = mGraphScale;
                float vel = GRAPH_SCALE_VEL; //1.0f + std::max(-0.25f, std::min(0.25f, (1.0f - GRAPH_SCALE_VEL)));
                mGraphScale *= (io.MouseWheel > 0.0f ? vel : 1.0f/vel);
                mGraphScale = std::min(GRAPH_SCALE_MAX, std::max(mGraphScale, GRAPH_SCALE_MIN));
                
                // center scaling on mouse
                Vec2f mposNew = screenToGraph(ImGui::GetMousePos());
                mGraphCenter += mposNew-mposOld;

                // TODO: framerate independent scaling!
                // std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();
                // float dt = 1.0f/30.0f;
                // if(mScaling)
                //   {
                //     dt = mDt; //std::chrono::duration_cast<std::chrono::nanoseconds>(t - mLastT).count()/1000000000.0f;
                    
                //     Vec2f mposOld = screenToGraph(ImGui::GetMousePos());
                //     float scaleOld = mGraphScale;
                //     float vel = 1.0f + std::max(-0.25f, std::min(0.25f, (1.0f - GRAPH_SCALE_VEL)*((float)dt)*50.0f));
                //     mGraphScale *= (io.MouseWheel > 0.0f ? 1.0f/vel : vel);
                //     mGraphScale = std::min(GRAPH_SCALE_MAX, std::max(mGraphScale, GRAPH_SCALE_MIN));
                
                //     // center scaling on mouse
                //     Vec2f mposNew = screenToGraph(ImGui::GetMousePos());
                //     mGraphCenter += mposNew-mposOld;
                //   }
                // else
                //   {
                //     dt = std::chrono::duration_cast<std::chrono::nanoseconds>(t - mLastT).count()/1000000000.0f;
                //     //std::chrono::high_resolution_clock::now();
                //     mScaling = true; 
                //   }
                // mLastT = t;
                // // else
                // //   {
                // //     std::chrono::high_resolution_clock::time_point t = std::chrono::high_resolution_clock::now();
                // //     float dt = std::chrono::duration_cast<std::chrono::nanoseconds>(t - mLastT).count()/1000000000.0;
                // //     mLastT = t;
                // //   }
              }
          }
        
        if(!active)
          {
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
                    mPlaceNode->draw(winDrawList, true); // draw "ghost" under mouse (always blocked)
                    mPlaceNode->drawConnections(winDrawList);
                    
                    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                      {
                        if(isHovered())
                          { // clicked inside graph -- place node.
                            mPlaceNode->setId(NEXT_ID++);
                            if(ImGui::IsMouseDown(ImGuiMouseButton_Left))
                              { mPlaceNode->setPlacing(); } // don't interact with ui after placing until mouse release
                            addNode(mPlaceNode);
                            doneAdding();
                            mPlaceNode = nullptr;
                            mPlacing = false;
                            if(ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
                              { // shift down -- keep placing
                                placeNode(mPlaceType);
                              }
                          }
                        else
                          { // clicked outside of graph -- stop placing.
                            stopPlacing();
                            mPlaceNode = nullptr;
                            mPlacing = false;
                          }
                      }
                  }
              }
            // PASTING
            else if(mPasting)
              {
                if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
                  {
                    mPasting = false;
                    // hide connections    
                    for(auto n : mClipboard) { n->setShowConnections(false); }
                  }
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
                        if(isHovered())
                          { // clicked inside graph -- paste clipboard
                            deselectAll();
                            std::vector<Node*> copied = makeCopies(mClipboard, true);
                    
                            
                            for(auto n : mClipboard)
                              {
                                n->setPlacing(); // don't interact with ui after placing until mouse release
                                n->setShowConnections(true); // draw connections again
                              };
                        
                            if(ImGui::IsKeyDown(GLFW_KEY_LEFT_ALT))
                              { // ALT pastes without external connections
                                disconnectExternal(mClipboard, true, true);
                              }
                            std::cout << "     IDS --> ";
                            for(auto n : mClipboard)
                              { // normal alpha
                                n->setId(NEXT_ID+n->id());
                                n->setSelected(true);
                                Vec4f mask = n->getColorMask(); mask.w = 1.0f;
                                n->setColorMask(mask);

                                //mNodes.emplace(n->id(), n);
                                addNode(n);
                              }
                            doneAdding();
                            std::cout << "\n";
                            NEXT_ID += mClipboard.size();

                            std::cout << "NEXT_ID = " << NEXT_ID << "\n";
                        
                            mClipboard.clear();
                            mClipboard = copied;
                            for(auto n : mClipboard)
                              { // transparent "ghost" alpha
                                Vec4f mask = n->getColorMask();
                                mask.w = GHOST_ALPHA;
                                n->setColorMask(mask);
                              }
                            mChangedSinceSave = true;

                            if(!ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT))  // stop pasting unless shift is held
                              {
                                mPasting = false;
                                // hide connections    
                                for(auto n : mClipboard)
                                  { n->setShowConnections(false); }
                              }
                            else
                              { } // keep pasting -- enable copied nodes again
                          }
                        else
                          { // not hovered -- cancel pasting
                            mPasting = false;
                          }
                      }
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
                                  { placeNode(nIter->first); }
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
}

// TODO: find path that doesn't intersect any node rects. (recursion?)

#define ORTHO_NODE_PADDING 10.0f
#define ORTHO_PADDING (CONNECTOR_SIZE/2.0f + CONNECTOR_PADDING + Vec2f(ORTHO_NODE_PADDING, ORTHO_NODE_PADDING))

std::vector<Vec2f> NodeGraph::findOrthogonalPath(const Vec2f &start, const Rect2f &startRect, const Vec2f &end, const Rect2f &endRect, Direction direction)
{
  std::vector<Vec2f> path;
  path.push_back(start);

  Rect2f sRect = startRect.expanded(ORTHO_NODE_PADDING/2.0f);
  Rect2f eRect = endRect.expanded(ORTHO_NODE_PADDING/2.0f);
  Vec2f sCenter = sRect.center();
  Vec2f eCenter = eRect.center();
  if(start.x+ORTHO_PADDING.x <= end.x-ORTHO_PADDING.x && std::abs(start.y - end.y) > 1.0f)
    { // "Z" shape (horizontal, vertical, horizontal)
      Vec2f sEdge = Vec2f(sRect.p2.x, start.y);
      Vec2f eEdge = Vec2f(eRect.p1.x, end.y);
      Vec2f avg = (sEdge + eEdge)/2.0f;
      path.push_back(Vec2f(avg.x, start.y));
      path.push_back(Vec2f(avg.x, end.y));
    }
  else if(end.x-ORTHO_PADDING.x <= start.x+ORTHO_PADDING.x)
    {
      Vec2f offsetStart = start + Vec2f(ORTHO_PADDING.x, 0);
      Vec2f offsetEnd   = end   - Vec2f(ORTHO_PADDING.x, 0);
      
      if(eRect.p1.y > sRect.p2.y || eRect.p2.y < sRect.p1.y)
        {
          float sEdgeY   = ((sCenter.y < eCenter.y) ? sRect.p2.y : sRect.p1.y);
          float eEdgeY   = ((eCenter.y > sCenter.y) ? eRect.p1.y : eRect.p2.y);
          float edgeAvgY = (sEdgeY + eEdgeY)/2.0f;
      
          Vec2f ps = Vec2f(offsetStart.x, edgeAvgY);
          Vec2f pe = Vec2f(offsetEnd.x,   edgeAvgY);
      
          // whether horizontal line will overlap one of the rects
          bool sOverlapped = intersects(sRect, ps, pe);
          bool eOverlapped = intersects(eRect, ps, pe);

          path.push_back(offsetStart);
          {
            if(sOverlapped || eOverlapped)
              {
                ps.y = sRect.p1.y;
                pe.y = eRect.p2.y;

                float xAvg = (eRect.p2.x + sRect.p1.x)/2.0f;
                path.push_back(ps);
                path.push_back(Vec2f(xAvg, ps.y));
                path.push_back(Vec2f(xAvg, pe.y));
                path.push_back(pe);
              }
            else
              {
                path.push_back(ps);
                path.push_back(pe);
              }
          }
          path.push_back(offsetEnd);
        }
      else
        {
          float sEdgeY   = ((sCenter.y < eCenter.y) ? sRect.p1.y : sRect.p2.y);
          float eEdgeY   = ((sCenter.y < eCenter.y) ? eRect.p2.y : eRect.p1.y);
          float edgeAvgY = (sEdgeY + eEdgeY)/2.0f;
      
          Vec2f ps = Vec2f(offsetStart.x, edgeAvgY);
          Vec2f pe = Vec2f(offsetEnd.x,   edgeAvgY);
      
          // whether horizontal line will overlap one of the rects
          bool sOverlapped = intersects(sRect, ps, pe);
          bool eOverlapped = intersects(eRect, ps, pe);

          path.push_back(offsetStart);
          {
            if(sOverlapped || eOverlapped)
              {
                // ps.y = (sCenter.y < eCenter.y) ? sRect.p2.y : sRect.p1.y;
                // pe.y = (sCenter.y < eCenter.y) ? eRect.p1.y : eRect.p2.y;

                bool sTop = (start.y - sRect.p1.y) < (sRect.p2.y - start.y) || (start.y - sRect.p1.y) < end.y;
                bool eTop = (end.y - eRect.p1.y) < (eRect.p2.y - end.y) || (end.y - eRect.p1.y) < start.y;
                
                ps.y = sTop ? sRect.p1.y : sRect.p2.y;
                pe.y = eTop ? eRect.p1.y : eRect.p2.y;

                float xAvg = (eRect.p2.x + sRect.p1.x)/2.0f;
                path.push_back(ps);
                path.push_back(Vec2f(xAvg, ps.y));
                path.push_back(Vec2f(xAvg, pe.y));
                path.push_back(pe);
              }
            else
              {
                path.push_back(ps);
                path.push_back(pe);
              }
          }
          path.push_back(offsetEnd);
        }
    }
  path.push_back(end);


  // TODO: avoid other node rects
  for(int i = 0; i < path.size(); i++)
    {

    }
  
  
  return path;
}
