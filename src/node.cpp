

#include "node.hpp"
using namespace astro;

#include <string>

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

void ConnectorBase::reset(bool rs)
{
  mNeedReset = rs;
  if(rs && (mDirection & CONNECTOR_INPUT))
    { for(auto con : mConnected) { con->reset(rs); } }
  else if(!rs && (mDirection & CONNECTOR_OUTPUT))
    { for(auto con : mConnected) { con->reset(rs); } }
}
    
bool ConnectorBase::BeginDraw()
{

}

void ConnectorBase::EndDraw()
{

}

void ConnectorBase::draw()
{
  // calculate screen position (center of connector) for connection lines
  screenPos = Vec2f(ImGui::GetCursorScreenPos()) + CONNECTOR_SIZE/2.0f - mParent->getGraph()->getCenter();
  
  // get color
  Vec4f connectorColor(0.5f, 0.5f, 0.5f, 1.0f);
  auto iter = CONNECTOR_COLORS.find(type());
  if(iter != CONNECTOR_COLORS.end())
    { connectorColor = iter->second; }
  
  // draw connector button 
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  CONNECTOR_ROUNDING);
  ImGui::PushStyleColor(ImGuiCol_Button, connectorColor);
  ImGui::Button(("##"+mName).c_str(), CONNECTOR_SIZE);
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();

  // input handling
  if(ImGui::IsItemHovered())
    {
      ImGui::BeginTooltip();
      ImGui::Text(mName.c_str());
      ImGui::EndTooltip();

      if(ImGui::IsItemClicked(ImGuiMouseButton_Middle))  { disconnectAll(); }   // MIDDLE CLICK -- disconnect all
      if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))   { beginConnecting(); } // LEFT MOUSE DOWN -- start connecting
    }
  
  // drag/drop
  if(ImGui::BeginDragDropSource())
    {
      //std::cout << "DROP SOURCE --> " << ((mDirection == CONNECTOR_INPUT ? "CON_IN" : "CON_OUT")+type()) << "\n";
      ImGui::SetDragDropPayload(((mDirection == CONNECTOR_INPUT ? "CON_IN" : "CON_OUT")+type()).c_str(), &mThisPtr, sizeof(ConnectorBase*));
      ImGui::EndDragDropSource();
    }
  if(ImGui::BeginDragDropTarget())
    {
      //std::cout << "DROP TARGET --> " << ((mDirection == CONNECTOR_INPUT ? "CON_OUT" : "CON_IN")+type()) << "\n";
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
  if(ImGui::BeginPopupContextItem(("nodeContext-"+mName).c_str()))
    {
      if(ImGui::MenuItem("Disconnect All"))
        { disconnectAll(); }
      ImGui::EndPopup();
    }
  if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) { endConnecting(); }   // LEFT MOUSE UP -- stop connecting
}

void ConnectorBase::drawConnections(ImDrawList *drawList)
{
  // get color
  Vec4f connectorColor(0.5f, 0.5f, 0.5f, 1.0f);
  auto iter = CONNECTOR_COLORS.find(type());
  if(iter != CONNECTOR_COLORS.end()) { connectorColor = iter->second; }
  Vec4f connectingColor = Vec4f(connectorColor.x, connectorColor.y, connectorColor.z, connectorColor.w*0.8f);              // color when making connection
  Vec4f connectedColor  = Vec4f(connectorColor.x*0.75f, connectorColor.y*0.75f, connectorColor.z*0.75f, connectorColor.w); // color when fully connected
  Vec4f dotColor        = Vec4f(connectorColor.x*0.4f, connectorColor.y*0.4f, connectorColor.z*0.4f, 1.0f); // color when fully connected

  Vec2f offsetPos = screenPos + mParent->getGraph()->getCenter();
  Vec2f protrudePos = getProtrudePos() + mParent->getGraph()->getCenter();

  
  // connecting (draw line to mouse)
  if(isConnecting())
    { // use foreground drawlist (only while actively connecting, otherwise connections will show above file dialog)
      ImGui::GetForegroundDrawList()->AddLine(offsetPos, ImGui::GetMousePos(), ImColor(connectingColor), 1.0f);
      ImGui::GetForegroundDrawList()->AddLine(offsetPos, ImGui::GetMousePos(), ImColor(connectingColor), 1.0f);
    }
  else if(mConnected.size() > 0)
    { // draw protruding line
      drawList->AddLine(offsetPos, protrudePos, ImColor(connectedColor), 3.0f);
      // draw connection line
      for(auto con : mConnected)
        {
          Vec2f offsetPos2 = con->screenPos + mParent->getGraph()->getCenter();
          Vec2f protrudePos2 = con->getProtrudePos() + mParent->getGraph()->getCenter();
          // std::cout << "   DRAWING CONNECTION --> thisPos=" << screenPos << ", otherPos=" << con->screenPos << "\n";
          drawList->AddLine(protrudePos, protrudePos2, ImColor(connectedColor), 3.0f);
          drawList->AddLine(protrudePos, protrudePos2, ImColor(connectedColor), 3.0f);
        }
      // draw protruding dot
      //if(!isConnecting() && mConnected.size() > 0)
        {
          drawList->AddCircleFilled(protrudePos, 5.0f, ImColor(connectedColor), 20);
          // // draw other connection dot(s)
          // for(auto con : mConnected)
          //   { drawList->AddCircleFilled(con->getProtrudePos() + mParent->getGraph()->getCenter(), 5.0f, ImColor(connectedColor), 20); }
        }
    }
  // draw connection dot
  drawList->AddCircleFilled(offsetPos, 5.0f, ImColor(dotColor), 20);
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

void Node::drawConnections(ImDrawList *drawList)
{
  // TODO: improve drawing repetition
  BeginDraw();
  {
    ImDrawList *winDrawList = nullptr;
    
    // over output child
    if(mOutputs.size() > 0)
      {
        ImGui::BeginChild(("nodeOutputs"+std::to_string(id())).c_str());
        winDrawList = ImGui::GetWindowDrawList();
        // draw connection lines over window area
        for(auto con : mOutputs) { con->drawConnections(winDrawList); }
        ImGui::EndChild();
      }
    // over input child
    if(mInputs.size() > 0)
      {
        ImGui::BeginChild(("nodeInputs"+std::to_string(id())).c_str());
        winDrawList = ImGui::GetWindowDrawList();
        for(auto con : mInputs) { con->drawConnections(winDrawList); }
        ImGui::EndChild();
      }

    // over parent window (TODO: find path through node rects via orthogonal lines)
    for(auto con : mOutputs) { con->drawConnections(drawList); }
    for(auto con : mInputs)  { con->drawConnections(drawList); }
  
    // over node window
    winDrawList = ImGui::GetWindowDrawList();
    // over parent window
    for(auto con : mOutputs) { con->drawConnections(winDrawList); }
    for(auto con : mInputs)  { con->drawConnections(winDrawList); }
    
    // draw border
    float borderW = 1.0f;
    if(mActive)        { borderW = NODE_ACTIVE_BORDER_W;      }
    else if(mSelected) { borderW = NODE_SELECTED_BORDER_W;    }
    else if(mHover)    { borderW = NODE_HIGHLIGHTED_BORDER_W; }
    else               { borderW = NODE_DEFAULT_BORDER_W;     }
    Rect2f borderRect = rect().expanded(borderW*2.0f);
    ImGui::PushClipRect(borderRect.p1, borderRect.p2, false);
    for(auto con : mOutputs) { con->drawConnections(winDrawList); }
    for(auto con : mInputs)  { con->drawConnections(winDrawList); }
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
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, NODE_PADDING);
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, NODE_PADDING);
      ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, NODE_ROUNDING);
      ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
      ImGui::PushStyleColor(ImGuiCol_Border, Vec4f(1,0,0,0));
      Vec2f childSize(std::max(size().x, mMinSize.x), std::max(size().y, mMinSize.y));
      if(ImGui::BeginChild((name()+" ("+std::to_string(id())+")").c_str(), childSize, false, wFlags))
        { mDrawing = true; }
      ImGui::PopStyleColor();
      ImGui::PopStyleVar(4);
    }
  return mDrawing;
}

void Node::EndDraw()
{
  ImGui::EndChild();
  mDrawing = false;
}

bool Node::draw(ViewSettings *settings, bool blocked)
{
  ImGui::SetNextWindowPos(pos()+mGraph->getCenter());
  if(BeginDraw() || mFirstFrame)
  {
    // draw background
    ImGui::GetWindowDrawList()->AddRectFilled(rect().p1+mGraph->getCenter(), rect().p2+mGraph->getCenter(),
                                              ImColor(settings->nodeBgColor), NODE_ROUNDING);
    // draw border
    float  borderW = 1.0f;
    Rect2f borderRect = rect() + mGraph->getCenter();
    Vec4f  borderColor(1.0f, 1.0f, 1.0f, 1.0f);
    if(mActive)
      {
        borderW     = NODE_ACTIVE_BORDER_W;
        borderColor = NODE_ACTIVE_BORDER_COLOR;
      }
    else if(mSelected)
      {
        borderW     = NODE_SELECTED_BORDER_W;
        borderColor = NODE_SELECTED_BORDER_COLOR;
      }
    else if(mHover)
      {
        borderW     = NODE_HIGHLIGHTED_BORDER_W;
        borderColor = NODE_HIGHLIGHTED_BORDER_COLOR;
      }
    else
      {
        borderW     = NODE_DEFAULT_BORDER_W;
        borderColor = NODE_DEFAULT_BORDER_COLOR;
      }
    Rect2f clipRect = borderRect.expanded(borderW);
    ImGui::PushClipRect(clipRect.p1, clipRect.p2, false);
    ImGui::GetWindowDrawList()->AddRect(borderRect.p1, borderRect.p2, ImColor(borderColor), NODE_ROUNDING+borderW/2.0f, ImDrawCornerFlags_All, borderW);
    ImGui::PopClipRect();
    
    mActive = false;
    mHover  = false;
    
    ImGui::BeginGroup();
    {
      if(mInputs.size() > 0)
        {
          ImGui::BeginGroup(); DrawInputs(); ImGui::EndGroup();
          ImGui::SameLine();
          ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(0.0f, NODE_PADDING.y));
        }
      else // add padding for node body
        { ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + NODE_PADDING); }

      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, NODE_PADDING);
      ImGui::BeginGroup();
      {
        ImGui::Text("ID: %d", id());
        ImGuiWindowFlags bodyFlags = (ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
        Vec2f bodySize = Vec2f(std::max(mBodySize.x, mMinSize.x), std::max(mBodySize.y, mMinSize.y));
        if(ImGui::BeginChild("##bodyChild", bodySize, false, bodyFlags))
          {
            ImGui::BeginGroup();
            ImGui::PopStyleVar();
            onDraw();
            ImGui::EndGroup();
            mBodySize = Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin();
            mBodySize = Vec2f(std::max(mBodySize.x, mMinSize.x), std::max(mBodySize.y, mMinSize.y));
            ImGui::SetWindowSize(mBodySize);
          }
        else { ImGui::PopStyleVar(); }
        ImGui::EndChild();
      }
      ImGui::EndGroup();
      
      if(mOutputs.size() > 0)
        {
          ImGui::SameLine();
          ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) - Vec2f(0.0f, NODE_PADDING.y)); // remove node padding
          ImGui::BeginGroup(); DrawOutputs(); ImGui::EndGroup();
        }
    }
    ImGui::EndGroup();
    
    // calculate size
    Vec2f windowSize = Vec2f(ImGui::GetItemRectMax()) - Vec2f(ImGui::GetItemRectMin());
    windowSize = Vec2f(std::max(mMinSize.x, windowSize.x), std::max(mMinSize.y, windowSize.y)); // no less than minimum size
    setSize(windowSize + Vec2f((mOutputs.size() == 0 ? NODE_PADDING.x : 0.0f), NODE_PADDING.y));
    
    ImGuiWindowFlags bodyFlags = (ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
    ImGui::BeginChild("##bodyChild", mBodySize, false, bodyFlags); ImGui::EndChild();

    mActive = ImGui::IsItemActive();
    mHover  = !blocked && (ImGui::IsItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows));

    if(!blocked && (mHover || mClicked || mDragging) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
        mClicked = true;
        if(!mGraph->isSelectedHovered())
          { mGraph->deselectAll(); }
        setSelected(true);
      }
    else if(((!mHover && !mActive) || ImGui::IsMouseReleased(ImGuiMouseButton_Left)) || (!mHover && !mActive))
      { mClicked = false; }
    
    ImGuiIO &io = ImGui::GetIO();
    if(!blocked && ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&               // mouse drag --> move selected nodes
       mSelected && (mDragging || mClicked) &&                                    // only initiate node move when clicked or already dragging
       !mGraph->isSelectedActive() &&                                             // don't move if interacting with ui element on node
       !mGraph->isSelecting() && !mGraph->isPanning() && !mGraph->isConnecting()) // don't move if selecting with rect or connecting nodes
      {
        mDragging = true;
        if(io.KeyCtrl) { mGraph->copySelected(); }  // CTRL+drag --> copy selected elements
        mGraph->moveSelected(Vec2f(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left)));
        ImGui::ResetMouseDragDelta();
      }
    else
      { mDragging = false; }
  }
  EndDraw();
     
  mFirstFrame = false;
  return mState;
}

void Node::DrawInputs()
{
  if(mInputs.size() == 0) { mInputsSize = Vec2f(0,0); return; } // no inputs -- don't create child
  
  // draw inputs
  Vec2f canvasPos  = ImGui::GetCursorScreenPos();
  Vec2f canvasSize = ImGui::GetContentRegionAvail();
  Vec2f childSize  = Vec2f(CONNECTOR_SIZE.x, (CONNECTOR_PADDING.y+CONNECTOR_SIZE.y)*mInputs.size());
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, CONNECTOR_PADDING);
  ImGui::BeginGroup();
  if(ImGui::BeginChild(("nodeInputs"+std::to_string(id())).c_str(), childSize+2.0f*CONNECTOR_PADDING, false) || mFirstFrame)
  {
    for(int i = 0; i < mInputs.size(); i++)
      {
        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + CONNECTOR_PADDING);
        mInputs[i]->draw();
      }
  }
  ImGui::EndChild();
  ImGui::EndGroup();
  ImGui::PopStyleVar();
  mInputsSize = Vec2f(ImGui::GetItemRectMax()) - Vec2f(ImGui::GetItemRectMin());
  
  // draw input separator
  if(mInputs.size() > 0)
    {
      Vec2f p1 = canvasPos + Vec2f(CONNECTOR_SIZE.x + 2.0f*CONNECTOR_PADDING.x, CONNECTOR_PADDING.y);
      Vec2f p2 = Vec2f(p1.x, canvasPos.y + canvasSize.y-CONNECTOR_PADDING.y);
      ImGui::GetWindowDrawList()->AddLine(p1, p2, ImColor(1.0f, 1.0f, 1.0f, 0.5f), 1.0f);
    }
}

void Node::DrawOutputs()
{
  if(mOutputs.size() == 0) { mOutputsSize = Vec2f(0,0); return; } // no outputs -- don't create child
  
  // draw outputs
  Vec2f canvasPos  = ImGui::GetCursorScreenPos();
  Vec2f canvasSize = ImGui::GetContentRegionAvail();
  Vec2f childSize  = Vec2f(CONNECTOR_SIZE.x, (CONNECTOR_PADDING.y+CONNECTOR_SIZE.y)*mOutputs.size());
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, CONNECTOR_PADDING);
  ImGui::BeginGroup();
  if(ImGui::BeginChild(("nodeOutputs"+std::to_string(id())).c_str(), childSize+2.0f*CONNECTOR_PADDING, false) || mFirstFrame)
  {
    for(int i = 0; i < mOutputs.size(); i++)
      {
        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + CONNECTOR_PADDING);
        mOutputs[i]->draw();
      }
  }
  ImGui::EndChild();
  ImGui::EndGroup();
  ImGui::PopStyleVar();
  mOutputsSize = Vec2f(ImGui::GetItemRectMax()) - Vec2f(ImGui::GetItemRectMin());
  
  // draw output separator
  if(mOutputs.size() > 0)
    {
      Vec2f p1 = canvasPos + Vec2f(0.0f, CONNECTOR_PADDING.y);
      Vec2f p2 = canvasPos + Vec2f(0.0f, canvasSize.y-CONNECTOR_PADDING.y);
      ImGui::GetWindowDrawList()->AddLine(p1, p2, ImColor(1.0f, 1.0f, 1.0f, 0.5f), 1.0f);
    }
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
          std::cout << "     -->  READ PARAM '" << name << "' : '" << value << "'\n";
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
  
  notFirstFrame();
  Vec2f nextPos(params["nodePos"]);
  setPos(nextPos);
  setSaveParams(params);
  return "";
}
