#ifndef GROUP_NODE_HPP
#define GROUP_NODE_HPP

#include "node.hpp"

namespace astro
{
  class GroupNode : public Node
  {
  protected:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return {}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return {}; }

    std::vector<Node*> mContents;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
  public:
    GroupNode(const std::vector<Node*> &contents={});
    ~GroupNode();
    virtual std::string type() const           { return "GroupNode"; }
    
    const std::vector<Node*>& contents() const { return mContents; }
    std::vector<Node*>&       contents()       { return mContents; }
    std::vector<Node*>        popContents()
    {
      std::vector<Node*> cont = mContents;
      for(auto n : cont)
        {
          n->setGroupLevel(mGroupLevel);         // same group level as where this group was
          n->setPos(n->pos() + rect().center()); // move node to group node location
        }
      mContents.clear();
      return cont;
    }
  };
}
#endif // GROUP_NODE_HPP
