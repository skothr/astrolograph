#include "settingsForm.hpp"
using namespace astro;



    
bool SettingBase::draw(float scale, bool busy)
{
  if(!isGroup())
    {
      ImGui::TextUnformatted(mName.c_str());
      ImGui::SameLine(mParent->labelColWidth()*scale);
      ImGui::SetNextItemWidth(mParent->inputColWidth()*scale);
    }
  return onDraw(scale, busy);
}



SettingsForm::SettingsForm()
{ }

SettingsForm::~SettingsForm()
{
  for(auto s : mSettings) { delete s; }
}

bool SettingsForm::draw(float scale, bool busy)
{
  float col1W = mLabelColW*scale;
  float col2W = mLabelColW*scale;
  for(auto s : mSettings)
    { busy |= s->draw(scale, busy); }
  return busy;
}
