#include "timeNode.hpp"
using namespace astro;

#include <cstdlib>

#include "imgui.h"

#include "nodeGraph.hpp"
#include "setting.hpp"
#include "locationNode.hpp"
#include "timeWidget.hpp"
#include "viewSettings.hpp"



//// TIME NODE ////

TimeNode::TimeNode()
  : TimeNode(DateTime::now())
{ }

TimeNode::TimeNode(const DateTime &dt)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Time Node"), mWidget(new TimeWidget(dt))
{
  mSettings.push_back(new Setting<DateTime>   ("Date",       "date",      &mWidget->get()));
  mSettings.push_back(new Setting<DateTime>   ("Saved Date", "savedDate", &mWidget->getSaved()));
  mSettings.push_back(new Setting<std::string>("Saved Name", "savedName", &mWidget->getName()));
  mSettings.push_back(new Setting<bool>       ("Live",       "live",      &mLiveMode));
  outputs()[TIMENODE_OUTPUT_DATE]->set(&mWidget->get());
  setTitle("Time");
}
TimeNode::~TimeNode()
{
  if(mWidget) { delete mWidget; }
}
void TimeNode::onUpdate()
{
  DateTime  dt    = mWidget->get();    
  Location *locIn = inputs()[TIMENODE_INPUT_LOCATION]->get<Location>();
  if(locIn)
    {
      double offset = locIn->getTimezoneOffset(dt); // (sets utcOffset and dstOffset)
      mWidget->set(dt);
    }
  
  if(mLiveMode)
    {
      mWidget->set(DateTime::now());
      mWidget->setName("");
    }
}

void TimeNode::onDraw()
{
  mWidget->setGraph(mGraph);
  float scale = mGraph->getScale();
  DateTime dt = mWidget->get();

  ImGui::BeginGroup();
  std::string dStr = dt.toString(true, false);
  std::string tStr = dt.toString(false, true);
  ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(dStr.c_str());
  ImGui::SameLine(260*scale-ImGui::CalcTextSize(tStr.c_str()).x);
  ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted(tStr.c_str());
  ImGui::EndGroup();

  ImGui::SameLine(276*scale);
  if(ImGui::Button(mLiveMode ? "PAUSE" : "LIVE"))
    { mLiveMode = !mLiveMode; }

  if(mLiveMode) // live time
    {
      ImGui::BeginGroup();
      ImGui::SetCursorPos(Vec2f(ImGui::GetCursorPos()) + Vec2f(276*scale, 0));
      ImGui::TextColored(Vec4f(1.0f, 0.0f, 0.0f, 1.0f), "%s", "[LIVE]");
      ImGui::EndGroup();
    }
  else
   { mActive = mWidget->draw("", scale, isBlocked()); }
}




//// TIME SHIFT NODE ////
TimeShiftNode::TimeShiftNode()
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Time Shift Node"),
    mDateTime(DateTime::now())
{
  mSettings.push_back(new Setting<int>   ("Years",   "years",   &mYears));
  mSettings.push_back(new Setting<int>   ("Months",  "months",  &mMonths));
  mSettings.push_back(new Setting<int>   ("Days",    "days",    &mDays));
  mSettings.push_back(new Setting<int>   ("Hours",   "hours",   &mHours));
  mSettings.push_back(new Setting<int>   ("Minutes", "minutes", &mMinutes));
  mSettings.push_back(new Setting<double>("Seconds", "seconds", &mSeconds));
  outputs()[TIMENODE_OUTPUT_DATE]->set(&mDateTime);
  setTitle("Shift");
}

TimeShiftNode::~TimeShiftNode()
{ }

void TimeShiftNode::onUpdate()
{
  DateTime *dtIn = inputs()[TIMENODE_INPUT_LOCATION]->get<DateTime>();
  if(dtIn)
    {
      mDateTime = *dtIn;
      mDateTime.setYear(mDateTime.year() + mYears);
      mDateTime.setMonth(mDateTime.month() + mMonths);
      mDateTime.setDay(mDateTime.day() + mDays);
      mDateTime.setHour(mDateTime.hour() + mHours);
      mDateTime.setMinute(mDateTime.minute() + mMinutes);
      mDateTime.setSecond(mDateTime.second() + mSeconds);
      mDateTime.fix();
    }
}

void TimeShiftNode::onDraw()
{
  float scale = mGraph->getScale();

  float inputW = 100.0f;
  ImGui::BeginGroup();
  {
    // ImGui::PushFont(getViewSettings()->titleFont);
    // ImGui::TextUnformatted("Shift");
    // ImGui::PopFont();
    // ImGui::Separator();

    bool edited = (ImGui::IsKeyPressed(GLFW_KEY_ENTER) || ImGui::IsKeyPressed(GLFW_KEY_TAB) ||
                   ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Left));
    int years = mYears; int months = mMonths; int days = mDays; int hours = mHours; int minutes = mMinutes; double seconds = mSeconds;
    ImGui::Indent();
    ImGui::TextUnformatted("Years   ");
    ImGui::SetNextItemWidth(inputW*scale);
    ImGui::SameLine(); if(ImGui::InputInt("##years",   &years,   1,   10) && edited) { mYears = years; }
    ImGui::TextUnformatted("Months  ");
    ImGui::SetNextItemWidth(inputW*scale);
    ImGui::SameLine(); if(ImGui::InputInt("##months",  &months,  1,   3) && edited)  { mMonths = months; }
    ImGui::TextUnformatted("Days    ");
    ImGui::SetNextItemWidth(inputW*scale);
    ImGui::SameLine(); if(ImGui::InputInt("##days",    &days,    1,   7) && edited)  { mDays = days; }
    ImGui::TextUnformatted("Hours   ");
    ImGui::SetNextItemWidth(inputW*scale);
    ImGui::SameLine(); if(ImGui::InputInt("##hours",   &hours,   1,   6) && edited)  { mHours = hours; }
    ImGui::TextUnformatted("Minutes ");
    ImGui::SetNextItemWidth(inputW*scale);
    ImGui::SameLine(); if(ImGui::InputInt("##minutes", &minutes, 1,   15) && edited)  { mMinutes = minutes; }
    ImGui::TextUnformatted("Seconds ");
    ImGui::SetNextItemWidth(inputW*scale);
    ImGui::SameLine(); if(ImGui::InputDouble("##seconds", &seconds, 1.0, 5.0, "%.2f") && edited) { mSeconds = seconds; }
    ImGui::Unindent();
  }
  ImGui::EndGroup();
}









//// TIME SPAN NODE ////
const std::vector<std::string> TimeSpanNode::SPEED_UNITS = {{"secs", "mins", "hours", "days", "years"}};
const std::vector<double>      TimeSpanNode::SPEED_MULTS = {{1.0f, 60.0, 60.0*60.0, 60.0*60.0*24.0, 60.0*60.0*24.0*365.0}};

TimeSpanNode::TimeSpanNode()
  : TimeSpanNode(DateTime::now(), DateTime::now())
{ }

TimeSpanNode::TimeSpanNode(const DateTime &dtStart, const DateTime &dtEnd)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Time Span Node"),
    mStartWidget(new TimeWidget(dtStart)), mEndWidget(new TimeWidget(dtEnd)), mDate(dtStart)
{
  // setMinSize(Vec2f(512, 512));
  mSettings.push_back(new Setting<std::string>("Start Saved Name", "startSavedName", &mStartWidget->getName()));
  mSettings.push_back(new Setting<DateTime>   ("Start Date",       "startDate",      &mStartWidget->get()));
  mSettings.push_back(new Setting<std::string>("End Saved Name",   "endSavedName",   &mEndWidget->getName()));
  mSettings.push_back(new Setting<DateTime>   ("End Date",         "endDate",        &mEndWidget->get()));
  mSettings.push_back(new Setting<DateTime>   ("Current Date",     "currentDate",    &mDate));
  mSettings.push_back(new Setting<double>     ("Speed",            "speed",          &mSpeed));
  mSettings.push_back(new Setting<double>     ("Noise",            "noise",          &mNoise));
  mSettings.push_back(new Setting<int>        ("Units",            "units",          &mUnitIndex));
  outputs()[TIMESPANNODE_OUTPUT_DATE]->set(&mDate);
  setTitle("Time Span");
}

TimeSpanNode::~TimeSpanNode()
{
  if(mStartWidget) { delete mStartWidget; }
  if(mEndWidget)   { delete mEndWidget; }
}

void TimeSpanNode::onUpdate()
{
  bool startConnected = false;
  bool endConnected = false;
  DateTime dtStart = mStartWidget->get();
  DateTime dtEnd   = mEndWidget->get();
  if(inputs()[TIMESPANNODE_INPUT_STARTDATE]->get<DateTime>())
    {
      startConnected = true;
      dtStart = *inputs()[TIMESPANNODE_INPUT_STARTDATE]->get<DateTime>();
    }
  if(inputs()[TIMESPANNODE_INPUT_ENDDATE]->get<DateTime>())
    {
      endConnected = true;
      dtEnd = *inputs()[TIMESPANNODE_INPUT_ENDDATE]->get<DateTime>();
    }
  
  if(mPlay)
    { // step date
      auto tNow = TICK_CLOCK::now();
      auto tDiff = std::chrono::duration_cast<std::chrono::microseconds>(tNow - mTLast);
      double dt = (double)tDiff.count() / 1000000.0; // time since last frame in seconds
      
      double speedSeconds = mSpeed * SPEED_MULTS[mUnitIndex]; // convert speed from days/realSecond to seconds/realSecond
      double randMult = (rand()/(double)RAND_MAX)*2.0 - 1.0;
      randMult *= SPEED_MULTS[mUnitIndex];
      
      mDate.setSecond(mDate.second() + dt*speedSeconds + dt*mNoise*randMult);
      mDate.fix();
      mTLast = tNow;
    }
  if(mLoop)
    { // loop range
      if(mDate > dtEnd)
        {
          double offset = mDate.diffDays(dtEnd);
          mDate = dtStart;
          mDate.setDay(mDate.day() + offset); mDate.fix();
        }
      else if(mDate < dtStart)
        {
          double offset = dtStart.diffDays(mDate);
          mDate = dtEnd;
          mDate.setDay(mDate.day() - offset); mDate.fix();
        }
    }
  else
    { // clamp date to range
      if(mDate > dtEnd)
        {
          mDate = dtEnd;
          if(mSpeed > 0.0) { mPlay = false; }
        }
      else if(mDate < dtStart)
        {
          mDate = dtStart;
          if(mSpeed < 0.0) { mPlay = false; }
        }
    }
}

void TimeSpanNode::onDraw()
{
  mStartWidget->setGraph(mGraph);
  mEndWidget->setGraph(mGraph);
  float scale = mGraph->getScale();
  
  bool startConnected = false;
  bool endConnected = false;
  DateTime dtStart = mStartWidget->get();
  DateTime dtEnd   = mEndWidget->get();
  if(inputs()[TIMESPANNODE_INPUT_STARTDATE]->get<DateTime>())
    {
      startConnected = true;
      dtStart = *inputs()[TIMESPANNODE_INPUT_STARTDATE]->get<DateTime>();
    }
  if(inputs()[TIMESPANNODE_INPUT_ENDDATE]->get<DateTime>())
    {
      endConnected = true;
      dtEnd = *inputs()[TIMESPANNODE_INPUT_ENDDATE]->get<DateTime>();
    }
  ImGui::TextUnformatted(mDate.toString().c_str());

  // controls
  ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Speed: ");
  ImGui::SetNextItemWidth(150*scale);
  ImGui::SameLine(); ImGui::InputDouble("##speed", &mSpeed, 1.0, 10.0, "%.8f");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(70*scale);
  if(ImGui::BeginCombo("##speedUnits", SPEED_UNITS[mUnitIndex].c_str()))
    {
      for(int i = 0; i < SPEED_UNITS.size(); i++)
        {
          if(ImGui::Selectable(SPEED_UNITS[i].c_str())) { mUnitIndex = i; }
        }
      ImGui::EndCombo();
    }
  ImGui::SameLine(); ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("per second");
  
  ImGui::AlignTextToFramePadding(); ImGui::TextUnformatted("Noise: ");
  ImGui::SetNextItemWidth(150*scale);
  ImGui::SameLine(); ImGui::InputDouble("##noise", &mNoise, 1.0, 10.0, "%.8f");

  // play button
  ImGui::SameLine();
  if(ImGui::Button((mPlay ? "Pause" : "Play")))
    {
      if(mDate >= dtEnd)
        { mDate = dtStart; }
      
      mPlay = !mPlay;
      mTLast = TICK_CLOCK::now();
    }
  // arrow buttons
  ImGui::PushButtonRepeat(true);
  {
    ImGui::SameLine();
    if(ImGui::Button("<"))
      {
        mPlay  = false;
        mDate.setSecond(mDate.second() - mSpeed*SPEED_MULTS[mUnitIndex]);
        mDate.fix();
      }
    ImGui::SameLine();
    if(ImGui::Button(">"))
      {
        mPlay  = false;
        mDate.setSecond(mDate.second() + mSpeed*SPEED_MULTS[mUnitIndex]);
        mDate.fix();
      }
  }
  ImGui::PopButtonRepeat();
  // reset button
  ImGui::SameLine();
  if(ImGui::Button("Reset"))
    {
      mPlay  = false;
      mDate  = dtStart;
    }

  ImGui::TextUnformatted("Loop "); ImGui::SameLine();
  ImGui::Checkbox("##loop", &mLoop);
  
  ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
  ImGui::TextUnformatted("Start Date:");
  ImGui::Spacing();
  if(!startConnected)
    { mStartWidget->draw("start", scale, isBlocked()); }
  else
    { ImGui::Text("  %s", dtStart.toString().c_str()); }
  
  ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
  ImGui::TextUnformatted("End Date:");
  ImGui::Spacing();
  if(!endConnected)
    { mEndWidget->draw("end", scale, isBlocked()); }
  else
    { ImGui::Text("  %s", dtEnd.toString().c_str()); }
}
