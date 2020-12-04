#include "node.hpp"
using namespace astro;

#include <string>

#include "imgui.h"
#include "chart.hpp"
#include "nodeGraph.hpp"
#include "viewSettings.hpp"
  
static std::unordered_map<std::string, Vec4f> CONNECTOR_COLORS =
  {{std::string(typeid(DateTime).name()), Vec4f(0.2f, 0.2f, 1.0f, 1.0f)},
   {std::string(typeid(Location).name()), Vec4f(0.2f, 1.0f, 0.2f, 1.0f)},
   {std::string(typeid(Chart).name()),    Vec4f(1.0f, 0.2f, 0.2f, 1.0f)}};


bool ConnectorBase::connect(ConnectorBase *other, bool force)
{
  if(!other || other == this || type() != other->type() || mDirection == other->mDirection) { return false; }
  
  // disconnect if already connected
  for(auto con : mConnected)
    {
      if(con == other)
        {
          disconnect(con);
          if(force) { break; }        // continue connecting anyway
          else      { return false; } // connection failed
        }
    }

  // inputs can only have one connection
  if(mDirection == CONNECTOR_INPUT && mConnected.size() > 0)
    { disconnect(mConnected[0]); }
  else if(other->mDirection == CONNECTOR_INPUT && other->mConnected.size() > 0)
    { other->disconnect(other->mConnected[0]); }

  other->mConnected.push_back(this);
  mConnected.push_back(other);
  
  other->parent()->onConnect(other);
  mParent->onConnect(this);
  return true;
}

void ConnectorBase::disconnect(ConnectorBase *other)
{
  if(!other || other == this) { return; }
  for(int i = 0; i < mConnected.size(); i++)
    {
      ConnectorBase *con = mConnected[i];
      if(con == other)
        {
          mConnected.erase(mConnected.begin() + i);
          // disconnect other
          for(int j = 0; j < con->mConnected.size(); j++)
            {
              if(con->mConnected[j] == this)
                {
                  con->mConnected.erase(con->mConnected.begin() + j);
                  break;
                }
            }
          break;
        }
    }
}

void ConnectorBase::disconnectAll()
{
  // disconnect all connections
  for(int i = 0; i < mConnected.size(); i++)
    {
      ConnectorBase *con = mConnected[i];
      for(int j = 0; j < con->mConnected.size(); j++)
        {
          if(con->mConnected[j] == mThisPtr)
            { con->mConnected.erase(con->mConnected.begin() + j--); }
        }
    }
  mConnected.clear();
}

void ConnectorBase::sendSignal(NodeSignal signal)
{
  mSignals = (NodeSignal)(mSignals | signal);
  for(auto con : mConnected) { con->sendSignal(signal); }
}

void ConnectorBase::draw(bool blocked)
{
  // calculate screen position (center of connector) for connection lines
  //graphPos = mParent->getGraph()->screenToGraph(Vec2f(ImGui::GetCursorScreenPos()) + CONNECTOR_SIZE/2.0f);// - mParent->getGraph()->getCenter();

  float scale = mParent->getScale();
  Vec2f conPadding = CONNECTOR_PADDING*scale;
  Vec2f conSize = CONNECTOR_SIZE*scale;
  
  // get color
  Vec4f connectorColor(0.5f, 0.5f, 0.5f, 1.0f);
  auto iter = CONNECTOR_COLORS.find(type());
  if(iter != CONNECTOR_COLORS.end())
    { connectorColor = iter->second; }
  
  // draw connector button 
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, CONNECTOR_ROUNDING*scale);
  ImGui::PushStyleColor(ImGuiCol_Button, connectorColor);
  ImGui::Button(("##"+mName).c_str(), conSize);
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();

  // input handling
  if(!blocked && ImGui::IsItemHovered())
    {
      ImGui::BeginTooltip();
      ImGui::TextUnformatted(mName.c_str());
      ImGui::EndTooltip();

      if(ImGui::IsItemClicked(ImGuiMouseButton_Middle))  { disconnectAll(); }   // MIDDLE CLICK -- disconnect all
      if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))   { beginConnecting(); } // LEFT MOUSE DOWN -- start connecting
    }
  
  // drag/drop
  if(ImGui::BeginDragDropSource())
    {
      if(mDirection == CONNECTOR_INPUT && mConnected.size() > 0)
        { // connecting from connected output connector instead of this connector
          endConnecting();
          mConnected[0]->beginConnecting();
          ImGui::SetDragDropPayload((("CON_OUT")+type()).c_str(), &mConnected[0]->mThisPtr, sizeof(ConnectorBase*));
        }
      else
        {
          ImGui::SetDragDropPayload(((mDirection == CONNECTOR_INPUT ? "CON_IN" : "CON_OUT")+type()).c_str(), &mThisPtr, sizeof(ConnectorBase*));
        }
      ImGui::EndDragDropSource();
    }
  if(ImGui::BeginDragDropTarget())
    {
      const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(((mDirection == CONNECTOR_INPUT ? "CON_OUT" : "CON_IN")+type()).c_str());
      if(payload)
        {
          ConnectorBase *source = *((ConnectorBase**)payload->Data);
          if(source)
            {
              std::cout << "Connecting " << source->parent()->id() << " to " << parent()->id() << "\n";
              connect(source);
            }
        }
      ImGui::EndDragDropTarget();
    }
  if(ImGui::BeginPopupContextItem(("nodeContext"+std::to_string(conId())).c_str()))
    {
      if(ImGui::MenuItem("Disconnect All")) { disconnectAll(); }
      ImGui::EndPopup();
    }
  if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) { endConnecting(); }   // LEFT MOUSE UP -- stop connecting
}

void ConnectorBase::drawConnections(ImDrawList *nodeDrawList, ImDrawList *graphDrawList)
{
  float scale = mParent->getScale();
  
  // get color
  Vec4f connectorColor(0.5f, 0.5f, 0.5f, 1.0f);
  auto iter = CONNECTOR_COLORS.find(type());
  if(iter != CONNECTOR_COLORS.end()) { connectorColor = iter->second; }
  Vec4f connectingColor = Vec4f(connectorColor.x, connectorColor.y, connectorColor.z, connectorColor.w*0.8f);                 // color when making connection
  Vec4f connectedColor  = Vec4f(connectorColor.x*0.75f,  connectorColor.y*0.75f,  connectorColor.z*0.75f,  connectorColor.w); // color when fully connected
  Vec4f dotColor        = Vec4f(connectorColor.x*0.4f,   connectorColor.y*0.4f,   connectorColor.z*0.4f,   1.0f);             // color of connector dot
  Vec4f connectDotColor = Vec4f(connectorColor.x*0.65f,  connectorColor.y*0.65f,  connectorColor.z*0.65f,  1.0f);             // color of connection dot

  float connectingW = 1.5f*scale;
  float connectedW = 3.0f*scale;

  NodeGraph *graph = mParent->getGraph();
  
  Vec2f offsetPos = graphPos;
  Vec2f protrudePos = getProtrudePos();
  
  std::vector<Vec2f> connectLines;
  // connecting (draw line to mouse)
  if(isConnecting())
    { // use foreground drawlist (only while actively connecting, otherwise connections will show above file dialog)
      ImDrawList *fgDrawList = ImGui::GetForegroundDrawList();
      if(mDirection == CONNECTOR_OUTPUT) { connectLines = graph->findOrthogonalPath(offsetPos, graph->screenToGraph(ImGui::GetMousePos())); }
      else                               { connectLines = graph->findOrthogonalPath(graph->screenToGraph(ImGui::GetMousePos()), offsetPos); }
      for(int i = 0; i < connectLines.size()-1; i++)
        { fgDrawList->AddLine(graph->graphToScreen(connectLines[i]), graph->graphToScreen(connectLines[i+1]), ImColor(connectingColor), connectingW); }
      for(int i = 1; i < connectLines.size()-1; i++)
        { fgDrawList->AddCircleFilled(graph->graphToScreen(connectLines[i]), 3.0f*scale, ImColor(connectDotColor), 20); }
    }
  else if(mConnected.size() > 0)
    { // draw connection line(s)
      for(auto con : mConnected)
        {
          Vec2f offsetPos2 = con->graphPos;
          Vec2f protrudePos2 = con->getProtrudePos();
          if(mDirection == CONNECTOR_OUTPUT) { connectLines = graph->findOrthogonalPath(offsetPos, offsetPos2); }
          else                               { connectLines = graph->findOrthogonalPath(offsetPos2, offsetPos); }          
          for(int i = 0; i < connectLines.size()-1; i++)
            { graphDrawList->AddLine(graph->graphToScreen(connectLines[i]), graph->graphToScreen(connectLines[i+1]), ImColor(connectedColor), connectedW); }
          for(int i = 1; i < connectLines.size()-1; i++)
            { graphDrawList->AddCircleFilled(graph->graphToScreen(connectLines[i]), 3.0f*scale, ImColor(connectDotColor), 20); }
        }
    }
  if(isConnecting() || mConnected.size() > 0 && connectLines.size() > 1)
    { // draw first and last lines over node window
      nodeDrawList->AddLine(graph->graphToScreen(connectLines[0]), graph->graphToScreen(connectLines[1]),
                            ImColor(connectedColor), 3.0f*scale);
      nodeDrawList->AddLine(graph->graphToScreen(connectLines[connectLines.size()-2]), graph->graphToScreen(connectLines[connectLines.size()-1]),
                            ImColor(connectedColor), 3.0f*scale);
    }
  // draw connection dot
  nodeDrawList->AddCircleFilled(graph->graphToScreen(offsetPos), 5.0f*scale, ImColor(dotColor), 12);
}


int Node::NEXT_ID = 0;

Node::Node(const std::vector<ConnectorBase*> &inputs_, const std::vector<ConnectorBase*> &outputs_, const std::string &name, Params *params)
  : mInputs(inputs_), mOutputs(outputs_), mParams(params)
{
  if(!mParams) { mParams = new Params(); }
  if(mParams->id < 0) { mParams->id = NEXT_ID++; } // set id
  mParams->name = name;
  for(int i = 0; i < mInputs.size(); i++)  { mInputs[i]->setParent(this, i);  mInputs[i]->setDirection(CONNECTOR_INPUT); }
  for(int i = 0; i < mOutputs.size(); i++) { mOutputs[i]->setParent(this, i); mOutputs[i]->setDirection(CONNECTOR_OUTPUT); }
  setSize(mMinSize);
}

Node::~Node()
{
  disconnectAll();
  for(auto c : mInputs)  { delete c; }
  for(auto c : mOutputs) { delete c; }
  delete mParams;
}

void Node::setPos(const Vec2f &p)
{
  mParams->rect.setPos(p);
  bringToFront();
}

float Node::getScale() const
{ return mGraph->getScale(); }

ViewSettings* Node::getViewSettings()
{ return mGraph->getViewSettings(); }

void Node::bringToFront()
{
  mParams->z = NODE_TOP_Z;
}

std::vector<Node::Connection> Node::getInputConnections()
{
  std::vector<Connection> connections;
  for(int i = 0; i < inputs().size(); i++)
    {
      std::vector<ConnectorBase*> connected = inputs()[i]->getConnected();
      for(int j = 0; j < connected.size(); j++)
        { connections.push_back(Connection{ connected[j]->parent()->id(), connected[j]->conId(), id(), inputs()[i]->conId() }); }
    }
  return connections;
}
std::vector<Node::Connection> Node::getOutputConnections()
{
  std::vector<Connection> connections;
  for(int i = 0; i < outputs().size(); i++)
    {
      std::vector<ConnectorBase*> connected = outputs()[i]->getConnected();
      for(int j = 0; j < connected.size(); j++)
        { connections.push_back(Connection{ id(), outputs()[i]->conId(), connected[j]->parent()->id(), connected[j]->conId() }); }
    }
  return connections;
}
std::vector<Node::Connection> Node::getConnections()
{
  std::vector<Connection> connections  = getInputConnections();
  std::vector<Connection> oConnections = getOutputConnections();
  connections.insert(connections.end(), oConnections.begin(), oConnections.end());
  return connections;
}

void Node::disconnectAll()
{
  for(auto c : inputs())  { c->disconnectAll(); }
  for(auto c : outputs()) { c->disconnectAll(); }
}

void Node::drawConnections(ImDrawList *graphDrawList)
{
  Rect2f sRect = mGraph->graphToScreen(rect());
  
  // TODO: improve drawing repetition
  BeginDraw();
  {
    ImDrawList *winDrawList = ImGui::GetWindowDrawList();
    // over output child
    if(mOutputs.size() > 0)
      {
        ImGui::BeginChild(("nodeOutputs"+std::to_string(id())).c_str());
        ImDrawList *conDrawList = ImGui::GetWindowDrawList();
        // draw connection lines over window area
        for(auto con : mOutputs) { con->drawConnections(conDrawList, graphDrawList); }
        ImGui::EndChild();
      }
    // over input child
    if(mInputs.size() > 0)
      {
        ImGui::BeginChild(("nodeInputs"+std::to_string(id())).c_str());
        ImDrawList *conDrawList = ImGui::GetWindowDrawList();
        for(auto con : mInputs) { con->drawConnections(conDrawList, graphDrawList); }
        ImGui::EndChild();
      }
    
    // draw over border
    Rect2f graphRect(mGraph->viewPos(), mGraph->viewPos()+mGraph->viewSize());
    Rect2f borderRect = sRect.expanded(getBorderWidth()*2.0f);
    ImGui::PushClipRect(graphRect.p1.getFloor(),  graphRect.p2.getCeil(),  false); // extend clipping to full graph
    ImGui::PushClipRect(borderRect.p1.getFloor(), borderRect.p2.getCeil(), true);  // clamp to border around node
    for(auto con : mOutputs) { con->drawConnections(winDrawList, graphDrawList); }
    for(auto con : mInputs)  { con->drawConnections(winDrawList, graphDrawList); }
    ImGui::PopClipRect();
    ImGui::PopClipRect();
  }
  EndDraw();
}

bool Node::isConnecting() const
{
  for(auto con : mInputs)  { if(con->isConnecting()) { return true; } }
  for(auto con : mOutputs) { if(con->isConnecting()) { return true; } }
  return false;
}

bool Node::BeginDraw()
{
  if(!mDrawing)
    {
      ImGuiWindowFlags wFlags = (ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(0,0));
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vec2f(0,0));
      ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, NODE_ROUNDING*mGraph->getScale());
      Vec2f childSize = mGraph->graphToScreenVec(Vec2f(std::max(size().x, mMinSize.x), std::max(size().y, mMinSize.y)));
      
      mVisible = ImGui::BeginChild((name()+" ("+std::to_string(id())+")").c_str(), childSize, false, wFlags);
      if(mVisible) { mDrawing = true; }
      ImGui::PopStyleVar(4);
    }
  return mDrawing;
}

void Node::EndDraw()
{
  ImGui::EndChild();
  mDrawing = false;
}

bool Node::draw(ImDrawList *graphDrawList, bool blocked)
{
  mBlocked = blocked;
  ViewSettings *vs = mGraph->getViewSettings();
  float scale = getScale();
  Rect2f sRect = mGraph->graphToScreen(rect());
  ImGui::SetNextWindowPos(sRect.p1.getFloor());
  BeginDraw();
  if(mVisible || mFirstFrame || mDragging || mClicked || (mSelected && mGraph->isSelectedDragged()))
  {
    ImGui::SetWindowFontScale(scale); // TODO: better font scaling (size/alignment jitter)
    Vec2f nodePadding = NODE_PADDING*scale;
    
    // // right click menu (cut/copy selected)
    if(!mActive && ImGui::BeginPopupContextWindow("nodeContext"))
      {
        ImGui::SetWindowFontScale(1.0f/scale);
        if(ImGui::MenuItem("Cut"))   { mGraph->cut(); }
        if(ImGui::MenuItem("Copy"))  { mGraph->copy(); }
        ImGui::EndPopup();
      }
    
    // draw background
    ImDrawList *nodeDrawList = ImGui::GetWindowDrawList();
    nodeDrawList->AddRectFilled(sRect.p1, sRect.p2, ImColor(vs->nodeBgColor), NODE_ROUNDING*scale);
    // draw border
    float  borderW = getBorderWidth();
    Vec4f  borderColor = getBorderColor();
    Rect2f borderRect = sRect;
    Rect2f clipRect = borderRect.expanded(borderW);
    Rect2f graphRect = Rect2f(mGraph->viewPos(), mGraph->viewPos()+mGraph->viewSize());
    ImGui::PushClipRect(graphRect.p1.getFloor(), graphRect.p2.getCeil(), false); // extend clipping to full graph
    ImGui::PushClipRect(clipRect.p1.getFloor(),  clipRect.p2.getCeil(),  true);  // clamp to border around node
    nodeDrawList->AddRect(borderRect.p1, borderRect.p2, ImColor(borderColor), NODE_ROUNDING*scale+borderW/2.0f, ImDrawCornerFlags_All, borderW);
    ImGui::PopClipRect();
    ImGui::PopClipRect();
    
    mActive = false;
    mHover  = mDragging || mClicked;

    ImGui::BeginGroup();
    {
      if(mInputs.size() > 0)
        {
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, nodePadding);
          ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, nodePadding);
          ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, nodePadding);
          ImGui::BeginGroup(); DrawInputs(blocked); ImGui::EndGroup();
          ImGui::SameLine();
          ImGui::PopStyleVar(3);
          ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(0.0f, nodePadding.y));
        }
      else // add padding for node body
        { ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + nodePadding); }

      if(mVisible || mFirstFrame)
        {
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, nodePadding);
          ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, nodePadding);
          ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, nodePadding);
          ImGui::BeginGroup();
          {
            // ImGui::Text("ID: %d", id());
            ImGuiWindowFlags bodyFlags = (ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
            Vec2f bodySize = mGraph->graphToScreenVec(Vec2f(std::max(mBodySize.x, mMinSize.x), std::max(mBodySize.y, mMinSize.y)));
            mBodyVisible = ImGui::BeginChild("##bodyChild", bodySize, false, bodyFlags);
            ImGui::BeginGroup();
            {
              ImGui::PopStyleVar(3);
              ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  Vec2f(ImGui::GetStyle().FramePadding)*scale);
              ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   Vec2f(ImGui::GetStyle().ItemSpacing)*scale);
              ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(ImGui::GetStyle().WindowPadding)*scale);
              ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing*scale);
              onDraw();
              ImGui::PopStyleVar(4);
              ImGui::EndGroup();
              if(mVisible && mBodyVisible)
                {
                  mBodySize = mGraph->screenToGraphVec(Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin());
                  mBodySize = Vec2f(std::max(mBodySize.x, mMinSize.x), std::max(mBodySize.y, mMinSize.y));
                }
              ImGui::SetWindowSize(mGraph->graphToScreenVec(mBodySize));
            }
            ImGui::EndChild();
          }
          ImGui::EndGroup();
          mActive |= ImGui::IsItemActive();
          
          // right click menu over body (cut/copy selected -- menus already added at top)
          if(!mActive && ImGui::BeginPopupContextItem("nodeContext"))
            {
              ImGui::SetWindowFontScale(1.0f/scale);
              ImGui::EndPopup();
            }
        }
      if(mOutputs.size() > 0)
        {
          ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, nodePadding);
          ImGui::SameLine();
          ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) - Vec2f(0.0f, nodePadding.y)); // remove node padding
          ImGui::BeginGroup(); DrawOutputs(blocked); ImGui::EndGroup();
          ImGui::PopStyleVar();
        }
    }
    ImGui::EndGroup();
    
    mActive |= ImGui::IsItemActive();
    mHover  = !blocked && (ImGui::IsItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows));
    
    //if(mVisible)
      { // calculate size
        Vec2f windowSize = mGraph->screenToGraphVec(Vec2f(ImGui::GetItemRectMax()) - Vec2f(ImGui::GetItemRectMin()));
        windowSize = Vec2f(std::max(mMinSize.x, windowSize.x), std::max(mMinSize.y, windowSize.y)); // no less than minimum size
        // setSize(windowSize + Vec2f(((mOutputs.size() == 0) ? nodePadding.x : 0.0f), nodePadding.y));
        Vec2f calcSize = (Vec2f(mInputsSize.x + mBodySize.x + mOutputsSize.x, std::max(mBodySize.y, std::max(mInputsSize.y, mOutputsSize.y))) +
                          2.0f*NODE_PADDING);// - (mOutputs.size() > 0 ? Vec2f(nodePadding.x /*????*/, 0.0f) : Vec2f(0,0)));
        setSize(calcSize);

        if(mBodyVisible || mFirstFrame)
          {
            ImGuiWindowFlags bodyFlags = (ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
            ImGui::BeginChild("##bodyChild", mGraph->graphToScreenVec(mBodySize), false, bodyFlags); ImGui::EndChild();
          }
      }

    if(!blocked && (mHover || mClicked || mDragging) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
        mClicked = true;
        if(!mGraph->isSelectedHovered() && !ImGui::GetIO().KeyCtrl) { mGraph->deselectAll(); }
        setSelected(true);
      }
    else if((ImGui::IsMouseReleased(ImGuiMouseButton_Left)))
      { mClicked = false; }
    
    ImGuiIO &io = ImGui::GetIO();
    if(mSelected && (mDragging || mClicked) &&                                       // selected, and being dragged (only one node per drag)
       ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&                              // only initiate node move when clicked or already dragging
       !mGraph->isSelectedActive() &&                                                // don't move if interacting with ui element on node
       !mGraph->isSelecting() && !mGraph->isPanning() && !mGraph->isConnecting())    // don't move if selecting with rect or connecting nodes
      {
        mDragging = true;
        if(io.KeyCtrl) { mGraph->copySelected(); }  // CTRL+drag --> copy selected elements
        mGraph->moveSelected(Vec2f(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left))/scale);
        ImGui::ResetMouseDragDelta();
      }
    else
      { mDragging = false; }
  }
  EndDraw();
     
  mFirstFrame = false;
  return mVisible;
}

void Node::update()
{
  onUpdate();
}

void Node::DrawInputs(bool blocked)
{
  if(mInputs.size() == 0) { mInputsSize = Vec2f(0,0); return; } // no inputs -- don't create child

  Rect2f sRect = mGraph->graphToScreen(rect());
  Vec2f conPadding = mGraph->graphToScreenVec(CONNECTOR_PADDING);
  Vec2f conSize = mGraph->graphToScreenVec(CONNECTOR_SIZE);

  // draw inputs
  Vec2f canvasPos  = ImGui::GetCursorScreenPos();
  Vec2f canvasSize = ImGui::GetContentRegionAvail();
  mInputsSize  = Vec2f(conSize.x, (conPadding.y+conSize.y)*mInputs.size())+conPadding+Vec2f(conPadding.x, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vec2f(0,0));//conPadding);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, conPadding);
  // if(ImGui::BeginChild(("nodeInputs"+std::to_string(id())).c_str(), childSize+2.0f*CONNECTOR_PADDING, false) || mFirstFrame)
  bool visible = ImGui::BeginChild(("nodeInputs"+std::to_string(id())).c_str(), mInputsSize, false, ImGuiWindowFlags_NoDecoration);
  ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + conPadding);
  ImGui::BeginGroup();
  {
    for(int i = 0; i < mInputs.size(); i++)
      {
        mInputs[i]->draw(blocked);
        mInputs[i]->graphPos = rect().p1+CONNECTOR_PADDING + CONNECTOR_SIZE/2.0f + Vec2f(0.0f, i*(CONNECTOR_SIZE.y + CONNECTOR_PADDING.y));
        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(0.0f, conPadding.y));
      }
  }
  ImGui::EndGroup();
  ImGui::EndChild();
  ImGui::PopStyleVar(2);
  
  // draw input separator
  if(mInputs.size() > 0)
    {
      Vec2f p1 = canvasPos + Vec2f(conSize.x + 2.0f*conPadding.x, conPadding.y);
      Vec2f p2 = Vec2f(p1.x, canvasPos.y + canvasSize.y-conPadding.y);
      ImGui::GetWindowDrawList()->AddLine(p1, p2, ImColor(1.0f, 1.0f, 1.0f, 0.5f), 1.0f);
    }
  
  mInputsSize = mGraph->screenToGraphVec(mInputsSize);
}

void Node::DrawOutputs(bool blocked)
{
  if(mOutputs.size() == 0) { mOutputsSize = Vec2f(0,0); return; } // no outputs -- don't create child
  
  Rect2f sRect = mGraph->graphToScreen(rect());
  Vec2f conPadding = mGraph->graphToScreenVec(CONNECTOR_PADDING);
  Vec2f conSize = mGraph->graphToScreenVec(CONNECTOR_SIZE);
  
  // draw outputs
  Vec2f canvasPos  = ImGui::GetCursorScreenPos();
  Vec2f canvasSize = ImGui::GetContentRegionAvail();
  ImGui::SetCursorPos(Vec2f(sRect.size().x - 2.0f*conPadding.x - conSize.x, ImGui::GetCursorPos().y));
  
  mOutputsSize  = Vec2f(conSize.x, (conPadding.y+conSize.y)*mOutputs.size())+conPadding+Vec2f(conPadding.x, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vec2f(0,0));//conPadding);
  bool visible = ImGui::BeginChild(("nodeOutputs"+std::to_string(id())).c_str(), mOutputsSize, false, ImGuiWindowFlags_NoDecoration);
  ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + conPadding);
  ImGui::BeginGroup();
  {
    for(int i = 0; i < mOutputs.size(); i++)
      {
        mOutputs[i]->draw(blocked);
        mOutputs[i]->graphPos = Vec2f(rect().p2.x - CONNECTOR_PADDING.x - CONNECTOR_SIZE.x/2.0f,
                                      rect().p1.y + CONNECTOR_PADDING.y + CONNECTOR_SIZE.y/2.0f + i*(CONNECTOR_SIZE.y + CONNECTOR_PADDING.y));
        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(0.0f, conPadding.y));
      }
  }
  ImGui::EndGroup();
  ImGui::EndChild();
  ImGui::PopStyleVar();
  
  // draw output separator
  if(mOutputs.size() > 0)
    {
      Vec2f p1 = canvasPos + Vec2f(0.0f, conPadding.y);
      Vec2f p2 = canvasPos + Vec2f(0.0f, canvasSize.y-conPadding.y);
      ImGui::GetWindowDrawList()->AddLine(p1, p2, ImColor(1.0f, 1.0f, 1.0f, 0.5f), 1.0f);
    }
  
  mOutputsSize = mGraph->screenToGraphVec(mOutputsSize);
}

std::map<std::string, std::string> Node::getSaveHeader(const std::string &saveStr)
{
  std::map<std::string, std::string> header;
  std::istringstream ss(saveStr);
  std::string temp;
  ss >> temp;
  ss.ignore(2, '{');
  std::string name = "";
  std::string value = "";
  do
    {
      ss >> name;
      ss.ignore(2, ':');
      ss >> std::quoted(value);
      ss.ignore(2, ',');

      if(name == "nodeType" || name == "nodeName" || name == "nodeId" || name == "nodePos")
        { header.emplace(name, value); }
    } while(!name.empty() && name != "\n" && name.find("}") == std::string::npos && value.find("}") == std::string::npos);
      
  return header;
}

std::string Node::toSaveString() const
{
  std::map<std::string, std::string> params;
  params.emplace("nodeType", type());
  params.emplace("nodeName", mParams->name);
  params.emplace("nodeId",   std::to_string(id()));
  params.emplace("nodePos",  mParams->rect.p1.toString());
  params.emplace("nodeSize", mParams->rect.size().toString());
  params.emplace("bodySize", mBodySize.toString());
  params.emplace("inputsSize", mInputsSize.toString());
  params.emplace("outputsSize", mOutputsSize.toString());
  getSaveParams(params);
      
  std::ostringstream ss;
  ss << "NODE { ";
  for(auto &p : params)
    { ss << p.first << " : " << std::quoted(p.second) << ", "; }
  ss << "}";
  return ss.str();
}

// returns remaining string after base class parameters
std::string Node::fromSaveString(const std::string &saveStr)
{
  // convert string to params
  std::map<std::string, std::string> params;
  std::istringstream ss(saveStr);
  std::string temp, nodeType;
  ss >> temp; // "NODE {"
  ss.ignore(2, '{');
  std::string name, value;
  do
    {
      ss >> name;
      ss.ignore(2, ':');
      ss >> std::quoted(value);
      ss.ignore(2, ',');
      if(!name.empty() && name != "\n" && name.find("}") == std::string::npos && value.find("}") == std::string::npos)
        {
          std::cout << "     -->  '" << name << "' = '" << value << "'\n";
          params.emplace(name, value);
        }
    } while(!name.empty() && name != "\n" && name.find("}") == std::string::npos && value.find("}") == std::string::npos);
  
  // load stored values
  ss.str(params["nodeType"]); ss.clear();
  ss >> nodeType;
  ss.str(params["nodeName"]); ss.clear();
  ss >> mParams->name;
  ss.str(params["nodeId"]); ss.clear();
  ss >> mParams->id;
  
  setFirstFrame(false);
  Vec2f nextPos;
  Vec2f nextSize;
  Vec2f bodySize;
  Vec2f inputsSize;
  Vec2f outputsSize;

  auto iter = params.find("nodePos"); if(iter != params.end()) { nextPos.fromString(iter->second); }
  iter = params.find("nodeSize");     if(iter != params.end()) { nextSize.fromString(iter->second); }
  iter = params.find("bodySize");     if(iter != params.end()) { bodySize.fromString(iter->second); }
  iter = params.find("inputsSize");   if(iter != params.end()) { inputsSize.fromString(iter->second); }
  iter = params.find("outputsSize");  if(iter != params.end()) { outputsSize.fromString(iter->second); }

  setPos(nextPos);
  setSize(nextSize);
  mBodySize    = bodySize;
  mOutputsSize = outputsSize;
  mInputsSize  = inputsSize;
  setSaveParams(params);
  return "";
}
