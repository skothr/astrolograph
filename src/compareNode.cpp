#include "compareNode.hpp"
using namespace astro;

#include "imgui.h"
#include "tools.hpp"

//// node connector indices ////
// inputs
#define COMPARENODE_INPUT_CHART_INNER 0
#define COMPARENODE_INPUT_CHART_OUTER 1
////////////////////////////////

static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
{ return {new Connector<Chart>("Inner Chart"), new Connector<Chart>("Outer Chart")}; }
static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
{ return {}; }

CompareNode::CompareNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Compare Node"), mCompare(new ChartCompare())
{
  
}

CompareNode::~CompareNode()
{ }


bool CompareNode::onDraw()
{
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
  
  ImGui::BeginGroup();
  {
    // DateTime *dtIn = inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>();
    // if(dtIn && *dtIn != mChart->date())
    //   {
    //     mChart->setDate(*dtIn);
    //     changed = true;
    //   }
    // Location *locIn = inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>();
    // if(locIn && *locIn != mChart->location())
    //   {
    //     mChart->setLocation(*locIn);
    //     changed = true;
    //   }

    // if(changed) { mChart->update(); }
    // changed = false;

    
    // double julDay = mChart->getSwe().getJulianDay();
    // astro::DateTime &date = mChart->date();
    // astro::Location &loc = mChart->location();
    // // TODO: fix progressed compare input (CompareNode)
    // astro::DateTime *compareDate = inputs()[CHARTNODE_INPUT_CURRENTDATE]->get<DateTime>();
    // //astro::DateTime *compareDate = nullptr;
    
    // ImGui::Text(date.toString().c_str());

    // ImGui::SameLine(); ImGui::Text("(JD =");
    // ImGui::SameLine(); ImGui::Text(to_string(julDay).c_str()); ImGui::SameLine(); ImGui::Text(")");
        
    // ImGui::Text(loc.toString().c_str());
    // ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x, ImGui::GetCursorPos().y + 10.0f));

    ImGui::Text("Chart Size:  "); // size of chart area
    ImGui::SameLine(); ImGui::SliderFloat("##chartWidth", &mChartWidth, CHART_SIZE_MIN, CHART_SIZE_MAX, "%.0f"); // Minimal displayed value is 5%

    // ImGui::SameLine();
    // ImGui::Checkbox("Show Progressed", &mProgressed);
    // ImGui::SameLine();
    // ImGui::Text(("(" + (compareDate ? progressed(date, *compareDate) : progressed(date, DateTime::now())).toString() + ")").c_str());
    
    // mChart->setProgressed(mProgressed);
    // mChart->setProgressedDate(compareDate);
    // mChart->update();
    
    // draw chart view
    mView.draw(mCompare, mChartWidth);
    // if(oChart)
    //   { mView.draw(oChart, mChartWidth); }
    // if(iChart)
    //   { mView.draw(iChart, mChartWidth); }
  }
  ImGui::EndGroup();

  return true;
}



