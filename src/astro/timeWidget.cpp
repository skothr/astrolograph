#include "timeWidget.hpp"
using namespace astro;

#include <fstream>
#include <nfd.h>
#include "imgui.h"
#include "tools.hpp"
#include "mainWindow.hpp"
#include "nodeGraph.hpp"
#include "fileDialog.hpp"

TimeWidget::TimeWidget()
  : mDate(DateTime::now()), mFileDialog(new FileDialog())
{ }
TimeWidget::TimeWidget(const DateTime &date)
  : mDate(date), mFileDialog(new FileDialog())
{ }

TimeWidget::TimeWidget(const TimeWidget &other)
  : mFileDialog(new FileDialog()), mDate(other.mDate), mSavedDate(other.mSavedDate), 
    mGraph(other.mGraph), mName(other.mName)
{ }
TimeWidget& TimeWidget::operator=(const TimeWidget &other)
{
  mGraph     = other.mGraph;
  mDate      = other.mDate;
  mSavedDate = other.mSavedDate;
  mName      = other.mName;
  return *this;
}

TimeWidget::~TimeWidget()
{
  if(mFileDialog) { delete mFileDialog; }
}

bool TimeWidget::saveDirCheck()
{
  if(!directoryExists(DATE_SAVE_DIR))
    { // make sure save directory exists
      std::cout << "Creating save directory (" << DATE_SAVE_DIR << ")...\n";
      if(!makeDirectory(DATE_SAVE_DIR))
        { std::cout << "ERROR: Could not create date save directory.\n"; return false; }
    }
  return true;
}

bool TimeWidget::checkFileDialog()
{
  bool success = false;
  
  #ifndef __NVCC__ // not needed for building CUDA files (std::quoted undefined)
  if(!saveDirCheck()) { return false; }
  if(!mGraph) { std::cout << "TimeWidget->mNodeGraph is null!\n"; return false; }
  //std::cout << "TimeWidget checking FileDialog...\n";
  if(mFileDialog->check())
    {
      std::string path = mFileDialog->getPath();
      if(!path.empty())
        {
          // LOADING
          if(mFileDialog->getType() == DIALOG_LOAD)
            {
              std::cout << "Loading date file --> " << path << "\n";
              if(fileExists(path))
                {
                  std::ifstream dateFile(path, std::ifstream::in);
                  std::string line  = "";
                  while(std::getline(dateFile, line))
                    {

                      std::string n = popName(line);
                      DateTime dt(line);
                      if(!n.empty())
                        {
                          mDate      = dt;
                          mSavedDate = dt;
                          mName      = n;
                          std::cout << "Date loading complete --> " << mDate << " || " << mSavedDate << "\n";
                          success = true;
                          break; // only read first date in file
                        }
                    }
                  if(!success) { std::cout << "Could not find valid date in file!\n"; }
                }
              else { std::cout << "File does not exist!\n"; }
            }
          // SAVING
          else if(mFileDialog->getType() == DIALOG_SAVE)
            {
              std::cout << "Saving date file --> " << path << "\n";
              mName = getBaseName(path);
              mSavedDate = mDate;
              std::ofstream dateFile(path, std::ios::out);
              dateFile << std::quoted(mName) << " " << mDate << "\n";
              std::cout << std::quoted(mName) << " " << mDate << "\n";
              std::cout << "Date saving complete." << "\n";
              success = true;
            }
        }
      else { std::cout << "Empty path string!\n"; }
    }
  #endif // __NVCC__
  return success;
}


bool TimeWidget::draw(const std::string &id, float scale, bool blocked)
{
  mFileDialog->setGraph(mGraph);
  bool popupActive = false;
  ImGui::BeginGroup();
  {
    // textbox widths
    const float yearWidth   = 90*scale;
    const float monthWidth  = 90*scale;
    const float dayWidth    = 90*scale;
    const float hourWidth   = 90*scale;
    const float minuteWidth = 90*scale;
    const float secondWidth = 90*scale;
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
    double tzVal     = mDate.utcOffset() + mDate.dstOffset();
    // whether changed from loaded date
    bool compare    = (!mName.empty());
    bool yearDiff   = compare && (yearVal   != mSavedDate.year());
    bool monthDiff  = compare && (monthVal  != mSavedDate.month());
    bool dayDiff    = compare && (dayVal    != mSavedDate.day());
    bool hourDiff   = compare && (hourVal   != mSavedDate.hour());
    bool minuteDiff = compare && (minuteVal != mSavedDate.minute());
    bool secondDiff = compare && (secondVal != mSavedDate.second());
    bool utcDiff    = compare && (mDate.utcOffset() != mSavedDate.utcOffset());
    bool dstDiff    = compare && (mDate.dstOffset() != mSavedDate.dstOffset());
    bool anyDiff    = compare && (mDate != mSavedDate);

    Vec2f fPad = ImGui::GetStyle().FramePadding;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, Vec2f(1,1));

    ImGuiInputTextFlags flags = ImGuiInputTextFlags_None;
    ImGuiIO &io = ImGui::GetIO();

    bool edited = (ImGui::IsKeyPressed(GLFW_KEY_ENTER) || ImGui::IsKeyPressed(GLFW_KEY_TAB) ||
                   ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Left));
    
    Vec2f tl, br;
    Vec4f changedColor = Vec4f(1.0f, 0.3f, 0.3f, 1.0f);
    float changedW = 1.0f*scale;
    ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos())+fPad+Vec2f(changedW/2.0f, 0.0f));
    ImGui::BeginGroup();
    {
      if(yearDiff)
        {
          tl = Vec2f(ImGui::GetCursorScreenPos()) - fPad;
          br = tl + ImGui::CalcTextSize("Year") + 2.0f*fPad;
          ImGui::GetWindowDrawList()->AddRect(tl, br, ImColor(changedColor), 0.0f, ImDrawCornerFlags_All, changedW);
        }
      ImGui::AlignTextToFramePadding(); ImGui::Text("Year");
      ImGui::PushItemWidth(yearWidth);
      ImGui::SameLine((55+changedW)*scale);
      if(ImGui::InputScalar(("##year"+id).c_str(),   ImGuiDataType_S16,    &yearVal,   &yearStep,   &yearFastStep, "%d", flags) && edited)
        { mDate.setYear(yearVal); }
      
      ImGui::PopItemWidth();
      
      if(monthDiff)
        {
          tl = Vec2f(ImGui::GetCursorScreenPos()) - fPad;
          br = tl + ImGui::CalcTextSize("Month") + 2.0f*fPad;
          ImGui::GetWindowDrawList()->AddRect(tl, br, ImColor(changedColor), 0.0f, ImDrawCornerFlags_All, changedW);
        }
      ImGui::AlignTextToFramePadding(); ImGui::Text("Month");
      ImGui::PushItemWidth(monthWidth);
      ImGui::SameLine((55+changedW)*scale);
      if(ImGui::InputScalar(("##month"+id).c_str(),  ImGuiDataType_S8,     &monthVal,  &monthStep,  &monthFastStep, "%d", flags) && edited)
        { mDate.setMonth(monthVal); }
      ImGui::PopItemWidth();

      if(dayDiff)
        {
          tl = Vec2f(ImGui::GetCursorScreenPos()) - fPad;
          br = tl + ImGui::CalcTextSize("Day") + 2.0f*fPad;
          ImGui::GetWindowDrawList()->AddRect(tl, br, ImColor(changedColor), 0.0f, ImDrawCornerFlags_All, changedW);
        }
      ImGui::AlignTextToFramePadding(); ImGui::Text("Day");
      ImGui::PushItemWidth(dayWidth);
      ImGui::SameLine((55+changedW)*scale);
      if(ImGui::InputScalar(("##day"+id).c_str(), ImGuiDataType_S8,     &dayVal,    &dayStep,    &dayFastStep, "%d", flags) && edited)
        { mDate.setDay(dayVal); }
      ImGui::PopItemWidth();
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    {
      if(hourDiff)
        {
          tl = Vec2f(ImGui::GetCursorScreenPos()) - fPad;
          br = tl + ImGui::CalcTextSize("Hour") + 2.0f*fPad;
          ImGui::GetWindowDrawList()->AddRect(tl, br, ImColor(changedColor), 0.0f, ImDrawCornerFlags_All, changedW);
        }
      ImGui::AlignTextToFramePadding(); ImGui::Text("Hour");
      ImGui::PushItemWidth(hourWidth);
      ImGui::SameLine(55*scale);
      if(ImGui::InputScalar(("##hour"+id).c_str(),   ImGuiDataType_S8,     &hourVal,   &hourStep,   &hourFastStep, "%d", flags) && edited)
        { mDate.setHour(hourVal); }
      ImGui::PopItemWidth();

      ImGui::AlignTextToFramePadding();
      if(minuteDiff)
        {
          tl = Vec2f(ImGui::GetCursorScreenPos()) - fPad;
          br = tl + ImGui::CalcTextSize("Minute") + 2.0f*fPad;
          ImGui::GetWindowDrawList()->AddRect(tl, br, ImColor(changedColor), 0.0f, ImDrawCornerFlags_All, changedW);
        }
      ImGui::AlignTextToFramePadding(); ImGui::Text("Minute");
      ImGui::PushItemWidth(minuteWidth);
      ImGui::SameLine(55*scale);
      if(ImGui::InputScalar(("##minute"+id).c_str(), ImGuiDataType_S8,     &minuteVal, &minuteStep, &minuteFastStep, "%d", flags) && edited)
        { mDate.setMinute(minuteVal); }
      ImGui::PopItemWidth();

      if(secondDiff)
        {
          tl = Vec2f(ImGui::GetCursorScreenPos()) - fPad;
          br = tl + ImGui::CalcTextSize("Second") + 2.0f*fPad;
          ImGui::GetWindowDrawList()->AddRect(tl, br, ImColor(changedColor), 0.0f, ImDrawCornerFlags_All, changedW);
        }
      ImGui::AlignTextToFramePadding(); ImGui::Text("Second");
      ImGui::PushItemWidth(secondWidth);
      ImGui::SameLine(55*scale);
      if(ImGui::InputScalar(("##second"+id).c_str(), ImGuiDataType_Double, &secondVal, &secondStep, &secondFastStep, "%2.2f", flags) && edited)
        { mDate.setSecond(secondVal); }
      ImGui::PopItemWidth();
    }
    ImGui::EndGroup();
    
    // display loaded name
    ImGui::Spacing();
    if(mName.empty())
      {
        ImGui::TextColored(ImColor(1.0f, 1.0f, 1.0f, 0.25f), "[]");
      }
    else
      {
        std::string sName = mName;
        if(mDate != mSavedDate) { sName = std::string("[") + mName + "]"; }
        ImGui::TextColored(ImColor(1.0f, 1.0f, 1.0f, 0.5f), "%s", sName.c_str());
      }

    // display UTC offset
    ImGui::SameLine(165*scale);
    if(utcDiff)
      {
        tl = Vec2f(ImGui::GetCursorScreenPos()) - fPad;
        br = tl + ImGui::CalcTextSize("UTC") + 2.0f*fPad;
        ImGui::GetWindowDrawList()->AddRect(tl, br, ImColor(changedColor), 0.0f, ImDrawCornerFlags_All, changedW);
      }
    ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("UTC");
    ImGui::PushItemWidth(tzWidth);
    ImGui::SameLine();
    ImGui::InputDouble(("##tzOffset"+id).c_str(), &tzVal, 0.0, 0.0, "%+2.2f", flags);
    ImGui::PopItemWidth();
    mDate.setUtcOffset(tzVal - mDate.dstOffset());

    bool dst = (mDate.dstOffset() != 0.0);
    ImGui::SameLine();
    if(dstDiff)
      {
        tl = Vec2f(ImGui::GetCursorScreenPos()) - fPad;
        br = tl + ImGui::CalcTextSize("DST") + 2.0f*fPad;
        ImGui::GetWindowDrawList()->AddRect(tl, br, ImColor(changedColor), 0.0f, ImDrawCornerFlags_All, changedW);
      }
    ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("DST");
    ImGui::SameLine(); ImGui::Checkbox("##DST", &dst);
    
    if(dst && mDate.dstOffset() == 0.0) { mDate.setDstOffset(1.0); }
    else if(!dst)                       { mDate.setDstOffset(0.0); }

    ImGui::PopStyleVar(); // FramePadding
    
    // load button
    if(ImGui::Button(("Load##date"+id).c_str()))
      { mFileDialog->open("Load Date File", DIALOG_LOAD, DATE_SAVE_DIR, {".date"}); }
    // save button
    ImGui::SameLine();
    if(ImGui::Button(("Save##date"+id).c_str()))
      { mFileDialog->open("Save Date File", DIALOG_SAVE, DATE_SAVE_DIR, {".date"}); }

    if(checkFileDialog()) { std::cout << "File dialog success!\n"; } // update dialog, and apply save/load if it succeeds
    
    if(!mName.empty())
      {
        ImGui::SameLine();
        if(ImGui::Button(("Reload##date"+id).c_str()))
          {
            if(mDate != mSavedDate)
              {
                mDate = mSavedDate;
                std::cout << "Re-loaded date '" << mName << "'!\n";
              }
          }
      }
  }
  ImGui::EndGroup();
  mDate.fix();
  return popupActive;
}
