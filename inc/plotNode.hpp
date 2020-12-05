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
    { return {new Connector<Chart>("Chart")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {}; }

    std::vector<float> mData;
    DateTime mOldStartDate;
    DateTime mOldEndDate;
    int      mDayRadius  = 30;
    Chart    mOldChart;
    ObjType  mOldObjType = OBJ_SUN;
    ObjType  mObjType    = OBJ_SUN;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      params.emplace("dayRadius", std::to_string(mDayRadius));
      params.emplace("object",    std::to_string((int)mObjType));
      return params;
    };
    
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      auto iter = params.find("dayRadius"); if(iter != params.end()) { std::stringstream ss(iter->second); ss >> mDayRadius; }
      iter = params.find("object");         if(iter != params.end()) { std::stringstream ss(iter->second); ss >> (int&)mObjType; }
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
          // ((PlotNode*)other)->mData = mData;
          ((PlotNode*)other)->mDayRadius = mDayRadius;
          ((PlotNode*)other)->mObjType = mObjType;
          return true;
        }
      else { return false; }
    }
  };
}

#endif // PLOT_NODE_HPP
