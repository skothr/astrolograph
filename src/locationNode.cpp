#include "locationNode.hpp"
using namespace astro;

#include "imgui.h"


// node connector indices
#define LOCNODE_OUTPUT_LOCATION 0


LocationNode::LocationNode()
  : LocationNode(Location())
{ }

LocationNode::LocationNode(const Location &loc)
  : Node({}, {new Connector<Location>("LocationOut")}, "Location Node"), mWidget(loc)
{
  outputs()[LOCNODE_OUTPUT_LOCATION]->set(&mWidget.get());
}


bool LocationNode::onDraw()
{
  mWidget.draw();
  return true;
}

