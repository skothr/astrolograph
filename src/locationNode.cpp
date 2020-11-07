#include "locationNode.hpp"
using namespace astro;

#include "imgui.h"


LocationNode::LocationNode()
  : LocationNode(Location())
{ }

LocationNode::LocationNode(const Location &loc)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Location Node"), mWidget(loc)
{
  outputs()[LOCNODE_OUTPUT_LOCATION]->set(&mWidget.get());
  // setMinSize(Vec2f(512, 512));
}


bool LocationNode::onDraw()
{
  mWidget.draw();
  return true;
}

