#include "timeNode.hpp"
using namespace astro;

#include "imgui.h"

// node connector indices
#define DATENODE_OUTPUT_DATE 0


TimeNode::TimeNode()
  : TimeNode(DateTime::now())
{ }

TimeNode::TimeNode(const DateTime &dt)
  : Node({}, {new Connector<DateTime>("DateTime Out")}, "Time Node"), mWidget(dt)
{
  outputs()[DATENODE_OUTPUT_DATE]->set(&mWidget.get());
}

bool TimeNode::onDraw()
{
  if(outputs()[DATENODE_OUTPUT_DATE]->needsReset())
    {
      mWidget.set(mWidget.getSaved());
      outputs()[DATENODE_OUTPUT_DATE]->reset(false);
    }
 
  DateTime dt = mWidget.get(); 
  ImGui::TextUnformatted(dt.toString().c_str());
  ImGui::SameLine();
  if(ImGui::Button(mLiveMode ? "PAUSE" : "LIVE"))
    {
      mLiveMode = !mLiveMode;
      mWidget.setName("");
    }
  
  if(mLiveMode) // live time
    { mWidget.set(DateTime::now()); }
  else
    { mWidget.draw(); }
  
  return true;
}


const std::vector<std::string> TimeSpanNode::SPEED_UNITS = {{"secs", "mins", "hours", "days", "years"}};
const std::vector<double> TimeSpanNode::SPEED_MULTS = {{1.0f, 60.0, 60.0*60.0, 60.0*60.0*24.0, 60.0*60.0*24.0*365.0}};

TimeSpanNode::TimeSpanNode()
  : TimeSpanNode(DateTime::now(), DateTime::now())
{ }

TimeSpanNode::TimeSpanNode(const DateTime &dtStart, const DateTime &dtEnd)
  : Node({}, {new Connector<DateTime>("DateTime Out")}, "Time Span Node"),
    mStartWidget(dtStart), mEndWidget(dtEnd), mDate(dtStart)
{
  outputs()[DATENODE_OUTPUT_DATE]->set(&mDate);
}

bool TimeSpanNode::onDraw()
{
  DateTime dtStart = mStartWidget.get();
  DateTime dtEnd   = mEndWidget.get();
  ImGui::TextUnformatted(mDate.toString().c_str());

  // controls
  ImGui::TextUnformatted("Speed: ");
  ImGui::SetNextItemWidth(150);
  ImGui::SameLine(); ImGui::InputDouble("##speed", &mSpeed, 1.0, 10.0, "%.8f");
  ImGui::SameLine();
  ImGui::SetNextItemWidth(70);
  if(ImGui::BeginCombo("##speedUnits", SPEED_UNITS[mUnitIndex].c_str()))
    {
      for(int i = 0; i < SPEED_UNITS.size(); i++)
        {
          if(ImGui::Selectable(SPEED_UNITS[i].c_str())) { mUnitIndex = i; }
        }
      ImGui::EndCombo();
    }
  ImGui::SameLine(); ImGui::TextUnformatted("per second");
  
  if(ImGui::Button((mPlay ? "Pause" : "Play")))
    {
      mPlay = !mPlay;
      mTLast = TICK_CLOCK::now();
    }
  ImGui::SameLine();
  if(ImGui::Button("Reset"))
    {
      mPlay = false;
      mTLast = TICK_CLOCK::now();
      mDate = mStartWidget.get();
    }

  if(mPlay)
    { // step date
      auto tNow = TICK_CLOCK::now();
      auto tDiff = std::chrono::duration_cast<std::chrono::microseconds>(tNow - mTLast);
      double dt = (double)tDiff.count() / 1000000.0; // time since last frame in seconds
      
      double speedSeconds = mSpeed * SPEED_MULTS[mUnitIndex]; // convert speed from days/realSecond to seconds/realSecond

      std::cout << "SPAN STEPPING " << speedSeconds*dt << " SECONDS...\n";
      mDate += DateTime(0,0,0,0,0, speedSeconds*dt);

      if(mDate >= mEndWidget.get())
        {
          mDate = mEndWidget.get();
          mPlay = false;
        }
      else if(mDate <= mStartWidget.get())
        {
          mDate = mStartWidget.get();
          mPlay = false;
        }
      mTLast = tNow;
    }

  ImGui::Spacing(); //ImGui::Separator();
  ImGui::TextUnformatted("Start Date:");
  ImGui::Spacing();
  mStartWidget.draw("start");
  ImGui::Spacing(); //ImGui::Separator();
  ImGui::TextUnformatted("End Date:");
  ImGui::Spacing();
  mEndWidget.draw("end");
  
  return true;
}
