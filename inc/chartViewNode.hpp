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
    float mChartWidth = (CHART_SIZE_MIN + CHART_SIZE_DEFAULT)/2.0f; // start halfways between minimum size and "default"

    std::vector<bool> mShowObjects;
    std::vector<bool> mFocusObjects;
    bool mOptionsOpen = false;
    bool mDisplayOpen = false;
    bool mOrbsOpen    = false;

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

    virtual bool onDraw() override;
    
    virtual std::map<std::string, std::string>& getSaveParams(std::map<std::string, std::string> &params) const override
    {
      params.emplace("width", std::to_string(mChartWidth));
      return params;
    };
    
    virtual std::map<std::string, std::string>& setSaveParams(std::map<std::string, std::string> &params) override
    {
      std::istringstream ss(params["width"]);
      ss >> mChartWidth;
      return params;
    };
    
  public:
    ChartViewNode();
    virtual std::string type() const { return "ChartViewNode"; }
    virtual bool copyTo(Node *other) override
    { // copy settings
      if(Node::copyTo(other))
        {
          ((ChartViewNode*)other)->mOptionsOpen = mOptionsOpen;
          ((ChartViewNode*)other)->mDisplayOpen = mDisplayOpen;
          ((ChartViewNode*)other)->mOrbsOpen    = mOrbsOpen;
          ((ChartViewNode*)other)->mChartWidth  = mChartWidth;

          for(int i = OBJ_SUN; i < OBJ_COUNT; i++)
            { ((ChartViewNode*)other)->mShowObjects[i] = mShowObjects[i]; }
          for(int i = ANGLE_OFFSET; i < ANGLE_END; i++)
            { ((ChartViewNode*)other)->mShowObjects[i-ANGLE_OFFSET+OBJ_COUNT] = mShowObjects[i-ANGLE_OFFSET+OBJ_COUNT]; }
          
          // (everything else set by connections)
          return true;
        }
      else { return false; }
    }

    void processInput(Chart *chart);
  };
}

#endif // CHART_VIEW_NODE_HPP
