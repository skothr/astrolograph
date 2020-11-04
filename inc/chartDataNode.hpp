#ifndef CHART_DATA_NODE_HPP
#define CHART_DATA_NODE_HPP

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"

namespace astro
{
  class ChartDataNode : public Node
  {
  private:
    // ChartView mWidget;
    // Chart *mChart = nullptr;

    bool mAngOpen = true;
    bool mObjOpen = true;
    bool mOrbOpen = false;
    
    virtual bool onDraw() override;
    
    // virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    // {
    //   if(!inputs()[CHARTNODE_INPUT_DATE]->get<DateTime>())
    //     { params.emplace("date", mChart->date().toSaveString()); }
    //   if(!inputs()[CHARTNODE_INPUT_LOCATION]->get<DateTime>())
    //     { params.emplace("location", mChart->location().toSaveString()); }
    //   return params;
    // };
    
    // virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    // {
    //   auto iter = params.find("date");
    //   if(iter != params.end()) { mChart->setDate(DateTime(iter->second)); }
    //   iter = params.find("location");
    //   if(iter != params.end()) { mChart->setLocation(Location(iter->second)); }
    //   mChart->update();
    //   return params;
    // };

    
  public:
    ChartDataNode();
    ~ChartDataNode();
    virtual std::string type() const { return "ChartDataNode"; }
  };
}

#endif // CHART_DATA_NODE_HPP
