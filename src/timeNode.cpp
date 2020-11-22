#include "timeNode.hpp"
using namespace astro;

#include "imgui.h"

#include "locationNode.hpp"
#include "nodeGraph.hpp"


TimeNode::TimeNode()
  : TimeNode(DateTime::now())
{ }

TimeNode::TimeNode(const DateTime &dt)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Time Node"), mWidget(dt)
{
  // setMinSize(Vec2f(512, 512));
  outputs()[TIMENODE_OUTPUT_DATE]->set(&mWidget.get());
}

bool TimeNode::onDraw()
{
  float scale = mGraph->getScale();
  
  // if(outputs()[TIMENODE_OUTPUT_DATE]->needsReset())
  //   {
  //     mWidget.set(mWidget.getSaved());
  //     outputs()[TIMENODE_OUTPUT_DATE]->setReset(false);
  //   }

  DateTime  dt    = mWidget.get();    
  Location *locIn = inputs()[TIMENODE_INPUT_LOCATION]->get<Location>();
  if(locIn)
    {
      double offset = locIn->getTimezoneOffset(dt); // (sets utcOffset and dstOffset)
      mWidget.set(dt);
    }

  ImGui::BeginGroup();
  ImGui::TextUnformatted(dt.toString(true, false).c_str());
  ImGui::TextUnformatted(dt.toString(false, true).c_str());
  ImGui::EndGroup();

  if(mLiveMode) // live time
    {
      ImGui::SameLine(210*scale);
      ImGui::TextColored(Vec4f(1.0f, 0.0f, 0.0f, 1.0f), "%s", "[LIVE]");
      ImGui::SameLine();
    }
  else
    {
      ImGui::SameLine(276*scale);
    }
  if(ImGui::Button(mLiveMode ? "PAUSE" : "LIVE"))
    { mLiveMode = !mLiveMode; }
  
  if(mLiveMode) { mWidget.set(DateTime::now()); mWidget.setName(""); }
  else          { mWidget.draw("", scale, isBlocked()); }
    
  return true;
}


const std::vector<std::string> TimeSpanNode::SPEED_UNITS = {{"secs", "mins", "hours", "days", "years"}};
const std::vector<double>      TimeSpanNode::SPEED_MULTS = {{1.0f, 60.0, 60.0*60.0, 60.0*60.0*24.0, 60.0*60.0*24.0*365.0}};

TimeSpanNode::TimeSpanNode()
  : TimeSpanNode(DateTime::now(), DateTime::now())
{ }

TimeSpanNode::TimeSpanNode(const DateTime &dtStart, const DateTime &dtEnd)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Time Span Node"),
    mStartWidget(dtStart), mEndWidget(dtEnd), mDate(dtStart)
{
  // setMinSize(Vec2f(512, 512));
  outputs()[TIMESPANNODE_OUTPUT_DATE]->set(&mDate);
}

bool TimeSpanNode::onDraw()
{
  float scale = mGraph->getScale();
  
  bool startConnected = false;
  bool endConnected = false;
  DateTime dtStart = mStartWidget.get();
  DateTime dtEnd   = mEndWidget.get();
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
  ImGui::TextUnformatted("Speed: ");
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
  ImGui::SameLine(); ImGui::TextUnformatted("per second");

  // play button
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

  if(mPlay)
    { // step date
      auto tNow = TICK_CLOCK::now();
      auto tDiff = std::chrono::duration_cast<std::chrono::microseconds>(tNow - mTLast);
      double dt = (double)tDiff.count() / 1000000.0; // time since last frame in seconds
      
      double speedSeconds = mSpeed * SPEED_MULTS[mUnitIndex]; // convert speed from days/realSecond to seconds/realSecond
      mDate.setSecond(mDate.second() + speedSeconds*dt);
      mDate.fix();
      mTLast = tNow;
    }
  // clamp date to range
  if(mDate >= dtEnd)
    {
      mDate = dtEnd;
      if(mSpeed > 0.0)
        { mPlay = false; }
    }
  else if(mDate <= dtStart)
    {
      mDate = dtStart;
      if(mSpeed < 0.0)
        { mPlay = false; }
    }

  ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
  ImGui::TextUnformatted("Start Date:");
  ImGui::Spacing();
  if(!startConnected)
    { mStartWidget.draw("start", scale, isBlocked()); }
  else
    { ImGui::Text("  %s", dtStart.toString().c_str()); }
  
  ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
  ImGui::TextUnformatted("End Date:");
  ImGui::Spacing();
  if(!endConnected)
    { mEndWidget.draw("end", scale, isBlocked()); }
  else
    { ImGui::Text("  %s", dtEnd.toString().c_str()); }
  
  return true;
}
