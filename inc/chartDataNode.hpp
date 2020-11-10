#ifndef CHART_DATA_NODE_HPP
#define CHART_DATA_NODE_HPP

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"

namespace astro
{
  //// node connector indices ////
  // inputs
#define DATANODE_INPUT_CHART 0
  // outputs
  ////////////////////////////////

  class ChartDataNode : public Node
  {
  private:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<Chart>("Chart")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {}; }
    
    bool mAngOpen = true;
    bool mObjOpen = true;
    bool mOrbOpen = false;

    std::vector<bool> mObjVisible;
    
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
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          ((ChartDataNode*)other)->mAngOpen = mAngOpen;
          ((ChartDataNode*)other)->mObjOpen = mObjOpen;
          ((ChartDataNode*)other)->mOrbOpen = mOrbOpen;
          for(int o = OBJ_SUN; o < OBJ_COUNT; o++) // start with objects visible
            { ((ChartDataNode*)other)->mObjVisible[o] = mObjVisible[o]; }
          for(int a = ANGLE_OFFSET; a < ANGLE_END; a++) // start with angles invisible
            { ((ChartDataNode*)other)->mObjVisible[OBJ_COUNT+a-ANGLE_OFFSET] = mObjVisible[OBJ_COUNT+a-ANGLE_OFFSET]; }
          // (everything else set by connections)
          return true;
        }
      else { return false; }
    }
    
  };
}

#endif // CHART_DATA_NODE_HPP
