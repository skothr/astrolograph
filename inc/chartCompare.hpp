#ifndef CHART_COMPARE_HPP
#define CHART_COMPARE_HPP

#include "astro.hpp"
#include "vector.hpp"
#include "ephemeris.hpp"

#include <vector>
#include "chart.hpp"

namespace astro
{
  class ChartCompare
  {
  private:
    Chart *mChartOuter = nullptr;
    Chart *mChartInner = nullptr;

    std::array<double, astro::ASPECT_COUNT> mAspectOrbs;
    std::array<bool,   astro::ASPECT_COUNT> mAspectFocus;
    std::array<bool,   astro::ASPECT_COUNT> mAspectVisible;
    
    // Ephemeris swe;
    std::vector<ChartAspect> mAspects; // obj1 --> outer chart, obj2 --> inner chart
    bool mNeedUpdate = true;
    
  public:
    ChartCompare();
    ~ChartCompare();

    void setAspectOrb(astro::AspectType asp, double orb)
    { if(asp > ASPECT_INVALID && asp < ASPECT_COUNT) { mAspectOrbs[(int)asp] = orb; } }
    double getAspectOrb(astro::AspectType asp)
    { if(asp > ASPECT_INVALID && asp < ASPECT_COUNT) { return mAspectOrbs[(int)asp]; } }
    void setAspectFocus(astro::AspectType asp, bool focus)
    { if(asp > ASPECT_INVALID && asp < ASPECT_COUNT) { mAspectFocus[(int)asp] = focus; } }
    void setAspectVisible(astro::AspectType asp, bool visible)
    { if(asp > ASPECT_INVALID && asp < ASPECT_COUNT) { mAspectVisible[(int)asp] = visible; } }
    bool getAspectFocus(astro::AspectType asp)   { return (asp > ASPECT_INVALID && asp < ASPECT_COUNT) ? mAspectFocus[(int)asp]   : false; }
    bool getAspectVisible(astro::AspectType asp) { return (asp > ASPECT_INVALID && asp < ASPECT_COUNT) ? mAspectVisible[(int)asp] : false; }

    std::vector<ChartAspect>& getAspects()             { return mAspects; }
    const std::vector<ChartAspect>& getAspects() const { return mAspects; }

    int aspectCount(AspectType a)
    {
      int count = 0;
      for(auto asp : mAspects)
        { count += (asp.type == a ? 1 : 0); }
      return count;
    }
    
    void update();
    void calcAspects();

    Chart* getOuterChart() { return mChartOuter; }
    Chart* getInnerChart() { return mChartInner; }

    void setOuterChart(Chart *chart) { mChartOuter = chart; }
    void setInnerChart(Chart *chart) { mChartInner = chart; }
    
    std::vector<ChartAspect>& aspects() { return mAspects; }
  };
  
}

#endif // CHART_COMPARE_HPP
