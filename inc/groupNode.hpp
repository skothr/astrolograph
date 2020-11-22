#ifndef GROUP_NODE_HPP
#define GROUP_NODE_HPP

#include "node.hpp"

namespace astro
{
  class GroupNode : public Node
  {
  protected:
    std::vector<Node*> mContents;

    
  public:
    GroupNode();
    ~GroupNode();
  };
}
#endif // GROUP_NODE_HPP
