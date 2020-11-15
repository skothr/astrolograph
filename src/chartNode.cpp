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
    
    // bool chartReset = outputs()[CHARTNODE_OUTPUT_CHART]->needsReset();

    if(dtIn && *dtIn != mChart->date())
      {
        if(mChart->changed())// || chartReset)
          {
            *dtIn = mChart->date();
            //outputs()[CHARTNODE_OUTPUT_CHART]->setReset(false);
          }
        else
          { mChart->setDate(*dtIn); }
      }
    if(locIn && *locIn != mChart->location())
      {
        if(mChart->changed())// || chartReset)
          {
            *locIn = mChart->location();
            //outputs()[CHARTNODE_OUTPUT_CHART]->setReset(false);
          }
        else
          { mChart->setLocation(*locIn); }
      }
    
    
    DateTime dt   = mChart->date();
    Location loc  = mChart->location();
    double julDay = mChart->swe().getJulianDayET(dt, loc, false);

    ImGui::TextUnformatted(dt.toString().c_str());
    ImGui::Text("(jd_ET = %.6f)", julDay);
    ImGui::TextUnformatted(loc.toString().c_str());
    ImGui::Spacing();

    // ephemeris options
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
    ImGui::SetNextTreeNodeOpen(mOptionsOpen);
    if(ImGui::CollapsingHeader("Options", nullptr, flags))
      {
        mOptionsOpen = true;
        ImGui::TextUnformatted("Houses    ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        HouseSystem hsCurrent = mChart->getHouseSystem();
        if(ImGui::BeginCombo("##houseSystem", getHouseSystemName(hsCurrent).c_str()))
          {
            for(const auto &hs : HOUSE_SYSTEM_NAMES)
              {
                if(ImGui::Selectable(((hs.first == hsCurrent ? "* " : "") + hs.second).c_str()))
                  { mChart->setHouseSystem(hs.first); }
              }
            ImGui::EndCombo();
          }

        // sidereal/tropical system
        ImGui::TextUnformatted("Zodiac    ");
        int calcType = (mChart->getSidereal() ? 1 : 0);
        ImGui::SameLine(); ImGui::TextUnformatted("Tropical"); ImGui::SameLine(); ImGui::RadioButton("##tropRB", &calcType, 0);
        ImGui::SameLine(); ImGui::TextUnformatted("Sidereal"); ImGui::SameLine(); ImGui::RadioButton("##realRB", &calcType, 1);
        
        mChart->setSidereal(calcType == 1);
        // draconic chart
        bool draconic = mChart->getDraconic();
        ImGui::TextUnformatted("Draconic  "); ImGui::SameLine(); if(ImGui::Checkbox("##drCheck", &draconic)) { mChart->setDraconic(draconic); }
      }
    else { mOptionsOpen = false; }
  }
  //ImGui::EndGroup();
  mChart->update(); // UPDATE (should be only one -- TODO: improve?)
  return true;
}

