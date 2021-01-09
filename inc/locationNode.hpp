#ifndef LOCATION_NODE_HPP
#define LOCATION_NODE_HPP

#include "astro.hpp"
#include "node.hpp"

namespace astro
{
  // forward declarations
  class LocationWidget;
  
  //// node connector indices ////
  // inputs
  // outputs
#define LOCNODE_OUTPUT_LOCATION       0
  ////////////////////////////////

  class LocationNode : public Node
  {
  private:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return {}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return {new Connector<Location>("Location Output")}; }
    
    LocationWidget *mWidget = nullptr;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
  public:
    LocationNode();
    LocationNode(const Location &dt);
    ~LocationNode();
    virtual std::string type() const { return "LocationNode"; }
  };

}

#endif // LOCATION_NODE_HPP
