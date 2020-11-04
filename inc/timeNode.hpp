#ifndef TIME_NODE_HPP
#define TIME_NODE_HPP

#include <chrono>

#include "astro.hpp"
#include "timeWidget.hpp"
#include "node.hpp"


namespace astro
{
  class TimeNode : public Node
  {
  private:
    TimeWidget mWidget;
    bool mLiveMode = false;
    
    virtual bool onDraw() override;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      params.emplace("savedName", mWidget.getName());
      params.emplace("date",      mWidget.get().toSaveString());
      params.emplace("live",      (mLiveMode ? "1" : "0"));
      return params;
    };

    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      std::string saveName = params["savedName"];
      if(saveName.empty())
        {
          mWidget.get().fromSaveString(params["date"]);
          mWidget.setSaved(mWidget.get());
        }
      else
        { mWidget.load(saveName); }
      mLiveMode = (params["live"] != "0");
      return params;
    };
    
  public:
    TimeNode();
    TimeNode(const DateTime &dt);
    virtual std::string type() const { return "TimeNode"; }
    
  };

#define TICK_CLOCK std::chrono::high_resolution_clock

  class TimeSpanNode : public Node
  {
  private:
    static const std::vector<std::string> SPEED_UNITS;
    static const std::vector<double> SPEED_MULTS;

    TimeWidget mStartWidget;
    TimeWidget mEndWidget;
    DateTime mDate; // current date (between start/end)

    TICK_CLOCK::time_point mTLast; // time of last frame
    bool   mPlay   = false;  // if true, date will be moving from start date to end date
    double mSpeed  = 1.0;    // in days per real second

    int mUnitIndex = 2;      // (default: hours)
    
    virtual bool onDraw() override;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      params.emplace("startSavedName", mStartWidget.getName());
      params.emplace("startDate",      mStartWidget.get().toSaveString());
      params.emplace("endSavedName", mEndWidget.getName());
      params.emplace("endDate",      mEndWidget.get().toSaveString());
      params.emplace("currentDate",  mDate.toSaveString());
      return params;
    };

    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      // start date
      mStartWidget.get().fromSaveString(params["startDate"]);
      mStartWidget.setSaved(mStartWidget.get());
      std::string saveName = params["startSavedName"];
      mStartWidget.load(saveName);
      mStartWidget.setName(saveName);
      // end date
      saveName = params["endSavedName"];
      mEndWidget.load(saveName);
      mEndWidget.setName(saveName);
      // current date
      mDate.fromSaveString(params["currentDate"]);
      return params;
    };
    
  public:
    TimeSpanNode();
    TimeSpanNode(const DateTime &dtStart, const DateTime &dtEnd);
    virtual std::string type() const { return "TimeSpanNode"; }
    
  };

}

#endif // TIME_NODE_HPP
