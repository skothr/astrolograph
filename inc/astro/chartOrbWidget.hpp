#ifndef CHART_ORB_WIDGET_HPP
#define CHART_ORB_WIDGET_HPP

#include <array>
#include <vector>
#include <ostream>
#include <string>
#include "astro.hpp"

namespace astro
{
  class ChartOrbWidget
  {
  private:
    ChartOrbs mOrbs;
    std::vector<BoolStruct> mConstObjOrbs;
    std::vector<BoolStruct> mConstAspOrbs;
    bool mAllConst = false;
    
  public:
    ChartOrbWidget();
    ~ChartOrbWidget();

    const ChartOrbs& getOrbs() const { return mOrbs; }
    ChartOrbs& getOrbs()             { return mOrbs; }
    
    void draw(float scale);
  };

}

#endif // CHART_ORB_WIDGET_HPP
