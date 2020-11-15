#include "chartViewNode.hpp"
using namespace astro;

#include "glfwKeys.hpp"
#include "imgui.h"
#include "tools.hpp"

ChartViewNode::ChartViewNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Chart View Node")
{
  // setMinSize(Vec2f(512, 512));
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
    }
}


bool ChartViewNode::onDraw()
{
  bool changed = false;
  Chart *chart = inputs()[CHARTVIEWNODE_INPUT_CHART]->get<Chart>();
  //outputs()[CHARTVIEWNODE_OUTPUT_CHART]->set(chart);

  //  ImGui::BeginGroup();
  {
    // size of chart
    ImGui::TextUnformatted("Chart Size:  ");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(360);
    ImGui::SliderFloat("##chartWidth", &mChartWidth, CHART_SIZE_MIN, CHART_SIZE_MAX, "%.0f");

    if(chart)
      { // draw chart view
        mView.draw(chart, mChartWidth);
        processInput(chart);
        ImGui::Spacing();
        ImGui::Spacing();
        
        double tzOffset = chart->date().tzOffset();
        ImGui::TextColored(ImColor(1.0f, 1.0f, 1.0f, 0.25f), "UTC%s%.1f", (tzOffset >= 0 ? "+" : ""), tzOffset);
        
        const DateTime &date = chart->date();
        const Location &loc  = chart->location();
        
        ImGui::TextUnformatted(date.toString().c_str());
        ImGui::TextUnformatted(loc.toString().c_str());
        // ImGui::Spacing();
      }
    else // draw empty chart
      { mView.draw((Chart*)nullptr, mChartWidth); }
  }
  //ImGui::EndGroup();
  
  return true;
}

