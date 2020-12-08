#ifndef TIME_NODE_HPP
#define TIME_NODE_HPP

#include <chrono>

#include "astro.hpp"
#include "timeWidget.hpp"
#include "node.hpp"


namespace astro
{
  //// node connector indices ////
  // inputs
#define TIMENODE_INPUT_LOCATION   0
  // outputs
#define TIMENODE_OUTPUT_DATE      0
  ////////////////////////////////
  
  class TimeNode : public Node
  {
  private:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<Location>("Location Input (sets timezone)")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {new Connector<DateTime>("Time Output")}; }
    
    TimeWidget mWidget;
    bool mLiveMode = false;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
  public:
    TimeNode();
    TimeNode(const DateTime &dt);
    virtual std::string type() const { return "TimeNode"; }    
  };

#define TICK_CLOCK std::chrono::high_resolution_clock
  
  //// node connector indices ////
  // inputs
#define TIMESPANNODE_INPUT_STARTDATE 0
#define TIMESPANNODE_INPUT_ENDDATE   1
  // outputs
#define TIMESPANNODE_OUTPUT_DATE     0
  ////////////////////////////////

  class TimeSpanNode : public Node
  {
  private:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<DateTime>("Start Time"), new Connector<DateTime>("End Time")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {new Connector<DateTime>("Time Output")}; }
    
    static const std::vector<std::string> SPEED_UNITS;
    static const std::vector<double>      SPEED_MULTS;

    TimeWidget mStartWidget;
    TimeWidget mEndWidget;
    DateTime   mDate; // current date (between start/end)

    TICK_CLOCK::time_point mTLast; // time of last frame
    bool   mPlay   = false;  // if true, date will be moving from start date to end date
    double mSpeed  = 1.0;    // in days per real second
    int mUnitIndex = 2;      // (default: hours)
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
  public:
    TimeSpanNode();
    TimeSpanNode(const DateTime &dtStart, const DateTime &dtEnd);
    virtual std::string type() const { return "TimeSpanNode"; }
  };

}

#endif // TIME_NODE_HPP
