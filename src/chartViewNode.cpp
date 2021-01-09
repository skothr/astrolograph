#include "chartViewNode.hpp"
using namespace astro;

#include "imgui.h"
#include "glfwKeys.hpp"
#include "tools.hpp"
#include "chart.hpp"
#include "chartView.hpp"
#include "chartParamWidget.hpp"
#include "setting.hpp"


#define NUM_PER_COL 4
  
ChartViewNode::ChartViewNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Chart View Node"),
    mView(new ChartView()), mParams(new ChartParams())
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

  mSettings.push_back(makeSettingGroup<BoolStruct, (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)> ("Visible Objects", "objVisible", &mParams->objVisible));
  mSettings.push_back(makeSettingGroup<double,     (OBJ_COUNT+OBJ_END-ANGLE_OFFSET)> ("Object Orbs",     "objOrbs",    &mParams->objOrbs));
  mSettings.push_back(makeSettingGroup<BoolStruct, ASPECT_COUNT>                     ("Visible Aspects", "aspVisible", &mParams->aspVisible));
  mSettings.push_back(makeSettingGroup<double,     ASPECT_COUNT>                     ("Aspect Orbs",     "aspOrbs",    &mParams->aspOrbs));
}

ChartViewNode::~ChartViewNode()
{
  if(mWidget) { delete mWidget; }
  if(mView)   { delete mView;   }
  if(mParams) { delete mParams; }
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

void ChartViewNode::onDraw()
{
  float scale = getScale();
  ViewSettings *viewSettings = getViewSettings();
  
  bool changed = false;
  Chart *chart = inputs()[CHARTVIEWNODE_INPUT_CHART]->get<Chart>();
  
  //if(chart) { chart->setParams(*mParams); }
  mParams->alpha = mColorMask.w;
  mWidget->setChart(chart);
  mWidget->draw(scale, isBlocked() || mPlacing, isBodyVisible());
  mView->setAlignAsc(mParams->alignAsc);
  mView->setShowHouses(mParams->showHouses);
  
  if(chart)
    { // draw chart view
      chart->setParams(*mParams);
      mView->draw(chart, scale, isBlocked(), *mParams);
      chart->setParams(*mParams);
    }
  else // draw empty chart
    { mView->draw((Chart*)nullptr, scale, isBlocked(), *mParams); }
  
  mActive |= mEditing;
  if(!(ImGui::IsItemHovered() || ImGui::IsWindowHovered() || isHovered()))
    { mActive = false; }
}

