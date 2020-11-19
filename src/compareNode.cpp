#include "compareNode.hpp"
using namespace astro;

#include "imgui.h"
#include "tools.hpp"

CompareNode::CompareNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Compare Node"), mCompare(new ChartCompare())
{
  // setMinSize(Vec2f(512, 512));
}

CompareNode::~CompareNode()
{ }


bool CompareNode::onDraw()
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
  
  //ImGui::BeginGroup();
  {
    ImGui::Text("Chart Size:  "); // size of chart area
    ImGui::SameLine();
    ImGui::SetNextItemWidth(360*scale);
    ImGui::SliderFloat("##chartWidth", &mChartWidth, CHART_SIZE_MIN, CHART_SIZE_MAX, "%.0f"); // Minimal displayed value is 5%

    // draw chart view
    mView.draw(mCompare, mChartWidth*scale);
  }
  //ImGui::EndGroup();

  return true;
}



