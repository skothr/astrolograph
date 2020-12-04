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
    bool mOrbsOpen = true;
    std::vector<bool> mAspVisible;
    std::vector<double> mAspOrbs;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      params.emplace("listOpen", (mListOpen ? "1" : "0"));
      params.emplace("orbsOpen", (mOrbsOpen ? "1" : "0"));
      std::string visStr;
      std::string orbStr;
      for(int a = 0; a < ASPECT_COUNT; a++) // start with all aspects visible
        {
          visStr.append(mAspVisible[a] ? "1" : "0");
          orbStr.append(to_string(mAspOrbs[a], 6) + ((a != ASPECT_COUNT-1) ? "|" : ""));
        }
      params.emplace("aspVisible", visStr);
      params.emplace("aspOrbs", orbStr);
      return params;
    };
    
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      auto iter = params.find("listOpen"); if(iter != params.end()) { mListOpen = (iter->second != "0"); }
      iter = params.find("orbsOpen"); if(iter != params.end()) { mOrbsOpen = (iter->second != "0"); }
      
      iter = params.find("aspVisible");
      if(iter != params.end())
        {
          for(int a = 0; a < ASPECT_COUNT; a++) // start with all aspects visible
            { mAspVisible[a] = (iter->second[a] != '0'); }
        }
      iter = params.find("aspOrbs");
      if(iter != params.end())
        {
          // for(int a = 0; a < ASPECT_COUNT; a++) // start with all aspects visible
          int a = 0;
          std::string orbStr;
          std::stringstream ss(iter->second);
          while(std::getline(ss, orbStr, '|'))
            {
              std::istringstream is(orbStr);
              is >> mAspOrbs[a++];
            }
        }
      return params;
    };
    
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
