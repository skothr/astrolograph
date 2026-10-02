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
    // bool mObjOrbsOpen     = false;
    // bool mAspOrbsOpen     = false;
    bool mAspDisplayOpen  = false;
    bool mOrbsOpen = false;

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
    bool& aspDisplayOpen()  { return mAspDisplayOpen; }
    // bool& objOrbsOpen()     { return mObjOrbsOpen; }
    // bool& aspOrbsOpen()     { return mAspOrbsOpen; }
    bool& orbsOpen()        { return mOrbsOpen; }
    
    const bool& settingsOpen() const   { return mSettingsOpen; }
    const bool& objDisplayOpen() const { return mObjDisplayOpen; }
    const bool& aspDisplayOpen() const { return mAspDisplayOpen; }
    // const bool& objOrbsOpen() const    { return mObjOrbsOpen; }
    // const bool& aspOrbsOpen() const    { return mAspOrbsOpen; }
    const bool& orbsOpen() const       { return mOrbsOpen; }

    void setChart(Chart *chart) { mChart = chart; }
    
    void draw(float scale, bool blocked, bool visible);
  };
  
};

#endif // CHART_PARAM_WIDGET_HPP
