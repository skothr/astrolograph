#include "chartNode.hpp"
using namespace astro;

#include "imgui.h"
#include "tools.hpp"


ChartNode::ChartNode(Chart *chart)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Chart Node"), mChart(chart)
{
  if(!mChart) { mChart = new Chart(DateTime::now(), Location()); }
  mChart->update();
  
  outputs()[CHARTNODE_OUTPUT_CHART]->set(mChart);
  
  for(auto &obj : mChart->objects())
    {
      if(obj->type < OBJ_COUNT) // planets/asateroids/etc
        { mChart->showObject((ObjType)obj->type, true); }
      else                      // angles
        { mChart->showObject((ObjType)obj->type, false); }
    }
}

ChartNode::ChartNode(const DateTime &dt, const Location &loc)
  : ChartNode(new Chart(dt, loc))
{ }

ChartNode::~ChartNode()
{
  delete mChart;
}


bool ChartNode::onDraw()
{
  ImGui::BeginGroup();
  {
    DateTime *dtIn = inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>();
    Location *locIn = inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>();
    
    if(dtIn && *dtIn != mChart->date())
      {
        if(mChart->changed())
          { *dtIn = mChart->date(); }
        else
          { mChart->setDate(*dtIn); }
      }
    if(locIn && *locIn != mChart->location())
      {
        if(mChart->changed())
          { *locIn = mChart->location(); }
        else
          { mChart->setLocation(*locIn); }
      }
    //mChart->update(); // UPDATE (should be only one -- TODO: improve?)
    
    DateTime dt   = mChart->date();
    Location loc  = mChart->location();
    double julDay = mChart->swe().getJulianDayET(dt, loc, false);

    ImGui::Text(dt.toString().c_str());
    ImGui::Text("(jd_ET = %.6f)", julDay);
    ImGui::Text(loc.toString().c_str());
    ImGui::Spacing();
  }
  ImGui::EndGroup();
  
  mChart->update();
  return true;
}

