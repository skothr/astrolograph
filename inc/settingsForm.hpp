#ifndef SETTINGS_FORM_HPP
#define SETTINGS_FORM_HPP

#include "astro.hpp"

#include <map>
#include <vector>
#include <sstream>
#include <type_traits>
#include "imgui.h"
#include "glfwKeys.hpp"


namespace astro
{
  template<typename T> class Setting;
  class SettingsForm;

  
  class SettingBase
  {
  protected:
    SettingsForm *mParent = nullptr;
    std::string mName;
    std::string mId;
    virtual bool onDraw(float scale, bool busy=false) { return busy; }
    
  public:
    SettingBase(const std::string &name, const std::string &id) : mName(name), mId(id) { }
    virtual ~SettingBase() { }
    std::string getName() const  { return mName; }
    std::string getId() const    { return mId; }
    virtual bool isGroup() const { return false; }
    virtual void setParent(SettingsForm *parent) { mParent = parent; }
    
    virtual bool hasChanged() const { return false; }
    
    bool draw(float scale, bool busy=false);
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const { return params; }
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) { return params; }
  };
  
  template<typename T>
  class Setting : public SettingBase
  {
  private:
    bool mDelete = false; // whether to delete data on destruction
    bool mActive = false;
  protected:
    T *mData = nullptr;
    virtual bool onDraw(float scale, bool busy=false) override { return busy; }
  public:
    Setting(const std::string &name, const std::string &id)               : SettingBase(name, id), mData(new T()),    mDelete(true)  { }
    Setting(const std::string &name, const std::string &id, const T &val) : SettingBase(name, id), mData(new T(val)), mDelete(true)  { }
    Setting(const std::string &name, const std::string &id, T *ptr)       : SettingBase(name, id), mData(ptr),        mDelete(false) { }
    virtual ~Setting() { if(mData && mDelete) { delete mData; } }
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      params.emplace("active", (mActive ? "1" : "0"));
      if(mData)
        {
          std::stringstream ss;
          ss << (*mData);
          params.emplace("value", ss.str());
          //std::cout << "SAVED SETTING (" << mId << ") --> " << "value" << " --> " << (*mData) << " / '" << ss.str() << "'\n";
        }
      return params;
    };
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      auto iter = params.find("active");
      if(mData && iter != params.end())
        { mActive = (iter->second != "0"); }
      
      iter = params.find("value");
      if(mData && iter != params.end())
        {
          std::stringstream ss(iter->second);
          ss >> (*mData);
          //std::cout << "LOADED SETTING (" << mId << ") --> " << "value" << " --> " << (*mData) << " / '" << ss.str() << "'\n";
        }
      return params;
    };
  };

  // drawing by type
  template<> inline bool Setting<bool>::onDraw(float scale, bool busy)
  { // BOOL (checkbox)
    ImGui::Checkbox(("##"+mId).c_str(), mData);
    return busy;
  }
  template<> inline bool Setting<int>::onDraw(float scale, bool busy)
  { // INT
    ImGui::InputInt(("##"+mId).c_str(), mData, 1, 10);
    return busy;
  }
  template<> inline bool Setting<float>::onDraw(float scale, bool busy)
  { // FLOAT
    ImGui::InputFloat(("##"+mId).c_str(), mData, 1.0f, 10.0f);
    return busy;
  }
  template<> inline bool Setting<std::string>::onDraw(float scale, bool busy)
  { // STRING
    char data[1024] = {0};
    std::copy(mData->begin(), mData->end(), data);
    if(ImGui::InputText(("##"+mId).c_str(), data, 1024))
      {
        // mChanged |= (data != *mData));
        *mData = data;
      }
    return busy;
  }
  template<> inline bool Setting<Vec2f>::onDraw(float scale, bool busy)
  { // VEC2F
    ImGui::BeginGroup();
    {
      ImGui::TextUnformatted("X:");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(120*scale);
      ImGui::InputFloat(("##"+mId+"X").c_str(), &mData->x, 1.0f, 10.0f, "%.2f");
      ImGui::TextUnformatted("Y:");
      ImGui::SameLine();
      ImGui::SetNextItemWidth(120*scale);
      ImGui::InputFloat(("##"+mId+"Y").c_str(), &mData->y, 1.0f, 10.0f, "%.2f");
    }
    ImGui::EndGroup();
    return busy;
  }
  template<> inline bool Setting<Vec4f>::onDraw(float scale, bool busy)
  { // COLOR (VEC4F)
    ImGuiWindowFlags wFlags = (ImGuiWindowFlags_AlwaysAutoResize |
                               ImGuiWindowFlags_NoMove           |
                               ImGuiWindowFlags_NoTitleBar       |
                               ImGuiWindowFlags_NoResize );
    ImGuiStyle& style = ImGui::GetStyle();
    static Vec4f lastColor; // save previous color in case user cancels
    std::string popupName = mId + "pop";
    std::string pickerName = mId + "pick";
    // choose graph background color
    if(ImGui::ColorButton(("##"+mId).c_str(), *mData,
                          ImGuiColorEditFlags_NoOptions|ImGuiColorEditFlags_DisplayRGB|ImGuiColorEditFlags_NoAlpha, ImVec2(20, 20)) && !busy)
      {
        // style.Colors[ImGuiCol_ModalWindowDimBg] = Vec4f(0,0,0,0);
        lastColor = *mData;
        ImGui::OpenPopup(popupName.c_str());
      }
    if(ImGui::BeginPopup(popupName.c_str(), wFlags))
      {
        busy = true;
        if(!ImGui::ColorPicker4(pickerName.c_str(), mData->data.data(), ImGuiColorEditFlags_NoSidePreview))
          {
            if((!ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow|ImGuiHoveredFlags_ChildWindows) && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) ||
               ImGui::IsKeyPressed(GLFW_KEY_ESCAPE))
              {
                *mData = lastColor;
                ImGui::CloseCurrentPopup();
              }
          }
        if(ImGui::Button("Select") || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) // selects color
          {
            //mChanged = (*mData != lastColor);
            ImGui::CloseCurrentPopup();
          }
        ImGui::SameLine();
        if(ImGui::Button("Cancel"))
          {
            *mData = lastColor;
            ImGui::CloseCurrentPopup();
          }
        ImGui::EndPopup();
      }
    return busy;
  }
  
  class ComboSetting : public Setting<int>
  {
  protected:
    std::vector<std::string> mChoices;
    
    virtual bool onDraw(float scale, bool busy=false) override;
  public:
    ComboSetting(const std::string &name, const std::string &id)
      : Setting(name, id) { }
    ComboSetting(const std::string &name, const std::string &id, int *selection, const std::vector<std::string> &choices)
      : Setting(name, id, selection), mChoices(choices) { }
    ~ComboSetting() { }
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      Setting<int>::getSaveParams(params);
      
      std::string choiceStr = "";
      for(auto &c : mChoices) { choiceStr += c + ","; }
      params.emplace("choices", choiceStr);
      //std::cout << "SAVED SETTING (" << mId << ") --> " << "choices" << " --> " << choiceStr << "\n";
      return params;
    };
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      Setting<int>::setSaveParams(params);

      mChoices.clear();
      auto iter = params.find("choices");
      if(iter != params.end())
        {
          std::stringstream ss(iter->second);
          std::string c;
          while(std::getline(ss, c, ','))
            { mChoices.push_back(c); }
        }
      return params;
    };
  };
  
  inline bool ComboSetting::onDraw(float scale, bool busy)
  { // VECTOR<STRING> (COMBOBOX)
    if(ImGui::BeginCombo(("##"+mId).c_str(), mChoices[*mData].c_str()))
      {
        busy = true;
        ImGui::SetWindowFontScale(scale);
        for(int i = 0; i < mChoices.size(); i++)
          {
            std::string &s = mChoices[i];
            if(ImGui::Selectable(((i == *mData ? "* " : "") + mChoices[i]).c_str()))
              {
                //mChanged |= (*mData != i);
                *mData = i;
              }
          }
        ImGui::EndCombo();
      }
    return busy;
  }  


  class SettingGroup : public SettingBase
  {
  private:
    bool mActive   = false;
  protected:
    bool mCollapse = false;
    std::vector<SettingBase*> mContents;
    virtual bool onDraw(float scale, bool busy=false) override;
  public:
    SettingGroup(const std::string &name, const std::string &id)
      : SettingBase(name, id) { }
    SettingGroup(const std::string &name, const std::string &id, const std::vector<SettingBase*> &contents, bool collapse=false)
      : SettingBase(name, id), mContents(contents), mCollapse(collapse) { }
    ~SettingGroup() { }
    virtual bool isGroup() const override { return true; }
    virtual void setParent(SettingsForm *parent) override { mParent = parent; for(auto s : mContents) { s->setParent(parent); } }
    
    std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const
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
      // params.emplace(mId, totalVal);
      // params.emplace(mId, totalVal);
      return params;
    };
    std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params)
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
                    {
                      //std::cout << "WARNING: Group failed to load setting (" << id << ") --> '" << val << "'\n";
                    }
                }
              s->setSaveParams(sParams);
            }
        }
      return params;
    };
  };

  inline bool SettingGroup::onDraw(float scale, bool busy)
  { // SETTING GROUP
    ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
    if(mCollapse)
      {
        ImGui::SetNextTreeNodeOpen(mActive);
        // ImGui::SetNextItemWidth(col1Width+col2Width);
        if(ImGui::CollapsingHeader(mName.c_str(), nullptr, flags))
          {
            mActive = true;
            ImGui::Indent();
            ImGui::BeginGroup();
            for(auto s : mContents) { busy |= s->draw(scale); }
            ImGui::EndGroup();
            ImGui::Unindent();
          }
        else { mActive = false; }
      }
    else
      {
        mActive = false;
        ImGui::TextUnformatted(mName.c_str());
        ImGui::Separator();
        ImGui::Indent();
        ImGui::BeginGroup();
        for(auto s : mContents) { busy |= s->draw(scale); }
        ImGui::EndGroup();
        ImGui::Unindent();
      }
    return busy;
  }


  class SettingsForm
  {
  protected:
    std::vector<SettingBase*> mSettings;
    float mLabelColW = 200;
    float mInputColW = 150;
    
  public:
    SettingsForm();
    ~SettingsForm();
    void add(SettingBase *setting)            { setting->setParent(this); mSettings.push_back(setting); }
    SettingBase* get(const std::string &name)
    {
      for(auto s : mSettings)
        { if(s->getName() == name) { return s; } }
      return nullptr;
    }
    bool draw(float scale, bool busy=false);

        
        
    std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const
    {
      for(auto s : mSettings)
        {
          std::map<std::string, std::string> sParams;
          s->getSaveParams(sParams);
          std::string val = "";
          for(auto v : sParams)
            {
              //std::cout << "SETTINGS FORM SAVING SETTING --> " << v.first << ":" << v.second << "|\n";
              val += v.first + ":" + v.second + "|";
            }
          params.emplace(s->getId(), val);
        }
      return params;
    };
    std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params)
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
                    {
                      //std::cout << "SETTINGS FORM SAVING SETTING (" << id << ") --> " << val << "\n";
                      sParams.emplace(id, val);
                    }
                  //else
                  //{ std::cout <<  "WARNING: SettingsForm failed to parse setting (" << id << ") --> " << val << "\n"; }
                }
              s->setSaveParams(sParams);
            }
          // else
          //   { std::cout <<  "WARNING: SettingsForm couldn't find setting id (" << s->getId() << ")\n"; }
        }
      return params;
    };
    
    void setLabelColWidth(float w) { mLabelColW = w; }
    void setInputColWidth(float w) { mInputColW = w; }
    float labelColWidth() const { return mLabelColW; }
    float inputColWidth() const { return mInputColW; }
  };
}

#endif // SETTINGS_FORM_HPP
