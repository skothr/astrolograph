#include "settingForm.hpp"

#include <algorithm>
#include "imgui.h"
#include "glfwKeys.hpp"
#include "nlohmann/json.hpp" // json forward declarations
using json = nlohmann::json;
#include "setting.hpp"



SettingForm::~SettingForm()
{
  for(auto s : mSettings)
    { if(s && s->getDelete()) { delete s; } }
  mSettings.clear();
}

json SettingForm::toJSON() const
{
  json js = json::object();
  for(auto s : mSettings)
    { js[s->getId()] = s->toJSON(); }
  return js;
}

bool SettingForm::fromJSON(const json &js)
{
  bool success = true;
  for(auto s : mSettings)
    {
      auto jss = js[s->getId()];
      if(!jss.is_null())
        { if(!s->fromJSON(jss)) { success = false; } }
      else
        { std::cout <<  "WARNING: SettingForm couldn't find setting id (" << s->getId() << ")\n"; success = false; }
    }
  return success;
}

void SettingForm::add(SettingBase *setting)
{
  if(setting)
    {
      setting->setLabelColWidth(mLabelColW); setting->setInputColWidth(mInputColW);
      mSettings.push_back(setting);
    }
}

SettingBase* SettingForm::get(const std::string &name)
{
  auto iter = std::find_if(mSettings.begin(), mSettings.end(), [&](SettingBase *s){ return (s->getName() == name); });
  return (iter != mSettings.end() ? *iter : nullptr);
}

void SettingForm::remove(const std::string &name)
{
  auto iter = std::find_if(mSettings.begin(), mSettings.end(), [&](SettingBase *s){ return (s->getName() == name); });
  if(iter != mSettings.end())
    {
      mSettings.erase(iter);
    }
}

void SettingForm::setLabelColWidth(float w)
{ mLabelColW = w; for(auto s : mSettings) { s->setLabelColWidth(w); } }
void SettingForm::setInputColWidth(float w)
{ mInputColW = w; for(auto s : mSettings) { s->setInputColWidth(w); } }

bool SettingForm::draw(float scale, bool busy, bool visible)
{
  for(int i = 0; i < mSettings.size(); i++)
    {
      bool changed = false;
      busy |= mSettings[i]->draw(scale, busy, changed, visible);
      if(changed && mSettings[i]->callback) { mSettings[i]->callback(); }
    }
  return busy;
}

