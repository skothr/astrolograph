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
    
  public:
    PlotNode();
    ~PlotNode();
    virtual std::string type() const { return "PlotNode"; }
  };
}

#endif // PLOT_NODE_HPP
