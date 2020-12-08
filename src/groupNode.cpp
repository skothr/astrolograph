#include "groupNode.hpp"
using namespace astro;



GroupNode::GroupNode(const std::vector<Node*> &contents)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Group"), mContents(contents)
{ }

GroupNode::~GroupNode()
{ }


void GroupNode::onUpdate()
{

}

void GroupNode::onDraw()
{
  for(auto n : mContents)
    {
      
    }
}
