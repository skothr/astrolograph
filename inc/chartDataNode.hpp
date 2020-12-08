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
    
    bool mAngOpen = false;
    bool mObjOpen = false;
    bool mOrbOpen = false;

    std::vector<bool> mShowObjects;
    std::vector<bool> mFocusObjects;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
  public:
    ChartDataNode();
    ~ChartDataNode();
    virtual std::string type() const { return "ChartDataNode"; }    
  };
}

#endif // CHART_DATA_NODE_HPP
