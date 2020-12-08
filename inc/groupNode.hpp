#ifndef GROUP_NODE_HPP
#define GROUP_NODE_HPP

#include "node.hpp"

namespace astro
{
  class GroupNode : public Node
  {
  protected:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return {}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return {new Connector<Location>("Location Output")}; }

    std::vector<Node*> mContents;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
  public:
    GroupNode(const std::vector<Node*> &contents);
    ~GroupNode();
    virtual std::string type() const { return "GroupNode"; }    
  };
}
#endif // GROUP_NODE_HPP
