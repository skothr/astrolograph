#ifndef SETTING_HPP
#define SETTING_HPP

#include <vector>
#include <sstream>
#include <iostream>
#include "imgui.h"
#include "glfwKeys.hpp"
#include "vector.hpp"
#include "chart.hpp"
#include "dateTime.hpp"
#include "location.hpp"

// TODO: avoid including big file in header
#include "nlohmann/json.hpp" // #include "nlohmann/json_fwd.hpp" // json forward declarations
using json = nlohmann::json;
#define JSON_SPACES 4

typedef const std::function<void(void)> SettingCallback;
  
////////////////////////////////
//// SETTING  -- BASE CLASS ////
////////////////////////////////
class SettingBase
{
protected:
  std::string mName;
  std::string mId;
  float mLabelColW = 200; // width of column with setting name labels    
  float mInputColW = 150; // width of column with setting input widget(s)
    
  virtual bool onDraw(float scale, bool busy, bool &changed, bool visible) { return busy; }
    
public:
  SettingCallback callback = nullptr;
  SettingBase(const std::string &name, const std::string &id, const SettingCallback &cb=nullptr) : mName(name), mId(id), callback(cb) { }
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
  bool draw(float scale, bool busy, bool &changed, bool visible)
  {
    if(!isGroup())
      {
        ImGui::TextUnformatted(mName.c_str());
        ImGui::SameLine(mLabelColW*scale);
        ImGui::SetNextItemWidth(mInputColW*scale);
      }
    return onDraw(scale, busy, changed, visible);
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
  T mStep    = T(); T mBigStep = T();
  T mMinVal  = T(); T mMaxVal  = T();
  bool mMinSet = false; bool mMaxSet = false;
  std::string mFormat = "";

  virtual bool onDraw(float scale, bool busy, bool &changed, bool visible) { return busy; }
public:
  typedef Setting<T> type;
  // construction
  Setting(const std::string &name, const std::string &id, T *ptr, const T &defaultVal=T(), const SettingCallback &cb=nullptr)
    : SettingBase(name, id, cb), mData(ptr), mDefault(*mData) { }
  Setting(const std::string &name, const std::string &id, const T &val, const T &defaultVal=T(), const SettingCallback &cb=nullptr)
    : Setting(name, id, new T(val), defaultVal, cb)           { mDelete = true; }
  Setting(const std::string &name, const std::string &id, const SettingCallback &cb=nullptr)
    : Setting(name, id, T(), T(), cb) { }
  // destruction
  virtual ~Setting() { if(mDelete && mData) { delete mData; } }

  void setFormat(const T &step=T(),   const T &bigStep=T(), const std::string &format="") { mStep = step; mBigStep = bigStep; mFormat = format; }
  void setMin   (const T &minVal) { mMinVal = minVal; mMinSet = true; }
  void setMax   (const T &maxVal) { mMaxVal = maxVal; mMaxSet = true; }
    
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
  if(mData) { ss << std::fixed << (*mData); }
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

// overloads for string (needs quotes?)
template<>
inline json Setting<std::string>::toJSON() const
{
  std::stringstream ss;
  if(mData) { ss << std::quoted(*mData); }
  json js = ss.str();
  return js;
}
template<>
inline bool Setting<std::string>::fromJSON(const json &js)
{
  std::stringstream ss(js.get<std::string>());
  if(!js.is_null()) { ss >> std::quoted(*mData); return true; }
  else              { return false; }
}


  
// // overloads for DateTime/Location
// template<>
// inline json Setting<DateTime>::toJSON() const
// {
//   std::stringstream ss;
//   if(mData) { ss << std::fixed << (mData->toSaveString()); }
//   json js = ss.str();
//   return js;
// }
// template<>
// inline bool Setting<DateTime>::fromJSON(const json &js)
// {
//   std::stringstream ss(js.get<std::string>());
//   if(!js.is_null()) { (mData->fromSaveString(ss.str())); return true; }
//   else              { return false; }
// }
// // overloads for DateTime/Location
// template<>
// inline json Setting<Location>::toJSON() const
// {
//   std::stringstream ss;
//   if(mData) { ss << std::fixed << (mData->toSaveString()); }
//   json js = ss.str();
//   return js;
// }
// template<>
// inline bool Setting<Location>::fromJSON(const json &js)
// {
//   std::stringstream ss(js.get<std::string>());
//   if(!js.is_null()) { (mData->fromSaveString(ss.str())); return true; }
//   else              { return false; }
// }

//// SETTING DRAW SPECIALIZATIONS (BY TYPE) ////
template<> inline bool Setting<bool>::onDraw(float scale, bool busy, bool &changed, bool visible)
{ //// BOOL (checkbox)
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
  changed |= ImGui::Checkbox(("##"+mId).c_str(), mData);
  ImGui::PopStyleVar();
  return busy;
}
template<> inline bool Setting<BoolStruct>::onDraw(float scale, bool busy, bool &changed, bool visible)
{ //// BOOL (checkbox)
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(0,0));
  changed |= ImGui::Checkbox(("##"+mId).c_str(), &mData->data);
  ImGui::PopStyleVar();
  return busy;
}
template<> inline bool Setting<int>::onDraw(float scale, bool busy, bool &changed, bool visible)
{ //// INT
  if(mStep == 0)      { mStep = 1.0f; }
  if(mBigStep == 0)   { mBigStep = 10.0f; }
  int v = *mData;
  if(ImGui::InputInt(("##"+mId).c_str(), &v, mStep, mBigStep))
    { if(mMaxSet) { v = std::min(mMaxVal, v); } if(mMinSet) { v = std::max(mMinVal, v); } changed = true; *mData = v; }
  return busy;
}
template<> inline bool Setting<float>::onDraw(float scale, bool busy, bool &changed, bool visible)
{ //// FLOAT
  if(mStep == 0.0f)    { mStep = 1.0f; }
  if(mBigStep == 0.0f) { mBigStep = 10.0f; }
  if(mFormat.empty())  { mFormat = "%.2f"; }
  float v = *mData;
  if(ImGui::InputFloat(("##"+mId).c_str(), &v, mStep, mBigStep, mFormat.c_str()))
    { if(mMaxSet) { v = std::min(mMaxVal, v); } if(mMinSet) { v = std::max(mMinVal, v); } changed = true; *mData = v; }
    
  return busy;
}
template<> inline bool Setting<double>::onDraw(float scale, bool busy, bool &changed, bool visible)
{ //// DOUBLE
  if(mStep == 0.0)    { mStep = 1.0f; }
  if(mBigStep == 0.0) { mBigStep = 10.0f; }
  if(mFormat.empty()) { mFormat = "%.2f"; }
  double v = *mData;
  if(ImGui::InputDouble(("##"+mId).c_str(), &v, 1.0f, 10.0f))
    { if(mMaxSet) { v = std::min(mMaxVal, v); } if(mMinSet) { v = std::max(mMinVal, v); } changed = true; *mData = v; }
  return busy;
}
template<> inline bool Setting<std::string>::onDraw(float scale, bool busy, bool &changed, bool visible)
{ //// STRING
  char data[1024] = {0};
  std::copy(mData->begin(), mData->end(), data);
  if(ImGui::InputText(("##"+mId).c_str(), data, 1024))
    { changed = true; *mData = data; }
  return busy;
}

  
template<> inline bool Setting<Vec2i>::onDraw(float scale, bool busy, bool &changed, bool visible)
{ //// VEC2I
  if(mStep.x == 0)    { mStep.x = 1; }       if(mStep.y == 0)     { mStep.y = 1; }
  if(mBigStep.x == 0) { mBigStep.x = 10; }   if(mBigStep.y == 0)  { mBigStep.y = 10; }
  if(mFormat.empty()) { mFormat = "%.2f"; }
  ImGui::BeginGroup();
  {
    bool edited = (ImGui::IsKeyPressed(GLFW_KEY_ENTER) || ImGui::IsKeyPressed(GLFW_KEY_TAB) ||
                   ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Left));
      
    static const std::string xLabel = "X="; static const std::string yLabel = "Y=";
    Vec2f tSize = ImGui::CalcTextSize(xLabel.c_str());
    ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos())-Vec2f(tSize.x+2.0f*ImGui::GetStyle().ItemInnerSpacing.x, 0.0f));
    ImGui::TextUnformatted(xLabel.c_str());
    ImGui::SameLine(); ImGui::SetNextItemWidth(mInputColW/2.0f*scale);
    int v = mData->x;
    if(ImGui::InputInt(("##"+mId+"X").c_str(), &v, mStep.x, mBigStep.x) && edited)
      { if(mMaxSet) { v = std::min(mMaxVal.x, v); } if(mMinSet) { v = std::max(mMinVal.x, v); } changed = true; mData->x = v; }

    tSize = ImGui::CalcTextSize(yLabel.c_str());
    ImGui::SameLine(); ImGui::TextUnformatted(yLabel.c_str());
    ImGui::SameLine(); ImGui::SetNextItemWidth(mInputColW/2.0f*scale);
    v = mData->y;
    if(ImGui::InputInt(("##"+mId+"Y").c_str(), &v, mStep.y, mBigStep.y) && edited)
      { if(mMaxSet) { v = std::min(mMaxVal.y, v); } if(mMinSet) { v = std::max(mMinVal.y, v); } changed = true; mData->y = v; }
  }
  ImGui::EndGroup();
  return busy;
}
template<> inline bool Setting<Vec2f>::onDraw(float scale, bool busy, bool &changed, bool visible)
{ //// VEC2F
  if(mStep.x == 0.0f)    { mStep.x = 1.0f; }     if(mStep.y == 0.0f)    { mStep.y = 1.0f; }
  if(mBigStep.x == 0.0f) { mBigStep.x = 10.0f; } if(mBigStep.y == 0.0f) { mBigStep.y = 10.0f; }
  if(mFormat.empty())    { mFormat = "%.2f"; }
  ImGui::BeginGroup();
  {
    static const std::string  xLabel = "X="; static const std::string  yLabel = "Y=";
    Vec2f tSize = ImGui::CalcTextSize(xLabel.c_str());
    ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos())-Vec2f(tSize.x+2.0f*ImGui::GetStyle().ItemInnerSpacing.x, 0.0f));
    ImGui::TextUnformatted(xLabel.c_str());
    ImGui::SameLine(); ImGui::SetNextItemWidth(mInputColW/2.0f*scale);
    float v = mData->x;
    if(ImGui::InputFloat(("##"+mId+"X").c_str(), &v, mStep.x, mBigStep.x, mFormat.c_str()))
      { if(mMaxSet) { v = std::min(mMaxVal.x, v); } if(mMinSet) { v = std::max(mMinVal.x, v); } changed = true; mData->x = v; }
      
    ImGui::SameLine(); ImGui::TextUnformatted(yLabel.c_str());
    ImGui::SameLine(); ImGui::SetNextItemWidth(mInputColW/2.0f*scale);
    v = mData->y;
    if(ImGui::InputFloat(("##"+mId+"Y").c_str(), &v, mStep.y, mBigStep.y, mFormat.c_str()))
      { if(mMaxSet) { v = std::min(mMaxVal.y, v); } if(mMinSet) { v = std::max(mMinVal.y, v); } changed = true; mData->y = v; }
  }
  ImGui::EndGroup();
  return busy;
}
template<> inline bool Setting<Vec4f>::onDraw(float scale, bool busy, bool &changed, bool visible)
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
      changed |= ImGui::ColorPicker4(pickerName.c_str(), mData->data.data(), cFlags, lastColor.data.data());
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
        { *mData = mDefault; changed = true; }
      ImGui::EndPopup();
    }
  return busy;
}
template<> inline bool Setting<Vec2d>::onDraw(float scale, bool busy, bool &changed, bool visible)
{ //// VEC2D
  if(mStep.x == 0.0)    { mStep.x = 1.0; }     if(mStep.y == 0.0)    { mStep.y = 1.0; }
  if(mBigStep.x == 0.0) { mBigStep.x = 10.0; } if(mBigStep.y == 0.0) { mBigStep.y = 10.0; }
  if(mFormat.empty())   { mFormat = "%.2f"; }
  ImGui::BeginGroup();
  {
    static const std::string xLabel = "X="; static const std::string yLabel = "Y=";
    Vec2f tSize = ImGui::CalcTextSize(xLabel.c_str());
    ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos())-Vec2f(tSize.x+2.0f*ImGui::GetStyle().ItemInnerSpacing.x, 0.0f));
    ImGui::TextUnformatted(xLabel.c_str());
    ImGui::SameLine(); ImGui::SetNextItemWidth(mInputColW/2.0f*scale);
    double v = mData->x;
    if(ImGui::InputDouble(("##"+mId+"X").c_str(), &v, mStep.x, mBigStep.x, mFormat.c_str()))
      { if(mMaxSet) { v = std::min(mMaxVal.x, v); } if(mMinSet) { v = std::max(mMinVal.x, v); } changed = true; mData->x = v; }

    tSize = ImGui::CalcTextSize(yLabel.c_str());
    ImGui::SameLine(); ImGui::TextUnformatted(yLabel.c_str()); ImGui::SameLine();
    ImGui::SetNextItemWidth(mInputColW/2.0f*scale);
    v = mData->x;
    if(ImGui::InputDouble(("##"+mId+"Y").c_str(), &v, mStep.y, mBigStep.y, mFormat.c_str()))
      { if(mMaxSet) { v = std::min(mMaxVal.y, v); } if(mMinSet) { v = std::max(mMinVal.y, v); } changed = true; mData->y = v; }
  }
  ImGui::EndGroup();
  return busy;
}

  

  
//////////////////////////
//// COMBOBOX SETTING ////
//////////////////////////
  
class ComboSetting : public Setting<int>
{
protected:
  std::vector<std::string> mChoices;
  virtual bool onDraw(float scale, bool busy, bool &changed, bool visible) override;

public:
  ComboSetting(const std::string &name, const std::string &id, const SettingCallback &cb=nullptr)
    : Setting<int>(name, id, 0, 0, cb) { }
  ComboSetting(const std::string &name, const std::string &id, int *selection, const std::vector<std::string> &choices, const SettingCallback &cb=nullptr)
    : Setting<int>(name, id, selection, 0, cb), mChoices(choices) { }
  ComboSetting(const std::string &name, const std::string &id, int *selection, const std::vector<std::string> &choices,
               int defaultVal, const SettingCallback &cb=nullptr)
    : Setting<int>(name, id, selection, defaultVal, cb), mChoices(choices) { }
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
inline bool ComboSetting::onDraw(float scale, bool busy, bool &changed, bool visible)
{ // COMBOBOX
  if(ImGui::BeginCombo(("##"+mId).c_str(), mChoices[*mData].c_str()))
    {
      busy = true;
      ImGui::SetWindowFontScale(scale);
      for(int i = 0; i < mChoices.size(); i++)
        {
          std::string &s = mChoices[i];
          if(ImGui::Selectable(((i == *mData ? "* " : "") + mChoices[i]).c_str()))
            { changed = true; *mData = i; }
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
  virtual bool onDraw(float scale, bool busy, bool &changed, bool visible) override;
  bool drawContents(float scale, bool busy, bool &changed, bool visible); // draws settings arranged in columns
public:
  SettingGroup(const std::string &name_, const std::string &id_, const std::vector<SettingBase*> &contents,
               bool collapse=false, bool deleteContents=true, const SettingCallback &cb=nullptr)
    : SettingBase(name_, id_, cb), mContents(contents), mCollapse(collapse), mDelete(deleteContents)
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
inline bool SettingGroup::drawContents(float scale, bool busy, bool &changed, bool visible)
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
            changed = false;
            busy |= s->draw(scale, false, changed, visible);
            if(changed && s->callback) { s->callback(); }
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
              
            changed = false;
            busy |= s->draw(scale, false, changed, visible);
            if(changed && s->callback) { s->callback(); }
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
  
inline bool SettingGroup::onDraw(float scale, bool busy, bool &changed, bool visible)
{ // SETTING GROUP
  ImGuiTreeNodeFlags flags = (ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_SpanAvailWidth);
  if(mCollapse)
    {
      ImGui::SetNextTreeNodeOpen(mOpen);
      if(ImGui::CollapsingHeader(mName.c_str(), nullptr, flags))
        {
          if(!busy) { mOpen = true; }
          busy |= drawContents(scale, busy, changed, visible);
        }
      else if(visible) { mOpen = false; }
    }
  else
    {
      mOpen = true; // no collapse -- always open
      ImGui::TextUnformatted(mName.c_str()); // draw title
      ImGui::Separator();
      busy |= drawContents(scale, busy, changed, visible);
    }
  return busy;
}

#endif // SETTING_HPP
