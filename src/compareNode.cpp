#include "compareNode.hpp"
using namespace astro;

#include "imgui.h"
#include "glfwKeys.hpp"
#include "tools.hpp"

CompareNode::CompareNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Compare Node"), mCompare(new ChartCompare())
{
  for(int i = 0; i < OBJ_END; i++)
    { mParams.objVisible[i] = true; }
  // setMinSize(Vec2f(512, 512));
}

CompareNode::~CompareNode()
{
  delete mCompare;
}

void CompareNode::processInput(Chart *chart)
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
      // if(changed)
      //   { inputs()[CHARTVIEWNODE_INPUT_CHART]->setReset(true); }
      
      mActive |= (changed || mEditYear || mEditMonth || mEditDay || mEditHour || mEditMinute || mEditSecond || mEditLat || mEditLon || mEditAlt);
    }
}

void CompareNode::onUpdate()
{ }


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
  
  ImGui::Text("Chart Size:  "); // size of chart area
  ImGui::SameLine();
  ImGui::SetNextItemWidth(360*scale);
  ImGui::SliderFloat("##chartWidth", &mChartWidth, CHART_SIZE_MIN, CHART_SIZE_MAX, "%.0f"); // Minimal displayed value is 5%

  // draw chart view
  mView.draw(mCompare, mChartWidth*scale, isBlocked(), mParams);

  if(mCompare->getOuterChart())
    { processInput(mCompare->getOuterChart()); }
}



