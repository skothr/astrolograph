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

    bool mListOpen = true;
    bool mOrbsOpen = false;
    std::vector<bool> mAspVisible;
    std::vector<double> mAspOrbs;
    
    virtual bool onDraw() override;
    
  public:
    AspectNode();
    virtual std::string type() const { return "AspectNode"; }
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          ((AspectNode*)other)->mListOpen = mListOpen;
          ((AspectNode*)other)->mOrbsOpen = mOrbsOpen;
          for(int a = 0; a < ASPECT_COUNT; a++) // start with all aspects visible
            {
              ((AspectNode*)other)->mAspVisible[a] = mAspVisible[a];
              ((AspectNode*)other)->mAspOrbs[a] = mAspOrbs[a];
            }
          // (everything else set by connections)
          return true;
        }
      else { return false; }
    }
  };
}

#endif // ASPECT_NODE_HPP
