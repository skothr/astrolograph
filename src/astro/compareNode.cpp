#include "compareNode.hpp"
using namespace astro;

#include "imgui.h"
#include "glfwKeys.hpp"
#include "tools.hpp"
#include "chartCompare.hpp"
#include "chartView.hpp"
#include "chartParamWidget.hpp"
#include "chartOrbWidget.hpp"
#include "setting.hpp"

CompareNode::CompareNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Compare Node"),
    mCompare(new ChartCompare()), mView(new ChartView()), mParams(new ChartParams())
{
  mParamWidget = new ChartParamWidget(mParams);
  mOrbWidget = new ChartOrbWidget();
  
  mSettings.push_back(new Setting<bool> ("Settings Open",          "settingsOpen", &mParamWidget->settingsOpen()));
  mSettings.push_back(new Setting<bool> ("Object Display Open",    "objDispOpen",  &mParamWidget->objDisplayOpen()));
  mSettings.push_back(new Setting<bool> ("Aspect Display Open",    "aspDispOpen",  &mParamWidget->aspDisplayOpen()));
  // mSettings.push_back(new Setting<bool> ("Object Orbs Open",       "objOrbsOpen",  &mParamWidget->objOrbsOpen()));
  // mSettings.push_back(new Setting<bool> ("Aspect Orbs Open",       "aspOrbsOpen",  &mParamWidget->aspOrbsOpen()));
  mSettings.push_back(new Setting<bool> ("Orbs Open",              "orbsOpen",     &mParamWidget->orbsOpen()));
  
  mSettings.push_back(new Setting<float>("Chart Width",     "width",        &mParams->chartWidth));
  mSettings.push_back(new Setting<bool> ("Align Ascendant", "alignAsc",     &mParams->alignAsc));
  mSettings.push_back(new Setting<bool> ("Show Houses",     "showHouses",   &mParams->showHouses));

  SettingGroup *group = nullptr;
  group = makeSettingGroup<BoolStruct, (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)> ("Visible Objects", "objVisible", &mParams->objVisible);
  if(group) { mSettings.push_back(group); }
  group = makeSettingGroup<BoolStruct, ASPECT_COUNT>                     ("Visible Aspects", "aspVisible", &mParams->aspVisible);
  if(group) { mSettings.push_back(group); }
  // group = makeSettingGroup<double,     (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)> ("Object Orbs",     "orbs",    &mParams->orbs);
  // if(group) { mSettings.push_back(group); }
  // group = makeSettingGroup<double,     ASPECT_COUNT>                     ("Aspect Orbs",     "aspOrbs",    &mParams->aspOrbs);
  // if(group) { mSettings.push_back(group); }
}

CompareNode::~CompareNode()
{
  if(mParamWidget) { delete mParamWidget; }
  if(mOrbWidget)   { delete mOrbWidget; }
  if(mView)        { delete mView; }
  if(mParams)      { delete mParams; }
  if(mCompare)     { delete mCompare; }
}

Chart* CompareNode::outerChart() { return mCompare->getOuterChart(); }
Chart* CompareNode::innerChart() { return mCompare->getInnerChart(); }

void CompareNode::processInput(Chart *chart)
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

void CompareNode::onUpdate()
{
  Chart *chart = inputs()[COMPARENODE_INPUT_CHART_OUTER]->get<Chart>();
  if(chart) { processInput(chart); }
}


void CompareNode::onDraw()
{
  float scale = getScale();
  
  Chart *iChart = inputs()[COMPARENODE_INPUT_CHART_INNER]->get<Chart>();
  Chart *oChart = inputs()[COMPARENODE_INPUT_CHART_OUTER]->get<Chart>();

  bool changed = false;
  if(oChart != mCompare->getOuterChart())
    { mCompare->setOuterChart(oChart); changed = true; }
  if(iChart != mCompare->getInnerChart())
    { mCompare->setInnerChart(iChart); changed = true; }
  if((oChart && ((oChart->date() != mDateOuter) || (oChart->location() != mLocOuter))) ||
     (iChart && ((iChart->date() != mDateInner) || (iChart->location() != mLocInner))))
    { changed = true; }
  
  if(changed) { mCompare->update(); changed = false; }

  bool visible = isBodyVisible();
  bool blocked = isBlocked();
  
  mParams->alpha = mColorMask.w;
  mParamWidget->setChart(mCompare->getOuterChart());
  
  ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
  ImGui::SetNextTreeNodeOpen(mSettingsOpen);
  if(ImGui::CollapsingHeader("Settings", nullptr, flags))
    {
      if(!blocked) { mSettingsOpen = true; }
      ImGui::Indent();
      
      mParamWidget->draw(scale, blocked, visible);

      ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
      ImGui::SetNextTreeNodeOpen(mParamWidget->orbsOpen());
      if(ImGui::CollapsingHeader("Orbs", nullptr, flags))
        {
          mParamWidget->orbsOpen() = true;
          mOrbWidget->draw(scale);
        }
      else if(visible) { mParamWidget->orbsOpen() = false; }
    }
  else if(visible) { mSettingsOpen = false; }
  
  mParams->orbs = mOrbWidget->getOrbs();
  mView->setAlignAsc(mParams->alignAsc);
  mView->setShowHouses(mParams->showHouses);

  if(mCompare->getInnerChart())
    {
      // for(int i = 0; i < OBJ_END; i++)
      //   { mParams->objFocused[i].data = mCompare->getInnerChart()->getObject(i)->focused; }
      //mCompare->setParams(*mParams);
      mCompare->update();
    }
  
  // draw chart view
  mView->draw(mCompare, scale, blocked, *mParams);
}




// void CompareNode::drawSettings()
// {
//   float scale = getScale();
//   bool blocked = isBlocked() || mPlacing;
//   bool visible = isBodyVisible();
//   ViewSettings *viewSettings = getViewSettings();
//   Chart *chart = inputs()[COMPARENODE_INPUT_CHART_INNER]->get<Chart>();
  
//   ImGuiIO& io = ImGui::GetIO();
//   float symSize = 20.0f*scale;

//   // options
//   ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
//   ImGui::SetNextTreeNodeOpen(mSettingsOpen);
//   if(ImGui::CollapsingHeader("Settings", nullptr, flags))
//     {
//       if(!blocked) { mSettingsOpen = true; }
//       ImGui::Indent();
      
//       float columnW = 180.0f*scale;
      
//       { // size of chart
//         ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(0.0f, 2.0f));
//         ImGui::TextUnformatted("Chart Size ");
//         ImGui::SameLine(columnW);
//         ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) - Vec2f(0.0f, 2.0f));
//         ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0.0f, 0.0f));
//         // ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, ImGui::GetStyle().GrabMinSize*scale);
//         ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, Vec2f(2.0f, 2.0f)*scale);
//         ImGui::SetNextItemWidth(330*scale);
//         ImGui::SliderFloat("##chartWidth", &mParams->chartWidth, CHART_SIZE_MIN, CHART_SIZE_MAX, "%.0f");
//         ImGui::PopStyleVar(2);
//       }

//       // align ascendant
//       ImGui::TextUnformatted("Align Ascendant");
//       ImGui::SameLine(columnW);
//       bool align = mParams->alignAsc;
//       ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
//       if(ImGui::Checkbox("##align", &align))
//         { mParams->alignAsc = align; }
      
//       // show houses 
//       ImGui::TextUnformatted("Show Houses    ");
//       ImGui::SameLine(columnW);
//       bool show = mParams->showHouses;
//       if(ImGui::Checkbox("##show", &show))
//         { mParams->showHouses = show; }
//       ImGui::PopStyleVar();
      
//       // OBJECT VISIBILITY //
//       // display settings (toggle object/angle visibility)
//       ImGui::SetNextTreeNodeOpen(mDisplayOpen);
//       if(ImGui::CollapsingHeader("Display", nullptr, flags))
//         {
//           mDisplayOpen = true;

//           ImGui::Spacing();
//           ImGui::PushFont(viewSettings->titleFont);
//           ImGui::TextUnformatted("Objects");
//           ImGui::PopFont();
//           ImGui::Separator();
//           ImGui::Indent();
          
//           ImGui::BeginTable("##angVis", 4, ImGuiTableFlags_SizingPolicyStretchX | ImGuiTableFlags_NoClip);
//           ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
//           ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
//           ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
//           ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);
//           {
//             ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
//             // angles
//             // ImGui::BeginGroup();
//             int numPerColumn = 4;
//             bool grouping = false;
//             for(int a = ANGLE_OFFSET; a < ANGLE_END; a++)
//               {
//                 if((a-ANGLE_OFFSET) % numPerColumn == 0)
//                   {
//                     if(a > ANGLE_OFFSET) { ImGui::Spacing(); }
//                     ImGui::BeginGroup();
//                     grouping = true;
//                   }
//                 int i = OBJ_COUNT+a-ANGLE_OFFSET; // obj index
//                 std::string name = getObjName(a);
//                 std::string longName = getObjNameLong(a);
//                 ChartImage *img = getWhiteImage(name);
//                 Vec4f color = getObjColor(name);
//                 ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

//                 ImGui::BeginGroup();
//                 {
//                   bool checked = mParams->objVisible[i];
//                   // if(a > ANGLE_OFFSET) { ImGui::SameLine(columnW*(a-ANGLE_OFFSET)); }
//                   ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
//                   if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
//                     {
//                       mParams->objVisible[i] = checked;
//                     }
//                   ImGui::PopStyleVar();
                  
//                   ImGui::SetNextItemWidth(symSize+10.0f*scale);
//                   ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
//                   ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
//                 }
//                 ImGui::EndGroup();
                
//                 bool hover = ImGui::IsItemHovered();
//                 if(chart)
//                   {
//                     if(hover)
//                       { // angle tooltip
//                         ChartObject *obj = chart->getObject(i);
//                         ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  Vec2f(ImGui::GetStyle().FramePadding)/scale);
//                         ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
//                         ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(ImGui::GetStyle().WindowPadding)/scale);
//                         ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing/scale);
//                         ImGui::SetTooltip("%s - %s %s", name.c_str(), getSignName(chart->getSign(obj->angle)).c_str(),
//                                           angle_string(fmod(obj->angle, 30.0), false).c_str());
//                         ImGui::PopStyleVar(4);
//                       }
//                     // set focus
//                     bool focused = (hover && io.KeyShift); // focus on this object with SHIFT+hover
//                     if(focused || !mParams->objFocused[i] || (mParams->objFocused[i] && mObjDisplayFocused[i]))
//                       {
//                         mParams->objFocused[i] = focused;
//                         mObjDisplayFocused[i] = focused;
//                       }
//                   }
                
//                 if((a-ANGLE_OFFSET) % numPerColumn == (numPerColumn-1))
//                   {
//                     ImGui::EndGroup();
//                     ImGui::TableNextColumn();
//                     grouping = false;
//                   }
//                 //ImGui::TableNextColumn();
//               }
//             if(grouping)
//               {
//                 // ImGui::Spacing();
//                 ImGui::EndGroup();
//                 grouping = false;
//               }

//             ImGui::TableNextRow(); ImGui::TableSetColumnIndex(0);
//             ImGui::Separator();
            
//             // objects
//             numPerColumn = 5;
//             for(int i = 0; i < OBJ_COUNT; i++)
//               {
//                 if(i % numPerColumn == 0)
//                   {
//                     if(i > 0) { ImGui::Spacing(); }
//                     ImGui::BeginGroup();
//                     grouping = true;
//                   }
              
//                 std::string name = getObjName(i);
//                 std::string longName = getObjNameLong(i);
//                 ChartImage *img = getWhiteImage(name);
//                 Vec4f color = getObjColor(name);
//                 ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

//                 ImGui::BeginGroup();
//                 {
//                   bool checked = mParams->objVisible[i];
                  
//                   ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
//                   if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
//                     {
//                       mParams->objVisible[i] = checked;
//                     }
//                   ImGui::PopStyleVar();
              
//                   ImGui::SetNextItemWidth(symSize+10.0f*scale);
//                   ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
//                   ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
//                 }
//                 ImGui::EndGroup();
                
//                 bool hover = !blocked && ImGui::IsItemHovered();
//                 if(chart)
//                   {
//                     if(hover)
//                       { // object tooltip
//                         ChartObject *obj = chart->getObject(i);
//                         ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  Vec2f(ImGui::GetStyle().FramePadding)/scale);
//                         ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
//                         ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(ImGui::GetStyle().WindowPadding)/scale);
//                         ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing/scale);
//                         ImGui::SetTooltip("%s - %s %s", name.c_str(), getSignName(chart->getSign(obj->angle)).c_str(),
//                                           angle_string(fmod(obj->angle, 30.0), false).c_str());
//                         ImGui::PopStyleVar(4);
//                       }
//                     // set focus
//                     bool focused = (hover && io.KeyShift); // focus on this object with SHIFT+hover
//                     if(focused || !mParams->objFocused[i] || (mParams->objFocused[i] && mObjDisplayFocused[i]))
//                       {
//                         mParams->objFocused[i] = focused;
//                         mObjDisplayFocused[i] = focused;
//                       }
//                   }

//                 if(i % numPerColumn == (numPerColumn-1))
//                   {
//                     ImGui::EndGroup();
//                     ImGui::TableNextColumn();
//                     grouping = false;
//                   }
//               }
//             if(grouping)
//               {
//                 ImGui::EndGroup();
//                 grouping = false;
//               }
//           }
//           //ImGui::Separator();
//           ImGui::EndTable();
//           ImGui::Unindent();
//           // ImGui::Separator();
//           ImGui::Spacing();


//           // aspect visibility
//           ImGui::PushFont(viewSettings->titleFont);
//           ImGui::TextUnformatted("Aspects");
//           ImGui::PopFont();
//           ImGui::Separator();
//           ImGui::Indent();
      
//           std::vector<ChartAspect> aspects;
//           std::array<int, ASPECT_COUNT> counts = {0};
//           if(chart)
//             {
//               aspects = chart->calcAspects(*mParams);
//               for(auto asp : aspects) { counts[asp.type]++; }
//             }
      
//           int numPerColumn = 4;
//           bool grouping = false;
//           for(int a = 0; a < ASPECT_COUNT; a++)
//             {
//               int i = a; // aspect type index
//               std::string name = getAspectName(a);
//               std::string longName = getAspectNameLong(a);
//               ChartImage *img = getImage(name);
//               Vec4f color = getAspectInfo(a)->color;
//               ImVec4 tintCol = ImVec4(color.x, color.y, color.z, color.w);

//               ImGui::BeginGroup();
//               {
//                 bool checked = mParams->aspVisible[i];
//                 ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
//                 if(ImGui::Checkbox(("##show-"+name).c_str(), &checked))
//                   {
//                     mParams->aspVisible[i] = checked;
//                     // if(chart) { chart->setAspectVisible(a, checked); }
//                   }
//                 ImGui::PopStyleVar();
                
//                 ImGui::SetNextItemWidth(symSize+10.0f*scale);
//                 ImGui::SameLine(); ImGui::Image(img->id(), ImVec2(symSize, symSize), ImVec2(0,0), ImVec2(1,1), tintCol, ImVec4(0,0,0,0));
//                 ImGui::SameLine(); ImGui::Text("%s", longName.c_str());
//               }
//               ImGui::EndGroup();
                
//               bool hover = ImGui::IsItemHovered();
//               if(chart)
//                 {
//                   if(hover)
//                     { // aspect tooltip
//                       ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,  Vec2f(ImGui::GetStyle().FramePadding)/scale);
//                       ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   Vec2f(ImGui::GetStyle().ItemSpacing)/scale);
//                       ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, Vec2f(ImGui::GetStyle().WindowPadding)/scale);
//                       ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetStyle().IndentSpacing/scale);
//                       ImGui::SetTooltip("%s (%d)", getAspectName(a).c_str(), counts[a]);
//                       ImGui::PopStyleVar(4);
//                     }   
//                   // set focus
//                   bool focused = (hover && io.KeyShift); // focus on this aspect type with SHIFT+hover
//                   if(focused || !mParams->aspFocused[a] || (mParams->aspFocused[a] && mAspDisplayFocused[a]))
//                     {
//                       mParams->aspFocused[a] = focused;
//                       mAspDisplayFocused[a] = focused;
//                     }
                  
//                 }
//             }
//           //ImGui::Separator();
//           ImGui::Spacing();
//           ImGui::Unindent();
//         }
//       else if(visible) { mDisplayOpen = false; }

//       // ORBS //
//       ImGui::SetNextTreeNodeOpen(mOrbsOpen);
//       if(ImGui::CollapsingHeader("Orbs", nullptr, flags))
//         {
//           mOrbsOpen = true;
//           mOrbWidget->draw(scale);
//         }
//       else if(visible) { mOrbsOpen = false; }
      
//       std::vector<ChartAspect> aspects;
//       std::array<int, ASPECT_COUNT> counts = {0};
//       if(chart)
//         {
//           aspects = chart->calcAspects(*mParams);
//           for(auto asp : aspects) { counts[asp.type]++; }
//         }
//       ImGui::Unindent();
//     } 
//   else if(visible) { mSettingsOpen = false; }
// }
