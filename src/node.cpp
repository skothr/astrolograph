

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
      if(con)
        {
          for(int j = 0; j < con->mConnected.size(); j++)
            {
              if(con->mConnected[j] == mThisPtr)
                {
                  con->mConnected.erase(con->mConnected.begin() + j--);
                }
            }
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
    
void ConnectorBase::BeginDraw()
{

}

void ConnectorBase::EndDraw()
{

}

void ConnectorBase::draw()
{
  // calculate screen position (center of connector) for connection lines
  screenPos = Vec2f(ImGui::GetCursorScreenPos()) + CONNECTOR_SIZE/2.0f;
  
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

  // connecting (draw line to mouse)
  if(isConnecting())
    { // use foreground drawlist (only while actively connecting, otherwise connections will show above file dialog)
      ImGui::GetForegroundDrawList()->AddLine(screenPos, ImGui::GetMousePos(), ImColor(connectingColor), 1.0f);
      ImGui::GetForegroundDrawList()->AddLine(screenPos, ImGui::GetMousePos(), ImColor(connectingColor), 1.0f);
    }
  else
    { // connected
      for(auto con : mConnected)
        {
          // std::cout << "   DRAWING CONNECTION --> thisPos=" << screenPos << ", otherPos=" << con->screenPos << "\n";
          drawList->AddLine(screenPos, con->screenPos, ImColor(connectedColor), 3.0f);
          drawList->AddLine(screenPos, con->screenPos, ImColor(connectedColor), 3.0f);
        }
    }
  
  // draw connector dot
  drawList->AddCircleFilled(screenPos, 5.0f, ImColor(dotColor), 6);
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
  // ImGui::SetNextWindowFocus();
  // BeginDraw();
  // EndDraw();
}

void Node::setSelected(bool selected, bool stopDragging)
{
  mSelected = selected;
  if(!mSelected && stopDragging && !mClicked)
    {
      // mClicked = false;
      mDragging = false;
    }
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
        for(auto con : mInputs)  { con->drawConnections(winDrawList); }
        ImGui::EndChild();
      }
    // over node window
    float borderW = 1.0f;
    if(mActive)        { borderW = NODE_ACTIVE_BORDER_W; }
    else if(mSelected) { borderW = NODE_SELECTED_BORDER_W; }
    else if(mHover)    { borderW = NODE_HIGHLIGHTED_BORDER_W; }
    else               { borderW = NODE_DEFAULT_BORDER_W; }
    Rect2f borderRect = rect().expanded(borderW);
    ImGui::PushClipRect(borderRect.p1, borderRect.p2, false);
    winDrawList = ImGui::GetWindowDrawList();
    for(auto con : mOutputs) { con->drawConnections(winDrawList); }
    for(auto con : mInputs)  { con->drawConnections(winDrawList); }
    ImGui::PopClipRect();
  }
  EndDraw();
  // over provided window
  for(auto con : mOutputs) { con->drawConnections(drawList); }
  for(auto con : mInputs)  { con->drawConnections(drawList); }
}

bool Node::isConnecting() const
{
  for(auto con : mInputs)  { if(con->isConnecting()) { return true; } }
  for(auto con : mOutputs) { if(con->isConnecting()) { return true; } }
  return false;
}

void Node::BeginDraw()
{
  ImGuiWindowFlags wFlags = (ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, NODE_PADDING);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, NODE_PADDING);
  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, NODE_ROUNDING);
  ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
  ImGui::PushStyleColor(ImGuiCol_Border, Vec4f(1,0,0,0));
  Vec2f childSize(std::max(size().x, mMinSize.x), std::max(size().y, mMinSize.y));
  ImGui::BeginChild((name()+" ("+std::to_string(id())+")").c_str(), childSize, false, wFlags);
  ImGui::PopStyleColor();
  ImGui::PopStyleVar(4);
}

void Node::EndDraw()
{
  ImGui::EndChild();
}

bool Node::draw(ViewSettings *settings, bool blocked)
{
  ImGui::SetNextWindowPos(pos()+mGraph->getCenter());
  // ImGui::PushClipRect(mGraph->viewPos(), mGraph->viewSize(), false);
  ImGuiIO &io = ImGui::GetIO();
  BeginDraw();
  {
    // draw background
    ImGui::GetWindowDrawList()->AddRectFilled(rect().p1+mGraph->getCenter(), rect().p2+mGraph->getCenter(),
                                              ImColor(settings->nodeBgColor), NODE_ROUNDING);
    // draw border
    float borderW = 1.0f;
    Rect2f borderRect = rect() + mGraph->getCenter();
    Vec4f borderColor(1.0f, 1.0f, 1.0f, 1.0f);
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
    borderRect.expand(borderW);
    ImGui::PushClipRect(borderRect.p1, borderRect.p2, false);
    ImGui::GetWindowDrawList()->AddRect(borderRect.p1, borderRect.p2, ImColor(borderColor), NODE_ROUNDING+borderW/2.0f, ImDrawCornerFlags_All, borderW);
    ImGui::PopClipRect();
    
    mActive = false;
    mHover = false;
    
    ImGui::BeginGroup();
    {
      if(mInputs.size() > 0)
        {
          ImGui::BeginGroup(); DrawInputs(); ImGui::EndGroup();
          ImGui::SameLine();
          ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(0.0f, NODE_PADDING.y));
        }
      else // add padding for node body
        { ImGui::SetCursorPos(NODE_PADDING + ImGui::GetCursorPos()); }
      
      ImGui::BeginGroup();
      {
        ImGui::Text("ID: %d", id());
        onDraw();
        // mActive |= ImGui::IsItemActive();
      }
      ImGui::EndGroup();
      // mActive |= ImGui::IsItemActive();

      ImGui::SameLine();
      if(mOutputs.size() > 0)
        { // remove node padding
          ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) - Vec2f(0.0f, NODE_PADDING.y));
          ImGui::BeginGroup(); DrawOutputs(); ImGui::EndGroup();
        }
    }
    ImGui::EndGroup();
    setSize(Vec2f(ImGui::GetItemRectMax()) - Vec2f(ImGui::GetItemRectMin()) + Vec2f(0.0f, NODE_PADDING.y));
    mActive = ImGui::IsItemActive();
    mHover  = !blocked && (ImGui::IsItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows));

    if(!blocked && (mHover || mClicked || mDragging) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      {
        mClicked = true;
        if(!mGraph->isSelectedHovered())
          { mGraph->deselectAll(); }
        setSelected(true);
      }
    else if(((!mHover && !mActive) || ImGui::IsMouseReleased(ImGuiMouseButton_Left)) ||
            (!mHover && !mActive))// && (mGraph->isSelectedActive() || mGraph->isSelectedHovered())))
      {
        mClicked = false;
      }
    
    if(!blocked && ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&               // mouse drag --> move selected nodes
       mSelected && (mDragging || mClicked) &&                 // only initiate node move when clicked
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
  Vec2f cursorPos  = ImGui::GetCursorPos();       // + CONNECTOR_PADDING; // save cursor position
  Vec2f canvasPos  = ImGui::GetCursorScreenPos(); // + CONNECTOR_PADDING;
  Vec2f canvasSize = ImGui::GetContentRegionAvail();

  Vec2f contentMin = ImGui::GetWindowContentRegionMin();
  Vec2f contentMax = ImGui::GetContentRegionMax();

  ImDrawList* window_draw = ImGui::GetWindowDrawList();

  if(mInputs.size() == 0)
    {
      mInputsSize = Vec2f(0,0);
      return;
    }
  
  // draw inputs
  ImGui::BeginGroup();
  ImGui::BeginChild(("nodeInputs"+std::to_string(id())).c_str(), 2.0f*CONNECTOR_PADDING + Vec2f(CONNECTOR_SIZE.x,(CONNECTOR_PADDING.y+CONNECTOR_SIZE.y)*mInputs.size()));
  {
    for(int i = 0; i < mInputs.size(); i++)
      {
        ImGui::SetCursorPos(CONNECTOR_PADDING + Vec2f(0.0f, (CONNECTOR_PADDING.y+CONNECTOR_SIZE.y)*i));
        mInputs[i]->draw();
      }
  }
  ImGui::EndChild();
  ImGui::EndGroup();
      
  // draw input separator
  if(mInputs.size() > 0)
    {
      Vec2f p1 = canvasPos + Vec2f(CONNECTOR_SIZE.x + 2.0f*CONNECTOR_PADDING.x, CONNECTOR_PADDING.y);
      Vec2f p2 = Vec2f(p1.x, canvasPos.y + canvasSize.y-CONNECTOR_PADDING.y);
      window_draw->AddLine(p1, p2, ImColor(1.0f, 1.0f, 1.0f, 0.5f), 1.0f);
    }
}

void Node::DrawOutputs()
{
  Vec2f cursorPos  = ImGui::GetCursorPos(); // save cursor position
  Vec2f canvasPos  = ImGui::GetCursorScreenPos();
  Vec2f canvasSize = ImGui::GetContentRegionAvail();

  Vec2f contentMin = ImGui::GetWindowContentRegionMin();
  Vec2f contentMax = ImGui::GetContentRegionMax();

  ImDrawList* window_draw = ImGui::GetWindowDrawList();
  
  if(mOutputs.size() == 0)
    {
      mOutputsSize = Vec2f(0,0);
      return;
    }

  ImGui::BeginGroup();
  ImGui::BeginChild(("nodeOutputs"+std::to_string(id())).c_str(), 2.0f*CONNECTOR_PADDING + Vec2f(CONNECTOR_SIZE.x, (CONNECTOR_PADDING.y+CONNECTOR_SIZE.y)*mOutputs.size()));
  {
    // draw outputs
    for(int i = 0; i < mOutputs.size(); i++)
      {
        ImGui::SetCursorPos(CONNECTOR_PADDING + Vec2f(0.0f, (CONNECTOR_PADDING.y+CONNECTOR_SIZE.y)*i));
        mOutputs[i]->draw();
      }
  }
  ImGui::EndChild();
  ImGui::EndGroup();
  
  // draw output separator
  if(mOutputs.size() > 0)
    {
      Vec2f p1 = canvasPos + Vec2f(0.0f, CONNECTOR_PADDING.y);
      Vec2f p2 = canvasPos + Vec2f(0.0f, canvasSize.y-CONNECTOR_PADDING.y);
      window_draw->AddLine(p1, p2, ImColor(1.0f, 1.0f, 1.0f, 0.5f), 1.0f);
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
  ss << "NODE { ";// << type() << ": " << std::quoted(mParams->name) << " " << id() << " " << mParams->pos;
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
      
  // erase current id
  // if(mParams->id >= 0)
  //   { ACTIVE_IDS.erase(mParams->id); }

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
      
  // check active id
  // if(ACTIVE_IDS.count(mParams->id) > 0)
  //   { std::cout << "WARNING: Overlapping Node ID! (fromSaveString()) --> " << mParams->id << "\n"; }
  // ACTIVE_IDS.emplace(mParams->id);
  return "";
}
