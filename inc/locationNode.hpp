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
      params.emplace("savedName", mWidget.getName());
      if(mWidget.getName().empty() || mWidget.get() != mWidget.getSaved())
        { params.emplace("location",  mWidget.get().toSaveString()); }
      params.emplace("dst",  (mWidget.getDST() ? "1" : "0"));
      return params;
    };
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      std::string saveName = params["savedName"];
      std::cout << "LOC: " << params["location"] << " | NAME: " << saveName << "\n";
      if(saveName.empty())
        {
          mWidget.get().fromSaveString(params["location"]);
          mWidget.setSaved(mWidget.get());
          mWidget.setName("");
          mWidget.setSaveName("");
        }
      else
        {
          mWidget.load(saveName);
          mWidget.setName(saveName);
          mWidget.setSaveName(saveName);
        }
      mWidget.setDST(params["dst"] != "0");
      return params;
    };
    virtual bool onDraw() override;    
    
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
  };

}

#endif // LOCATION_NODE_HPP
