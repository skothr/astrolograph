#include "locationWidget.hpp"
using namespace astro;

#include <fstream>
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

#include "imgui.h"


LocationWidget::LocationWidget()
{ }
LocationWidget::LocationWidget(const Location &loc)
  : mLocation(loc) { mLocation.fix(); }

LocationWidget::LocationWidget(const LocationWidget &other)
  : mLocation(other.mLocation), mSavedLocation(other.mSavedLocation), mDST(other.mDST)
{
  sprintf(mName, other.mName);
  sprintf(mSavedName, other.mSavedName);
}

LocationWidget& LocationWidget::operator=(const LocationWidget &other)
{
  mLocation = other.mLocation;
  mSavedLocation = other.mSavedLocation;
  sprintf(mName, other.mName);
  sprintf(mSavedName, other.mSavedName);
  mDST = other.mDST;
  return *this;
}

bool LocationWidget::save(const std::string &name)
{
  if(!fs::exists(LOCATION_SAVE_DIR))
    { // make sure save directory exists
      std::cout << "Creating save directory (" << LOCATION_SAVE_DIR << ")...\n";
      if(!fs::create_directory(LOCATION_SAVE_DIR))
        { std::cout << "ERROR: Could not create location save directory.\n"; return false; }
    }
  if(name.empty())
    { std::cout << "LocationWidget::save() --> Please enter a name!\n"; return false; }

  // read saved locations
  std::vector<LocationSave> data;
  bool update = false; // if true, updating saved location
  if(fs::exists(LOCATION_SAVE_PATH) && fs::is_regular_file(LOCATION_SAVE_PATH))
    {
      std::ifstream locationFile(LOCATION_SAVE_PATH, std::ifstream::in);
      std::string line  = "";
      while(std::getline(locationFile, line))
        {
          Location loc;
          std::string n = popName(line);
          loc.fromSaveString(line);
          if(n == name)
            { data.push_back({n, mLocation}); update = true; }
          else
            { data.push_back({n, loc}); }
        }
    }

  if(!update)
    { // append new location
      data.push_back({name, mLocation});
    }
  
  // write location to file
  std::ofstream locationFile(LOCATION_SAVE_PATH, std::ios::out);
  for(int i = 0; i < data.size(); i++)
    { locationFile << std::quoted(data[i].name) << " " << data[i].location.toSaveString() << "\n"; }

  mSavedLocation = mLocation;
  return true;
}

bool LocationWidget::load(const std::string &name)
{
  if(!fs::exists(LOCATION_SAVE_DIR)) { return false; }
  
  // read saved locations
  if(fs::exists(LOCATION_SAVE_PATH) && fs::is_regular_file(LOCATION_SAVE_PATH))
    {
      std::ifstream locationFile(LOCATION_SAVE_PATH, std::ifstream::in);
      std::string line  = "";
      while(std::getline(locationFile, line))
        {
          std::string n = popName(line);
          Location loc(line);
          if(n == name)
            {
              mSavedLocation = loc;
              mSavedLocation.fix();
              mLocation = mSavedLocation;
              sprintf(mName, "%s", name.c_str());
              return true;
            }
        }
    }
  return false;
}

bool LocationWidget::remove(const std::string &name)
{
  if(!fs::exists(LOCATION_SAVE_DIR))
    { // make sure save directory exists
      std::cout << "Creating save directory (" << LOCATION_SAVE_DIR << ")...\n";
      if(!fs::create_directory(LOCATION_SAVE_DIR))
        { std::cout << "ERROR: Could not create location save directory.\n"; return false; }
    }
  if(name.empty())
    { std::cout << "LocationWidget::remove() --> Empty name!\n"; return false; }

  // read saved locations
  std::vector<LocationSave> data;
  bool found = false; // if true, updating saved location
  if(fs::exists(LOCATION_SAVE_PATH) && fs::is_regular_file(LOCATION_SAVE_PATH))
    {
      std::ifstream locationFile(LOCATION_SAVE_PATH, std::ifstream::in);
      std::string line  = "";
      while(std::getline(locationFile, line))
        {
          Location loc;
          std::string n = popName(line);
          loc.fromSaveString(line);
          if(n == name) // remove by skipping
            { found = true; }
          else
            { data.push_back({n, loc}); }
        }
    }
  
  // write location to file
  std::ofstream locationFile(LOCATION_SAVE_PATH, std::ios::out);
  for(auto d : data)
    { locationFile << std::quoted(d.name) << " " << d.location.toSaveString() << "\n"; }

  return true;
}

std::vector<LocationSave> LocationWidget::loadAll()
{
  if(!fs::exists(LOCATION_SAVE_DIR)) { return {}; }
  
  // read all saved locations
  std::vector<LocationSave> data;
  if(fs::exists(LOCATION_SAVE_PATH) && fs::is_regular_file(LOCATION_SAVE_PATH))
    {
      std::ifstream locationFile(LOCATION_SAVE_PATH, std::ifstream::in);
      std::string line  = "";
      while(std::getline(locationFile, line))
        {
          Location loc;
          std::string n = popName(line);
          loc.fromSaveString(line);
          data.push_back({n, loc});
        }
    }
  return data;
}

void LocationWidget::draw()
{
  mLocation.fix();
  ImGui::BeginGroup();
  {
    // steps
    const double latStep        = 0.001; // degrees
    const double lonStep        = 0.001; // degrees
    const int    altStep        = 10;    // m
    // fast steps
    const double latFastStep    = 0.01; // degrees
    const double lonFastStep    = 0.01; // degrees
    const int    altFastStep    = 100;  // m

    double latVal = mLocation.latitude;
    double lonVal = mLocation.longitude;
    int    altVal = mLocation.altitude;

    // latitude input
    ImGui::PushItemWidth(200);
    ImGui::Text("Latitude  (°)");
    ImGui::SameLine();
    if(ImGui::InputDouble("##Latitude",  &latVal, latStep, latFastStep, "%.12f"))
      { mLocation.latitude = latVal; }
    ImGui::PopItemWidth();
    // longitude input
    ImGui::Text("Longitude (°)");
    ImGui::PushItemWidth(200);
    ImGui::SameLine();
    if(ImGui::InputDouble("##Longitude", &lonVal, lonStep, lonFastStep, "%.12f"))
      { mLocation.longitude = lonVal; }
    ImGui::PopItemWidth();
    // altitude input
    ImGui::Text("Altitude  (m)");
    ImGui::PushItemWidth(100);
    ImGui::SameLine();
    if(ImGui::InputInt("##Altitude",     &altVal, altStep, altFastStep))
      { mLocation.altitude = altVal; }
    ImGui::PopItemWidth();

    // display loaded name
    ImGui::Spacing();
    if(std::string(mName).empty())
      { ImGui::TextColored(ImColor(1.0f, 1.0f, 1.0f, 0.25f), "[]"); }
    else
      {
        std::string sName = mName;
        if(mLocation != mSavedLocation)
          { sName = std::string("[") + mName + "]"; }
        ImGui::TextColored(ImColor(1.0f, 1.0f, 1.0f, 0.5f), sName.c_str());
      }
    
    // load button
    ImGui::Button("Load##loc");
    if(ImGui::BeginPopupContextItem("loadPopup", ImGuiMouseButton_Left))
      {
        std::vector<LocationSave> loaded = loadAll();
        for(auto &loc : loaded)
          {
            if(ImGui::MenuItem(loc.name.c_str()))
             { 
                sprintf(mName, "%s", loc.name.c_str());
                mLocation = loc.location;
                mSavedLocation = loc.location;
                std::cout << "Location '" << loc.name << "' loaded!\n";
              }
          }
        ImGui::EndPopup();
      }
    
    // save button
    ImGui::SameLine();
    ImGui::Button("Save##loc");
    
    // save menu
    if(ImGui::BeginPopupContextItem("savePopup", ImGuiMouseButton_Left))
      {
        // text input for new save
        ImGui::Text("New");
        ImGui::SameLine();
        ImGui::InputText("##saveInput", mSavedName, LOCATION_NAME_BUFLEN);
        bool enter = ImGui::IsKeyPressed(ImGui::GetIO().KeyMap[ImGuiKey_Enter]);
        ImGui::SameLine();
        enter |= ImGui::Button("Save##loc2");
        
        if(enter)
          {
            if(!save(std::string(mSavedName)))
              { std::cout << "Failed to save location as '" << mSavedName << "'!\n"; }
            else
              {
                std::cout << "Location saved as '" << mSavedName << "'!\n";
                sprintf(mName, "%s", mSavedName);
              }
            ImGui::CloseCurrentPopup();
          }

        ImGui::Spacing();
        //ImGui::Separator();
        
        // display existing locations
        std::vector<LocationSave> loaded = loadAll();
        for(auto &loc : loaded)
          {
            // delete button (X)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
            if(ImGui::Button(("X##"+loc.name).c_str()))
              {
                std::cout << "Removing location '" << loc.name << "'!\n";
                remove(loc.name);
                if(mName == loc.name) { mName[0] = '\0'; } // (mName == "")
              }
            ImGui::PopStyleColor();
            
            // overwrite
            ImGui::SameLine();
            if(ImGui::MenuItem(loc.name.c_str()))
              {
                if(!save(loc.name))
                  { std::cout << "Failed to save location '" << loc.name << "'!\n"; }
                else
                  {
                    sprintf(mName, "%s", loc.name.c_str());
                    std::cout << "Location saved as '" << mName << "'!\n";
                  }
              }
            
          }
        ImGui::EndPopup();
      }
    else // fill buffer with current name
      { sprintf(mSavedName, "%s", mName); }

    // reload button
    if(!std::string(mName).empty())
      {
        ImGui::SameLine();
        if(ImGui::Button("Reload##loc"))
          { mLocation = mSavedLocation; }
      } 
    ImGui::Spacing();
    //ImGui::Separator();
    ImGui::Spacing();
    
    // time zone
    double utcOffset = mLocation.utcOffset + (mDST ? 1 : 0);
    std::string offsetStr = (std::string("(UTC")+(utcOffset >= 0.0 ? "+" : "")+to_string(utcOffset, 1)+")");
    ImGui::TextColored(Vec4f(1.0f, 1.0f, 1.0f, 0.25f), "%s", (mLocation.timezoneId.empty() ? "n/a" : mLocation.timezoneId).c_str());
    ImGui::SameLine(); ImGui::TextColored(Vec4f(1.0f, 1.0f, 1.0f, 0.25f), offsetStr.c_str());
    // update (NOTE: use springly for now --> ~2500 free queries per username per day)
    if(ImGui::Button("Update")) { mLocation.updateTimezone(); }
    ImGui::SameLine(); ImGui::Checkbox("Daylight Savings", &mDST);
    ImGui::Spacing();
  }
  ImGui::EndGroup();
  mLocation.fix();
}
