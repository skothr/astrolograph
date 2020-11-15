#include "plotNode.hpp"
using namespace astro;

#include "imgui.h"
#include "tools.hpp"


PlotNode::PlotNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Plot Node")
{
  setMinSize(Vec2f(1024, 512));
}

PlotNode::~PlotNode()
{ }

bool PlotNode::onDraw()
{
  Chart *chart = inputs()[PLOTNODE_INPUT_CHART]->get<Chart>();
  // DateTime *dtStartIn = inputs()[PLOTNODE_INPUT_STARTDATE]->get<DateTime>();
  // DateTime *dtEndIn   = inputs()[PLOTNODE_INPUT_ENDDATE]->get<DateTime>();

  Vec2f cursorPos = ImGui::GetCursorScreenPos();

  // TODO: May only need to update data points at start and end, or adjust!
  if(chart)
    {
      ObjType obj = OBJ_MARS;
      DateTime dtOrig  = chart->date();
      DateTime dtStart = dtOrig;
      DateTime dtEnd   = dtOrig;
      dtStart.setDay(dtStart.day() - mDayRadius); dtStart.fix();
      dtEnd.setDay(dtEnd.day() + mDayRadius); dtEnd.fix();
      
      if(dtStart.year() != mOldStartDate.year() || dtStart.month() != mOldStartDate.month() || dtStart.day() != mOldStartDate.day() ||
         dtEnd.year()   != mOldEndDate.year()   || dtEnd.month()   != mOldEndDate.month()   || dtEnd.day()   != mOldEndDate.day()   ||
         chart->getHouseSystem() != mOldChart.getHouseSystem() ||
         chart->getDraconic()    != mOldChart.getDraconic()    ||
         chart->getSidereal()    != mOldChart.getSidereal()    ||
         chart->getTruePos()     != mOldChart.getTruePos())
        {
          mOldChart.setDate(dtOrig);
          mOldChart.setLocation(chart->location());
          mOldChart.setHouseSystem(chart->getHouseSystem());
          mOldChart.setDraconic(chart->getDraconic());
          mOldChart.setSidereal(chart->getSidereal());
          mOldChart.setTruePos(chart->getTruePos());
          
          mData.clear();
          mData.reserve(2*mDayRadius);
          DateTime dt = dtStart;
          for(int i = 0; i < 2*mDayRadius; i++)
            {
              mOldChart.setDate(dt);
              mData.push_back(mOldChart.getSingleAngle(obj));
              dt.setDay(dt.day()+1); dt.fix();
            }
          mOldChart.setDate(dtOrig);
          mOldStartDate = dtStart;
          mOldEndDate   = dtEnd;
        }
      std::string overlay = dtStart.toString() + " --> " + dtEnd.toString();
      ImGui::PlotLines(getObjName(obj).c_str(), mData.data(), mData.size(), 0, overlay.c_str(), 0.0f, 360.0f, Vec2f(950, 500));
      ImGui::GetWindowDrawList()->AddLine(Vec2f(475, 0)+cursorPos, Vec2f(475, 500)+cursorPos, ImColor(Vec4f(1.0f, 0.0f, 0.0f, 1.0f)), 2.0f);
    }
  
  ImGui::InputInt("Day Radius", &mDayRadius, 1, 10);
  //if(mDayRadius > 365) { mDayRadius = 365; } // clamp number of data points
  
  return true;
}

