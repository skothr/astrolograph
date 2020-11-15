#ifndef PLOT_NODE_HPP
#define PLOT_NODE_HPP

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"

namespace astro
{
  //// node connector indices ////
  // inputs
#define PLOTNODE_INPUT_CHART      0
// #define PLOTNODE_INPUT_STARTDATE  1
// #define PLOTNODE_INPUT_ENDDATE    2
  // outputs
  ////////////////////////////////

  class PlotNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<Chart>("Chart")}; }//, new Connector<DateTime>("Start Date"), new Connector<DateTime>("End Date")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {}; }

    std::vector<float> mData;
    DateTime mOldStartDate;
    DateTime mOldEndDate;
    int mDayRadius = 30;
    Chart mOldChart;
    
    virtual bool onDraw() override;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      // if(!inputs()[PLOTNODE_INPUT_DATE]->get<DateTime>())
      //   { params.emplace("date", mPlot->date().toSaveString()); }
      // if(!inputs()[PLOTNODE_INPUT_LOCATION]->get<Location>())
      //   { params.emplace("location", mPlot->location().toSaveString()); }
      // params.emplace("houseSystem", getHouseSystemName(mPlot->getHouseSystem()));
      // params.emplace("sidereal",    (mPlot->getSidereal() ? "1" : "0"));
      // params.emplace("truePos",     (mPlot->getTruePos()  ? "1" : "0"));
      // params.emplace("draconic",    (mPlot->getDraconic() ? "1" : "0"));
      
      params.emplace("dayRadius", std::to_string(mDayRadius));
      return params;
    };
    
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      // auto iter = params.find("date");   if(iter != params.end()) { mPlot->setDate(DateTime(iter->second)); }
      // iter = params.find("location");    if(iter != params.end()) { mPlot->setLocation(Location(iter->second)); }
      // iter = params.find("houseSystem"); if(iter != params.end()) { mPlot->setHouseSystem(getHouseSystem(iter->second)); }
      // iter = params.find("sidereal");    if(iter != params.end()) { mPlot->setSidereal(iter->second != "0"); }
      // iter = params.find("truePos");     if(iter != params.end()) { mPlot->setTruePos(iter->second  != "0"); }
      // iter = params.find("draconic");    if(iter != params.end()) { mPlot->setDraconic(iter->second != "0"); }
      // mPlot->update();
      auto iter = params.find("dayRadius"); if(iter != params.end()) { std::stringstream ss(iter->second); ss >> mDayRadius; }
      return params;
    };
    
  public:
    PlotNode();
    ~PlotNode();
    virtual std::string type() const { return "PlotNode"; }
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          // // set date and location if inputs not connected
          // if(!inputs()[PLOTNODE_INPUT_DATE]->get<DateTime>())
          //   { ((PlotNode*)other)->mPlot->setDate(mPlot->date()); }
          // else
          //   { ((PlotNode*)other)->mPlot->setDate(*inputs()[PLOTNODE_INPUT_DATE]->get<DateTime>()); }
          // if(!inputs()[PLOTNODE_INPUT_LOCATION]->get<Location>())
          //   { ((PlotNode*)other)->mPlot->setLocation(mPlot->location()); }
          // else
          //   { ((PlotNode*)other)->mPlot->setLocation(*inputs()[PLOTNODE_INPUT_LOCATION]->get<Location>()); }
          
          // ((PlotNode*)other)->mPlot->setHouseSystem(mPlot->getHouseSystem());
          // ((PlotNode*)other)->mPlot->setSidereal(mPlot->getSidereal());
          // ((PlotNode*)other)->mPlot->setDraconic(mPlot->getDraconic());
          // ((PlotNode*)other)->mOptionsOpen = mOptionsOpen;
          // (everything else set by connections)

          ((PlotNode*)other)->mData = mData;
          ((PlotNode*)other)->mDayRadius = mDayRadius;
          
          return true;
        }
      else { return false; }
    }
  };
}

#endif // PLOT_NODE_HPP
