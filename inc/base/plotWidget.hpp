#ifndef PLOT_WIDGET_HPP
#define PLOT_WIDGET_HPP

#include <string>
#include <vector>
#include <map>
#include "geometry.hpp"
#include "dateTime.hpp"
#include "marketData.hpp"
#include "imtools.hpp"
namespace astro
{
#define FRAME_PADDING        Vec2f(24.0f, 24.0f)
  
#define LEGEND_TITLE         "Legend"
#define LEGEND_FRAME_PADDING Vec2f(12.0f, 12.0f)
#define LEGEND_ITEM_SPACING  5.0f
  
#define AXIS_FRAME_PADDING   Vec2f(5.0f, 5.0f)
#define AXIS_ITEM_SPACING    5.0f

#define AXIS_WIDTH 16.0f
  
// #define TOOLTIP_PADDING Vec2f(5.0f, 5.0f)
// #define TOOLTIP_SPACING Vec2f(5.0f, 5.0f)

  // forward declarations
  class ViewSettings;
  
  struct PlanetPoint
  {
    double angle = 0.0;
    double speed = 0.0;
    PlanetPoint(double a=0.0, double s=0.0) : angle(a), speed(s) { }
  };
  enum PlotType
    {
     PLOT_POINT = 0,
     PLOT_LINE,
     PLOT_BAR,
     PLOT_CANDLESTICK, // (market data)
     PLOT_PLANET,      // (speed and position)
    };
  
  struct Axis
  {
    bool xAxis = false;
    // params
    Vec2d  range     = Vec2d(0,360); // axis view range
    double offset    = 0.0;          // axis view offset
    // state
    double center    = 0.0;   // plot view center
    double viewSize  = 0.0;   // plot view size
    double viewScale = 1.0;   // scales axis view value to screen value (vector)
    bool   visible   = true;  // if true, axis is visible
    bool   locked    = false; // if true, locks axis range
    bool   logScale  = false; // if true, axis uses natural log scaling (WIP --> TODO)
    bool   autoScale = false; // if true, range/offset are chosen automatically
    // // data
    double dataMin = 0.0;
    double dataMax = 0.0;
    // labeling
    bool   drawLabels     = true;
    bool   drawLabelLines = false;
    double labelMin       = 0.0;   // first label value
    double labelMax       = 0.0;   // max label value (if == 0, won't be applied)
    double labelInterval  = 100.0; // plot-space interval for labels

    double p1() const        { return center - viewSize/2.0; }
    double p2() const        { return center + viewSize/2.0; }
    double size() const      { return viewSize; }
    double rangeSize() const { return range.y - range.x; }

    void move(double dp) { center += dp; }
    void setSize(double newSize)
    {
      double s = size();
      double p1Old = center-s/2.0;       double p2Old = center+s/2.0;
      double p1New = center-newSize/2.0; double p2New = center+newSize/2.0;
      //scale(p1New/p1Old);
       viewSize  *= (p1New/p1Old);
       viewScale *= (p1New/p1Old);
    }
    void scale(double mult)
    {
      if(logScale)
        {
          viewSize  = viewSize + log(mult);
          viewScale = viewScale + log(mult);
        }
      else
        { viewSize *= mult; viewScale *= mult; }
    }
    void scale(double mult, double focus)
    {
      if(logScale)
        {
          double focusDiff = log(exp(center) - focus);
          scale(mult);
          center += focusDiff+log(mult) - focusDiff;
        }
      else
        {
          double focusDiff = center - focus;
          scale(mult);
          center += focusDiff*mult - focusDiff;
        }
    }
  };

  struct DateAxis : public Axis
  {
    bool yearLabels  = true;
    bool monthLabels = true;
    bool dayLabels   = true;
  };
  
  // PARAMS FOR DATA SET
  struct PlotDataParams
  {
    std::string label    = "";
    bool        visible  = true;
    bool        focused  = false;
    Vec4f       color    = Vec4f(0.5f, 0.5f, 0.5f, 1.0f);
    float       width    = 2.0f;
    PlotType    plotType = PLOT_LINE;
    Axis       *axis     = nullptr;

    PlotDataParams() { }
    PlotDataParams(const std::string &label_, const Vec4f &color_, float width_)
      : label(label_), color(color_), width(width_) { }
    
    std::map<std::string, bool> flags;
    void setFlag(const std::string &name, bool val) { auto iter = flags.find(name); if(iter != flags.end()) { iter->second = val; } }
    bool getFlag(const std::string &name) const     { auto iter = flags.find(name); return ((iter != flags.end()) ? iter->second : false); }
  };
  struct PlanetDataParams : public PlotDataParams
  {
    bool showPos       = false; // plot planet angle position
    bool showSpeed     = false; // plot planet angle speed
    bool showRx        = false; // highlight retrograde periods
    bool showShadows   = false; // highlight retrograde shadow periods
    bool showRxBoxes   = false; // highlight shadow+rx area over position plot
    bool connectBreaks = false; // connect points that pass the 0/360 degree point with dotted lines

    PlanetDataParams() { }
    PlanetDataParams(const std::string &label_, const Vec4f &color_, float width_)
      : PlotDataParams(label_, color_, width_)  { }
  };

  struct AspectDataParams : public PlotDataParams
  {
    bool showOrb = true;  // plot aspect orb
    ObjType obj1 = OBJ_INVALID;
    ObjType obj2 = OBJ_INVALID;
    AspectType asp = ASPECT_SQUARE;

    AspectDataParams() { }
    AspectDataParams(const std::string &label_, const Vec4f &color_, float width_)
      : PlotDataParams(label_, color_, width_)  { }
  };


  struct AxisLabel
  {
    std::vector<std::string> text;      // each element is a line of text as part of the label
    double                   plotPos;   // position of label on plot axis
    Vec2d                    screenPos; // screen-space position of label
    Vec2f                    textSize;  // screen size of padded text
  };  

  struct RxBox
  {
    Rect2d rxBox;
    Rect2d shadowBox;
    bool   flipped     = false;
    bool   rxValid     = false;
    bool   dxValid     = false;
    bool   preShadowComplete = false;
    bool   postShadowComplete = false;
    RxBox(const Rect2d &rx, const Rect2d &shadow, bool flipped_)
      : rxBox(rx), shadowBox(shadow), flipped(flipped_)
    { }
    RxBox(const Rect2d &rx)
      : rxBox(rx), shadowBox(rx), flipped(false)
    { }
  };

  
  //// PLOT WIDGET ////
  class PlotWidget
  {
  public:
    
    struct DateAxis : public Axis
    {
      bool yearLabels  = true;
      bool monthLabels = true;
      bool dayLabels   = true;
    };

  protected:
    
    bool  mNeedUpdate   = false;
    bool  mClicked      = false;
    bool  mHovered      = false;
    bool  mPlotHovered  = false;
    bool  mXAxisHovered = false;
    bool  mYAxisHovered = false;
    bool  mContextOpen  = false;

    Vec4f mBgColor      = Vec4f(0.05f, 0.05f, 0.05f, 1.0f);
    Vec4f mAxisBgColor  = Vec4f(0.1f, 0.1f, 0.1f, 1.0f);

    float  mScale = 1.0f;
    Rect2d mScreenRect;
    Rect2d mOuterRect;
    Vec2d  mLastScreenSize;
    bool   mFirstFrame = true;
    Vec2d  mPadRatio    = Vec2d(0.1, 0.1);

    bool mAnyFocused = false;

    // double (vec)
    double plotToScreenVecAx(double pvx, const Axis *axis) const
    {
      if(axis->logScale)
        { return (log(pvx == 0.0 ? 1.0 : pvx) * axis->viewScale * (axis->xAxis ? 1.0 : -1.0)); }
      else
        { return (pvx * axis->viewScale * (axis->xAxis ? 1.0 : -1.0)); }
    } // flip if y axis
    double screenToPlotVecAx(double svx, const Axis *axis) const
    {
      if(axis->logScale)
        {
          double pvx = svx / axis->viewScale;
          return pvx * (axis == &mDateAxis ? 1.0 : -1.0);
        }
      else
        {
          double pvx = svx / axis->viewScale;
          return pvx * (axis == &mDateAxis ? 1.0 : -1.0);
        }
    } // flip if y axis
    // double (pos)
    double plotToScreenPosAx(double ppx, const Axis *axis) const
    {
      if(axis->logScale)
        {
          return (plotToScreenVecAx(ppx, axis) - axis->center +
                  (axis == &mDateAxis ? mScreenRect.size().x/2.0f + mScreenRect.p1.x : mScreenRect.size().y/2.0f + mScreenRect.p1.y));
        }
      else
        {
          return (plotToScreenVecAx(ppx - axis->center, axis) +
                  (axis == &mDateAxis ? mScreenRect.size().x/2.0f + mScreenRect.p1.x : mScreenRect.size().y/2.0f + mScreenRect.p1.y));
        }
    }
    double screenToPlotPosAx(double spx, const Axis *axis) const
    {
      if(axis->logScale)
        {
          double ppx (screenToPlotVecAx(spx - (axis == &mDateAxis ?
                                               (mScreenRect.size().x/2.0 + mScreenRect.p1.x) : (mScreenRect.size().y/2.0 + mScreenRect.p1.y)), axis));
          return ppx + axis->center;
        }
      else
        {
          double ppx (screenToPlotVecAx(spx - (axis == &mDateAxis ?
                                               (mScreenRect.size().x/2.0 + mScreenRect.p1.x) : (mScreenRect.size().y/2.0 + mScreenRect.p1.y)), axis));
          return ppx + axis->center;
        }
    }
    
    // Vec2d (vec)
    Vec2d plotToScreenVec(const Vec2d &pv,  const Axis *yAxis) const   { return Vec2d(plotToScreenVecAx(pv.x, &mDateAxis), plotToScreenVecAx(pv.y, yAxis)); }
    Vec2d screenToPlotVec(const Vec2d &sv,  const Axis *yAxis) const   { return Vec2d(screenToPlotVecAx(sv.x, &mDateAxis), screenToPlotVecAx(sv.y, yAxis)); }
    // Vec2d (pos)
    Vec2d plotToScreenPos(const Vec2d &pp,  const Axis *yAxis) const   { return Vec2d(plotToScreenPosAx(pp.x, &mDateAxis), plotToScreenPosAx(pp.y, yAxis)); }
    Vec2d screenToPlotPos(const Vec2d &sp,  const Axis *yAxis) const   { return Vec2d(screenToPlotPosAx(sp.x, &mDateAxis), screenToPlotPosAx(sp.y, yAxis)); }
    // Rect2d
    Rect2d screenToPlotRect(const Rect2d &pr, const Axis *yAxis) const { return Rect2d(screenToPlotPos(pr.p1, yAxis), screenToPlotPos(pr.p2, yAxis)); }
    Rect2d plotToScreenRect(const Rect2d &sr, const Axis *yAxis) const { return Rect2d(plotToScreenPos(sr.p1, yAxis), plotToScreenPos(sr.p2, yAxis)); }
    
    bool handleIO();

    float getXAxisHeight(ViewSettings *vs, float scale, const DateAxis *axis);
    float getYAxisWidth(ViewSettings *vs, float scale, const Axis *axis);
    
  public:
    std::string mTitle = "";

    double scalePlotPosAx(double ppx, const Axis *axis)   { return (axis->logScale ? log(ppx) : ppx); }
    double unscalePlotPosAx(double ppx, const Axis *axis) { return (axis->logScale ? exp(ppx) : ppx); }
    
    // axes
    DateAxis mDateAxis;
    Axis mPlanetPosAxis;
    Axis mPlanetSpeedAxis;
    Axis mPlanetAspectAxis;
    Axis mAspectAxis;
    Axis mMarketAxis;
    Axis mVolumeAxis;
    std::vector<Axis*> mYAxes;
    
    // params
    std::vector<PlanetDataParams*>               mPlanetParams;
    std::vector<AspectDataParams*>               mAspectParams;
    PlotDataParams*                              mMarketParams;
    PlotDataParams*                              mVolumeParams;
    std::vector<PlotDataParams*>                 mDataParams; // list of all data params
    // data
    std::vector<const std::vector<PlanetPoint>*> mPlanetData;
    std::vector<const std::vector<RxBox>*>       mRetrogrades;
    std::vector<const std::vector<double>*>      mAspectData;
    const StockData                             *mMarketData = nullptr;
    // std::vector<MarketPlotPoint>              mMarketData;

    DateTime mStartDate       = DateTime::now();
    DateTime mEndDate         = DateTime::now();
    int      mNumDays         = 0;
    DateTime mMarketStartDate = DateTime::now();
    DateTime mMarketEndDate   = DateTime::now();
    
    bool  mLockX        = false;
    bool  mLockY        = true;
    
    bool debug          = false; // show data boundaries
    bool drawVolumeLine = false; // draw volume data as a line above bars
    bool chartCenter    = false; // center view on chart date
    bool drawChartDate  = false; // draw chart date
    DateTime chartDate;          // chart date
    DateTime chartDateLast;      // chart date
    
    PlotWidget();
    ~PlotWidget();

    // void addYAxis(Axis *axis) { mYAxes.push_back(axis); }
    // void addData(const std::vector<double> *data, PlotDataParams *params) { mData.push_back(data); mParams.push_back(params); }
    
    void clearPlanets();
    void clearAspects();
    void addPlanet(const std::vector<PlanetPoint> *points, const std::vector<RxBox> *rx, PlanetDataParams *params);
    void addAspect(const std::vector<double> *points, AspectDataParams *params);
    //void loadMarketData(const std::string &csvPath, bool updateView);
    void setMarketData(const StockData *marketData, bool updateView);
    void updateDates();

    void resetView();

    void setBgColor(const Vec4f &color)  { mBgColor = color; }
    void setDateRange(const DateTime &dtStart, const DateTime &dtEnd);
  
    bool isHovered() const { return mHovered; }
    
    Vec2f draw(ViewSettings *vs, bool blocked, float scale=1.0f, const Vec2f &size=Vec2f(512, 512));
    Vec2f drawLegend(ViewSettings *vs, bool blocked, float scale); // returns legend screen size
    std::vector<AxisLabel> getXAxisLabels(ViewSettings *vs, float scale, DateAxis *axis);
    std::vector<AxisLabel> getYAxisLabels(ViewSettings *vs, float scale, float xOffset, Axis *axis, bool right=false);
    void drawXAxisLabels(ViewSettings *vs, float scale, DateAxis *axis, const std::vector<AxisLabel> &labels);
    void drawYAxisLabels(ViewSettings *vs, float scale, Axis *axis, const std::vector<AxisLabel> &labels, bool right=false);

    bool contextOpen() const { return mContextOpen; }
  };
}


#endif // PLOT_WIDGET_HPP
