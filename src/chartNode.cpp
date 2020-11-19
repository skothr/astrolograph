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

void ChartNode::onUpdate()
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
  mChart->update();
}

bool ChartNode::onDraw()
{
  float scale = getScale();
  
  DateTime *dtIn = inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>();
  Location *locIn = inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>();
  
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
      ImGui::SetNextItemWidth(150*scale);
      HouseSystem hsCurrent = mChart->getHouseSystem();
      if(ImGui::BeginCombo("##houseSystem", getHouseSystemName(hsCurrent).c_str()))
        {
          ImGui::SetWindowFontScale(scale);
          for(const auto &hs : HOUSE_SYSTEM_NAMES)
            {
              if(ImGui::Selectable(((hs.first == hsCurrent ? "* " : "") + hs.second).c_str()))
                { mChart->setHouseSystem(hs.first); }
            }
          ImGui::EndCombo();
        }

      // sidereal/tropical system
      ImGui::TextUnformatted("Zodiac    ");
      int zType = (int)mChart->getZodiac();
      ImGui::SameLine();
      ImGui::SetNextItemWidth(150*scale);
      if(ImGui::BeginCombo("##zopdiacType", getZodiacName((ZodiacType)zType).c_str()))
        {
          ImGui::SetWindowFontScale(scale);
          for(int z = 0; z < ZODIAC_COUNT; z++)
            {
              if(ImGui::Selectable(((z == zType ? "* " : "") + getZodiacName((ZodiacType)z)).c_str()))
                { mChart->setZodiac((ZodiacType)z); }
            }
          ImGui::EndCombo();
        }
      // ImGui::BeginGroup();
      // ImGui::TextUnformatted("Tropical"); ImGui::SameLine(); ImGui::RadioButton("##tropRB", &zType, (int)ZODIAC_TROPICAL);
      // ImGui::TextUnformatted("Sidereal"); ImGui::SameLine(); ImGui::RadioButton("##realRB", &zType, (int)ZODIAC_SIDEREAL);
      // ImGui::TextUnformatted("Draconic"); ImGui::SameLine(); ImGui::RadioButton("##dracRB", &zType, (int)ZODIAC_DRACONIC);
      // ImGui::EndGroup();
      // mChart->setZodiac((ZodiacType)zType);
        
    }
  else if(isBodyVisible()) { mOptionsOpen = false; }
  return true;
}

