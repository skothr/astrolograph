#ifndef CHART_VIEW_NODE_HPP
#define CHART_VIEW_NODE_HPP


#include "node.hpp"
#include "astro.hpp"

// forward declarations
class SettingGroup;

namespace astro
{
  // forward declarations
  class Chart;
  class ChartView;
  class ChartParams;
  class ChartParamWidget;
  class ChartOrbWidget;
  
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
    { return { new Connector<Chart>("Chart Input") }; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return { new Connector<Chart>("Chart Ouput") }; }

    bool mSettingsOpen = false;
    bool mDisplayOpen  = false;
    bool mOrbsOpen     = false;
    std::array<bool, (OBJ_END-ANGLE_OFFSET+OBJ_COUNT)> mObjDisplayFocused;
    std::array<bool, (ASPECT_COUNT)>                   mAspDisplayFocused;
    std::array<bool, (OBJ_END-ANGLE_OFFSET+OBJ_COUNT)> mOrbsFocused;
    
    ChartView        *mView        = nullptr;
    ChartParamWidget *mParamWidget = nullptr;
    ChartParams      *mParams      = nullptr;
    ChartOrbWidget   *mOrbWidget   = nullptr;
    
    // date modify flags
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
    // editing anything
    bool mEditing    = false;

    virtual void onUpdate() override;
    virtual void onDraw() override;
    virtual void onResize(const Vec2f &dSize) override;

    void drawSettings();

    
  public:
    ChartViewNode();
    ~ChartViewNode();
    virtual std::string type() const { return "ChartViewNode"; }
    
    void processInput(Chart *chart);
  };
}

#endif // CHART_VIEW_NODE_HPP
