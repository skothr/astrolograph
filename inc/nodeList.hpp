#ifndef NODE_LIST_HPP
#define NODE_LIST_HPP

#include "vector.hpp"

namespace astro
{
  class NodeGraph;

  
  class NodeList
  {
  private:
    NodeGraph *mGraph = nullptr;

    Vec2f mViewPos;
    Vec2f mViewSize;
    
  public:
    NodeList(NodeGraph *graph);
    ~NodeList();

    void setPos(const Vec2f &p) { mViewPos = p; }
    void setSize(const Vec2f &s) { mViewSize = s; }

    void draw();
  };
}


#endif // NODE_LIST_HPP
