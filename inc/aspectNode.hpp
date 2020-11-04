#ifndef ASPECT_NODE_HPP
#define ASPECT_NODE_HPP

#include "astro.hpp"
#include "node.hpp"

namespace astro
{
  class AspectNode : public Node
  {
  private:
    virtual bool onDraw() override;
    
  public:
    AspectNode();
    virtual std::string type() const { return "AspectNode"; }
  };
}

#endif // ASPECT_NODE_HPP
