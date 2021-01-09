#ifndef VEDIC_NODE_HPP
#define VEDIC_NODE_HPP

#include <string>
#include <vector>
#include "astro.hpp"
#include "node.hpp"
#include "chart.hpp"


namespace astro
{
  ////////////////////////////////
  //// node connector indices ////
  // inputs
#define VEDICNODE_INPUT_CHART   0
  // outputs
  ////////////////////////////////
  
  class VedicNode : public Node
  {
  protected:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return {new Connector<Chart>("Chart Input")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return { }; }

    ObjData *mMoonData = nullptr;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
  public:
    VedicNode();
    ~VedicNode();
    virtual std::string type() const { return "VedicNode"; }
  };
}


#endif // VEDIC_NODE_HPP
