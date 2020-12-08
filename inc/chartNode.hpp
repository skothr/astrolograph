#ifndef CHART_NODE_HPP
#define CHART_NODE_HPP

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"

namespace astro
{
  // forward declarations
  class SettingForm;

  //// node connector indices ////
  // inputs
#define CHARTNODE_INPUT_DATE        0
#define CHARTNODE_INPUT_LOCATION    1
  // outputs
#define CHARTNODE_OUTPUT_CHART      0
  ////////////////////////////////

  class ChartNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<DateTime>("Date"), new Connector<Location>("Location")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {new Connector<Chart>("Chart")}; }
    
    Chart       *mChart       = nullptr;
    SettingForm *mSettingForm = nullptr;

    bool mOptionsOpen = false;
    bool mTruePos = false; // 
    std::vector<std::string> mZNames;
    std::vector<std::string> mHsNames;
    int mHouseSystem = 0;  // combo index
    int mZodiac      = ZODIAC_TROPICAL; // combo index

    virtual void onUpdate() override;
    virtual void onDraw() override;
    
  public:
    ChartNode(Chart *chart=nullptr);
    ChartNode(const DateTime &dt, const Location &loc);
    ~ChartNode();
    virtual std::string type() const { return "ChartNode"; }
    virtual bool onConnect(ConnectorBase *con) override;
    
    Chart* chart() { return mChart; }
  };
}

#endif // CHART_NODE_HPP
