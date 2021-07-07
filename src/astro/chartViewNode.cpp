#include "chartViewNode.hpp"
using namespace astro;

#include "imgui.h"
#include "glfwKeys.hpp"
#include "tools.hpp"
#include "chart.hpp"
#include "chartView.hpp"
#include "chartParamWidget.hpp"
#include "chartOrbWidget.hpp"
#include "setting.hpp"
#include "viewSettings.hpp"



#define NUM_PER_COL 4
  
ChartViewNode::ChartViewNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Chart View Node", true),
    mView(new ChartView()), mParams(new ChartParams())
{
  mOrbWidget   = new ChartOrbWidget();

  mSettings.push_back(new Setting<bool> ("Settings Open",   "settingsOpen", &mSettingsOpen));
  mSettings.push_back(new Setting<bool> ("Display Open",    "displayOpen",  &mDisplayOpen));
  mSettings.push_back(new Setting<bool> ("Orbs Open",       "orbsOpen",     &mOrbsOpen));
  
  mSettings.push_back(new Setting<float>("Chart Width",     "width",        &mParams->chartWidth));
  mSettings.push_back(new Setting<bool> ("Align Ascendant", "alignAsc",     &mParams->alignAsc));
  mSettings.push_back(new Setting<bool> ("Show Houses",     "showHouses",   &mParams->showHouses));

  for(int o = 0; o < OBJ_END; o++)
    {
      for(int a = 0; a < ASPECT_COUNT; a++)
        {
          std::string oName = getObjName(o);
          std::string aName = getAspectName(a);
          mSettings.push_back(new Setting<float>(oName + "/" + aName + "  Orb", "orb"+oName+aName, &mOrbWidget->getOrbs().objOrbs[o][a]));
        }
    }
  
  mSettings.push_back(makeSettingGroup<BoolStruct, (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)> ("Visible Objects", "objVisible", &mParams->objVisible));
  mSettings.push_back(makeSettingGroup<BoolStruct, ASPECT_COUNT>                     ("Visible Aspects", "aspVisible", &mParams->aspVisible));
}

ChartViewNode::~ChartViewNode()
{
  if(mOrbWidget)   { delete mOrbWidget; }
  if(mView)        { delete mView;   }
  if(mParams)      { delete mParams; }
}

void ChartViewNode::processInput(Chart *chart)
{
  if(chart)
    {
      mEditYear = ImGui::IsKeyDown(GLFW_KEY_1); mEditMonth  = ImGui::IsKeyDown(GLFW_KEY_2); mEditDay    = ImGui::IsKeyDown(GLFW_KEY_3);
      mEditHour = ImGui::IsKeyDown(GLFW_KEY_4); mEditMinute = ImGui::IsKeyDown(GLFW_KEY_5); mEditSecond = ImGui::IsKeyDown(GLFW_KEY_6);
      mEditLat  = ImGui::IsKeyDown(GLFW_KEY_Q); mEditLon    = ImGui::IsKeyDown(GLFW_KEY_W); mEditAlt    = ImGui::IsKeyDown(GLFW_KEY_E);
      // scroll to edit date -- hold number keys for date, or Q/W/E for location
      // 1 --> year,     2 --> month,     3 --> day,     4 --> hour,     5 --> minute,     6 --> second
      // Q --> latitude, W --> longitude, E --> altitude
      if(isHovered() && !isBlocked())
        {
          bool changed = false;
          ImGuiIO &io = ImGui::GetIO();
          DateTime dt  = chart->date();
          Location loc = chart->location();
          
          // key multipliers
          float mult = 1.0f;
          if(io.KeyShift) { mult *= 0.1f;   } // SHIFT --> x0.1
          if(io.KeyCtrl)  { mult *= 10.0f;  } // CTRL  --> x10
          if(io.KeyAlt)   { mult *= 60.0f;  } // ALT   --> x60
          float delta = io.MouseWheel*mult;
          if(delta != 0.0f)
            {
              if(mEditYear)   { dt.setYear  (dt.year()   + delta); changed = true; } // date/time
              if(mEditMonth)  { dt.setMonth (dt.month()  + delta); changed = true; }
              if(mEditDay)    { dt.setDay   (dt.day()    + delta); changed = true; }
              if(mEditHour)   { dt.setHour  (dt.hour()   + delta); changed = true; }
              if(mEditMinute) { dt.setMinute(dt.minute() + delta); changed = true; }
              if(mEditSecond) { dt.setSecond(dt.second() + delta); changed = true; }
              if(mEditLat)    { loc.latitude  += delta;            changed = true; } // location
              if(mEditLon)    { loc.longitude += delta;            changed = true; }
              if(mEditAlt)    { loc.altitude  += delta;            changed = true; }
              chart->setDate(dt.fixed());
              chart->setLocation(loc.fixed());
            }
          // ESCAPE -- reset to current time
          if(ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) { chart->setDate(DateTime::now()); changed = true; }
          mEditing = (changed || mEditYear || mEditMonth || mEditDay || mEditHour || mEditMinute || mEditSecond || mEditLat || mEditLon || mEditAlt);
        }
    }
}

void ChartViewNode::onUpdate()
{
  Chart *chart = inputs()[CHARTVIEWNODE_INPUT_CHART]->get<Chart>();
  if(chart)
    {
      outputs()[CHARTVIEWNODE_OUTPUT_CHART]->set(chart);
      processInput(chart);
      //chart->update();
    }
}

void ChartViewNode::drawSettings()
{
  float scale = getScale();
  bool blocked = isBlocked() || mPlacing;
  bool visible = isBodyVisible();
  ViewSettings *viewSettings = getViewSettings();
  Chart *chart = inputs()[CHARTVIEWNODE_INPUT_CHART]->get<Chart>();
  
  ImGuiIO& io = ImGui::GetIO();
  float symSize = 20.0f*scale;

  // options
  ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
  ImGui::SetNextTreeNodeOpen(mSettingsOpen);
  if(ImGui::CollapsingHeader("Settings", nullptr, flags))
    {
      if(!blocked) { mSettingsOpen = true; }
      ImGui::Indent();
      
      float columnW = 180.0f*scale;
      
      { // size of chart
        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(0.0f, 2.0f));
        ImGui::TextUnformatted("Chart Size ");
        ImGui::SameLine(columnW);
        ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) - Vec2f(0.0f, 2.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0.0f, 0.0f));
        // ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, ImGui::GetStyle().GrabMinSize*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, Vec2f(2.0f, 2.0f)*scale);
        ImGui::SetNextItemWidth(330*scale);
        ImGui::SliderFloat("##chartWidth", &mParams->chartWidth, CHART_SIZE_MIN, CHART_SIZE_MAX, "%.0f");
        ImGui::PopStyleVar(2);
      }

      // align ascendant
      ImGui::TextUnformatted("Align Ascendant");
      ImGui::SameLine(columnW);
      bool align = mParams->alignAsc;
      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
      if(ImGui::Checkbox("##align", &align))
        { mParams->alignAsc = align; }
      
      // show houses 
      ImGui::TextUnformatted("Show Houses    ");
      ImGui::SameLine(columnW);
      bool show = mParams->showHouses;
      if(ImGui::Checkbox("##show", &show))
        { mParams->showHouses = show; }
      ImGui::PopStyleVar();
      
      // OBJECT VISIBILITY //
      // display settings (toggle object/angle visibility)
      ImGui::SetNextTreeNodeOpen(mDisplayOpen);
      if(ImGui::CollapsingHeader("Display", nullptr, flags))
        {
          mDisplayOpen = true;

          ImGui::Spacing();
          ImGui::PushFont(viewSettings->titleFont);
          ImGui::TextUnformatted("Objects");
          ImGui::PopFont();
          ImGui::Separator();
          ImGui::Indent();
          
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
                std::string name = getObjName(a);
                std::string longName = getObjNameLong(a);
                ChartImage *img = getWhiteImage(name);
                Vec4f color = getObjColor(name);
                ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

                ImGui::BeginGroup();
                {
                  bool checked = mParams->objVisible[i];
                  // if(a > ANGLE_OFFSET) { ImGui::SameLine(columnW*(a-ANGLE_OFFSET)); }
                  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
                  if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
                    {
                      mParams->objVisible[i] = checked;
                    }
                  ImGui::PopStyleVar();
                  
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
                        ChartObject *obj = chart->getObject(i);
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
                    if(focused || !mParams->objFocused[i] || (mParams->objFocused[i] && mObjDisplayFocused[i]))
                      {
                        mParams->objFocused[i] = focused;
                        mObjDisplayFocused[i] = focused;
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
              
                std::string name = getObjName(i);
                std::string longName = getObjNameLong(i);
                ChartImage *img = getWhiteImage(name);
                Vec4f color = getObjColor(name);
                ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

                ImGui::BeginGroup();
                {
                  bool checked = mParams->objVisible[i];
                  
                  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
                  if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
                    {
                      mParams->objVisible[i] = checked;
                    }
                  ImGui::PopStyleVar();
              
                  ImGui::SetNextItemWidth(symSize+10.0f*scale);
                  ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
                  ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
                }
                ImGui::EndGroup();
                
                bool hover = !blocked && ImGui::IsItemHovered();
                if(chart)
                  {
                    if(hover)
                      { // object tooltip
                        ChartObject *obj = chart->getObject(i);
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
                    if(focused || !mParams->objFocused[i] || (mParams->objFocused[i] && mObjDisplayFocused[i]))
                      {
                        mParams->objFocused[i] = focused;
                        mObjDisplayFocused[i] = focused;
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
          //ImGui::Separator();
          ImGui::EndTable();
          ImGui::Unindent();
          // ImGui::Separator();
          ImGui::Spacing();


          // aspect visibility
          ImGui::PushFont(viewSettings->titleFont);
          ImGui::TextUnformatted("Aspects");
          ImGui::PopFont();
          ImGui::Separator();
          ImGui::Indent();
      
          std::vector<ChartAspect> aspects;
          std::array<int, ASPECT_COUNT> counts = {0};
          if(chart)
            {
              aspects = chart->calcAspects(*mParams);
              for(auto asp : aspects) { counts[asp.type]++; }
            }
      
          int numPerColumn = 4;
          bool grouping = false;
          for(int a = 0; a < ASPECT_COUNT; a++)
            {
              int i = a; // aspect type index
              std::string name = getAspectName(a);
              std::string longName = getAspectNameLong(a);
              ChartImage *img = getImage(name);
              Vec4f color = getAspectInfo(a)->color;
              ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

              ImGui::BeginGroup();
              {
                bool checked = mParams->aspVisible[i];
                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
                if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
                  {
                    mParams->aspVisible[i] = checked;
                    // if(chart) { chart->setAspectVisible(a, checked); }
                  }
                ImGui::PopStyleVar();
                
                ImGui::SetNextItemWidth(symSize+10.0f*scale);
                ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
                ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
              }
              ImGui::EndGroup();
                
              bool hover = ImGui::IsItemHovered();
              if(chart)
                {
                  if(hover)
                    { // aspect tooltip
                      ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  Vec2f(ImGui::GetStyle().FramePadding)/scale);
                      ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
                      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(ImGui::GetStyle().WindowPadding)/scale);
                      ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing/scale);
                      ImGui::SetTooltip("%s (%d)", getAspectName(a).c_str(), counts[a]);
                      ImGui::PopStyleVar(4);
                    }   
                  // set focus
                  bool focused = (hover && io.KeyShift); // focus on this aspect type with SHIFT+hover
                  if(focused || !mParams->aspFocused[a] || (mParams->aspFocused[a] && mAspDisplayFocused[a]))
                    {
                      mParams->aspFocused[a] = focused;
                      mAspDisplayFocused[a] = focused;
                    }
                  
                }
            }
          //ImGui::Separator();
          ImGui::Spacing();
          ImGui::Unindent();
        }
      else if(visible) { mDisplayOpen = false; }

      // ORBS //
      ImGui::SetNextTreeNodeOpen(mOrbsOpen);
      if(ImGui::CollapsingHeader("Orbs", nullptr, flags))
        {
          mOrbsOpen = true;
          mOrbWidget->draw(scale);
        }
      else if(visible) { mOrbsOpen = false; }
      
      std::vector<ChartAspect> aspects;
      std::array<int, ASPECT_COUNT> counts = {0};
      if(chart)
        {
          aspects = chart->calcAspects(*mParams);
          for(auto asp : aspects) { counts[asp.type]++; }
        }
      ImGui::Unindent();
    } 
  else if(visible) { mSettingsOpen = false; }
}

void ChartViewNode::onDraw()
{
  float  scale   = getScale();
  bool   changed = false;
  Chart *chart   = inputs()[CHARTVIEWNODE_INPUT_CHART]->get<Chart>();

  //Vec2f p0 = ImGui::GetCursorScreenPos();
  mParams->alpha = mColorMask.w;
  //mParamWidget->setChart(chart); mParamWidget->draw(scale, isBlocked() || mPlacing, isBodyVisible());
  drawSettings();
  
  mView->setAlignAsc(mParams->alignAsc);
  mView->setShowHouses(mParams->showHouses);
  mParams->orbs = mOrbWidget->getOrbs();
  
  if(chart)
    { // draw chart view
      // chart->setParams(*mParams);
      mView->draw(chart, scale, isBlocked(), *mParams);
      chart->setParams(*mParams);
    }
  else // draw empty chart
    { mView->draw((Chart*)nullptr, scale, isBlocked(), *mParams); }
  
  mActive |= mEditing;
  if(!(ImGui::IsItemHovered() || ImGui::IsWindowHovered() || isHovered()))
    { mActive = false; }

  // Vec2f p1 = ImGui::GetCursorScreenPos();

  // Vec2f padding = Vec2f(100,100)*scale;
  // drawAspect(ImGui::GetWindowDrawList(), p0+padding, p1-padding+Vec2f(mParams->chartWidth*scale, 0.0f), Vec4f(1.0f, 0.0f, 0.0f, 1.0f), 1.0, "square", scale);
}

void ChartViewNode::onResize(const Vec2f &dSize)
{
  Vec2f newSize = Vec2f(mParams->chartWidth, mParams->chartWidth) + dSize;
  float newW = std::max(newSize.x, newSize.y);
  mParams->chartWidth = std::max(CHART_SIZE_MIN, std::min(CHART_SIZE_MAX, newW));
}
