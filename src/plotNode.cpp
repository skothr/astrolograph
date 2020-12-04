#include "plotNode.hpp"
using namespace astro;

#include "imgui.h"
#include "tools.hpp"


PlotNode::PlotNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Plot Node")
{
  //setMinSize(Vec2f(1024, 512));
}

PlotNode::~PlotNode()
{ }

void PlotNode::onUpdate()
{
  Chart *chart = inputs()[PLOTNODE_INPUT_CHART]->get<Chart>();
  // DateTime *dtStartIn = inputs()[PLOTNODE_INPUT_STARTDATE]->get<DateTime>();
  // DateTime *dtEndIn   = inputs()[PLOTNODE_INPUT_ENDDATE]->get<DateTime>();
  if(chart)
    { // TODO: May only need to update data points at start and end, or adjust!
      DateTime dtOrig  = chart->date();
      DateTime dtStart = dtOrig;
      DateTime dtEnd   = dtOrig;
      dtStart.setDay(dtStart.day() - mDayRadius); dtStart.fix();
      dtEnd.setDay(dtEnd.day() + mDayRadius); dtEnd.fix();
      
      if(mObjType != mOldObjType ||
         (dtStart.year() != mOldStartDate.year()) || (dtStart.month() != mOldStartDate.month()) || (dtStart.day() != mOldStartDate.day()) ||
         (dtEnd.year()   != mOldEndDate.year())   || (dtEnd.month()   != mOldEndDate.month())   || (dtEnd.day()   != mOldEndDate.day())   ||
         (chart->getHouseSystem() != mOldChart.getHouseSystem()) ||
         (chart->getZodiac()      != mOldChart.getZodiac())      ||
         (chart->getTruePos()     != mOldChart.getTruePos()))
        {
          mOldChart.setDate(dtOrig);
          mOldChart.setLocation(chart->location());
          mOldChart.setHouseSystem(chart->getHouseSystem());
          mOldChart.setZodiac(chart->getZodiac());
          mOldChart.setTruePos(chart->getTruePos());
          
          mData.clear();
          mData.reserve(2*mDayRadius);
          DateTime dt = dtStart;
          for(int i = 0; i < 2*mDayRadius; i++)
            {
              mOldChart.setDate(dt);
              mData.push_back(mOldChart.getSingleAngle(mObjType));
              dt.setDay(dt.day()+1); dt.fix();
            }
          mOldChart.setDate(dtOrig);
          mOldStartDate = dtStart;
          mOldEndDate   = dtEnd;
          mOldObjType   = mObjType;
        }
    }
}

void PlotNode::onDraw()
{
  float scale = getScale();
  
  Chart *chart = inputs()[PLOTNODE_INPUT_CHART]->get<Chart>();
  ImGui::SetNextItemWidth(120*scale);
  ImGui::InputInt("Day Radius", &mDayRadius, 1, 10);

  ImGui::SameLine();
  ImGui::SetNextItemWidth(150*scale);
  if(ImGui::BeginCombo("Object##obj", getObjNameLong(mObjType).c_str()))
    {
      ImGui::SetWindowFontScale(scale);
      for(int o = 0; o < OBJ_END; o++)
        {
          if(ImGui::Selectable(((o == mObjType ? "* " : "") + getObjNameLong((ObjType)o)).c_str()))
            { mOldObjType = mObjType; mObjType = (ObjType)o; break; }
        }
      ImGui::EndCombo();
    }
  
  if(chart)
    {
      Vec2f cursorPos = ImGui::GetCursorScreenPos();
      std::string overlay = mOldStartDate.toString() + " --> " + mOldEndDate.toString();
      ImGui::PlotLines(getObjName(mObjType).c_str(), mData.data(), mData.size(), 0, overlay.c_str(), 0.0f, 360.0f, Vec2f(950, 500)*scale);
      ImGui::GetWindowDrawList()->AddLine(Vec2f(475, 0)*scale + cursorPos, Vec2f(475, 500)*scale + cursorPos, ImColor(Vec4f(1.0f, 0.0f, 0.0f, 1.0f)), 2.0f);
    }
}

