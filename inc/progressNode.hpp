#ifndef PROGRESS_NODE_HPP
#define PROGRESS_NODE_HPP

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"

namespace astro
{
  //// node connector indices ////
  // inputs
#define PROGRESSNODE_INPUT_CHART       0
#define PROGRESSNODE_INPUT_DATE        1
#define PROGRESSNODE_INPUT_LOCATION    2
  // outputs
#define PROGRESSNODE_OUTPUT_CHART      0
  ////////////////////////////////

  class ProgressNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<Chart>("Chart"), new Connector<DateTime>("Date"), new Connector<Location>("Location")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {new Connector<Chart>("Progressed Chart")}; }

    Chart *mChart = nullptr;

    virtual bool onDraw() override;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      if(!inputs()[PROGRESSNODE_INPUT_DATE]->get<DateTime>())
        { params.emplace("date",     mChart->date().toSaveString()); }
      if(!inputs()[PROGRESSNODE_INPUT_LOCATION]->get<DateTime>())
        { params.emplace("location", mChart->location().toSaveString()); }
      return params;
    };
    
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      auto iter = params.find("date");
      if(iter != params.end()) { mChart->setDate(DateTime(iter->second)); }
      iter = params.find("location");
      if(iter != params.end()) { mChart->setLocation(Location(iter->second)); }
      return params;
    };
    
  public:
    ProgressNode();
    ~ProgressNode();
    virtual std::string type() const { return "ProgressNode"; }
    virtual bool copyTo(Node *other) override
    { // (no extra data to copy -- mChart is set based on connections)
      if(Node::copyTo(other)) { return true; }
      else                    { return false; }
    }
  };
}

#endif // PROGRESS_NODE_HPP
