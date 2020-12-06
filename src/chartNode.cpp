#include "chartNode.hpp"
using namespace astro;

#include "imgui.h"
#include "tools.hpp"
#include "settingForm.hpp"

ChartNode::ChartNode(Chart *chart)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Chart Node"), mChart(chart)
{
  for(auto hs : HOUSE_SYSTEM_NAMES)     { mHsNames.push_back(hs.second); if(hs.first == HOUSE_PLACIDUS) { mHouseSystem = mHsNames.size()-1; } }
  for(int i = 0; i < ZODIAC_COUNT; i++) { mZNames.push_back(getZodiacName((ZodiacType)i)); }
  
  mSettings = new SettingForm(150.0f, 135.0f);
  mSettings->add(new SettingGroup("Options", "opt",
                                  {   new ComboSetting ("House System",   "hsys",     &mHouseSystem, mHsNames),
                                      new ComboSetting ("Zodiac",         "zodiac",   &mZodiac,      mZNames),
                                      new Setting<bool>("True Positions", "truePos",  &mTruePos) }, true));
  
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
  delete mSettings;
  delete mChart;
}

    
void ChartNode::getSaveParams(std::map<std::string, std::string> &params) const
{
  mSettings->getSaveParams(params);
  if(!inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>())
    { params.emplace("date", mChart->date().toSaveString()); }
  if(!inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>())
    { params.emplace("location", mChart->location().toSaveString()); }
};
    
void ChartNode::setSaveParams(std::map<std::string, std::string> &params)
{
  mSettings->setSaveParams(params);
  auto iter = params.find("date");   if(iter != params.end()) { mChart->setDate(DateTime(iter->second)); }
  iter = params.find("location");    if(iter != params.end()) { mChart->setLocation(Location(iter->second)); }

  HouseSystem hs = HOUSE_INVALID;
  for(auto h : HOUSE_SYSTEM_NAMES)
    { if(mHsNames[mHouseSystem] == h.second) { hs = h.first; } }
  mChart->setHouseSystem(hs);
  mChart->setZodiac((ZodiacType)mZodiac);
  mChart->setTruePos(mTruePos);
  mChart->update();
};




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

  // chart options
  std::map<std::string, std::string> oldData;
  std::map<std::string, std::string> newData;
  mSettings->getSaveParams(oldData);
  mSettings->draw(scale);
  mSettings->getSaveParams(newData);
  mChanged |= (oldData != newData);
}
