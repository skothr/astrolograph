#include "node.hpp"
using namespace astro;

#include <string>

#include <imgui.h>
#include <imgui_internal.h>
#include "nlohmann/json.hpp" // json definitions
using json = nlohmann::json;

#include "chart.hpp"
#include "astroWindow.hpp"
#include "nodeGraph.hpp"
#include "viewSettings.hpp"
#include "setting.hpp"

int Node::NEXT_INTERNAL_ID = 0;
Node::Node(const std::vector<ConnectorBase*> &inputs_, const std::vector<ConnectorBase*> &outputs_, const std::string &name)
  : mInputs(inputs_), mOutputs(outputs_), mInternalId(NEXT_INTERNAL_ID++), mName(name)
{
  //if(mId < 0) { mId = NEXT_ID++; } // set id
  for(int i = 0; i < mInputs.size();  i++) { mInputs[i]->setParent(this, i);  mInputs[i]->setDirection(CONNECTOR_INPUT); }
  for(int i = 0; i < mOutputs.size(); i++) { mOutputs[i]->setParent(this, i); mOutputs[i]->setDirection(CONNECTOR_OUTPUT); }
  setSize(mMinSize);
}

Node::~Node()
{
  disconnectAll();
  for(auto c : mInputs)   { delete c; }
  for(auto c : mOutputs)  { delete c; }
  for(auto s : mSettings) { delete s; }
  mSettings.clear();
}

void Node::setPos(const Vec2f &p)
{
  mRect.setPos(p);
  bringToFront();
}

void Node::setSize(const Vec2f &s)
{
  mRect.setSize(s);
}

float Node::getScale() const
{ return mGraph->getScale(); }

ViewSettings* Node::getViewSettings()
{ return mGraph->getViewSettings(); }

void Node::bringToFront()
{
  mZLevel = NODE_TOP_Z;
}

std::vector<Node::Connection> Node::getInputConnections(int conId)
{
  std::vector<Connection> connections;
  if(conId >= 0)
    { // return connections from specified input connector
      std::vector<ConnectorBase*> connected = inputs()[conId]->getConnected();
      for(auto c : connected) { connections.push_back(Connection{ id(), inputs()[conId]->conId(), c->parent()->id(), c->conId() }); }
    }
  else
    { // return all input connections
      for(int i = 0; i < inputs().size(); i++)
        {
          std::vector<ConnectorBase*> connected = inputs()[i]->getConnected();
          for(int j = 0; j < connected.size(); j++)
            { connections.push_back(Connection{ connected[j]->parent()->id(), connected[j]->conId(), id(), inputs()[i]->conId() }); }
        }
    }
  return connections;
}

std::vector<Node::Connection> Node::getOutputConnections(int conId)
{
  std::vector<Connection> connections;
  if(conId >= 0)
    { // return connections from specified output connector
      std::vector<ConnectorBase*> connected = outputs()[conId]->getConnected();
      for(auto c : connected) { connections.push_back(Connection{ id(), outputs()[conId]->conId(), c->parent()->id(), c->conId() }); }
    }
  else
    { // return all output connections
      std::vector<Connection> connections;
      for(int i = 0; i < outputs().size(); i++)
        {
          std::vector<ConnectorBase*> connected = outputs()[i]->getConnected();
          for(int j = 0; j < connected.size(); j++)
            { connections.push_back(Connection{ id(), outputs()[i]->conId(), connected[j]->parent()->id(), connected[j]->conId() }); }
        }
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
  // TODO: improve repetitive drawing
  BeginDraw();
  {
    ImDrawList *winDrawList = ImGui::GetWindowDrawList();
    winDrawList->_FringeScale = getScale();
    // // over output child
    // if(mOutputs.size() > 0)
    //   {
    //     ImGui::BeginChild(("nodeOutputs"+std::to_string(id())).c_str());
    //     ImDrawList *conDrawList = ImGui::GetWindowDrawList();
    //     conDrawList->_FringeScale = getScale();
    //     // draw connection lines over window area
    //     for(auto con : mOutputs) { con->drawConnections(conDrawList, graphDrawList); }
    //     ImGui::EndChild();
    //   }
    // // over input child
    // if(mInputs.size() > 0)
    //   {
    //     ImGui::BeginChild(("nodeInputs"+std::to_string(id())).c_str());
    //     ImDrawList *conDrawList = ImGui::GetWindowDrawList();
    //     conDrawList->_FringeScale = getScale();
    //     for(auto con : mInputs) { con->drawConnections(conDrawList, graphDrawList); }
    //     ImGui::EndChild();
    //   }
    
    // draw over border
    Rect2f graphRect(mGraph->viewPos(), mGraph->viewPos()+mGraph->viewSize());
    Rect2f borderRect = sRect.expanded(getBorderWidth()*2.0f*getScale());
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
  if(mConnectingTo)        { return true; }
  for(auto con : mInputs)  { if(con->isConnecting()) { return true; } }
  for(auto con : mOutputs) { if(con->isConnecting()) { return true; } }
  return false;
}

bool Node::BeginDraw()
{
  if(!mDrawing)
    {
      ImGuiWindowFlags wFlags = (ImGuiWindowFlags_NoDecoration      |
                                 ImGuiWindowFlags_NoMove            |
                                 ImGuiWindowFlags_NoScrollWithMouse   // (body contents can still be scrolled if enabled)
                                 );
      
      ImGui::PushStyleColor(ImGuiCol_DragDropTarget, Vec4f(0,0,0,0));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(0,0));
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vec2f(0,0));
      //ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, NODE_ROUNDING*mGraph->getScale());
      Vec2f childSize = mGraph->graphToScreenVec(size());//Vec2f(std::max(size().x, mMinSize.x), std::max(size().y, mMinSize.y)));
      
      ImGui::SetNextWindowPos(mGraph->graphToScreen(pos()));
      mDrawing = ImGui::BeginChild((name()+" ("+std::to_string(mInternalId)+")").c_str(), childSize, false, wFlags);
      if(mDrawing || mFirstFrame) { mVisible = true; }
      ImGui::PopStyleVar(3);
      ImGui::SetWindowFontScale(getScale());
    }
  return mDrawing;
}

void Node::EndDraw()
{
  ImGui::EndChild();
  ImGui::PopStyleColor(); // DragDropTarget
  mDrawing = false;
}


float Node::getBorderWidth() const
{
  float borderW = NODE_DEFAULT_BORDER_W;
  if(isConnecting()) { borderW = NODE_CONNECTING_BORDER_W;  }
  else if(mActive)   { borderW = NODE_ACTIVE_BORDER_W;      }
  else if(mSelected) { borderW = NODE_SELECTED_BORDER_W;    }
  else if(mHover)    { borderW = NODE_HIGHLIGHTED_BORDER_W; }
  return borderW;
}

Vec4f Node::getBorderColor() const
{
  Vec4f borderColor = NODE_DEFAULT_BORDER_COLOR;
  if(!mGraph->isLocked())
    {
      if(isConnecting()) { borderColor = NODE_CONNECTING_BORDER_COLOR;  }
      else if(mActive)        { borderColor = NODE_ACTIVE_BORDER_COLOR;      }
      else if(mSelected)      { borderColor = NODE_SELECTED_BORDER_COLOR;    }
      else if(mHover)         { borderColor = NODE_HIGHLIGHTED_BORDER_COLOR; }
    }
  return borderColor;
}


void Node::drawBody()
{
  ImGuiStyle *style = &ImGui::GetStyle();
  float scale       = getScale();
  Vec2f nodePadding = mGraph->graphToScreenVec(NODE_PADDING);
  
  if(mGroupLevel == 0)
    {
      ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, nodePadding);
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      nodePadding);
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,     nodePadding);
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    nodePadding);
    }
  else // (for GroupNode)
    {
      style->Alpha = mColorMask.w; // set global transparency (for sub-widgets)}
      mActive = false;
      mHover  = mDragging || mClicked;
    }
  
  ImGui::BeginGroup();
  {
    // ImGui::Text("ID: %d", id());
    ImGuiWindowFlags bodyFlags = (ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse);
    Vec2f bodySize = mGraph->graphToScreenVec(Vec2f(std::max(mBodySize.x, mMinSize.x), std::max(mBodySize.y, mMinSize.y)));
    if(mGroupLevel == 0)
      {
        ImGui::PopStyleVar(4);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    mGraph->graphToScreenVec(Vec2f(style->WindowPadding)));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,  style->ChildBorderSize*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,     mGraph->graphToScreenVec(Vec2f(style->FramePadding)));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      mGraph->graphToScreenVec(Vec2f(style->ItemSpacing)));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, mGraph->graphToScreenVec(Vec2f(style->ItemInnerSpacing)));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,      mGraph->graphToScreenVec(Vec2f(style->CellPadding)));
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing,    style->IndentSpacing*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize,      style->GrabMinSize*scale);
      }
    mBodyVisible = ImGui::BeginChild("##bodyChild", bodySize, false, bodyFlags);
    ImGui::BeginGroup(); onDraw(); ImGui::EndGroup();
    
    if(mBodyVisible)
      {
        mBodySize = mGraph->screenToGraphVec(Vec2f(ImGui::GetItemRectMax()) - ImGui::GetItemRectMin());
        mBodySize = Vec2f(std::max(mBodySize.x, mMinSize.x), std::max(mBodySize.y, mMinSize.y));
        ImGui::SetWindowSize(mGraph->graphToScreenVec(mBodySize));
      }
    ImGui::EndChild();
  }
  ImGui::EndGroup();
  mActive |= ImGui::IsItemActive();
  mHover  |= !mBlocked && (ImGui::IsItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows));

  if(mGroupLevel == 0)
    {
      ImGui::PopStyleVar(8);
    }
  else
    {
      Vec2f calcSize = (Vec2f(mInputsSize.x + mBodySize.x + mOutputsSize.x, std::max(mBodySize.y, std::max(mInputsSize.y, mOutputsSize.y))) +
                        2.0f*NODE_PADDING);
      setSize(calcSize);
      style->Alpha = 1.0f;
    }
}


bool Node::draw(ImDrawList *graphDrawList, bool blocked)
{
  ImGuiStyle *style = &ImGui::GetStyle();
  ViewSettings *vs = mGraph->getViewSettings();
  float scale = getScale();
  Rect2f sRect = mGraph->graphToScreen(rect());

  if(mFirstFrame || mGraph->isLocked()) { blocked = true; }
  
  mBlocked = blocked;
  
  BeginDraw();
  if(mColorMask.w == 1.0f && !mFirstFrame && !mPlacing && !mActive && !mClicked && !mDragging)
    { // right click menu (cut/copy selected)
      if(ImGui::BeginPopupContextWindow("nodeContext"))
        {
          ImGui::SetWindowFontScale(1.0f/scale); // scale-invariant text
          if(ImGui::MenuItem("Cut"))   { mGraph->getWindow()->cut(); }
          if(ImGui::MenuItem("Copy"))  { mGraph->getWindow()->copy(); }
          ImGui::EndPopup();
        }
    }
  
  if(mVisible || mFirstFrame || mDragging || mClicked || (mSelected && mGraph->isSelectedDragged()))
    {
      style->Alpha = mColorMask.w; // set global transparency (for sub-widgets)
      Vec2f nodePadding = mGraph->graphToScreenVec(NODE_PADDING);
    
      // draw background
      ImDrawList *nodeDrawList = ImGui::GetWindowDrawList();
      Vec4f bgColor = vs->nodeBgColor;
      nodeDrawList->AddRectFilled(sRect.p1, sRect.p2, ImColor(bgColor*mColorMask), NODE_ROUNDING*scale);
      // draw border
      float  borderW = getBorderWidth()*scale;
      Vec4f  borderColor = getBorderColor();
      Rect2f borderRect = sRect;
      Rect2f clipRect = borderRect.expanded(borderW);
      Rect2f graphRect = Rect2f(mGraph->viewPos(), mGraph->viewPos()+mGraph->viewSize());
      ImGui::PushClipRect(graphRect.p1, graphRect.p2, false);                      // extend clipping to full graph
      ImGui::PushClipRect(clipRect.p1.getFloor(),  clipRect.p2.getCeil(),  true);  // clamp to border around node
      nodeDrawList->AddRect(borderRect.p1, borderRect.p2, ImColor(borderColor*mColorMask), NODE_ROUNDING*scale+borderW/2.0f, ImDrawCornerFlags_All, borderW);
      ImGui::PopClipRect();
      ImGui::PopClipRect();
    
      mActive = false;
      mHover  = mDragging || mClicked;

      ImGui::BeginGroup();
      {
        if(mInputs.size() > 0)
          {
            ImGui::BeginGroup(); DrawInputs(blocked); ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::SetCursorPos(mGraph->graphToScreenVec(Vec2f(mInputsSize.x, 0.0f)) + nodePadding);
          }
        else // add padding for node body
          { ImGui::SetCursorPos(nodePadding); }

        if(mVisible || mFirstFrame)
          {
            drawBody();
            
            if(mColorMask.w == 1.0f && !mFirstFrame && !mPlacing && !mActive && !mClicked && !mDragging)
              {
                style->Alpha = 1.0f; // reset global transparency (for context menu)
                if(ImGui::BeginPopupContextItem("nodeContext"))
                  { // right click menu over body (cut/copy selected -- menus already added at top)
                    ImGui::EndPopup();
                    if(!mSelected) { mGraph->deselectAll(); }
                    setSelected(true);
                  }
                style->Alpha = mColorMask.w; // set global transparency (for sub-widgets)
              }
          }
        if(mOutputs.size() > 0)
          {
            ImGui::SameLine();
            ImGui::SetCursorPos(mGraph->graphToScreenVec(Vec2f(mInputsSize.x+mBodySize.x+NODE_PADDING.x, 0.0f)) + nodePadding);
            ImGui::BeginGroup(); DrawOutputs(blocked); ImGui::EndGroup();
          }
      }
      ImGui::EndGroup();
    
      mActive |= ImGui::IsItemActive();
      mHover  |= !blocked && (ImGui::IsItemHovered() || ImGui::IsWindowHovered());

      // adjust size
      Vec2f calcSize = Vec2f(mInputsSize.x + mBodySize.x + mOutputsSize.x,
                             std::max(mBodySize.y+NODE_PADDING.y, std::max(mInputsSize.y, mOutputsSize.y))) + NODE_PADDING+Vec2f(NODE_PADDING.x, 0);
      setSize(calcSize);
      // ImGui::SetWindowSize(calcSize);

      // clicking node
      if(!blocked && !mFirstFrame && (mHover || mClicked || mDragging) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
          mClicked = true;
          if(!mGraph->isSelectedHovered() && !ImGui::GetIO().KeyCtrl)
            { mGraph->deselectAll(); }
          mClickedUnselected = !mSelected; // clicked while unselected
          setSelected(true);
        }
      else if((ImGui::IsMouseReleased(ImGuiMouseButton_Left)))
        { mClicked = false; }

      // dragging node
      ImGuiIO &io = ImGui::GetIO();
      if(mSelected && (mDragging || mClicked) &&                                       // selected, and being dragged (only one node per drag)
         ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&                              // only initiate node move when clicked or already dragging
         !mGraph->isSelectedActive() &&                                                // don't move if interacting with ui element on node
         !mGraph->isSelecting() && !mGraph->isPanning() && !mGraph->isConnecting())    // don't move if selecting with rect or connecting nodes
        {
          if(io.KeyCtrl)
            {
              if(mClickedUnselected)
                { mGraph->deselectAll(); setSelected(true); mClickedUnselected = false; }
              mGraph->copySelected();
            }  // CTRL+drag --> copy selected elements
        
          mDragging = true;
          mGraph->moveSelected(Vec2f(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left))/scale);
          ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
        }
      else
        {
          if(mDragging) { mGraph->doneMoving(); }
          mDragging = false;
        }
    }
  EndDraw();

  // reset transparency
  style->Alpha = 1.0f;
  if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)) { mFirstFrame = false; mPlacing = false; }

  // divert drag/drop to connectors if connecting
  ImGui::PushStyleColor(ImGuiCol_DragDropTarget, Vec4f(0,0,0,0));
  ConnectorBase *connectingFrom = mGraph->getConnectingTo();
  if(connectingFrom && ImGui::BeginDragDropTarget())
    {
      bool connected = false;
      for(auto con : mInputs)
        {
          const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(("COUT_" + con->type()).c_str());
          if(payload)
            {
              ConnectorBase *source = *((ConnectorBase**)payload->Data);
              if(source) { con->connect(source); connected = true; break; }
            }
          if(!mConnectingTo && connectingFrom->type() == con->type() && connectingFrom->direction() == CONNECTOR_OUTPUT) { mConnectingTo = con; }
        }
      if(!connected)
        {
          for(auto con : mOutputs)
            {
              const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(("CIN_" + con->type()).c_str());
              if(payload)
                {
                  ConnectorBase *source = *((ConnectorBase**)payload->Data);
                  if(source) { con->connect(source); connected = true; break; }
                }
              if(!mConnectingTo && connectingFrom->type() == con->type() && connectingFrom->direction() == CONNECTOR_INPUT) { mConnectingTo = con; }
            }
        }
      if(connected) { mConnectingTo = nullptr; }
      ImGui::EndDragDropTarget();
    }
  else
    { mConnectingTo = nullptr; }
  ImGui::PopStyleColor();
  
  return mVisible;
}

void Node::update()
{
  onUpdate();
}

void Node::DrawInputs(bool blocked)
{
  if(mInputs.size() == 0) { mInputsSize = Vec2f(0,0); return; } // no inputs -- don't create child

  float scale      = getScale();
  Vec2f conPadding = CONNECTOR_PADDING; // mGraph->graphToScreenVec(CONNECTOR_PADDING);
  Vec2f conSize    = CONNECTOR_SIZE;    // mGraph->graphToScreenVec(CONNECTOR_SIZE);

  mInputsSize = Vec2f(conSize.x, (conPadding.y+conSize.y)*mInputs.size())+Vec2f(2.0f*conPadding.x, 0.0f);
  
  // draw inputs
  Vec2f canvasPos  = mGraph->graphToScreen(pos()); // ImGui::GetCursorScreenPos();
  Vec2f canvasSize = mGraph->graphToScreenVec(size());          // ImGui::GetContentRegionAvail();
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    conPadding*scale);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,     conPadding*scale);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      conPadding*scale);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, conPadding*scale);
  //bool visible = ImGui::BeginChild(("nodeInputs"+std::to_string(id())).c_str(), mGraph->graphToScreenVec(mInputsSize), false, ImGuiWindowFlags_NoDecoration);
  ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + conPadding*scale);
  ImGui::BeginGroup();
  {
    for(int i = 0; i < mInputs.size(); i++)
      {
        mInputs[i]->draw(blocked);
        mInputs[i]->graphPos = rect().p1+CONNECTOR_PADDING + CONNECTOR_SIZE/2.0f + Vec2f(0.0f, i*(CONNECTOR_SIZE.y + CONNECTOR_PADDING.y));
        //ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(0.0f, conPadding.y));
      }
  }
  ImGui::EndGroup();
  //ImGui::EndChild();
  ImGui::PopStyleVar(4);
  
  // draw input separator
  if(mInputs.size() > 0)
    {
      Vec2f p1 = canvasPos + mGraph->graphToScreenVec(Vec2f(conSize.x + 2.0f*conPadding.x, conPadding.y));
      Vec2f p2 = Vec2f(p1.x, canvasPos.y + canvasSize.y-conPadding.y*scale);
      Vec4f sepColor = Vec4f(1.0f, 1.0f, 1.0f, 0.5f)*mColorMask;
      ImDrawList *winDrawList = ImGui::GetWindowDrawList();
      winDrawList->_FringeScale = scale;
      winDrawList->AddLine(p1, p2, ImColor(sepColor), 1.0f);
    }
}

void Node::DrawOutputs(bool blocked)
{
  if(mOutputs.size() == 0) { mOutputsSize = Vec2f(0,0); return; } // no outputs -- don't create child
  
  float scale = getScale();
  Rect2f sRect = rect(); //mGraph->graphToScreen(rect());
  Vec2f conPadding = CONNECTOR_PADDING; //mGraph->graphToScreenVec(CONNECTOR_PADDING);
  Vec2f conSize = CONNECTOR_SIZE;//mGraph->graphToScreenVec(CONNECTOR_SIZE);
  
  // draw outputs
  Vec2f canvasPos  = ImGui::GetCursorScreenPos();
  Vec2f canvasSize = ImGui::GetContentRegionAvail();
  
  ImGui::SetCursorScreenPos(mGraph->graphToScreen(Vec2f(rect().p2.x - CONNECTOR_PADDING.x - CONNECTOR_SIZE.x, rect().p1.y + CONNECTOR_PADDING.y)));
  
  mOutputsSize = Vec2f(conSize.x, (conPadding.y+conSize.y)*mOutputs.size())+conPadding+Vec2f(conPadding.x, 0.0f);
  
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    conPadding*scale);// Vec2f(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,     conPadding*scale);// Vec2f(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      conPadding*scale);// Vec2f(0,0));
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, conPadding*scale);// Vec2f(0,0));
  //bool visible = ImGui::BeginChild(("nodeOutputs"+std::to_string(id())).c_str(), mGraph->graphToScreenVec(mOutputsSize), false, ImGuiWindowFlags_NoDecoration);
  //ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(conPadding.x, 0.0f)*scale);
  ImGui::BeginGroup();
  {
    for(int i = 0; i < mOutputs.size(); i++)
      {
        mOutputs[i]->draw(blocked);
        mOutputs[i]->graphPos = Vec2f(rect().p2.x - CONNECTOR_PADDING.x - CONNECTOR_SIZE.x/2.0f,
                                      rect().p1.y + CONNECTOR_PADDING.y + CONNECTOR_SIZE.y/2.0f + i*(CONNECTOR_SIZE.y + CONNECTOR_PADDING.y));
      }
  }
  ImGui::EndGroup();
  //ImGui::EndChild();
  ImGui::PopStyleVar(4);
  
  // draw output separator
  if(mOutputs.size() > 0)
    {
      Vec2f p1 = canvasPos;// + mGraph->graphToScreenVec(Vec2f(0.0f, conPadding.y));
      Vec2f p2 = canvasPos + Vec2f(0.0f, canvasSize.y-conPadding.y*getScale());
      Vec4f sepColor = Vec4f(1.0f, 1.0f, 1.0f, 0.5f)*mColorMask;
      ImDrawList *winDrawList = ImGui::GetWindowDrawList();
      winDrawList->_FringeScale = scale;
      winDrawList->AddLine(p1, p2, ImColor(sepColor), 1.0f);
    }
}

json Node::toJSON() const
{
  json js     = json::object();
  json header = json::object();
  header["nodeType"]    = type();
  header["nodeId"]      = id();
  header["nodeName"]    = mName;
  header["nodePos"]     = pos().toString();
  header["nodeSize"]    = size().toString();
  header["bodySize"]    = mBodySize.toString();
  header["inputsSize"]  = mInputsSize.toString();
  header["outputsSize"] = mOutputsSize.toString();
  
  js["header"] = header;

  for(auto s : mSettings)
    { js[s->getId()] = s->toJSON(); }
  
  return js;
}

// returns remaining string after base class parameters
bool Node::fromJSON(const json &js)
{
  if(js.contains("header"))
    {
      json header = js["header"];
      if(header.is_null())               { return false; }
      
      if(header.contains("nodeType") && header["nodeType"] != type()) { return false; }

      if(header.contains("nodeId"))      { setId(header["nodeId"]); }
      if(header.contains("nodePos"))     { setPos(Vec2f(header["nodePos"].get<std::string>())); }
      if(header.contains("nodeSize"))    { setSize(Vec2f(header["nodeSize"].get<std::string>())); }
      if(header.contains("bodySize"))    { mBodySize = Vec2f(header["bodySize"].get<std::string>()); }
      if(header.contains("inputsSize"))  { mInputsSize = Vec2f(header["inputsSize"].get<std::string>()); }
      if(header.contains("outputsSize")) { mOutputsSize = Vec2f(header["outputsSize"].get<std::string>()); }
      for(auto s : mSettings)
        {
          if(js.contains(s->getId()))
            {
              json jss = js[s->getId()];
              s->fromJSON(jss);
            }
        }
    }
  return true;
}

// Copies child class data to other node (must be same type)
//  --> override in child class if data needs to be copied
bool Node::copyTo(Node *other)
{
  if(other && (type() == other->type()))
    {
      int oldId = other->id();
      other->fromJSON(toJSON());
      // other->setId(oldId);
      return true;
    }
  else
    { return false; }
}


std::ostream& Node::print(std::ostream &os) const
{
  os << "|=== NODE ==========================================================|\n"
     << "|= id        = " << id()   << "\n"
     << "|= type      = " << type() << "\n"
     << "|= pos       = " << pos()  << "\n"
     << "|= size      = " << size() << "\n";
  os << "|===================================================================|\n";
  return os;
}
