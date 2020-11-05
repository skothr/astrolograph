#ifndef CHART_VIEW_HPP
#define CHART_VIEW_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "imgui.h"
#include <GL/glew.h>

#include "astro.hpp"
#include "chart.hpp"
#include "chartCompare.hpp"
#include "node.hpp"

namespace astro
{
// chart params
#define CHART_SIZE                900.0f     // initial chart size
#define CHART_SIZE_DEFAULT        1024.0f    // default chart size (used to calculate scaling ratio)
#define CHART_SIZE_MIN            512.0f     // minimum chart size
#define CHART_SIZE_MAX            1660.0f    // maximum chart size
#define OUTER_RING_W              32.0f
#define CHART_PADDING             25.0f
#define ANGLE_SYMBOL_OFFSET       50.0f      // distance from outer zodiac ring border to draw angle symbols (e.g. ASC)

#define CHART_RING_W              92.0f      // width of zodiac sign ring
#define CHART_EARTH_RADIUS        80.0f      // radius of inner reference circle (with house numbers)
#define CHART_OBJRING_W           38.0f      // radius of ring where objects are shown

#define TEXT_HEIGHT               24.0f      // height of text
#define TEXT_PADDING              32.0f      // spacing between text
#define TEXT_LINE_PADDING         10.0f      // spacing between text

#define CHART_SYMBOL_SIZE         32.0f      // default size of obejct symbols (should be same as image file size) 
#define CHART_SYMBOL_SIZE_SMALL   20.0f      // default size of obejct symbols (should be same as image file size) 

#define CHART_HOUSE_NUM_OFFSET    14.0f
#define CHART_HOUSE_CIRCLE_RADIUS 14.0f
#define DEGREE_TICK_SIZE_1        5.0f
#define DEGREE_TICK_SIZE_5        12.0f
#define DEGREE_TICK_SIZE_10       12.0f
#define DEGREE_TICK_SIZE_15       16.0f
#define DEGREE_TICK_SIZE_30       20.0f
  
#define OUTLINE_W                 3.0f // zodiac chart line width
#define OBJRING_OUTLINE_W         1.0f // object ring line width

  struct ViewParams
  {
    // defined
    Vec2f pos;        // chart position
    Vec2f size;       // chart size
    Vec2f center;     // chart center
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
      
    ViewParams(const Vec2f &p, const Vec2f &s)
      : pos(p), size(s), center(p + s/2.0f)
    { calculate(); }
    
    void calculate()
    {
      minSize = std::min(size.x, size.y);
      sizeRatio = minSize / CHART_SIZE_DEFAULT;
      symbolSize = sizeRatio * CHART_SYMBOL_SIZE;
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
    bool mSelected   = true;
    // view settings
    bool mAlignAsc   = false; // rotate chart so ascendant points left
    
    std::vector<bool> mShowObjects;
    std::vector<bool> mFocusObjects;
    
    float screenAngle(Chart *chart, float longitude) // convert longitude (degrees) to angle on screen (radians) based on chart orientation
    { return M_PI/180.0f * (longitude - (mAlignAsc ? chart->getObject(ANGLE_DSC)->angle : 0.0f)); }
    float screenAngle(ChartCompare *compare, float longitude) // convert longitude (degrees) to angle on screen (radians) based on chart orientation
    { return M_PI/180.0f * (longitude - (mAlignAsc ? compare->getInnerChart()->getObject(ANGLE_DSC)->angle : 0.0f)); }
    
  public:
    ChartView();
    
    bool isSelected() const      { return mSelected; }
    void setSelected(bool selected) { mSelected = selected; }
    
    void setAlignAsc(bool align) { mAlignAsc = true; }
  
    void renderZodiac(Chart *chart, const ViewParams &params, ImDrawList *draw_list);
    void renderHouses(Chart *chart, const ViewParams &params, ImDrawList *draw_list);
    void renderAngles(Chart *chart, const ViewParams &params, ImDrawList *draw_list);
    void renderAspects(Chart *chart, const ViewParams &params, ImDrawList *draw_list);
    void renderObjects(Chart *chart, int level, const ViewParams &params, ImDrawList *draw_list);
    void renderChart(Chart *chart, const Vec2f &chartSize);
    void renderCompareAspects(ChartCompare *compare, const ViewParams &params, ImDrawList *draw_list);
    void renderChartCompare(ChartCompare *compare, const Vec2f &chartSize);
    
    bool draw(Chart *chart, float chartWidth);
    bool draw(ChartCompare *compare, float chartWidth);
  };
}

#endif // CHART_VIEW_HPP
