#ifndef SETTINGS_FORM_HPP
#define SETTINGS_FORM_HPP

#include <vector>
#include <string>
#include "nlohmann/json_fwd.hpp" // json forward declarations
using json = nlohmann::json;

// forward declarations
class SettingBase;

class SettingForm
{
protected:
  std::vector<SettingBase*>    mSettings;
  float mLabelColW = 200; // width of column with setting name labels    
  float mInputColW = 150; // width of column with setting input widget(s)
    
public:
  SettingForm()  { }
  SettingForm(float labelColW, float inputColW) { setLabelColWidth(labelColW); setInputColWidth(inputColW); }
  ~SettingForm();

  json toJSON() const;
  bool fromJSON(const json &js);
    
  void add(SettingBase *setting);
  SettingBase* get(const std::string &name);
  void remove(const std::string &name);
  bool draw(float scale, bool busy=false, bool visible=true);
    
  void setLabelColWidth(float w);
  void setInputColWidth(float w);
  float labelColWidth() const { return mLabelColW; }
  float inputColWidth() const { return mInputColW; }
};

#endif // SETTINGS_FORM_HPP
