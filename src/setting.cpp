#include "setting.hpp"
using namspace astro;

// #include <nlohmann/json.hpp>
// using namespace nlohmann


//// SETTING GROUP SAVE/LOAD ////
void SettingGroup::getSaveParams(std::map<std::string, std::string> &params) const
{
  params.emplace("active", mActive ? "1" : "0");
  std::string totalVal = "";
  for(auto s : mContents)
    {
      std::map<std::string, std::string> sParams;
      s->getSaveParams(sParams);
      std::string val = "";
      for(auto v : sParams)
        { val += v.first + ";" + v.second + "/"; }
      params.emplace(s->getId(), val);
    }
}

void SettingGroup::setSaveParams(std::map<std::string, std::string> &params)
{
  auto iter = params.find("active");
  if(iter != params.end()) { mActive = (iter->second != "0"); }
      
  for(auto s : mContents)
    {
      iter = params.find(s->getId());
      if(iter != params.end())
        {
          std::map<std::string, std::string> sParams;
          std::stringstream ss(iter->second);
          std::string line;
          while(std::getline(ss, line, '/'))
            {
              std::stringstream ss2(line);
              std::string id, val;
              if(std::getline(ss2, id, ';') && std::getline(ss2, val, '/'))
                { sParams.emplace(id, val); }
              else
                { std::cout << "WARNING: Group failed to load setting (" << id << ") --> '" << val << "'\n"; }
            }
          s->setSaveParams(sParams);
        }
    }
}
