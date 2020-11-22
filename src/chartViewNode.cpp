#include "chartViewNode.hpp"
using namespace astro;

#include "glfwKeys.hpp"
#include "imgui.h"
#include "tools.hpp"

ChartViewNode::ChartViewNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Chart View Node"),
    mFocusObjects(OBJ_COUNT+(ANGLE_END-ANGLE_OFFSET), false)
{
  // setMinSize(Vec2f(512, 512));
  for(int i = OBJ_SUN; i < OBJ_COUNT; i++)
    { mShowObjects.push_back(true); }
  for(int i = ANGLE_OFFSET; i < ANGLE_END; i++)
    { mShowObjects.push_back(false); }
  for(int i = OBJ_SUN; i < OBJ_END; i++)
    { mObjectOrbs.push_back(5.0); }
}


void ChartViewNode::processInput(Chart *chart)
{
  if(!chart) { return; }
  
  bool hover = ImGui::IsWindowHovered() || ImGui::IsItemHovered();
  ImGuiIO& io = ImGui::GetIO();
  if(!io.WantCaptureKeyboard)
    {
      mEditYear   = ImGui::IsKeyDown(GLFW_KEY_1);
      mEditMonth  = ImGui::IsKeyDown(GLFW_KEY_2);
      mEditDay    = ImGui::IsKeyDown(GLFW_KEY_3);
      mEditHour   = ImGui::IsKeyDown(GLFW_KEY_4);
      mEditMinute = ImGui::IsKeyDown(GLFW_KEY_5);
      mEditSecond = ImGui::IsKeyDown(GLFW_KEY_6);
      mEditLat = ImGui::IsKeyDown(GLFW_KEY_Q);
      mEditLon = ImGui::IsKeyDown(GLFW_KEY_W);
      mEditAlt = ImGui::IsKeyDown(GLFW_KEY_E);
    }

  // scroll to edit date -- hold number keys for date, or Q/W/E for location
  // 1 --> year,     2 --> month,     3 --> day,     4 --> hour,     5 --> minute,     6 --> second
  // Q --> latitude, W --> longitude, E --> altitude
  if(hover)
    {
      float delta = io.MouseWheel;
      // key multipliers
      if(io.KeyShift) { delta *= 0.1f;   } // SHIFT --> x0.1
      if(io.KeyCtrl)  { delta *= 10.0f;  } // CTRL  --> x10
      if(io.KeyAlt)   { delta *= 60.0f;  } // ALT   --> x60
      
      DateTime dt  = chart->date();
      Location loc = chart->location();

      bool changed = false;
      if(delta != 0.0f)
        {
          // date/time scroll
          if(mEditYear)   { dt.setYear(  dt.year()   + delta); changed = true; }
          if(mEditMonth)  { dt.setMonth( dt.month()  + delta); changed = true; }
          if(mEditDay)    { dt.setDay(   dt.day()    + delta); changed = true; }
          if(mEditHour)   { dt.setHour(  dt.hour()   + delta); changed = true; }
          if(mEditMinute) { dt.setMinute(dt.minute() + delta); changed = true; }
          if(mEditSecond) { dt.setSecond(dt.second() + delta); changed = true; }
          chart->setDate(dt.fixed());
          
          // location
          if(mEditLat)      { loc.latitude  += delta; changed = true; }
          else if(mEditLon) { loc.longitude += delta; changed = true; }
          else if(mEditAlt) { loc.altitude  += delta; changed = true; }
          chart->setLocation(loc.fixed());
        }
        
      // ESCAPE -- reset to current time
      if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
        {
          DateTime resetDate = DateTime::now();
          chart->setDate(resetDate);
          changed = true;
        }
      // if(changed)
      //   { inputs()[CHARTVIEWNODE_INPUT_CHART]->setReset(true); }
      
      mActive |= (changed || mEditYear || mEditMonth || mEditDay || mEditHour || mEditMinute || mEditSecond || mEditLat || mEditLon || mEditAlt);
    }
}


bool ChartViewNode::onDraw()
{
  float scale = getScale();
  ViewSettings *viewSettings = getViewSettings();
  
  bool changed = false;
  Chart *chart = inputs()[CHARTVIEWNODE_INPUT_CHART]->get<Chart>();
  //outputs()[CHARTVIEWNODE_OUTPUT_CHART]->set(chart);
  
  // size of chart
  // if(mOptionsOpen)
    {
      ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(0.0f, 2.0f));
      ImGui::TextUnformatted("Chart Size ");
      ImGui::SameLine();
      ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) - Vec2f(0.0f, 2.0f));
      ImGui::SetNextItemWidth(360*scale);
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(2.0f, 2.0f));
      ImGui::SliderFloat("##chartWidth", &mChartWidth, CHART_SIZE_MIN, CHART_SIZE_MAX, "%.0f");
      ImGui::PopStyleVar();
    }

  if(chart)
    { // draw chart view
      mView.draw(chart, mChartWidth*scale, isBlocked());
      processInput(chart);
      // ImGui::Spacing();
      // ImGui::Spacing();
    }
  else // draw empty chart
    { mView.draw((Chart*)nullptr, mChartWidth*scale, isBlocked()); }

  ImGuiIO& io = ImGui::GetIO();
  float symSize = 20.0f*scale;
  // options
  ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
  ImGui::SetNextTreeNodeOpen(mOptionsOpen);
  if(ImGui::CollapsingHeader("Options", nullptr, flags))
    {
      mOptionsOpen = true;
      ImGui::Indent();

      // align ascendant
      ImGui::TextUnformatted("Align Ascendant");
      ImGui::SameLine();
      bool align = mAlignAsc;
      if(ImGui::Checkbox("##align", &align))
        {
          mAlignAsc = align;
          mView.setAlignAsc(mAlignAsc);
        }
      // show houses 
      ImGui::TextUnformatted("Show Houses    ");
      ImGui::SameLine();
      bool show = mShowHouses;
      if(ImGui::Checkbox("##show", &show))
        {
          mShowHouses = show;
          mView.setShowHouses(mShowHouses);
        }

      
      // display settings (toggle object/angle visibility)
      float columnW = 180.0f*scale;
      // ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(20*scale, 0.0f));
      //ImGui::BeginGroup();
      ImGui::SetNextTreeNodeOpen(mDisplayOpen);
      if(ImGui::CollapsingHeader("Visibility", nullptr, flags))
        {
          mDisplayOpen = true;

          ImGui::Indent();
          // ImGui::PushFont(viewSettings->titleFont);
          // ImGui::TextUnformatted("Visibility");
          // ImGui::PopFont();
          ImGui::Separator();
          // ImGui::Indent();
          
          ImGui::BeginTable("##angVis", 4, ImGuiTableFlags_SizingPolicyStretchX | ImGuiTableFlags_NoClip);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          {
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
            // angles
            for(int a = ANGLE_OFFSET; a < ANGLE_END; a++)
              {
                int i = OBJ_COUNT+a-ANGLE_OFFSET; // obj index
                std::string name = getObjName((astro::ObjType)a);
                std::string longName = getObjNameLong((astro::ObjType)a);
                ChartImage *img = getWhiteImage(name);
                Vec4f color = getObjColor(name);
                ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

                ImGui::BeginGroup();
                {
                  bool checked = mShowObjects[i];
                  // if(a > ANGLE_OFFSET) { ImGui::SameLine(columnW*(a-ANGLE_OFFSET)); }
                  if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
                    {
                      mShowObjects[i] = checked;
                      if(chart) { chart->showObject((ObjType)a, checked); }
                    }

                  ImGui::SetNextItemWidth(symSize+10.0f*scale);
                  ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
                  ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
                }
                ImGui::EndGroup();
                
                bool hover = ImGui::IsItemHovered();
                if(chart)
                  {
                    if(hover)
                      { // angle tooltip
                        ChartObject *obj = chart->getObject((ObjType)i);
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  Vec2f(ImGui::GetStyle().FramePadding)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(ImGui::GetStyle().WindowPadding)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing/scale);
                        ImGui::SetTooltip("%s - %s %s", name.c_str(), getSignName(chart->getSign(obj->angle)).c_str(),
                                          angle_string(fmod(obj->angle, 30.0), false).c_str());
                        ImGui::PopStyleVar(4);
                      }   
                    // set focus
                    bool focused = (hover && io.KeyShift); // focus on this object with SHIFT+hover
                    if(mFocusObjects[i] == chart->objects()[a]->focused)
                      {
                        mFocusObjects[i] = focused;
                        chart->setObjFocus((ObjType)a, mFocusObjects[i]);
                      }
                  }
              
                ImGui::TableNextColumn();
              }
            
            // objects
            int numPerColumn = 5;
            for(int i = OBJ_SUN; i < OBJ_COUNT; i++)
              {
                if(i % numPerColumn == 0)
                  { ImGui::BeginGroup(); }
              
                std::string name = getObjName((astro::ObjType)i);
                ChartImage *img = getWhiteImage(name);
                Vec4f color = getObjColor(name);
                ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

                ImGui::BeginGroup();
                {
                  // if(chart) { mShowObjects[i] = chart->getObject((ObjType)i)->visible; }
                  bool checked = mShowObjects[i];
                  if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
                    {
                      mShowObjects[i] = checked;
                      if(chart) { chart->showObject((ObjType)i, checked); }
                    }
              
                  ImGui::SetNextItemWidth(symSize+10.0f*scale);
                  ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
                
                  std::string capName = name; // capitalize first letter
                  capName[0] = toupper(capName[0]);
                  ImGui::SameLine(); ImGui::Text("%s", capName.c_str());
                }
                ImGui::EndGroup();
                
                bool hover = !isBlocked() && ImGui::IsItemHovered();
                if(chart)
                  {
                    if(hover)
                      { // object tooltip
                        ChartObject *obj = chart->getObject((ObjType)i);
                        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  Vec2f(ImGui::GetStyle().FramePadding)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(ImGui::GetStyle().WindowPadding)/scale);
                        ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing/scale);
                        ImGui::SetTooltip("%s - %s %s", name.c_str(), getSignName(chart->getSign(obj->angle)).c_str(),
                                          angle_string(fmod(obj->angle, 30.0), false).c_str());
                        ImGui::PopStyleVar(4);
                      }
                    // set focus
                    bool focused = (hover && io.KeyShift); // focus on this object with SHIFT+hover
                    if(mFocusObjects[i] == chart->objects()[i]->focused)
                      {
                        mFocusObjects[i] = focused;
                        chart->setObjFocus((ObjType)i, mFocusObjects[i]);
                      }
                  }

                if(i % numPerColumn == (numPerColumn-1))
                  {
                    ImGui::EndGroup();
                    ImGui::TableNextColumn();
                  }
              }
          }
          ImGui::EndTable();
          // ImGui::Unindent();
          ImGui::Unindent();
        }
      else if(isBodyVisible()) { mDisplayOpen = false; }
      
      ImGui::SetNextTreeNodeOpen(mOrbsOpen);
      if(ImGui::CollapsingHeader("Orbs", nullptr, flags))
        {
          mOrbsOpen = true;
          ImGui::Indent();
          // ImGui::PushFont(viewSettings->titleFont);
          // ImGui::TextUnformatted("Orbs");
          // ImGui::PopFont();
          ImGui::Separator();
          // ImGui::Indent();
          
          ImGui::BeginTable("##orbs", 4, ImGuiTableFlags_SizingPolicyStretchX | ImGuiTableFlags_NoClip);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
          {
            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
          
            // objects
            int numPerColumn = 5;
            for(int i = OBJ_SUN; i < OBJ_COUNT; i++)
              {
                if(i % numPerColumn == 0)
                  { ImGui::BeginGroup(); }
              
                std::string name = getObjName((astro::ObjType)i);
                ChartImage *img = getWhiteImage(name);
                Vec4f color = getObjColor(name);
                ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

                ImGui::BeginGroup();
                {
                  ImGui::SetNextItemWidth(50*scale);
                  ImGui::InputDouble(("##setorb"+std::to_string(i)).c_str(), &mObjectOrbs[i], 0.0, 0.0, "%.2f", ImGuiInputTextFlags_None);//ImGuiInputTextFlags_AutoSelectAll);
                  ImGui::SetNextItemWidth(symSize+10.0f*scale);
                  ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
                  
                  std::string capName = name; // capitalize first letter
                  capName[0] = toupper(capName[0]);
                  ImGui::SameLine(); ImGui::Text("%s", capName.c_str());
                }
                ImGui::EndGroup();
                
                bool hover = !isBlocked() && ImGui::IsItemHovered();
                if(chart)
                  {
                    if(hover)
                      { // object tooltip
                        ChartObject *obj = chart->getObject((ObjType)i);
                        ImGui::SetTooltip("%s - %s %s", name.c_str(), getSignName(chart->getSign(obj->angle)).c_str(),
                                          angle_string(fmod(obj->angle, 30.0), false).c_str());
                      }
                    // set focus
                    bool focused = (hover && io.KeyShift); // focus on this object with SHIFT+hover
                    if(mFocusObjects[i] == chart->objects()[i]->focused)
                      {
                        mFocusObjects[i] = focused;
                        chart->setObjFocus((ObjType)i, mFocusObjects[i]);
                      }
                  }

                if(i % numPerColumn == (numPerColumn-1))
                  {
                    ImGui::EndGroup();
                    ImGui::TableNextColumn();
                  }
              }
          }
          ImGui::EndTable();
          // ImGui::Unindent();
          ImGui::Unindent();
        }
      else if(isBodyVisible()) { mOrbsOpen = false; }
      //ImGui::EndGroup();
      ImGui::Unindent();
    }
  else if(isBodyVisible()) { mOptionsOpen = false; }
  
  return true;
}

