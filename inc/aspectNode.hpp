#ifndef ASPECT_NODE_HPP
#define ASPECT_NODE_HPP

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"

namespace astro
{
  //// node connector indices ////
  // inputs
#define ASPECTNODE_INPUT_CHART 0
  // outputs
  ////////////////////////////////

  class AspectNode : public Node
  {
  private:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<Chart>("Chart")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {}; }

    bool mListOpen = false;
    bool mOrbsOpen = false;
    std::vector<BoolStruct> mAspVisible;
    std::vector<double> mAspOrbs;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
  public:
    AspectNode();
    virtual std::string type() const { return "AspectNode"; }
  };
}

#endif // ASPECT_NODE_HPP
