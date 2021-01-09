#include "compareNode.hpp"
using namespace astro;

#include "imgui.h"
#include "glfwKeys.hpp"
#include "tools.hpp"
#include "chartCompare.hpp"
#include "chartView.hpp"
#include "chartParamWidget.hpp"
#include "setting.hpp"

CompareNode::CompareNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Compare Node"),
    mCompare(new ChartCompare()), mView(new ChartView()), mParams(new ChartParams())
{
  mWidget = new ChartParamWidget(mParams);
  
  mSettings.push_back(new Setting<bool> ("Settings Open",          "settingsOpen", &mWidget->settingsOpen()));
  mSettings.push_back(new Setting<bool> ("Object Display Open",    "objDispOpen",  &mWidget->objDisplayOpen()));
  mSettings.push_back(new Setting<bool> ("Object Orbs Open",       "objOrbsOpen",  &mWidget->objOrbsOpen()));
  mSettings.push_back(new Setting<bool> ("Aspect Display Open",    "aspDispOpen",  &mWidget->aspDisplayOpen()));
  mSettings.push_back(new Setting<bool> ("Aspect Orbs Open",       "aspOrbsOpen",  &mWidget->aspOrbsOpen()));
  
  mSettings.push_back(new Setting<float>("Chart Width",     "width",        &mParams->chartWidth));
  mSettings.push_back(new Setting<bool> ("Align Ascendant", "alignAsc",     &mParams->alignAsc));
  mSettings.push_back(new Setting<bool> ("Show Houses",     "showHouses",   &mParams->showHouses));

  SettingGroup *group = nullptr;
  group = makeSettingGroup<BoolStruct, (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)> ("Visible Objects", "objVisible", &mParams->objVisible);
  if(group) { mSettings.push_back(group); }
  group = makeSettingGroup<double,     (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)> ("Object Orbs",     "objOrbs",    &mParams->objOrbs);
  if(group) { mSettings.push_back(group); }
  group = makeSettingGroup<BoolStruct, ASPECT_COUNT>                     ("Visible Aspects", "aspVisible", &mParams->aspVisible);
  if(group) { mSettings.push_back(group); }
  group = makeSettingGroup<double,     ASPECT_COUNT>                     ("Aspect Orbs",     "aspOrbs",    &mParams->aspOrbs);
  if(group) { mSettings.push_back(group); }
}

CompareNode::~CompareNode()
{
  if(mWidget)  { delete mWidget; }
  if(mView)    { delete mView; }
  if(mParams)  { delete mParams; }
  if(mCompare) { delete mCompare; }
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
  
  mParams->alpha = mColorMask.w;
  mWidget->setChart(mCompare->getOuterChart());
  mWidget->draw(scale, isBlocked(), isBodyVisible());
  
  mView->setAlignAsc(mParams->alignAsc);
  mView->setShowHouses(mParams->showHouses);

  if(mCompare->getInnerChart())
    {
      // for(int i = 0; i < OBJ_END; i++)
      //   { mParams->objFocused[i].data = mCompare->getInnerChart()->getObject((ObjType)i)->focused; }
      //mCompare->setParams(*mParams);
      mCompare->update();
    }
  
  // draw chart view
  mView->draw(mCompare, scale, isBlocked(), *mParams);
}



