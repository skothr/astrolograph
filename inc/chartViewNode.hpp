#ifndef CHART_VIEW_NODE_HPP
#define CHART_VIEW_NODE_HPP

#include "astro.hpp"
#include "chartView.hpp"
#include "node.hpp"

namespace astro
{
  //// node connector indices ////
  // inputs
#define CHARTVIEWNODE_INPUT_CHART        0
  // outputs
#define CHARTVIEWNODE_OUTPUT_CHART       0
  ////////////////////////////////

  class ChartViewNode : public Node
  {
  private:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<Chart>("Chart Input")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {}; }//new Connector<Chart>("Chart Ouput (Copy)")}; }

    ChartView mView;
    float mChartWidth = CHART_SIZE_MIN; // (CHART_SIZE_MIN + CHART_SIZE_DEFAULT)/2.0f; // start halfways between minimum size and "default"

    ChartParams mParams;
    bool mOptionsOpen = false;
    bool mDisplayOpen = false;
    bool mOrbsOpen    = false;

    bool mAlignAsc    = false; // align chart so ascendant points to the left
    bool mShowHouses  = true;  // show houses on chart
    
    // date modify flags (not saved)
    bool mEditYear   = false; // toggled with 1 key
    bool mEditMonth  = false; // toggled with 2 key
    bool mEditDay    = false; // toggled with 3 key
    bool mEditHour   = false; // toggled with 4 key
    bool mEditMinute = false; // toggled with 5 key
    bool mEditSecond = false; // toggled with 6 key
    // location modify flags
    bool mEditLat    = false; // toggled with Q key
    bool mEditLon    = false; // toggled with W key
    bool mEditAlt    = false; // toggled with E key

    virtual void onUpdate() override;
    virtual void onDraw() override;

  public:
    ChartViewNode();
    virtual std::string type() const { return "ChartViewNode"; }
    
    void processInput(Chart *chart);
  };
}

#endif // CHART_VIEW_NODE_HPP
