#include "chartNode.hpp"
using namespace astro;

#include "imgui.h"
#include "tools.hpp"
#include "settingForm.hpp"

ChartNode::ChartNode(Chart *chart)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Chart Node"), mChart(chart)
{
  if(!mChart) { mChart = new Chart(DateTime::now(), Location()); }
  for(auto &obj : mChart->objects())
    {
      if(obj->type < OBJ_COUNT) // planets/asateroids/etc
        { mChart->showObject((ObjType)obj->type, true); }
      else                      // angles
        { mChart->showObject((ObjType)obj->type, false); }
    }
  mChart->update();
  outputs()[CHARTNODE_OUTPUT_CHART]->set(mChart);
  
  for(auto hs : HOUSE_SYSTEM_NAMES)     { mHsNames.push_back(hs.second); if(hs.first == HOUSE_PLACIDUS) { mHouseSystem = mHsNames.size()-1; } }
  for(int i = 0; i < ZODIAC_COUNT; i++) { mZNames.push_back(getZodiacName((ZodiacType)i)); }

  SettingGroup *group = new SettingGroup("Options", "opt", { }, true, false);
    
  mSettings.push_back(new Setting<DateTime>("Date/Time",      "date",     &mChart->date()));
  mSettings.push_back(new Setting<Location>("Location",       "location", &mChart->location()));
  mSettings.push_back(new Setting<bool>    ("Options Open",   "optOpen",  &group->open()));
  mSettings.push_back(new ComboSetting     ("House System",   "hsys",     &mHouseSystem,      mHsNames, HOUSE_PLACIDUS));
  mSettings.push_back(new ComboSetting     ("Zodiac",         "zodiac",   &mZodiac,           mZNames, ZODIAC_TROPICAL));
  mSettings.push_back(new Setting<bool>    ("True Positions", "truePos",  &mTruePos,          false));

  group->add(mSettings[3]);
  group->add(mSettings[4]);
  group->add(mSettings[5]);
  
  mSettingForm = new SettingForm(150.0f, 135.0f);
  mSettingForm->add(group); //("True Positions", "truePos",  &mTruePos) }, true));
}

ChartNode::ChartNode(const DateTime &dt, const Location &loc)
  : ChartNode(new Chart(dt, loc)) { }

ChartNode::~ChartNode()
{
  delete mSettingForm;
  delete mChart;
}

bool ChartNode::onConnect(ConnectorBase *con)
{
  Direction dir     = con->direction();
  int       index   = con->conId();
  bool      success = false;
  if(dir == CONNECTOR_INPUT)
    {
      if(index == CHARTNODE_INPUT_DATE)
        {
          DateTime *dt = con->get<DateTime>();
          std::cout << "Date input connected!\n"
                    << mChart->date() << " --> " << (dt ? dt->toString() : "") << "\n";
          if(dt) { mChart->setDate(*dt); success = true; }
        }
      else if(index == CHARTNODE_INPUT_LOCATION)
        {
          Location *loc = con->get<Location>();
          std::cout << "Location input connected!\n"
                    << mChart->location() << " --> " << (loc ? loc->toString() : "") << "\n";
          if(loc) { mChart->setLocation(*loc); success = true; }
        }
    }
  else if(dir == CONNECTOR_OUTPUT)
    {
      if(index == CHARTNODE_OUTPUT_CHART)
        {
          std::cout << "Chart output connected!\n";
          success = true;
        }
    }
  
  mChart->update();
  return success;
}

void ChartNode::onUpdate()
{
  DateTime *dtIn  = inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>();
  Location *locIn = inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>();
  if(dtIn && *dtIn != mChart->date())
    {
      if(mChart->hasChanged()) { *dtIn = mChart->date(); }
      else                     { mChart->setDate(*dtIn); }
    }
  if(locIn && *locIn != mChart->location())
    {
      if(mChart->hasChanged()) { *locIn = mChart->location(); }
      else                     { mChart->setLocation(*locIn); }
    }
  
  HouseSystem hs = HOUSE_INVALID;
  for(auto h : HOUSE_SYSTEM_NAMES)
    { if(mHsNames[mHouseSystem] == h.second) { hs = h.first; } }
  mChart->setHouseSystem(hs);
  mChart->setZodiac((ZodiacType)mZodiac);
  mChart->setTruePos(mTruePos);

  mChanged |= mChart->hasChanged();
  mChart->update();
}

void ChartNode::onDraw()
{
  float scale = getScale();
  
  DateTime *dtIn = inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>();
  Location *locIn = inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>();
  
  DateTime dt   = mChart->date();
  Location loc  = mChart->location();
  double julDay = mChart->swe().getJulianDayET(dt, loc);

  ImGui::TextUnformatted(dt.toString().c_str());
  ImGui::Text("(jd_ET = %.6f)", julDay);
  ImGui::TextUnformatted(loc.toString().c_str());
  ImGui::Spacing();

  // chart options (TODO: keep track of changes instead of comparing whole JSON)
  json jsOld = mSettingForm->getJson();
  mSettingForm->draw(scale);
  json jsNew = mSettingForm->getJson();
  mChanged |= (jsOld != jsNew);
}
