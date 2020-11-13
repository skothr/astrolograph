#include "chartNode.hpp"
using namespace astro;

#include "imgui.h"
#include "tools.hpp"


ChartNode::ChartNode(Chart *chart)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Chart Node"), mChart(chart)
{
  // setMinSize(Vec2f(512, 512));
  
  if(!mChart) { mChart = new Chart(DateTime::now(), Location()); }
  outputs()[CHARTNODE_OUTPUT_CHART]->set(mChart);
  
  for(auto &obj : mChart->objects())
    {
      if(obj->type < OBJ_COUNT) // planets/asateroids/etc
        { mChart->showObject((ObjType)obj->type, true); }
      else                      // angles
        { mChart->showObject((ObjType)obj->type, false); }
    }
  mChart->update();
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
  //ImGui::BeginGroup();
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
    
    DateTime dt   = mChart->date();
    Location loc  = mChart->location();
    double julDay = mChart->swe().getJulianDayET(dt, loc, false);

    ImGui::Text(dt.toString().c_str());
    ImGui::Text("(jd_ET = %.6f)", julDay);
    ImGui::Text(loc.toString().c_str());
    ImGui::Spacing();

    // ephemeris options
    ImGuiTreeNodeFlags flags = 0;//ImGuiTreeNodeFlags_SpanAvailWidth;
    ImGui::SetNextTreeNodeOpen(mOptionsOpen);
    if(ImGui::CollapsingHeader("Options", nullptr, flags))
      {
        mOptionsOpen = true;
        ImGui::TextUnformatted("House System: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        if(ImGui::BeginCombo("##houseSystem", getHouseSystemName(mChart->getHouseSystem()).c_str()))
          {
            // std::vector<HouseSystem> systems;
            // for(auto hs : HOUSE_SYSTEM_NAMES) { systems.push_back(hs.first); }
            // std::sort(systems.begin(), systems.end(),
            //           [](const HouseSystem &hs1, const HouseSystem &hs2) { return getHouseSystemName(hs1) < getHouseSystemName(hs2); });
            //for(auto hs : systems)
              // {
              //   if(ImGui::Selectable(getHouseSystemName(hs).c_str()))
              //     { mChart->setHouseSystem(hs); }
              // }
            for(const auto &hs : HOUSE_SYSTEM_NAMES)
              {
                if(ImGui::Selectable(hs.second.c_str()))
                  { mChart->setHouseSystem(hs.first); }
              }
            ImGui::EndCombo();
          }
      }
    else { mOptionsOpen = false; }
  }
  //ImGui::EndGroup();
  mChart->update(); // UPDATE (should be only one -- TODO: improve?)
  return true;
}

