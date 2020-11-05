#include "node.hpp"
using namespace astro;

#include "imgui.h"
#include <string>

#include "chart.hpp"
  
static std::unordered_map<std::string, Vec4f> CONNECTOR_COLORS =
  {{std::string(typeid(DateTime).name()), Vec4f(0.2f, 0.2f, 1.0f, 1.0f)},
   {std::string(typeid(Location).name()), Vec4f(0.2f, 1.0f, 0.2f, 1.0f)},
   {std::string(typeid(Chart).name()),    Vec4f(1.0f, 0.2f, 0.2f, 1.0f)}};


bool ConnectorBase::connect(ConnectorBase *other)
{
  if(!other || other == this || type() != other->type() || mDirection == other->mDirection) { return false; }
  
  // disconnect if already connected
  for(auto con : mConnected)
    {
      if(con == other)
        {
          disconnect(con);
          break;
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
                { con->mConnected.erase(con->mConnected.begin() + j--); }
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
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  5);
  ImGui::PushStyleColor(ImGuiCol_Button, connectorColor);
  if(ImGui::Button(("##"+mName).c_str(), CONNECTOR_SIZE))
    { std::cout << "CONNECTOR CLICKED [" << type() << "]\n"; }
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();

  // input handling
  bool hover = ImGui::IsItemHovered();
  if(hover)
    {
      ImGui::BeginTooltip();
      ImGui::Text(mName.c_str());
      ImGui::EndTooltip();

      //if(!ImGui::GetIO().WantCaptureMouse)
        {
          if(ImGui::IsItemClicked(ImGuiMouseButton_Middle))  { disconnectAll(); }   // MIDDLE CLICK -- disconnect all
          if(ImGui::IsMouseClicked(ImGuiMouseButton_Left))   { beginConnecting(); } // LEFT MOUSE DOWN -- start connecting
        }
    }
  if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)) { endConnecting(); }   // LEFT MOUSE UP -- stop connecting
  
  // drag/drop
  if(ImGui::BeginDragDropSource())
    {
      ImGui::SetDragDropPayload(((mDirection == CONNECTOR_INPUT ? "CON_IN" : "CON_OUT")+type()).c_str(), &mThisPtr, sizeof(ConnectorBase*));
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
              std::cout << "Connecting " << source->type() << " to " << type() << "\n";
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

  // draw connector dot
  Vec4f dotColor = Vec4f(connectorColor.x*0.4f, connectorColor.y*0.4f, connectorColor.z*0.4f, 1.0f); // color when fully connected
  ImGui::GetWindowDrawList()->AddCircleFilled(screenPos, 5.0f, ImColor(dotColor), 6);
}

void ConnectorBase::drawConnections()
{
  // get color
  Vec4f connectorColor(0.5f, 0.5f, 0.5f, 1.0f);
  auto iter = CONNECTOR_COLORS.find(type());
  if(iter != CONNECTOR_COLORS.end()) { connectorColor = iter->second; }
  Vec4f connectingColor = Vec4f(connectorColor.x, connectorColor.y, connectorColor.z, connectorColor.w*0.8f);              // color when making connection
  Vec4f connectedColor  = Vec4f(connectorColor.x*0.75f, connectorColor.y*0.75f, connectorColor.z*0.75f, connectorColor.w); // color when fully connected
  Vec4f dotColor        = Vec4f(connectorColor.x*0.4f, connectorColor.y*0.4f, connectorColor.z*0.4f, 1.0f); // color when fully connected

  ImDrawList *fg_draw = ImGui::GetForegroundDrawList();
  ImDrawList *bg_draw = ImGui::GetBackgroundDrawList();
  ImDrawList *win_draw = ImGui::GetWindowDrawList();

  ImGui::PushClipRect(Vec2f(0,0), Vec2f(4000,4000), false);
  {
    if(mDirection == CONNECTOR_OUTPUT)
      { // connected
        for(auto con : mConnected)
          {
            win_draw->AddLine(screenPos, con->screenPos, ImColor(connectedColor), 3.0f);
            win_draw->AddLine(screenPos, con->screenPos, ImColor(connectedColor), 3.0f);
          }
      }
    // connecting (draw line to mouse)
    if(isConnecting())
      {
        win_draw->AddLine(screenPos, ImGui::GetMousePos(), ImColor(connectingColor), 1.0f);
        win_draw->AddLine(screenPos, ImGui::GetMousePos(), ImColor(connectingColor), 1.0f);
      }
  }
  ImGui::PopClipRect();
}


int Node::NEXT_ID = 0;
std::unordered_set<int> Node::ACTIVE_IDS;


Node::Node(const std::vector<ConnectorBase*> &inputs_, const std::vector<ConnectorBase*> &outputs_, const std::string &name, NodeParams *params)
  : mInputs(inputs_), mOutputs(outputs_), mParams(params)
{
  if(!mParams) { mParams = new NodeParams(); }
  mParams->name = name;
  for(int i = 0; i < mInputs.size(); i++)  { mInputs[i]->setParent(this, i);  mInputs[i]->setDirection(CONNECTOR_INPUT); }
  for(int i = 0; i < mOutputs.size(); i++) { mOutputs[i]->setParent(this, i); mOutputs[i]->setDirection(CONNECTOR_OUTPUT); }

  if(mParams->id < 0)
    {
      while(ACTIVE_IDS.count(NEXT_ID) > 0) { NEXT_ID++; } // no id overlap
      mParams->id = NEXT_ID++;
      ACTIVE_IDS.emplace(mParams->id);
    }
}

Node::~Node()
{
  for(auto c : mInputs)  { delete c; }
  for(auto c : mOutputs) { delete c; }
  ACTIVE_IDS.erase(mParams->id);
  // if(mParams->id == NEXT_ID-1) { NEXT_ID--; }
  delete mParams;
}

bool Node::draw()
{
  ImGuiWindowFlags wFlags = (ImGuiWindowFlags_NoCollapse  |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_NoResize    |
                             ImGuiWindowFlags_AlwaysAutoResize);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(10, 10));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0); // square frames
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,  0);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, mMinSize);
  
  if(mSelected)
    {
      ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
      ImGui::PushStyleColor(ImGuiCol_Border, Vec4f(1,0,0,1));
    }
      
  if(ImGui::Begin((name()+" ("+std::to_string(id())+")").c_str(), &mState, wFlags))
    {
      if(mSelected) { ImGui::PopStyleColor(); ImGui::PopStyleVar();  }

      // draw connectors
      drawInputs();
      ImGui::SameLine();
      ImGui::BeginGroup();
      if(!onDraw()) { mState = false; }
      ImGui::EndGroup();
      ImGui::SameLine();
      drawOutputs();
      
      if(pos() != mNextPos)
        { ImGui::SetWindowPos(mNextPos); }
      if(!mFirstFrame)
        {
          // set window position
          ImGui::SetWindowPos(Vec2f(ImGui::GetMousePos()) - Vec2f(ImGui::GetWindowSize())/2.0f, ImGuiCond_Once);
          // draw connection lines
          for(auto con : mOutputs) { con->drawConnections(); }
          for(auto con : mInputs)  { con->drawConnections(); }
        }
      
      mSelected = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootWindow | ImGuiFocusedFlags_ChildWindows);
      //mSize = ImGui::GetWindowSize();
      mParams->rect.setPos(ImGui::GetWindowPos());
      mNextPos = pos();
    }
  ImGui::End();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
  mFirstFrame = false;
  return mState;
}

bool Node::drawInputs()
{
  Vec2f cursorPos  = ImGui::GetCursorPos(); // save cursor position
  Vec2f canvasPos  = ImGui::GetCursorScreenPos();
  Vec2f canvasSize = ImGui::GetContentRegionAvail();

  Vec2f contentMin = ImGui::GetWindowContentRegionMin();
  Vec2f contentMax = ImGui::GetWindowContentRegionMax();

  ImDrawList* window_draw = ImGui::GetWindowDrawList();
  float offset = 0.0f;

  if(mInputs.size() == 0) { return false; }
  
  // draw inputs
  ImGui::BeginChild(("nodeInputs"+std::to_string(id())).c_str(), CONNECTOR_PADDING + Vec2f(CONNECTOR_SIZE.x,(CONNECTOR_PADDING.y+CONNECTOR_SIZE.y)*mInputs.size()));
  {
    // draw input separator
    if(mInputs.size() > 0)
      {
        Vec2f p1 = canvasPos + Vec2f(CONNECTOR_SIZE.x + CONNECTOR_PADDING.x, -CONNECTOR_PADDING.y);
        Vec2f p2 = Vec2f(p1.x, canvasPos.y + canvasSize.y+CONNECTOR_PADDING.y);
        window_draw->AddLine(p1, p2, ImColor(1.0f, 1.0f, 1.0f, 0.5f), 1.0f);
      }
  
    for(int i = 0; i < mInputs.size(); i++)
      {
        ImGui::SetCursorPos(Vec2f(0.0f, (CONNECTOR_PADDING.y+CONNECTOR_SIZE.y)*i));
        mInputs[i]->draw();
        //mSelected |= ImGui::IsItemActive();
      }
  }
  ImGui::EndChild();
  // reset cursor for drawing node body
  cursorPos = Vec2f(cursorPos.x + CONNECTOR_PADDING.x*2.0f + CONNECTOR_SIZE.x, contentMin.y);
  ImGui::SetCursorPos(cursorPos);
  return true;
}

bool Node::drawOutputs()
{
  Vec2f cursorPos  = ImGui::GetCursorPos(); // save cursor position
  Vec2f canvasPos  = ImGui::GetCursorScreenPos();
  Vec2f canvasSize = ImGui::GetContentRegionAvail();

  Vec2f contentMin = ImGui::GetWindowContentRegionMin();
  Vec2f contentMax = ImGui::GetContentRegionMax();

  ImDrawList* window_draw = ImGui::GetWindowDrawList();

  float offset = 0.0f;
  
  if(mOutputs.size() == 0) { return false; }
  
  ImGui::BeginChild(("nodeOutputs"+std::to_string(id())).c_str(), CONNECTOR_PADDING + Vec2f(CONNECTOR_SIZE.x, (CONNECTOR_PADDING.y+CONNECTOR_SIZE.y)*mOutputs.size()));
  {
    // draw output separator
    if(mOutputs.size() > 0)
      {
        Vec2f p1 = canvasPos + Vec2f(0.0f, -CONNECTOR_PADDING.y);
        Vec2f p2 = canvasPos + Vec2f(0.0f, canvasSize.y+CONNECTOR_PADDING.y);
        window_draw->AddLine(p1, p2, ImColor(1.0f, 1.0f, 1.0f, 0.5f), 1.0f);
      }

    // draw outputs  
    for(int i = 0; i < mOutputs.size(); i++)
      {
        ImGui::SetCursorPos(Vec2f(CONNECTOR_PADDING.x, (CONNECTOR_PADDING.y+CONNECTOR_SIZE.y)*i));
        mOutputs[i]->draw();
        //mSelected |= ImGui::IsItemActive();
      }
  }
  ImGui::EndChild();
  
  // reset cursor
  ImGui::SetCursorPos(Vec2f(cursorPos.x, contentMin.y));
  return true;
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
  if(mParams->id >= 0)
    { ACTIVE_IDS.erase(mParams->id); }

  // load stored values
  ss.str(params["nodeType"]); ss.clear();
  ss >> nodeType;
  ss.str(params["nodeName"]); ss.clear();
  ss >> mParams->name;
  ss.str(params["nodeId"]); ss.clear();
  ss >> mParams->id;
  mNextPos.fromString(params["nodePos"]);

  setSaveParams(params);
      
  // check active id
  if(ACTIVE_IDS.count(mParams->id) > 0)
    { std::cout << "WARNING: Overlapping Node ID! (fromSaveString()) --> " << mParams->id << "\n"; }
  ACTIVE_IDS.emplace(mParams->id);
  // while(ACTIVE_IDS.count(NEXT_ID) > 0) { NEXT_ID++; } // no id overlap
      
  // std::stringstream tmp; tmp << ss.rdbuf();
  // std::string remaining = tmp.str();
      
  // return remaining;
  return "";
}
