#include "node.hpp"
using namespace astro;

#include <string>

#include <imgui.h>
#include <imgui_internal.h>
#include "nlohmann/json.hpp" // json definitions
using json = nlohmann::json;

#include "imtools.hpp"
#include "chart.hpp"
#include "astroWindow.hpp"
#include "nodeGraph.hpp"
#include "viewSettings.hpp"
#include "setting.hpp"
#include "cudaField.hpp"

int Node::NEXT_INTERNAL_ID = 0;
Node::Node(const std::vector<ConnectorBase*> &inputs_, const std::vector<ConnectorBase*> &outputs_, const std::string &name, bool resizable)
  : mInputs(inputs_), mOutputs(outputs_), mInternalId(NEXT_INTERNAL_ID++), mName(name), mResizable(resizable)
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


void Node::addInput(ConnectorBase *con)
{
  con->setParent(this, mInputs.size());
  con->setDirection(CONNECTOR_INPUT);
  mInputs.push_back(con);
}
void Node::addOutput(ConnectorBase *con)
{
  con->setParent(this, mOutputs.size());
  con->setDirection(CONNECTOR_OUTPUT);
  mOutputs.push_back(con);
}

void Node::removeInput(int i)
{
  mInputs[i]->disconnectAll();
  mInputs.erase(mInputs.begin() + i);
  for(int ii = 0; ii < mInputs.size(); ii++)
    { mInputs[ii]->setParent(this, ii-1); }
}
void Node::removeOutput(int i)
{
  mOutputs[i]->disconnectAll();
  mOutputs.erase(mOutputs.begin() + i);
  for(int ii = 0; ii < mOutputs.size(); ii++)
    { mOutputs[ii]->setParent(this, ii); }
}

void Node::clearInputs()
{
  for(int i = 0; i < mInputs.size(); i++)
    { mInputs[i]->disconnectAll(); delete mInputs[i]; }
  mInputs.clear();
}
void Node::clearOutputs()
{
  for(int i = 0; i < mOutputs.size(); i++)
    { mOutputs[i]->disconnectAll(); delete mOutputs[i]; }
  mOutputs.clear();
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
  BeginDraw();
  {
    mDrawList = ImGui::GetWindowDrawList();
    mDrawList->_FringeScale = getScale();
    // draw over border
    Rect2f graphRect(mGraph->viewPos(), mGraph->viewPos()+mGraph->viewSize());
    Rect2f borderRect = sRect.expanded(getBorderWidth()*2.0f*getScale());
    ImGui::PushClipRect(graphRect.p1.getFloor(),  graphRect.p2.getCeil(),  false); // extend clipping to full graph
    ImGui::PushClipRect(borderRect.p1.getFloor(), borderRect.p2.getCeil(), true);  // clamp to border around node
    for(auto con : mOutputs) { con->drawConnections(mDrawList, graphDrawList); }
    for(auto con : mInputs)  { con->drawConnections(mDrawList, graphDrawList); }
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
      Vec2f childSize = mGraph->graphToScreenVec(size());
      
      ImGui::PushStyleColor(ImGuiCol_DragDropTarget, Vec4f(0,0,0,0));
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(0,0));
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Vec2f(0,0));
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
      else if(mActive)   { borderColor = NODE_ACTIVE_BORDER_COLOR;      }
      else if(mSelected) { borderColor = NODE_SELECTED_BORDER_COLOR;    }
      else if(mHover)    { borderColor = NODE_HIGHLIGHTED_BORDER_COLOR; }
    }
  return borderColor;
}
void Node::bodySeparator(ImDrawList *drawList)
{
  // draw separator
  float scale = getScale();
  Vec2f bSize   = mBodySize*getScale();
  Vec2f padding = NODE_PADDING*getScale();
  Vec2f p0 = ImGui::GetCursorScreenPos();
  drawList->AddLine(p0 + Vec2f(padding.x, padding.y/2.0f), p0 + Vec2f(bSize.x-padding.x, padding.y/2.0f), ImColor(Vec4f(1.0f, 1.0f, 1.0f, 0.44)), 2.0*scale);
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
    Vec2f bodySize = mGraph->graphToScreenVec(Vec2f(std::max(mBodySize.x, mMinSize.x), std::max(mBodySize.y, mMinSize.y)) + Vec2f(0.0f, NODE_PADDING.y*scale));
    if(mGroupLevel == 0)
      {
        ImGui::PopStyleVar(4);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,     mGraph->graphToScreenVec(Vec2f(style->WindowPadding)));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,   style->ChildBorderSize*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,      mGraph->graphToScreenVec(Vec2f(2.0f, 2.0f)));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,       mGraph->graphToScreenVec(Vec2f(style->ItemSpacing)));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing,  mGraph->graphToScreenVec(Vec2f(style->ItemInnerSpacing)));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding,       mGraph->graphToScreenVec(Vec2f(style->CellPadding)));
        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing,     style->IndentSpacing*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize,     14.0f * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 0.0f  * scale);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize,       2.0f * scale);
        ImGui::GetStyle().ScrollbarScaling = scale;
      }
    Vec2f p0 = ImGui::GetCursorScreenPos();
    Vec2f padding = NODE_PADDING*scale;
    if(!mTitle.empty())
      { // get title size and draw separator
        ImGui::PushFont(getViewSettings()->titleFont);
        mTitleSize = ImGui::CalcTextSize(mTitle.c_str());
        ImGui::PopFont();
        ImGui::SetCursorScreenPos(p0+Vec2f(0, mTitleSize.y));
        bodySeparator(ImGui::GetWindowDrawList());
        ImGui::SetCursorScreenPos(p0);
      }

    mBodyVisible = ImGui::BeginChild("##bodyChild", bodySize, false, bodyFlags);
    mBodyDrawList = ImGui::GetWindowDrawList();
    ImGui::BeginGroup();
    // draw title
    if(!mTitle.empty())
      {
        ImGui::PushFont(getViewSettings()->titleFont);
        mTitleSize = ImGui::CalcTextSize(mTitle.c_str());
        mTitlePos = Vec2f(ImGui::GetCursorScreenPos()) + Vec2f((bodySize.x - mTitleSize.x)/2.0f, 0.0f);
        ImGui::SetCursorScreenPos(mTitlePos);
        ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(mTitle.c_str());
        ImGui::PopFont();
        
      }
    ImGui::SetCursorScreenPos(Vec2f(ImGui::GetCursorScreenPos()) + Vec2f(0.0f, padding.y));
    onDraw();
    ImGui::EndGroup();
    
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
      ImGui::PopStyleVar(10);
      ImGui::GetStyle().ScrollbarScaling = 1.0;
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
      if(BeginContext("nodeContext"+std::to_string(mId), CONTEXT_WINDOW_RCLICK, mHover && !mActive))
        {
          setSelected(true);
          if(ImGui::MenuItem("Cut"))   { mGraph->getWindow()->cut(); }
          if(ImGui::MenuItem("Copy"))  { mGraph->getWindow()->copy(); }
          EndContext(true);
        }
      else { EndContext(false); }
    }

  //mHoveredCon = nullptr; // set by DrawInputs/DrawOutputs
  bool conHovered = false;
  if(mVisible || mFirstFrame || mDragging || mClicked || (mSelected && mGraph->isSelectedDragged()))
    {
      style->Alpha = mColorMask.w; // set global transparency (for sub-widgets)
      Vec2f nodePadding = mGraph->graphToScreenVec(NODE_PADDING);
    
      // draw background
      mDrawList = ImGui::GetWindowDrawList();
      Vec4f bgColor = vs->nodeBgColor;
      mDrawList->AddRectFilled(sRect.p1, sRect.p2, ImColor(bgColor*mColorMask), NODE_ROUNDING*scale);
      // draw border
      float  borderW = getBorderWidth()*scale;
      Vec4f  borderColor = getBorderColor();
      Rect2f borderRect = sRect;
      Rect2f clipRect = borderRect.expanded(borderW);
      Rect2f graphRect = Rect2f(mGraph->viewPos(), mGraph->viewPos()+mGraph->viewSize());
      ImGui::PushClipRect(graphRect.p1, graphRect.p2, false);                      // extend clipping to full graph
      ImGui::PushClipRect(clipRect.p1.getFloor(),  clipRect.p2.getCeil(),  true);  // clamp to border around node
      mDrawList->AddRect(borderRect.p1, borderRect.p2, ImColor(borderColor*mColorMask), NODE_ROUNDING*scale+borderW/2.0f, ImDrawCornerFlags_All, borderW);
      ImGui::PopClipRect();
      ImGui::PopClipRect();
    
      mActive = false;
      mHover  = mDragging || mClicked;
      
      ImGui::BeginGroup();
      {
        if(mInputs.size() > 0)
          {
            ImGui::BeginGroup(); conHovered |= DrawInputs(blocked); ImGui::EndGroup();
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

                if(BeginContext("nodeContext"+std::to_string(mId), CONTEXT_WINDOW_RCLICK, mHover && ImGui::IsItemHovered() && !mActive))
                  {
                    if(!mSelected) { mGraph->deselectAll(); }
                    setSelected(true);
                    EndContext(true);
                  }
                else { EndContext(false); }
                
                style->Alpha = mColorMask.w; // set global transparency (for sub-widgets)
              }
          }
        if(mOutputs.size() > 0)
          {
            ImGui::SameLine();
            ImGui::SetCursorPos(mGraph->graphToScreenVec(Vec2f(mInputsSize.x+mBodySize.x+NODE_PADDING.x, 0.0f)) + nodePadding);
            ImGui::BeginGroup(); conHovered |= DrawOutputs(blocked||conHovered); ImGui::EndGroup();
          }
      }
      ImGui::EndGroup();
      
      mActive |= ImGui::IsItemActive();
      mHover  |= !blocked && (ImGui::IsItemHovered() || ImGui::IsWindowHovered());

      // adjust size
      Vec2f calcSize = Vec2f(mInputsSize.x + mBodySize.x + mOutputsSize.x,
                             std::max(mBodySize.y+NODE_PADDING.y, std::max(mInputsSize.y, mOutputsSize.y))) + NODE_PADDING+Vec2f(NODE_PADDING.x, 0);
      setSize(calcSize);

      bool resizeHovered = false;
      if(mResizable)
        {
          Vec2f  mp         = ImGui::GetMousePos();
          Rect2f resizeRect = mGraph->graphToScreen(Rect2f(mRect.p2-Vec2f(RESIZE_HOTSPOT_SIZE, RESIZE_HOTSPOT_SIZE), mRect.p2) - Vec2f(1,1)*NODE_ROUNDING/2.0f);
          resizeHovered     = mResizable && resizeRect.expanded(5.0f).contains(mp) && ((mp-resizeRect.p2).length2() <= (mp-resizeRect.p1).length2());
          Vec4f  resizeCol  = (mResizeClicked ? RESIZE_HOTSPOT_COLOR_CLICKED : (resizeHovered ? RESIZE_HOTSPOT_COLOR_HOVERED : RESIZE_HOTSPOT_COLOR_INACTIVE));
          
          mBodyDrawList->AddTriangleFilled(resizeRect.p2, Vec2f(resizeRect.p1.x, resizeRect.p2.y), Vec2f(resizeRect.p2.x, resizeRect.p1.y), ImColor(resizeCol));
        }
      
      if(!blocked && !mFirstFrame && (mHover || mClicked || mResizeClicked || mDragging) && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && g_contextsOpen == 0)
        { // clicking node
          if(mResizable && resizeHovered)
            {
              mResizeClicked = true;
              mResizeOffset = mRect.p2 - mGraph->screenToGraph(ImGui::GetMousePos());
              if(!mGraph->isSelectedHovered() && !ImGui::GetIO().KeyCtrl)
                { mGraph->deselectAll(); }
              mClickedUnselected = !mSelected; // clicked while unselected
              setSelected(true);
            }
          else
            {
              mClicked = true;
              if(!mGraph->isSelectedHovered() && !ImGui::GetIO().KeyCtrl)
                { mGraph->deselectAll(); }
              mClickedUnselected = !mSelected; // clicked while unselected
              setSelected(true);
            }
        }
      else if((ImGui::IsMouseReleased(ImGuiMouseButton_Left)))
        { mClicked = false; mResizeClicked = false; mResizeOffset = Vec2f(0,0); }

      // dragging node
      ImGuiIO &io = ImGui::GetIO();
      if(mSelected && (mDragging || mClicked || mResizeClicked) &&                     // selected, and being dragged (only one node per drag)
         ImGui::IsMouseDragging(ImGuiMouseButton_Left) &&                              // only initiate node move when clicked or already dragging
         !mGraph->isSelectedActive() &&                                                // don't move if interacting with ui element on node
         !mGraph->isSelecting() && !mGraph->isPanning() && !mGraph->isConnecting())    // don't move if selecting with rect or connecting nodes
        {
          if(mResizeClicked)
            {
              mDragging = true;
              // Vec2f dSize = mGraph->screenToGraphVec(Vec2f(ImGui::GetMouseDragDelta(ImGuiMouseButton_Left)));
              ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

              Vec2f mp      = mGraph->screenToGraph(ImGui::GetMousePos());
              Vec2f newSize = mp - (mRect.p1+Vec2f(mInputsSize.x, NODE_PADDING.y)) + mResizeOffset;
              newSize.x = std::max(newSize.x, mMinSize.x);
              newSize.y = std::max(newSize.y, mMinSize.y);
              // if(ImGui::IsKeyDown(GLFW_KEY_LEFT_SHIFT) || ImGui::IsKeyDown(GLFW_KEY_RIGHT_SHIFT)) // shift key -- isometric scaling
              //   { float iso = (newSize.x+newSize.y)/2.0f; newSize = Vec2f(iso, iso); }
              Vec2f dSize = newSize - mBodySize;
              if(dSize.x != 0.0f || dSize.y != 0.0f) { onResize(dSize); }
              // mBodySize = newSize;
            }
          else
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
        }
      else
        {
          if(mDragging) { mGraph->doneMoving(); }
          mDragging = false;
        }
    }
  EndDraw();
  // mConnectingTo = mHoveredCon;
  mConnectingTo = (!(mHover || conHovered) ? nullptr : mHoveredCon);

  // reset transparency
  style->Alpha = 1.0f;
  if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)) { mFirstFrame = false; mPlacing = false; }

  // divert drag/drop to connectors if connecting
  ImGui::PushStyleColor(ImGuiCol_DragDropTarget, Vec4f(0,0,0,0));
  ConnectorBase *connectingFrom = mGraph->getConnectingFrom();
  if(!conHovered && !blocked && connectingFrom && ImGui::BeginDragDropTarget())
    {
      // get input/output connector type counts
      std::map<std::string, std::vector<ConnectorBase*>> iTypes; // maps connector types to input connectors
      std::map<std::string, std::vector<ConnectorBase*>> oTypes; // maps connector types to output connectors
      for(auto con : mInputs)
        {
          std::string typeStr = con->conType();
          // if(typeStr.find("CudaFieldTex") != std::string::npos) { typeStr = std::string(typeid(CudaFieldBase).name()).substr(0, 26); }
          if(iTypes.find(typeStr) == iTypes.end()) { iTypes[typeStr] = { }; } iTypes[typeStr].push_back(con);
        }
      for(auto con : mOutputs)
        {
          std::string typeStr = con->conType();
          // if(typeStr.find("CudaFieldTex") != std::string::npos) { typeStr = std::string(typeid(CudaFieldBase).name()).substr(0, 26); }
          if(oTypes.find(typeStr) == oTypes.end()) { oTypes[typeStr] = { }; } oTypes[typeStr].push_back(con);
        }
      
      // choose connector based on where mouse is within node
      Vec2f nodeMPos = (mGraph->screenToGraph(Vec2f(ImGui::GetMousePos())) - pos()) / size();

      // check for drag/drop payload
      bool connected = false;
      
      if(nodeMPos.y >= 0.0f && nodeMPos.y <= 1.0f)
        {
          for(auto &iter : iTypes)
            { // get connector for this type based on mouse position
              int index = (int)std::floor(nodeMPos.y*(float)iter.second.size());
              index = std::min(index, (int)iter.second.size() - 1);
              ConnectorBase *con = iter.second[index];
              
              // check for drag/drop payload
              const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(("COUT_" + iter.first).c_str());
              if(payload)
                {
                  ConnectorBase *source = *((ConnectorBase**)payload->Data);
                  if(source) { con->connect(source); connected = true; break; }
                }
              if(connectingFrom->typeValid(con) && connectingFrom->direction() == CONNECTOR_OUTPUT) { mConnectingTo = con; }
            }
          if(!connected)
            {
              for(auto &iter : oTypes)
                { // get connector for this type based on mouse position
                  int index = (int)std::floor(nodeMPos.y*(float)iter.second.size());
                  index = std::min(index, (int)iter.second.size() - 1);
                  ConnectorBase *con = iter.second[index];
              
                  const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(("CIN_" + con->conType()).c_str());
                  if(payload)
                    {
                      ConnectorBase *source = *((ConnectorBase**)payload->Data);
                      if(source) { con->connect(source); connected = true; break; }
                    }
                  if(connectingFrom->typeValid(con) && connectingFrom->direction() == CONNECTOR_INPUT) { mConnectingTo = con; }
                }
            }
        }
      if(connected) { mConnectingTo = nullptr; }
      ImGui::EndDragDropTarget();
    }
  else if(!connectingFrom) { mConnectingTo = nullptr; }
  ImGui::PopStyleColor();
  
  return mVisible;
}


// returns true once all input dependencies are met
bool Node::updateReady()
{
  for(auto con : mInputs) if(con->required())
                            { if(!con->parent()->updated()) { return false; } } // required input not updated yet}
  return true;
}

void Node::update()
{
  if(!mUpdateComputed && updateReady()) { onUpdate(); mUpdateComputed = true; }
}

bool Node::DrawInputs(bool blocked)
{
  if(mInputs.size() == 0) { mInputsSize = Vec2f(0,0); return false; } // no inputs -- don't create child

  float scale      = getScale();
  Vec2f conPadding = CONNECTOR_PADDING;
  Vec2f conSize    = CONNECTOR_SIZE;
  ConnectorBase *connectingFrom = mGraph->getConnectingFrom();
  bool  conHovered = false;

  mInputsSize = Vec2f(conSize.x, (conPadding.y+conSize.y)*mInputs.size())+Vec2f(2.0f*conPadding.x, 0.0f);
  
  // draw inputs
  Vec2f canvasPos  = mGraph->graphToScreen(pos());
  Vec2f canvasSize = mGraph->graphToScreenVec(size());
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    conPadding*scale);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,     conPadding*scale);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      conPadding*scale);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, conPadding*scale);
  ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos())  + conPadding*scale);
  ImGui::BeginGroup();
  {
    for(int i = 0; i < mInputs.size(); i++)
      {
        if(mInputs[i]->draw(blocked||conHovered, mClicked) && (!connectingFrom || mInputs[i]->typeValid(connectingFrom)))
          { mHoveredCon = mInputs[i]; conHovered = true; }
        mInputs[i]->graphPos = rect().p1+CONNECTOR_PADDING + CONNECTOR_SIZE/2.0f + Vec2f(0.0f, i*(CONNECTOR_SIZE.y + CONNECTOR_PADDING.y));
      }
  }
  ImGui::EndGroup(); ImGui::PopStyleVar(4);
  
  // draw input separator
  if(mInputs.size() > 0)
    {
      Vec2f p1 = canvasPos + mGraph->graphToScreenVec(Vec2f(conSize.x + 2.0f*conPadding.x, conPadding.y));
      Vec2f p2 = Vec2f(p1.x, canvasPos.y + canvasSize.y-conPadding.y*scale);
      Vec4f sepColor = Vec4f(1.0f, 1.0f, 1.0f, 0.5f)*mColorMask;
      mDrawList = ImGui::GetWindowDrawList();
      mDrawList->_FringeScale = scale;
      mDrawList->AddLine(p1, p2, ImColor(sepColor), 1.0f);
    }
  return conHovered;
}

bool Node::DrawOutputs(bool blocked)
{
  if(mOutputs.size() == 0) { mOutputsSize = Vec2f(0,0); return false; } // no outputs -- don't create child
  
  float scale = getScale();
  Rect2f sRect = rect();
  Vec2f conPadding = CONNECTOR_PADDING;
  Vec2f conSize = CONNECTOR_SIZE;
  ConnectorBase *connectingFrom = mGraph->getConnectingFrom();
  bool  conHovered = false;

  
  // draw outputs
  Vec2f canvasPos  = ImGui::GetCursorScreenPos();
  Vec2f canvasSize = ImGui::GetContentRegionAvail();
  
  ImGui::SetCursorScreenPos(mGraph->graphToScreen(Vec2f(rect().p2.x - CONNECTOR_PADDING.x - CONNECTOR_SIZE.x, rect().p1.y + CONNECTOR_PADDING.y)));
  
  mOutputsSize = Vec2f(conSize.x, (conPadding.y+conSize.y)*mOutputs.size())+conPadding+Vec2f(conPadding.x, 0.0f);
  
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    conPadding*scale);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,     conPadding*scale);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      conPadding*scale);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, conPadding*scale);
  ImGui::BeginGroup();
  {
    for(int i = 0; i < mOutputs.size(); i++)
      {
        if(mOutputs[i]->draw(blocked||conHovered, mClicked) && (!connectingFrom || mOutputs[i]->typeValid(connectingFrom)))
          { mHoveredCon = mOutputs[i]; conHovered = true; }
        mOutputs[i]->graphPos = Vec2f(rect().p2.x - CONNECTOR_PADDING.x - CONNECTOR_SIZE.x/2.0f,
                                      rect().p1.y + CONNECTOR_PADDING.y + CONNECTOR_SIZE.y/2.0f + i*(CONNECTOR_SIZE.y + CONNECTOR_PADDING.y));
      }
  }
  ImGui::EndGroup(); ImGui::PopStyleVar(4);
  
  // draw output separator
  if(mOutputs.size() > 0)
    {
      Vec2f p1 = canvasPos;
      Vec2f p2 = canvasPos + Vec2f(0.0f, canvasSize.y-conPadding.y*getScale());
      Vec4f sepColor = Vec4f(1.0f, 1.0f, 1.0f, 0.5f)*mColorMask;
      mDrawList = ImGui::GetWindowDrawList();
      mDrawList->_FringeScale = scale;
      mDrawList->AddLine(p1, p2, ImColor(sepColor), 1.0f);
    }
  return conHovered;
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
      onLoad();
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
      //other->update();
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
