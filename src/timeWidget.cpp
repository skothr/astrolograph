#include "timeWidget.hpp"
using namespace astro;

#include <fstream>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "imgui.h"


TimeWidget::TimeWidget()
  : mDate(DateTime::now())
{ }

TimeWidget::TimeWidget(const DateTime &date)
  : mDate(date)
{ mDate.fix(); }

TimeWidget::TimeWidget(const TimeWidget &other)
  : mDate(other.mDate), mSavedDate(other.mSavedDate), mDST(other.mDST)
{
  sprintf(mName, "%s", other.mName);
  sprintf(mSavedName, "%s", other.mSavedName);
}

TimeWidget& TimeWidget::operator=(const TimeWidget &other)
{
  mDate = other.mDate;
  mSavedDate = other.mSavedDate;
  sprintf(mName, "%s", other.mName);
  sprintf(mSavedName, "%s", other.mSavedName);
  mDST = other.mDST;
  return *this;
}

bool TimeWidget::save(const std::string &name)
{
  if(!fs::exists(DATE_SAVE_DIR))
    { // make sure save directory exists
      std::cout << "Creating save directory (" << DATE_SAVE_DIR << ")...\n";
      if(!fs::create_directory(DATE_SAVE_DIR))
        { std::cout << "ERROR: Could not create date save directory.\n"; return false; }
    }
  if(name.empty())
    { std::cout << "TimeWidget::save() --> Please enter a name!\n"; return false; }

  // read saved dates
  std::vector<DateSave> data;
  bool update = false; // if true, updating saved date
  if(fs::exists(DATE_SAVE_PATH) && fs::is_regular_file(DATE_SAVE_PATH))
    {
      std::ifstream dateFile(DATE_SAVE_PATH, std::ifstream::in);
      std::string line  = "";
      while(std::getline(dateFile, line))
        {
          DateTime dt;
          std::string n = popName(line);
          dt.fromSaveString(line);
          if(n == name)
            { data.push_back({n, mDate}); update = true; }
          else
            { data.push_back({n, dt}); }
        }
    }

  if(!update)
    { // append new date
      data.push_back({name, mDate});
    }
  
  // write date to file
  std::ofstream dateFile(DATE_SAVE_PATH, std::ios::out);
  for(auto &d : data)
    { dateFile << std::quoted(d.name) << " " << d.date.toSaveString() << "\n"; }

  mSavedDate = mDate;
  return true;
}

bool TimeWidget::load(const std::string &name)
{
  if(!fs::exists(DATE_SAVE_DIR)) { return false; }
  
  // read saved dates
  if(fs::exists(DATE_SAVE_PATH) && fs::is_regular_file(DATE_SAVE_PATH))
    {
      std::ifstream dateFile(DATE_SAVE_PATH, std::ifstream::in);
      std::string line  = "";
      while(std::getline(dateFile, line))
        {
          std::string n = popName(line);
          DateTime dt(line);
          if(n == name)
            {
              mSavedDate = dt;
              mSavedDate.fix();
              mDate = mSavedDate;
              return true;
            }
        }
    }
  return false;
}

bool TimeWidget::remove(const std::string &name)
{
  if(!fs::exists(DATE_SAVE_DIR))
    { // make sure save directory exists
      std::cout << "Creating save directory (" << DATE_SAVE_DIR << ")...\n";
      if(!fs::create_directory(DATE_SAVE_DIR))
        { std::cout << "ERROR: Could not create date save directory.\n"; return false; }
    }
  if(name.empty())
    { std::cout << "TimeWidget::remove() --> Empty name!\n"; return false; }

  // read saved dates
  std::vector<DateSave> data;
  bool found = false; // if true, updating saved date
  if(fs::exists(DATE_SAVE_PATH) && fs::is_regular_file(DATE_SAVE_PATH))
    {
      std::ifstream dateFile(DATE_SAVE_PATH, std::ifstream::in);
      std::string line  = "";
      while(std::getline(dateFile, line))
        {
          DateTime dt;
          std::string n = popName(line);
          dt.fromSaveString(line);
          if(n == name) // remove by skipping
            { found = true; }
          else
            { data.push_back({n, dt}); }
        }
    }
  
  // write date to file
  std::ofstream dateFile(DATE_SAVE_PATH, std::ios::out);
  for(auto d : data)
    { dateFile << std::quoted(d.name) << " " << d.date.toSaveString() << "\n"; }

  return true;
}

std::vector<DateSave> TimeWidget::loadAll()
{
  if(!fs::exists(DATE_SAVE_DIR)) { return {}; }
  
  // read all saved dates
  std::vector<DateSave> data;
  if(fs::exists(DATE_SAVE_PATH) && fs::is_regular_file(DATE_SAVE_PATH))
    {
      std::ifstream dateFile(DATE_SAVE_PATH, std::ifstream::in);
      std::string line = "";
      while(std::getline(dateFile, line))
        {
          if(!line.empty() && line != "\n")
            {
              DateTime dt;
              std::string n = popName(line);
              dt.fromSaveString(line);
              data.push_back({n, dt});
            }
        }
    }
  return data;
}

void TimeWidget::draw(const std::string &id, float scale)
{
  ImGui::BeginGroup();
  {
    // textbox widths
    const float yearWidth   = 100*scale;
    const float monthWidth  = 100*scale;
    const float dayWidth    = 100*scale;
    const float hourWidth   = 100*scale;
    const float minuteWidth = 100*scale;
    const float secondWidth = 100*scale;
    const float tzWidth     = 55*scale;
    // steps
    const short  yearStep   = 1;
    const char   monthStep  = 1;
    const char   dayStep    = 1;
    const char   hourStep   = 1;
    const char   minuteStep = 1;
    const double secondStep = 1.0;
    const double tzStep     = 1.0;
    // fast steps (ctrl+click)
    const short  yearFastStep   = 10;
    const char   monthFastStep  = 2;
    const char   dayFastStep    = 7;
    const char   hourFastStep   = 6;
    const char   minuteFastStep = 15;
    const double secondFastStep = 15.0;
    const double tzFastStep     = 15.0;
    // values
    short  yearVal   = mDate.year();
    char   monthVal  = mDate.month();
    char   dayVal    = mDate.day();
    char   hourVal   = mDate.hour();
    char   minuteVal = mDate.minute();
    double secondVal = mDate.second();

    //ImGui::Spacing();
    ImGui::BeginGroup();
    {
      ImGui::Text("Year");
      ImGui::PushItemWidth(yearWidth);
      ImGui::SameLine(55*scale); if(ImGui::InputScalar(("##year"+id).c_str(),   ImGuiDataType_S16,    &yearVal,   &yearStep,   &yearFastStep, "%d"))
                                   { mDate.setYear(yearVal); }
      ImGui::PopItemWidth();
      ImGui::Text("Month");
      ImGui::PushItemWidth(monthWidth);
      ImGui::SameLine(55*scale); if(ImGui::InputScalar(("##month"+id).c_str(),  ImGuiDataType_S8,     &monthVal,  &monthStep,  &monthFastStep, "%d"))
                                   { mDate.setMonth(monthVal); }
      ImGui::PopItemWidth();
      ImGui::Text("Day");
      ImGui::PushItemWidth(dayWidth);
      ImGui::SameLine(55*scale); if(ImGui::InputScalar(("##day"+id).c_str(), ImGuiDataType_S8,     &dayVal,    &dayStep,    &dayFastStep, "%d"))
                                   { mDate.setDay(dayVal); }
      ImGui::PopItemWidth();
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    {
      ImGui::Text("Hour");
      ImGui::PushItemWidth(hourWidth);
      ImGui::SameLine(55*scale); if(ImGui::InputScalar(("##hour"+id).c_str(),   ImGuiDataType_S8,     &hourVal,   &hourStep,   &hourFastStep, "%d"))
                                   { mDate.setHour(hourVal); }
      ImGui::PopItemWidth();
      ImGui::Text("Minute");
      ImGui::PushItemWidth(minuteWidth);
      ImGui::SameLine(55*scale); if(ImGui::InputScalar(("##minute"+id).c_str(), ImGuiDataType_S8,     &minuteVal, &minuteStep, &minuteFastStep, "%d"))
                                   { mDate.setMinute(minuteVal); }
      ImGui::PopItemWidth();
      ImGui::Text("Second");
      ImGui::PushItemWidth(secondWidth);
      ImGui::SameLine(55*scale); if(ImGui::InputScalar(("##second"+id).c_str(), ImGuiDataType_Double, &secondVal, &secondStep, &secondFastStep, "%2.2f"))
                                   { mDate.setSecond(secondVal); }
      ImGui::PopItemWidth();
    }
    ImGui::EndGroup();
    
    // display loaded name
    ImGui::Spacing();
    if(std::string(mName).empty())
      {
        ImGui::TextColored(ImColor(1.0f, 1.0f, 1.0f, 0.25f), "[]");
      }
    else
      {
        std::string sName = mName;
        if(mDate != mSavedDate)
          { sName = std::string("[") + mName + "]"; }
        ImGui::TextColored(ImColor(1.0f, 1.0f, 1.0f, 0.5f), "%s", sName.c_str());
      }

    // display UTC offset
    double tzVal = mDate.utcOffset()+mDate.dstOffset();
    ImGui::SameLine(165*scale);
    ImGui::TextUnformatted("UTC");
    ImGui::PushItemWidth(tzWidth);
    ImGui::SameLine();
    ImGui::InputDouble(("##tzOffset"+id).c_str(), &tzVal, 0.0, 0.0, "%+2.2f");
    mDate.setUtcOffset(tzVal-mDate.dstOffset());
    ImGui::PopItemWidth();

    bool dst = (mDate.dstOffset() != 0.0);
    ImGui::SameLine();
    ImGui::TextUnformatted("DST");
    ImGui::SameLine(); ImGui::Checkbox("##DST", &dst);
    if(dst && mDate.dstOffset() == 0.0) { mDate.setDstOffset(1.0); }
    else if(!dst)                       { mDate.setDstOffset(0.0); }

    
    // load button
    ImGui::Button(("Load##date"+id).c_str());
    if(ImGui::BeginPopupContextItem(("loadPopup##"+id).c_str(), ImGuiMouseButton_Left))
      {
        std::vector<DateSave> loaded = loadAll();
        for(auto &dt : loaded)
          {
            if(ImGui::MenuItem((dt.name+"##"+id).c_str()))
              {
                sprintf(mName, "%s", dt.name.c_str());
                mDate = dt.date;
                mSavedDate = dt.date;
                std::cout << "Date '" << dt.name << "' loaded!\n";
              }
          }
        ImGui::EndPopup();
      }
    
    // save button
    ImGui::SameLine();
    ImGui::Button(("Save##date"+id).c_str());

    // save menu
    if(ImGui::BeginPopupContextItem(("savePopup##"+id).c_str(), ImGuiMouseButton_Left))
      {
        // text input for new save
        ImGui::Text("New");
        ImGui::SameLine();
        ImGui::InputText(("##saveInput"+id).c_str(), mSavedName, DATE_NAME_BUFLEN);
        ImGuiIO& io = ImGui::GetIO();
            
        bool enter = ImGui::IsKeyPressed(io.KeyMap[ImGuiKey_Enter]);
        ImGui::SameLine();
        enter |= ImGui::Button(("Save##date2"+id).c_str());
        if(enter)
          {
            if(!save(std::string(mSavedName)))
              { std::cout << "Failed to save date as '" << mSavedName << "'!\n"; }
            else
              {
                std::cout << "Date saved as '" << mSavedName << "'!\n";
                sprintf(mName, "%s", mSavedName);
              }
            ImGui::CloseCurrentPopup();
          }
        
        ImGui::Spacing();
        ImGui::Separator();
        
        // display existing dates
        std::vector<DateSave> loaded = loadAll();
        for(auto &dt : loaded)
          {
            // delete button (X)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
            if(ImGui::Button(("X##"+dt.name+id).c_str()))
              {
                std::cout << "Removing date '" << dt.name << "'!\n";
                remove(dt.name);
                if(mName == dt.name) { mName[0] = '\0'; } // (mName == "")
                if(mSavedName == dt.name) { mSavedName[0] = '\0'; } // (mSavedName == "")
              }
            ImGui::PopStyleColor();
            
            // overwrite
            ImGui::SameLine();
            if(ImGui::MenuItem((dt.name+"##"+id).c_str()))
              {
                if(!save(dt.name))
                  { std::cout << "Failed to save date '" << dt.name << "'!\n"; }
                else
                  {
                    sprintf(mName, "%s", dt.name.c_str());
                    std::cout << "Date saved as '" << mName << "'!\n";
                  }
              }
          }
        ImGui::EndPopup();
      }
    else // fill buffer with current name
      { sprintf(mSavedName, "%s", mName); }

    if(!std::string(mName).empty())
      {
        ImGui::SameLine();
        if(ImGui::Button(("Reload##date"+id).c_str()))
          { mDate = mSavedDate; }
      }
  }
  ImGui::EndGroup();
  mDate.fix();
}
