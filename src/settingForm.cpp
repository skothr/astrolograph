#include "settingForm.hpp"
using namespace astro;

#include <algorithm>

#include "imgui.h"
#include "glfwKeys.hpp"

SettingBase* SettingForm::get(const std::string &name)
{
  auto iter = std::find_if(mSettings.begin(), mSettings.end(), [&](SettingBase *s){ return (s->getName() == name); });
  return (iter != mSettings.end() ? *iter : nullptr);
}

json SettingForm::getJson() const
{
  json js = json::object();
  for(auto s : mSettings)
    { js[s->getId()] = s->getJson(); }
  return js;
}

void SettingForm::setJson(const json &js)
{
  for(auto s : mSettings)
    {
      auto jss = js[s->getId()];
      if(!jss.is_null())
        { s->setJson(jss); }
      else
        { std::cout <<  "WARNING: SettingForm couldn't find setting id (" << s->getId() << ")\n"; }
    }
}

bool SettingForm::draw(float scale, bool busy)
{
  for(auto s : mSettings) { busy |= s->draw(scale, busy); }
  return busy;
}

