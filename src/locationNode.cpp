#include "locationNode.hpp"
using namespace astro;

#include "imgui.h"
#include "nodeGraph.hpp"
#include "locationWidget.hpp"
#include "setting.hpp"

LocationNode::LocationNode()
  : LocationNode(Location())
{ }
LocationNode::LocationNode(const Location &loc)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Location Node"), mWidget(new LocationWidget(loc))
{
  mSettings.push_back(new Setting<std::string>("Name",           "name",           &mWidget->getName()));
  // mSettings.push_back(new Setting<std::string>("Saved Name",     "savedName", &mWidget->getSavedName()));
  mSettings.push_back(new Setting<Location>   ("Location",       "location",       &mWidget->get()));
  mSettings.push_back(new Setting<Location>   ("Saved Location", "savedLocation",  &mWidget->getSaved()));
  mSettings.push_back(new Setting<bool>       ("Expanded", "expanded",             &mWidget->getExpanded()));
  outputs()[LOCNODE_OUTPUT_LOCATION]->set(&mWidget->get());
}
LocationNode::~LocationNode()
{ if(mWidget) { delete mWidget; } }

void LocationNode::onUpdate()
{
  mWidget->update();
}

void LocationNode::onDraw()
{
  mWidget->setGraph(mGraph);
  float scale = mGraph->getScale();
  mWidget->draw(scale, isBlocked());
}

