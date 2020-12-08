#ifndef COMPARE_NODE_HPP
#define COMPARE_NODE_HPP

#include "astro.hpp"
#include "chartView.hpp"
#include "node.hpp"
#include "chartCompare.hpp"

namespace astro
{
  //// node connector indices ////
  // inputs
#define COMPARENODE_INPUT_CHART_INNER 0
#define COMPARENODE_INPUT_CHART_OUTER 1
  // outputs
  ////////////////////////////////
  
  class CompareNode : public Node
  {
  private:
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()  { return {new Connector<Chart>("Inner Chart"), new Connector<Chart>("Outer Chart")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS() { return {}; }

    ChartView mView;
    ChartCompare *mCompare = nullptr;

    DateTime mDateOuter;
    DateTime mDateInner;
    Location mLocOuter;
    Location mLocInner;
    
    ChartParams mParams;
    float mChartWidth = CHART_SIZE_MIN;

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
    
    void processInput(Chart *chart);
    
    virtual void onUpdate() override;
    virtual void onDraw() override;

  public:
    CompareNode();
    ~CompareNode();
    virtual std::string type() const { return "ChartCompareNode"; }
    
    Chart* outerChart() { return mCompare->getOuterChart(); }
    Chart* innerChart() { return mCompare->getInnerChart(); }
  };
}

#endif // COMPARE_NODE_HPP
