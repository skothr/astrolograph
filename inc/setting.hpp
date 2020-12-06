#ifndef SETTING_HPP
#define SETTING_HPP

#include <map>
#include <vector>
#include <sstream>
#include "vector.hpp"

#include <iostream>
#include "imgui.h"
#include "glfwKeys.hpp"

namespace astro
{
  //// SETTING BASE CLASS ////
  class SettingBase
  {
  protected:
    std::string mName;
    std::string mId;
    float mLabelColW = 200; // width of column with setting name labels    
    float mInputColW = 150; // width of column with setting input widget(s)
    virtual bool onDraw(float scale, bool busy=false) { return busy; }
    
  public:
    SettingBase(const std::string &name, const std::string &id) : mName(name), mId(id) { }
    virtual ~SettingBase() { }

    virtual void setLabelColWidth(float width) { mLabelColW = width; }
    virtual void setInputColWidth(float width) { mInputColW = width; }
    std::string getName() const { return mName; }
    std::string getId() const   { return mId; }
    
    virtual bool isGroup() const    { return false; }
    virtual bool hasChanged() const { return false; }
    virtual void getSaveParams(std::map<std::string, std::string> &params) const { }
    virtual void setSaveParams(std::map<std::string, std::string> &params)       { }
    
    bool draw(float scale, bool busy=false)
    {
      if(!isGroup())
        {
          ImGui::TextUnformatted(mName.c_str());
          ImGui::SameLine(mLabelColW*scale);
          ImGui::SetNextItemWidth(mInputColW*scale);
        }
      return onDraw(scale, busy);
    }
  };

  //// SETTING TEMPLATE CLASS ////
  template<typename T>
  class Setting : public SettingBase
  {
  private:
    bool mDelete = false; // whether to delete data on destruction
    bool mActive = false;
  protected:
    T *mData   = nullptr;
    T mDefault;
    virtual bool onDraw(float scale, bool busy=false) { return busy; }
  public:
    typedef Setting<T> type;
    // construction
    Setting(const std::string &name, const std::string &id, T *ptr)
      : SettingBase(name, id), mData(ptr), mDelete(false)
    { mDefault = *mData; }
    Setting(const std::string &name, const std::string &id)
      : Setting(name, id, new T())
    { mDelete = true; }
    Setting(const std::string &name, const std::string &id, const T &val)
      : Setting(name, id, new T(val))
    { mDelete = true; }
    // with default value
    Setting(const std::string &name, const std::string &id, T *ptr, const T &defaultVal)
      : SettingBase(name, id), mData(ptr), mDelete(false)
    { mDefault = defaultVal; }
    Setting(const std::string &name, const std::string &id, const T &val, const T &defaultVal)
      : Setting(name, id, new T(val))
    {
      mDelete = true;
      mDefault = defaultVal;
    }

    
    ~Setting() { if(mData && mDelete) { delete mData; } }
    
    virtual void getSaveParams(std::map<std::string, std::string> &params) const override;
    virtual void setSaveParams(std::map<std::string, std::string> &params) override;
  };

  //// COMBOBOX SETTING ////
  class ComboSetting : public Setting<int>
  {
  protected:
    std::vector<std::string> mChoices;
    virtual bool onDraw(float scale, bool busy=false) override;

  public:
    ComboSetting(const std::string &name, const std::string &id)
      : Setting<int>(name, id) { }
    ComboSetting(const std::string &name, const std::string &id, int *selection, const std::vector<std::string> &choices)
      : Setting<int>(name, id, selection), mChoices(choices) { }
    ~ComboSetting() { }
    
    virtual void getSaveParams(std::map<std::string, std::string> &params) const override;
    virtual void setSaveParams(std::map<std::string, std::string> &params) override;
  };

  //// SETTING GROUP ////
  class SettingGroup : public SettingBase
  {
  protected:
    bool mCollapse = false;
    bool mActive   = false;
    std::vector<SettingBase*> mContents;
    virtual bool onDraw(float scale, bool busy=false) override;
    
  public:
    SettingGroup(const std::string &name, const std::string &id)
      : SettingBase(name, id) { }
    SettingGroup(const std::string &name, const std::string &id, const std::vector<SettingBase*> &contents, bool collapse=false)
      : SettingBase(name, id), mContents(contents), mCollapse(collapse) { }
    ~SettingGroup() { }
    virtual bool isGroup() const override { return true; }

    // pass to content settings
    virtual void setLabelColWidth(float w) override { SettingBase::setLabelColWidth(w); for(auto s : mContents) { s->setLabelColWidth(w); } }
    virtual void setInputColWidth(float w) override { SettingBase::setInputColWidth(w); for(auto s : mContents) { s->setInputColWidth(w); } }
    
    void getSaveParams(std::map<std::string, std::string> &params) const;
    void setSaveParams(std::map<std::string, std::string> &params);
  };

  //// SETTING SAVE/LOAD ////
  template<typename T>
  inline void Setting<T>::getSaveParams(std::map<std::string, std::string> &params) const
  {
    params.emplace("active", (mActive ? "1" : "0"));
    if(mData)
      {
        std::stringstream ss;
        ss << (*mData);
        params.emplace("value", ss.str());
      }
  }
  template<typename T>
  inline void Setting<T>::setSaveParams(std::map<std::string, std::string> &params)
  {
    auto iter = params.find("active");
    if(mData && iter != params.end())
      { mActive = (iter->second != "0"); }
      
    iter = params.find("value");
    if(mData && iter != params.end())
      {
        std::stringstream ss(iter->second);
        ss >> (*mData);
      }
  }

  //// COMBO SETTING SAVE/LOAD ////
  inline void ComboSetting::getSaveParams(std::map<std::string, std::string> &params) const
  {
    Setting<int>::getSaveParams(params);
      
    std::string choiceStr = "";
    for(auto &c : mChoices) { choiceStr += c + ","; }
    params.emplace("choices", choiceStr);
  }
  inline void ComboSetting::setSaveParams(std::map<std::string, std::string> &params)
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
  }

  //////////////////////////////////////////
  //// SETTING DRAW OVERLOADS (BY TYPE) ////
  //////////////////////////////////////////
  template<> inline bool Setting<bool>::onDraw(float scale, bool busy)
  { //// BOOL (checkbox)
    ImGui::Checkbox(("##"+mId).c_str(), mData);
    return busy;
  }
  template<> inline bool Setting<int>::onDraw(float scale, bool busy)
  { //// INT
    ImGui::InputInt(("##"+mId).c_str(), mData, 1, 10);
    return busy;
  }
  template<> inline bool Setting<float>::onDraw(float scale, bool busy)
  { //// FLOAT
    ImGui::InputFloat(("##"+mId).c_str(), mData, 1.0f, 10.0f);
    return busy;
  }
  template<> inline bool Setting<std::string>::onDraw(float scale, bool busy)
  { //// STRING
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
  { //// VEC2F
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
  { //// COLOR (VEC4F)
    ImGuiStyle& style = ImGui::GetStyle();
    static Vec4f       lastColor; // save previous color in case user cancels
    static std::string editId = "";
    std::string buttonName = "##" + mId + "btn";
    std::string popupName  = "##" + mId + "pop";
    std::string pickerName = "##" + mId + "pick";
    // choose graph background color
    ImGuiColorEditFlags cFlags = (ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoAlpha);
    if(ImGui::ColorButton(buttonName.c_str(), *mData, cFlags, ImVec2(20, 20)))// && !busy)
      {
        lastColor = *mData;
        ImGui::OpenPopup(popupName.c_str());
      }
    ImGuiWindowFlags wFlags = (ImGuiWindowFlags_AlwaysAutoResize |
                               ImGuiWindowFlags_NoMove           |
                               ImGuiWindowFlags_NoTitleBar       |
                               ImGuiWindowFlags_NoResize );
    if(ImGui::BeginPopup(popupName.c_str(), wFlags))
      {
        busy = true; // busy picking color;
        ImGui::ColorPicker4(pickerName.c_str(), mData->data.data(), cFlags, lastColor.data.data());
        
        if(ImGui::Button("Select") || ImGui::IsKeyPressed(GLFW_KEY_ENTER)) // selects color
          { ImGui::CloseCurrentPopup(); }
        ImGui::SameLine();
        if(ImGui::Button("Cancel")) // cancels current selection
          {
            *mData = lastColor;
            ImGui::CloseCurrentPopup();
          }
        ImGui::SameLine();
        if(ImGui::Button("Reset")) // resets to default value
          { *mData = mDefault; }
        ImGui::EndPopup();
      }
    return busy;
  }
  
  //// COMBO SETTING ////
  inline bool ComboSetting::onDraw(float scale, bool busy)
  { // COMBOBOX
    if(ImGui::BeginCombo(("##"+mId).c_str(), mChoices[*mData].c_str()))
      {
        busy = true;
        ImGui::SetWindowFontScale(scale);
        for(int i = 0; i < mChoices.size(); i++)
          {
            std::string &s = mChoices[i];
            if(ImGui::Selectable(((i == *mData ? "* " : "") + mChoices[i]).c_str()))
              { *mData = i; }
          }
        ImGui::EndCombo();
      }
    return busy;
  }

  //// SETTING GROUP ////
  inline bool SettingGroup::onDraw(float scale, bool busy)
  { // SETTING GROUP
    ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
    if(mCollapse)
      {
        ImGui::SetNextTreeNodeOpen(mActive);
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

  //// SETTING GROUP SAVE/LOAD ////
  inline void SettingGroup::getSaveParams(std::map<std::string, std::string> &params) const
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
  inline void SettingGroup::setSaveParams(std::map<std::string, std::string> &params)
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

}

#endif // SETTING_HPP
