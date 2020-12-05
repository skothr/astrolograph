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
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      params.emplace("savedName", mWidget.getName());
      params.emplace("date",      mWidget.get().toSaveString());
      params.emplace("live",      (mLiveMode ? "1" : "0"));
      return params;
    };

    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      auto iter = params.find("savedName");
      std::string saveName = ((iter != params.end()) ? iter->second : "");
      if(!saveName.empty()) { mWidget.load(saveName); mWidget.setName(saveName); }
      else { std::cout << "WARNING: Saved name empty!\n"; }

      iter = params.find("date");
      if(iter != params.end())
        { mWidget.get().fromSaveString(iter->second); }
      else { std::cout << "WARNING: Could not find 'date' param!\n"; }
      
      iter = params.find("live");
      if(iter != params.end()) { mLiveMode = (iter->second != "0"); }
      else { std::cout << "WARNING: Could not find 'live' param!\n"; }
      return params;
    };
    
  public:
    TimeNode();
    TimeNode(const DateTime &dt);
    virtual std::string type() const { return "TimeNode"; }
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          ((TimeNode*)other)->mWidget = mWidget;
          ((TimeNode*)other)->mLiveMode = mLiveMode;
          return true;
        }
      else { return false; }
    }
    
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
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      params.emplace("startSavedName", mStartWidget.getName());
      params.emplace("startDate",      mStartWidget.get().toSaveString());
      params.emplace("endSavedName",   mEndWidget.getName());
      params.emplace("endDate",        mEndWidget.get().toSaveString());
      params.emplace("currentDate",    mDate.toSaveString());
      params.emplace("speed",          std::to_string(mSpeed));
      params.emplace("unit",           std::to_string(mUnitIndex));
      return params;
    };

    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      // start date
      auto iter = params.find("startDate");
      if(iter != params.end()) { mStartWidget.get().fromSaveString(iter->second); }
      mStartWidget.setSaved(mStartWidget.get());
      
      iter = params.find("startSavedName");
      if(iter != params.end())
        {
          mStartWidget.load(iter->second);
          mStartWidget.setName(iter->second);
        }
      // end date
      iter = params.find("endDate");
      if(iter != params.end()) { mEndWidget.get().fromSaveString(iter->second); }
      mEndWidget.setSaved(mEndWidget.get());
      
      iter = params.find("endSavedName");
      if(iter != params.end())
        {
          mEndWidget.load(iter->second);
          mEndWidget.setName(iter->second);
        }
      // current date
      iter = params.find("currentDate");
      if(iter != params.end()) { mDate.fromSaveString(iter->second); }
      
      // speed
      iter = params.find("speed");
      if(iter != params.end()) { std::stringstream ss(iter->second); ss >> mSpeed; }
      iter = params.find("unit");
      if(iter != params.end()) { std::stringstream ss(iter->second); ss >> mUnitIndex; }
      return params;
    };
    
  public:
    TimeSpanNode();
    TimeSpanNode(const DateTime &dtStart, const DateTime &dtEnd);
    virtual std::string type() const { return "TimeSpanNode"; }
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          ((TimeSpanNode*)other)->mStartWidget = mStartWidget;
          ((TimeSpanNode*)other)->mEndWidget   = mEndWidget;
          ((TimeSpanNode*)other)->mDate        = mDate;
          ((TimeSpanNode*)other)->mPlay        = mPlay;
          ((TimeSpanNode*)other)->mSpeed       = mSpeed;
          ((TimeSpanNode*)other)->mUnitIndex   = mUnitIndex;
          return true;
        }
      else { return false; }
    }
    
  };

}

#endif // TIME_NODE_HPP
