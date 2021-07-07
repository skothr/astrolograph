#ifndef CHART_VIEW_HPP
#define CHART_VIEW_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"

namespace astro
{

  // forward declarations
  class ChartCompare;

  struct ViewParams
  {
    // defined
    Vec2f pos;        // chart position
    Vec2f size;       // chart size
    Vec2f center;     // chart center
    bool blocked;     // whether mouse is blocked
    float alpha=1.0f; // alpha mask (TODO: full color mask)
    // calculated
    float minSize;    // minimum dimension (x/y) value
    float sizeRatio;  // ratio of size to default size
    float symbolSize; // size of object symbols in object ring
    float oRadius;    // outer zodiac radius from center
    float iRadius;    // inner zodiac radius from center
    float eRadius;    // earth radius from center (house line start)
    float objRadius;  // object ring radius from center
    float symRadius;  // symbol ring radius from center
    float angRadius;  // outer angle radius from center
    float objRingW;   // width of object ring
      
    ViewParams(const Vec2f &p, const Vec2f &s, bool blocked_, float alpha_)
      : pos(p), size(s), center(p + s/2.0f), blocked(blocked_), alpha(alpha_)
    { calculate(); }
    
    void calculate()
    {
      minSize = std::min(size.x, size.y);
      sizeRatio = minSize / CHART_SIZE_DEFAULT;
      symbolSize = sizeRatio * CHART_OBJ_SYMBOL_SIZE;
      objRingW = CHART_OBJRING_W*sizeRatio;

      // radii
      oRadius = minSize/2.0f - sizeRatio*(CHART_PADDING + ANGLE_SYMBOL_OFFSET);
      iRadius = oRadius - sizeRatio*CHART_RING_W;
      eRadius = sizeRatio*CHART_EARTH_RADIUS;
      objRadius = iRadius - objRingW;
      symRadius = iRadius - objRingW/2.0f;
      angRadius = oRadius + sizeRatio*ANGLE_SYMBOL_OFFSET;
    }
  };
  
  class ChartView
  {
  private:
    // view settings
    bool mAlignAsc   = false; // rotate chart so ascendant points left
    bool mShowHouses = true;  // if true, show interactive house number outside chart
    std::vector<bool> mShowObjects;
    std::vector<bool> mFocusObjects;
    
    float screenAngle(Chart *chart, float longitude);          // convert longitude (degrees) to angle on screen (radians) based on chart orientation
    float screenAngle(ChartCompare *compare, float longitude); // convert longitude (degrees) to angle on screen (radians) based on chart orientation
  public:
    ChartView();
    
    void setAlignAsc(bool align)  { mAlignAsc = align; }
    void setShowHouses(bool show) { mShowHouses = show; }
    bool getAlignAsc() const      { return mAlignAsc; }
    bool getShowHouses() const    { return mShowHouses; }

    // void BeginTooltip();
    // void EndTooltip();
  
    void renderZodiac(Chart *chart, const ViewParams &params, ImDrawList *draw_list, ChartParams &chartParams);
    void renderHouses(Chart *chart, const ViewParams &params, ImDrawList *draw_list, ChartParams &chartParams);
    void renderAngles(Chart *chart, const ViewParams &params, ImDrawList *draw_list, ChartParams &chartParams);
    void renderAspects(Chart *chart, const ViewParams &params, ImDrawList *draw_list, ChartParams &chartParams);
    void renderCompareAspects(ChartCompare *compare, const ViewParams &params, ImDrawList *draw_list, ChartParams &chartParams);
    void renderObjects(Chart *chart, int level, const ViewParams &params, ImDrawList *draw_list, ChartParams &chartParams);
    
    void renderChart(Chart *chart, float scale, bool blocked, ChartParams &chartParams);
    void renderChartCompare(ChartCompare *compare, float scale, bool blocked, ChartParams &chartParams);
    void draw(Chart *chart, float scale, bool blocked, ChartParams &chartParams);
    void draw(ChartCompare *compare, float scale, bool blocked, ChartParams &chartParams);
  };
}

#endif // CHART_VIEW_HPP
