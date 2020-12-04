#include "chartViewNode.hpp"
using namespace astro;

#include "glfwKeys.hpp"
#include "imgui.h"
#include "tools.hpp"

ChartViewNode::ChartViewNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Chart View Node")
{ }

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
      mEditLat    = ImGui::IsKeyDown(GLFW_KEY_Q);
      mEditLon    = ImGui::IsKeyDown(GLFW_KEY_W);
      mEditAlt    = ImGui::IsKeyDown(GLFW_KEY_E);
    }

  // scroll to edit date -- hold number keys for date, or Q/W/E for location
  // 1 --> year,     2 --> month,     3 --> day,     4 --> hour,     5 --> minute,     6 --> second
  // Q --> latitude, W --> longitude, E --> altitude
  if(hover && !isBlocked())
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
      
      mActive |= (changed || mEditYear || mEditMonth || mEditDay || mEditHour || mEditMinute || mEditSecond || mEditLat || mEditLon || mEditAlt);
    }
}

void ChartViewNode::onUpdate()
{ }

void ChartViewNode::onDraw()
{
  float scale = getScale();
  ViewSettings *viewSettings = getViewSettings();
  
  bool changed = false;
  Chart *chart = inputs()[CHARTVIEWNODE_INPUT_CHART]->get<Chart>();
  
  ImGuiIO& io = ImGui::GetIO();
  float symSize = 20.0f*scale;
  // options
  ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
  ImGui::SetNextTreeNodeOpen(mOptionsOpen);
  if(ImGui::CollapsingHeader("Options", nullptr, flags))
    {
      mOptionsOpen = true;
      ImGui::Indent();
      
      float columnW = 180.0f*scale;
      
      // size of chart
      {
        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(0.0f, 2.0f));
        ImGui::TextUnformatted("Chart Size ");
        ImGui::SameLine(columnW);
        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) - Vec2f(0.0f, 2.0f));
        ImGui::SetNextItemWidth(330*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(2.0f, 2.0f));
        ImGui::SliderFloat("##chartWidth", &mChartWidth, CHART_SIZE_MIN, CHART_SIZE_MAX, "%.0f");
        ImGui::PopStyleVar();
      }


      // align ascendant
      ImGui::TextUnformatted("Align Ascendant");
      ImGui::SameLine(columnW);
      bool align = mAlignAsc;
      if(ImGui::Checkbox("##align", &align))
        {
          mAlignAsc = align;
          mView.setAlignAsc(mAlignAsc);
        }
      // show houses 
      ImGui::TextUnformatted("Show Houses    ");
      ImGui::SameLine(columnW);
      bool show = mShowHouses;
      if(ImGui::Checkbox("##show", &show))
        {
          mShowHouses = show;
          mView.setShowHouses(mShowHouses);
        }

      
      // display settings (toggle object/angle visibility)
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
            // ImGui::BeginGroup();
            int numPerColumn = 4;
            bool grouping = false;
            for(int a = ANGLE_OFFSET; a < ANGLE_END; a++)
              {
                if((a-ANGLE_OFFSET) % numPerColumn == 0)
                  {
                    if(a > ANGLE_OFFSET) { ImGui::Spacing(); }
                    ImGui::BeginGroup();
                    grouping = true;
                  }
                int i = OBJ_COUNT+a-ANGLE_OFFSET; // obj index
                std::string name = getObjName((astro::ObjType)a);
                std::string longName = getObjNameLong((astro::ObjType)a);
                ChartImage *img = getWhiteImage(name);
                Vec4f color = getObjColor(name);
                ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

                ImGui::BeginGroup();
                {
                  bool checked = mParams.objVisible[i];
                  // if(a > ANGLE_OFFSET) { ImGui::SameLine(columnW*(a-ANGLE_OFFSET)); }
                  if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
                    {
                      mParams.objVisible[i] = checked;
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
                    if(mParams.objFocused[i] == chart->objects()[i]->focused)
                      {
                        mParams.objFocused[i] = focused;
                        chart->setObjFocus((ObjType)a, mParams.objFocused[i]);
                      }
                  }
                
                if((a-ANGLE_OFFSET) % numPerColumn == (numPerColumn-1))
                  {
                    ImGui::EndGroup();
                    ImGui::TableNextColumn();
                    grouping = false;
                  }
                //ImGui::TableNextColumn();
              }
            if(grouping)
              {
                // ImGui::Spacing();
                ImGui::EndGroup();
                grouping = false;
              }

            ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
            ImGui::Separator();
            
            // objects
            numPerColumn = 5;
            for(int i = 0; i < OBJ_COUNT; i++)
              {
                if(i % numPerColumn == 0)
                  {
                    if(i > 0) { ImGui::Spacing(); }
                    ImGui::BeginGroup();
                    grouping = true;
                  }
              
                std::string name = getObjName((astro::ObjType)i);
                std::string longName = getObjNameLong((astro::ObjType)i);
                ChartImage *img = getWhiteImage(name);
                Vec4f color = getObjColor(name);
                ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

                ImGui::BeginGroup();
                {
                  // if(chart) { mShowObjects[i] = chart->getObject((ObjType)i)->visible; }
                  bool checked = mParams.objVisible[i];
                  if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
                    {
                      mParams.objVisible[i] = checked;
                      if(chart) { chart->showObject((ObjType)i, checked); }
                    }
              
                  ImGui::SetNextItemWidth(symSize+10.0f*scale);
                  ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
                  ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
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
                    if(mParams.objFocused[i] == chart->objects()[i]->focused)
                      {
                        mParams.objFocused[i] = focused;
                        chart->setObjFocus((ObjType)i, mParams.objFocused[i]);
                      }
                  }

                if(i % numPerColumn == (numPerColumn-1))
                  {
                    ImGui::EndGroup();
                    ImGui::TableNextColumn();
                    grouping = false;
                  }
              }
            if(grouping)
              {
                ImGui::EndGroup();
                grouping = false;
              }
          }
          ImGui::Separator();
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
            bool grouping = false;
            for(int i = OBJ_SUN; i < OBJ_COUNT; i++)
              {
                if(i % numPerColumn == 0)
                  { 
                   ImGui::BeginGroup();
                    grouping = true;
                  }
              
                std::string name = getObjName((astro::ObjType)i);
                std::string longName = getObjNameLong((astro::ObjType)i);
                ChartImage *img = getWhiteImage(name);
                Vec4f color = getObjColor(name);
                ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

                ImGui::BeginGroup();
                {
                  ImGui::SetNextItemWidth(50*scale);
                  ImGui::InputDouble(("##setorb"+std::to_string(i)).c_str(), &mParams.objOrbs[i], 0.0, 0.0, "%.2f", ImGuiInputTextFlags_None);
                  ImGui::SetNextItemWidth(symSize+10.0f*scale);
                  ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
                  ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
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
                    if(mParams.objFocused[i] == chart->objects()[i]->focused)
                      {
                        mParams.objFocused[i] = focused;
                        chart->setObjFocus((ObjType)i, mParams.objFocused[i]);
                      }
                  }

                if(i % numPerColumn == (numPerColumn-1))
                  {
                    ImGui::EndGroup();
                    ImGui::TableNextColumn();
                    grouping = false;
                  }
              }
            if(grouping)
              {
                ImGui::EndGroup();
                grouping = false;
              }
            ImGui::Separator();
          }
          ImGui::EndTable();
          ImGui::Unindent();
        }
      else if(isBodyVisible()) { mOrbsOpen = false; }
      //ImGui::EndGroup();
      ImGui::Unindent();
    }
  else if(isBodyVisible()) { mOptionsOpen = false; }

  
  if(chart)
    { // draw chart view
      mView.draw(chart, mChartWidth*scale, isBlocked(), mParams);
      processInput(chart);
    }
  else // draw empty chart
    { mView.draw((Chart*)nullptr, mChartWidth*scale, isBlocked(), mParams); }
}

