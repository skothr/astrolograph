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

void SettingForm::getSaveParams(std::map<std::string, std::string> &params) const
{
  for(auto s : mSettings)
    {
      std::map<std::string, std::string> sParams;
      s->getSaveParams(sParams);
      std::string val = "";
      for(auto v : sParams) { val += v.first + ":" + v.second + "|"; }
      params.emplace(s->getId(), val);
    }
}

void SettingForm::setSaveParams(std::map<std::string, std::string> &params)
{
  for(auto s : mSettings)
    {
      auto iter = params.find(s->getId());
      if(iter != params.end())
        {
          std::map<std::string, std::string> sParams;
          std::stringstream ss(iter->second);
          std::string line;
          while(std::getline(ss, line, '|'))
            {
              std::stringstream ss2(line);
              std::string id, val;
              if(std::getline(ss2, id, ':') && std::getline(ss2, val, '|'))
                { sParams.emplace(id, val); }
              else
                { std::cout <<  "WARNING: SettingForm failed to parse setting (" << id << ") --> " << val << "\n"; }
            }
          s->setSaveParams(sParams);
        }
      else
        { std::cout <<  "WARNING: SettingForm couldn't find setting id (" << s->getId() << ")\n"; }
    }
}

bool SettingForm::draw(float scale, bool busy)
{
  for(auto s : mSettings) { busy |= s->draw(scale, busy); }
  return busy;
}

