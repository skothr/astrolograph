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

    virtual bool onDraw() override;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      if(!inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>())
        { params.emplace("date", mChart->date().toSaveString()); }
      if(!inputs()[CHARTNODE_INPUT_LOCATION]->get<DateTime>())
        { params.emplace("location", mChart->location().toSaveString()); }
      return params;
    };
    
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      auto iter = params.find("date");
      if(iter != params.end()) { mChart->setDate(DateTime(iter->second)); }
      iter = params.find("location");
      if(iter != params.end()) { mChart->setLocation(Location(iter->second)); }
      mChart->update();
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
          // set date and location if inputs not conencted
          if(!inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>())     { ((ChartNode*)other)->mChart->setDate(mChart->date()); }
          else                                                     { ((ChartNode*)other)->mChart->setDate(*inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>()); }
          if(!inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>()) { ((ChartNode*)other)->mChart->setLocation(mChart->location()); }
          else                                                     { ((ChartNode*)other)->mChart->setLocation(*inputs()[CHARTNODE_INPUT_LOCATION]->get<Location>()); }
          // (everything else set by connections)
          return true;
        }
      else { return false; }
    }

    
    Chart* chart() { return mChart; }
  };
}

#endif // CHART_NODE_HPP
