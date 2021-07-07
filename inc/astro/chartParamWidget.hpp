#ifndef CHART_PARAM_WIDGET_HPP
#define CHART_PARAM_WIDGET_HPP

#include "astro.hpp"

namespace astro
{
  // forward declarations
  class ChartParams;
  class Chart;
  
  class ChartParamWidget
  {
  private:
    bool mSettingsOpen    = false;
    bool mObjDisplayOpen  = false;
    bool mObjOrbsOpen     = false;
    bool mAspDisplayOpen  = false;
    bool mAspOrbsOpen     = false;

    std::array<bool, (OBJ_END-ANGLE_OFFSET+OBJ_COUNT)> mObjDisplayFocused;
    std::array<bool, (OBJ_END-ANGLE_OFFSET+OBJ_COUNT)> mObjOrbsFocused;
    std::array<bool, ASPECT_COUNT> mAspDisplayFocused;
    std::array<bool, ASPECT_COUNT> mAspOrbsFocused;

    ChartParams mLocalParams;
    ChartParams *mParams = nullptr;
    Chart       *mChart  = nullptr;
    
  public:
    ChartParamWidget(ChartParams *params);

    bool& settingsOpen()    { return mSettingsOpen; }
    bool& objDisplayOpen()  { return mObjDisplayOpen; }
    bool& objOrbsOpen()     { return mObjOrbsOpen; }
    bool& aspDisplayOpen()  { return mAspDisplayOpen; }
    bool& aspOrbsOpen()     { return mAspOrbsOpen; }
    
    const bool& settingsOpen() const   { return mSettingsOpen; }
    const bool& objDisplayOpen() const { return mObjDisplayOpen; }
    const bool& objOrbsOpen() const    { return mObjOrbsOpen; }
    const bool& aspDisplayOpen() const { return mAspDisplayOpen; }
    const bool& aspOrbsOpen() const    { return mAspOrbsOpen; }

    void setChart(Chart *chart) { mChart = chart; }
    
    void draw(float scale, bool blocked, bool visible);
  };
  
};

#endif // CHART_PARAM_WIDGET_HPP
