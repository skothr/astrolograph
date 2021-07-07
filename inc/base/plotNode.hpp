#ifndef PLOT_NODE_HPP
#define PLOT_NODE_HPP

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"
#include "plotWidget.hpp"

namespace astro
{
  // forward declarations
  class FileDialog;
  class SettingForm;

  //// node connector indices ////
  // inputs
#define PLOTNODE_INPUT_CHART      0
#define PLOTNODE_INPUT_MARKETDATA 1
#define PLOTNODE_INPUT_STARTDATE  2
#define PLOTNODE_INPUT_ENDDATE    3
  // outputs
  ////////////////////////////////

  class PlotNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<Chart>("Chart"),
              new Connector<MarketData>("Market Data"),
              new Connector<DateTime>("Start Date"),
              new Connector<DateTime>("End Date")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {}; }

    PlotWidget *mWidget = nullptr;
    Vec2f mPlotSize = Vec2f(1420, 690);
    
    FileDialog  *mFileDialog    = nullptr;
    std::string mMarketDataPath = "";
    // std::string mMarketDataPath = "";
    char mTicker[8] = "";

    DateTime mOldStartDate;
    DateTime mOldEndDate;
    Chart    mOldChart;
    bool     mUpdateView    = false;
    bool     mReloadData    = false;
    bool     mReloadAspects = false;
    std::vector<std::vector<PlanetPoint>> mPlanetData; // planet data
    std::vector<std::vector<RxBox>>       mRxData;     // planet data
    std::vector<PlanetDataParams*>        mParams;     // planet plot params

    SettingForm *mSettingForm = nullptr;
    bool         mSettingsOpen    = false;
    bool         mAspectsOpen     = false;
    bool         mViewParamsOpen  = false;
    bool         mResetParamsOpen = false;
    bool         mLabelParamsOpen = false;

    std::vector<std::string> mAspNames;
    std::vector<std::string> mObjNames;
    ObjType    mAspectObj1 = OBJ_SATURN;
    ObjType    mAspectObj2 = OBJ_URANUS;
    AspectType mAspectType = ASPECT_SQUARE;
    AspectDataParams    mAspectParams;
    std::vector<double> mAspectData;
    
    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onResize(const Vec2f &dSize) override;

    // returns path to downloaded data (./data/XXX-auto.csv)
    std::string getTickerDataCurl(std::string ticker);
    bool checkFileDialog();

    std::vector<std::vector<RxBox>> findRetrogrades(const std::vector<std::vector<PlanetPoint>> &planetData, const DateTime &dtStart, const DateTime &dtEnd);

    
  public:
    PlotNode();
    ~PlotNode();
    virtual std::string type() const { return "PlotNode"; }
  };
}

#endif // PLOT_NODE_HPP
