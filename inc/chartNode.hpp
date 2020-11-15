#ifndef CHART_NODE_HPP
#define CHART_NODE_HPP

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"

namespace astro
{
  //// node connector indices ////
  // inputs
#define CHARTNODE_INPUT_DATE        0
#define CHARTNODE_INPUT_LOCATION    1
  // outputs
#define CHARTNODE_OUTPUT_CHART      0
  ////////////////////////////////

  class ChartNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<DateTime>("Date"), new Connector<Location>("Location")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {new Connector<Chart>("Chart")}; }
    
    Chart *mChart = nullptr;
    bool   mOptionsOpen = false;

    virtual bool onDraw() override;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      if(!inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>())
        { params.emplace("date", mChart->date().toSaveString()); }
      if(!inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>())
        { params.emplace("location", mChart->location().toSaveString()); }
      params.emplace("houseSystem", getHouseSystemName(mChart->getHouseSystem()));
      params.emplace("sidereal",    (mChart->getSidereal() ? "1" : "0"));
      params.emplace("truePos",     (mChart->getTruePos()  ? "1" : "0"));
      params.emplace("draconic",    (mChart->getDraconic() ? "1" : "0"));
      
      params.emplace("optionsOpen", (mOptionsOpen ? "1" : "0"));
      return params;
    };
    
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      auto iter = params.find("date");   if(iter != params.end()) { mChart->setDate(DateTime(iter->second)); }
      iter = params.find("location");    if(iter != params.end()) { mChart->setLocation(Location(iter->second)); }
      iter = params.find("houseSystem"); if(iter != params.end()) { mChart->setHouseSystem(getHouseSystem(iter->second)); }
      iter = params.find("sidereal");    if(iter != params.end()) { mChart->setSidereal(iter->second != "0"); }
      iter = params.find("truePos");     if(iter != params.end()) { mChart->setTruePos(iter->second  != "0"); }
      iter = params.find("draconic");    if(iter != params.end()) { mChart->setDraconic(iter->second != "0"); }
      mChart->update();
      iter = params.find("optionsOpen"); if(iter != params.end()) { mOptionsOpen = (iter->second != "0"); }
      return params;
    };
    
  public:
    ChartNode(Chart *chart=nullptr);
    ChartNode(const DateTime &dt, const Location &loc);
    ~ChartNode();
    virtual std::string type() const { return "ChartNode"; }
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          // set date and location if inputs not connected
          if(!inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>())
            { ((ChartNode*)other)->mChart->setDate(mChart->date()); }
          else
            { ((ChartNode*)other)->mChart->setDate(*inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>()); }
          if(!inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>())
            { ((ChartNode*)other)->mChart->setLocation(mChart->location()); }
          else
            { ((ChartNode*)other)->mChart->setLocation(*inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>()); }
          
          ((ChartNode*)other)->mChart->setHouseSystem(mChart->getHouseSystem());
          ((ChartNode*)other)->mChart->setSidereal(mChart->getSidereal());
          ((ChartNode*)other)->mChart->setDraconic(mChart->getDraconic());
          ((ChartNode*)other)->mOptionsOpen = mOptionsOpen;
          // (everything else set by connections)
          return true;
        }
      else { return false; }
    }

    
    Chart* chart() { return mChart; }
  };
}

#endif // CHART_NODE_HPP
