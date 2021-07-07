#include "nodeConnector.hpp"
using namespace astro;

#include <string>
#include "imgui.h"
#include "imgui_internal.h"
#include "nlohmann/json.hpp" // json definitions
using json = nlohmann::json;

#include "chart.hpp"
#include "dateTime.hpp"
#include "marketData.hpp"
#include "nodeGraph.hpp"
#include "viewSettings.hpp"
#include "setting.hpp"
#include "cudaField.hpp"

static std::unordered_map<std::string, Vec4f> CONNECTOR_COLORS =
  {{std::string(typeid(DateTime).name()),                          Vec4f(0.2f, 0.2f, 1.0f, 1.0f)},
   {std::string(typeid(Location).name()),                          Vec4f(0.2f, 1.0f, 0.2f, 1.0f)},
   {std::string(typeid(Chart).name()),                             Vec4f(1.0f, 0.2f, 0.2f, 1.0f)},
   {std::string(typeid(MarketData).name()).substr(0, 26),          Vec4f(1.0f, 1.0f, 0.2f, 1.0f)},
   {std::string(typeid(CudaFieldBase).name()).substr(0, 26),       Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   {std::string(typeid(CudaField<int>).name()).substr(0, 26),      Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   {std::string(typeid(CudaField<int2>).name()).substr(0, 26),     Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   {std::string(typeid(CudaField<int3>).name()).substr(0, 26),     Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   {std::string(typeid(CudaField<int4>).name()).substr(0, 26),     Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   {std::string(typeid(CudaField<float>).name()).substr(0, 26),    Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   {std::string(typeid(CudaField<float2>).name()).substr(0, 26),   Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   {std::string(typeid(CudaField<float3>).name()).substr(0, 26),   Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   //{std::string(typeid(CudaField<float4>).name()).substr(0, 26),   Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   {std::string(typeid(CudaField<double>).name()).substr(0, 26),   Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   {std::string(typeid(CudaField<double2>).name()).substr(0, 26),  Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   {std::string(typeid(CudaField<double3>).name()).substr(0, 26),  Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   {std::string(typeid(CudaField<double4>).name()).substr(0, 26),  Vec4f(0.2f, 1.0f, 1.0f, 1.0f)},
   {std::string(typeid(CudaFieldTex).name()).substr(0, 26),        Vec4f(1.0f, 0.2f, 1.0f, 1.0f)},
   {std::string(typeid(CudaFluid<float>).name()).substr(0, 26),    Vec4f(1.0f, 1.0f, 1.0f, 1.0f)}};


std::string ConnectorBase::conType() const
{
  std::string typeStr = type(); // field connections standardized as base type
  if(typeStr.find("CudaField") != std::string::npos) { typeStr = std::string(typeid(CudaFieldBase).name()).substr(0, 26); }
  return typeStr;
}

bool ConnectorBase::typeValid(ConnectorBase *other) const
{
  std::string typeStr      = conType();
  std::string otherTypeStr = other->conType();
  //
  // if(typeStr.find("CudaFieldTex") != std::string::npos)      { typeStr      = std::string(typeid(CudaFieldBase).name()).substr(0, 26); }
  // if(otherTypeStr.find("CudaFieldTex") != std::string::npos) { otherTypeStr = std::string(typeid(CudaFieldBase).name()).substr(0, 26); }
  
  // if(type().find("CudaFieldTex") != std::string::npos != otherTypeStr.find("CudaFieldTex") != std::string::npos)
  //   { return (typeStr.find("CudaFieldBase") != std::string::npos || otherTypeStr.find("CudaFieldBase") != std::string::npos); }

  return (typeStr == otherTypeStr);
}


//// CONNECTIONS ////

bool ConnectorBase::connect(ConnectorBase *other, bool force)
{
  std::cout << "Connecting " << parent()->id() << ":" << (direction() == CONNECTOR_INPUT ? "in[" : "out[") << conId()
            << "] to " << other->parent()->id() <<  ":" << (other->direction() == CONNECTOR_INPUT ? "in[" : "out[") << other->conId() << "]\n";
  
  if(!other || other == this || !typeValid(other) || mDirection == other->mDirection) { return false; }
  
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
            { con->mConnected.erase(con->mConnected.begin() + (j--)); }
        }
    }
  mConnected.clear();
}

void ConnectorBase::sendSignal(NodeSignal signal)
{
  mSignals = (NodeSignal)(mSignals | signal);
  for(auto con : mConnected) { con->sendSignal(signal); }
}

bool ConnectorBase::draw(bool blocked, bool clicked)
{
  float scale = mParent->getScale();
  Vec4f mask = mParent->getColorMask();
  Vec2f conPadding = CONNECTOR_PADDING*scale;
  Vec2f conSize = CONNECTOR_SIZE*scale;
  float hoverPad = conPadding.y*0.5f;
  
  // get color
  Vec4f connectorColor = Vec4f(0.5f, 0.5f, 0.5f, 1.0f)*mask;
  auto iter = CONNECTOR_COLORS.find(type());
  if(iter != CONNECTOR_COLORS.end()) { connectorColor = iter->second; }
  
  // draw connector button
  Vec2f p0 = ImGui::GetCursorScreenPos();
  Rect2f cr = Rect2f(p0, p0+conSize).expanded(hoverPad);

  // ImGui::PushStyleColor(ImGuiCol_Button, connectorColor);
  // if(mHovered) { ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered)); }
  // ImGui::Button(("##"+mName).c_str(), conSize);
  // ImGui::PopStyleColor(mHovered ? 2 : 1);

  // Vec2f p1 = ImGui::GetCursorScreenPos();

  // input handling (invisible button to expand hover area)  
  ImGui::SetCursorScreenPos(cr.p1);
  ImGui::InvisibleButton(("##"+mName+"invis").c_str(), cr.size());
  mHovered = (!mParent->getGraph()->isSelecting() && !blocked && (// ImGui::IsItemHovered() || 
                                                                  cr.contains(ImGui::GetMousePos())));
  
  if(mHovered)
    {
      //if()
        {
          ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    Vec2f(ImGui::GetStyle().WindowPadding)/scale);
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,     Vec2f(ImGui::GetStyle().FramePadding)/scale);
          ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
          ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, Vec2f(ImGui::GetStyle().ItemInnerSpacing)/scale);
          ImGui::BeginTooltip();
          {
            ImGui::TextUnformatted(mName.c_str());
            ImGui::EndTooltip();
          }
          ImGui::PopStyleVar(4);
        }
      
      if(ImGui::IsItemClicked(ImGuiMouseButton_Middle))            { disconnectAll(); }   // MIDDLE CLICK -- disconnect all
      if(!clicked && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) { beginConnecting(); } // LEFT MOUSE DOWN -- start connecting
    }
  
  // drag/drop
  std::string sourceType = (mDirection == CONNECTOR_INPUT ? "CIN_" : "COUT_")+conType();
  std::string targetType = (mDirection == CONNECTOR_INPUT ? "COUT_" : "CIN_")+conType();

  ImGuiDragDropFlags ddFlags = ( ImGuiDragDropFlags_SourceNoPreviewTooltip   |
                                 //ImGuiDragDropFlags_SourceNoDisableHover     |
                                 ImGuiDragDropFlags_SourceNoHoldToOpenOthers |
                                 ImGuiDragDropFlags_AcceptNoDrawDefaultRect  |
                                 ImGuiDragDropFlags_AcceptNoPreviewTooltip
                                 );

  // handle drag/drop
  if(isConnecting() && ImGui::BeginDragDropSource(ddFlags))
    {
      ImGui::SetDragDropPayload(sourceType.c_str(), &mThisPtr, sizeof(ConnectorBase*));
      ImGui::EndDragDropSource();
    }  
  if(ImGui::BeginDragDropTarget())
    {
      const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(targetType.c_str());
      // IsDelivery() // ?
      if(payload)
        {
          ConnectorBase *source = *((ConnectorBase**)payload->Data);
          if(source)
            {
              std::cout << "Connecting " << source->parent()->id() << ":" << (source->direction() == CONNECTOR_INPUT ? "in[" : "out[") << source->conId()
                        << "] to " << parent()->id() <<  ":" << (direction() == CONNECTOR_INPUT ? "in[" : "out[") << conId() << "]\n";
              connect(source);
            }
        }
      ImGui::EndDragDropTarget();
    }

  ConnectorBase *connectingFrom = mParent->getGraph()->getConnectingFrom();
  bool highlight = (isConnecting() || ((mParent->connectingTo() == this || mHovered) &&
                                       connectingFrom && connectingFrom != this && connectingFrom->parent() != parent() &&
                                       typeValid(connectingFrom) && connectingFrom->direction() != direction()));
  
  // draw visible button
  ImGui::SetCursorScreenPos(p0);  
  ImGui::PushStyleColor(ImGuiCol_Button, connectorColor);
  if(mHovered || highlight) { ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered)); }
  ImGui::Button(("##"+mName).c_str(), conSize);
  ImGui::PopStyleColor(mHovered || highlight ? 2 : 1);
  Vec2f p1 = ImGui::GetCursorScreenPos();
  
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,    Vec2f(ImGui::GetStyle().WindowPadding)/scale);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,     Vec2f(ImGui::GetStyle().FramePadding)/scale);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,      Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
  ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, Vec2f(ImGui::GetStyle().ItemInnerSpacing)/scale);
  if(ImGui::BeginPopupContextItem((std::string("nodeContext")+(direction() == CONNECTOR_INPUT ? "I" : "O")+std::to_string(conId())).c_str()))
    {
      ImGui::SetWindowFontScale(1.0f/scale); // scale-invariant text
      if(ImGui::MenuItem("Disconnect All")) { disconnectAll(); }
      ImGui::EndPopup();
    }
  ImGui::PopStyleVar(4);
  if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) { endConnecting(); }   // LEFT MOUSE UP -- stop connecting

  if(highlight)
    { // highlight with border
      Rect2f bRect    = cr;
      Rect2f clipRect = bRect.expanded(3.0f*scale);
      ImGui::PushClipRect(clipRect.p1.getFloor(), clipRect.p2.getCeil(), false);
      ImGui::GetWindowDrawList()->AddRect(bRect.p1, bRect.p2, ImColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), 0.0f, 3.0f);
      ImGui::PopClipRect();
      if(isConnecting()) { ImGui::GetWindowDrawList()->AddRect(bRect.p1, bRect.p2, ImColor(Vec4f(1.0f, 0.0f, 0.0f, 1.0f)), 0.0f, 3.0f); }
    }
  
  ImGui::SetCursorScreenPos(p1);
  return mHovered;
}

void ConnectorBase::drawConnections(ImDrawList *nodeDrawList, ImDrawList *graphDrawList)
{
  float scale = mParent->getScale();
  //float alpha = (ghost ? GHOST_ALPHA : 1.0f);
  Vec4f mask = mParent->getColorMask();
  // get color
  Vec4f connectorColor = Vec4f(0.5f, 0.5f, 0.5f, 1.0f);
  auto iter = CONNECTOR_COLORS.find(type());
  if(iter != CONNECTOR_COLORS.end()) { connectorColor = iter->second; }
  Vec4f connectingColor = Vec4f(connectorColor.x, connectorColor.y, connectorColor.z, connectorColor.w*0.8f)*mask;              // color when making connection
  Vec4f connectedColor  = Vec4f(connectorColor.x*0.75f, connectorColor.y*0.75f, connectorColor.z*0.75f, connectorColor.w)*mask; // color when fully connected
  Vec4f dotColor        = Vec4f(connectorColor.x*0.4f, connectorColor.y*0.4f, connectorColor.z*0.4f, 1.0f)*mask;                // color of connector dot
  Vec4f connectDotColor = Vec4f(connectorColor.x*0.65f, connectorColor.y*0.65f, connectorColor.z*0.65f, 1.0f)*mask;             // color of connection dot

  float connectedW  = 3.0f*scale; // line width while connected
  float connectingW = 1.5f*scale; // line width while actively dragging and making connection

  NodeGraph *graph = mParent->getGraph();
  
  Vec2f offsetPos   = graphPos;
  Vec2f protrudePos = getProtrudePos();

  std::vector<Vec2f> connectLines;
      
  // connecting (draw line to mouse)
  Vec2f gmp = graph->screenToGraph(ImGui::GetMousePos());
  if(isConnecting())
    { // use foreground drawlist (only while actively connecting, otherwise connections will show above file dialog)
      ImDrawList *fgDrawList = ImGui::GetForegroundDrawList();
      fgDrawList->_FringeScale = scale;

      Rect2f rect = Rect2f(gmp, gmp);
      Node *hoveredNode = parent()->getGraph()->getHovered();
      if(hoveredNode && hoveredNode != parent() && hoveredNode->connectingTo())
        {
          ConnectorBase *con2 = hoveredNode->connectingTo();
          if(con2 != this && typeValid(con2) && con2->direction() != direction())
            {
              rect = hoveredNode->rect();
              gmp = con2->graphPos;
            }
        }
      
      if(mDirection == CONNECTOR_OUTPUT)
        { connectLines = graph->findOrthogonalPath(offsetPos, mParent->rect(), gmp, rect, mDirection); }
      else
        { connectLines = graph->findOrthogonalPath(gmp, rect, offsetPos, mParent->rect(), mDirection); }
      
      for(int i = 0; i < connectLines.size()-1; i++)
        { fgDrawList->AddLine(graph->graphToScreen(connectLines[i]), graph->graphToScreen(connectLines[i+1]), ImColor(connectingColor), connectingW); }
      for(int i = 1; i < connectLines.size()-1; i++)
        { fgDrawList->AddCircleFilled(graph->graphToScreen(connectLines[i]), 3.0f*scale, ImColor(connectDotColor), 32); }
    }
  else if(direction() == CONNECTOR_OUTPUT)
    { // draw connection line(s)
      for(auto con : mConnected)
        {
          if(!con->parent()->getShowConnections()) { continue; }
          Vec2f offsetPos2 = con->graphPos;
          Vec2f protrudePos2 = con->getProtrudePos();
          if(mDirection == CONNECTOR_OUTPUT)
            { connectLines = graph->findOrthogonalPath(offsetPos, mParent->rect(), offsetPos2, con->mParent->rect(), mDirection); }
          else
            { connectLines = graph->findOrthogonalPath(offsetPos2, con->mParent->rect(), offsetPos, mParent->rect(), con->mDirection); }          
          for(int i = 0; i < connectLines.size()-1; i++)
            { graphDrawList->AddLine(graph->graphToScreen(connectLines[i]), graph->graphToScreen(connectLines[i+1]), ImColor(connectedColor), connectedW); }
          for(int i = 1; i < connectLines.size()-1; i++)
            { graphDrawList->AddCircleFilled(graph->graphToScreen(connectLines[i]), 3.0f*scale, ImColor(connectDotColor), 32); }
        }
    }
  // else
  //   {
  //     connectLines.push_back()
  //   }
  
  if(isConnecting() || (mConnected.size() > 0)) // && connectLines.size() > 1))
    { // draw first and last lines over node window
      
      nodeDrawList->AddLine(graph->graphToScreen(offsetPos), graph->graphToScreen(protrudePos),
                            ImColor(connectedColor), 3.0f*scale);
      
      // nodeDrawList->AddLine(graph->graphToScreen(connectLines[0]), graph->graphToScreen(connectLines[1]),
      //                       ImColor(connectedColor), 3.0f*scale);
      // nodeDrawList->AddLine(graph->graphToScreen(connectLines[connectLines.size()-2]), graph->graphToScreen(connectLines[connectLines.size()-1]),
      //                       ImColor(connectedColor), 3.0f*scale);
    }
  // draw connection dot
  nodeDrawList->AddCircleFilled(graph->graphToScreen(offsetPos), CONNECTOR_POINT_RADIUS*scale, ImColor(dotColor), 32);
}
