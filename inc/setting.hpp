#ifndef SETTING_HPP
#define SETTING_HPP

#include <vector>
#include <sstream>
#include <iostream>
#include "imgui.h"
#include "glfwKeys.hpp"
#include "vector.hpp"
#include "chart.hpp"

// TODO: avoid including big file in header
#include "nlohmann/json.hpp" // #include "nlohmann/json_fwd.hpp" // json forward declarations
using json = nlohmann::json;
#define JSON_SPACES 4

namespace astro
{
  ////////////////////////////////
  //// SETTING  -- BASE CLASS ////
  ////////////////////////////////
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
    virtual json toJSON() const         { return json::object(); }
    virtual bool fromJSON(const json &js) { return true; }
    
    virtual void setLabelColWidth(float width) { mLabelColW = width; }
    virtual void setInputColWidth(float width) { mInputColW = width; }

    virtual bool hasChanged() const { return false; } // TODO
    virtual bool getDelete()  const { return false; }
    
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

    std::ostream& print(std::ostream &os) const
    {
      //os << getId() << " -->  " << std::setw(JSON_SPACES) << toJSON();
      os << getId() << " = " << toJSON();
      return os;
    }
  };
  
  ////////////////////////////////////
  //// SETTING --  TEMPLATE CLASS ////
  ////////////////////////////////////
  
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
    virtual json toJSON() const override;
    virtual bool fromJSON(const json &js) override;
    
    virtual bool getDelete() const override { return mDelete; }
  };

  ///////////////////////////
  //// SETTING SAVE/LOAD ////
  ///////////////////////////
  template<typename T>
  inline json Setting<T>::toJSON() const
  {
    std::stringstream ss;
    if(mData) { ss << (*mData); }
    json js = ss.str();
    return js;
  }
  template<typename T>
  inline bool Setting<T>::fromJSON(const json &js)
  {
    std::stringstream ss(js.get<std::string>());
    if(!js.is_null()) { ss >> (*mData); return true; }
    else              { return false; }
  }

  //// SETTING DRAW SPECIALIZATIONS (BY TYPE) ////
  template<> inline bool Setting<bool>::onDraw(float scale, bool busy)
  { //// BOOL (checkbox)
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
    ImGui::Checkbox(("##"+mId).c_str(), mData);
    ImGui::PopStyleVar();
    return busy;
  }
  template<> inline bool Setting<BoolStruct>::onDraw(float scale, bool busy)
  { //// BOOL (checkbox)
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
    ImGui::Checkbox(("##"+mId).c_str(), &mData->data);
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
  template<> inline bool Setting<double>::onDraw(float scale, bool busy)
  { //// FLOAT
    ImGui::InputDouble(("##"+mId).c_str(), mData, 1.0f, 10.0f);
    return busy;
  }
  template<> inline bool Setting<std::string>::onDraw(float scale, bool busy)
  { //// STRING
    char data[1024] = {0};
    std::copy(mData->begin(), mData->end(), data);
    if(ImGui::InputText(("##"+mId).c_str(), data, 1024))
      {
        // mChanged |= (data != *mData)); // TODO -- mark if changed
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
        if(ImGui::Button("Cancel") || ImGui::IsKeyPressed(GLFW_KEY_ESCAPE)) // cancels current selection
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

  

  
  //////////////////////////
  //// COMBOBOX SETTING ////
  //////////////////////////
  
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
    virtual json toJSON() const override;
    virtual bool fromJSON(const json &js) override;
  };

  //// COMBO SETTING SAVE/LOAD ////
  inline json ComboSetting::toJSON() const
  {
    json combo = json::object();
    combo["selection"] = mChoices[*mData];
    return combo;
  }
  inline bool ComboSetting::fromJSON(const json &js)
  {
    if(js.contains("selection"))
      {
        std::string selection = js["selection"];
        auto iter = std::find(mChoices.begin(), mChoices.end(), selection);
        if(iter != mChoices.end())
          { *mData = (iter - mChoices.begin()); }
        else
          { std::cout << "WARNING: Could not find combo setting: '" << selection << "' in choices! (" << getId() << ")\n"; }
      }
    return true;
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

  /////////////////////////////
  //// SETTING GROUP CLASS ////
  /////////////////////////////
  
  class SettingGroup : public SettingBase
  {
  protected:
    std::vector<SettingBase*> mContents;
    bool mCollapse   = false; // true if group is collapsible (collapsing header vs. text title)
    bool mDelete     = false; // true if settings should be deleted
    bool mOpen       = false; // true if group is not collapsed
    int  mNumColumns = 1;     // number of setting columns (if 0, calculate best fit)
    bool mHorizontal = false; // if true, orders settings horizontally in columns (column-major)
    virtual bool onDraw(float scale, bool busy=false) override;
    bool drawContents(float scale, bool busy); // draws settings arranged in columns
  public:
    SettingGroup(const std::string &name_, const std::string &id_, const std::vector<SettingBase*> &contents, bool collapse=false, bool deleteContents=true)
      : SettingBase(name_, id_), mContents(contents), mCollapse(collapse), mDelete(deleteContents)
    {
      // TODO?
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
    
    // JSON
    virtual json toJSON() const override;
    virtual bool fromJSON(const json &js) override;
    virtual bool isGroup() const override { return true; }

    std::vector<SettingBase*>& contents() { return mContents; }
    const std::vector<SettingBase*>& contents() const { return mContents; }
    
    void add(SettingBase *setting) { mContents.push_back(setting); }
    const bool& open() const  { return mOpen; }
    bool& open() { return mOpen; }

    // pass to contents (TODO: replace with column organization)
    virtual void setLabelColWidth(float w) override { SettingBase::setLabelColWidth(w); for(auto s : mContents) { s->setLabelColWidth(w); } }
    virtual void setInputColWidth(float w) override { SettingBase::setInputColWidth(w); for(auto s : mContents) { s->setInputColWidth(w); } }

    virtual bool getDelete() const override { return mDelete; }
    
    void setColumns(int numColumns, bool horizontal=false)
    {
      mNumColumns = numColumns;
      mHorizontal = horizontal;
    }
  };
  
  // makes a group of settings referencing a vector of values
  template<typename T>
  inline SettingGroup* makeSettingGroup(const std::string &name, const std::string &id, std::vector<T> *contentData, bool collapse=false)
  {
    if(!contentData) { return nullptr; }
    std::vector<SettingBase*> contents;
    for(int i; i < contentData->size(); i++)
      {
        std::string index = std::to_string(i);
        contents.push_back(new Setting<T>(name+index, id+index, &contentData->at(i)));
      }
    return new SettingGroup(name, id, contents, collapse);
  }
  // makes a group of settings from an array of values  
  template<typename T, int N>
  inline SettingGroup* makeSettingGroup(const std::string &name, const std::string &id, std::array<T, N> *contentData, bool collapse=false)
  {
    if(!contentData) { return nullptr; }
    std::vector<SettingBase*> contents;
    for(int i; i < N; i++)
      {
        std::string index = std::to_string(i);
        contents.push_back(new Setting<T>(name+index, id+index, &contentData->at(i)));
      }
    return new SettingGroup(name, id, contents, collapse);
  }
  
  //// SETTING GROUP SAVE/LOAD ////
  inline json SettingGroup::toJSON() const
  {
    json js = json::object();
    if(mCollapse) { js["open"] = mOpen; }
    json contents = json::object();
    for(auto s : mContents) { contents[s->getId()] = s->toJSON(); }
    
    js["contents"] = contents;
    return js;
  }
  inline bool SettingGroup::fromJSON(const json &js)
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
          { for(int i = 0; i < mContents.size(); i++) { mContents[i]->fromJSON(contents[mContents[i]->getId()]); } }
        else { success = false; }
      }
    else { success = false; }
    return success;
  }
  
  //// SETTING GROUP DRAW //// 
  // TODO?: Add flag (or something) to prevent interaction during node placement (debounce)
  inline bool SettingGroup::drawContents(float scale, bool busy)
  { // draws settings arranged in columns

    // TODO
    // if(mNumColumns == 0)
    //   { // determine number of columns from available area and setting sizes
    //     Vec2f areaSize = Vec2f(ImGui::GetContentRegionMax()) - ImGui::GetWindowPos();
    //   }
    
    ImGui::Indent();
    ImGui::BeginGroup();
    {
      if(mHorizontal)
        { // draw each row horizontally (grouped by column for alignment)
          int numPerCol = (int)std::ceil(mContents.size() / mNumColumns);
          int row = 0;
          for(int i = 0; i < mContents.size(); i++)
            {
              SettingBase *s = mContents[numPerCol - (i % numPerCol)];
              if(i % mNumColumns == 0)
                {
                  // ImGui::BeginGroup();
                  // // get column width
                  // float labelColW = 0;
                  // for(int j = i; j < i+numPerCol; j++)
                  //   {
                  //     labelColW = std::max(labelColW, ImGui::CalcTextSize(mContents[numPerCol - (j % numPerCol)]->getName().c_str()).x/scale);
                  //     // inputColW = std::max(labelColW, ImGui::CalcTextSize(mContents[numPerCol - (j % numPerCol)]->name()).x/scale);
                  //   }
                  // for(int j = i; j < i+numPerCol; j++)
                  //   { mContents[numPerCol - (j % numPerCol)]->setLabelColWidth(labelColW+10.0f); }
                }
              busy |= s->draw(scale);
              if((i % numPerCol) == numPerCol || i == mContents.size()-1)
                { // last element in column
                  ImGui::EndGroup();
                  if(i < (mContents.size()-1)) // next column
                    { ImGui::SameLine(); }
                }
            }
        }
      else
        { // draw each column as group
          int numPerCol = (int)std::ceil(mContents.size() / mNumColumns);
          for(int i = 0; i < mContents.size(); i++)
            {
              SettingBase *s = mContents[i];
              if(i % numPerCol == 0)
                {
                  // // get column width
                  // float labelColW = 0;
                  // int lastInCol = std::min(i+numPerCol, (int)mContents.size());
                  // for(int j = i; j < lastInCol; j++)
                  //   {
                  //     labelColW = std::max(labelColW, ImGui::CalcTextSize(mContents[j]->getName().c_str()).x/scale);
                  //     // inputColW = std::max(labelColW, ImGui::CalcTextSize(mContents[numPerCol - (j % numPerCol)]->name()).x/scale); // TODO?
                  //   }
                  // for(int j = i; j < lastInCol; j++) { mContents[j]->setLabelColWidth(labelColW+10.0f); }
                  ImGui::BeginGroup();
                }
              
              busy |= s->draw(scale);
              if((i % numPerCol) == numPerCol-1 || i == mContents.size()-1)
                { // last element in column
                  ImGui::EndGroup();
                  if(i < (mContents.size()-1)) // next column
                    { ImGui::SameLine(); }
                }
            }
        }
    }
    ImGui::EndGroup();
    ImGui::Unindent();
    return busy;
  }
  
  inline bool SettingGroup::onDraw(float scale, bool busy)
  { // SETTING GROUP
    ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
    if(mCollapse)
      {
        ImGui::SetNextTreeNodeOpen(mOpen);
        if(ImGui::CollapsingHeader(mName.c_str(), nullptr, flags))
          {
            if(!busy)
             {
               mOpen = true;
            }
            busy |= drawContents(scale, busy);
          }
        else { mOpen = false; } // TODO: handle visibility (out of frame) ?
      }
    else
      {
        mOpen = true; // no collapse -- always open
        ImGui::TextUnformatted(mName.c_str()); // draw title
        ImGui::Separator();
        busy |= drawContents(scale, busy);
      }
    return busy;
  }
}

#endif // SETTING_HPP
