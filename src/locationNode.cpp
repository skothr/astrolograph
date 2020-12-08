#include "locationNode.hpp"
using namespace astro;

#include "imgui.h"
#include "nodeGraph.hpp"


LocationNode::LocationNode()
  : LocationNode(Location())
{ }

LocationNode::LocationNode(const Location &loc)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Location Node"), mWidget(loc)
{
  mSettings.push_back(new Setting<std::string>("savedName", "savedName", mWidget.getName()));
  mSettings.push_back(new Setting<Location>   ("Location",  "location",  &mWidget.get()));

  // virtual void getSaveParams(std::map<std::string, std::string> &params) const override
  //   {
  //     params.emplace("location",  mWidget.get().toSaveString());
  //     params.emplace("savedName", mWidget.getName());
  //   };
  //   virtual void setSaveParams(std::map<std::string, std::string> &params) override
  //   {
  //     auto iter = params.find("savedName");
  //     std::string saveName = ((iter != params.end()) ? iter->second : "");
  //     if(!saveName.empty()) { mWidget.load(saveName); mWidget.setName(saveName); }
  //     else { std::cout << "WARNING: Saved name empty!\n"; }
     
  //     iter = params.find("location");
  //     if(iter != params.end()) { mWidget.get().fromSaveString(iter->second); }
  //     else { std::cout << "WARNING: Could not find 'location' param!\n"; }
  //   };
    
  outputs()[LOCNODE_OUTPUT_LOCATION]->set(&mWidget.get());
}

void LocationNode::onUpdate()
{
  mWidget.update();
}

void LocationNode::onDraw()
{
  float scale = mGraph->getScale();
  mWidget.draw(scale, isBlocked());
}

