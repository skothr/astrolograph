#include "locationWidget.hpp"
using namespace astro;

#include <fstream>
#include <nfd.h>
#include "imgui.h"
#include "tools.hpp"
#include "nodeGraph.hpp"
#include "fileDialog.hpp"

LocationWidget::LocationWidget()
    : mFileDialog(new FileDialog())
{ }
LocationWidget::LocationWidget(const Location &loc)
  : mFileDialog(new FileDialog()), mLocation(loc)
{ }

LocationWidget::LocationWidget(const LocationWidget &other)
  : mFileDialog(new FileDialog()), mLocation(other.mLocation), mSavedLocation(other.mSavedLocation), mName(other.mName)
{ }

LocationWidget::~LocationWidget()
{
  if(mFileDialog) { delete mFileDialog; }
}

LocationWidget& LocationWidget::operator=(const LocationWidget &other)
{
  mLocation = other.mLocation;
  mSavedLocation = other.mSavedLocation;
  mName = other.mName;
  return *this;
}

bool LocationWidget::saveDirCheck()
{
  if(!directoryExists(LOCATION_SAVE_DIR))
    { // make sure save directory exists
      std::cout << "Creating save directory (" << LOCATION_SAVE_DIR << ")...\n";
      if(!makeDirectory(LOCATION_SAVE_DIR))
        { std::cout << "ERROR: Could not create location save directory.\n"; return false; }
    }
  return true;
}

bool LocationWidget::checkFileDialog()
{
  bool success = false;
#ifndef __NVCC__ // not needed for building CUDA files (std::quoted undefined)
  if(!saveDirCheck()) { return false; }
  if(!mGraph) { std::cout << "LocationWidget->mNodeGraph is null!\n"; return false; }
  if(mFileDialog->check())
    {
      std::string path = mFileDialog->getPath();
      if(!path.empty())
        {
          // LOADING
          if(mFileDialog->getType() == DIALOG_LOAD)
            {
              std::cout << "Loading location file --> " << path << "\n";
              if(fileExists(path))
                {
                  std::ifstream locFile(path, std::ifstream::in);
                  std::string line  = "";
                  while(std::getline(locFile, line))
                    {
                      std::string n = popName(line);
                      Location loc(line);
                      if(!n.empty())
                        {
                          mLocation      = loc;
                          mSavedLocation = loc;
                          mName          = n;
                          std::cout << "Location loading complete." << "\n";
                          success = true;
                          break; // only read first location in file
                        }
                    }
                  if(!success) { std::cout << "Could not find valid location in file!\n"; }
                }
              else { std::cout << "File does not exist!\n"; }
            }
          // SAVING
          else if(mFileDialog->getType() == DIALOG_SAVE)
            {
              std::cout << "Saving location file --> " << path << "\n";
              mName = getBaseName(path);
              mSavedLocation = mLocation;
              std::ofstream locFile(path, std::ios::out);
              locFile << std::quoted(mName) << " " << mLocation.toSaveString() << "\n";
              std::cout << std::quoted(mName) << " " << mLocation.toSaveString() << "\n";
              std::cout << "Location saving complete." << "\n";
              success = true;
            }
        }
      else { std::cout << "Empty path string!\n"; }
    }
#endif // __NVCC__
  return success;
}

void LocationWidget::update()
{
  
}

void LocationWidget::draw(float scale, bool blocked)
{
  mFileDialog->setGraph(mGraph);
  // mLocation.fix();
  // steps
  const double latStep        = 0.01; // degrees
  const double lonStep        = 0.01; // degrees
  const int    altStep        = 10;   // m
  // fast steps
  const double latFastStep    = 0.1; // degrees
  const double lonFastStep    = 0.1; // degrees
  const int    altFastStep    = 100; // m

  double latVal = mLocation.latitude;
  double lonVal = mLocation.longitude;
  int    altVal = mLocation.altitude;

  bool compare = (mName != "");
  bool anyDiff = compare && (mLocation  != mSavedLocation);
  bool latDiff = compare && (mLocation.latitude  != mSavedLocation.latitude);
  bool lonDiff = compare && (mLocation.longitude != mSavedLocation.longitude);
  bool altDiff = compare && (mLocation.altitude  != mSavedLocation.altitude);
    
  Vec2f tl, br;
  Vec4f changedColor = Vec4f(1.0f, 0.3f, 0.3f, 1.0f); // color for red changed rect
  float changedW = 1.0f*scale;  // line width for red changed rect
  Vec2f fPad = Vec2f(2,2);      // padding for red changed rect
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(1,1));

  ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos())+fPad+Vec2f(changedW/2.0f, 0.0f));
  ImGui::BeginGroup();
  {
    // latitude input
    if(latDiff)
      {
        tl = Vec2f(ImGui::GetCursorScreenPos()) - fPad;
        br = tl + ImGui::CalcTextSize("Latitude") + 2.0f*fPad;
        ImGui::GetWindowDrawList()->AddRect(tl, br, ImColor(changedColor), 0.0f, ImDrawCornerFlags_All, changedW);
      }
    ImGui::Text("Latitude  (°)");
    ImGui::SameLine();
    ImGui::PushItemWidth(200*scale);
    if(ImGui::InputDouble("##Latitude",  &latVal, latStep, latFastStep, "%.12f"))
      { mLocation.latitude = latVal; }
    ImGui::PopItemWidth();
    // longitude input
    if(lonDiff)
      {
        tl = Vec2f(ImGui::GetCursorScreenPos()) - fPad;
        br = tl + ImGui::CalcTextSize("Longitude") + 2.0f*fPad;
        ImGui::GetWindowDrawList()->AddRect(tl, br, ImColor(changedColor), 0.0f, ImDrawCornerFlags_All, changedW);
      }
    ImGui::Text("Longitude (°)");
    ImGui::SameLine();
    ImGui::PushItemWidth(200*scale);
    if(ImGui::InputDouble("##Longitude", &lonVal, lonStep, lonFastStep, "%.12f"))
      { mLocation.longitude = lonVal; }
    ImGui::PopItemWidth();

    // altitude input
    if(altDiff)
      {
        tl = Vec2f(ImGui::GetCursorScreenPos()) - fPad;
        br = tl + ImGui::CalcTextSize("Altitude") + 2.0f*fPad;
        ImGui::GetWindowDrawList()->AddRect(tl, br, ImColor(changedColor), 0.0f, ImDrawCornerFlags_All, changedW);
      }
    ImGui::Text("Altitude  (m)");
    ImGui::SameLine();
    ImGui::PushItemWidth(100*scale);
    if(ImGui::InputInt("##Altitude",     &altVal, altStep, altFastStep))
      { mLocation.altitude = altVal; }
    ImGui::PopItemWidth();

    // display loaded name
    ImGui::Spacing();
    if(mName.empty())
      { ImGui::TextColored(ImColor(1.0f, 1.0f, 1.0f, 0.25f), "[]"); }
    else
      {
        std::string sName = mName;
        if(mLocation != mSavedLocation)
          { sName = std::string("[") + mName + "]"; }
        ImGui::TextColored(ImColor(1.0f, 1.0f, 1.0f, 0.5f), "%s", sName.c_str());
      }
    
    ImGui::PopStyleVar(); // FramePadding
    
    // time zone
    double utcOffset = mLocation.utcOffset;
    std::string offsetStr = (std::string("(UTC")+(utcOffset >= 0.0 ? "+" : "")+to_string(utcOffset, 1)+")");
    ImGui::TextColored(Vec4f(1.0f, 1.0f, 1.0f, 0.25f), "%s", (mLocation.timezoneId.empty() ? "n/a" : mLocation.timezoneId).c_str());
    ImGui::SameLine(); ImGui::TextColored(Vec4f(1.0f, 1.0f, 1.0f, 0.25f), "%s", offsetStr.c_str());
    
    // load button
    if(ImGui::Button("Load##loc")) { mFileDialog->open("Open Location", DIALOG_LOAD, LOCATION_SAVE_DIR, {".loc"}); }
    // save button
    ImGui::SameLine();
    if(ImGui::Button("Save##loc")) { mFileDialog->open("Save Location", DIALOG_SAVE, LOCATION_SAVE_DIR, {".loc"}); }

    if(checkFileDialog()) { std::cout << "File dialog success!\n"; }
    
    // reload button
    if(!mName.empty())
      {
        ImGui::SameLine();
        if(ImGui::Button("Reload##loc"))
          { mLocation = mSavedLocation; }
      }
    ImGui::SameLine();
    // update button (NOTE: use springly for now --> ~2500 free queries per username per day)
    if(ImGui::Button("Update Timezone")) { mLocation.updateTimezone(); }

    ImGui::SameLine();
    if((!mExpanded && ImGui::Button("Expand")) || (mExpanded && ImGui::Button("Collapse")))
      { mExpanded = !mExpanded; }
    
    if(mExpanded)
      { // draw location on sphere
        ImDrawList *drawList = ImGui::GetWindowDrawList();
        Vec2f sPos = ImGui::GetCursorScreenPos();
        const float geoRad = 128.0f*scale;
        const float padding = 32.0f*scale;
            
        const int nLat = 18;
        const int nLon = 36;

        ImGui::BeginChild("##geoPos", Vec2f(geoRad+padding, geoRad+padding)*2.0f, true);
        {
          drawList->AddRectFilled(sPos, sPos + Vec2f(geoRad+padding, geoRad+padding)*2.0f, ImColor(Vec4f(0.0f, 0.0f, 0.0f, 1.0f)), 0.0f);
          drawList->AddCircle(sPos + Vec2f(geoRad+padding, geoRad+padding), geoRad,        ImColor(Vec4f(1.0f, 1.0f, 1.0f, 1.0f)), 32, 2.0f);

          float offset = fmod(mLocation.longitude, 180.0f/nLon)*M_PI/180.0f;
          for(int i = 0; i < nLon; i++)
            {
              std::vector<Vec2f> points;
              float angle = offset + i/(float)nLon * M_PI;

              float maxCurve = geoRad*sin(angle)*(i < nLon/2 ? -1.0f : 1.0f);
              Vec2f p1       = sPos + Vec2f(geoRad+padding, padding);
              Vec2f p2       = sPos + Vec2f(geoRad+padding, 2.0f*geoRad + padding);
              Vec2f pMid     = (p1+p2)/2.0f + Vec2f(maxCurve, 0.0f);
              for(int j = 0; j < 50; j++)
                {
                  float alpha = j/100.0f;
                  float smoothed = (cos(2.0f*M_PI*alpha)+1.0f)/2.0f;
                  points.push_back(p1*(1.0f-smoothed) + pMid*smoothed);
                }
              for(int j = 0; j < 50; j++)
                {
                  float alpha = (j+50)/100.0f;
                  float smoothed = (cos(2.0f*M_PI*alpha)+1.0f)/2.0f;
                  points.push_back(pMid*(1.0f-smoothed) + p2*smoothed);
                }
              drawList->AddPolyline((const ImVec2*)points.data(), points.size(), ImColor(Vec4f(0.5f, 0.5f, 0.5f, 1.0f)), false, 1.0f);
            }
        }
        ImGui::EndChild();
      }
    
    ImGui::Spacing();
    ImGui::Spacing();
  }
  ImGui::EndGroup();
  mLocation.fix();
}
