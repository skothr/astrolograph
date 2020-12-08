#ifndef SETTINGS_FORM_HPP
#define SETTINGS_FORM_HPP

#include <map>
#include <vector>
#include <sstream>

#include "astro.hpp"
#include "setting.hpp"

namespace astro
{  
  class SettingForm
  {
  protected:
    std::vector<SettingBase*> mSettings;
    float mLabelColW = 200; // width of column with setting name labels    
    float mInputColW = 150; // width of column with setting input widget(s)
    
  public:
    SettingForm()  { }
    SettingForm(float labelColW, float inputColW) { setLabelColWidth(labelColW); setInputColWidth(inputColW); }
    ~SettingForm() { for(auto s : mSettings) { delete s; } }
    
    void add(SettingBase *setting)
    {
      if(setting)
        {
          setting->setLabelColWidth(mLabelColW); setting->setInputColWidth(mInputColW);
          mSettings.push_back(setting);
        }
    }
    SettingBase* get(const std::string &name);
    bool draw(float scale, bool busy=false);
        
    json getJson() const;
    void setJson(const json &js);
    
    void setLabelColWidth(float w) { mLabelColW = w; for(auto s : mSettings) { s->setLabelColWidth(w); } }
    void setInputColWidth(float w) { mInputColW = w; for(auto s : mSettings) { s->setInputColWidth(w); } }
    float labelColWidth() const { return mLabelColW; }
    float inputColWidth() const { return mInputColW; }
  };
}

#endif // SETTINGS_FORM_HPP
