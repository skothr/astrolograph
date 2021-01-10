#ifndef NODE_LIST_HPP
#define NODE_LIST_HPP

#include "vector.hpp"

namespace astro
{
  class NodeGraph;
  class ViewSettings;
  
  class NodeList
  {
  private:
    NodeGraph *mGraph = nullptr;
    ViewSettings *mViewSettings = nullptr;
    Vec2f mViewPos;
    Vec2f mViewSize;
    
  public:
    NodeList(NodeGraph *graph=nullptr, ViewSettings *viewSettings=nullptr);
    ~NodeList();

    void setGraph(NodeGraph *graph) { mGraph = graph; }
    void setViewSettings(ViewSettings *viewSettings) { mViewSettings = viewSettings; }
    void setPos(const Vec2f &p)     { mViewPos = p; }
    void setSize(const Vec2f &s)    { mViewSize = s; }

    void draw();
  };
}


#endif // NODE_LIST_HPP
