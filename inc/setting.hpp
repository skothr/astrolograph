#ifndef SETTING_HPP
#define SETTING_HPP

#include <vector>
#include <sstream>
#include <iostream>
#include "imgui.h"
#include "glfwKeys.hpp"
#include "vector.hpp"

// TODO: avoid including big file in header
#include "nlohmann/json.hpp" // #include "nlohmann/json_fwd.hpp" // json forward declarations
using json = nlohmann::json;

namespace astro
{
  //// SETTING BASE CLASS ////
  class SettingBase
  {
  protected:
    std::string mName = "";
    std::string mId   = "";
    float mLabelColW = 200; // width of column with setting name labels    
    float mInputColW = 150; // width of column with setting input widget(s)
    virtual bool onDraw(float scale, bool busy=false) { return busy; }
    
  public:
    SettingBase(const std::string &name, const std::string &id) : mName(name), mId(id) { }
    virtual ~SettingBase() { }

    virtual bool isGroup() const { return false; }
    std::string getName() const  { return mName; }
    std::string getId() const    { return mId; }
    
    // JSON
    virtual json getJson() const         { return json::object(); }
    virtual bool setJson(const json &js) { return true; }
    
    virtual void setLabelColWidth(float width) { mLabelColW = width; }
    virtual void setInputColWidth(float width) { mInputColW = width; }

    virtual bool hasChanged() const { return false; } // TODO
    
    // TODO: improve flexibility
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
  protected:
    T *mData   = nullptr;
    T mDefault;
    virtual bool onDraw(float scale, bool busy=false) { return busy; }
  public:
    typedef Setting<T> type;
    // construction
    Setting(const std::string &name, const std::string &id, T *ptr, const T &defaultVal=T())
      : SettingBase(name, id), mData(ptr), mDefault(*mData) { }
    Setting(const std::string &name, const std::string &id, const T &val, const T &defaultVal = T())
      : Setting(name, id, new T(val), defaultVal)           { mDelete = true; }
    Setting(const std::string &name, const std::string &id)
      : Setting(name, id, T())                              { }
    // destruction
    virtual ~Setting() { if(mDelete && mData) { delete mData; } }
    
    // JSON
    virtual json getJson() const override;
    virtual bool setJson(const json &js) override;
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
    ComboSetting(const std::string &name, const std::string &id, int *selection, const std::vector<std::string> &choices, int defaultVal)
      : Setting<int>(name, id, selection, defaultVal), mChoices(choices) { }
    ~ComboSetting() { }
    
    // JSON
    virtual json getJson() const override;
    virtual bool setJson(const json &js) override;
  };


  
  //// SETTING GROUP ////
  class SettingGroup : public SettingBase
  {
  protected:
    bool mCollapse = false; // true if group is collapsible (collapsing header vs. text title)
    bool mDelete   = false; // true if settings should be deleted
    bool mOpen     = false; // true if group is not collapsed
    std::vector<SettingBase*> mContents;
    virtual bool onDraw(float scale, bool busy=false) override;
    
  public:
    SettingGroup(const std::string &name_, const std::string &id_, const std::vector<SettingBase*> &contents, bool collapse=false, bool deleteContents=true)
      : SettingBase(name_, id_), mContents(contents), mCollapse(collapse), mDelete(deleteContents)
    {
      // if(mCollapse) { mContents.push_back(new Setting<bool>("Group Open", "open", &mOpen)); }
    }
    ~SettingGroup()
    {
      if(mDelete)
        {
          for(auto s : mContents) { delete s; }
          mContents.clear();
        }
    }

    void add(SettingBase *setting) { mContents.push_back(setting); }
    const bool& open() const  { return mOpen; }
    bool& open() { return mOpen; }

    // JSON
    virtual json getJson() const override;
    virtual bool setJson(const json &js) override;
    
    virtual bool isGroup() const override { return true; }

    // pass to contents
    virtual void setLabelColWidth(float w) override { SettingBase::setLabelColWidth(w); for(auto s : mContents) { s->setLabelColWidth(w); } }
    virtual void setInputColWidth(float w) override { SettingBase::setInputColWidth(w); for(auto s : mContents) { s->setInputColWidth(w); } }
  };

  // makes a group of settings from a vector of values
  template<typename T>
  inline SettingGroup* makeSettingGroup(const std::string &name, const std::string &id, std::vector<T> *contentData, bool collapse=false)
  {
    std::vector<SettingBase*> contents;
    for(int i; i < contentData->size(); i++)
      {
        std::string index = std::to_string(i);
        T *ptr = &((*contentData)[i]);
        contents.push_back(new Setting<T>(name+index, id+index, ptr));
      }
    return new SettingGroup(name, id, contents, collapse);
  }

  // makes a group of settings from an array of values  
  template<typename T, int N>
  inline SettingGroup* makeSettingGroup(const std::string &name, const std::string &id, std::array<T, N> *contentData, bool collapse=false)
  {
    std::vector<SettingBase*> contents;
    for(int i; i < N; i++)
      {
        std::string index = std::to_string(i);
        T *ptr = &((*contentData)[i]);
        contents.push_back(new Setting<T>(name+index, id+index, ptr));
      }
    return new SettingGroup(name, id, contents, collapse);
  }
  
  //////////////////////////////////
  //// SAVING/LOADING FROM JSON ////
  //////////////////////////////////

  
  //// SETTING SAVE/LOAD ////
  template<typename T>
  inline json Setting<T>::getJson() const
  {
    json js;
    if(mData)
      {
        std::stringstream ss; ss << (*mData);
        js = ss.str();
      }
    return js;
  }
  template<typename T>
  inline bool Setting<T>::setJson(const json &js)
  {
    bool success = true;
    if(!js.is_null()) { std::stringstream(js.get<std::string>()) >> (*mData); }
    else              { success = false; }
    return success;
  }

  //// COMBO SETTING SAVE/LOAD ////
  inline json ComboSetting::getJson() const
  {
    json combo = json::object();
    combo["selection"] = Setting<int>::getJson();
    combo["choices"]   = mChoices;
    return combo;
  }
  inline bool ComboSetting::setJson(const json &js)
  {
    if(js.contains("selection"))
      { Setting<int>::setJson(js["selection"]); }
    if(js.contains("choices"))
      {
        json choices = js["choices"];
        mChoices.clear();
        mChoices.reserve(choices.size());
        for(auto c : choices) { mChoices.push_back(c); }
      }
    return true;
  }

  //// SETTING GROUP SAVE/LOAD ////
  inline json SettingGroup::getJson() const
  {
    json js = json::object();
    if(mCollapse) { js["open"] = mOpen; }
    
    json contents = json::array();
    for(auto s : mContents)
      { contents.push_back(s->getJson()); }
    js["contents"] = contents;
    return js;
  }
  inline bool SettingGroup::setJson(const json &js)
  {
    bool success = true;
    if(mCollapse)
      {
        if(js.contains("open")) { mOpen = js["open"].get<bool>(); }
        else                    { success = false; }
      }

    if(js.contains("contents"))
      {
        json contents = js["contents"];
        if(contents.size() == mContents.size())
          {
            for(int i = 0; i < mContents.size(); i++)
              { mContents[i]->setJson(contents[i]); }
          }
        else { success = false; }
      }
    else { success = false; }
    return success;
  }

  
  //////////////////////////////////////////
  //// SETTING DRAW OVERLOADS (BY TYPE) ////
  //////////////////////////////////////////
  
  template<> inline bool Setting<bool>::onDraw(float scale, bool busy)
  { //// BOOL (checkbox)
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
    ImGui::Checkbox(("##"+mId).c_str(), mData);
    ImGui::PopStyleVar();
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
        // mChanged |= (data != *mData)); // TODO
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
    if(ImGui::ColorButton(buttonName.c_str(), *mData, cFlags, ImVec2(20, 20)) && !busy)
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
        bool hover = ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
        ImGui::ColorPicker4(pickerName.c_str(), mData->data.data(), cFlags, lastColor.data.data());
        hover |= ImGui::IsItemHovered();
        
        if(ImGui::Button("Select") || ImGui::IsKeyPressed(GLFW_KEY_ENTER) || (!hover && ImGui::IsMouseClicked(ImGuiMouseButton_Left))) // selects color
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
        ImGui::SetNextTreeNodeOpen(mOpen);
        if(ImGui::CollapsingHeader(mName.c_str(), nullptr, flags))
          {
            mOpen = true;
            ImGui::Indent();
            ImGui::BeginGroup();
            for(auto s : mContents) { busy |= s->draw(scale); }
            ImGui::EndGroup();
            ImGui::Unindent();
          }
        else { mOpen = false; }
      }
    else
      {
        mOpen = false;
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
}

#endif // SETTING_HPP
