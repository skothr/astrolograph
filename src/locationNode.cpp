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

