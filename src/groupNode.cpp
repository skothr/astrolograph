#include "groupNode.hpp"
using namespace astro;
#include "nodeGraph.hpp"
#include "imgui.h"
//#include "setting.hpp"

GroupNode::GroupNode(const std::vector<Node*> &contents)
  : Node(CONNECTOR_INPUTS(), CONNECTOR_OUTPUTS(), "Group"), mContents(contents)
{
  Vec2f avgCenter;
  Vec2f totalSize;
  for(auto n : mContents)
    {
      avgCenter += n->rect().center();
      totalSize.y += n->size().y;
      totalSize.x = std::max(totalSize.x, n->size().x);
    }
  avgCenter /= mContents.size();
  setPos(avgCenter-totalSize/2.0f);
  // make node positions an offset relative to this group node
  for(auto n : mContents)
    {
      n->setSelected(false);
      n->setPos(n->pos() - avgCenter);
      //mSettings.insert(mSettings.end(), n->getNodeSettings().begin(), n->getNodeSettings().end());
      for(auto con : n->inputs())
        {
          if(con->getConnected().size() > 0)
            {
              for(auto con2 : con->getConnected())
                {
                  // TODO: ???
                }
            }
        }
      outputs().insert(outputs().end(), n->outputs().begin(), n->outputs().end());
    }
}

GroupNode::~GroupNode()
{
  for(auto n : mContents) { delete n; }
  mContents.clear();
}

void GroupNode::onUpdate()
{
  for(auto n : mContents)
    {
      n->setGroupLevel(mGroupLevel+1);
      n->update();
    }
}

void GroupNode::onDraw()
{
  bool  blocked = isBlocked();
  float scale   = getScale();
  Vec2f originalCursor = pos()+NODE_PADDING;
  Vec2f cursor = originalCursor;
  float totalHeight = NODE_PADDING.y;
  for(auto n : mContents)
    {      
      Vec2f p = n->pos();
      n->setPos(cursor);
      n->setBlocked(blocked);
      n->BeginDraw();
      {
        n->drawBody();
        mActive |= n->isActive();
      }
      n->EndDraw();
      
      n->setPos(p);
      totalHeight += n->getBodySize().y + NODE_PADDING.y;
      cursor.y = originalCursor.y + totalHeight;
      ImGui::SetCursorScreenPos(mGraph->graphToScreen(cursor));
      ImGui::Separator();
      totalHeight += NODE_PADDING.y;
      cursor.y = originalCursor.y + totalHeight;
      ImGui::SetCursorScreenPos(mGraph->graphToScreen(cursor));
    }
}
