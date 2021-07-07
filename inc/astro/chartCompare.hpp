#ifndef CHART_COMPARE_HPP
#define CHART_COMPARE_HPP

#include <vector>

#include "astro.hpp"
#include "chart.hpp"
#include "vector.hpp"
#include "ephemeris.hpp"

namespace astro
{
  class ChartCompare
  {
  private:
    Chart *mChartOuter = nullptr;
    Chart *mChartInner = nullptr;
    
    // Ephemeris swe;
    std::vector<ChartAspect> mAspects; // obj1 --> outer chart, obj2 --> inner chart
    bool mNeedUpdate = true;
    
  public:
    ChartCompare();
    ~ChartCompare();

    // std::vector<ChartAspect>& getAspects()             { return mAspects; }
    // const std::vector<ChartAspect>& getAspects() const { return mAspects; }

    int aspectCount(AspectType a)
    {
      int count = 0;
      for(auto asp : mAspects)
        { count += (asp.type == a ? 1 : 0); }
      return count;
    }
    
    void update();
    
    std::vector<ChartAspect> calcAspects(const ChartParams &params);

    Chart* getOuterChart() { return mChartOuter; }
    Chart* getInnerChart() { return mChartInner; }

    void setOuterChart(Chart *chart) { mChartOuter = chart; }
    void setInnerChart(Chart *chart) { mChartInner = chart; }
    
    std::vector<ChartAspect>& aspects() { return mAspects; }
  };
  
}

#endif // CHART_COMPARE_HPP
