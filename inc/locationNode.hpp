#ifndef LOCATION_NODE_HPP
#define LOCATION_NODE_HPP

#include "astro.hpp"
#include "locationWidget.hpp"
#include "node.hpp"

namespace astro
{
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
    
    LocationWidget mWidget;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      params.emplace("location",  mWidget.get().toSaveString());
      params.emplace("savedName", mWidget.getName());
      return params;
    };
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      auto iter = params.find("savedName");
      std::string saveName = ((iter != params.end()) ? iter->second : "");
      if(!saveName.empty()) { mWidget.load(saveName); mWidget.setName(saveName); }
      else { std::cout << "WARNING: Saved name empty!\n"; }
     
      iter = params.find("location");
      if(iter != params.end()) { mWidget.get().fromSaveString(iter->second); }
      else { std::cout << "WARNING: Could not find 'location' param!\n"; }

      return params;
    };
    virtual void onUpdate() override;
    virtual void onDraw() override;
    
  public:
    LocationNode();
    LocationNode(const Location &dt);
    virtual std::string type() const { return "LocationNode"; }
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          ((LocationNode*)other)->mWidget = mWidget;
          return true;
        }
      else { return false; }
    }
    
    // bool getDST() const { return mWidget.getDST(); }
  };

}

#endif // LOCATION_NODE_HPP
