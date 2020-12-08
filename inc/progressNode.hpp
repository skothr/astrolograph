#ifndef PROGRESS_NODE_HPP
#define PROGRESS_NODE_HPP

#include "astro.hpp"
#include "chart.hpp"
#include "node.hpp"
#include "settingForm.hpp"


namespace astro
{
  //// node connector indices ////
  // inputs
#define PROGRESSNODE_INPUT_CHART       0
#define PROGRESSNODE_INPUT_DATE        1
#define PROGRESSNODE_INPUT_LOCATION    2
  // outputs
#define PROGRESSNODE_OUTPUT_CHART      0
  ////////////////////////////////

  class ProgressNode : public Node
  {
  private:
    // node connectors
    static std::vector<ConnectorBase*> CONNECTOR_INPUTS()
    { return {new Connector<Chart>("Chart"), new Connector<DateTime>("Date"), new Connector<Location>("Location")}; }
    static std::vector<ConnectorBase*> CONNECTOR_OUTPUTS()
    { return {new Connector<Chart>("Progressed Chart")}; }

    Chart *mChart = nullptr;
    SettingForm mSettingForm;

    virtual void onUpdate() override;
    virtual void onDraw() override;
    
  public:
    ProgressNode();
    ~ProgressNode();
    virtual std::string type() const { return "ProgressNode"; }
  };
}

#endif // PROGRESS_NODE_HPP
