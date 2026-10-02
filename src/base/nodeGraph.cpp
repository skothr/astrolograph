#include "nodeGraph.hpp"

#include "glfwKeys.hpp"
#include "imgui.h"

#include "nlohmann/json.hpp" // json definitions
using json = nlohmann::json;

#include <fstream>
#include "tools.hpp"
#include "imtools.hpp"

#include "mainWindow.hpp"
#include "geometry.hpp"
#include "viewSettings.hpp"
#include "labelNode.hpp"
#include "groupNode.hpp"
#include "timeNode.hpp"
#include "locationNode.hpp"
#include "chartNode.hpp"
#include "progressNode.hpp"
#include "compareNode.hpp"
#include "chartDataNode.hpp"
#include "chartViewNode.hpp"
#include "aspectNode.hpp"
#include "plotNode.hpp"
#include "marketDataNode.hpp"
#include "moonNode.hpp"
#include "vedicNode.hpp"
using namespace astro;

#include "neuralNetNode.hpp"

#include "staticFieldNode.hpp"
#include "mandelbrotNode.hpp"
#include "fftNode.hpp"
#include "fluidNode.hpp"
#include "hyperFluidNode.hpp"
#include "modularFluidNode.hpp"
#include "fieldViewNode.hpp"
#include "fieldOperators.hpp"
#include "fieldDelOperators.hpp"
#include "fieldChannels.hpp"


const std::unordered_map<std::string, NodeType> NodeGraph::NODE_TYPES =
  {{ "LabelNode",          {"LabelNode",             "Label Node",          [](){ return new LabelNode();          }} },
   { "TimeNode",           {"TimeNode",              "Time Node",           [](){ return new TimeNode();           }} },
   { "TimeSpanNode",       {"TimeSpanNode",          "Time Span Node",      [](){ return new TimeSpanNode();       }} },
   { "TimeShiftNode",      {"TimeShiftNode",         "Time Shift Node",     [](){ return new TimeShiftNode();      }} },
   { "LocationNode",       {"LocationNode",          "Location Node",       [](){ return new LocationNode();       }} },
   { "ChartNode",          {"ChartNode",             "Chart Node",          [](){ return new ChartNode();          }} },
   { "ProgressNode",       {"ProgressNode",          "Progress Node",       [](){ return new ProgressNode();       }} },
   { "ChartViewNode",      {"ChartViewNode",         "Chart View Node",     [](){ return new ChartViewNode();      }} },
   { "ChartCompareNode",   {"ChartCompareNode",      "Chart Compare Node",  [](){ return new CompareNode();        }} },
   { "ChartDataNode",      {"ChartDataNode",         "Chart Data Node",     [](){ return new ChartDataNode();      }} },
   { "AspectNode",         {"AspectNode",            "Aspect Node",         [](){ return new AspectNode();         }} },
   { "PlotNode",           {"PlotNode",              "Plot Node",           [](){ return new PlotNode();           }} },
   { "MarketDataNode",     {"MarketDataNode",        "Market Data Node",    [](){ return new MarketDataNode();     }} },
   { "MoonNode",           {"MoonNode",              "Moon Node",           [](){ return new MoonNode();           }} },
   { "VedicNode",          {"VedicNode",             "Vedic Node",          [](){ return new VedicNode();          }} },
   
   // fields
   { "NeuralNetNode",      {"NeuralNetNode",         "Neural Net Node",     [](){ return new NeuralNetNode();      }} },
   { "FieldViewNode",      {"FieldViewNode",         "Field View Node",     [](){ return new FieldViewNode();      }} },
   { "FieldChannelViewNode",{"FieldChannelViewNode", "Channel View Node",   [](){ return new FieldChannelViewNode(); }} },
   { "FFTNode",            {"FFTNode",               "FFT Node",            [](){ return new FFTNode();            }} },
   { "FluidNode",          {"FluidNode",             "Fluid Node",          [](){ return new FluidNode();          }} },
   { "HyperFluidNode",     {"HyperFluidNode",        "Hyper Fluid Node",    [](){ return new HyperFluidNode();     }} },
   { "MandelbrotNode",     {"MandelbrotNode",        "Mandelbrot Node",     [](){ return new MandelbrotNode();     }} },

   // modular fluids (TODO)
   // { "FluidStateNode",     {"FluidStateNode",     "Fluid State Node",    [](){ return new FluidStateNode();     }} },
   // { "StaticFieldNode",    {"StaticFieldNode",    "Static Field Node",   [](){ return new StaticFieldNode();    }} },
   
   // field operators
   { "FieldAddNode",       {"FieldAddNode",          "Field Add Node",      [](){ return new FieldAddNode();       }} },
   { "FieldAbsNode",       {"FieldAbsNode",          "Field Abs Node",      [](){ return new FieldAbsNode();       }} },
   { "FieldLogNode",       {"FieldLogNode",          "Field Log Node",      [](){ return new FieldLogNode();       }} },
   { "FieldExpNode",       {"FieldExpNode",          "Field Exp Node",      [](){ return new FieldExpNode();       }} },
   { "FieldMultNode",      {"FieldMultNode",         "Field Mult Node",     [](){ return new FieldMultNode();      }} },
   { "FieldNegNode",       {"FieldNegNode",          "Field Negate Node",   [](){ return new FieldNegNode();       }} },
   { "FieldMaxNode",       {"FieldMaxNode",          "Field Max Node",      [](){ return new FieldMaxNode();       }} },
   { "FieldNormNode",      {"FieldNormNode",         "Field Norm Node",     [](){ return new FieldNormNode();      }} },

   // channels
   { "ChannelSplitNode",     {"ChannelSplitNode",    "Channel Split Node",    [](){ return new ChannelSplitNode();     }} },
   { "ChannelCombineNode",   {"ChannelCombineNode",  "Channel Combine Node",  [](){ return new ChannelCombineNode();   }} },
   
   // ∇
   { "FieldGradNode",      {"FieldGradNode",         "Field Gradient Node", [](){ return new FieldGradNode();      }} },
   // { "FieldDDerivNode",    {"FieldDDerivNode",    "Field DDeriv Node",   [](){ return new FieldDDerivNode();    }} },
   { "FieldDivNode",       {"FieldDivNode",          "Field Divergence Node",[](){ return new FieldDivNode();       }} },
   { "FieldCurlNode",      {"FieldCurlNode",         "Field Curl Node",     [](){ return new FieldCurlNode();      }} },
  };

const std::vector<NodeGroup> NodeGraph::NODE_GROUPS =
  { {"Basic",         { "LabelNode" }},
    {"Data Input",    { "MarketDataNode" }},
    {"Parameter",     { "TimeNode",  "LocationNode", "TimeSpanNode", "TimeShiftNode" }},
    {"Calculation",   { "ChartNode", "ProgressNode", "VedicNode" }},
    {"Visualization", { "ChartViewNode", "ChartCompareNode", "ChartDataNode", "AspectNode", "PlotNode", "MoonNode" }},
    {"NN",            { "NeuralNetNode" }},
    {"Field",         { "FieldViewNode",    "FieldChannelViewNode", "FluidNode", "MandelbrotNode", }},
    {"Channel",       { "ChannelSplitNode", "ChannelCombineNode", }},

    {"Operator",      { "FieldAddNode", "FieldMultNode", "FieldNegNode",  "FieldAbsNode", "FieldMaxNode",  "FieldNormNode",
                        "FieldLogNode", "FieldExpNode",  "FieldGradNode", "FieldDivNode", "FieldCurlNode", "FFTNode" }},
    // {"Modular Fluid", { "FluidStateNode", "StaticFieldNode" }}, // TODO
    {"WIP",           {  }},
  };

Node* NodeGraph::makeNode(const std::string &nodeType)
{
  const auto &iter = NODE_TYPES.find(nodeType);
  if(iter != NODE_TYPES.end()) { return iter->second.get(); }
  else                         { return nullptr; }
}

NodeGraph::NodeGraph(MainWindow *window, ViewSettings *viewSettings)
  : mWindow(window), mViewSettings(viewSettings)
{
  mAddNodeJSON = new json;
  *mAddNodeJSON = json::array();
}

NodeGraph::~NodeGraph()
{
  clear();
  if(mAddNodeJSON)  { delete mAddNodeJSON; }
}

//// JSON CONVERSION ////

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
  mUnsavedChanges = false;
  std::cout << "=============================================================================================\n";
  if(version != SAVE_FILE_VERSION)
    {
      std::cout << "= WARNING: Save file may be out of date -- file may not have loaded properly. (current version: " << SAVE_FILE_VERSION << ")\n";
      std::cout << "=============================================================================================\n";
    }
  return true;
}


void NodeGraph::addNode(Node *n, bool select)
{
  if(n && !mLocked)
    {
      std::cout << "ADDING NODE -->\n";
      std::cout << n << "\n";
      
      mUnsavedChanges = true;

      if(n->id() < 0) { n->setId(NEXT_ID++); }
      // TODO: check if id should be an offset?
      if(mNodes.find(n->id()) != mNodes.end())
        { n->setId(NEXT_ID++); } // new id if it already taken
      
      n->setGraph(this);
      mNodes.emplace(n->id(), n);
      if(select) { deselectAll(); n->setSelected(true); }

      // normal alpha
      Vec4f mask = n->getColorMask();
      mask.w = 1.0f;
      n->setColorMask(mask);

      mAddNodeJSON->push_back(n->id());
    }
}

void NodeGraph::addNodes(std::vector<Node*> &nodes, bool select)
{
  for(auto n : nodes)
    { addNode(n, select); }
  doneAdding();
  NEXT_ID += nodes.size(); // TODO: check
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

void NodeGraph::removeNodes(std::vector<Node*> &nodes, bool deleteNodes)
{
  for(auto n : nodes)
    {
      mNodes.erase(n->id());
      if(deleteNodes) { delete n; }
    }
}


void NodeGraph::clear()
{
  mUndoStack.clear();
  for(auto n : mNodes) { delete n.second; }
  mNodes.clear();
  mNodes = std::unordered_map<int, Node*>(); // clear nodes and free allocation (?)
  NEXT_ID = 0;
  mGraphCenter = Vec2f(0,0);
  mGraphScale  = 1.0f;
  mUnsavedChanges = false;
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
            std::cout << " --> REMOVING NODES\n";
            json removed = json::array();
            for(auto jsid : js)
              {
                int id = jsid.get<int>();
                auto iter = mNodes.find(id);
                if(iter != mNodes.end())
                  {
                    removed.push_back(iter->second->toJSON());
                    delete iter->second;
                    mNodes.erase(id);
                  }
                else { std::cout << "WARNING: Could not find node to remove! (" << id << ")\n"; }
              }
            
            if(removed.size() > 0)
              { mRedoStack.push_back(Action{ACTION_ADD_NODES, removed}); }
          }
          break;
        case ACTION_MOVE_NODES:
          {
            std::cout << "[MOVE NODES]\n";
            json js = std::any_cast<json>(a.data);
            std::cout << "UN-MOVING NODES\n";
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
            std::cout << "ADDING NODES\n";
            
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
            std::cout << "RE-MOVING NODES\n";
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
  ConnectorBase *connectingFrom = nullptr;
  for(auto n : mNodes)
    { if(n.second->isConnecting()) { return true; } }
  return false;
}

ConnectorBase* NodeGraph::getConnectingFrom()
{
  ConnectorBase *connectingFrom = nullptr;
  for(auto n : mNodes)
    {
      if(n.second->isConnecting())
        {
          for(auto con : n.second->inputs())  { if(con->isConnecting()) { connectingFrom = con; break; } }
          if(connectingFrom) { break; }
          for(auto con : n.second->outputs()) { if(con->isConnecting()) { connectingFrom = con; break; } }
          if(connectingFrom) { break; }
        }
    }
  return connectingFrom;
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
      mUnsavedChanges = true;
      mMovingNodes    = true;
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
  mCopying     = false;
  mMovingNodes = false;
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
      if(disconnectInputs)
        { // inputs
          for(auto c : n->getInputConnections())
            {
              if(std::find(group.begin(), group.end(), mNodes[c.nodeOut]) == group.end())
                { n->inputs()[c.conIn]->disconnect(mNodes[c.nodeOut]->outputs()[c.conOut]); }
            }
        }
      if(disconnectOutputs)
        { // outputs
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
          //addNode(n, false);
          js.push_back(n->toJSON());
        }
      mUndoStack.push_back(Action{ACTION_COPY_NODES, js});
      // doneAdding();
      
      NEXT_ID += newNodes.size();
      mUnsavedChanges = true;
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
        { mNodes.erase(n->id()); }
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
            { mNodes.erase(n->id()); }
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



//// INPUT ////

void NodeGraph::handleInput()
{
  if(!mLocked)
    {
      ImGuiIO &io = ImGui::GetIO();
      Rect2f gRect = screenToGraph(Rect2f(mViewPos, mViewPos + mViewSize)); // view bounds in graph space
      
      // mouse info
      Vec2f mp  = ImGui::GetMousePos(); // mouse pos in screen space
      Vec2f gmp = screenToGraph(mp);    // mouse pos in graph space
      bool  lbClick = ImGui::IsMouseClicked(ImGuiMouseButton_Left  ); // click --> true if button newly pressed
      bool  mbClick = ImGui::IsMouseClicked(ImGuiMouseButton_Middle);
      bool  lbDown  = ImGui::IsMouseDown(ImGuiMouseButton_Left  );    // down  --> true if button held
      bool  mbDown  = ImGui::IsMouseDown(ImGuiMouseButton_Middle);

      bool overGraph = isHovered();              // true if mouse is over graph window
      bool bgHover   = ImGui::IsWindowHovered(); // true if mouse is directly over graph background
      bool placingNode = (mWindow->isPasting() || mWindow->isPlacing()); // true if adding a node


      // check if mouse is hovering over a node, or if node UI is active
      //  TODO: already class member?
      bool nodeActive = false;
      bool nodeHover  = false;
      for(auto n : mNodes)
        {
          nodeActive |= n.second->isActive();
          nodeHover  |= n.second->isHovered() || n.second->rect().intersection(gRect).contains(gmp);
        }


      
      // ESCAPE key --> deselect all nodes
      if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) { deselectAll(); }
      
      // DELETE key --> delete selected nodes (unless interacting, e.g. for text input)
      if(!nodeActive && ImGui::IsKeyPressed(GLFW_KEY_DELETE))
        {
          for(auto n : mSelectedNodes) { mNodes.erase(n->id()); delete n; mUnsavedChanges = true; }
          mSelectedNodes.clear();
        }

      // stop panning if mouse released (unconditionally)
      if(mPanning && !(lbDown || mbDown)) { mPanning  = false; mPanClick = gmp; }
      // stop selecting if mouse released (unconditionally)
      if(mSelecting && !lbDown) { mSelecting = false; mSelectAnchor = Vec2f(); mSelectRect = Rect2f(); }

      
      if(!nodeActive)
        {
          if(!placingNode)
            {
              if(bgHover)
                { // mouse directly over background
                  if(mbClick || (io.KeyShift && lbClick))
                    { // pan view center (middle click-drag or SHIFT + left click-drag)
                      mPanning  = true;
                      mPanClick = gmp;
                      ImGui::ResetMouseDragDelta(lbClick ? ImGuiMouseButton_Left : ImGuiMouseButton_Middle);
                    }
                  else if(!mSelecting && lbClick)
                    { // start drawing selection rectangle
                      mSelecting = true;
                      mSelectAnchor  = screenToGraph(ImGui::GetMousePos());
                      mSelectRect.p1 = mSelectAnchor;
                      mSelectRect.p2 = mSelectAnchor;
                      if(!io.KeyCtrl) { deselectAll(); }
                    }
                }
              
            }
          else
            { // placing or pasting new node
              if(mbClick)
                { // pan view center (only middle click-drag, shift used to multi-paste)
                  mPanning = true;
                  mPanClick = gmp;
                  ImGui::ResetMouseDragDelta(lbClick ? ImGuiMouseButton_Left : ImGuiMouseButton_Middle);
                }
              if(mPanning && !mbDown)
                {
                  mPanning = false;
                  mPanClick = gmp;
                } // stop panning
            }

        }

      if(mSelecting)
        { // selection rect (click+drag)
          if(ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            {
              Rect2f select(Vec2f(std::min(mSelectAnchor.x, gmp.x), std::min(mSelectAnchor.y, gmp.y)),
                            Vec2f(std::max(mSelectAnchor.x, gmp.x), std::max(mSelectAnchor.y, gmp.y)));
              mSelectRect = select.fixed().intersection(gRect);
            }
          // select nodes that intersect selection rect
          for(auto n : mNodes) { n.second->setSelected((n.second->isSelected() && io.KeyCtrl) || n.second->rect().intersects(mSelectRect)); }
          
          // draw selection rect in foreground
          ImGui::GetForegroundDrawList()->AddRect(graphToScreen(mSelectRect.p1), graphToScreen(mSelectRect.p2),
                                                  ImColor(Vec4f(1.0f,1.0f,1.0f,0.5f)), 0.0f, ImDrawCornerFlags_All, 3.0f);
        }


      
      
      if(overGraph && !nodeActive)
        {
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
      
          // MOUSE SCROLL --> zoom/scale graph view
          if(io.KeyCtrl && std::abs(io.MouseWheel) > 0.0f)
            {
              mScaling = true;
              float vel = GRAPH_SCALE_VEL;
          
              float oldScale = mGraphScale;
              mGraphScale *= (io.MouseWheel > 0.0f ? vel : 1.0f/vel);
              mGraphScale = std::min(GRAPH_SCALE_MAX, std::max(mGraphScale, GRAPH_SCALE_MIN));
              
              // adjust center so gmp remains constant
              Vec2f gmpNew = screenToGraph(mp);
              mGraphCenter += gmpNew - gmp;
              gmp = gmpNew;
              
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
          else { mScaling = false; }
        }

      
      // right click over background --> context menu
      if(!nodeActive && ImGui::BeginPopupContextWindow("nodeGraphContext"))
        {
          if(ImGui::MenuItem("Recenter"))    { mGraphCenter = Vec2f(0,0); }
          if(ImGui::MenuItem("Reset Scale")) { mGraphScale = 1.0f; }
          ImGui::EndPopup();
        }
    }
}



//// DRAWING ////

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
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(0, 0));
  ImGui::PushStyleColor(ImGuiCol_ChildBg, mViewSettings->graphBgColor);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, mViewSettings->graphBgColor);
  ImGui::BeginChild("nodeGraph", mViewSize, true, wFlags);
  ImGui::PopStyleColor(2);
  ImGui::PopStyleVar();
  mDrawing = true;
  g_closeContexts = (mPanning || mScaling || mSelecting || mCopying);
}

void NodeGraph::EndDraw()
{
  mDrawing = false;
  ImGui::EndChild();
  ImGui::PopStyleVar(2);
  g_contextsOpen = 0;
}

void NodeGraph::drawGrid(ImDrawList *drawList)
{
  Vec2f graphTL   = screenToGraph(mViewPos);           // graph coordinates of top-left corner of view
  Vec2f graphBR   = screenToGraph(mViewPos+mViewSize); // graph coordinates of bottom-right corner of view
  Vec2f gViewSize = screenToGraphVec(mViewSize);       // size of nodegraph view in graph space
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

void NodeGraph::drawNodes(const std::vector<std::pair<int, Node*>> &nodes)
{
  // draw node connections (drawn underneath node UIs)
  Node *hoveredNode = getHovered();
  float connectedW  = 3.0f*mGraphScale; // line width while connected
  float connectingW = 1.5f*mGraphScale; // line width while actively dragging and making connection
  Vec2f gmp         = screenToGraph(ImGui::GetMousePos()); // mouse pos in graph space
  ImDrawList *fgDrawList = ImGui::GetForegroundDrawList();
  fgDrawList->_FringeScale = mGraphScale;
  
  std::vector<ConnectionPath> paths;
  std::vector<ConnectorBase*> cActive;
  std::vector<ConnectorBase*> cInactive;
  
  // find all active connections
  for(auto iter : nodes)
    {
      Node *n = iter.second;
      Rect2f nRect = n->rect();
      for(auto c1 : n->connectors())
        {
          Vec2f mp          = gmp;
          Vec2f offsetPos   = c1->graphPos;
          Vec2f protrudePos = c1->getProtrudePos();
          bool  activePaths = false;
          
          if(c1->isConnecting())
            { // connector is actively being connected via mouse
              activePaths = true;
              Rect2f rect = Rect2f(mp, mp);
              ConnectorBase *c2 = nullptr; // by default, just find a path to the mouse
              if(hoveredNode && hoveredNode != n && hoveredNode->connectingTo())
                { // snap the connection to this connector instead of mouse
                  c2 = hoveredNode->connectingTo();
                  if(c2 != c1 && c1->typeValid(c2) && c2->direction() != c1->direction())
                    {
                      rect = hoveredNode->rect();
                      mp   = c2->graphPos;
                    }
                }
              if(c1->direction() == CONNECTOR_OUTPUT)
                { paths.push_back(ConnectionPath{ c1, c2, offsetPos, mp, nRect, rect, c1->getLineColor() }); }
              else
                { paths.push_back(ConnectionPath{ c2, c1, mp, offsetPos, rect, nRect, c1->getLineColor() }); }
            }
          if(c1->isConnected())
            { // connector has established connection(s)
              activePaths = true;
              if(c1->direction() == CONNECTOR_OUTPUT)
                {
                  for(auto c2 : c1->getConnected())
                    {
                      if(!c2->parent()->getShowConnections()) { continue; }
                      Vec2f offsetPos2 = c2->graphPos;
                      Vec2f protrudePos2 = c2->getProtrudePos();
                      if(c1->direction() == CONNECTOR_OUTPUT)
                        { paths.push_back(ConnectionPath{ c1, c2, offsetPos, offsetPos2, nRect, c2->parent()->rect(), c1->getLineColor() }); }
                      else
                        { paths.push_back(ConnectionPath{ c2, c1, offsetPos2, offsetPos, c2->parent()->rect(), nRect, c1->getLineColor() }); }
                    }
                }
            }
          if(activePaths) { cActive.push_back(c1);   }
          else            { cInactive.push_back(c1); }
        }
    }

  // [...?]
    
  // draw active connection paths
  for(auto &p : paths)
    {
      // find suitable path (TODO: improve)
      std::vector<Vec2f> connectLines = findOrthogonalPath(p.p1, p.r1, p.p2, p.r2, p.c1 ? p.c1->direction() : CONNECTOR_OUTPUT);
      // draw connection path
      for(int i = 0; i < connectLines.size()-1; i++)
        { mWinDrawList->AddLine(graphToScreen(connectLines[i]), graphToScreen(connectLines[i+1]), ImColor(p.color), connectedW); }
      // draw dot on connection endpoint
      Vec4f dotColor = p.c2 ? p.c2->getDotColor() : p.c1->getDotColor();
      for(int i = 1; i < connectLines.size()-1; i++)
        { mWinDrawList->AddCircleFilled(graphToScreen(connectLines[i]), 3.0f*mGraphScale, ImColor(dotColor), 32); }
    }
    
  // draw nodes
  bool posChanged = false;
  for(int i = 0; i < nodes.size(); i++) // draw from back to front
    {
      Node *n = nodes[i].second;
      Vec2f p0 = n->pos();
      nodes[i].second->draw(mWinDrawList, mBlocked[i]);
      Vec2f p1 = n->pos();

      if(p1 != p0) { posChanged = true; }
        
      if(mShowIds)
        {
          ImGui::SetCursorPos(graphToScreen(nodes[i].second->pos()) - mViewPos - Vec2f(0.0f, 20.0f));
          ImGui::Text("%d", nodes[i].second->id());
        }
    }

  // draw dots, and overlay active connections over node drawlist
  ConnectorBase *nConnecting = getConnectingFrom();
  for(auto c : cInactive)
    {
      Node *n = c->parent();
      if(c->isConnecting() || c->isConnected() || (hoveredNode && c == hoveredNode->connectingTo() && nConnecting && c->direction() != nConnecting->direction()))
        {
          Rect2f sRect = graphToScreen(n->rect());
          n->BeginDraw();
          Rect2f graphRect  = Rect2f(mViewPos, mViewPos + mViewSize);
          Rect2f borderRect = sRect.expanded(n->getBorderWidth()/2.0f*mGraphScale);
          ImGui::PushClipRect(graphRect.p1.getFloor(),  graphRect.p2.getCeil(),  false); // extend clipping to full graph
          ImGui::PushClipRect(borderRect.p1.getFloor(), borderRect.p2.getCeil(), true);  // clamp to border around node
          n->drawList()->AddLine(graphToScreen(c->graphPos), graphToScreen(c->getProtrudePos()), ImColor(c->getLineColor()), connectedW);
          ImGui::PopClipRect(); ImGui::PopClipRect();
          n->EndDraw();
        }
      n->drawList()->AddCircleFilled(graphToScreen(c->graphPos), CONNECTOR_POINT_RADIUS*mGraphScale, ImColor(c->getDotColor()), 32);
    }
  for(auto c : cActive)
    {
      Node *n = c->parent();
      if(c->isConnecting() || c->isConnected() || (hoveredNode && c == hoveredNode->connectingTo() && nConnecting && c->direction() != nConnecting->direction()))
        {
          Rect2f sRect = graphToScreen(n->rect());
          n->BeginDraw();
          Rect2f graphRect  = Rect2f(mViewPos, mViewPos + mViewSize);
          Rect2f borderRect = sRect.expanded(n->getBorderWidth()/2.0f*mGraphScale);
          ImGui::PushClipRect(graphRect.p1.getFloor(),  graphRect.p2.getCeil(),  false); // extend clipping to full graph
          ImGui::PushClipRect(borderRect.p1.getFloor(), borderRect.p2.getCeil(), true);  // clamp to border around node
          n->drawList()->AddLine(graphToScreen(c->graphPos), graphToScreen(c->getProtrudePos()), ImColor(c->getLineColor()), connectedW);
          ImGui::PopClipRect(); ImGui::PopClipRect();
          n->EndDraw();
        }
      n->drawList()->AddCircleFilled(graphToScreen(c->graphPos), CONNECTOR_POINT_RADIUS*mGraphScale, ImColor(c->getDotColor()), 32);
    }
}

void NodeGraph::draw()
{
  BeginDraw();
  {
    Rect2f gRect = screenToGraph(Rect2f(mViewPos, mViewPos + mViewSize));
    Vec2f  gmp   = screenToGraph(ImGui::GetMousePos());
    
    mWinDrawList = ImGui::GetWindowDrawList();
    ImDrawList *fgDrawList = ImGui::GetForegroundDrawList();
    mWinDrawList->_FringeScale = getScale();
    fgDrawList->_FringeScale   = getScale();
    
    // apply user input
    handleInput();
    
    // reset click copy flag if mouse released
    if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) { mClickCopied = false; }
    
    // keep track of whether graph view was the last thing clicked (NOTE: hovering to bring in focus?) TODO: rename to "mFocused"
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { mInFocus = isHovered(); }
    else if(isHovered()) { mInFocus = true; }

    // fix node positions (no overlapping) -- TODO(?)
    fixPositions();
    
    // update list of selected nodes
    mSelectedNodes.clear();
    for(auto n : mNodes)
      { if(n.second->isSelected()) { mSelectedNodes.push_back(n.second); } }
    // move selected nodes to front
    if(!mSelecting) { for(auto n : mSelectedNodes) { n->bringToFront(); } }
    
    // sort nodes by z order
    std::vector<std::pair<int, Node*>> sorted(mNodes.begin(), mNodes.end());
    std::sort(sorted.begin(), sorted.end(),
              [](const std::pair<int, Node*> &n1, const std::pair<int, Node*> &n2) -> bool
              { return (n1.second->getZ() < n2.second->getZ()) || (n1.second->getZ() == n2.second->getZ() && n1.first < n2.first); });
    // clean up z ordering
    for(int i = 0; i < sorted.size(); i++) { sorted[i].second->setZ(i); }
    
    // block input to nodes behind topmost
    mBlocked.clear();
    mBlocked.resize(sorted.size(), false);
    bool mouseBlocked = false;
    for(int i = sorted.size()-1; i >= 0; i--)
      {
        Node *n = sorted[i].second;
        bool b = n->rect().expanded(-1.0f).contains(gmp);
        mBlocked[i] = mouseBlocked;
        mouseBlocked |= b;
      }
    // check for mouse hovering over node (topmost only)
    mHoveredNode = nullptr;
    for(int i = 0; i < sorted.size(); i++) // sort from back to front for drawing (TODO: improve ordering system)
      {
        Node *n = sorted[i].second;
        if(!mBlocked[i] && n->rect().contains(gmp)) { mHoveredNode = n; }
      }    
    
    // draw background grid and nodes
    drawGrid(mWinDrawList);    
    drawNodes(sorted);
  }
  EndDraw();
}




//// UPDATE STEP ////

void NodeGraph::update(double dt)
{  
  //if(!mScaling) { mLastT = std::chrono::high_resolution_clock::now(); }
  //else { mDt += dt; }
  
  bool changed = false;

  //// determine node update order (based on connection dependencies)
  std::vector<Node*> startNodes;   // nodes with no input dependencies
  std::vector<Node*> endNodes;     // nodes with no input dependencies
  std::vector<Node*> intermediate; // nodes that have both inputs and outputs
  std::vector<Node*> disconnected; // nodes with no connections
  for(auto iter : mNodes)
   { 
      Node *n = iter.second;
      std::vector<Node::Connection> inputs  = n->getInputConnections();
      std::vector<Node::Connection> outputs = n->getOutputConnections();
      bool hasInputs  = (n->inputs().size()  == 0 || inputs.size()  != 0);
      bool hasOutputs = (n->outputs().size() == 0 || outputs.size() != 0);
      if(hasInputs && hasOutputs) { intermediate.push_back(n); } // both
      else if(!hasInputs)         { startNodes.push_back(n);   } // just inputs
      else if(!hasOutputs)        { endNodes.push_back(n);     } // just outputs
      else                        { disconnected.push_back(n); } // no connections
    }

  //// TODO: pull from input nodes to request data
  for(int i = 0; i < mNodes.size(); i++)
    {
      // update start nodes
      for(auto n : startNodes)   { n->update(); changed |= n->hasChanged(); }
      // update disconnected nodes
      for(auto n : endNodes)     { n->update(); changed |= n->hasChanged(); }  
      // update intermediate nodes
      for(auto n : intermediate) { n->update(); changed |= n->hasChanged(); }  
      // update end nodes
      for(auto n : disconnected) { n->update(); changed |= n->hasChanged(); }

      // check if any node updates still pending
      std::vector<Node*> noUpdate;
      for(auto iter : mNodes) { Node *n = iter.second; if(!n->updated()) { noUpdate.push_back(n); } }
      
      // print nodes that failed to update
      if(noUpdate.size() > 0) { std::cout << "Failed to update Nodes (" << noUpdate.size() << ") -->\n"; }
      else { break; } // all nodes were updated

      for(auto n : noUpdate) { std::cout << "(i=" << i << ")    --> " << n->name() << " / " << n->id() << "\n"; }
    }

  mUnsavedChanges |= changed; // mark if graph has changed
  
  for(auto n : mNodes)     { n.second->resetUpdate(); }
  for(auto n : startNodes) { n->setChanged(false);    } // reset changed state
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
